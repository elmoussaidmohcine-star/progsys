/* TD1 - Exercice 8 : lecture binaire d'une structure
 * Usage : ./write_struct | ./read_struct
 */
#include <unistd.h>
#include "utils.h"

int main(void)
{
    struct nopad s;
    size_t done = 0;

    while (done < sizeof(s)) {
        ssize_t rc = read(STDIN_FILENO, (char *)&s + done, sizeof(s) - done);
        exit_if(rc == -1, "read");
        if (rc == 0) {
            fprintf(stderr, "EOF inattendu (%zu/%zu octets)\n", done, sizeof(s));
            return EXIT_FAILURE;
        }
        done += rc;
    }
    printf("c1 = '%c', l = %ld, c2 = '%c'\n", s.c1, s.l, s.c2);
    return EXIT_SUCCESS;
}
