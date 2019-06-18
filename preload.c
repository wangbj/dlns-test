/**
 * a mini-loader to load dso into new linker namespace
 */

#include <link.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <assert.h>

static void dump_link_map(struct link_map* map) {
  struct link_map* curr = map;

  while(curr) {
    fprintf(stderr, "[link_map] dso: %s, loaded at: %p, _DYNAMIC: %p\n",
	    curr->l_name, (void*)curr->l_addr, curr->l_ld);
    curr = curr->l_next;
  }
}

__attribute__((constructor)) int dl_start(void)
{
  char* dso = secure_getenv("DLNS");
  if (!dso) {
    fprintf(stderr, "unable to find DLNS");
    return -1;
  }

  fprintf(stderr, "open %s\n", dso);
  void* link_map = dlmopen(LM_ID_NEWLM, dso, RTLD_NOW | RTLD_LOCAL);
  if (!link_map) {
    fprintf(stderr, "dlmopen %s failed: %s\n", dso, dlerror());
    return -1;
  }

  dump_link_map(link_map);

  return 0;
}
