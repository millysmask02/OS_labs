#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

pthread_mutex_t g_mtx;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

char arrCounter[] = {'0', '\0'};

void* writer(void* args) {
	(void)args;

	while (1) {
		pthread_mutex_lock(&g_mtx);
		printf("[writer] Mutex locked\n");

		printf("[writer] writing to array...\n");
		arrCounter[0] += 1;
		
		pthread_cond_broadcast(&g_cond);
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
		printf("[reader] %lx Mutex locked\n", pthread_self());

		printf("[writer] Condition wait\n");
		pthread_cond_wait(&g_cond, &g_mtx);

		printf("[reader] %lx reading array: %s\n", pthread_self(), arrCounter);
				
		pthread_mutex_unlock(&g_mtx);
		printf("[reader] %lx Mutex unlocked\n", pthread_self());
	}

	pthread_exit(0);
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	pthread_mutex_init(&g_mtx, NULL);
	int threadsCount = 11;

	pthread_t threads[threadsCount];
	for (int i = 0; i < threadsCount - 1; ++i) {
		pthread_create(&threads[i], NULL, reader, NULL);
	}
	pthread_create(&threads[threadsCount - 1], NULL, writer, NULL);

	for (int i = 0; i < threadsCount; ++i) {
		pthread_join(threads[i], NULL);
	}

	pthread_cond_destroy(&g_cond);
	pthread_mutex_destroy(&g_mtx);
	return 0;
}
