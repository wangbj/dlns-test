#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <time.h>

static void handle_sigalrm(int sig) {
}

void* thread_fn(void* param) {
  signal(SIGALRM, handle_sigalrm);
  printf("pause = %d\n", pause());
  return NULL;
}

int main(int argc, char* argv[]) {
  pthread_t thread;

  pthread_create(&thread, NULL, thread_fn, NULL);

  struct timespec ts = {1, 0};
  // NB: this could be racy because no guarantee
  // `thread' called pause already
  nanosleep(&ts, NULL);
  pthread_kill(thread, SIGALRM);

  pthread_join(thread, NULL);

  return 0;
}
