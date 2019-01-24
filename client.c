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

void effect(struct strand *s, int broadcast) {
  char matrix[kStrandCnt][kLEDCnt * 3];
  int meteorTrailDecay = 64;
  int meteorRandomDecay = 1;
  int meteorSize = 10;

  for (int i = 0; i < kStrandCnt; ++i)
    for (int j = 0; j < kLEDCnt; ++j) {
      int pixel = j * 3;
      matrix[i][pixel + 0] = 0x0;
      matrix[i][pixel + 1] = 0x0;
      matrix[i][pixel + 2] = 0x0;
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
          int pixel = (kLEDCnt - (j - k) - 1) * 3;
          matrix[i][pixel + 0] = 0xff;
          matrix[i][pixel + 1] = 0xff;
          matrix[i][pixel + 2] = 0xff;
        }
      }

      if (send(s[i].sock, matrix[i], kLEDCnt*3, 0) < 0) {
        fprintf(stderr, "Send failed");
        return;
      }
    }
    usleep(10000);
    if (send(broadcast, "d255", 4, 0) < 0) {
      fprintf(stderr, "Send failed");
      return;
    }
    usleep(10000);
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

int main(int c, char **v) {
  struct strand strands[kStrandCnt];

  strands[0].host = 209;
  strands[1].host = 205;
  strands[2].host = 218;

  for (int i = 0; i < kStrandCnt; ++i)
    createConnection(&strands[i]);

  int broadcast = createBroadcast();

  effect(strands, broadcast);

  for (int i = 0; i < kStrandCnt; ++i)
    close(strands[i].sock);
  close(broadcast);

  return 0;
}

