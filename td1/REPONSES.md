# TD 1 - Read/write : réponses

Compilation : `make` (et `make clean`).

| Fichier | Exercice |
|---|---|
| `utils.h` | macro `exit_if()` + `struct nopad` (et variantes) |
| `echo.c` | 3. Écriture avec `write(2)` |
| `write_number.c` | 4. Écriture d'un entier |
| `read.c` | 6. Lecture avec `read(2)` |
| `read_number.c` | 7. Lecture d'un entier |
| `write_struct.c`, `read_struct.c` | 8. Lecture/écriture d'une structure |

---

## 1. Pages de manuel et sections

```sh
man -k printf | grep ^printf
```

- **Section 3** : documentation de la fonction de bibliothèque telle qu'elle est
  *implémentée* sur ce système (ici la glibc / Linux man-pages), avec ses
  extensions, ses particularités et ses bugs connus.
- **Section 3p / 3posix** : documentation de la même fonction telle qu'elle est
  *spécifiée par la norme POSIX*. C'est le comportement garanti et portable ;
  l'implémentation réelle peut être différente ou plus riche.
- `printf()` (fonction de la libc) est documentée dans **`printf(3)`** (et
  `printf(3p)` pour la version POSIX).
- Les autres pages trouvées :
  - `printf(1)` (et `printf(1p)`) : la **commande** shell `printf` (section 1 =
    commandes utilisateur), utilisée dans les scripts : `printf "%d\n" 42`.
  - Les pages sœurs listées par `man -k printf` : `fprintf(3)`, `sprintf(3)`,
    `snprintf(3)`, `dprintf(3)`, `vprintf(3)`… qui sont la même famille de
    fonctions (écriture sur un `FILE*`, dans une chaîne, sur un descripteur,
    avec une `va_list`…), souvent documentées dans la même page.

Rappel des sections : 1 commandes, **2 appels système**, **3 fonctions de
bibliothèque**, 4 fichiers spéciaux, 5 formats de fichiers, 6 jeux, 7 divers
(conventions, `inode(7)`, `signal(7)`…), 8 commandes d'administration.

## 2. Implémentation des appels système

```sh
objdump -d /usr/lib/x86_64-linux-gnu/libc.so.6 | less     # puis /syscall
```

Exemple typique (fonction `write`, simplifié) :

```asm
<__write>:
    ...
    mov    $0x1,%eax        ; numéro de l'appel système : 1 = write
    syscall                 ; passage en mode noyau
    cmp    $0xfffffffffffff000,%rax
    ja     <erreur>         ; si -4095 <= rax < 0 : erreur -> errno = -rax, retour -1
    ret

<__read>:
    xor    %eax,%eax        ; 0 = read
    syscall
```

Le système d'exploitation reconnaît l'appel demandé grâce au **numéro d'appel
système placé dans le registre `rax` (`eax`) juste avant l'instruction
`syscall`** (0 = `read`, 1 = `write`, 2 = `open`, 3 = `close`, 39 = `getpid`,
57 = `fork`, 59 = `execve`, 60 = `exit`, 231 = `exit_group`…). Le noyau
utilise ce numéro comme indice dans sa table des appels système
(`sys_call_table`). Les arguments sont passés dans les registres `rdi`, `rsi`,
`rdx`, `r10`, `r8`, `r9` et la valeur de retour revient dans `rax` (une valeur
négative entre -4095 et -1 est un code d'erreur que la libc range dans
`errno` en renvoyant -1). Cela correspond à la table donnée en lien.

## 3. Écriture avec `write(2)` — `echo.c`

`ssize_t write(int fd, const void *buf, size_t count);` écrit au plus `count`
octets ; elle renvoie le nombre d'octets réellement écrits (qui peut être
**inférieur** à `count`) ou -1 en cas d'erreur (avec `errno` positionné).
Le programme boucle donc jusqu'à avoir tout écrit et teste chaque retour.

**La macro `exit_if(cond, prefix)`** : si `cond` est vraie, elle affiche le
fichier et la ligne, puis appelle `perror(prefix)` qui affiche sur `stderr`
`prefix: <message correspondant à errno>` (texte obtenu via `strerror(errno)`),
puis termine le programme avec `exit(EXIT_FAILURE)`. Le `do { } while(0)`
permet d'utiliser la macro comme une instruction normale (avec `;`, dans un
`if` sans accolades…).

Test d'erreur :

```sh
$ ./echo blabla < . 1>&0
echo.c:12: write: Bad file descriptor
```

