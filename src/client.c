#include "common.h"

int main(void) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo("127.0.0.1", PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        freeaddrinfo(res);
        return 1;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }
    freeaddrinfo(res);

    printf("connected to server on port %s\n", PORT);

    char buf[BUFSIZE];
    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(buf, sizeof buf, stdin)) break;

        int len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') buf[--len] = '\0';
        if (len == 0) continue;

        if (send_all(sockfd, buf, len) < 0) {
            perror("send");
            break;
        }

        if (strcmp(buf, "bye") == 0) break;

        int n = recv(sockfd, buf, sizeof buf - 1, 0);
        if (n <= 0) { printf("server disconnected\n"); break; }
        buf[n] = '\0';
        printf("server: %s\n", buf);
    }

    close(sockfd);
    return 0;
}
