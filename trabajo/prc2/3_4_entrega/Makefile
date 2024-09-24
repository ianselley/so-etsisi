# Compilación de un programa en C

# Generación del fichero ejecutable:
# ./4entrega-_id_
TARGET=4entrega-bt0480
# sustituye _id_ por tu identificador

$(TARGET): main.o procesos.o
	cc main.o procesos.o -o $(TARGET)
	./$(TARGET)

main.o: main.c
	cc -c main.c -o main.o

procesos.o: procesos.c
	cc -c procesos.c -o procesos.o

# Añade las reglas para obtener cada fichero objeto compilando el fuente correspodiente.
# Ten en cuenta que si cambia un fichero .c sólo se debe recompilar ese fichero.


# Deberás completar la regla "clean" para que borre el ejecutable y todos los ficheros
# objetos generados.
clean: # No tiene dependencias
	rm *.o $(TARGET)

# un ejemplo de ejecución manual de cada paso es:
# $ cc -c procesos.c -o procesos.o
# $ cc -c main.c -o main.o
# $ cc procesos.o main.o -o 4entrega-todo     
# $ ./4entrega-_id_
# Posición de tabla 0: 1 /bin/bash 0
# Posición de tabla 1: 2 /bin/top 1
# Posición de tabla 2: 3 /bin/unzip 2
# Posición de tabla 3: Vacía