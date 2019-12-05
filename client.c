  
#include <arpa/inet.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const int kStrandCnt = 3;
static const int kLEDCnt = 120;
static const int newstrandcount =8;

struct strand {
  int sock;
  int host;
};

void effect(struct strand *s, int w, int h) {
  char matrix[kStrandCnt][kLEDCnt * 3];
  int pixel[kStrandCnt];


//This loop below does the initial color of the full LED strands before the flash
  for (int i = 0; i < kStrandCnt; ++i)
    for (int j = 0; j < kLEDCnt; ++j) {
      //for (int d = 0x00; d < 0xFF; d++) {
        matrix[i][j *3 + 0] = 0x00; //red
        matrix[i][j *3 + 1] = (j * kLEDCnt)/255; //green
        matrix[i][j *3 + 2] = 0x00; //blue
        //usleep(1000);
      //}
    }

// The loop below dictates brightness values for RGB for each sparkle pixel
  for (int c = 0; c < 500; ++c) {  // c represents number of times to loop through sparkle pixel
    for (int i = 0; i < kStrandCnt; ++i) {
      pixel[i] = rand() % kLEDCnt;
        for (int b = 0; b < 256; b++) { //newly added loop to increase color slowly
          matrix[i][pixel[i] * 3 + 0] = b; //red
          matrix[i][pixel[i] * 3 + 1] = 0x00; //green
          matrix[i][pixel[i] * 3 + 2] = 0x00; //blue
          if (send(s[i].sock, matrix[i], kLEDCnt*3, 0) < 0) {
            fprintf(stderr, "Send failed");
            return;
          }
          usleep(100);
      }

      for (int b = 255; b >= 0; b--) { //newly added loop to decrease color slowly
          matrix[i][pixel[i] * 3 + 0] = b; //red
          matrix[i][pixel[i] * 3 + 1] = 0x00; //green
          matrix[i][pixel[i] * 3 + 2] = 0x00; //blue
          if (send(s[i].sock, matrix[i], kLEDCnt*3, 0) < 0) {
            fprintf(stderr, "Send failed");
            return;
          }
          usleep(100);
      }
    }
     fprintf(stdout, "Loop 1");

    usleep(60000);
    // This loop here will dictate what the color of the pixel led will end on the blink
    /*
    for (int i = 0; i < kStrandCnt; ++i) {
      matrix[i][pixel[i] * 3 + 0] = 0x00; //red
      matrix[i][pixel[i] * 3 + 1] = 0x00; //green
      matrix[i][pixel[i] * 3 + 2] = 0x00; //blue
      if (send(s[i].sock, matrix[i], kLEDCnt*3, 0) < 0) {
        fprintf(stderr, "Send failed");
        return;
      }
    }
    */
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
  /*strands[3].host = 201;
  strands[4].host = 202;
  strands[5].host = 203;
  strands[6].host = 204;
  strands[7].host = 206;
  strands[8].host = 207;
  */
  

  for (int i = 0; i < kStrandCnt; ++i)
    createConnection(&strands[i]);

  effect(strands, w, h);

  for (int i = 0; i < kStrandCnt; ++i)
    close(strands[i].sock);

  return 0;
}
