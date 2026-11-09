/* TD2 - Exercice 3 : deuxième open du même fichier */
#include <fcntl.h>
#include "transfer.h"

#define FICHIER "donnees.txt"
#define N 4

static void lire_afficher(int fd)
{
    char buf[N];
    ssize_t n = read(fd, buf, N);
    exit_if(n == -1, "read");
    write_all(STDOUT_FILENO, buf, n);
    write_all(STDOUT_FILENO, "\n", 1);
}

int main(void)
{
    int fd = open(FICHIER, O_RDONLY);
    exit_if(fd == -1, "open");
    lire_afficher(fd);                  /* abcd */

    int fd2 = open(FICHIER, O_RDONLY);  /* nouvelle entrée dans la table des fichiers ouverts */
    exit_if(fd2 == -1, "open");
    lire_afficher(fd2);                 /* abcd : offset propre, repart de 0 */

    exit_if(close(fd2) == -1, "close");
    exit_if(close(fd) == -1, "close");
    return EXIT_SUCCESS;
}
