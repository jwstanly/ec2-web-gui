#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "responses.h"

#define TRANSFER_BUFFER_SIZE 1000
#define TIMEOUT_SEC 10

int count;
int base_port;

void handleError(int retVal, char* message) {
	if(retVal == -1) {
		fprintf(stderr, "%s syscall failed\n", message);
		exit(-1);
	}
}

struct ThreadData {
	unsigned long ip;
	int counter;// = 0;
	pthread_mutex_t mutex;// = PTHREAD_MUTEX_INITIALIZER;
	time_t lastClaimTime;
};

struct ThreadStarter {
	struct ThreadData* data;
	unsigned long ip;
	int browserfd;
	int vncfd;
};

void returnAddr(struct ThreadData* addrs, unsigned long ip);
#include "threads.h"
#include "server.h"

int main() {
	count = 10;
	base_port = 8000;

	struct ThreadData* data = initializeThreadData();

	int server;
	int port = 8080;

	setupServerSocket(&server, port);

	serverLoop(server, data);

	free(data);
}
