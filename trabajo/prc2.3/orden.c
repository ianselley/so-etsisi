#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "orden.h"

#define MAX_LIN 1024

char salir[] = "exit";
static struct orden actual;
static char orden[MAX_LIN];
static int i;
static int num_param;
static int longitud;

static void borrar_orden_actual(void)
{
	i = 0;
	num_param = 0;
	longitud = strlen(orden);
	actual.entrada = NULL;
	actual.salida = NULL;
}

static void saltar_blancos(void)
{
	while (orden[i] == ' ' || orden[i] == '\t')
	{
		i++;
	}
}

static void final_palabra(void)
{
	while (orden[i] != ' ' && orden[i] != '\t' && orden[i] != '\0')
	{
		i++;
	}
}

static char *obtener_cadena(void)
{
	char *pal = &orden[i];
	final_palabra();
	orden[i] = '\0';
	i++;
	return pal;
}

static void extraer_param(void)
{
	actual.args[num_param++] = obtener_cadena();
}

static void redirigir_entrada(void)
{
	i++;
	saltar_blancos();
	actual.entrada = obtener_cadena();
	if (strlen(actual.entrada) == 0)
	{
		fprintf(stderr, "Falta fichero de entrada\n");
		actual.args[0] = NULL;
	}
}

static void redirigir_salida(void)
{
	i++;
	saltar_blancos();
	actual.salida = obtener_cadena();
	if (strlen(actual.salida) == 0)
	{
		fprintf(stderr, "Falta fichero de salida\n");
		actual.args[0] = NULL;
	}
}

static void extraer_palabra(void)
{
	saltar_blancos();
	switch (orden[i])
	{
	case '<':
		redirigir_entrada();
		break;
	case '>':
		redirigir_salida();
		break;
	case '\0':
		break;
	default:
		extraer_param();
	}
}

static void analizar_orden(void)
{
	do
	{
		extraer_palabra();
	} while (i < longitud && num_param < MAX_ARGS - 2);
	if (num_param >= MAX_ARGS - 2)
	{
		fprintf(stderr, "Demasiados argumentos\n");
		actual.args[0] = NULL;
	}
	actual.args[num_param] = NULL;
}

struct orden *leer_orden(const char *indicador)
{
	struct termios oldt, newt;
	int ch;
	int pos = 0;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;

	newt.c_lflag &= ~(ICANON | ECHO);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	do
	{
		fprintf(stdout, "%s ", indicador);
		fflush(stdout);

		pos = 0;
		while (1)
		{
			ch = getchar();

			if (ch == '\n')
			{
				orden[pos] = '\0';
				fprintf(stdout, "\n");
				break;
			}
			else if (ch == 127 || ch == '\b')
			{
				if (pos > 0)
				{
					pos--;
					fprintf(stdout, "\b \b");
					fflush(stdout);
				}
			}
			else if (ch >= 32 && ch <= 126)
			{
				orden[pos++] = ch;
				fputc(ch, stdout);
				fflush(stdout);
			}
		}

		if (feof(stdin))
		{
			actual.args[0] = salir;
			actual.args[1] = NULL;
			actual.entrada = NULL;
			actual.salida = NULL;
		}
		else
		{
			borrar_orden_actual();
			analizar_orden();
		}
	} while (actual.args[0] == NULL);

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return &actual;
}