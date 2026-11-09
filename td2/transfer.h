#ifndef TRANSFER_H
#define TRANSFER_H

#include <unistd.h>
#include "utils.h"

#define BUFFER_SIZE 256

/* Recopie tout ce qui est lu sur fd_in vers fd_out jusqu'à EOF */
static inline void transfer(int fd_in, int fd_out)
{
    char buffer[BUFFER_SIZE];
    ssize_t len_read, wrote, rc;

    while (1) {
        len_read = read(fd_in, buffer, BUFFER_SIZE);
        exit_if(len_read == -1, "read");
        if (len_read == 0)          /* EOF */
            return;

        wrote = 0;
        do {                        /* write peut écrire moins que demandé */
            rc = write(fd_out, buffer + wrote, len_read - wrote);
            exit_if(rc == -1, "write");
            wrote += rc;
        } while (wrote < len_read);
    }
}

/* Écrit exactement len octets */
static inline void write_all(int fd, const void *buf, size_t len)
{
    size_t done = 0;
    while (done < len) {
        ssize_t rc = write(fd, (const char *)buf + done, len - done);
        exit_if(rc == -1, "write");
        done += rc;
    }
}

#endif
