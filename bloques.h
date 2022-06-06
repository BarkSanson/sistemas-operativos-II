#include <stdio.h>  //printf(), fprintf(), stderr, stdout, stdin
#include <fcntl.h> //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> //S_IRUSR, S_IWUSR
#include <stdlib.h>  //exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <unistd.h> // SEEK_SET, read(), write(), open(), close(), lseek()
#include <errno.h>  //errno
#include <string.h> // strerror()
#include <sys/mman.h>
 
#define BLOCKSIZE 1024 // bytes
#define SUCCESS_EXIT 0
#define ERROR_EXIT -1 
#define DEBUG1 0
#define RED "\033[0;31m"
#define YELLOW  "\033[0;33m" 
#define BOLD_GREEN "\033[1;32m"
#define RESET_COLOR "\033[0m"
 
void* do_mmap(int fd);

int bmount(const char *camino);
int bumount();
int bwrite(unsigned int nbloque, const void *buf);
int bread(unsigned int nbloque, void *buf);

void mi_waitSem();
void mi_signalSem();
