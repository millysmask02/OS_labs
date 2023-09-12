#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
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
	struct stat buffer;

	if (stat(SHM_NAME, &buffer) != 0) {
		fprintf(stderr, "Cannot find shared memory file. Please run prog1 executable file first.\n");
		exit(1);
	}
		
	key_t shm_key = ftok(SHM_NAME, 1);
	int shmid = shmget(shm_key, SHM_SIZE, IPC_CREAT | 0666);
	if (shmid == -1) {
		fprintf(stderr, "%s(%d)\n", strerror(errno), errno);
		exit(1);
	}
	printf("[second] key: %d\n[second] mem_id: %d\n", shm_key, shmid);

	char* shm_ptr = shmat(shmid, NULL, SHM_RDONLY);
	printf("[second] shm_ptr: %p\n", shm_ptr);

	clock_gettime(CLOCK_REALTIME, &ts);
	struct tm* curr = localtime(&ts.tv_sec);
	double sec_ns = (double)curr->tm_sec + ((double)ts.tv_nsec / 1000000000.);
	printf("{%2d:%2d:%.3lf} pid = %d reading shmem: %s\n", curr->tm_hour, curr->tm_min, sec_ns, getpid(), shm_ptr);
		
	shmdt(shm_ptr);

	// shmctl(shmid, IPC_RMID, NULL);
	
	return 0;
}
