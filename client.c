#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

static const int kStrandCnt = 3;
static const int kLEDCnt = 120;

struct strand {
  int sock;
  int host;
};

struct pixel {
  char r;
  char g;
  char b;
};

#define for_x for (int x = 0; x < w; x++)
#define for_y for (int y = 0; y < h; y++)
#define for_xy for_x for_y
class matrix {
 public:
  matrix(int w, int h)
    : w_(w),
      h_(3*h) {
    storage_ = new char[w_*h_]();
  }
  ~matrix() {
    delete storage_;
  }

  void set(int x, int y, char val) {
    storage_[(x * h_) + (3 * y)] = val;
  }

  char get(int x, int y) {
    return storage_[(x * h_) + (3 * y)];
  }

  char *column(int x) {
    return storage_ + (x * h_);
  }

  void show() {
    printf("\033[H");
    for (int y = 0; y < (h_/3); y++) {
      for (int x = 0; x < w_; x++)
        printf(get(x,y) ? "\033[07m  \033[m" : "  ");
      printf("\033[E");
    }
    fflush(stdout);
  }

  matrix& operator=(const matrix &other) {
    memcpy(storage_, other.storage_, w_ * h_);
    return *this;
  }

 private:
  char *storage_;
  int w_;
  int h_;
};


void evolve(matrix &univ, int w, int h) {
  matrix n(w, h);

  for_y for_x {
    int v = 0;
    for (int y1 = y - 1; y1 <= y + 1; y1++)
      for (int x1 = x - 1; x1 <= x + 1; x1++)
        if (univ.get((x1 + w) % w, (y1 + h) % h))
          v++;

    if (univ.get(x,y)) v--;
    n.set(x,y,(v == 3 || (v == 2 && univ.get(x,y))));
  }
  univ = n;
}

int rand_num(int max, int min) {
  return rand() % (max - min + 1) + min;
}

int createConnection(struct strand *s) {
  struct sockaddr_in server;

  // Create socket
  s->sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (s->sock == -1) {
    printf("Could not create socket");
    return 0;
  }

  char addr[32];
  sprintf(addr, "192.168.1.%d", s->host);
  server.sin_addr.s_addr = inet_addr(addr);
  server.sin_family = AF_INET;
  server.sin_port = htons(5000);

  // Connect to remote server
  if (connect(s->sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
    perror("connect failed. Error");
    return 0;
  }

  return 1;
}

int effect(struct strand *strands, int cnt) {
  const int w = 5;
  const int h = kLEDCnt;
  matrix univ(w, h);
  for_xy univ.set(x,y,rand() < RAND_MAX / 10 ? 1 : 0);
  for (int c = 0; c < 500; ++c) {
    univ.show();
    for (int i = 0; i < cnt; ++i)
      if (send(strands[i].sock, univ.column(i+1), kLEDCnt*3, 0) < 0) {
        puts("Send failed");
        return 1;
      }
    evolve(univ, w, h);
    usleep(200000);
  }

  return 0;
}

int main(int argc, char* argv[]) {
  struct strand strands[kStrandCnt];

  strands[0].host = 201;
  strands[1].host = 200;
  strands[2].host = 213;

  srand(time(0));

  for (int i = 0; i < kStrandCnt; ++i)
    createConnection(&strands[i]);

  effect(strands, kStrandCnt);

  for (int i = 0; i < kStrandCnt; ++i)
    close(strands[i].sock);

  return 0;
}

