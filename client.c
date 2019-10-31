#include <arpa/inet.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hsluv.h"

static const int kStrandCnt = 20;
static const int kLEDCnt = 120;
static volatile int keepRunning = 1;
static volatile double s_ = 100.0;
static volatile double l_ = 5.0;

struct strand {
  int sock;
  int host;
};

void *inputThread(void *vargp){
  char buffer[256];
  while(keepRunning) {
    scanf("%5s", buffer);
    if (buffer[0] == 's')
      s_ = atoi(buffer + 1);
    else if (buffer[0] == 'l')
      l_ = atoi(buffer + 1);
    else if (buffer[0] == 'q')
      keepRunning = 0;
  }
  return NULL;
}

void effect(struct strand *s) {
  double matrix[kStrandCnt][kLEDCnt * 3];
  double l = 0;
  while(keepRunning) {
    if (l_ != l) {
      l = l_;
      for (int i = 0; i < kStrandCnt; ++i) {
        for (int j = 0; j < kLEDCnt; ++j) {
          double h, r, g, b;
          h = j * (360.0 / kLEDCnt);
          hsluv2rgb(h, s_, l, &r, &g, &b);

          matrix[i][j * 3 + 0] = r;
          matrix[i][j * 3 + 1] = g;
          matrix[i][j * 3 + 2] = b;
        }
      }
    }


    for (int i = 0; i < kStrandCnt; ++i) {
      for (int j = 0; j < kLEDCnt; ++j) {
        double h, s, l, r, g, b;

        r = matrix[i][j * 3 + 0];
        g = matrix[i][j * 3 + 1];
        b = matrix[i][j * 3 + 2];
        rgb2hsluv(r, g, b, &h, &s, &l);

        h += 360.0 / kLEDCnt;

        if (h > 360.0)
          h -= 360.0;

        hsluv2rgb(h, s_, l, &r, &g, &b);

        matrix[i][j * 3 + 0] = r;
        matrix[i][j * 3 + 1] = g;
        matrix[i][j * 3 + 2] = b;
      }

      char output_matrix[kLEDCnt * 3];
      for (int j = 0; j < kLEDCnt * 3; ++j) {
        output_matrix[j] = matrix[i][j] * 255.0;
      }

      if (send(s[i].sock, output_matrix, kLEDCnt*3, 0) < 0) {
        fprintf(stderr, "Send failed");
        return;
      }

      usleep(800);
    }
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
  struct strand strands[kStrandCnt];

  strands[ 0].host = 203;
  strands[ 1].host = 209;
  strands[ 2].host = 206;
  strands[ 3].host = 201;
  strands[ 4].host = 200;
  strands[ 5].host = 213;
  strands[ 6].host = 210;
  strands[ 7].host = 207;
  strands[ 8].host = 202;
  strands[ 9].host = 216;
  strands[10].host = 220;
  strands[11].host = 205;
  strands[12].host = 217;
  strands[13].host = 212;
  strands[14].host = 218;
  strands[15].host = 204;
  strands[16].host = 211;
  strands[17].host = 214;
  strands[18].host = 219;
  strands[19].host = 208;

  pthread_t thread_id;
  pthread_create(&thread_id, NULL, inputThread, NULL);

  for (int i = 0; i < kStrandCnt; ++i)
    createConnection(&strands[i]);

  effect(strands);

  for (int i = 0; i < kStrandCnt; ++i)
    close(strands[i].sock);

  pthread_join(thread_id, NULL);

  return 0;
}

