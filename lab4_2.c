#include <stdio.h>
#include <stdlib.h>
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

#define SHM_NAME "shmem_file"
#define SHM_SIZE 64


int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	struct timespec ts;
	struct sembuf unsetSem = {0, -1, 0};
	struct stat buffer;

	if (stat(SHM_NAME, &buffer) != 0) {
		fprintf(stderr, "Cannot find shared memory file. Please run prog1 executable file first.\n");
		exit(1);
	}
		
	key_t key = ftok(SHM_NAME, 1);

	int semid = semget(key, 1, IPC_CREAT | 0666);
	if (semid == -1) {
		fprintf(stderr, "semget failed: %s(%d)\n", strerror(errno), errno);
		exit(1);
	}

	int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
	if (shmid == -1) {
		fprintf(stderr, "shmget failed: %s(%d)\n", strerror(errno), errno);
		exit(1);
	}
	printf("[second] key: %d\n[second] mem_id: %d\n[second] sem_id: %d\n", key, shmid, semid);

	char* shm_ptr = shmat(shmid, NULL, SHM_RDONLY);
	printf("[second] shm_ptr: %p\n", shm_ptr);

	printf("[second] Waiting...\n");
	
	/*
		initially, semaphore value is 0,
		so the prog2 process is blocked

		at the moment when sem_op is 1 in prog1,
		unlock prog2 process
	*/

	clock_gettime(CLOCK_REALTIME, &ts);
	struct tm* curr = localtime(&ts.tv_sec);
	double sec_ns = (double)curr->tm_sec + ((double)ts.tv_nsec / 1000000000.);
	printf("{%2d:%2d:%.3lf} pid = %d reading shmem: %s\n", curr->tm_hour, curr->tm_min, sec_ns, getpid(), shm_ptr);
		
	shmdt(shm_ptr);
	
	int res = semop(semid, &unsetSem, 1);
	if (res == -1) {
		int err = errno;
		printf("[second] semop: %s(%d)\n", strerror(err), err);
	}
	else {
		printf("[second] unset semaphore\n");
	}
	/*
		prog1 process can be unlocked now 
	*/

	return 0;
}
