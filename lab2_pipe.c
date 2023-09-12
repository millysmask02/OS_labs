#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


int main(int argc, char** argv) {
	(void)argc;
	(void)argv;
		
	int pipedes[2];
	pid_t pid;
	pipe(pipedes);

	struct timespec ts;
	
	// pid = fork();
	if ((pid = fork()) > 0) {
		clock_gettime(CLOCK_REALTIME, &ts);
		struct tm* curr = localtime(&ts.tv_sec);
		double sec_ns = (double)curr->tm_sec + ((double)ts.tv_nsec / 1000000000.);
		char str[1024];
		
		sprintf(str, "[PARENT] {%2d:%2d:%.3lf} pid = %d", curr->tm_hour, curr->tm_min, sec_ns, getpid());
		close(pipedes[0]);
		write(pipedes[1], (void*) str, strlen(str) + 1);
		close(pipedes[1]);
	}
	else {
		sleep(5);
		clock_gettime(CLOCK_REALTIME, &ts);
		struct tm* curr = localtime(&ts.tv_sec);
		double sec_ns = (double)curr->tm_sec + ((double)ts.tv_nsec / 1000000000.);
		char buf[1024];
		int len;
		close(pipedes[1]);
		printf("[CHILD] {%2d:%2d:%.3lf}: ", curr->tm_hour, curr->tm_min, sec_ns);
		if ((len = read(pipedes[0], buf, 1024)) != 0) {
			printf("%s\n", buf);
		}
		close(pipedes[0]);
	}
	return 0;
}
