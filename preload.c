/**
 * a mini-loader to load dso into new linker namespace
 */
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <link.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

static void dump_link_map(struct link_map* map) {
  struct link_map* curr = map;

  while(curr) {
    fprintf(stderr, "[link_map] dso: %s, loaded at: %p, _DYNAMIC: %p\n",
	    curr->l_name, (void*)curr->l_addr, curr->l_ld);
    curr = curr->l_next;
  }
}

static void* dlns_handle;

typedef int (*pause_pfn)(void);

// exposed LD_PRELOAD
int pause(void) {
  fprintf(stderr, "called pause.\n");

  // call pause from new the lmid
  pause_pfn dlns_pause = dlsym(dlns_handle, "pause");
  assert(dlns_pause);

  return dlns_pause();
}

__attribute__((constructor)) int dl_start(void)
{
  char* dso = secure_getenv("DLNS");
  if (!dso) {
    fprintf(stderr, "unable to find DLNS");
    return -1;
  }

  fprintf(stderr, "open %s\n", dso);
  void* link_map = dlmopen(LM_ID_NEWLM, dso, RTLD_LAZY | RTLD_LOCAL);
  if (!link_map) {
    fprintf(stderr, "dlmopen %s failed: %s\n", dso, dlerror());
    return -1;
  }

  dump_link_map(link_map);

  dlns_handle = (void*)link_map;

  return 0;
}
