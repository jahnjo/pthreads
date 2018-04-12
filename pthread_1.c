#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct threadArgs {
    int stonesAvailable;
    int turn;
    pthread_mutex_t lock;
    pthread_cond_t jerry, tom;
};

void* jerryTurn(void* arg) {
    struct threadArgs *jerryArgs = (struct threadArgs*)arg;
    pthread_mutex_init(&(*jerryArgs).lock, NULL);
    pthread_cond_init(&(*jerryArgs).jerry, NULL);
    pthread_cond_init(&(*jerryArgs).tom, NULL);


    while ((*jerryArgs).stonesAvailable > 0) {
        pthread_mutex_lock(&(*jerryArgs).lock);

        while((*jerryArgs).turn == 1) {
            pthread_cond_wait(&(*jerryArgs).jerry, &(*jerryArgs).lock);
        }
        
        (*jerryArgs).stonesAvailable -= 1;
        printf("Jerry picks up 1 stone, %d left\n",(*jerryArgs).stonesAvailable);
        (*jerryArgs).turn = 1;
        pthread_cond_signal(&(*jerryArgs).tom);
        sleep(1);
        pthread_mutex_unlock(&(*jerryArgs).lock);
    }

    pthread_exit(0);

}

void* tomTurn(void* arg) {
    struct threadArgs *tomArgs = (struct threadArgs*)arg;

    pthread_mutex_init(&(*tomArgs).lock, NULL);
    pthread_cond_init(&(*tomArgs).jerry, NULL);
    pthread_cond_init(&(*tomArgs).tom, NULL);


    while ((*tomArgs).stonesAvailable > 0) {
        pthread_mutex_lock(&(*tomArgs).lock);
        while((*tomArgs).turn == 0) {
            pthread_cond_wait(&(*tomArgs).tom, &(*tomArgs).lock);
        }
        (*tomArgs).stonesAvailable -= rand() % 3 + 1;
        printf("Tom Thread stones: %d\n",(*tomArgs).stonesAvailable);
        (*tomArgs).turn = 0;
        pthread_cond_signal(&(*tomArgs).jerry);
        sleep(1);
        pthread_mutex_unlock(&(*tomArgs).lock);
    }

    pthread_exit(0);

}


int main(int argc, char *argv[]) {
    struct threadArgs *share;
    share = malloc(sizeof(struct threadArgs));
    (*share).stonesAvailable = rand() % 20 + 80;
    (*share).turn = 1;
    pthread_t jerryID, tomID;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&jerryID, &attr, jerryTurn, (void*) share);
    pthread_create(&tomID, &attr, tomTurn, (void*) share);

    pthread_join(jerryID, NULL);
    pthread_join(tomID, NULL);
    

}
