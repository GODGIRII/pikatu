#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT    "3000"
#define BUFSIZE 4096
#define BACKLOG 10

int send_all(int fd, const char *buf, int len);

#endif
