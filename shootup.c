  
#include <arpa/inet.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const int kStrandCnt = 19;
static const int kLEDCnt = 120;
static const int singlekStrandCnt = 1;

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
  //fprintf(stderr, "f,%d\n", (num * 3 + 2));
}

void effect(struct strand *s, int w, int h) {
  char matrix[singlekStrandCnt][kLEDCnt * 3];
  int meteorTrailDecay = 64;
  int meteorRandomDecay = 1;
  int meteorSize = 10;

//Blanking loop - will set colors of all leds before the pattern
  for (int i = 0; i < singlekStrandCnt; ++i) {
    //for (int j = kLEDCnt; j <= 0; --j) {
    for (int j = 0; j < kLEDCnt; ++j) {
      matrix[i][j *3 + 0] = 0x0;
      matrix[i][j *3 + 1] = 0x0;
      matrix[i][j *3 + 2] = 0x0;
    }
    //sleep(2);
  }


  for (int j = kLEDCnt + kLEDCnt; j > 0; --j) { 
  //for (int j = 0; j < kLEDCnt + kLEDCnt; ++j) {
    for (int i = 0; i < singlekStrandCnt; ++i) {
      for (int k = kLEDCnt-1; k >= 0; --k) {
      //for (int k = 0; k < kLEDCnt; ++k) {
        if ((!meteorRandomDecay) || ((rand() % 10) > 5)) {
          fadeToBlack(matrix[i], k, meteorTrailDecay);
        }
      }

      //for (int k = 0; k < meteorSize; ++k) {
        for (int k = 0; k < meteorSize; ++k) {

        if ((j - k < kLEDCnt) && (j - k >= 0)) {
          matrix[i][(j - k) * 3 + 0] = 0x00;
          matrix[i][(j - k) * 3 + 1] = 0x00;
          matrix[i][(j - k) * 3 + 2] = 0xdd;
          //fprintf(stderr, "k,%d\n", (j - k) * 3 + 1);
        }
      }

      if (send(s[i].sock, matrix[i], kLEDCnt*3, 0) < 0) {
        fprintf(stderr, "Send failed on i,%d\n", i);
        fprintf(stderr, "Send failed on kLEDCnt,%d\n", kLEDCnt*3);
        return;
      }
      usleep(2000);
    }
  }
}

/*void cascade(int startIndex, int endIndex){
  int steps = (endIndex + 1 - startIndex);
  int i = (startIndex + endIndex)>>1;
  //int stepdir = 1;
  for (int q=0; q < steps; q++) {
   int index = i + ( q% 2 == 0 ? q/2 : -(q/2+1)); //index lookup here
   fprintf(stderr, "printing index value%d\n", index);
   effect(&strands[index], w, h);
}
}
*/

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
  strands[19].host = 203;
  strands[18].host = 209;
  strands[17].host = 206;
  strands[16].host = 201;
  strands[15].host = 200;
  strands[14].host = 213;
  strands[13].host = 210;
  strands[12].host = 207;
  strands[11].host = 202;
  strands[10].host = 216;
  strands[ 9].host = 220;
  strands[ 8].host = 205;
  strands[ 7].host = 217;
  strands[ 6].host = 212;
  strands[ 5].host = 218;
  strands[ 4].host = 204;
  strands[ 3].host = 211;
  strands[ 2].host = 214;
  strands[ 1].host = 219;
  strands[ 0].host = 208;
  

  for (int i = 0; i < kStrandCnt; ++i)
    createConnection(&strands[i]);
  
  int startIndex =0;
  int endIndex = kStrandCnt;
  int steps = (endIndex + 1 - startIndex);
  int i = (startIndex + endIndex)>>1;
  //int stepdir = 1;
  for (int q=0; q < steps; q++) {
   int index = i + ( q% 2 == 0 ? q/2 : -(q/2+1)); //index lookup here
   fprintf(stderr, "printing index value%d\n", index);
   effect(&strands[index], w, h);
   usleep(10);
}
  /*
  effect(&strands[0], w, h);
  fprintf(stderr, "Sending to strand 1\n");
  sleep(2);
  effect(&strands[1], w, h);
  fprintf(stderr, "Sending to strand 1\n");
  sleep(2);
  */
  //effect(&strands[1], w, h);

  //for (int i =0; i < kStrandCnt - 1; ++i) {
    //effect(&strands[i], w, h);
    //sleep(2);
  //}
  fprintf(stderr, "Done with pattern");

  for (int i = 0; i < kStrandCnt; ++i)
    close(strands[i].sock);

  return 0;
}








/*
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

    usleep(10000 + rand() % 90000);
  }
}
*/