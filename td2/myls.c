/* TD2 - Exercice 4 (+ "pour aller plus loin") : mini ls
 * Usage : ./myls [-l] [-R] [rep ...]     (par défaut : .)
 *   -l : affiche type, droits, nombre de liens, propriétaire, groupe, taille, date (ctime)
 *   -R : affichage récursif
 */
#include <dirent.h>
#include <getopt.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "utils.h"

static int opt_long = 0;
static int opt_rec  = 0;

static char type_char(mode_t m)
{
    if (S_ISREG(m))  return '-';
    if (S_ISDIR(m))  return 'd';
    if (S_ISLNK(m))  return 'l';
    if (S_ISCHR(m))  return 'c';
    if (S_ISBLK(m))  return 'b';
    if (S_ISFIFO(m)) return 'p';
    if (S_ISSOCK(m)) return 's';
    return '?';
}

static void droits(mode_t m, char out[10])
{
    const char *rwx = "rwxrwxrwx";
    for (int i = 0; i < 9; i++)
        out[i] = (m & (1 << (8 - i))) ? rwx[i] : '-';
    out[9] = '\0';
}

static void afficher_entree(const char *chemin, const char *nom)
{
    struct stat st;

    if (!opt_long) {
        printf("%s\n", nom);
        return;
    }
    /* lstat : on veut le type du lien lui-même, pas de sa cible */
    if (lstat(chemin, &st) == -1) {
        perror(chemin);
        return;
    }
    char perms[10];
    droits(st.st_mode, perms);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    char date[64];
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M", localtime(&st.st_ctime));

    printf("%c%s %3lu %-8s %-8s %8lld %s %s",
           type_char(st.st_mode), perms, (unsigned long)st.st_nlink,
           pw ? pw->pw_name : "?", gr ? gr->gr_name : "?",
           (long long)st.st_size, date, nom);

    if (S_ISLNK(st.st_mode)) {
        char cible[PATH_MAX];
        ssize_t n = readlink(chemin, cible, sizeof(cible) - 1);
        if (n != -1) {
            cible[n] = '\0';
            printf(" -> %s", cible);
        }
    }
    printf("\n");
}

static void lister(const char *rep)
{
    DIR *d = opendir(rep);
    if (d == NULL) {
        perror(rep);
        return;
    }
    printf("%s:\n", rep);

    struct dirent *e;
    char chemin[PATH_MAX];
    while ((e = readdir(d)) != NULL) {
        snprintf(chemin, sizeof(chemin), "%s/%s", rep, e->d_name);
        afficher_entree(chemin, e->d_name);
    }
    printf("\n");

    if (opt_rec) {
        rewinddir(d);
        while ((e = readdir(d)) != NULL) {
            if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
                continue;
            struct stat st;
            snprintf(chemin, sizeof(chemin), "%s/%s", rep, e->d_name);
            /* lstat pour ne pas suivre les liens symboliques (boucles infinies) */
            if (lstat(chemin, &st) == 0 && S_ISDIR(st.st_mode))
                lister(chemin);
        }
    }
    exit_if(closedir(d) == -1, "closedir");
}

int main(int argc, char *argv[])
{
    int c;
    while ((c = getopt(argc, argv, "lR")) != -1) {
        switch (c) {
        case 'l': opt_long = 1; break;
        case 'R': opt_rec  = 1; break;
        default:
            fprintf(stderr, "Usage : %s [-l] [-R] [rep ...]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (optind == argc)
        lister(".");
    for (int i = optind; i < argc; i++)
        lister(argv[i]);
    return EXIT_SUCCESS;
}
