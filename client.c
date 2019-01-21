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
	char matrix[cnt][kLEDCnt*3];
	int last[cnt];
	int done = 0;

	for (int i = 0; i < cnt; ++i) {
		memset(matrix[i], 0, kLEDCnt*3);
		last[i] = kLEDCnt - 1;
	}

	do {
		for (int i = 0; i < cnt; ++i) {
			struct pixel *p = (struct pixel *)matrix[i];

			for (int j = last[i] - 1; j > 0; j--) {
				p[j].r = p[j - 1].r;
				p[j].g = p[j - 1].g;
				p[j].b = p[j - 1].b;
			}

			if (rand_num(5, 0) == 0) {
				p[0].g = rand_num(255, 0);
			} else {
				p[0].g = 0;
			}

			if (p[last[i] - 1].g != 0)
				last[i]--;
		}


		for (int i = 0; i < cnt; ++i)
			if (send(strands[i].sock, matrix[i], kLEDCnt*3, 0) < 0) {
				puts("Send failed");
				return 1;
			}

		usleep(100000);

		done = 1;
		for (int i = 0; i < cnt; ++i) {
			done &= (last[i] == 0);
		}
	} while(!done);

	return 0;
}

int main(int argc, char* argv[])
{
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

