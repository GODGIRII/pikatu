<<<<<<< HEAD
=======

>>>>>>> main
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
<<<<<<< HEAD
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
=======
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
>>>>>>> main
#include <netdb.h>
#include <arpa/inet.h>

#define PORT    "3000"
<<<<<<< HEAD
#define BUFSIZE 4096
#define BACKLOG 10
=======
#define BACKLOG 10
#define BUFSIZE 4096
>>>>>>> main

int send_all(int fd, const char *buf, int len);

#endif
