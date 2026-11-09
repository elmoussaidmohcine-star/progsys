/* TD4 - Exercice 3 : le père exécute ps avec sa sortie standard dans le tube,
 * le fils lit le tube en continu et affiche tout en majuscules.
 */
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "utils.h"

#define BUFFER_SIZE 256

int main(void)
{
    int tube[2];
    exit_if(pipe(tube) == -1, "pipe");

    pid_t pid = fork();
    exit_if(pid == -1, "fork");

    if (pid == 0) {
        /* ---- fils ---- */
        char buf[BUFFER_SIZE];
        ssize_t n;
        exit_if(close(tube[1]) == -1, "close");   /* sinon jamais d'EOF */

        while ((n = read(tube[0], buf, BUFFER_SIZE)) > 0) {
            for (ssize_t i = 0; i < n; i++)
                buf[i] = toupper((unsigned char)buf[i]);
            for (ssize_t done = 0; done < n; ) {
                ssize_t rc = write(STDOUT_FILENO, buf + done, n - done);
                exit_if(rc == -1, "write");
                done += rc;
            }
        }
        exit_if(n == -1, "read");
        return EXIT_SUCCESS;
    }

    /* ---- père : sa sortie standard devient l'entrée du tube ---- */
    exit_if(close(tube[0]) == -1, "close");
    exit_if(dup2(tube[1], STDOUT_FILENO) == -1, "dup2");
    exit_if(close(tube[1]) == -1, "close");

    execlp("ps", "ps", NULL);
    exit_if(1, "execlp");
}
