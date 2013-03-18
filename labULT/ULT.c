#include <assert.h>
#include <stdlib.h>


/* We want the extra information from these definitions */
#ifndef __USE_GNU
#define __USE_GNU
#endif /* __USE_GNU */
#include <ucontext.h>

#include "ULT.h"

int initialized = 0;
int currentThread;
ThrdCtlBlk* threads[ULT_MAX_THREADS];
ThrdCtlBlk* readyQueue[ULT_MAX_THREADS];


/**
 * Always called by the implicitly created 1st thread, exactly once, when the first library call is made.
 * First initializes all the pointers in the threads array to be NULL, then saves the current context in
 * threads[0] and changes initialized to 1.
 * 
 * @return 0 if success, applicable constant if error.
 */
int ULT_init()
{
	initialized = 1;

	// Initialize arrays
	int i;
	for (i = 0; i < ULT_MAX_THREADS; i++) threads[i] = NULL;
	for (i = 0; i < ULT_MAX_THREADS; i++) readyQueue[i] = NULL;

	// Allocate memory for the ThrdCtlBlks and associated ucontext_ts
	threads[0] = malloc(sizeof(struct ThrdCtlBlk));
	if (!threads[0]) return ULT_NOMEMORY;
	
	threads[0]->tid = 0;
	threads[0]->ucp = malloc(sizeof(ucontext_t));
	if (!threads[0]->ucp) return ULT_NOMEMORY;
	
	// Create a context for the initial thread, changing the stack pointer and stack size
	getcontext(threads[0]->ucp);

	threads[0]->ucp->uc_stack.ss_sp = malloc(ULT_MIN_STACK);
	threads[0]->ucp->uc_stack.ss_size = ULT_MIN_STACK;
	currentThread = 0;
	return 0;
}




Tid ULT_CreateThread(void (*fn)(void *), void *parg)
{
	if (!initialized) ULT_init();


	

}



Tid ULT_Yield(Tid wantTid)
{
	if (!initialized) ULT_init();

	// Taking care of defined constants
	if (wantTid == ULT_ANY) {
		if (readyQueue[0]) wantTid = readyQueue[0]->tid;
		else return ULT_NONE;
	}

	else if (wantTid == ULT_SELF) wantTid = currentThread;

	// No such thread with Tid wantTid.
	else if (wantTid < 0 || wantTid >= ULT_MAX_THREADS) return ULT_INVALID;
	else if (!threads[wantTid]) return ULT_INVALID;

	
	volatile int dontRepeat = 0;
	

	// Add caller to the ready queue
	enqueue(readyQueue, threads[currentThread]);

	// Save the caller's context, only if 
	//getcontext(threads[currentThread]->ucp);


	getcontext(threads[currentThread]->ucp);

	if (!dontRepeat) {

		dontRepeat = 1;
		// Save a pointer to the tcb of the thread we're yielding to so that we can switch
		// contexts and then later return its tid.

		ThrdCtlBlk* temp;
		temp = dequeue(readyQueue);
		if(!temp) return ULT_INVALID;
		// Update currentThread
		currentThread = wantTid;

		setcontext(temp->ucp);	

	}

	return wantTid;
}


Tid ULT_DestroyThread(Tid tid)
{
  assert(0); /* TBD */
  return ULT_FAILED;
}

/**
 * Helper Functions
 */

int enqueue(ThrdCtlBlk** queue, ThrdCtlBlk* tcb)
{


	int i;
	for (i = 0; (queue[i]) && i < ULT_MAX_THREADS; i++);
	if (i == ULT_MAX_THREADS) return -1;
	queue[i] = tcb;
	return i;
}

ThrdCtlBlk* dequeue(ThrdCtlBlk** queue)
{
	int i;
	ThrdCtlBlk* ret;
	ret = queue[0];
	queue[0] = NULL;

	for (i = 1; i < ULT_MAX_THREADS && (queue[i]); i++) {
		queue[i-1] = queue[i];
	}
	if (i != ULT_MAX_THREADS) queue[i] = NULL;

	return ret;
}




