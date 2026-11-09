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

/* Structure partagée entre write_struct.c et read_struct.c */
struct nopad {
    char c1;
    long l;
    char c2;
};

/* Version réordonnée (taille minimale sans attribut packed) */
struct reordered {
    long l;
    char c1;
    char c2;
};

/* Version compactée par le compilateur (pas de padding, accès non alignés) */
struct packed {
    char c1;
    long l;
    char c2;
} __attribute__((packed));

#endif /* UTILS_H */
