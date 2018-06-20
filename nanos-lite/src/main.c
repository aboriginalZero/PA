#include "common.h"

extern void load_prog(const char *filename);

/* Uncomment these macros to enable corresponding functionality. */
#define HAS_ASYE
#define HAS_PTE

void init_mm(void);
void init_ramdisk(void);
void init_device(void);
void init_irq(void);
void init_fs(void);
uint32_t loader(_Protect *, const char *);

int main() {
#ifdef HAS_PTE
  init_mm();
#endif

  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);
  // Log("qqq\n");
  init_ramdisk();
  // Log("333\n");
  init_device();

#ifdef HAS_ASYE
  Log("Initializing interrupt/exception handler...");
  init_irq();
#endif
  // Log("444\n");
  init_fs();

  // uint32_t entry = loader(NULL, "/bin/dummy");

  // // uint32_t entry = loader(NULL, NULL);
  // ((void (*)(void))entry)();
  // Log("1111\n");
  // load_prog("/bin/pal");
  load_prog("/bin/videotest");
  // _trap();

  panic("Should not reach here");
}
