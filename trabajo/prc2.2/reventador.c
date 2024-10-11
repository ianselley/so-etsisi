#define _MINIX

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include "generador.h"
#include "probar-clave.h"

#define TAM_LIN 255

#define CLAVE_ENCONTRADA 0
#define CLAVE_NO_ENCONTRADA 1
#define TRABAJADOR_VACIO 2
#define HERMANO_ENCONTRO_CLAVE 3

static int probar_claves_intervalo(int inicio, int fin, const char *salt_hash, const char *salt)
{
    int i = inicio;
    int pid = getpid();

    char msg[TAM_LIN];
    int status_clave = CLAVE_NO_ENCONTRADA;
    char *const clave = calloc(longitud() + 2, sizeof(char));

    while (i <= fin && status_clave != CLAVE_ENCONTRADA)
    {
        if (genera_palabra(i, clave) != OK)
        {
            return !OK;
        }
        status_clave = probar_clave(clave, salt_hash, salt);
        if (status_clave == CLAVE_ENCONTRADA)
        {
            snprintf(msg, TAM_LIN, "[%d] Clave encontrada: %s\n", pid, clave);
            write(STDOUT_FILENO, msg, strlen(msg));
        }
        i++;
    }

    if (status_clave == CLAVE_NO_ENCONTRADA)
    {
        snprintf(msg, TAM_LIN, "[%d] No se ha encontrado la clave\n", pid);
        write(STDOUT_FILENO, msg, strlen(msg));
    }

    free(clave);
    return status_clave;
}

static void trabajador(int fin_int_prev, int fin_int, const char *salt_hash)
{
    int pid = getpid();
    char salt[3];
    char msg[TAM_LIN];
    int status_clave;
    salt[0] = salt_hash[0];
    salt[1] = salt_hash[1];
    salt[2] = '\0';

    snprintf(msg, TAM_LIN, "pid: %d, salt_hash: %s, salt: %s\n", pid, salt_hash, salt);
    write(STDOUT_FILENO, msg, strlen(msg));
    status_clave = probar_claves_intervalo(fin_int_prev, fin_int, salt_hash, salt);
    exit(status_clave);
}

static void manejador(int sig)
{
    if (sig == SIGTERM)
    {
        exit(HERMANO_ENCONTRO_CLAVE);
    }
}

static void quitar_hijo(const int pid, int hijos_activos[], int *ultimo_hijo)
{
    int i;
    int encontrado = 0;
    for (i = 0; i <= *ultimo_hijo; i++)
    {
        if (hijos_activos[i] == pid)
        {
            encontrado = 1;
        }
        if (encontrado && i < *ultimo_hijo)
        {
            hijos_activos[i] = hijos_activos[i + 1];
        }
    }
    if (encontrado)
    {
        *ultimo_hijo = *ultimo_hijo - 1;
    }
}

static void avisar_hijos(const int hijos_activos[], const int ultimo_hijo)
{
    int i;
    for (i = 0; i <= ultimo_hijo; i++)
    {
        kill(hijos_activos[i], SIGTERM);
    }
}

static int crear_trabajadores(const int trabajadores, const char *salt_hash)
{
    int i;
    char msg[TAM_LIN];
    int pid;
    int palabras_x_trab;   /* palabras por trabajador */
    int fin_int;           /* final del intervalo */
    int fin_int_prev = -1; /* final del intervalo previo */
    int trabajador_vacio = 0;
    int status_hijo;
    int clave_encontrada = 0;
    int palabras_totales = total_palabras();
    int *hijos_activos = calloc(trabajadores, sizeof(int));
    int *ultimo_hijo_activo;

    struct sigaction sa;
    sa.sa_handler = manejador;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, NULL);

    palabras_x_trab = (palabras_totales / trabajadores) + 1;

    *ultimo_hijo_activo = -1;
    for (i = 0; i < trabajadores; i++)
    {
        fin_int = fin_int_prev + palabras_x_trab;
        fin_int_prev++;
        if (fin_int >= palabras_totales)
        {
            fin_int = palabras_totales - 1;
        }
        if (fin_int_prev >= palabras_totales)
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
            if (trabajador_vacio)
            {
                exit(TRABAJADOR_VACIO);
            }
            trabajador(fin_int_prev, fin_int, salt_hash);
        }
        else
        {
            *ultimo_hijo_activo = *ultimo_hijo_activo + 1;
            hijos_activos[*ultimo_hijo_activo] = pid;
        }
        fin_int_prev = fin_int;
    }
    for (i = 0; i < trabajadores; i++)
    {
        pid = wait(&status_hijo);
        status_hijo = WEXITSTATUS(status_hijo);
        quitar_hijo(pid, hijos_activos, ultimo_hijo_activo);
        if (status_hijo == CLAVE_ENCONTRADA)
        {
            clave_encontrada = 1;
            avisar_hijos(hijos_activos, *ultimo_hijo_activo);
            snprintf(msg, TAM_LIN, "El hijo con pid: %d encontró la clave\n", pid);
        }
        else if (status_hijo == CLAVE_NO_ENCONTRADA)
        {
            snprintf(msg, TAM_LIN, "El hijo con pid: %d no encontró la clave\n", pid);
        }
        else if (status_hijo == HERMANO_ENCONTRO_CLAVE)
        {
            snprintf(msg, TAM_LIN, "El hijo con pid: %d terminó porque un hermano encontró la clave\n", pid);
        }
        write(STDOUT_FILENO, msg, strlen(msg));
    }

    if (clave_encontrada)
    {
        snprintf(msg, TAM_LIN, "\nControlador: clave encontrada\n");
    }
    else
    {
        snprintf(msg, TAM_LIN, "\nControlador: no se ha encontrado la clave\n");
    }
    write(STDOUT_FILENO, msg, strlen(msg));
    free(hijos_activos);
    return OK;
}

static int trabajar(const char alfabeto[], int longitud, int trab, const char *salt_hash)
{
    if (configura_generador(alfabeto, longitud) != OK)
    {
        return !OK;
    }
    return crear_trabajadores(trab, salt_hash);
}

static void uso(const char prog[])
{
    printf("Uso: %s <alfabeto> <longitud> <num trabajadores> <salt_hash>\n", prog);
}

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

    printf("\n\n");

    if (argc != 5)
    {
        uso(argv[0]);
        return !OK;
    }
    if (str2int(argv[2], &longitud) != OK)
    {
        printf("la longitud no es válida\n");
        return !OK;
    }
    if (str2int(argv[3], &trabajadores) != OK)
    {
        printf("el número de trabajadores no es válido\n");
        return !OK;
    }
    if (trabajar(argv[1], longitud, trabajadores, argv[4]) != OK)
    {
        printf("el programa ha fallado\n");
        return !OK;
    }
    return OK;
}
