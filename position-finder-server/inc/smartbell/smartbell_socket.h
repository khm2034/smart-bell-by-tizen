#ifndef __SMARTBELL_SOCKET_H__
#define __SMARTBELL_SOCKET_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#define  BUFF_SIZE   1024
int smart_bell_socket_connect(char* data);
#endif
