/* 
 * tsh - A tiny shell program with job control
 * 
 * <Put your name and login ID here>
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
#include "jobs.h"


/* Global variables */
int verbose = 0;            /* if true, print additional output */

extern char **environ;      /* defined in libc */
static char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
static struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);   // calls vvvv
void do_bgfg(char **argv);      // calls vvvv
void waitfg(pid_t pid);         // looping doing a waitpid, WNOHANG.  Use a nanosleep to wait a reasonable
                                // amount of time.

void sigchld_handler(int sig);  // We could loop through the jobs and check if they've exited...
void sigtstp_handler(int sig);
void sigint_handler(int sig);

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

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

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
    
    /**
     * Calling builtin_cmd() to check to see if the user has entered a builtin
     * command.  If so, the command will be executed within the function.
     */
    
    if (!builtin_cmd(argv)) {

        int pid;
        int status;
        int isbg;       // 1 if bg, 0 if fg

        // Checking to see if the last argument is an & and setting isfg accordingly.
        // In the case that the last arg is an &, the \0 char is freed and the & is
        // replaced with a '\0'.
        if (!strcmp(argv[argNum - 1], "&")) {
            isbg = 1;
            free(argv[argNum]);
            argv[argNum - 1] = '\0';
            argNum--;
        }
        else isbg = 0;

        // Blocking SIGCHLD with sigprocmask().
        sigset_t s;
        sigemptyset(&s);
        sigaddset(&s, SIGCHLD);
        sigprocmask(SIG_BLOCK, &s, NULL);

        pid = fork();

        // Adding the new process to the job list with addjob(), only in the parent
        // process (i.e. the shell).  Also printing the job with new function printjob()
        // to match rtest04.
        if (pid > 0 && isbg) {
            addjob(jobs, pid, BG, cmdline);
            printjob(jobs, pid);
        }


        // Unblocking SIGCHLD, from BOTH parent and child.
        sigprocmask(SIG_UNBLOCK, &s, NULL);

        // Parent process code
        if (pid > 0) {
            if (isbg) waitpid(pid, &status, WNOHANG);
            else waitpid(pid, &status, 0);

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
    
}


/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 * Return 1 if a builtin command was executed; return 0
 * if the argument passed in is *not* a builtin command.
 */
int builtin_cmd(char **argv) 
{
    if (!strcmp(argv[0], "quit")) {
        kill(getpid(), SIGQUIT);
    return 1;

    }
    else if (strcmp(argv[0], "fg") == 0 || strcmp(argv[0], "bg") == 0) {
        do_bgfg(argv);
        return 1;
    }

    else if (!strcmp(argv[0], "jobs")) {
        listjobs(jobs);
        return 1;
    }

    else return 0;     /* not a builtin command */
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv) 
{
    // Determine whether the command to execute is fg or bg.

    return;
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
    return;
}

/*****************
 * Signal handlers
 *****************/

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.  
 */
void sigchld_handler(int sig) 
{
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    if (fgpid(jobs))
        kill(-(fgpid(jobs)), sig);   
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
    if (fgpid(jobs))
        kill(-(fgpid(jobs)), sig);    
}

/*********************
 * End signal handlers
 *********************/



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



