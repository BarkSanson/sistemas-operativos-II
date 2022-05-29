#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#define SEM_NAME "/mymutes"
#define SEM_INIT_VALUE 1

sem_t* initSem();
void deleteSem();
void signalSem(sem_t* sem);
void waitSem(sem_t* sem);