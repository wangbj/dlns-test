#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/personality.h>
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
    fprintf(stderr, "%s [--library-path <library_path>] <preloader> <dso> <app> [args]\n", argv[0]);
    exit(1);
  }

  int i = 1;

  const char library_path_opt[] = "--library-path";
  int library_path_opt_len = strlen(library_path_opt);
  char* library_path = NULL;

  if (strncmp(argv[i], library_path_opt, library_path_opt_len) == 0) {
    ++i;
    library_path = strdup(argv[i]);
    printf("ll : %s\n", library_path);
    ++i;
  }

  char* dso = argv[i++];
  char* dso_full = realpath(dso, NULL);
  assert(dso_full);

  char* dlns = realpath(argv[i++], NULL);
  assert(dlns);

  char* dlns_env, *ldpreload_env, *library_path_env = NULL;

  assert(asprintf(&ldpreload_env, "LD_PRELOAD=%s", dso_full) > 0);
  assert(asprintf(&dlns_env, "DLNS=%s", dlns) > 0);

  if(library_path) {
    assert(asprintf(&library_path_env, "LD_LIBRARY_PATH=%s", realpath(library_path, NULL)) > 0);
  }

  char* const envs[] = {
    "PATH=/bin:/usr/bin",
    ldpreload_env,
    dlns_env,
    library_path? library_path_env: NULL,
    NULL,
  };

  char** args = calloc(argc - 2, sizeof(char*));

  args[0] = strdup(argv[i++]); // app
  int j = 1;
  for (; i < argc; i++) {
    args[j++] = strdup(argv[i]);
  }
  args[j] = NULL;

  prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);
  personality(PER_LINUX | ADDR_NO_RANDOMIZE);

  execvpe(args[0], args, envs);

  perror("execvpe failed");
  exit(1);
}

