# TD 3 - Bufferisation - Fork : réponses

Compilation : `make`.

| Fichier | Exercice |
|---|---|
| `buffer.c` | 2. libc et buffers |
| `fork_pid.c` | 3. Fork |
| `lance.c`, `segfault.c` | 4. Fork et redirection |

---

## 1. Appels système et libc

| Fonction | Nature | Indice dans le prototype |
|---|---|---|
| `printf()` | **libc** (3) | travaille sur `stdout` (`FILE *` implicite), format |
| `fopen()` | **libc** (3) | renvoie un `FILE *` |
| `fclose()` | **libc** (3) | prend un `FILE *` |
| `fread()` | **libc** (3) | prend un `FILE *` |
| `open()` | **appel système** (2) | renvoie un `int` descripteur |
| `close()` | **appel système** (2) | prend un `int` descripteur |
| `lseek()` | **appel système** (2) | prend un `int` descripteur, renvoie `off_t` |
| `rewind()` | **libc** (3) | prend un `FILE *` (≈ `fseek(f, 0, SEEK_SET)`) |
| `write()` | **appel système** (2) | prend un `int` descripteur |
| `exit()` | **libc** (3) | vide les buffers, appelle les `atexit`, puis `_exit` |
| `_exit()` | **appel système** (2) | termine immédiatement le processus |

Règle : les fonctions manipulant un `FILE *` (flux bufferisés) sont dans la
libc ; celles manipulant un descripteur `int` sont des appels système. Les
fonctions de la libc sont construites au-dessus des appels système
(`fopen` → `open`, `fread` → `read`, `printf` → `write`, `exit` → `_exit`).

## 2. libc et buffers — `buffer.c`

La libc utilise trois modes de bufferisation (`setvbuf(3)`) :
1. **sans buffer** (`_IONBF`) : `stderr` ; chaque `fprintf` fait aussitôt un `write`.
2. **bufferisé par ligne** (`_IOLBF`) : `stdout` **quand c'est un terminal** ;
   le buffer est vidé à chaque `'\n'` (ou quand il est plein, ou à la lecture
   sur stdin, ou à `exit`).
3. **bufferisé par bloc** (`_IOFBF`) : `stdout` **quand c'est un fichier ou un
   tube** ; le buffer (≈ 4 Kio) n'est vidé que lorsqu'il est plein, à
   `fflush` ou à `exit`.

`write(2)` n'a aucun buffer visible en espace utilisateur : les données sont
transmises au noyau immédiatement.

