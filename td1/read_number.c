/* TD1 - Exercice 7 : lecture de ce que produit write_number
 * Usage : ./write_number | ./read_number
 */
#include <unistd.h>
#include "utils.h"

/* Lit exactement `len` octets (read peut renvoyer moins sur un tube) */
static int read_all(int fd, void *buf, size_t len)
{
    size_t done = 0;
    while (done < len) {
        ssize_t rc = read(fd, (char *)buf + done, len - done);
        exit_if(rc == -1, "read");
        if (rc == 0)
            return 0;               /* EOF prématuré */
        done += rc;
    }
    return 1;
}

int main(void)
{
    long n;
    ssize_t nb;

    if (!read_all(STDIN_FILENO, &n, sizeof(n))) {
        fprintf(stderr, "EOF inattendu\n");
        return EXIT_FAILURE;
    }
    if (!read_all(STDIN_FILENO, &nb, sizeof(nb))) {
        fprintf(stderr, "EOF inattendu\n");
        return EXIT_FAILURE;
    }
    printf("valeur lue : %ld\n", n);
    printf("octets écrits par le premier write : %zd\n", nb);
    return EXIT_SUCCESS;
}
