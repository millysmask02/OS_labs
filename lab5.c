#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

pthread_mutex_t g_mtx;
char arrCounter[] = {'0', '\0'};

void* writer(void* args) {
	(void)args;

	while (1) {
		pthread_mutex_lock(&g_mtx);
		printf("\n[writer] Mutex locked\n");
		
		printf("[writer] writing to array...\n");
		arrCounter[0] += 1;
		sleep(1);
		
		pthread_mutex_unlock(&g_mtx);
		printf("[writer] Mutex unlocked\n");
		sleep(5);
	}

	pthread_exit(0);
}

void* reader(void* args) {
	(void)args;

	while (1) {
		pthread_mutex_lock(&g_mtx);
		printf("\n[reader] %lx Mutex locked\n", pthread_self());
		
		
		printf("[reader] %lx reading array: %s\n", pthread_self(), arrCounter);
		sleep(1);
				
		pthread_mutex_unlock(&g_mtx);
		printf("[reader] %lx Mutex unlocked\n", pthread_self());
		sleep(5);
	}

	pthread_exit(0);
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	pthread_mutex_init(&g_mtx, NULL);
	int threadsCount = 11;

	pthread_t threads[threadsCount];
	pthread_create(&threads[0], NULL, writer, NULL);
	for (int i = 1; i < threadsCount; ++i) {
		pthread_create(&threads[i], NULL, reader, NULL);
	}

	for (int i = 0; i < threadsCount; ++i) {
		pthread_join(threads[i], NULL);
	}

	pthread_mutex_destroy(&g_mtx);
	return 0;
}
