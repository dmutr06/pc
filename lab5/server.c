#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include "cache.h"

#define PORT 8080
#define STATIC_PATH "public"
#define BUFFER_SIZE 4096

void *handle_client(void *client_socket_ptr);
void send_response(int client_socket, const char *status, const char *content_type, const char *body, int body_len);
void send_file_response(int client_socket, const char *filepath);
void send_404(int client_socket);

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t thread;
        int *new_sock = malloc(sizeof(int));
        *new_sock = client_socket;

        if (pthread_create(&thread, NULL, handle_client, (void *)new_sock) != 0) {
            perror("Thread creation failed");
            close(client_socket);
            free(new_sock);
        }
        pthread_detach(thread);
    }

    close(server_socket);
    return 0;
}

void *handle_client(void *client_socket_ptr) {
    int client_socket = *(int *)client_socket_ptr;
    free(client_socket_ptr);

    char buffer[BUFFER_SIZE];
    int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        
        char method[16], path[256], protocol[16];
        if (sscanf(buffer, "%15s %255s %15s", method, path, protocol) == 3) {
            if (strcmp(method, "GET") == 0) {
                if (strcmp(path, "/") == 0) {
                    strcpy(path, "/index.html");
                }

                char filepath[512];
                snprintf(filepath, sizeof(filepath), "%s%s", STATIC_PATH, path);

                if (strstr(filepath, "..") != NULL) {
                    send_404(client_socket);
                } else {
                    send_file_response(client_socket, filepath);
                }
            } else {
                const char *response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
                send(client_socket, response, strlen(response), 0);
            }
        }
    }

    close(client_socket);
    return NULL;
}

void send_file_response(int client_socket, const char *filepath) {
    FileCache *file_info = get_file_cache(filepath);
    
    if (file_info == NULL) {
        send_404(client_socket);
        return;
    }

    char header[512];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 200 OK\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %d\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              file_info->content_type, file_info->file_size);
    send(client_socket, header, header_len, 0);

    off_t offset = 0;
    sendfile(client_socket, file_info->fd, &offset, file_info->file_size);
}

void send_404(int client_socket) {
    const char *body = "<html><body><h1>404 Not Found</h1></body></html>";
    send_response(client_socket, "404 Not Found", "text/html", body, strlen(body));
}

void send_response(int client_socket, const char *status, const char *content_type, const char *body, int body_len) {
    char header[512];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 %s\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %d\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              status, content_type, body_len);

    send(client_socket, header, header_len, 0);
    send(client_socket, body, body_len, 0);
}
