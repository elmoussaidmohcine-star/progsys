/* TD4 - Exercices 1 et 2 : tube non nommé entre un père et son fils
 * Usage : ./mypipe [nb_messages]   (défaut : 10)
 *
 * Version corrigée : chaque processus ferme l'extrémité du tube qu'il
 * n'utilise pas (cf. REPONSES.md pour l'explication du blocage).
 */
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "utils.h"

#define MESSAGE      "Hello World"
#define NB_DEFAUT    10
#define TAILLE_LECT  500

int main(int argc, char *argv[])
{
    int nb = argc > 1 ? atoi(argv[1]) : NB_DEFAUT;
    int tube[2];

    exit_if(pipe(tube) == -1, "pipe");

    pid_t pid = fork();
    exit_if(pid == -1, "fork");

    if (pid == 0) {
        /* ---- fils : lecteur ---- */
        char buf[TAILLE_LECT];
        exit_if(close(tube[1]) == -1, "close");

        ssize_t n = read(tube[0], buf, TAILLE_LECT);   /* une seule lecture */
        exit_if(n == -1, "read");
        exit_if(write(STDOUT_FILENO, buf, n) == -1, "write");

        fprintf(stderr, "\nJe suis le fils, j'ai lu %zd octets et je quitte\n", n);
        exit_if(close(tube[0]) == -1, "close");
        return EXIT_SUCCESS;
    }

    /* ---- père : écrivain ---- */
    exit_if(close(tube[0]) == -1, "close");   /* indispensable, cf. exercice 2 */

    size_t total = 0;
    for (int i = 0; i < nb; i++) {
        ssize_t rc = write(tube[1], MESSAGE, strlen(MESSAGE));
        exit_if(rc == -1, "write");
        total += rc;
    }
    fprintf(stderr, "Je suis le père, j'ai écrit %zu octets et je quitte\n", total);
    exit_if(close(tube[1]) == -1, "close");
    return EXIT_SUCCESS;
}
