#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t	open_offset;
} Finfo;

extern size_t fs_filesz(int fd);
extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern void fb_write(const void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, size_t len);

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs(){
  file_table[FD_FB].size = _screen.width * _screen.height * 4;
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char *pathname, int flags, int mode){
  for (int i=0;i<NR_FILES;i++) {
    if(strcmp(pathname, file_table[i].name)==0){
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("can't find the file\n");
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
  ssize_t fs_size=fs_filesz(fd);
	switch(fd){
		case FD_STDIN:
		case FD_STDOUT:
		case FD_STDERR:
			break;
		// case FD_FB:
		// 	break;
		case FD_EVENTS:
			Log("ccccc\n");
			len = events_read((void *)buf, len);
			break;
		case FD_DISPINFO:{
			// Log("bbbbbb\n");
			if (file_table[fd].open_offset>=fs_size)
				return 0;
			if(len>fs_size-file_table[fd].open_offset)
				len=fs_size-file_table[fd].open_offset;
			dispinfo_read(buf,file_table[fd].open_offset,len);
			file_table[fd].open_offset+=len;	
			break;
		}
		default:
			// Log("aaaaaa\n");
			if(file_table[fd].open_offset>=fs_size||len==0)
				return 0;
			if(len>fs_size-file_table[fd].open_offset)
				len=fs_size-file_table[fd].open_offset;
			ramdisk_read(buf,file_table[fd].disk_offset+file_table[fd].open_offset,len);
			file_table[fd].open_offset+=len;
			break;
	}
	return len;
}

size_t fs_filesz(int fd) {
	return file_table[fd].size;
}

int fs_close(int fd) {
	return 0;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
  ssize_t fs_size = fs_filesz(fd);
	switch(fd) {
		case FD_STDOUT:
		case FD_STDERR:{
			for(int i=0;i<len;i++)
				_putc(((char *)buf)[i]);
				break;
		}
		case FD_FB:{
			fb_write(buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
		}	
		default:
			if(file_table[fd].open_offset>=fs_size)
				return 0;	
			if(len>fs_size-file_table[fd].open_offset)
				len=fs_size-file_table[fd].open_offset;
			fb_write(buf,file_table[fd].disk_offset+file_table[fd].open_offset,len);
			file_table[fd].open_offset += len;
			break;
	}
	return len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
	ssize_t fs_size = fs_filesz(fd);
	switch(whence) {
		case SEEK_SET:
		// Log("22222\n");
			if(offset>=0&&offset<=file_table[fd].size)
				file_table[fd].open_offset = offset;
				break;
		case SEEK_CUR:
		// Log("33333\n");
			if((offset+file_table[fd].open_offset>=0)&&(offset+file_table[fd].open_offset<=fs_size))
				file_table[fd].open_offset+=offset;
				break;
		case SEEK_END:
		// Log("111111\n");
			file_table[fd].open_offset = file_table[fd].size + offset;
			break;
	}
	return file_table[fd].open_offset;
}