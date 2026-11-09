# TD 2 - Read/write (suite) : réponses

Compilation : `make` (crée aussi `donnees.txt` contenant `abcdefgh`).

| Fichier | Exercice |
|---|---|
| `utils.h`, `transfer.h` | `exit_if()`, fonction `transfer()` fournie, `write_all()` |
| `filetype.c` | 1. Type de l'entrée standard (`fstat`) |
| `client.c`, `serveur.c` | 2. Chat par FIFO nommée |
| `alias.c`, `alias_open.c`, `alias_dup.c` | 3. Descripteurs et `dup` |
| `myls.c` | 4. `ls` (+ options `-l`, `-R`, droits, date, utilisateur) |
| `chat_serveur.c`, `chat_client.c` | 5.1 Chat à deux (bidirectionnel) |

---

## 1. Type de fichier - entrée standard

`fstat(STDIN_FILENO, &st)` remplit une `struct stat` ; le type est codé dans
`st.st_mode` et se teste avec les macros `S_ISREG`, `S_ISDIR`, `S_ISCHR`,
`S_ISBLK`, `S_ISFIFO`, `S_ISLNK`, `S_ISSOCK` (cf. `inode(7)`).

```
$ ./filetype                 -> périphérique caractère (le terminal, /dev/pts/N)
$ ./filetype < /etc          -> répertoire
$ ./filetype < ./filetype    -> fichier régulier
$ echo | ./filetype          -> FIFO / tube
```

Remarque : `S_ISLNK` n'est jamais vrai avec `fstat` : la redirection ouvre la
cible du lien (un descripteur ne peut pas désigner un lien) ; il faut `lstat`
sur un chemin pour voir un lien symbolique.

## 2. Pipe/FIFO nommé (chat)

**Client** (`client.c`) : `open("versserveur", O_WRONLY)` puis
`transfer(STDIN_FILENO, fd)`.

**Envoyer depuis son poste vers la FIFO du serveur : que se passe-t-il ?**
Rien n'arrive au serveur (selon le cas, `open` reste bloqué ou les messages
sont perdus). Une FIFO n'est qu'un *point de rendez-vous* dans le système de
fichiers : les données transitent **dans la mémoire du noyau de la machine**
où sont exécutés les processus. Le répertoire personnel est partagé par NFS,
donc on voit bien le fichier `versserveur`, mais ouvrir la FIFO sur *son*
poste crée un tube dans le noyau de *son* poste, différent de celui de la
machine où tourne le serveur. NFS ne transporte pas le contenu des FIFOs
entre machines. `open` en écriture bloque tant qu'aucun lecteur **local** n'a
ouvert la FIFO.

**En SSH sur ssh.enseirb-matmeca.fr ça fonctionne : pourquoi ?** Le client et
le serveur s'exécutent alors sur **la même machine**, donc sous le même noyau :
ils partagent le même tube interne.

**Plusieurs clients en même temps** : tous les clients écrivent dans le même
tube ; le serveur lit tout, mélangé, dans l'ordre d'arrivée, sans savoir qui
a écrit quoi. Les écritures de taille ≤ `PIPE_BUF` (4096 octets sous Linux)
sont **atomiques** (une ligne n'est pas coupée par celle d'un autre), au-delà
elles peuvent s'entrelacer. Le serveur reçoit EOF (`read` renvoie 0) seulement
quand **tous** les écrivains ont fermé la FIFO.

**Serveur** (`serveur.c`) : `mknod(fifo, S_IFIFO | 0666, 0)` (= `mkfifo`),
`open(O_RDONLY)`, puis `transfer(fd, STDOUT_FILENO)`.

**Transmettre des fichiers entiers (binaires, images…)** : il ne faut rien
supposer sur le contenu : pas de fonctions de chaînes (`strlen`, `printf("%s")`,
`fgets`…) qui s'arrêtent au `'\0'` ou au `'\n'`, mais uniquement `read`/`write`
avec le nombre d'octets **réellement lus**, en bouclant sur les écritures
partielles, jusqu'à EOF. C'est exactement ce que fait `transfer()`, donc
aucune modification n'est nécessaire :

```sh
./serveur > copie.png &
./client < image.png
cmp image.png copie.png    # identiques
```

## 3. Descripteurs de fichier et `dup`

```
$ ./alias_open        $ ./alias_dup
abcd                  abcd
abcd                  efgh
```