Résultats (« | » = pause d'une seconde) :

| Compilation | `./buffer` (terminal) | `./buffer 2>&1 \| cat -u` (tube) |
|---|---|---|
| 1. `-DUTILISER_FPRINTF1` | `\| World!Hello` | `\| World!Hello` |
| 2. `-DUTILISER_FPRINTF2 -DUTILISER_SORTIE_ERREUR2` | `Hello \| World!` (write puis stderr non bufferisé) | `Hello \| World!` |
| 3. `-DUTILISER_FPRINTF1 -DUTILISER_FPRINTF2 -DUTILISER_SORTIE_ERREUR2` | `\| World!Hello` | `\| World!Hello` |
| 4. cas 3 avec `CHAINE1='"Hello\n"'` | `Hello⏎ \| World!` | `\| World!Hello⏎` |
| (bonus) `-DUTILISER_FPRINTF1 -DUTILISER__EXIT` | `\| World!` (Hello perdu) | `\| World!` |

Explications :
- **Cas 1** : `Hello` est écrit par `fprintf` dans le buffer de stdout, sans
  `\n` : il n'est pas transmis. ` World!` est écrit par `write`, donc
  immédiatement. `Hello` n'est émis qu'au `exit()` (qui fait `fflush`) : on
  obtient ` World!Hello`, avec rien d'affiché pendant la seconde d'attente.
- **Cas 2** : `write` puis `fprintf(stderr)` : aucun des deux n'est bufferisé,
  l'ordre est respecté : `Hello`, pause, ` World!`.
- **Cas 3** : stdout bufferisé, stderr non bufferisé : ` World!` (stderr) sort
  avant `Hello` (stdout, vidé à `exit`). Cela montre que les deux flux ont des
  buffers indépendants.
- **Cas 4 (ajout de `\n`)** : dans le terminal, stdout est bufferisé **par
  ligne** : le `\n` provoque le vidage, `Hello` s'affiche **avant** la pause.
  Dans le tube (`| cat -u`), stdout est bufferisé **par bloc** : le `\n` ne
  change rien, `Hello\n` n'est vidé qu'à `exit`, donc après ` World!`. C'est
  ce qui permet de distinguer bufferisation par ligne et par bloc.
- **Avec `_exit`** : `_exit` est l'appel système, il ne vide pas les buffers
  de la libc : tout ce qui est encore dans le buffer de stdout est **perdu**.
  `exit` (libc) appelle `fflush` sur tous les flux puis `_exit`.

Commandes :

```sh
rm -f buffer && make buffer CFLAGS+="-DUTILISER_FPRINTF1" && ./buffer && ./buffer 2>&1 | cat -u
rm -f buffer && make buffer CFLAGS+='-DUTILISER_FPRINTF1 -DUTILISER_FPRINTF2 -DUTILISER_SORTIE_ERREUR2 -DCHAINE1="\"Hello\n\""'
```

## 3. Fork — `fork_pid.c`

Le problème : `printf` sans `\n` laisse le texte dans le buffer de stdout.
`fork` duplique tout l'espace mémoire, **y compris ce buffer** : le texte
serait affiché deux fois (par l'enfant et par le parent à sa terminaison),
alors que le parent ne doit plus rien afficher après `fork`.

Trois solutions :
1. **`fflush(stdout)` juste avant `fork()`** : le buffer est vide au moment de
   la duplication (solution retenue : simple, explicite, et sans effet de bord).
2. **Le parent termine par `_exit()`** au lieu de `exit()`/`return` : son
   buffer n'est jamais vidé, seule la copie de l'enfant est affichée (fonctionne,
   mais c'est l'enfant qui affiche le texte du parent et c'est fragile).
3. **Supprimer la bufferisation** : `setvbuf(stdout, NULL, _IONBF, 0)` au
   début du programme (ou écrire le premier message avec `write(2)` ou sur
   `stderr`).

Le parent attend son enfant (`wait`) pour que `getppid()` dans l'enfant
renvoie bien le pid du parent (sinon l'enfant orphelin est rattaché à
`init`/`systemd`, pid 1 ou un *subreaper*).

```
$ ./fork_pid
Parent : pid = 549. Enfant : pid = 550, pid du parent = 549
```

## 4. Fork et redirection — `lance.c`

```sh
./lance sortie ls -l /etc        # équivalent : ls -l /etc > sortie
```

1. La commande est écrite dans `sortie` avec `write`, puis exécutée avec
   `execvp(argv[2], argv + 2)` (`execvp` cherche la commande dans le `PATH` ;
   `argv + 2` est déjà un tableau terminé par `NULL`). Sans `fork`, `execvp`
   **remplace** le programme courant : rien après n'est exécuté (sauf échec).
2. On fait un `fork` : l'enfant appelle `execvp`, le parent continue.
3. Dans l'enfant, `dup2(fd, STDOUT_FILENO)` fait du descripteur 1 une copie
   de `fd` ; on ferme ensuite `fd` (inutile). Les descripteurs sont conservés
   à travers `exec`, donc la commande écrit dans le fichier.
   Le fichier est ouvert avec `O_WRONLY | O_CREAT | O_TRUNC` comme `>`. Comme
   l'offset est partagé entre parent et enfant (même entrée de la table des
   fichiers ouverts), la sortie de la commande s'ajoute **après** la ligne de
   commande écrite par le parent.
4. Le parent attend l'enfant avec `waitpid` et décode le statut :
   `WIFEXITED`/`WEXITSTATUS` (terminaison normale et code de retour),
   `WIFSIGNALED`/`WTERMSIG` (tué par un signal), `WCOREDUMP`.
5. Test avec `segfault.c` (écriture à l'adresse 0, non mappée → le noyau
   envoie `SIGSEGV`, signal 11) :

```
$ ./lance out.txt ./segfault
./segfault a été tué par le signal 11 (Segmentation fault)
$ cat out.txt
./segfault
```

Remarque : le message `Je vais faire une faute de segmentation...` de
`segfault` n'apparaît pas dans `out.txt` ! Sa sortie standard est un fichier,
donc bufferisée par bloc, et le processus est tué par le signal avant d'avoir
vidé son buffer (pas de passage par `exit`). Encore un effet des buffers.

Si la commande n'existe pas, `execvp` échoue : l'enfant affiche l'erreur et
quitte avec le code 127 (comme le shell).
