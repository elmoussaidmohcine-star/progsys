/* TD2 - Exercice 3 : version de base, lit et affiche les 4 premiers octets */
#include <fcntl.h>
#include "transfer.h"

#define FICHIER "donnees.txt"
#define N 4

int main(void)
{
    char buf[N];
    int fd = open(FICHIER, O_RDONLY);
    exit_if(fd == -1, "open");

    ssize_t n = read(fd, buf, N);
    exit_if(n == -1, "read");
    write_all(STDOUT_FILENO, buf, n);
    write_all(STDOUT_FILENO, "\n", 1);

    exit_if(close(fd) == -1, "close");
    return EXIT_SUCCESS;
}
