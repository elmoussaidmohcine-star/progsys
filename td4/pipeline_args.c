/* TD4 - Exercice 5 : pipeline fourni sur la ligne de commande
 *   ./pipeline_args ls -l -- grep toto -- less   <=>   ls -l | grep toto | less
 */
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "utils.h"

#define SEPARATEUR "--"

/*
 * Découpe argv (terminé par NULL) en commandes séparées par "--".
 * Les "--" sont remplacés par NULL dans argv, ce qui termine chaque commande.
 * Renvoie un tableau (alloué) de pointeurs vers le début de chaque commande,
 * et le nombre de commandes dans *nb.
 */
static char ***split_args(char *argv[], int *nb)
{
    int argc = 0;
    while (argv[argc] != NULL)
        argc++;

    char ***cmds = malloc((argc + 1) * sizeof(*cmds));
    exit_if(cmds == NULL, "malloc");

    int n = 0;
    char **debut = argv;
    for (int i = 0; i <= argc; i++) {
        if (argv[i] == NULL || strcmp(argv[i], SEPARATEUR) == 0) {
            argv[i] = NULL;
            if (debut[0] == NULL) {
                fprintf(stderr, "commande vide dans le pipeline\n");
                exit(EXIT_FAILURE);
            }
            cmds[n++] = debut;
            debut = &argv[i + 1];
        }
    }
    *nb = n;
    return cmds;
}

static pid_t lancer_pipeline(char **cmds[], int n)
{
    int entree = STDIN_FILENO;
    pid_t pid = -1;

    for (int i = 0; i < n; i++) {
        int tube[2] = { -1, -1 };
        int dernier = (i == n - 1);

        if (!dernier)
            exit_if(pipe(tube) == -1, "pipe");

        pid = fork();
        exit_if(pid == -1, "fork");

        if (pid == 0) {
            if (entree != STDIN_FILENO) {
                exit_if(dup2(entree, STDIN_FILENO) == -1, "dup2");
                exit_if(close(entree) == -1, "close");
            }
            if (!dernier) {
                exit_if(close(tube[0]) == -1, "close");
                exit_if(dup2(tube[1], STDOUT_FILENO) == -1, "dup2");
                exit_if(close(tube[1]) == -1, "close");
            }
            execvp(cmds[i][0], cmds[i]);
            fprintf(stderr, "%s: ", cmds[i][0]);
            perror("execvp");
            _exit(127);
        }

        if (entree != STDIN_FILENO)
            exit_if(close(entree) == -1, "close");
        if (!dernier) {
            exit_if(close(tube[1]) == -1, "close");
            entree = tube[0];
        }
    }
    return pid;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage : %s cmd1 [args] [-- cmd2 [args] ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int nb;
    char ***cmds = split_args(argv + 1, &nb);
    const char *nom = cmds[nb - 1][0];

    pid_t dernier = lancer_pipeline(cmds, nb);
    free(cmds);

    int status;
    exit_if(waitpid(dernier, &status, 0) == -1, "waitpid");
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        fprintf(stderr, "%s tué par le signal %d (%s)\n",
                nom, WTERMSIG(status), strsignal(WTERMSIG(status)));
    return EXIT_FAILURE;
}
