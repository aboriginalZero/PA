#include "common.h"
#include "syscall.h"

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4],result=-1;
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
	a[2] = SYSCALL_ARG3(r);
	a[3] = SYSCALL_ARG4(r);


uintptr_t sys_write(int fd, const void *buf, size_t len) {
	uintptr_t i = 0;
	if (fd == 1 || fd == 2) {
		for(; len > 0; len--) {
			_putc(((char*)buf)[i]);
			i++;;
		}
	}
	return i;
}

  switch (a[0]) {
    case SYS_none:{
      result = 1; 
      break;
    }
    case SYS_exit:{
      _halt(a[1]);
      break;
    }
    case SYS_write:{
      result = sys_write(a[1], (void *)a[2], a[3]);
			// result = fs_write(a[1], (void *)a[2], a[3]);
			break;  
    }
			
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  SYSCALL_ARG1(r) = result;
  return NULL;
}
