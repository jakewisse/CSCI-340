#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#define N         5             // number of philosophers
#define LEFT(i)   (((i)+N-1)%N) // formula for philosopher to left
#define RIGHT(i)  (((i)+1)%N)   // formula for philosopher to left

#define DURATION 10             // run for DURATION seconds

typedef enum { THINKING, HUNGRY, EATING } state_t;  // three possible states

pthread_mutex_t mutex;
pthread_cond_t s[N];
state_t state[N];

int done;          // when set to true, all threads will quit
int eatcount[N];   // count how many times each philosopher got to eat

// forward declare the following functions so we can first focus on main()
void philosopher(void *);  // philosopher thread code
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
  pthread_mutex_init(&mutex, NULL);
  for (i = 0; i < N; i++) {
    pthread_cond_init(&s[i], NULL);
    eatcount[i] = 0;
  }

  for (i = 0; i < N; i++) {   // create N philosopher threads
    if (pthread_create(&thread[i], NULL, (void *) philosopher, (void *) i)
	!= 0) {
      perror("pthread_create");
      return 1;
    }
  }

  mysleep(DURATION*100);     // sleep for DURATION seconds

  done = 1;                   // have everybody stop

  for (i = 0; i < N; i++) {   // wait for each philosopher to to finish
    if (pthread_join(thread[i], NULL) != 0) {  // NOTE:  in order 0..N-1
      perror("pthread_join");
      return 2;
    }
  }

  // destroy all mutex
  pthread_mutex_destroy(&mutex);
  for (i = 0; i < N; i++) {
    pthread_cond_destroy(&s[i]);
  }

  printf("\n");
  for (i = 0; i < N; i++) {
    printf("Philosopher %d got to eat %d times\n", i, eatcount[i]);
  }

  return 0;
}

void philosopher(void *pi)   // pi is a philosopher number casted as a pointer
{
  int i = (int) pi;          // convert pointer to an integer

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
  pthread_mutex_lock(&mutex);   // enter critical region
  state[i] = HUNGRY;    // record fact that philosopher, i, is hungry
  test(i);              // try to acquire 2 forks
  while (state[i] != EATING)
    /* From man page of pthread_cond_wait(): "Threads waiting on
       mutexes and condition variables are selected to proceed in an
       order dependent upon the scheduling policy rather than in some
       fixed order (for example, FIFO or priority).  Thus, the
       scheduling policy determines which thread(s) are awakened and
       allowed to proceed." */

    pthread_cond_wait(&s[i], &mutex);  // block until forks are available

  pthread_mutex_unlock(&mutex);  // exit critical region
}

void put_forks(int i)   // i:  philosopher number
{
  pthread_mutex_lock(&mutex);   // enter critical region
  state[i] = THINKING;  // philosopher has finished eating
  test(LEFT(i));        // see if left neighbor can now eat
  test(RIGHT(i));       // see if right neighbor can now eat
  pthread_mutex_unlock(&mutex);  // exit critical region
}

void test(int i)
{
  if (state[i] == HUNGRY &&
      state[LEFT(i)] != EATING && state[RIGHT(i)] != EATING) {
    state[i] = EATING;
    pthread_cond_signal(&s[i]);  // signal and allow thread i to eat
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
