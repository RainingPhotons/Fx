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

template <class T>
class matrix {
 public:
  matrix(int w, int h)
    : w_(w),
      h_(h) {
    storage_ = new T[w_*h_]();
  }
  ~matrix() {
    delete storage_;
  }

  void set(int x, int y, T val) {
    storage_[(x * h_) + y] = val;
  }

  T get(int x, int y) {
    return storage_[(x * h_) + y];
  }

  T *column(int x) {
    return storage_ + (x * h_);
  }

  void show() {
    int max_display_height = h_;
    if (max_display_height > 50)
      max_display_height = 50;
    printf("\033[H");
    for (int y = 0; y < max_display_height; y++) {
      for (int x = 0; x < w_; x++) {
        double intensity = get(x,y);
        if (intensity < 0.01)
          intensity = 0.0;
        printf("\033[38;5;%dm",(int)(intensity*255));
        printf(get(x,y) ? "\033[07m  \033[m" : "  ");
        printf("\033[0m");
      }
      printf("\033[E");
    }
    fflush(stdout);
  }

  matrix& operator=(const matrix &other) {
    memcpy(storage_, other.storage_, w_ * h_ * sizeof(T));
    return *this;
  }
 private:
  T *storage_;
  int w_;
  int h_;
};


void evolve(matrix<double> &current, matrix<double> &previous, int w, int h) {
  double damping = 0.8;

  for (int y = 1; y < h - 1; y++) {
    for (int x = 1; x < w - 1; x++) {
      double sum = previous.get(x-1, y) + previous.get(x+1,y) +
                   previous.get(x,y+1) + previous.get(x,y-1) +
                   previous.get(x-1,y-1) + previous.get(x-1,y+1) +
                   previous.get(x+1,y-1) + previous.get(x+1,y+1);
      double result = (sum / 4.0) - current.get(x,y);
      double particle = result * damping;
      current.set(x, y, particle);
    }
  }
}

void convert(char *leds, double *particle, int length) {
  for (int i = 0; i < length; ++i) {
    double p = particle[i];
    double hue = p * 360;
    double saturation = 1.0;
    double value = 0.75;

    double chroma = value * saturation;
    double hue1 = hue / 60.0;
    double x = chroma * (1 - fabs(fmod(hue1, 2.0) - 1));
    double r1, g1, b1;

    if (hue1 >= 0.0 && hue1 <= 1.0) {
      r1 = chroma;
      g1 = x;
      b1 = 0.0;
    } else if (hue1 >= 1.0 && hue1 <= 2.0) {
      r1 = x;
      g1 = chroma;
      b1 = 0.0;
    } else if (hue1 >= 2.0 && hue1 <= 3.0) {
      r1 = 0.0;
      g1 = chroma;
      b1 = x;
    } else if (hue1 >= 3.0 && hue1 <= 4.0) {
      r1 = 0.0;
      g1 = x;
      b1 = chroma;
    } else if (hue1 >= 4.0 && hue1 <= 5.0) {
      r1 = x;
      g1 = 0.0;
      b1 = chroma;
    } else if (hue1 >= 5.0 && hue1 <= 6.0) {
      r1 = chroma;
      g1 = 0.0;
      b1 = x;
    }

    double m = value - chroma;
    r1 += m;
    g1 += m;
    b1 += m;

    if (p < 0.01) {
      r1 = 0.0;
      g1 = 0.0;
      b1 = 0.0;
    }

    leds[i * 3 + 0] = (char)(255.0 * r1);
    leds[i * 3 + 1] = (char)(255.0 * g1);
    leds[i * 3 + 2] = (char)(255.0 * b1);
  }
}

void game(struct strand *s, int w, int h) {
  matrix<double> current(w, h);
  matrix<double> previous(w, h);
  current.set(2,10,255.0);
  for (int c = 0; c < 5000; ++c) {
    current.show();
    for (int i = 0; i < kStrandCnt; ++i) {
      char leds[kLEDCnt * 3];
      convert(leds, current.column(i + 1), h);
      if (send(s[i].sock, leds, kLEDCnt*3, 0) < 0) {
        fprintf(stderr, "Send failed");
        return;
      }
    }

    if (c & 1)
      evolve(current, previous, w, h);
    else
      evolve(previous, current, w, h);
    usleep(200000);
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

  strands[0].host = 201;
  strands[1].host = 200;
  strands[2].host = 213;

  for (int i = 0; i < kStrandCnt; ++i)
    createConnection(&strands[i]);

  game(strands, w, h);

  for (int i = 0; i < kStrandCnt; ++i)
    close(strands[i].sock);

  return 0;
}

