/* TD2 - Pour aller plus loin : chat à deux (côté client)
 * Deux FIFOs : versserveur (client -> serveur) et versclient (serveur -> client).
 * poll(2) permet d'attendre à la fois sur le clavier et sur la FIFO
 * (sinon un read bloquant sur l'un empêcherait de traiter l'autre).
 */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>
#include "transfer.h"

#define VERS_SERVEUR "versserveur"
#define VERS_CLIENT  "versclient"

/* Recopie un bloc de in vers out ; renvoie 0 à EOF */
static int relayer(int in, int out)
{
    char buf[BUFFER_SIZE];
    ssize_t n = read(in, buf, sizeof(buf));
    exit_if(n == -1, "read");
    if (n == 0)
        return 0;
    write_all(out, buf, n);
    return 1;
}

int main(void)
{
    
    

    /* Ordre inverse de celui du serveur, sinon interblocage :
     * open d'une FIFO bloque jusqu'à ce que l'autre extrémité soit ouverte. */
    int out = open(VERS_SERVEUR, O_WRONLY);
    exit_if(out == -1, "open");
    int in  = open(VERS_CLIENT, O_RDONLY);
    exit_if(in == -1, "open");

    struct pollfd fds[2] = {
        { .fd = STDIN_FILENO, .events = POLLIN },
        { .fd = in,           .events = POLLIN },
    };
    while (1) {
        exit_if(poll(fds, 2, -1) == -1, "poll");
        if (fds[0].revents & (POLLIN | POLLHUP))
            if (!relayer(STDIN_FILENO, out)) break;
        if (fds[1].revents & (POLLIN | POLLHUP))
            if (!relayer(in, STDOUT_FILENO)) break;
    }
    close(in);
    close(out);
    return EXIT_SUCCESS;
}
