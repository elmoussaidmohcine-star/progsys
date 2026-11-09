/* TD4 - Exercice 4 : exécution d'un pipeline de commandes codé en dur
 *   pipeline        : ls -l | tr -d [:blank:]
 *   pipeline_simple : ls -l | grep .c | wc -l     (-DSIMPLER_VERSION)
 */
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "utils.h"

#if !defined(SIMPLER_VERSION)
char  *ls[] = { "ls", "-l", NULL};
char  *tr[] = { "tr", "-d", "[:blank:]", NULL};
char **commandes[] = { ls, tr };
#else
char  *ls[]   = { "ls",   "-l", NULL};
char  *grep[] = { "grep", ".c", NULL};
char  *wc[]   = { "wc",   "-l", NULL};
char **commandes[] = { ls, grep, wc };
#endif
int nb_commandes = sizeof(commandes) / sizeof(char*);

/*
 * Lance cmds[0] | cmds[1] | ... | cmds[n-1].
 * Renvoie le pid du dernier processus du pipeline.
 */
static pid_t lancer_pipeline(char **cmds[], int n)
{
    int entree = STDIN_FILENO;   /* entrée de la commande courante */
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

        /* parent : on ferme tout ce qui ne lui sert plus, sinon les lecteurs
         * ne verraient jamais la fin de fichier */
        if (entree != STDIN_FILENO)
            exit_if(close(entree) == -1, "close");
        if (!dernier) {
            exit_if(close(tube[1]) == -1, "close");
            entree = tube[0];
        }
    }
    return pid;
}

static int attendre(pid_t dernier, const char *nom)
{
    int status;
    exit_if(waitpid(dernier, &status, 0) == -1, "waitpid");

    if (WIFEXITED(status))
        return WEXITSTATUS(status);   /* on propage le code de retour */

    if (WIFSIGNALED(status))
        fprintf(stderr, "%s tué par le signal %d (%s)\n",
                nom, WTERMSIG(status), strsignal(WTERMSIG(status)));
    else
        fprintf(stderr, "%s : terminaison anormale\n", nom);
    return EXIT_FAILURE;
}

int main(void)
{
    pid_t dernier = lancer_pipeline(commandes, nb_commandes);
    return attendre(dernier, commandes[nb_commandes - 1][0]);
}
