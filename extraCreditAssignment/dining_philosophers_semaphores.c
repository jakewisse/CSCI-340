#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#include "sem_impl.h"

#define N         5               // number of philosophers
#define LEFT(i)   (((i)+N-1)%N)   // formula for philosopher to left
#define RIGHT(i)  (((i)+1)%N)     // formula for philosopher to left

#define DURATION 10               // run for DURATION seconds

typedef enum { THINKING, HUNGRY, EATING } state_t;  // three possible states

semaphore_t mutex;
semaphore_t s[N];
state_t state[N];

int done;          // when set to true, all threads will quit
int eatcount[N];   // count how many times each philosopher got to eat

// forward declare the following functions so we can first focus on main()
void philosopher(int *);   // philosopher thread code
void think(int i);         // what philosopher does when thinking
void take_forks(int i);    // try to pick up BOTH forks
void eat(int i);           // what philosopher does when eating
void put_forks(int i);     // put down both forks
void test(int i);
void mysleep(int milliseconds);  // sleep for milliseconds

int main(int argc, char **argv)
{
  int i;
  pthread_t thread[N];        // each of the N philosophers

  done = 0;
  mutex = 1;                  // normal initialization of mutex
  for (i = 0; i < N; i++) {
    s[i] = 0;                 // initially all philosopher sems will block
    eatcount[i] = 0;
  }

  for (i = 0; i < N; i++) {   // create N philosopher threads
    if (pthread_create(&thread[i], NULL, (void *) philosopher, (void *) i)
	!= 0) {
      perror("pthread_create");
      return 1;
    }
  }

  mysleep(DURATION*1000);     // sleep for DURATION seconds

  done = 1;                   // have everybody stop

  for (i = 0; i < N; i++) {   // wait for each philosopher to to finish
    if (pthread_join(thread[i], NULL) != 0) {  // NOTE:  in order 0..N-1
      perror("pthread_join");
      return 2;
    }
  }

  printf("\n");
  for (i = 0; i < N; i++) {
    printf("Philosopher %d got to eat %d times\n", i, eatcount[i]);
  }

  return 0;
}

void philosopher(int *pi)   // pi is philosopher number casted to a pointer
{
  int i = (int) pi;

  printf("philosopher thread %d starting...\n", i);

  while (!done) {
    think(i);                      // philosopher is thinking...
    take_forks(i);                 // acquire two forks or block
    eat(i);                        // yum, yum --- spaghetti!
    put_forks(i);                  // put both forks back on table
  }
}

void take_forks(int i)  // i:  philosopher number
{
  down(&mutex);         // enter critical region
  state[i] = HUNGRY;    // record fact that philosopher, i, is hungry
  test(i);              // try to acquire 2 forks
  up(&mutex);           // exit critical region
  down(&s[i]);          // block if forks were not acquired
}

void put_forks(int i)   // i:  philosopher number
{
  down(&mutex);         // enter critical region
  state[i] = THINKING;  // philosopher has finished eating
  test(LEFT(i));        // see if left neighbor can now eat
  test(RIGHT(i));       // see if right neighbor can now eat
  up(&mutex);           // exit critical region
}

void test(int i)
{
  if (state[i] == HUNGRY &&
      state[LEFT(i)] != EATING && state[RIGHT(i)] != EATING) {
    state[i] = EATING;
    up(&s[i]);
  }
}

void think(int i)
{
  int milliseconds = random() % 1000;

  printf("philosopher %d is thinking for %d milliseconds...\n", i,
	 milliseconds);

  mysleep(milliseconds);
}

void eat(int i)
{
  int milliseconds = random() % 1000;

  eatcount[i]++;

  printf("\tphilosopher %d is eating for %d milliseconds...\n", i,
	 milliseconds);

  mysleep(milliseconds);
}

void mysleep(int milliseconds)
{
  struct timespec ts;

  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000;

  nanosleep(&ts, NULL);
}