Explication (3 niveaux de tables) :
- **table des descripteurs** (propre au processus) → **table des fichiers
  ouverts** (dans le noyau : mode d'ouverture + **offset courant**) →
  **table des inodes** (le fichier lui-même).
- Un **deuxième `open`** crée une **nouvelle entrée** dans la table des
  fichiers ouverts, avec son propre offset à 0 : on relit `abcd`.
- **`dup`** crée un nouveau descripteur qui pointe vers **la même entrée** de
  la table des fichiers ouverts : l'offset est **partagé** (le premier `read`
  l'a avancé à 4), donc on lit la suite : `efgh`. C'est de l'*aliasing* :
  les deux descripteurs sont deux noms pour la même ouverture. (C'est aussi ce
  qui se passe après un `fork`, et ce que fait le shell avec `2>&1`.)

## 4. `directory`, `ls` — `myls.c`

- `opendir` / `readdir` / `closedir` pour parcourir le répertoire.
- `lstat` (et non `stat`) sur `rep/nom` pour connaître le type : `stat` suit
  les liens symboliques et afficherait le type de la cible ; `fstat` nécessite
  un descripteur ouvert. Lettres : `-` régulier, `d` répertoire, `l` lien,
  `c`/`b` périphériques, `p` FIFO, `s` socket.
- Les répertoires sont passés en arguments ; sans argument on liste `.`.

```sh
ln -s donnees.txt lien
./myls -l             # ... lrwxrwxrwx 1 user group 11 2026-11-09 12:23 lien -> donnees.txt
./myls . .. / /tmp
./myls -R -l .
```

## 5. Pour aller plus loin

### 5.1 Chat à deux

Voir `chat_serveur.c` / `chat_client.c` (lancer le serveur, puis le client,
dans le même répertoire et sur la même machine).

**Pourquoi deux FIFOs ?** Un tube est **unidirectionnel** et ne garde pas
l'identité de l'écrivain : si les deux processus lisaient et écrivaient dans
la même FIFO, chacun pourrait relire ses propres messages (celui qui lit le
premier « consomme » les données, quel que soit le destinataire). Il faut donc
un tube par sens : `versserveur` (client → serveur) et `versclient`
(serveur → client).

**Attention aux blocages** :
- `open` d'une FIFO bloque jusqu'à ce que l'autre extrémité soit ouverte : le
  client et le serveur doivent ouvrir les deux FIFOs **dans le même ordre**
  (serveur : `versserveur` en lecture puis `versclient` en écriture ; client :
  `versserveur` en écriture puis `versclient` en lecture), sinon interblocage.
- `read` est bloquant : on ne peut pas attendre à la fois le clavier et la
  FIFO avec deux `read` successifs. Solutions : `poll(2)`/`select(2)` (utilisé
  ici), deux processus (`fork` : l'un lit le clavier, l'autre la FIFO), ou
  `O_NONBLOCK`.

### 5.2 Chat à N

Une seule FIFO `versserveur` partagée par tous les clients en écriture (les
écritures ≤ `PIPE_BUF` sont atomiques, les messages ne se mélangent pas), et
**une FIFO par client** pour la réception (par ex. `versclient_<pid>`). Le
client, en se connectant, envoie un message d'inscription contenant le nom de
sa FIFO ; le serveur tient la liste des clients et **rediffuse** chaque
message reçu en l'écrivant dans toutes les FIFOs clientes (en retirant un
client quand l'écriture échoue avec `EPIPE`, pensez à ignorer `SIGPIPE`).

### 5.3 Ne pas recevoir son propre écho

On définit un protocole : chaque message est `[id (1 octet)][longueur][données]`
(ou `[id][ligne terminée par \n]`). L'identifiant est attribué par le serveur
lors de l'inscription (ou dérivé du pid, modulo 256). Le serveur rediffuse le
message tel quel ; un client qui lit un message dont le premier octet est son
propre id ne l'affiche pas (ou, mieux, le serveur ne le renvoie pas à
l'émetteur). Comme une écriture ≤ `PIPE_BUF` est atomique, il faut écrire
l'en-tête et les données **en un seul `write`** pour ne pas les séparer.

### 5.4 ls, la suite

Implémenté dans `myls.c` :
- `getopt(argc, argv, "lR")` ; `-l` active le format long (désactivé par
  défaut : seuls les noms sont affichés) ;
- `-R` : après l'affichage d'un répertoire, `rewinddir` puis appel récursif
  sur chaque sous-répertoire (hors `.` et `..`, et sans suivre les liens
  symboliques grâce à `lstat`, pour éviter les boucles) ;
- format long : type + droits `rwxrwxrwx` (bits de `st_mode`), nombre de liens,
  propriétaire (`getpwuid`), groupe (`getgrgid`), taille, date `st_ctime`
  (date de dernier changement de l'inode — Unix ne stocke traditionnellement
  pas de vraie date de création), et cible des liens (`readlink`).
