/**
 * ULT.c
 *
 * Implementation of a user-level thread library.
 *
 * Created by Jake Wisse.
 * 
 */

#include <assert.h>
#include <stdlib.h>


/* We want the extra information from these definitions */
#ifndef __USE_GNU
#define __USE_GNU
#endif /* __USE_GNU */
#include <ucontext.h>

#include "ULT.h"

int initialized = 0;
Tid currentThread;
Tid zombie = -1;
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

	// Initialize arrays.
	int i;
	for (i = 0; i < ULT_MAX_THREADS; i++) threads[i] = NULL;
	for (i = 0; i < ULT_MAX_THREADS; i++) readyQueue[i] = NULL;

	// Allocate memory for the ThrdCtlBlks and associated ucontext_ts.
	threads[0] = malloc(sizeof(struct ThrdCtlBlk));
	if (!threads[0]) return ULT_NOMEMORY;
	
	threads[0]->tid = 0;
	threads[0]->ucp = malloc(sizeof(ucontext_t));
	if (!threads[0]->ucp) return ULT_NOMEMORY;
	
	// Create a context for the initial thread, allocate space for the stack, 
	// and change the stack pointer and stack size.
	getcontext(threads[0]->ucp);

	threads[0]->ucp->uc_stack.ss_sp = malloc(ULT_MIN_STACK);
	if (!threads[0]->ucp->uc_stack.ss_sp) return ULT_NOMEMORY;
	threads[0]->ucp->uc_stack.ss_size = ULT_MIN_STACK;
	currentThread = 0;
	return 0;
}


Tid ULT_CreateThread(void (*fn)(void *), void *parg)
{
	if (!initialized) ULT_init();

	// Find an unused Tid
	Tid new;
	if ((new = findTid()) <  0) return ULT_NOMORE;

	// Allocate memory for the TCB and ucontext_t
	threads[new] = malloc(sizeof(struct ThrdCtlBlk));
	if (!threads[0]) return ULT_NOMEMORY;
	threads[new]->tid = new;
	threads[new]->ucp = malloc(sizeof(ucontext_t));
	if (!threads[new]->ucp) return ULT_NOMEMORY;

	getcontext(threads[new]->ucp);

	threads[new]->ucp->uc_stack.ss_sp = malloc(ULT_MIN_STACK);
	if (!threads[new]->ucp->uc_stack.ss_sp) return ULT_NOMEMORY;
	threads[new]->ucp->uc_stack.ss_size = ULT_MIN_STACK;

	// Add the newly created thread to the readyQueue.
	enqueue(readyQueue, threads[new]);

	// The makecontext() call.  To be replaced.
	makecontext(threads[new]->ucp, (void (*)(void))&Stub, 2, fn, parg);

	// threads[new]->ucp->uc_stack.ss_sp -= sizeof(char *);
	// *(char *)threads[new]->ucp->uc_stack.ss_sp = (char *)parg;
	// threads[new]->ucp->uc_stack.ss_sp -= sizeof(char *);
	// *(char *)threads[new]->ucp->uc_stack.ss_sp = (char *)fn;

	// Changing the EIP to point to the stub function.
	// threads[new]->ucp->uc_mcontext.gregs[REG_EIP] = 0x8049d0f;

	// Address of Stub(): 0x8049d0f

	return new;
}


Tid ULT_Yield(Tid wantTid)
{
	volatile int done;

	if (!initialized) ULT_init();

	// Taking care of defined constants and possible wantTid errors
	if (wantTid == ULT_ANY) {
		if (readyQueue[0]) wantTid = readyQueue[0]->tid;
		else return ULT_NONE;
	}
	else if (wantTid == ULT_SELF) wantTid = currentThread;
	else if (wantTid < 0 || wantTid >= ULT_MAX_THREADS) return ULT_INVALID;
	else if (!threads[wantTid]) return ULT_INVALID;
		

	// Add caller to the ready queue
	enqueue(readyQueue, threads[currentThread]);

	// Yes, this done flag solution is klugey, but incrementing the $pc was pretty unpredictable.
	done = 0;
	getcontext(threads[currentThread]->ucp);
	if(!done) {
		done = 1;
		if (!dequeue(readyQueue, wantTid)) return ULT_INVALID;
		currentThread = wantTid;
		setcontext(threads[wantTid]->ucp);
	}

	// Destroying the zombie thread left by a thread that called ULT_DestroyThread()
	// on itself.
	if (zombie >= 0) ULT_DestroyThread(zombie);
	
	return wantTid;
}


/**
 * ULT_DestroyThread - Destroys the thread identified by Tid tid.
 *
 * Two possibilities: (1) The thread to be destroyed is an inactive thread that is on the ready queue.  Easy.
 * (2) The thread to be destroyed is the current thread, and we must identify ourselves as a zombie and then
 * yield to any and have them destroy us, or there are no other ready threads and we free our memory and exit.
 * 
 * @param  tid [Thread to be destroyed]
 * @return     [tid, the thread that was successfully destroyed.  Or, ULT_NONE in response to tid being ULT_ANY,
 *              or ULT_INVAlID if tid doesn't correspond to an existing thread.]
 */
Tid ULT_DestroyThread(Tid tid)
{
	// Suicide case
	if (tid == ULT_SELF || tid == currentThread) {
		tid = currentThread;
		
		// There exists another ready thread to yield to and have kill this one.
		if (readyQueue[0]) {
			zombie = tid;
			ULT_Yield(ULT_ANY);
		}

		// Last available thread, so we backtrack how we created it and then exit.
		else {
			free(threads[tid]->ucp->uc_stack.ss_sp);
			free(threads[tid]->ucp);
			free(threads[tid]);
			threads[tid] = NULL;
			exit(0);
		}
	}

	else {
		// Take care of the ULT_ANY constant.
		if (tid == ULT_ANY) {
			if (!readyQueue[0]) return ULT_NONE;
			tid = readyQueue[0]->tid;
		}

		// Remove the thread from the readyQueue.
		if (!dequeue(readyQueue, tid)) return ULT_INVALID;

		// Deallocate all the memory dedicated to the thread and set its pointer in threads[] to NULL.
		free(threads[tid]->ucp->uc_stack.ss_sp);
		free(threads[tid]->ucp);
		free(threads[tid]);
		threads[tid] = NULL;
	}

	return tid;
}


void Stub(void (*fn)(void *), void *arg){

  Tid ret;
  if (zombie >= 0) ULT_DestroyThread(zombie);
  fn(arg); // call root function
  ret = ULT_DestroyThread(ULT_SELF);
  assert(ret == ULT_NONE); // we should only get here if we are the last thread.
  exit(0); // all threads are done, so process should exit 
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


ThrdCtlBlk* dequeue(ThrdCtlBlk** queue, Tid tid)
{
	int i;
	ThrdCtlBlk* ret;
	if (!queue[0]) return NULL;

	// Find tid.
	i = 0;
	while (queue[i] && i < ULT_MAX_THREADS) {
		if (queue[i]->tid == tid) break;
		i++;
	}

	// tid was not found.
	if (i == ULT_MAX_THREADS) return NULL;
	
	ret = queue[i];

	// Shift pointers
	queue[i] = NULL;
	i++;
	for (; i < ULT_MAX_THREADS; i++) {
		queue[i-1] = queue[i];
		queue[i] = NULL;
	}

	return ret;
}


Tid findTid()
{
	int i;
	for (i = 0; (threads[i]) && i < ULT_MAX_THREADS; i++);
	if (i == ULT_MAX_THREADS) return ULT_NOMORE;
	else return i;
}
