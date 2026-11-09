# TD 4 - Fork et tubes non nommés : réponses

Compilation : `make`.

| Fichier | Exercice |
|---|---|
| `mypipe.c` | 1. et 2. Tube non nommé / règles pour les tubes |
| `majuscules.c` | 3. Mise en majuscules + `ps` |
| `pipeline.c` (→ `pipeline` et `pipeline_simple`) | 4. Pipeline codé en dur |
| `pipeline_args.c` | 5. Pipeline donné en arguments (`--`) |

---

## 1. Création d'un tube non nommé — `mypipe.c`

`pipe(tube)` crée un tube : `tube[0]` = extrémité de lecture, `tube[1]` =
extrémité d'écriture. Après `fork`, le père **et** le fils possèdent les deux
descripteurs (la table des descripteurs est copiée). Le père écrit
`NB` fois `"Hello World"` (11 octets) ; le fils fait **un seul** `read` d'au
plus 500 octets.

```
$ ./mypipe
Je suis le père, j'ai écrit 110 octets et je quitte
Hello WorldHello World...(x10)
Je suis le fils, j'ai lu 110 octets et je quitte
```

Remarque : selon l'ordonnancement, le fils peut lire avant que le père ait
tout écrit, et donc lire moins de 110 octets : un `read` sur un tube renvoie ce
qui est disponible, pas forcément ce qui a été demandé.

## 2. Règles pour les tubes

Avec 50 ou 100 messages (550 / 1100 octets) le fils lit au plus 500 octets
(le reste est perdu, détruit à la fermeture du tube).

**Pour 5000 / 10000 messages (55 / 110 Ko), la première version (qui ne
ferme pas les extrémités inutiles) se bloque.** Avec `ps`, on voit le père
en état `S` (endormi dans `pipe_write`) et le fils terminé (zombie `Z` ou
disparu).

Schéma des tables de descripteurs après `fork` :

```
          père                         fils
   0 stdin  1 stdout  2 stderr   0 stdin  1 stdout  2 stderr
   3 ──┐ tube[0] (lecture)       3 ──┐ tube[0]
   4 ──┼─┐ tube[1] (écriture)    4 ──┼─┐ tube[1]
       │ │                           │ │
       └─┴───────────►  [ TUBE (64 Kio) ]  ◄───────┘
```

Explication : un tube a une capacité limitée (64 Kio sous Linux). Quand il est
plein, `write` **bloque** tant qu'il existe un lecteur potentiel, c'est-à-dire
tant qu'**au moins un descripteur ouvert en lecture** existe sur le tube. Le
fils a lu 500 octets et est terminé, mais **le père lui-même possède encore
`tube[0]`** : il reste un lecteur (lui-même !) qui ne lira jamais → blocage
infini.

**Correction** : chaque processus ferme immédiatement l'extrémité qu'il
n'utilise pas : le père ferme `tube[0]`, le fils ferme `tube[1]`
(c'est également ce qui permet à un lecteur de recevoir EOF : `read` ne renvoie
0 que lorsque **tous** les descripteurs en écriture sont fermés).

Désormais, quand le fils a terminé, il n'y a plus aucun lecteur : le `write`
suivant du père échoue (`EPIPE`) et le noyau lui envoie le signal **`SIGPIPE`**,
dont l'action par défaut est de tuer le processus. Le père ne peut donc pas
afficher son message.

```
$ ./mypipe 10000 &
[1] 1234
Je suis le fils, j'ai lu 500 octets et je quitte
[1]+  Broken pipe             ./mypipe 10000
$ echo $?            # (en avant-plan) : 141 = 128 + 13 (SIGPIPE)
```

**Statut de complétion : `Broken pipe`** — le père a été tué par le signal
`SIGPIPE` (n° 13 ; le shell code cela par 128 + 13 = 141). Ce cas peut même se
produire avec 1000 messages (11 Ko, moins que la capacité du tube) si le fils
a fini avant que le père ait terminé d'écrire. On pourrait ignorer `SIGPIPE`
(`signal(SIGPIPE, SIG_IGN)`) et traiter l'erreur `EPIPE` de `write`.

