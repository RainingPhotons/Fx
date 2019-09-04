#include <arpa/inet.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int createBroadcast() {
  struct sockaddr_in server;

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock == -1) {
    fprintf(stderr, "Could not create socket");
    return -1;
  }

  int broadcast = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                 &broadcast, sizeof(broadcast)) == -1) {
    perror("unable to broadcast");
    return -1;
  }

  server.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  server.sin_family = AF_INET;
  server.sin_port = htons(5000);

  if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
    perror("connect failed. Error");
    return -1;
  }

  return sock;
}

int main(int argc, char **argv) {

  if (argc < 2) {
    fprintf(stderr, "need parameter to send\n");
  }

  int broadcast = createBroadcast();

  if (send(broadcast, argv[1], strlen(argv[1]), 0) < 0) {
    fprintf(stderr, "Send failed");
    return -1;
  }

  close(broadcast);

  return 0;
}

