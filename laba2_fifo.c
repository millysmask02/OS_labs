#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <time.h>

int main(int argc, char** argv) {
	(void) argc;
	(void) argv;
	
	int status = mkfifo("./fifo_file", 0777);
	if (status == -1) {
		int error = errno;
		fprintf(stderr, "Error %d: %s", error, strerror(error));
		exit(1);
	}

	int fd;
	struct timespec ts;
	pid_t pid;
	
	if ((pid = fork()) > 0) {
		clock_gettime(CLOCK_REALTIME, &ts);
		struct tm* curr = localtime(&ts.tv_sec);
		double sec_ns = (double)curr->tm_sec + ((double)ts.tv_nsec / 1000000000.);
		char str[1024];
		sprintf(str, "[PARENT] {%2d:%2d:%.3lf} pid = %d", curr->tm_hour, curr->tm_min, sec_ns, getpid());
		
		fd = open("./fifo_file", O_WRONLY);
		write(fd, (void*) str, sizeof(str));
		close(fd);
		unlink("./fifo_file");
	}
	else {
		sleep(5);
		clock_gettime(CLOCK_REALTIME, &ts);
		struct tm* curr = localtime(&ts.tv_sec);
		double sec_ns = (double)curr->tm_sec + ((double)ts.tv_nsec / 1000000000.);
		char buf[1024];
		int len;
		printf("[CHILD] {%2d:%2d:%.3lf}: ", curr->tm_hour, curr->tm_min, sec_ns);
		
		fd = open("./fifo_file", O_RDONLY);
		if ((len = read(fd, buf, 1024)) != 0) {
			printf("%s\n", buf);
		}
		close(fd);
	}
	return 0;
}
