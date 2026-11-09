/* TD1 - Exercice 4 : écriture binaire d'un long avec write(2) */
#include <unistd.h>
#include "utils.h"

int main(void)
{
    long n = 5;
    ssize_t rc, rc2;

    /* On écrit le contenu mémoire de n (sizeof(long) octets), pas sa représentation texte */
    rc = write(STDOUT_FILENO, &n, sizeof(n));
    exit_if(rc == -1, "write");

    /* Puis le nombre d'octets écrits par le premier write */
    rc2 = write(STDOUT_FILENO, &rc, sizeof(rc));
    exit_if(rc2 == -1, "write");

    return EXIT_SUCCESS;
}
