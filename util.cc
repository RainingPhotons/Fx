#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

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

int order_strands(int* host, int max_strands) {
  FILE *fp;
  fp = fopen("order.txt", "r");

  int i = 0;

  if (fp) {
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
      if (i >= max_strands) {
        fprintf(stderr, "Too many strands (>%d)\n", i);
        i = 0;
        break;
      }
      int strand_number = atoi(line);
      if (strand_number < 200 || strand_number > 256) {
        fprintf(stderr, "Bad file, number out of range (%d)\n", strand_number);
        i = 0;
        break;
      }
      host[i++] = strand_number;
    }

    fclose (fp);
  }

  return i;
}
