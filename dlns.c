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

pthread_key_t local_pkey;

static __attribute__((noinline)) void pkey_destroy(void* p) {
  fprintf(stderr, "called %s\n", __func__);
  free(p);
}

int pause(void) {
  fprintf(stderr, "called pause from dlns\n");

  unsigned long* p = calloc(2, sizeof(long));

  printf("calloc symbol resolved to %p\n", calloc);
  printf("call calloc returned %p\n", p);

  p[0] = 0x1234;
  p[1] = 0x5678;

  printf("free resolved to %p\n", free);

  pthread_key_create(&local_pkey, pkey_destroy);
  pthread_setspecific(local_pkey, p);

  return syscall(SYS_pause);
}

__attribute__((constructor)) int dlns_init(void) {
  fprintf(stderr, "dlns_init\n");
  return 0;
}
