/* TD2 - Exercice 1 : type de l'entrée standard avec fstat(2) */
#include <sys/stat.h>
#include <unistd.h>
#include "utils.h"

static const char *type_name(mode_t m)
{
    if (S_ISREG(m))  return "fichier régulier";
    if (S_ISDIR(m))  return "répertoire";
    if (S_ISCHR(m))  return "périphérique caractère (ex : terminal)";
    if (S_ISBLK(m))  return "périphérique bloc";
    if (S_ISFIFO(m)) return "FIFO / tube";
    if (S_ISLNK(m))  return "lien symbolique";
    if (S_ISSOCK(m)) return "socket";
    return "inconnu";
}

int main(void)
{
    struct stat st;
    exit_if(fstat(STDIN_FILENO, &st) == -1, "fstat");
    printf("entrée standard : %s\n", type_name(st.st_mode));
    return EXIT_SUCCESS;
}
