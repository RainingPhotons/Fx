#include <iostream>
#include <iomanip>
#include <map>
#include <thread>

#include <unistd.h>

#include "util.h"

static const int kStrandCnt = 20;
static const int kMaxLine = 8;

static std::map<int, int> strand_map;

static volatile bool keepRunning = true;
static volatile int seen_strands[kStrandCnt] = { 0 };

void input(){
  char buffer[kMaxLine];
  while (keepRunning) {
		std::cin.get(buffer, kMaxLine);
    if (buffer[0] == 'q')
      keepRunning = false;
  }
}

void read_strands(int sock) {
  char buffer[kMaxLine];
  int16_t *buffer_ptr = (int16_t *)buffer;
	while (keepRunning) {
		if (0 > read(sock, buffer, kMaxLine)) {
			keepRunning = false;
			fprintf(stderr, "error\n");
		}

		const uint32_t board_idx = strand_map[buffer_ptr[0]];
		if (board_idx < kStrandCnt) {
			seen_strands[board_idx] = 1;
		}
	}
}

void loop() {
	for (int i = 0; i < kStrandCnt; ++i)
		std::cout << std::setw(4) << i;
	std::cout << std::endl;

  while (keepRunning) {
		std::cout << '\r';
		for (int i = 0; i < kStrandCnt; ++i)
			std::cout << std::setw(4) << seen_strands[i];
		std::cout << std::flush;
	}
}

int main(int argc, char **argv){
	int read_sock = 0;
	int host[kStrandCnt] = {0};

	if (order_strands(host, kStrandCnt) == 0) {
		fprintf(stderr, "Exiting due to ordering file errors.\n");
		return -1;
	}

  for (int i = 0; i < kStrandCnt; ++i)
		strand_map.insert(std::make_pair(host[i], i));

  create_connection_read(INADDR_ANY, &read_sock, 5002);
	std::thread thread_input(input);
	std::thread thread_read_strands(read_strands, read_sock);

	loop();

	thread_input.join();
	thread_read_strands.join();
	close(read_sock);

  return 0;
}

