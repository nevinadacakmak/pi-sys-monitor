#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int socket_client(){
    int client_fd = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    const char *hello = "Hello from client";

    //create client socket file descriptor
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    //convert IPv4 address from text to binary format
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or Address not supported");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    //connect to the server
    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    //send message to server
    send(client_fd, hello, strlen(hello), 0);
    printf("Hello message sent to server\n");

    //read response from server
    ssize_t valread = read(client_fd, buffer, 1024 - 1);
    if (valread > 0) {
        buffer[valread] = '\0';
        printf("Server replied: %s\n", buffer);
    }

    //close the connection
    close(client_fd);
    return 0;
}