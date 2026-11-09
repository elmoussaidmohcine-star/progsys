/* TD3 - Exercice 2 : mise en évidence des buffers de la libc
 *
 * Macros de configuration (ex : make buffer CFLAGS+="-DUTILISER_FPRINTF1") :
 *   CHAINE1, CHAINE2           chaînes affichées
 *   UTILISER_FPRINTF1/2        fprintf(3) au lieu de write(2)
 *   UTILISER_SORTIE_ERREUR1/2  stderr au lieu de stdout
 *   UTILISER__EXIT             _exit(2) au lieu de exit(3)
 */
#include <string.h>
#include <unistd.h>
#include "utils.h"

#ifndef CHAINE1
#define CHAINE1 "Hello"
#endif
#ifndef CHAINE2
#define CHAINE2 " World!"
#endif

#ifdef UTILISER_SORTIE_ERREUR1
#define FLUX1 stderr
#define FD1   STDERR_FILENO
#else
#define FLUX1 stdout
#define FD1   STDOUT_FILENO
#endif

#ifdef UTILISER_SORTIE_ERREUR2
#define FLUX2 stderr
#define FD2   STDERR_FILENO
#else
#define FLUX2 stdout
#define FD2   STDOUT_FILENO
#endif

int main(void)
{
#ifdef UTILISER_FPRINTF1
    exit_if(fprintf(FLUX1, "%s", CHAINE1) < 0, "fprintf");
#else
    exit_if(write(FD1, CHAINE1, strlen(CHAINE1)) == -1, "write");
#endif

    sleep(1);

#ifdef UTILISER_FPRINTF2
    exit_if(fprintf(FLUX2, "%s", CHAINE2) < 0, "fprintf");
#else
    exit_if(write(FD2, CHAINE2, strlen(CHAINE2)) == -1, "write");
#endif

#ifdef UTILISER__EXIT
    _exit(EXIT_SUCCESS);   /* ne vide PAS les buffers de la libc */
#else
    exit(EXIT_SUCCESS);    /* appelle fflush sur tous les flux puis _exit */
#endif
}
