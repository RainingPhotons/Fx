#include <algorithm>
#include <chrono>
#include <map>
#include <thread>

#include <arpa/inet.h>
#include <math.h>
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
static volatile bool do_snake = false;
static std::map<int, int> strand_map;

static volatile int comet_matrix_lr[kStrandCnt][kLEDCnt];
static volatile int comet_matrix_rl[kStrandCnt][kLEDCnt];

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

void input_thread(){
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
    } else if (buffer[0] == 'n') {
      do_snake = !do_snake;
    } else if (buffer[0] == 'c') {
      if (buffer[1] == 'r')
        comet_matrix_rl[0][10] = 128;
      else if (buffer[1] == 'l')
        comet_matrix_lr[kStrandCnt - 1][10] = 128;
      else {
        const int strand = atoi(buffer + 1);
        if (strand >= 0 && strand < kStrandCnt) {
          comet_matrix_lr[strand][10] = 128;
          comet_matrix_rl[strand][10] = 128;
        }
      }
    }
  }
}

void read_strands_thread(int sock){
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
    if (moving_average > 100) {
      const int led_idx = rand() % kLEDCnt;
      comet_matrix_lr[board_idx][led_idx] = 128;
      comet_matrix_rl[board_idx][led_idx] = 128;
    }
  }
}

void display(struct strand *s, char matrix[kStrandCnt][kLEDCnt * 3]) {
  for (int i = 0; i < kStrandCnt; ++i) {
    if (send(s[i].sock, matrix[i], kLEDCnt*3, 0) < 0) {
      fprintf(stderr, "Send failed");
      return;
    }
  }
}

void flash(char matrix[kStrandCnt][kLEDCnt * 3]) {
  const int kLineNumber = 10;
  const int strand = flash_ - 1;

  if (strand >= 0) {
    matrix[strand][(kLineNumber * 3) + 0] = 128;
    matrix[strand][(kLineNumber * 3) + 1] = 128;
    matrix[strand][(kLineNumber * 3) + 2] = 128;
  }

  if (flash_ > 0)
    flash_--;
}

void snake(char matrix[kStrandCnt][kLEDCnt * 3]) {
  static int pos = 0;
  static const int kMaxPos = kStrandCnt * kLEDCnt;

  if (do_snake) {
    const int line_num = pos / kStrandCnt;
    const int line_pos = pos % kStrandCnt;
    const int line_pos_dir = (line_num % 2) ? (kStrandCnt - line_pos) : line_pos;

    matrix[line_pos_dir][(line_num * 3) + 0] = 128;
    matrix[line_pos_dir][(line_num * 3) + 1] = 128;
    matrix[line_pos_dir][(line_num * 3) + 2] = 128;

    if (pos++ > kMaxPos)
      pos = 0;
  }
}

void display_comet_rl(char matrix[kStrandCnt][kLEDCnt * 3]) {
  const int kCometTail = 10;

  for (int j = 0; j < kLEDCnt; ++j) {
    int highest_strand = -1;
    for (int i = 0; i < kStrandCnt; ++i) {
      if (comet_matrix_rl[i][j] != 0) {
        matrix[i][(j * 3) + 0] = comet_matrix_rl[i][j];
        matrix[i][(j * 3) + 1] = comet_matrix_rl[i][j];
        matrix[i][(j * 3) + 2] = comet_matrix_rl[i][j];
        highest_strand = i;
        comet_matrix_rl[i][j] = comet_matrix_rl[i][j] - kCometTail;
        if (comet_matrix_rl[i][j] < 0)
          comet_matrix_rl[i][j] = 0;
      }
    }

    if (highest_strand != -1 && highest_strand < (kStrandCnt - 1)) {
      comet_matrix_rl[highest_strand + 1][j] = 128;
    }
  }
}

void display_comet_lr(char matrix[kStrandCnt][kLEDCnt * 3]) {
  const int kCometTail = 10;

  for (int j = 0; j < kLEDCnt; ++j) {
    int lowest_strand = kStrandCnt;
    for (int i = kStrandCnt - 1; i >= 0; --i) {
      if (comet_matrix_lr[i][j] != 0) {
        matrix[i][(j * 3) + 0] = comet_matrix_lr[i][j];
        matrix[i][(j * 3) + 1] = comet_matrix_lr[i][j];
        matrix[i][(j * 3) + 2] = comet_matrix_lr[i][j];
        lowest_strand = i;
        comet_matrix_lr[i][j] = comet_matrix_lr[i][j] - kCometTail;
        if (comet_matrix_lr[i][j] < 0)
          comet_matrix_lr[i][j] = 0;
      }
    }

    if (lowest_strand != 0 && lowest_strand != kStrandCnt) {
      comet_matrix_lr[lowest_strand - 1][j] = 128;
    }
  }
}

void togetherness(char matrix[kStrandCnt][kLEDCnt * 3]) {
  for (int i = 0; i < kStrandCnt; ++i) {
    if (l_[i] > 10.0) {
      int j;
      for (j = i + 1; j < kStrandCnt; ++j) {
        if (l_[j] < 10.0)
          break;
      }
      // grouped strands
      if (j - i > 1) {
        for (int k = i; k < j; ++k) {
          for (int l = 0; l < kLEDCnt; ++l) {
            matrix[k][l * 3 + 0] = 255;//r;
            matrix[k][l * 3 + 1] = 255;//g;
            matrix[k][l * 3 + 2] = 255;//b;
          }
        }
      }
    }
  }
}

void effect(double matrix[kStrandCnt][kLEDCnt * 3],
            char output_matrix[kStrandCnt][kLEDCnt * 3]) {
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

      // the colors are computed in floating point [0.0 .. 1.0]
      // they need to be converted over to [0 .. 255]
      output_matrix[i][j * 3 + 0] = r * 255.0;
      output_matrix[i][j * 3 + 1] = g * 255.0;
      output_matrix[i][j * 3 + 2] = b * 255.0;
    }
  }
}

void setup(double matrix[kStrandCnt][kLEDCnt * 3]) {
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
  for (int i = 0; i < kStrandCnt; ++i) {
    for (int j = 0; j < kLEDCnt; ++j) {
      comet_matrix_rl[i][j] = 0;
      comet_matrix_lr[i][j] = 0;
    }
  }
}

void loop(struct strand *s) {
  double matrix[kStrandCnt][kLEDCnt * 3];
  char output_matrix[kStrandCnt][kLEDCnt * 3];

  setup(matrix);

  std::chrono::system_clock::time_point time_point =
    std::chrono::system_clock::now();

  while(keepRunning) {
    effect(matrix, output_matrix);
    flash(output_matrix);
    snake(output_matrix);
    display_comet_rl(output_matrix);
    display_comet_lr(output_matrix);
    togetherness(output_matrix);
    display(s, output_matrix);

    // delay until time to iterate again
    time_point += std::chrono::milliseconds(25);
    std::this_thread::sleep_until(time_point);
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

  srand(time(NULL));

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

  std::thread thread_input_id(input_thread);

  create_connection_read(INADDR_ANY, &read_sock, 5002);
  std::thread thread_read_id(read_strands_thread, read_sock);

  for (int i = 0; i < kStrandCnt; ++i)
    create_connection_write(strands[i].host, &strands[i].sock, 5000);

  loop(strands);

  for (int i = 0; i < kStrandCnt; ++i)
    close(strands[i].sock);

  thread_input_id.join();
  thread_read_id.join();

  close(read_sock);

  return 0;
}

