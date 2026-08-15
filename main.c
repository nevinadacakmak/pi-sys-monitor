#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <syslog.h>
#include <sys/types.h>

#include "metrics.h"

volatile sig_atomic_t keep_running = 1;

void sig_handler(int sig){
    (void)sig;
    keep_running = 0;
}

int main(){
    int server_fd, client_fd;
    struct sockaddr_in address;

    socklen_t addrlen = sizeof(address);
    char buffer[8192] = {0};

    struct sigaction sa;
    sa.sa_handler=&sig_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);

    //daemonize steps

    //fork, parent exists
    int c_pid = 0;
    c_pid = fork();

    if (c_pid!=0) //parent
    {
        exit(0);
    }

    //call setsid()
    setsid();

    //close fd 0 1 2 and reopen them pointing to /dev/null
    int fd = open("/dev/null", O_RDWR);
    if (fd == -1) {
        perror("Failed to open /dev/null");
        return 1;
    }

    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    close(fd);

    //switch to syslog
    openlog("pi-sys-monitor", LOG_PID | LOG_CONS, LOG_USER);

    chdir("/");

    //--------

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        syslog(LOG_INFO, "error, sigaction");
        return 1;
    }

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        syslog(LOG_INFO, "error, sigaction");
        return 1;
    }

    //create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        syslog(LOG_INFO, "Socket creation failed");
        exit(EXIT_FAILURE);
    }

    //http server
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //define server address details
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    //bind socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, addrlen) < 0) {
        syslog(LOG_INFO, "Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    //listen for incoming connection
    if (listen(server_fd, 4) < 0) {
        syslog(LOG_INFO, "Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    syslog(LOG_INFO, "Server is listening on port %d\n", 8080);

    while (keep_running) {
        //accept a client connection
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            if (errno == EINTR && !keep_running){ //even though i set _restart, this is a safety net
                break;
            }
            syslog(LOG_INFO, "Accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        //receive message sent by client
        ssize_t received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (received > 0) {
            buffer[received] = '\0';
            syslog(LOG_INFO, "Request received:\n%s\n", buffer);

            //parse the http request

            char method[16] = {0};
            char path[256] = {0};
            sscanf(buffer, "%15s %255s", method, path);
 
            char response[8192];
            int n;

            if (strcmp(path, "/metrics") == 0) {
                char body[4096];
                calculate_metrics(1, body, sizeof(body));
                size_t body_len = strlen(body);
                syslog(LOG_INFO, "hi\n");

                //reply to client
                char response[8192];
                n = snprintf(response, sizeof(response),
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/json; charset=utf-8\r\n"
                    "Content-Length: %zu\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "%s",
                    body_len, body);

                send(client_fd, response, n, 0);
                syslog(LOG_INFO, "message sent to client\n");
            }
            else{
                const char *body = "{\"error\":\"not found\"}";
                size_t body_len = strlen(body);
 
                n = snprintf(response, sizeof(response),
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: application/json; charset=utf-8\r\n"
                    "Content-Length: %zu\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "%s",
                    body_len, body);
 
                syslog(LOG_INFO, "404: %s", path);
            }
        }

        close(client_fd);
    }
    
    close(server_fd);
    closelog();
    return 0;
}