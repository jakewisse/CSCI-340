/**
 *	Jake Wisse
 *	CSCI 340
 *	4 February 2013
 *
 * 	handle.c -	A simple program to override the default response for SIGINT,
 * 				printing, "Nice try," instead of terminating.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "util.h"


/*
 * First, print out the process ID of this process.
 *
 * Then, set up the signal handler so that ^C causes
 * the program to print "Nice try.\n" and continue looping.
 *
 * Finally, loop forever, printing "Still here\n" once every
 * second.
 */

/**
 *	Signal Handler to print out "Nice try." when current process receives a
 *	SIGINT.
 */	
void sigintHandler() {

	const int STDOUT = 1;
	ssize_t bytes;
	bytes = write(STDOUT, "Nice try.\n", 10);
	if (bytes != 10)
	exit(-999);
}

void sigusr1Handler() {

	ssize_t bytes;
	const int STDOUT = 1;
	bytes = write(STDOUT, "Exiting.\n", 9);
	if(bytes != 9)
		exit(-999);
	exit(1);
}

int main(int argc, char **argv)
{
	struct timespec req, rem;   // needed for nanosleep()
	req.tv_sec = 1;
	req.tv_nsec = 0;

	// Register the signal handlers
	signal(SIGINT, sigintHandler);
	signal(SIGUSR1, sigusr1Handler);

	printf("%d\n", getpid());

	while(1) {


	    // nanosleep returns -1 if interrupted
	    if (nanosleep(&req, &rem) == -1) {
	      // I got interrupted --- wait remaining fraction of a second next time
	      req.tv_sec = rem.tv_sec;
	      req.tv_nsec = rem.tv_nsec;
	    }
		else {
			ssize_t bytes;
			const int STDOUT = 1;
			bytes = write(STDOUT, "Still here...\n", 14);
			if(bytes != 14)
				exit(-999);
			// one second and NO nanoseconds
			req.tv_sec = 1;
			req.tv_nsec = 0;
	    }


	}

  return 0;
}






