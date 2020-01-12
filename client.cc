#include <algorithm>
#include <map>

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
static const int kMaxLine = 8;
static const int kGravity = 800;
static const int kWindowSize = 8;
static volatile int keepRunning = 1;
static volatile double s_ = 100.0;
static volatile double l_[kStrandCnt];
static volatile int moving_window[kStrandCnt][kWindowSize];
static volatile int moving_window_sum[kStrandCnt];
static volatile int mw_idx[kStrandCnt];
static volatile int flash_ = 0;
static std::map<int, int> strand_map;

struct strand {
  int sock;
  int host;
};

int moving_window_average(int new_value, int idx) {
  moving_window_sum[idx] += new_value;
  moving_window_sum[idx] -= moving_window[idx][mw_idx[idx]];
  moving_window[idx][mw_idx[idx]] = new_value;
  mw_idx[idx]++;
  if (mw_idx[idx] == kWindowSize)
    mw_idx[idx] = 0;

  return moving_window_sum[idx] / kWindowSize;
}

void *input_thread(void *vargp){
  char buffer[256];
  while(keepRunning) {
    scanf("%5s", buffer);
    if (buffer[0] == 's') {
      s_ = atoi(buffer + 1);
    } else if (buffer[0] == 'l') {
      for (int i = 0; i < kStrandCnt; ++i)
        l_[i] = atoi(buffer + 1);
    } else if (buffer[0] == 'q') {
      keepRunning = 0;
    } else if (buffer[0] == 'f') {
      flash_ = kStrandCnt + 1;
    }
  }
  return NULL;
}

void *read_strands_thread (void *vargp){
  int sock = *((int *)vargp);
  char buffer[kMaxLine];
  int16_t *buffer_ptr = (int16_t *)buffer;

  while(keepRunning) {
    if (0 > read(sock, buffer, kMaxLine)) {
      keepRunning = 0;
      fprintf(stderr, "error\n");
    }
    const int board_idx = strand_map[buffer_ptr[0]];
    const int x = buffer_ptr[1];
    const int y = buffer_ptr[2];
    const int z = buffer_ptr[3];
    const double magnitude = sqrt(x*x + y*y + z*z);
    const int value = (magnitude / 10.0) - kGravity;
    const int moving_average = moving_window_average(value, board_idx);
    l_[board_idx] = std::min(80, std::max(1, moving_average));
  }

  return NULL;
}

void display(struct strand *s, double matrix[kStrandCnt][kLEDCnt * 3]) {
  for (int i = 0; i < kStrandCnt; ++i) {
    // the colors are computed in floating point [0.0 .. 1.0]
    // they need to be converted over to [0 .. 255]
    char output_matrix[kLEDCnt * 3];
    for (int j = 0; j < kLEDCnt * 3; ++j) {
      output_matrix[j] = matrix[i][j] * 255.0;
    }

    // flash dots
    if ((flash_ - 1)== i) {
      const int line_num = 10;
      output_matrix[(line_num * 3) + 0] = 128;
      output_matrix[(line_num * 3) + 1] = 128;
      output_matrix[(line_num * 3) + 2] = 128;
    }

    if (send(s[i].sock, output_matrix, kLEDCnt*3, 0) < 0) {
      fprintf(stderr, "Send failed");
      return;
    }

    usleep(1000);
  }

  if (flash_ > 0)
    flash_--;

  // if using less than 20 strands this will
  // keep about the same refresh rate as 20 strands
  usleep((20 - kStrandCnt) * 1000);
}

void effect(struct strand *s) {
  double matrix[kStrandCnt][kLEDCnt * 3];
  double ll[kStrandCnt] = { 0 };
  for (int i = 0; i < kStrandCnt; ++i) {
    if (l_[i] != ll[i]) {
      ll[i] = l_[i];
      for (int j = 0; j < kLEDCnt; ++j) {
        double h, r, g, b;
        h = (j + (i * 10)) * (360.0 / (kLEDCnt + (kStrandCnt * 10)));
        hsluv2rgb(h, s_, ll[i], &r, &g, &b);

        matrix[i][j * 3 + 0] = r;
        matrix[i][j * 3 + 1] = g;
        matrix[i][j * 3 + 2] = b;
      }
    }
  }

  while(keepRunning) {
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

        l = l_[i];
        hsluv2rgb(h, s_, l, &r, &g, &b);

        matrix[i][j * 3 + 0] = r;
        matrix[i][j * 3 + 1] = g;
        matrix[i][j * 3 + 2] = b;
      }
    }

    display(s, matrix);
  }
}

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
int main(int c, char **v) {
  struct strand strands[kStrandCnt];
  int read_sock = -1;

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

  for (int i = 0; i < kStrandCnt; ++i)
    l_[i] = 5.0;

  for (int i = 0; i < kStrandCnt; ++i)
    strand_map.insert(std::make_pair(strands[i].host, i));

  for (int i = 0; i < kStrandCnt; ++i) {
    for (int j = 0; j < kWindowSize; ++j)
      moving_window[i][j] = 0;
    moving_window_sum[i] = 0;
    mw_idx[i] = 0;
  }

  pthread_t thread_input_id;
  pthread_create(&thread_input_id, NULL, input_thread, NULL);

  create_connection_read(INADDR_ANY, &read_sock, 5002);
  pthread_t thread_read_id;
  pthread_create(&thread_read_id, NULL, read_strands_thread, (void *)&read_sock);

  for (int i = 0; i < kStrandCnt; ++i)
    create_connection_write(strands[i].host, &strands[i].sock, 5000);

  effect(strands);

  for (int i = 0; i < kStrandCnt; ++i)
    close(strands[i].sock);

  pthread_join(thread_input_id, NULL);
  pthread_join(thread_read_id, NULL);

  close(read_sock);

  return 0;
}

