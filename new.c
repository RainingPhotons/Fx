#include <arpa/inet.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const int kStrandCnt = 1;
static const int kLEDCnt = 120;
static const int newkStrandCnt = 1;

struct strand {
  int sock;
  int host;
};

// *** REPLACE FROM HERE ***
void loop() {
  // ---> here we call the effect function <---
}

// ---> here we define the effect function <---
void effect(){
    char matrix[kStrandCnt][kLEDCnt * 3];
    for int i =0; i < 3; i++){
        // FADE IN
        for int j = 0; j < 256; j++){
            switch(i) {
                case 0 : matrix[i][j * 3 + 0] = 0xFF;
                case 1 : matrix[i][j * 3 + 1] = 0xFF;
                case 2 : matrix[i][j * 3 + 2] = 0xFF;
            }
        if (send(s[i].sock, matrix[i], kLEDCnt*3, 0) < 0) {
        fprintf(stderr, "Send failed");
        return;
        }
        usleep(3000);
        }
    for int j = 0; j >= 0; j--){
            switch(i) {
                case 0 : matrix[i][j * 3 + 0] = 0xFF;
                case 1 : matrix[i][j * 3 + 1] = 0xFF;
                case 2 : matrix[i][j * 3 + 2] = 0xFF;
            }
        if (send(s[i].sock, matrix[i], kLEDCnt*3, 0) < 0) {
        fprintf(stderr, "Send failed");
        return;
        }
        }
    }
}
// *** REPLACE TO HERE ***


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

  strands[0].host = 208;
  //strands[1].host = 205;
  //strands[2].host = 218;
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