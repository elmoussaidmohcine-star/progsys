/* TD3 - Exercice 4.5 : génère une erreur de segmentation (SIGSEGV)
 * en écrivant à l'adresse 0 (pointeur NULL), qui n'est pas mappée.
 */
#include <stdio.h>

void gen_segfault(void)
{
    *((volatile int *)0) = 42;
}

int main(void)
{
    printf("Je vais faire une faute de segmentation...\n");
    gen_segfault();
    return 0;
}
