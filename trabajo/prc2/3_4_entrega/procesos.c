#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "procesos.h"

/* estructura proceso con PID, ejecutable y estado */
struct proceso
{
   /**** PRÁCTICA ****/
   /* declara un campo entero de nombre zz0000_pid donde zz0000
      será tu matrícula */
   int bt0480_pid;

   /* declara un campo de tipo array de caracteres de longitud
      MAX_EJECUTABLE de nombre zz0000_ejecutable */
   char bt0480_ejecutable[MAX_EJECUTABLE];

   /* declara un campo entero de nombre zz0000_estado */
   int bt0480_estado;
};

int anadir_proceso(struct proceso *procesos[], int ix, int pid,
                   char *ejecutable, int estado)
{
   struct proceso *proceso_pointer;
   proceso_pointer = calloc(1, sizeof(struct proceso));
   if (proceso_pointer == NULL)
   {
      return MEM_ERROR;
   }

   proceso_pointer->bt0480_pid = pid;
   strncpy(proceso_pointer->bt0480_ejecutable, ejecutable, MAX_EJECUTABLE);
   proceso_pointer->bt0480_estado = estado;

   procesos[ix] = proceso_pointer;

   return 0;
}

void listar_procesos(struct proceso *procesos[])
{
   int i;
   struct proceso *proceso_i;
   for (i = 0; i < MAX_PROCESOS; i++)
   {
      proceso_i = procesos[i];
      if (proceso_i == NULL)
      {
         printf("Posición de tabla %d: Vacía\n", i);
      }
      else
      {
         printf("Posición de tabla %d: %d, %s, %d\n", i, proceso_i->bt0480_pid, proceso_i->bt0480_ejecutable, proceso_i->bt0480_estado);
      }
   }
}

void liberar_procesos(struct proceso *procesos[])
{
   int i;
   struct proceso *proceso_i;
   for (i = 0; i < MAX_PROCESOS; i++)
   {
      proceso_i = procesos[i];
      if (proceso_i != NULL)
      {
         free(proceso_i);
      }
   }
}