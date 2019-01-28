#include <arpa/inet.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const int kStrandCnt = 12;
static const int kLEDCnt = 120;

struct strand {
  int sock;
  int host;
};

void fadeToBlack(char *leds, int num, char fadeValue) {
  uint8_t r, g, b;

  r = leds[num * 3 + 0];
  g = leds[num * 3 + 1];
  b = leds[num * 3 + 2];

  r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
  g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
  b=(b<=10)? 0 : (int) b-(b*fadeValue/256);

  leds[num * 3 + 0] = r;
  leds[num * 3 + 1] = g;
  leds[num * 3 + 2] = b;
}

void effect(struct strand *s, int w, int h) {
  char matrix[kStrandCnt][kLEDCnt * 3];
  int meteorTrailDecay = 64;
  int meteorRandomDecay = 1;
  int meteorSize = 10;

  for (int i = 0; i < kStrandCnt; ++i)
    for (int j = 0; j < kLEDCnt; ++j) {
      matrix[i][j *3 + 0] = 0x0;
      matrix[i][j *3 + 1] = 0x0;
      matrix[i][j *3 + 2] = 0x0;
    }

  for (int j = 0; j < kLEDCnt + kLEDCnt; ++j) {
    for (int i = 0; i < kStrandCnt; ++i) {
      for (int k = 0; k < kLEDCnt; ++k) {
        if ((!meteorRandomDecay) || ((rand() % 10) > 5)) {
          fadeToBlack(matrix[i], k, meteorTrailDecay);
        }
      }

      for (int k = 0; k < meteorSize; ++k) {
        if ((j - k < kLEDCnt) && (j - k >= 0)) {
          matrix[i][(j - k) * 3 + 0] = 0xff;
          matrix[i][(j - k) * 3 + 1] = 0xff;
          matrix[i][(j - k) * 3 + 2] = 0xff;
        }
      }

      if (send(s[i].sock, matrix[i], kLEDCnt*3, 0) < 0) {
        fprintf(stderr, "Send failed");
        return;
      }
      usleep(8000);
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
  int w = 0, h = 0;
  if (c > 1) w = atoi(v[1]);
  if (c > 2) h = atoi(v[2]);
  if (w <= 0) w = 80;
  if (h <= 0) h = 120;

  struct strand strands[kStrandCnt];

  strands[ 0].host = 209;
  strands[ 1].host = 203;
  strands[ 2].host = 201;
  strands[ 3].host = 219;
  strands[ 4].host = 206;
  strands[ 5].host = 204;
  strands[ 6].host = 217;
  strands[ 7].host = 212;
  strands[ 8].host = 214;
  strands[ 9].host = 215;
  strands[10].host = 218;
  strands[11].host = 205;

  for (int i = 0; i < kStrandCnt; ++i)
    createConnection(&strands[i]);

  effect(strands, w, h);

  for (int i = 0; i < kStrandCnt; ++i)
    close(strands[i].sock);

  return 0;
}

