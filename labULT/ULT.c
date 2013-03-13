#include <assert.h>

/* We want the extra information from these definitions */
#ifndef __USE_GNU
#define __USE_GNU
#endif /* __USE_GNU */
#include <ucontext.h>

#include "ULT.h"

static int ULT_initialized = 0;
static ThrdCtlBlk TCB;

/**
 * Always called by the implicitly created 1st thread, exactly once, when the first library call is made.
 * Saves the current context in contextArray[0] and changes the flag ULT_initialized to 1.
 * 
 * @return 0 if success, 1 if error.
 */
int ULT_Init()
{
	TCB.contextArray[0] = malloc(sizeof(ucontext_t));
	getcontext(TCB.contextArray[0]);
	ULT_initialized = 1;  // ULT lib has been initialized.
}




Tid ULT_CreateThread(void (*fn)(void *), void *parg)
{
	if (!ULT_initialized) ULT_Init();

	// Stepping through the array to find the first vacant element of contextArray[].
	// If we reach the end of the array, ULT_NOMORE is returned.
	int i;
	for(i = 0; (TCB.contextArray[i]) || i < ULT_MAX_THREADS; i++);
	if (i >= ULT_MAX_THREADS) return ULT_NOMORE;

	// Allocate space for the new threads stack.  If the malloc fails, ULT_NOMEMORY is returned.
	void* stack;
	if (!(stack = malloc(ULT_MIN_STACK))) return ULT_NOMEMORY;

	// Allocate memory for the new element in contextArray[] and copy the current context
	// to it.
	TCB.contextArray[i] = malloc(sizeof(ucontext_t));
	getcontext(TCB.contextArray[i]);
	
	// The makecontext() portion.  Editing the fields of the ucontext_t struct pointed at by
	// contextArray[i].

	TCB.contextArray[i]->uc_mcontext.gregs[REG_EIP] = fn;	// Change the instruction pointer
	
}



Tid ULT_Yield(Tid wantTid)
{
	if (!ULT_initialized) ULT_Init();

	assert(0); /* TBD */
	return ULT_FAILED;

}


Tid ULT_DestroyThread(Tid tid)
{
  assert(0); /* TBD */
  return ULT_FAILED;
}





