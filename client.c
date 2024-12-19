#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024

int main() {
    char *ip = "127.0.0.1";
    int port = 5566;

    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("[-]Socket error");
        exit(1);
    }

    // Initialize server address struct
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[-]Connection error");
        exit(1);
    }
    printf("Connected to the server.\n");

    // Main client loop
    while (1) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        fflush(stdout);
        printf("user > ");
        // select to handle multiple calls from server and request..
        if (select(sock + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("[-]Select error");
            exit(1);
        }

        
        if (FD_ISSET(sock, &read_fds)) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                printf("Server disconnected.\n");
                close(sock);
                exit(0);
            } else {
                printf("Server:\n");
                printf("%s\n", buffer);
            }
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
        
            fgets(command, BUFFER_SIZE, stdin);
            command[strcspn(command, "\n")] = '\0'; 

            send(sock, command, strlen(command), 0);

            if (strcmp(command, "/logout") == 0) {
                printf("Bye!! Have a nice day\n");
                close(sock);
                exit(0);
            }
        }


    }

    return 0;
}
