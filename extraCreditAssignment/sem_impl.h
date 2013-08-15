#ifndef sem_impl_h
#define sem_impl_h
#include <pthread.h>

pthread_mutex_t semMutex;
pthread_cond_t semCond;
inline int initialized;
initialized = 0;

inline void initialize()
{
  pthread_mutex_init(&semMutex, NULL);
  pthread_cond_init(&semCond, NULL);
  initialized = 1;
}


// NOTE:  This semaphore implementation is INCOMPLETE!  Read the comments.
  
typedef unsigned int semaphore_t;

/* Don't get in a habit of declaring inline functions! As a general
   rule, header files (.h) should NOT contain code --- only #defines,
   structures, and function prototypes */

inline void down(semaphore_t *s)
{
  if (!initialized) initialize();

  // NOTE:  These steps NEED to be done atomically (i.e. use mutex)
  pthread_mutex_lock(&semMutex);
  if (*s == 0)
    pthread_cond_wait(&semCond, &semMutex);  // block until *s > 0  (NEED condition variable)
  (*s)--;
  pthread_mutex_unlock(&semMutex);
}

inline void up(semaphore_t *s)
{
  if (!initialized) initialize();

  // NOTE:  This step NEEDS to be done atomically (i.e. use mutex)
  pthread_mutex_lock(&semMutex);
  (*s)++;
  pthread_cond_signal(&semCond);
  pthread_mutex_unlock(&semMutex);
}

#endif
