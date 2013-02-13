/* 
 * psh - A prototype tiny shell program with job control
 * 
 * Jake Wisse <jacob.wisse@stono.cs.cofc.edu>
 * SID: 20030939
 * CSCI 340
 * 4 February 2013
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "util.h"


/* Global variables */
int verbose = 0;            /* if true, print additional output */

extern char **environ;      /* defined in libc */
static char prompt[] = "psh> ";    /* command line prompt (DO NOT CHANGE) */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);

/* Here are helper routines that we've provided for you */
void usage(void);
void sigquit_handler(int sig);



/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }


    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/

void eval(char *cmdline) 
{

    /**
     * Parse the command line into a two dim. array,
     * delimited by spaces.
     */
    
    char *itr;
    itr = cmdline;
    char **argv;
    int argSize;
    argSize = 0;
    int argNum;
    argNum = 0;
    int i;

    argv = (char **) malloc(MAXARGS * sizeof(char *));
    while (*itr != '\n') {

        // Find the length of the first argument
        while (*itr != ' ' && *itr != '\n') {
            argSize++;
            itr++;
        }

        // Allocate memory for the argument plus a null character
        argv[argNum] = (char *) malloc((argSize+1) * sizeof(char)); 
        
        // Move itr to point back to the beginning of the argument
        itr -= argSize; 

        // Copying the characters of the argument into the argv[argNum] array.
        for (i = 0; i<argSize ;i++) {
            argv[argNum][i] = *itr;
            itr++;
        }

        // Appending a null terminating character
        argv[argNum][i] = '\0';

        argSize = 0;
        argNum++;
        if (*itr != '\n') itr++;

    }
    
    // Adding a null pointer as the last element of argv 
    argv[argNum] = NULL;
    
    // Calling builtin_cmd() to check to see if the user has entered 'quit',
    // the only builtin command at this point.
    builtin_cmd(argv);

    int pid;
    int status;

    pid = fork();

    // Parent process code
    if (pid > 0) {
        wait(&status);

        // Deallocating the argv data structure, but still utilizing the
	// argNum variable in a for loop, rather than a while(argv[somePointer])
	// loop.
        for (i=0; i<argNum; i++) {
            free(argv[argNum]);
        }
        free(argv);
    }

    // Child process code.  Exits the forked process if execv fails.
    else if (execv(argv[0], &argv[0])) exit(2);
}


/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately. 
 * Return 1 if a builtin command was executed; return 0
 * if the argument passed in is *not* a builtin command.
 */

/**
 * Note: argument was **argv, changed to just a one dim. array
 */

int builtin_cmd(char **argv) 
{
    if (!strcmp(argv[0], "quit")) {
        kill(getpid(), SIGQUIT);
	return 1;
    }
    else return 0;     /* not a builtin command */
}





/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}



