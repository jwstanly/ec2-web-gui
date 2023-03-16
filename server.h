int claimAddr(struct ThreadData* addrs, unsigned long ip) {
	time_t t = time(NULL);

	//First check if known
	for(int i = 0; i < count; i++) {
		pthread_mutex_lock(&(addrs[i].mutex));
		if(addrs[i].ip == ip) {
			addrs[i].lastClaimTime = t;
			addrs[i].counter++;
			return i;
			// Must unlock mutex outside of function
		}
		pthread_mutex_unlock(&(addrs[i].mutex));
	}

	//Check if available
	for(int i = 0; i < count; i++) {
		pthread_mutex_lock(&(addrs[i].mutex));
		if(addrs[i].ip == 0 && ((t - addrs[i].lastClaimTime) >= TIMEOUT_SEC)) {
			addrs[i].ip = ip;
			addrs[i].lastClaimTime = t;
			addrs[i].counter++;
			return i;
			// Must unlock mutex outside of function
		}
		pthread_mutex_unlock(&(addrs[i].mutex));
	}

	return -1;
}

void returnAddr(struct ThreadData* addrs, unsigned long ip) {
	//First check if known
	for(int i = 0; i < count; i++) {
		int found = 0;
		pthread_mutex_lock(&(addrs[i].mutex));
		if(addrs[i].ip == ip) {
			found = 1;
			addrs[i].counter--;
			if(addrs[i].counter == 0) addrs[i].ip = 0;
		}
		pthread_mutex_unlock(&(addrs[i].mutex));
		if(found == 1) break;
	}
}

void setupServerSocket(int* server, int port) {
	struct sockaddr_in address;
	handleError(*server = socket(AF_INET, SOCK_STREAM, 0), "socket");

	memset(&address, 0, sizeof(address));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);

	handleError(bind(*server, (struct sockaddr*) &address, sizeof(address)), "bind");
	handleError(listen(*server, 20), "listen"); // Maximum of 20 connections
}

void openVNCConnection(int* client, int idx) {
	int port = base_port + idx;
	struct sockaddr_in address;

	handleError(*client = socket(AF_INET, SOCK_STREAM, 0), "client socket");

	memset(&address, 0, sizeof(address));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	address.sin_port = htons(port);

	handleError(connect(*client, (struct sockaddr*) &address, sizeof(address)), "client connect");
}

void serverLoop(int server, struct ThreadData* addrs) {
	struct sockaddr_in clientAddr;
	unsigned int len = sizeof(clientAddr);
	int client;

	while(1) {
		handleError(client = accept(server, (struct sockaddr*) &clientAddr, &len), "accept");
		int thread = claimAddr(addrs, clientAddr.sin_addr.s_addr);

		printf("The thread is: %d\n", thread);

		if(thread == -1) {
			write(client, fullResponse, strlen(fullResponse));
		} else {
			struct ThreadStarter* ts = malloc(sizeof(struct ThreadStarter));
			ts->data = addrs;
			ts->ip = clientAddr.sin_addr.s_addr;
			ts->browserfd = client;
			openVNCConnection(&(ts->vncfd), thread);

			pthread_t toVNCThread;
			pthread_create(&toVNCThread, NULL, &toVNCHandler, &ts);
			//pthread_detach(toVNCThread);
			pthread_join(toVNCThread, NULL);

			pthread_mutex_unlock(&(addrs[thread].mutex));
		}

		close(client);
	}
}
