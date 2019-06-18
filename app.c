#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <elf.h>
#include <assert.h>

typedef int (*getpid_pfn)(void);
struct thread_params {
  long thread_id;
  getpid_pfn getpid;
};

static int ended_with(const char* s, const char* pattern) {
  int n = strlen(pattern);
  int m = strlen(s);

  if (n > m) return 0;

  int j = m - 1;
  int k = n - 1;

  while (k >= 0) {
    if(pattern[k--] != s[j--]) {
      return 0;
    }
  }
  return 1;
}

static unsigned long resolve(int fd, unsigned long dlstart, const char* sym_name) {
  unsigned long off = 0;

  Elf64_Ehdr ehdr;
  Elf64_Phdr phdr;
  Elf64_Phdr* dynamic = NULL;

  assert(pread(fd, &ehdr, sizeof(ehdr), off) == sizeof(ehdr));

  off += ehdr.e_phoff;
  for (int i = 0; i < ehdr.e_phnum; i++) {
    assert(pread(fd, &phdr, sizeof(phdr), off) == sizeof(phdr));
    off += sizeof(phdr);
    if (phdr.p_type == PT_DYNAMIC) {
      dynamic = &phdr;
      break;
    }
  }

  if (!dynamic) {
    fprintf(stderr, "found no dynamic segment");
    return -1;
  }

  unsigned long dyn[64] = {0,};
  unsigned long* ptr = (unsigned long*)(dynamic->p_vaddr + dlstart);

  while(*ptr) {
    if (ptr[0] < 64)
      dyn[ptr[0]] = ptr[1];
    ptr += 2;
  }

  const char* strtab = (const char*)dyn[DT_STRTAB];
  if (!strtab) {
    fprintf(stderr, "found no strtab");
    return -1;
  }

  Elf64_Sym* symtab = (Elf64_Sym*)dyn[DT_SYMTAB];
  if (!symtab) {
    fprintf(stderr, "found no symtab");
    return -1;
  }

  unsigned long symtab_nb = 1 + dynamic->p_memsz / sizeof(void*);

  for (int i = 0; i < symtab_nb; i++) {
    const char* name = (const char*)&strtab[symtab[i].st_name];
    if (!strcmp(name, sym_name)) {
      return symtab[i].st_value + dlstart;
    }
  }

  return -1;
}

unsigned long symbol_resolve(const char* dso, const char* sym) {
  int fd;
  char* buffer;

  unsigned long sym_addr = -1;

  fd = open("/proc/self/maps", O_RDONLY);
  assert(fd >= 0);

  buffer = calloc(1, 8192);
  assert(buffer);

  ssize_t n = read(fd, buffer, 8192);
  assert(n > 0);

  char* next = buffer;
  char* dso_fullname = NULL;
  int found_dlns_dso = 0;
  unsigned long dlstart, dlend;

  while(1) {
    char* curr = strsep(&next, "\n");
    if (!curr) break;

    char* words = curr;
    char* endptr = NULL;
    dlstart = strtoul(curr, &endptr, 16);
    dlend   = strtoul(1 + endptr, &words, 16);
    words += 1;
    while(1) {
      while(words && (words[0] == ' ' || words[0] == '\t'))
	++words;
      char* wp = strsep(&words, " \t");
      if (!wp) break;
      if (ended_with(wp, dso)) {
	dso_fullname = wp;
	found_dlns_dso = 1;
	goto found;
      }
    }
  }

 found:
  close(fd);
  
  if (found_dlns_dso) {
    printf("found %s loaded at %lx-%lx\n", dso_fullname, dlstart, dlend);
    fd = open(dso_fullname, O_RDONLY);
    assert(fd >= 0);
    sym_addr = resolve(fd, dlstart, sym);
  }
  return sym_addr;
}

void* threadfn(void* param) {

  struct thread_params* tp = (struct thread_params*)param;

  pthread_key_t pkey;
  unsigned long *data = malloc(sizeof(unsigned long));
  assert(data);

  *data = 0x1234;

  pthread_key_create(&pkey, free);
  pthread_setspecific(pkey, data);
  
  fprintf(stderr, "#%lu local_getpid addr = %p\n", tp->thread_id, tp->getpid);

  if (tp->getpid != (getpid_pfn)-1UL) {
    printf("#%lu local_getpid: %u\n", tp->thread_id, tp->getpid());
    printf("#%lu local_getpid: %u\n", tp->thread_id, tp->getpid());
  }

  printf("#%lu my own tls: %lx\n", tp->thread_id, *(unsigned long*)pthread_getspecific(pkey));

  return 0;
}

int main(int argc, char* argv[]) {
  pthread_t threads[4];

  getpid_pfn local_getpid = (getpid_pfn)symbol_resolve("libdlns.so", "local_getpid");
  struct thread_params params[] = {
    {0, local_getpid},
    {1, local_getpid},
    {2, local_getpid},
    {3, local_getpid},
  };
  
  for (int i = 0; i < 4; i++) {
    assert(pthread_create(&threads[i], NULL, threadfn, &params[i]) == 0);
  }

  for (int i = 0; i < 4; i++) {
    pthread_join(threads[i], NULL);
  }

  return 0;
}
