#include "common.h"
#include "memory.h"
#include "fs.h"
//这边一直没有修改造成
#define DEFAULT_ENTRY ((void *)0x8048000)
extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern size_t get_ramdisk_size();
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_filesz(int fd);
extern int fs_open(const char *pathname, int flags, int mode);
extern int fs_close(int fd);

uintptr_t loader(_Protect *as, const char *filename) {
  // //将ramdisk中从0开始的所有内容放置在0x4000000,并
  // //把这个地址作为程序的入口返回即可.
  // ramdisk_read(DEFAULT_ENTRY,0,get_ramdisk_size());

  // int fd = fs_open(filename, 0, 0);
	// fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd)); 
	// fs_close(fd); 
  int fd = fs_open(filename, 0, 0);
  size_t nbyte = fs_filesz(fd);
  void *pa;
  void *va;

  Log("loaded: [%d]%s size:%d", fd, filename, nbyte);

  void *end = DEFAULT_ENTRY + nbyte;
  for (va = DEFAULT_ENTRY; va < end; va += PGSIZE) {
    pa = new_page();
    Log("Map va to pa: 0x%08x to 0x%08x", va, pa);
    _map(as, va, pa);
    fs_read(fd, pa, (end - va) < PGSIZE ? (end - va) : PGSIZE);
  }

  return (uintptr_t)DEFAULT_ENTRY;
}
