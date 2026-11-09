/* TD3 - Exercice 4 : ./lance sortie commande arg1 arg2 ...
 * équivalent à : commande arg1 arg2 ... > sortie
 */
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "utils.h"

static void write_all(int fd, const char *buf, size_t len)
{
    size_t done = 0;
    while (done < len) {
        ssize_t rc = write(fd, buf + done, len - done);
        exit_if(rc == -1, "write");
        done += rc;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage : %s sortie commande [args ...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    exit_if(fd == -1, "open");

    /* 1. on écrit la commande à exécuter dans sortie */
    for (int i = 2; i < argc; i++) {
        write_all(fd, argv[i], strlen(argv[i]));
        write_all(fd, i < argc - 1 ? " " : "\n", 1);
    }

    /* 2. un nouveau processus exécute la commande */
    pid_t pid = fork();
    exit_if(pid == -1, "fork");

    if (pid == 0) {
        /* 3. redirection de la sortie standard vers le fichier */
        exit_if(dup2(fd, STDOUT_FILENO) == -1, "dup2");
        exit_if(close(fd) == -1, "close");
        execvp(argv[2], argv + 2);
        /* on n'arrive ici que si execvp a échoué */
        perror("execvp");
        _exit(127);
    }

    exit_if(close(fd) == -1, "close");

    /* 4. statut de terminaison */
    int status;
    exit_if(waitpid(pid, &status, 0) == -1, "waitpid");

    if (WIFEXITED(status)) {
        printf("%s s'est terminé normalement, code de retour %d\n",
               argv[2], WEXITSTATUS(status));
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        printf("%s a été tué par le signal %d (%s)%s\n",
               argv[2], WTERMSIG(status), strsignal(WTERMSIG(status)),
               WCOREDUMP(status) ? ", core dumped" : "");
    }
    return EXIT_FAILURE;
}