`< .` ouvre le répertoire courant **en lecture seule** sur le descripteur 0,
puis `1>&0` fait de la sortie standard une copie de ce descripteur. `write`
sur un descripteur ouvert en lecture seule échoue avec `EBADF`.

## 4. Écriture d'un entier — `write_number.c`

```sh
$ ./write_number
$                      # rien de lisible (ou caractères de contrôle bizarres)
$ ./write_number | od -x
0000000 0005 0000 0000 0000 0008 0000 0000 0000
```

`write` écrit le **contenu mémoire brut** de la variable et non sa
représentation textuelle. Les octets `05 00 00 00 00 00 00 00` ne sont pas des
caractères imprimables (0x05 n'est pas le caractère `'5'` = 0x35), d'où un
affichage vide/illisible dans le terminal. Avec `od` on voit :
- les 8 octets du `long` (sizeof(long) = 8 sur x86_64) valant 5, en
  **little-endian** (octet de poids faible en premier) ;
- puis 8 octets valant 8 : le nombre d'octets écrits par le premier `write`
  (de type `ssize_t`, lui aussi sur 8 octets).

## 5. Gestion des erreurs

`perror()` **n'est pas un appel système** : c'est une fonction de la
bibliothèque C (section 3, `perror(3)`). Elle ne fait que lire la variable
globale `errno` (positionnée par la libc après l'échec d'un appel système),
la traduire en texte (`strerror`) et écrire le message sur `stderr`, ce qui
nécessite elle-même, en interne, un appel système `write(2)`. Elle ne demande
aucun service propre au noyau.

## 6. Lecture avec `read(2)` — `read.c`

`read` renvoie le nombre d'octets lus (≤ `BUFFER_SIZE`), `0` à la fin de
fichier (entrée fermée, `Ctrl-d`) ou -1 en cas d'erreur. On réécrit
exactement `len_read` octets (et non `BUFFER_SIZE`).

```sh
./read                          # écho ligne par ligne (le terminal envoie les lignes)
./read < input_file             # équivalent de cat
./read < input_file > output_file && cmp input_file output_file   # copie identique
./read < .                      # read.c:14: read: Is a directory   (EISDIR)
```

## 7. Lecture d'un entier — `read_number.c`

```sh
$ ./write_number | ./read_number
valeur lue : 5
octets écrits par le premier write : 8
```

On lit `sizeof(long)` octets directement dans l'adresse d'un `long`. On boucle
car sur un tube `read` peut renvoyer moins d'octets que demandé.

## 8. Lecture et écriture d'une structure

```sh
$ ./write_struct | od -c
0000000   a  \0  \0  \0  \0  \0  \0  \0 005  \0  \0  \0  \0  \0  \0  \0
0000020   b  \0  \0  \0  \0  \0  \0  \0
$ ./write_struct | ./read_struct
c1 = 'a', l = 5, c2 = 'b'
```

- **Taille de la structure : 24 octets** (et non 1 + 8 + 1 = 10). Le
  compilateur respecte l'**alignement** : un `long` doit être à une adresse
  multiple de 8, donc 7 octets de *padding* sont insérés après `c1` ; et la
  taille totale doit être un multiple de l'alignement de la structure (8) pour
  que les éléments d'un tableau restent alignés, d'où 7 octets de padding
  après `c2`. Disposition : `c1` (0) | pad (1–7) | `l` (8–15) | `c2` (16) | pad (17–23).
  Remarque : sans `memset`, les octets de padding contiennent des restes de la
  pile (fuite d'information possible quand on écrit la structure brute).
- **Avec `make CFLAGS+=-O3` la taille ne change pas** : la disposition mémoire
  des structures est fixée par l'ABI de la plateforme (System V x86_64), pas
  par le niveau d'optimisation ; sinon des fichiers compilés avec des options
  différentes ne pourraient plus échanger de structures.
- **Peut-on réduire la taille ? Oui :**
  1. **Réordonner les champs** du plus grand au plus petit :
     `struct { long l; char c1; char c2; }` → **16 octets** (8 + 1 + 1 + 6 de
     padding final), tout en restant aligné et portable.
  2. **Compacter** avec `__attribute__((packed))` (GCC/Clang) ou
     `#pragma pack(1)` → **10 octets**, mais le `long` n'est plus aligné :
     accès plus lents, voire impossibles (bus error) sur certaines
     architectures, et non portable.

  (Voir `struct reordered` et `struct packed` dans `utils.h` ; `write_struct`
  affiche les trois tailles sur `stderr`.)
