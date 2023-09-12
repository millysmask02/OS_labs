#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

void exitHandler() {
	fprintf(stdout, "=== Call of exit handler ===\n");
	fprintf(stdout, "Exit from %d\n", getpid());
}

void sigIntHandler(int signum) {
	fprintf(stdout, "=== %d SIGINT caught ===\n", getpid());
	fprintf(stdout, "%d (%s)\n", signum, strsignal(signum));
}

void sigTermHandler(int signum) {
	fprintf(stdout, "=== %d SIGTERM caught ===\n", getpid());
	fprintf(stdout, "%d (%s)\n", signum, strsignal(signum));
}


int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	printf("[CURRENT]: PID = %6d; PPID = %6d\n", getpid(), getppid());

	if (atexit(exitHandler)) {
		fprintf(stderr, "Set atexit error\n");
		exit(-2);
	}
	
	signal(SIGINT, sigIntHandler);


	struct sigaction sigTermAct; 
	sigTermAct.sa_handler = sigTermHandler; 
	
	sigemptyset(&sigTermAct.sa_mask);
    
	sigTermAct.sa_flags = SA_RESTART; // sa_flags is a catch-all for special features
	sigaction(SIGTERM, &sigTermAct, NULL); // hooking sigaction to the proper signal


	fprintf(stdout, "Forked\n");
	pid_t pid = fork();
	switch(pid) {
		case -1:
			fprintf(stderr, "Fork error: %d", errno);
			exit(-1);
		case 0:
			fprintf(stdout, "[CHILD]: PID = %6d; PPID = %6d\n", getpid(), getppid());
			sleep(30);
			break;
		default:
			fprintf(stdout, "[PARENT]: PID = %6d; PPID = %6d\n", getpid(), getppid());
			sleep(30);
			break;
	}
	return 0;
}
