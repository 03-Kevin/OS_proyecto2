#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

#define PORT 8080
#define SHIFT 3 // Desplazamiento para el cifrado César

void cifrarCesar(char mensaje[], int shift) {
    for (int i = 0; mensaje[i]; i++) {
        if (isalpha(mensaje[i])) {
            char base = islower(mensaje[i]) ? 'a' : 'A';
            mensaje[i] = (mensaje[i] - base + shift) % 26 + base;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <nombre_archivo>\n", argv[0]);
        return 1;
    }

    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        perror("Error al crear el socket");
        return 1;
    }

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (connect(client_sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error al conectar al servidor");
        close(client_sock);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        close(client_sock);
        return 1;
    }

    char *buffer = NULL;
    size_t buffer_len = 0;
    ssize_t line_len;
    while ((line_len = getline(&buffer, &buffer_len, file)) != -1) {
        cifrarCesar(buffer, SHIFT);
        send(client_sock, buffer, strlen(buffer), 0);
    }

    free(buffer);
    fclose(file);
    close(client_sock);

    return 0;
}
