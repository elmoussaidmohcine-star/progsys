/* TD1 - Exercice 6 : recopie de l'entrée standard sur la sortie standard */
#include <unistd.h>
#include "utils.h"

#define BUFFER_SIZE 256

int main(void)
{
    char buffer[BUFFER_SIZE];
    ssize_t len_read, wrote, rc;

    while (1) {
        len_read = read(STDIN_FILENO, buffer, BUFFER_SIZE);
        exit_if(len_read == -1, "read");
        if (len_read == 0)          /* EOF (Ctrl-d) */
            break;

        wrote = 0;
        while (wrote < len_read) {  /* écritures partielles possibles */
            rc = write(STDOUT_FILENO, buffer + wrote, len_read - wrote);
            exit_if(rc == -1, "write");
            wrote += rc;
        }
    }
    return EXIT_SUCCESS;
}
