/* TD2 - Exercice 2 : client du chat, écrit l'entrée standard dans la FIFO
 * Usage : ./client [fifo]     (par défaut : versserveur)
 */
#include <fcntl.h>
#include "transfer.h"

#define FIFO_DEFAUT "versserveur"

int main(int argc, char *argv[])
{
    const char *fifo = argc > 1 ? argv[1] : FIFO_DEFAUT;
    int fd = open(fifo, O_WRONLY);   /* bloque tant qu'aucun lecteur n'a ouvert la FIFO */
    exit_if(fd == -1, "open");

    transfer(STDIN_FILENO, fd);

    exit_if(close(fd) == -1, "close");
    return EXIT_SUCCESS;
}
