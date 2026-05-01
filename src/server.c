/*
     * Server.c - Configure and run a TCP echo server
     * Port : 4200
     * IP   : 192.168.0.129
*/

#include "common.h"

static volatile int running = 1;

// Signal handler set to 0

static void handle_sigint(int sig) {
    (void)sig;
    running = 0;
}

int create_server_socket(void) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        return -1;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        freeaddrinfo(res);
        return -1;
    }

    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("bind");
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }
    freeaddrinfo(res);

    if (listen(sockfd, BACKLOG) < 0) {
        perror("listen");
        close(sockfd);
        return -1;
    }

    printf("server listening on port %s\n", PORT);
    return sockfd;
}

void *handle_client(void *arg) {
    int fd = *(int *)arg;
    free(arg);

    char msg[BUFSIZE];
    while (1) {
        int n = recv(fd, msg, sizeof(msg) - 1, 0);
        if (n < 0) { perror("recv");                     break; }
        if (n == 0){ printf("client disconnected\n");    break; }

        msg[n] = '\0';
        printf("client says: %s\n", msg);

        if (send_all(fd, msg, n) < 0) {
            perror("send");
            break;
        }

        if (strcmp(msg, "bye") == 0) {
            printf("closing connection\n");
            break;
        }
    }

    close(fd);
    return NULL;
}

void run_server(int sockfd) {
    signal(SIGINT, handle_sigint);

    while (running) {
        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof client_addr;

        int client_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
        if (client_fd < 0) {
            if (running) perror("accept");
            continue;
        }

        char ip[INET6_ADDRSTRLEN];
        struct sockaddr_in *s = (struct sockaddr_in *)&client_addr;
        inet_ntop(AF_INET, &s->sin_addr, ip, sizeof ip);
        printf("new connection from %s\n", ip);

        int *fd_ptr = malloc(sizeof(int));
        if (!fd_ptr) { close(client_fd); continue; }
        *fd_ptr = client_fd;

        pthread_t t;
        pthread_create(&t, NULL, handle_client, fd_ptr);
        pthread_detach(t);
    }

    close(sockfd);
    printf("server stopped\n");
}

int main(void) {
    int sockfd = create_server_socket();
    if (sockfd < 0) return 1;

    run_server(sockfd);
    return 0;
}
