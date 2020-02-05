#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int create_connection(in_addr_t addr, int* sock, int port, int read);
int create_connection_write(int host, int* sock, int port);
int create_connection_read(in_addr_t addr, int* sock, int port);
int order_strands(int* host, int max_strands);
