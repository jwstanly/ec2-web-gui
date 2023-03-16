struct ThreadData* initializeThreadData() {
	struct ThreadData* data = malloc(count * sizeof(struct ThreadData));
	for(int i = 0; i < count; i++) {
		data[i].ip = 0;
		data[i].counter = 0;
		data[i].lastClaimTime = 0;
		pthread_mutex_init(&(data[i].mutex), NULL);
	}

	return data;
}

void* fromVNCHandler(void* voidData) {
	struct ThreadStarter* ts = (struct ThreadStarter*)voidData;
	char* buffer = malloc(TRANSFER_BUFFER_SIZE);

	int len;
	while(1) {
		len = read(ts->vncfd, buffer, TRANSFER_BUFFER_SIZE);
		len = write(ts->browserfd, buffer, len);
		if(len == -1) break;
	}

	free(buffer);
	return NULL;
}

void* toVNCHandler(void* voidData) {
	struct ThreadStarter* ts = (struct ThreadStarter*)voidData;

	//Launch other thread
	pthread_t fromVNCThread;
	pthread_create(&fromVNCThread, NULL, &fromVNCHandler, &ts);

	char* buffer = malloc(TRANSFER_BUFFER_SIZE);

	int len;
	while(1) {
		len = read(ts->browserfd, buffer, TRANSFER_BUFFER_SIZE);
		if(len == -1) break;
		write(ts->vncfd, buffer, len);
		write(STDOUT_FILENO, buffer, len);
	}

	pthread_join(fromVNCThread, NULL);

	returnAddr(ts->data, ts->ip);

	free(buffer);
	free(ts); //This clear is not thread safe, but will probably not be an issue...

	return NULL;
}
