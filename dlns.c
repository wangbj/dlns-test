#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <elf.h>
#include <assert.h>

static pthread_key_t local_pkey = -1;

static int gettid(void) {
  return syscall(SYS_gettid);
}

static void pkey_destroy(void* p)
{
  fprintf(stderr, "%u called %s\n", gettid(), __func__);
  free(p);
}

int local_getpid(void) {

  if (pthread_getspecific(local_pkey) == NULL) {
    unsigned long* data = malloc(sizeof(unsigned long));
    assert(data);
    pthread_setspecific(local_pkey, data);
    *data = gettid();
  }

  unsigned long* cached = pthread_getspecific(local_pkey);
  assert(cached);
  return *cached;
}

__attribute__((constructor)) int dlns_init(void) {
  fprintf(stderr, "dlns_init\n");

  assert(pthread_key_create(&local_pkey, pkey_destroy) == 0);  

  return 0;
}
