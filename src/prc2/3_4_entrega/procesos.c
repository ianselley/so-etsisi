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

/* función para añadir la información de un proceso al array procesos
   en la posición ix */
int anadir_proceso(struct proceso *procesos[], int ix, int pid,
                   char *ejecutable, int estado)
{
   /**** PRÁCTICA ****/
   /* Declara un puntero a struct proceso */
   struct proceso *proceso_pointer;

   /* Reserva memoria para guardar la información del nuevo proceso.
      Contempla que puede producirse error en la reserva.
      Devuelve MEM_ERROR
   */
   proceso_pointer = calloc(1, sizeof(struct proceso));

   if (proceso_pointer == NULL)
   {
      return MEM_ERROR;
   }

   /* Modifica el valor del pid de la estructura reservada
      dinámicamente accediendo a través del operador '->' */
   proceso_pointer->bt0480_pid = pid;

   /* Utiliza la función strncpy para copiar el parámetro con la ruta del
      ejecutable a la estructura */
   strncpy(proceso_pointer->bt0480_ejecutable, ejecutable, MAX_EJECUTABLE);

   /* establece su estado */
   proceso_pointer->bt0480_estado = estado;

   /* Asigna a la posición ix del array de punteros el nuevo registro */
   procesos[ix] = proceso_pointer;

   return 0;
}

/* función para listar */
void listar_procesos(struct proceso *procesos[])
{
   /**** PRÁCTICA ****/
   /* Recorre el array de procesos con for, escribiendo con printf los campos
      de cada proceso. Si una posición está vacía, imprimir "Vacía" */
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

/* función para liberar */
void liberar_procesos(struct proceso *procesos[])
{
   /**** PRÁCTICA ****/
   /* Recorre el array de procesos con for liberando la memoria de cada
      proceso */
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