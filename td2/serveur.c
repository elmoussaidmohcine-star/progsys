/* TD2 - Exercice 2 : serveur du chat, crée la FIFO et affiche ce qui y est lu
 * Usage : ./serveur [fifo]    (par défaut : versserveur)
 */
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "transfer.h"

#define FIFO_DEFAUT "versserveur"

int main(int argc, char *argv[])
{
    const char *fifo = argc > 1 ? argv[1] : FIFO_DEFAUT;

    /* 1. création de la FIFO (équivalent de mkfifo(3)) ; on tolère qu'elle existe déjà */
    exit_if(mknod(fifo, S_IFIFO | 0666, 0) == -1 && errno != EEXIST, "mknod");

    /* 2. ouverture en lecture (bloque jusqu'à l'arrivée d'un écrivain) */
    int fd = open(fifo, O_RDONLY);
    exit_if(fd == -1, "open");

    /* 3 et 4. lecture sur la FIFO, écriture sur la sortie standard
     * (read/write binaires : fonctionne aussi pour des fichiers quelconques) */
    transfer(fd, STDOUT_FILENO);

    exit_if(close(fd) == -1, "close");
    return EXIT_SUCCESS;
}
