#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void serve_html(int client_socket) {
    FILE *file = fopen("index.html", "r");
    if (!file) {
        const char *error = "HTTP/1.1 404 Not Found\r\n\r\nFile not found.";
        send(client_socket, error, strlen(error), 0);
        return;
    }

    char response[BUFFER_SIZE] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    send(client_socket, response, strlen(response), 0);

    char chunk[1024];
    while (fgets(chunk, sizeof(chunk), file)) {
        send(client_socket, chunk, strlen(chunk), 0);
    }

    fclose(file);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    listen(server_fd, 3);

    printf("Server running on http://localhost:%d\n", PORT);

    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        serve_html(client_socket);
        close(client_socket);
    }

    return 0;
}
