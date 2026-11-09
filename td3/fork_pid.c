/* TD3 - Exercice 3 : fork et buffer de printf
 *
 * Problème : printf("%d", getpid()) sans '\n' reste dans le buffer de stdout ;
 * fork() duplique la mémoire du processus, donc aussi ce buffer : il serait
 * affiché deux fois (une fois par le parent à sa terminaison, une fois par
 * l'enfant).
 *
 * Solutions possibles :
 *   1. fflush(stdout) juste avant fork()            <- solution retenue
 *   2. le parent termine avec _exit() (ne vide pas le buffer) : seul l'enfant
 *      l'affiche (mais l'enfant hérite alors du texte du parent...)
 *   3. désactiver le buffer : setvbuf(stdout, NULL, _IONBF, 0)
 *      (ou écrire avec write(2) / sur stderr, non bufferisé)
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "utils.h"

int main(void)
{
    printf("Parent : pid = %d. ", getpid());

    /* Solution 1 : on vide le buffer avant de dupliquer le processus */
    exit_if(fflush(stdout) == EOF, "fflush");

    pid_t pid = fork();
    exit_if(pid == -1, "fork");

    if (pid == 0) {
        printf("Enfant : pid = %d, pid du parent = %d\n", getpid(), getppid());
        exit(EXIT_SUCCESS);
    }

    /* Le parent n'affiche plus rien ; il attend l'enfant pour que getppid()
     * renvoie bien son pid (sinon l'enfant pourrait être adopté par init). */
    exit_if(wait(NULL) == -1, "wait");
    return EXIT_SUCCESS;
}