## 3. Mise en majuscules — `majuscules.c`

- Le fils ferme `tube[1]`, puis lit en boucle sur `tube[0]` jusqu'à EOF (`read`
  renvoie 0), passe chaque octet en majuscule avec `toupper` et l'écrit sur sa
  sortie standard.
- Le père ferme `tube[0]`, fait `dup2(tube[1], STDOUT_FILENO)`, ferme
  `tube[1]` (la copie sur le descripteur 1 suffit), puis
  `execlp("ps", "ps", NULL)`. La sortie de `ps` part dans le tube.
- Quand `ps` se termine, le dernier descripteur d'écriture est fermé : le fils
  reçoit EOF et se termine. (Si le fils n'avait pas fermé `tube[1]`, il serait
  lui-même écrivain et ne recevrait jamais EOF.)

```
$ ./majuscules
  PID TTY          TIME CMD
 4242 PTS/0    00:00:00 BASH
 4250 PTS/0    00:00:00 PS
 4251 PTS/0    00:00:00 MAJUSCULES
```

## 4. Pipeline de commandes — `pipeline.c`

Algorithme pour `n` commandes (fonction `lancer_pipeline`) :

```
entree = STDIN
pour i de 0 à n-1 :
    si i n'est pas la dernière : pipe(tube)
    fork
    enfant :  dup2(entree, 0)       si entree != STDIN
              dup2(tube[1], 1)      si pas la dernière
              ferme tous les descripteurs de tube inutiles
              execvp(cmd[i])
    parent :  ferme entree (si != STDIN) et tube[1]
              entree = tube[0]       (lecture pour la commande suivante)
```

Points importants :
- Le parent doit fermer au fur et à mesure ses copies des extrémités des
  tubes : sinon un lecteur (par ex. `tr` ou `wc`) ne verrait jamais EOF et le
  pipeline ne se terminerait jamais.
- Le parent attend **le dernier processus** (`waitpid(pid_dernier)`) : s'il
  s'est terminé normalement (`WIFEXITED`), on renvoie `WEXITSTATUS(status)`
  (valeur propagée) ; sinon on affiche la raison (`WIFSIGNALED`, `WTERMSIG`,
  `strsignal`) et on renvoie `EXIT_FAILURE`.
- La même boucle fonctionne pour **2 commandes ou plus** (`pipeline_simple`
  compilé avec `-DSIMPLER_VERSION` : `ls -l | grep .c | wc -l`). Avec plus de
  deux commandes, chaque commande intermédiaire a **deux** tubes (un en
  entrée, un en sortie).
- `nb_commandes = sizeof(commandes) / sizeof(char*)` : `commandes` est un
  tableau de `char **`, et `sizeof(char**) == sizeof(char*)`, donc cela donne
  bien le nombre d'éléments (`sizeof(commandes[0])` serait plus rigoureux).

```
$ ./pipeline          # ls -l | tr -d [:blank:]
total136
-rw-r--r--1useruser300Nov912:26Makefile
...
$ ./pipeline_simple   # ls -l | grep .c | wc -l
5
```

Remarque : les autres processus du pipeline (non attendus) deviennent zombies
jusqu'à la fin de `pipeline`, puis sont adoptés et nettoyés par `init`. On
pourrait aussi tous les attendre après le dernier.

## 5. Pipeline donné en arguments — `pipeline_args.c`

```sh
./pipeline_args ls -l -- grep toto -- less     # <=> ls -l | grep toto | less
```

`split_args` parcourt `argv`, remplace chaque `"--"` par `NULL` (ce qui
termine la commande précédente : `execvp` attend un tableau terminé par
`NULL`) et mémorise le début de chaque commande dans un tableau `char ***`.
Le reste est identique à l'exercice 4 (on réutilise `lancer_pipeline`).

```
$ ./pipeline_args ls -l -- grep '\.c' -- wc -l ; echo $?
4
0
$ ./pipeline_args ls -- false ; echo $?          # code du dernier propagé
1
```
