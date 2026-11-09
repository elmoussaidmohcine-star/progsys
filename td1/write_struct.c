/* TD1 - Exercice 8 : écriture binaire d'une structure */
#include <string.h>
#include <unistd.h>
#include "utils.h"

int main(void)
{
    struct nopad s;
    ssize_t rc;

    /* memset : sinon les octets de padding contiennent des restes de la pile */
    memset(&s, 0, sizeof(s));
    s.c1 = 'a';
    s.l  = 5;
    s.c2 = 'b';

    fprintf(stderr, "sizeof(struct nopad)     = %zu\n", sizeof(struct nopad));
    fprintf(stderr, "sizeof(struct reordered) = %zu\n", sizeof(struct reordered));
    fprintf(stderr, "sizeof(struct packed)    = %zu\n", sizeof(struct packed));

    rc = write(STDOUT_FILENO, &s, sizeof(s));
    exit_if(rc == -1, "write");
    return EXIT_SUCCESS;
}
