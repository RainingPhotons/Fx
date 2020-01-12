#include <stdio.h>

static const int kStrandCnt = 20;
static const int kIterations = 5;

static int propagation_[2][kStrandCnt];

void print_array(int* array, int size){
  for (int i = 0; i < size; ++i) {
    printf("%3d", array[i]);
  }
  printf("\n");
}

void propagate(int* array, int strand) {

}

int main(int argc, char const *argv[]){
  for (int i = 0; i < kStrandCnt; ++i) {
    propagation_[0][i] = 0;
    propagation_[1][i] = 0;
  }

  propagation_[0][10] = 100;

  for (int j = 0; j < kIterations; ++j) {
    const int read_idx = j & 1;
    const int write_idx = (~read_idx) & 1;


    for (int i = 0; i <= (kStrandCnt >> 1); ++i) {
      const int value = propagation_[read_idx][i];
      if (value > 0)
        propagation_[write_idx][i - 1] = value >> 1;
    }

    for (int i = (kStrandCnt >> 1); i < kStrandCnt; ++i) {
      const int value = propagation_[read_idx][i];
      if (value > 0)
        propagation_[write_idx][i + 1] = value >> 1;
    }

    for (int i = 0; i < kStrandCnt; ++i)
      propagation_[read_idx][i] = 0;


    print_array(propagation_[write_idx], kStrandCnt);
  }
}
