/***
 * httptar.c
 * 
 * Este programa es un servidor que recibe un mensaje por HTTP y envía un archivo comprimido en zip del directorio de trabajo.
 * NOTA: Todo está hardcodeado. Muchas pruebas, mucho tiempo perdido en finales sin salida al trabajar con Minix 312a y Windows.
 * 
 * Programado por: Jorge Dueñas Lerín
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <winsock.h>
#include <sys/stat.h>

#define ZIPPATH "utilidades\\7za.exe"
#define WORKING_PATH ".\\trabajo"
#define WORKING_FILE ".\\trabajo.zip"
#define PORT 8842
#define RCV_BUFFER 4096

// Función para verificar si un directorio existe
int directoryExists(char *path) {
    struct stat stats;
    if (stat(path, &stats) == 0 && S_ISDIR(stats.st_mode)) {
        return 1; // El directorio existe
    }
    return 0; // El directorio no existe
}

// Función para crear el directorio si no existe
void createDirectoryIfNotExists(char *path) {
    struct stat stats;
    if (stat(path, &stats) != 0) {
        // El directorio no existe, lo creamos
        printf("Creando el directorio de trabajo: %s\n", path);
        mkdir(path);
    }
}

// Comprime path_wk en path_file con el comando tar
int compress(char *path_wk, char *path_file) {
    if (!directoryExists(path_wk)) {
        printf("Error: El directorio %s no existe.\n", path_wk);
        return 1;
    }
    char command[100];
    sprintf(command, "tar -a -c -f %s %s", path_file, path_wk);
    system(command);
    return 0;
}

// Borra fichero en path
int deleteFile(char *path) {
    if (remove(path) == 0) {
        return 0;
    } else {
        return 1;
    }
}

// Escribe en el socket el fichero WORKING_FILE con el protocolo HTTP
int sendFile(SOCKET new_socket) {
    FILE *file;
    char buffer[RCV_BUFFER];
    int read;

    file = fopen(WORKING_FILE, "rb");
    if (file == NULL) {
        printf("Error al abrir archivo.\n");
        return 1;
    }

    // Cabecera HTTP y envío del fichero
    char header[255];
    sprintf(header, "HTTP/1.0 200 OK\nContent-Type: application/octet-stream\nContent-Disposition: attachment; filename=\"%s\"\n\n", WORKING_FILE);
    send(new_socket, header, strlen(header), 0);

    while ((read = fread(buffer, 1, RCV_BUFFER, file)) > 0) {
        if (send(new_socket, buffer, read, 0) < 0) {
            printf("Error al enviar archivo.\n");
            printf("%d\n", WSAGetLastError());
            return 1;
        }
    }

    fclose(file);
    return 0;
}

// Abre socket TCP windows y recibe mensaje HTTP, comprime WORKING_PATH y envía WORKING_FILE
int openServer() {
    WSADATA wsaData;
    SOCKET s;
    struct sockaddr_in server, client;
    int c;
    char buffer[RCV_BUFFER];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Error al inicializar Winsock.\n");
        return 1;
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        printf("Error al crear socket.\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(s, (struct sockaddr *) &server, sizeof(server)) == SOCKET_ERROR) {
        printf("Error al hacer bind.\n");
        return 1;
    }

    listen(s, 3);

    printf("Esperando conexiones...\n");

    c = sizeof(struct sockaddr_in);
    SOCKET new_socket = accept(s, (struct sockaddr *) &client, &c);
    if (new_socket == INVALID_SOCKET) {
        printf("Error al aceptar conexión.\n");
        return 1;
    }

    printf("Conexión aceptada.\n");

    recv(new_socket, buffer, RCV_BUFFER, 0);

    // Verifica y crea el directorio de trabajo si es necesario
    createDirectoryIfNotExists(WORKING_PATH);

    // Comprime el directorio de trabajo
    if (compress(WORKING_PATH, WORKING_FILE) != 0) {
        printf("Error al comprimir el directorio de trabajo.\n");
        return 1;
    }

    // Envía el archivo comprimido
    if (sendFile(new_socket) != 0) {
        printf("Error al enviar el archivo comprimido.\n");
        return 1;
    }

    // Borra el archivo comprimido
    deleteFile(WORKING_FILE);

    closesocket(new_socket);
    closesocket(s);
    WSACleanup();

    return 0;
}

int main() {
    printf("Arrancando server!\n");
    while(1) {
        openServer();
    }
    return 0;
}
