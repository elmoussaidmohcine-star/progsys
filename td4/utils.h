#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>

/*
 * exit_if(cond, prefix) : si la condition est vraie, affiche sur la sortie
 * d'erreur le fichier et la ligne, puis le message correspondant à errno
 * (via perror(3), préfixé par `prefix`), et termine le processus avec
 * EXIT_FAILURE.
 */
#define exit_if(cond, prefix)                                    \
    do {                                                         \
        if (cond) {                                              \
            fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);      \
            perror(prefix);                                      \
            exit(EXIT_FAILURE);                                  \
        }                                                        \
    } while (0)

#endif /* UTILS_H */
