#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#define SHM_NAME "shmem_file"
#define SHM_SIZE 64

bool g_exitFlag = false;

void signalHandler() {
	g_exitFlag = true;
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);

	struct timespec ts;
	struct sembuf setSem = {0, 1, 0};
	struct stat buffer;
	FILE* file;

	if (stat(SHM_NAME, &buffer) != 0) {
		if (!(file = fopen(SHM_NAME, "w"))) {
			fprintf(stderr, "Shared memory file cannot be created: %s(%d)\n", strerror(errno), errno);
			exit(1);
		}
		fclose(file);
	}
	
	key_t key = ftok(SHM_NAME, 1);
	
	int shmid = shmget(key, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0666);
	if (shmid == -1 && errno == EEXIST) {
		fprintf(stderr, "The program is already running: %s(%d)", strerror(errno), errno);
		exit(1);
	}
	if (shmid == -1) {
		fprintf(stderr, "shmget failed: %s(%d)\n", strerror(errno), errno);
		exit(1);
	}
	
	int semid = semget(key, 1, IPC_CREAT | 0666);
	if (semid == -1) {
		fprintf(stderr, "semget failed: %s(%d)\n", strerror(errno), errno);
		exit(1);
	}
	
	printf("[first] key: %d\n[first] mem_id: %d\n[first] sem_id: %d\n", key, shmid, semid);

	char* shm_ptr = shmat(shmid, NULL, 0);
	printf("[first] shm_ptr: %p\n", shm_ptr);

	semctl(semid, 0, SETVAL, (int)0); // init semaphore value with 0
	
	while(!g_exitFlag) {
		char str[64];
		clock_gettime(CLOCK_REALTIME, &ts);
		struct tm* curr = localtime(&ts.tv_sec);
		double sec_ns = (double)curr->tm_sec + ((double)ts.tv_nsec / 1000000000.);
		sprintf(str, "{%2d:%2d:%.3lf} pid = %d", curr->tm_hour, curr->tm_min, sec_ns, getpid());
		
		/* 
			at this point semaphore is open for prog1 process (semaphore value = 0), 
			so we can access shared memory
		*/
		strcpy(shm_ptr, str);
		// sleep(5);
		
		/*
			allowing prog2 process to access critical section
			by setting sem_op = 1
		*/
		int res = semop(semid, &setSem, 1);
		if (res == -1) {
			int err = errno;
			printf("[first] semop: %s(%d)\n", strerror(err), err);
		}
		sleep(1);

		/* 
			waiting for semaphore to be opened 
			(when semaphore value will be reset to 0) 
			for prog1 process for next iteration
		*/
	}

	shmdt(shm_ptr);
	shmctl(shmid, IPC_RMID, NULL);
	semctl(semid, 0, IPC_RMID);

	printf("[first] Destroying the shared memory segment and semaphore\n");
	remove(SHM_NAME);
	
	return 0;
}
