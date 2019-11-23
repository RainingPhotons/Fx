  
#include <arpa/inet.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const int kStrandCnt = 3;
static const int kLEDCnt = 120;

struct strand {
  int sock;
  int host;
};

void effect(struct strand *s, int w, int h) {
  char matrix[kStrandCnt][kLEDCnt * 3];
  int pixel[kStrandCnt];

  for (int i = 0; i < kStrandCnt; ++i)
    for (int j = 0; j < kLEDCnt; ++j) {
      matrix[i][j *3 + 0] = 0x10;
      matrix[i][j *3 + 1] = 0x0;
      matrix[i][j *3 + 2] = 0x0;
    }

  for (int c = 0; c < 500; ++c) {
    for (int i = 0; i < kStrandCnt; ++i) {
      pixel[i] = rand() % kLEDCnt;
      matrix[i][pixel[i] * 3 + 0] = 0xff;
      matrix[i][pixel[i] * 3 + 1] = 0x00;
      matrix[i][pixel[i] * 3 + 2] = 0x00;
      if (send(s[i].sock, matrix[i], kLEDCnt*3, 0) < 0) {
        fprintf(stderr, "Send failed");
        return;
      }
    }

    usleep(2000);
    for (int i = 0; i < kStrandCnt; ++i) {
      matrix[i][pixel[i] * 3 + 0] = 0x10;
      matrix[i][pixel[i] * 3 + 1] = 0x00;
      matrix[i][pixel[i] * 3 + 2] = 0x00;
      if (send(s[i].sock, matrix[i], kLEDCnt*3, 0) < 0) {
        fprintf(stderr, "Send failed");
        return;
      }
    }
    usleep(10000 + rand() % 90000);
  }
}

int createConnection(struct strand *s) {
  struct sockaddr_in server;

  s->sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (s->sock == -1) {
    fprintf(stderr, "Could not create socket");
    return 0;
  }

  char addr[32];
  sprintf(addr, "192.168.1.%d", s->host);
  server.sin_addr.s_addr = inet_addr(addr);
  server.sin_family = AF_INET;
  server.sin_port = htons(5000);

  if (connect(s->sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
    perror("connect failed. Error");
    return 0;
  }

  return 1;
}

int main(int c, char **v) {
  int w = 0, h = 0;
  if (c > 1) w = atoi(v[1]);
  if (c > 2) h = atoi(v[2]);
  if (w <= 0) w = 80;
  if (h <= 0) h = 120;

  struct strand strands[kStrandCnt];

  strands[0].host = 209;
  strands[1].host = 205;
  strands[2].host = 218;

  for (int i = 0; i < kStrandCnt; ++i)
    createConnection(&strands[i]);

  effect(strands, w, h);

  for (int i = 0; i < kStrandCnt; ++i)
    close(strands[i].sock);

  return 0;
}
