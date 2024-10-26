#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "orden.h"

#define OK 0

int ejecuta_pwd(struct orden *o)
{
	char *buf;
	if (o->args[1] == NULL && o->entrada == NULL && o->salida == NULL)
	{
		buf = getcwd(buf, 255);
		printf("%s\n", buf);
		return OK;
	}
	else
	{
		fprintf(stderr, "El comando pwd no admite parámetros, ni redirección de entrada o salida\n");
		return !OK;
	}
}

int ejecuta_cd(struct orden *o)
{
	int correcto = OK;
	if (o->args[2] == NULL && o->entrada == NULL && o->salida == NULL)
	{
		if (o->args[1] == NULL)
		{
			chdir(getenv("HOME"));
		}
		else
		{
			correcto = chdir(o->args[1]);
		}
		if (correcto != OK)
		{
			printf("Directorio inexistente\n");
			return !OK;
		}
		return OK;
	}
	else
	{
		fprintf(stderr, "El comando cd solo admite un parámetro sin redirección de entrada ni salida\n");
		return !OK;
	}
}

int ejecuta_exit(struct orden *o)
{
	if (!(o->args[1] == NULL && o->entrada == NULL && o->salida == NULL))
	{
		fprintf(stderr, "El comando exit no admite parámetros, ni redirección de entrada o salida\n");
		return !OK;
	}
	return OK;
}

int redirigir_entrada(const char *entrada)
{
	int fd;
	fd = open(entrada, O_RDONLY);
	if (fd == -1)
	{
		printf("Error abriendo el archivo de entrada\n");
		return !OK;
	}
	if (close(STDIN_FILENO) == -1)
	{
		printf("Error cerrando la entrada estándar\n");
		return !OK;
	}
	if (dup2(fd, STDIN_FILENO) == -1)
	{
		printf("Error duplicando el descriptor del fichero en la entrada estándar\n");
		return !OK;
	}
}

int redirigir_salida(const char *salida)
{
	int fd;
	fd = open(salida, O_CREAT | O_WRONLY, S_IRWXU);
	if (fd == -1)
	{
		printf("Error abriendo el archivo de salida\n");
		return !OK;
	}
	if (close(STDOUT_FILENO) == -1)
	{
		printf("Error cerrando la salida estándar\n");
		return !OK;
	}
	if (dup2(fd, STDOUT_FILENO) == -1)
	{
		printf("Error duplicando el descriptor del fichero en la salida estándar\n");
		return !OK;
	}
}

int ejecuta_externa(struct orden *o)
{
	int correcto;
	int status_hijo;
	int pid = fork();
	if (pid == -1)
	{
		fprintf(stderr, "Error al crear el proceso que ejecuta la orden externa");
	}
	else if (pid == 0)
	{
		if (o->entrada != NULL)
		{
			redirigir_entrada(o->entrada);
		}
		if (o->salida != NULL)
		{
			redirigir_salida(o->salida);
		}
		correcto = execvp(o->args[0], o->args);
		if (correcto != OK)
		{
			fprintf(stderr, "El comando no se reconoce\n");
			exit(!OK);
		}
		exit(OK);
	}
	else
	{
		wait(&status_hijo);
		return status_hijo;
	}
}

int mostrar_orden(struct orden *o)
{
	char *comando = o->args[0];

	if (strcmp(comando, "pwd") == OK)
	{
		return ejecuta_pwd(o);
	}
	if (strcmp(comando, "cd") == OK)
	{
		return ejecuta_cd(o);
	}
	if (strcmp(comando, "exit") == OK)
	{
		return ejecuta_exit(o);
	}
	return ejecuta_externa(o);
}

void limpiar_orden(struct orden *o)
{
	int i;
	for (i = 1; i < MAX_ARGS; i++)
	{
		o->args[i] = NULL;
	}
}

int main(const int argc, const char *argv[])
{
	struct orden *actual;
	int correcto;
	do
	{
		actual = leer_orden("bt0480>");
		correcto = mostrar_orden(actual);
		limpiar_orden(actual);
	} while (correcto != OK || strcmp("exit", actual->args[0]) != 0);
	return 0;
}
