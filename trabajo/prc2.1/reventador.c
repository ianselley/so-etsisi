#define _MINIX

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "generador.h"

#define BUFFER_SIZE 60

/**
 * Muestra todas las palabras que puede genera el generador.
 * Es importante, para los pasos siguientes, entender cómo se
 * reserva memoria para recoger la palabra y cómo se libera,
 * porque será necesario aplicarlo en el paso 5 de la práctica.
 */
static int escribir_palabras(void)
{
    int i;
    char *const palabra = calloc(longitud() + 2, sizeof(char));

    for (i = 0; i < total_palabras(); i++)
    {
        if (genera_palabra(i, palabra) != OK)
        {
            return !OK;
        }
        fprintf(stdout, "%s\n", palabra);
    }
    free(palabra);
    return OK;
}

static int escribir_intervalos(const int trabajadores)
{
    int i;
    int palabras_x_trab;   /* palabras por trabajador */
    int fin_int;           /* final del intervalo*/
    int fin_int_prev = -1; /* final del intervalo previo*/
    int trabajador_vacio = 0;
    palabras_x_trab = (total_palabras() / trabajadores) + 1;

    for (i = 0; i < trabajadores; i++)
    {
        fin_int = fin_int_prev + palabras_x_trab;
        fin_int_prev++;
        if (fin_int >= total_palabras())
        {
            fin_int = total_palabras() - 1;
        }
        if (fin_int_prev >= total_palabras())
        {
            trabajador_vacio = 1;
        }
        if (!trabajador_vacio)
        {
            printf("Trabajador %d: %d - %d\n", i, fin_int_prev, fin_int);
        }
        else
        {
            printf("Trabajador %d: No tiene intervalo\n", i);
        }
        fin_int_prev = fin_int;
    }
    return 0;
}

static void escribir_en_buffer(char *bfr, int pid, int inicio, int fin)
{
    snprintf(bfr, BUFFER_SIZE,
             "Trabajador hijo con pid %d: %d - %d\n", pid, inicio, fin);
}

static void escribir_en_buffer_sin_intervalo(char *bfr, int pid)
{
    snprintf(bfr, BUFFER_SIZE,
             "Trabajador hijo con pid %d: No tiene intervalo\n", pid);
}

static int crear_trabajadores(const int trabajadores)
{
    int i;
    int pid;
    int pid_hijo;
    int palabras_x_trab;   /* palabras por trabajador */
    int fin_int;           /* final del intervalo */
    int fin_int_prev = -1; /* final del intervalo previo */
    int trabajador_vacio = 0;
    char buffer[BUFFER_SIZE];
    int *status_hijo;
    int resultado_snprintf;
    palabras_x_trab = (total_palabras() / trabajadores) + 1;

    for (i = 0; i < trabajadores; i++)
    {
        fin_int = fin_int_prev + palabras_x_trab;
        fin_int_prev++;
        if (fin_int >= total_palabras())
        {
            fin_int = total_palabras() - 1;
        }
        if (fin_int_prev >= total_palabras())
        {
            trabajador_vacio = 1;
        }
        pid = fork();
        if (pid == -1)
        {
            perror("fork");
        }
        else if (pid == 0)
        {
            pid_hijo = getpid();
            if (!trabajador_vacio)
            {
                escribir_palabras_intervalo(fin_int_prev, fin_int);
            }
            write(STDOUT_FILENO, buffer, strlen(buffer));
            return 0;
        }
        else
        {
            wait(status_hijo);
        }
        fin_int_prev = fin_int;
    }
    return 0;
}

static int escribir_palabras_intervalo(int inicio, int fin)
{
    int i;
    char *const palabra = calloc(longitud() + 2, sizeof(char));

    for (i = inicio; i <= fin; i++)
    {
        if (genera_palabra(i, palabra) != OK)
        {
            return !OK;
        }
        fprintf(stdout, "%s\n", palabra);
    }
    free(palabra);
    return OK;
}

/* Realiza el trabajo del reventador una vez que se ha obtenido la
 * información necesaria de los argumentos del programa.
 * Si todo va bien, devuelve OK. En caso contrario devuelve !OK.
 */
static int trabajar(const char alfabeto[], const int longitud, const int trab)
{
    if (configura_generador(alfabeto, longitud) != OK)
    {
        return !OK;
    }
    return crear_trabajadores(trab);
}

/**
 * Muestra un mensaje indicando cómo debe usarse el programa. Si se
 * modifican los argumentos del programa, hay que modificar esta función.
 */
static void uso(const char prog[])
{
    fprintf(stderr, "Uso: %s <alfabeto> <longitud> <num trabajadores>\n", prog);
}

/**
 * Dada una cadena de caracteres, obtiene su representación numérica entera.
 * Tiene los siguientes parámetros:
 *	cadena: la versión textual del número.
 *	num: referencia al entero que albergará el valor numérico de la cadena.
 * Si cadena contiene una cadena numérica válida, devuelve OK.
 * En caso contrario, devuelve !OK y el valor referenciado por num es
 * indeterminado.
 */
static int str2int(const char cadena[], long *num)
{
    char *endptr;

    errno = OK;
    *num = strtol(cadena, &endptr, 10);
    if (errno != OK || *endptr != '\0' || *num < 1)
    {
        return !OK;
    }
    return OK;
}

int main(const int argc, const char *argv[])
{
    long longitud;
    long trabajadores;

    if (argc != 4)
    {
        uso(argv[0]);
        return !OK;
    }
    if (str2int(argv[2], &longitud) != OK)
    {
        fprintf(stderr, "la longitud no es válida\n");
        return !OK;
    }
    if (str2int(argv[3], &trabajadores) != OK)
    {
        fprintf(stderr, "el número de trabajadores no es válido\n");
        return !OK;
    }
    if (trabajar(argv[1], longitud, trabajadores) != OK)
    {
        fprintf(stderr, "el programa ha fallado\n");
        return !OK;
    }
    return OK;
}
