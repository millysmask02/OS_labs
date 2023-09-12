#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>

pthread_rwlock_t g_rwlock;
char arrCounter[] = {'0', '\0'};

void* writer(void* args) {
	(void)args;

	while (1) {
		pthread_rwlock_wrlock(&g_rwlock);
		printf("\n[writer] RWLock locked\n");
		
		printf("[writer] writing to array...\n");
		arrCounter[0] += 1;
		sleep(1);
		
		pthread_rwlock_unlock(&g_rwlock);
		printf("[writer] RWLock unlocked\n");
		sleep(1);
	}

	pthread_exit(0);
}

void* reader(void* args) {
	(void)args;

	while (1) {
		pthread_rwlock_rdlock(&g_rwlock);
		printf("\n[reader] %lx RWLock locked\n", pthread_self());
		
		
		printf("[reader] %lx reading array: %s\n", pthread_self(), arrCounter);
		sleep(1);
				
		pthread_rwlock_unlock(&g_rwlock);
		printf("[reader] %lx RWLock unlocked\n", pthread_self());
		sleep(1);
	}

	pthread_exit(0);
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	pthread_rwlock_init(&g_rwlock, NULL);
	int threadsCount = 11;

	pthread_t threads[threadsCount];
	pthread_create(&threads[0], NULL, writer, NULL);
	for (int i = 1; i < threadsCount; ++i) {
		pthread_create(&threads[i], NULL, reader, NULL);
	}

	for (int i = 0; i < threadsCount; ++i) {
		pthread_join(threads[i], NULL);
	}

	pthread_rwlock_destroy(&g_rwlock);
	return 0;
}
