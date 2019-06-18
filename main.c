#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

int main(int argc, char* argv[])
{
  if (argc != 4) {
    fprintf(stderr, "%s <preloader> <dso> <app> [args]\n", argv[0]);
    exit(1);
  }

  assert(setenv("DLNS", argv[2], 0) == 0);

  char* dso = argv[1];
  char* ldpreload = realpath(dso, NULL);
  assert(ldpreload);

  assert(asprintf(&ldpreload, "LD_PRELOAD=%s", ldpreload) > 0);
  char* const envs[] = {
    "PATH=/bin:/usr/bin",
    ldpreload,
    NULL,
  };

  char** args = calloc(argc - 2, sizeof(char*));
  args[0] = strdup(argv[3]); // app

  int i = 4;
  for (; i < argc; i++) {
    args[i-3] = strdup(argv[i]);
  }
  args[i] = NULL;

  execvpe(argv[3], args, envs);

  perror("execvpe failed");
  exit(1);
}

