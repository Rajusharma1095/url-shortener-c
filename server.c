#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_URL_LENGTH 2048
#define SHORT_CODE_LENGTH 7

unsigned long hash_url(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void to_base62(unsigned long num, char *output) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    int i = 0;
    while (num > 0 && i < SHORT_CODE_LENGTH - 1) {
        output[i++] = charset[num % 62];
        num /= 62;
    }
    output[i] = '\0';
}

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

// Handle POST request for URL shortening
void handle_shorten(int client_socket, const char *url) {
    unsigned long hash = hash_url(url);
    char short_code[SHORT_CODE_LENGTH];
    to_base62(hash, short_code);

    FILE *file = fopen("urls.txt", "a");
    if (!file) {
        const char *error = "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to save URL.";
        send(client_socket, error, strlen(error), 0);
        return;
    }
    fprintf(file, "%s %s\n", short_code, url);
    fclose(file);

    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), 
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<h1>Shortened URL: </h1><a href=\"/%s\">%s</a>",
        short_code, short_code);
    send(client_socket, response, strlen(response), 0);
}

// Redirect short code to long URL
void handle_redirect(int client_socket, const char *short_code) {
    FILE *file = fopen("urls.txt", "r");
    if (!file) {
        const char *error = "HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to open URLs file.";
        send(client_socket, error, strlen(error), 0);
        return;
    }

    char code[SHORT_CODE_LENGTH], url[MAX_URL_LENGTH];
    int found = 0;

    while (fscanf(file, "%s %s", code, url) != EOF) {
        if (strcmp(code, short_code) == 0) {
            // Redirect
            char response[BUFFER_SIZE];
            snprintf(response, sizeof(response), 
                "HTTP/1.1 302 Found\r\nLocation: %s\r\n\r\n", url);
            send(client_socket, response, strlen(response), 0);
            found = 1;
            break;
        }
    }

    if (!found) {
        const char *error = "HTTP/1.1 404 Not Found\r\n\r\nShort code not found.";
        send(client_socket, error, strlen(error), 0);
    }

    fclose(file);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

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

        recv(client_socket, buffer, sizeof(buffer), 0);

        if (strncmp(buffer, "POST /shorten", 13) == 0) {
            // Extract URL from POST data
            char *url = strstr(buffer, "long_url=") + 9;
            if (url) {
                handle_shorten(client_socket, url);
            } else {
                const char *error = "HTTP/1.1 400 Bad Request\r\n\r\nInvalid URL format.";
                send(client_socket, error, strlen(error), 0);
            }
        } else if (strncmp(buffer, "GET /", 5) == 0) {
            char *short_code = buffer + 5;
            if (strlen(short_code) > 0) {
                handle_redirect(client_socket, short_code);
            } else {
                serve_html(client_socket);
            }
        }

        close(client_socket);
    }

    return 0;
}
