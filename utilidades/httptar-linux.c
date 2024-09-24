/***
 * httptar.c
 *
 * This program is a server that receives an HTTP message and sends a compressed zip file of the working directory.
 * NOTE: Everything is hardcoded.
 *
 * Ported to Linux by: OpenAI's ChatGPT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // for close()
#include <arpa/inet.h>
#include <sys/stat.h>
#include <errno.h>

#define WORKING_PATH "./trabajo"
#define WORKING_FILE "./trabajo.zip"
#define PORT 8842
#define RCV_BUFFER 4096

// Function to check if a directory exists
int directoryExists(char *path)
{
    struct stat stats;
    if (stat(path, &stats) == 0 && S_ISDIR(stats.st_mode))
    {
        return 1; // The directory exists
    }
    return 0; // The directory does not exist
}

// Function to create the directory if it does not exist
void createDirectoryIfNotExists(char *path)
{
    struct stat stats;
    if (stat(path, &stats) != 0)
    {
        // The directory does not exist, create it
        printf("Creating working directory: %s\n", path);
        if (mkdir(path, 0777) == -1)
        {
            printf("Error creating directory: %s\n", strerror(errno));
        }
    }
}

// Compress path_wk into path_file using zip command
int compress(char *path_wk, char *path_file)
{
    if (!directoryExists(path_wk))
    {
        printf("Error: Directory %s does not exist.\n", path_wk);
        return 1;
    }
    char command[256];
    sprintf(command, "zip -r %s %s", path_file, path_wk);
    system(command);
    return 0;
}

// Delete file at path
int deleteFile(char *path)
{
    if (remove(path) == 0)
    {
        return 0;
    }
    else
    {
        perror("Error deleting file");
        return 1;
    }
}

// Write the file WORKING_FILE to the socket with HTTP protocol
int sendFile(int new_socket)
{
    FILE *file;
    char buffer[RCV_BUFFER];
    int read;

    file = fopen(WORKING_FILE, "rb");
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return 1;
    }

    // Get file size
    fseek(file, 0L, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    // HTTP header and file sending
    char header[512];
    sprintf(header, "HTTP/1.0 200 OK\r\n"
                    "Content-Type: application/octet-stream\r\n"
                    "Content-Disposition: attachment; filename=\"%s\"\r\n"
                    "Content-Length: %ld\r\n\r\n",
            WORKING_FILE, filesize);
    send(new_socket, header, strlen(header), 0);

    while ((read = fread(buffer, 1, RCV_BUFFER, file)) > 0)
    {
        if (send(new_socket, buffer, read, 0) < 0)
        {
            printf("Error sending file.\n");
            return 1;
        }
    }

    fclose(file);
    return 0;
}

// Open TCP socket, receive HTTP message, compress WORKING_PATH, and send WORKING_FILE
int openServer()
{
    int s;
    struct sockaddr_in server, client;
    socklen_t c;
    char buffer[RCV_BUFFER];

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        perror("Error creating socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        close(s);
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Error binding socket");
        close(s);
        return 1;
    }

    if (listen(s, 3) < 0)
    {
        perror("Error listening");
        close(s);
        return 1;
    }

    printf("Waiting for connections...\n");

    c = sizeof(struct sockaddr_in);
    int new_socket = accept(s, (struct sockaddr *)&client, &c);
    if (new_socket < 0)
    {
        perror("Error accepting connection");
        close(s);
        return 1;
    }

    printf("Connection accepted.\n");

    recv(new_socket, buffer, RCV_BUFFER, 0);

    // Verify and create the working directory if necessary
    createDirectoryIfNotExists(WORKING_PATH);

    // Compress the working directory
    if (compress(WORKING_PATH, WORKING_FILE) != 0)
    {
        printf("Error compressing the working directory.\n");
        close(new_socket);
        close(s);
        return 1;
    }

    // Send the compressed file
    if (sendFile(new_socket) != 0)
    {
        printf("Error sending the compressed file.\n");
        deleteFile(WORKING_FILE);
        close(new_socket);
        close(s);
        return 1;
    }

    // Delete the compressed file
    deleteFile(WORKING_FILE);

    close(new_socket);
    close(s);

    return 0;
}

int main()
{
    printf("Starting server!\n");
    while (1)
    {
        openServer();
    }
    return 0;
}
