#include <arpa/inet.h>
#include <stdio.h>

#include "connection.h"

int create_connection(in_addr_t addr, int* sock, int port, int read) {
  struct sockaddr_in server;

  *sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (*sock == -1) {
    fprintf(stderr, "Could not create socket");
    return 0;
  }

  server.sin_addr.s_addr = addr;
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  if (read) {
    if (bind(*sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect to server failed");
        return 0;
    }
  } else {
    if (connect(*sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
      perror("connect failed. Error");
      return 0;
    }
  }

  return 1;
}

int create_connection_write(int host, int* sock, int port) {
  char addr[32];
  sprintf(addr, "192.168.1.%d", host);

  return create_connection(inet_addr(addr), sock, port, 0);
}

int create_connection_read(in_addr_t addr, int* sock, int port) {
  return create_connection(addr, sock, port, 1);
}
