#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <assert.h>
#include <time.h>
#include "syscall.h"

extern char end;
uint32_t program_break = &end;

// TODO: discuss with syscall interface
#ifndef __ISA_NATIVE__

extern
// FIXME: this is temporary

int _syscall_(int type, uintptr_t a0, uintptr_t a1, uintptr_t a2){
  int ret = -1;
  asm volatile("int $0x80": "=a"(ret): "a"(type), "b"(a0), "c"(a1), "d"(a2));
  return ret;
}

void _exit(int status) {
  _syscall_(SYS_exit, status, 0, 0);
}

int _open(const char *path, int flags, mode_t mode) {
  _exit(SYS_open);
}

int _write(int fd, void *buf, size_t count){
  return _syscall_(SYS_write,fd,(uintptr_t)buf, count);
}

extern char end;
void *_sbrk(intptr_t increment){
  // char *_end = &end;
  // char *new_end = _end + increment;
  // int ret = _syscall_(SYS_brk, (uintptr_t)new_end, 0, 0);
  // if (ret != 0) return (void *)-1;
  // void *old_end = _end;
  // _end = new_end;
  // return old_end;
  // return (void *)-1;
  // Log("%d\n",increment);
  if(increment == 0){
    //program_break = &end;
 // Log("%d\n",increment);
    return (void *)program_break;
  }else{
    uint32_t old_program_break; 
    old_program_break = program_break ;
    program_break = program_break +increment ;
    int result = _syscall_(SYS_brk, program_break, 0, 0);
    if(result !=0) return (void *)-1;
 // Log("%d\n",increment);
    return (void *)old_program_break;
  }
}

int _read(int fd, void *buf, size_t count) {
  _exit(SYS_read);  
}

int _close(int fd) {
  _exit(SYS_close);
}

off_t _lseek(int fd, off_t offset, int whence) {
  _exit(SYS_lseek);
}

// The code below is not used by Nanos-lite.
// But to pass linking, they are defined as dummy functions

// not implement but used
int _fstat(int fd, struct stat *buf) {
  return 0;
}

int execve(const char *fname, char * const argv[], char *const envp[]) {
  assert(0);
  return -1;
}

int _execve(const char *fname, char * const argv[], char *const envp[]) {
  return execve(fname, argv, envp);
}

int _kill(int pid, int sig) {
  _exit(-SYS_kill);
  return -1;
}

pid_t _getpid() {
  _exit(-SYS_getpid);
  return 1;
}

char **environ;

time_t time(time_t *tloc) {
  assert(0);
  return 0;
}

int signal(int num, void *handler) {
  assert(0);
  return -1;
}

pid_t _fork() {
  assert(0);
  return -1;
}

int _link(const char *d, const char *n) {
  assert(0);
  return -1;
}

int _unlink(const char *n) {
  assert(0);
  return -1;
}

pid_t _wait(int *status) {
  assert(0);
  return -1;
}

clock_t _times(void *buf) {
  assert(0);
  return 0;
}

int _gettimeofday(struct timeval *tv) {
  assert(0);
  tv->tv_sec = 0;
  tv->tv_usec = 0;
  return 0;
}

int _fcntl(int fd, int cmd, ... ) {
  assert(0);
  return 0;
}

int pipe(int pipefd[2]) {
  assert(0);
  return 0;
}

int dup(int oldfd) {
  assert(0);
  return 0;
}

int dup2(int oldfd, int newfd) {
  assert(0);
  return 0;
}

pid_t vfork(void) {
  assert(0);
  return 0;
}

#endif
