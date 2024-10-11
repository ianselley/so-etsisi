# Genera el reventador de claves
# Variables
CC = cc
TARGET = reventador
OBJS = generador.o reventador.o probar-clave.o

# Regla por defecto
all: $(TARGET)

# Regla para el ejecutable
$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

# Regla para generador.o
generador.o: generador.c generador.h
	$(CC) -c generador.c

probar-clave.o: probar-clave.c probar-clave.h
	$(CC) -c probar-clave.c

# Regla para reventador.o
reventador.o: reventador.c generador.h probar-clave.h
	$(CC) -c reventador.c

# Regla para limpiar los archivos compilados
clean:
	rm -f $(OBJS) $(TARGET)

