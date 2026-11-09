/* TD1 - Exercice 3 : echo avec write(2) */
#include <string.h>
#include <unistd.h>
#include "utils.h"

/* Écrit entièrement `len` octets de `buf` sur `fd` (gère les écritures partielles) */
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
    for (int i = 1; i < argc; i++) {
        write_all(STDOUT_FILENO, argv[i], strlen(argv[i]));
        if (i < argc - 1)
            write_all(STDOUT_FILENO, " ", 1);
    }
    write_all(STDOUT_FILENO, "\n", 1);
    return EXIT_SUCCESS;
}
