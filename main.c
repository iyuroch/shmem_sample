#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#define SEM_KEY "tmp"
#define SHM_KEY "/dev/shm"
#define PROJECT_ID 1111

void lock(int sem_id);
void unlock(int sem_id);
int init_semaphore();
int init_shmem();
int shmem_nattach(int shm_id);
int shmem_atime(int shm_id);

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
#if defined(_linux_)
    struct seminfo* _buf;
#endif
};

int main () {
    int sem_id, shm_id;
    int *shm_val;

    pid_t pid = getpid();

    shm_id = init_shmem();
    sem_id = init_semaphore();

    shm_val = (int *) shmat(shm_id, 0, 0);
    #ifdef DEBUG 
    printf("access time %i\n", shmem_atime(shm_id));
    #endif
    while (shmem_nattach(shm_id) < 2) {
	if (*shm_val != 0)
	    break;
	usleep(1);
    }

    *shm_val = 0;

    lock(sem_id);
    #ifdef DEBUG
    printf("in lock section %i\n", pid);
    #endif
    
    printf("shm_val %i\n", *shm_val);
    #ifdef DEBUG
    printf("no. of attahced %i\n", shmem_nattach(shm_id));
    #endif
    *shm_val = (int ) pid;

    #ifdef DEBUG
    printf("unlocking %i\n", pid);
    #endif
    unlock(sem_id);
}

int shmem_nattach(int shm_id) {
    struct shmid_ds buf;
    shmctl(shm_id, IPC_STAT, &buf);

    return buf.shm_nattch;
}

int shmem_atime(int shm_id) {
    struct shmid_ds buf;
    shmctl(shm_id, IPC_STAT, &buf);

    return buf.shm_ctime;
}

int init_shmem() {
    int shm_id, shm_key;

    if ((shm_key = ftok(SHM_KEY, PROJECT_ID)) == -1)
	perror("ftok");

    if ((shm_id = shmget(shm_key, 200, 0660 | IPC_CREAT)) == -1)
	perror("shmget");

    return shm_id;
}

int init_semaphore() {
    int sem_id;
    key_t sem_key;

    if ((sem_key = ftok(SEM_KEY, PROJECT_ID)) == -1)
        perror ("ftok");

    sem_id = semget(sem_key, 1, 0660 | IPC_CREAT | IPC_EXCL);
    if (sem_id == -1)
    {
	if (errno != EEXIST)
	    perror("semget error creating");
	
	if ((sem_id = semget(sem_key, 1, 0660 | IPC_CREAT)) == -1)
	    perror("semget error getting");

	while (1)
	{
            union semun arg;
	    struct semid_ds ds;

	    arg.buf = &ds;
            semctl(sem_id, 0, IPC_STAT, arg);
	    if (ds.sem_otime != 0)
		break;
	}
    } else {
        struct sembuf init_val = {0, 1, SEM_UNDO};
        semop(sem_id, &init_val, 1);
    }

    return sem_id; 
}

void lock(int sem_id) {
    struct sembuf block_val = {0, -1, SEM_UNDO};
    if(semop(sem_id, &block_val, 1) == -1)
    	perror("semop lock");
}

void unlock(int sem_id) {
    struct sembuf block_val = {0, 1, SEM_UNDO};
    if (semop(sem_id, &block_val, 1) == -1)
    	perror("semop unlock");
}