#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

int main(int argc, char* argv[])
{
  if (argc > 1 && argc < 4) {
    fprintf(stderr, "%s <preloader> <dso> <app> [args]\n", argv[0]);
    exit(1);
  }

  char* dso = argv[1];
  char* dso_full = realpath(dso, NULL);
  assert(dso_full);

  char* dlns = realpath(argv[2], NULL);
  assert(dlns);

  char* dlns_env, *ldpreload_env;

  assert(asprintf(&ldpreload_env, "LD_PRELOAD=%s", dso_full) > 0);
  assert(asprintf(&dlns_env, "DLNS=%s", dlns) > 0);

  char* const envs[] = {
    "PATH=/bin:/usr/bin",
    ldpreload_env,
    dlns_env,
    NULL,
  };

  char** args = calloc(argc - 2, sizeof(char*));
  args[0] = strdup(argv[3]); // app

  int i = 1;
  for (; i + 3 < argc; i++) {
    args[i] = strdup(argv[i+3]);
  }
  args[i] = NULL;

  execvpe(args[0], args, envs);

  perror("execvpe failed");
  exit(1);
}

