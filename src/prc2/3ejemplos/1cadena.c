/* 1cadena.c */
#include <stdio.h>
int main(int argc, char *argv[]) {
    char *cadena = "Hola";/* Array de 5 caracteres: 'H','o','l','a','\0' */
    printf("%s\n", cadena);  /* Imprime la cadena */

    char* p = cadena;      
    printf("%s\n", p);     

    char *q = cadena;
    printf("%s\n", q);      
    return 0;
}
