#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct threadArgs {
    int stonesAvailable;
    int turn,startSignal;
    pthread_mutex_t lock;
    pthread_cond_t jerry, tom, start;
};

void* broadcastThread(void* arg) {
    struct threadArgs *startArgs = (struct startArgs*)arg;

    printf("Broadcasting start signal in 5 seconds...\n");
    for (int i = 0; i < 5; i++) {
        sleep(1);
    }
    printf("Broadcasting start signal...\n");
    (*startArgs).startSignal = 1;
    pthread_cond_broadcast(&(*startArgs).start);
    pthread_exit(0);
}

void* jerryTurn(void* arg) {
    struct threadArgs *jerryArgs = (struct threadArgs*)arg;

    //wait for start broadcast
    pthread_mutex_lock(&(*jerryArgs).lock);
    while ((*jerryArgs).startSignal == 0) {
        pthread_cond_wait(&(*jerryArgs).start, &(*jerryArgs).lock);
    }
    pthread_mutex_unlock(&(*jerryArgs).lock);

    //keep picking stones until none left
    while ((*jerryArgs).stonesAvailable > 0) {
        //lock thread and wait for tom to signal it is jerry's turn
        pthread_mutex_lock(&(*jerryArgs).lock);
        while((*jerryArgs).turn == 1) {
            pthread_cond_wait(&(*jerryArgs).jerry, &(*jerryArgs).lock);
        }
        if ((*jerryArgs).stonesAvailable <= 0) {
            printf("Jerry is the winner!\n");
            pthread_exit(0);
        }
        (*jerryArgs).stonesAvailable -= 1;
        printf("Jerry picks up 1 stone, %d left\n",(*jerryArgs).stonesAvailable);
        (*jerryArgs).turn = 1;
        pthread_cond_signal(&(*jerryArgs).tom);
        pthread_mutex_unlock(&(*jerryArgs).lock);
    }
    pthread_exit(0);
}

void* tomTurn(void* arg) {
    struct threadArgs *tomArgs = (struct threadArgs*)arg;
    int stones;

    //wait on start broadcast
    pthread_mutex_lock(&(*tomArgs).lock);
    while ((*tomArgs).startSignal == 0) {
        pthread_cond_wait(&(*tomArgs).start, &(*tomArgs).lock);
    }
    pthread_mutex_unlock(&(*tomArgs).lock);
    //enter if there are still stones to pick up
    while ((*tomArgs).stonesAvailable > 0) {
        //Wait for tom's turn and signal from jerry
        pthread_mutex_lock(&(*tomArgs).lock);
        while((*tomArgs).turn == 0) {
            pthread_cond_wait(&(*tomArgs).tom, &(*tomArgs).lock);
        }
        if ((*tomArgs).stonesAvailable <= 0) {
            printf("Tom is the winner!\n");
            pthread_exit(0);
        }
        srand(time(NULL));
        stones = rand() % 4 + 1;
        if ((*tomArgs).stonesAvailable >= stones) {
            (*tomArgs).stonesAvailable -= stones;
            printf("Tom picks up %d stones, %d left\n",stones, (*tomArgs).stonesAvailable);
        } else if ((*tomArgs).stonesAvailable < stones) {
            printf("Tom picks up %d stones, %d left\n",(*tomArgs).stonesAvailable, 0);
            (*tomArgs).stonesAvailable -= (*tomArgs).stonesAvailable;
        }
        (*tomArgs).turn = 0;
        pthread_cond_signal(&(*tomArgs).jerry);
        pthread_mutex_unlock(&(*tomArgs).lock);
    }

    pthread_exit(0);

}


int main(int argc, char *argv[]) {
    struct threadArgs *share;
    share = malloc(sizeof(struct threadArgs));
    srand(time(NULL));
    (*share).stonesAvailable = rand() % 60 + 20;
    int g =  (*share).stonesAvailable;

    printf("amount of stones: %d\n", (*share).stonesAvailable);

    (*share).turn = 1;

    pthread_mutex_init(&(*share).lock, NULL);
    pthread_cond_init(&(*share).jerry, NULL);
    pthread_cond_init(&(*share).tom, NULL);
    pthread_cond_init(&(*share).start, NULL);    
    (*share).startSignal = 0;

    pthread_t jerryID, tomID, startID;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&startID, &attr, broadcastThread, (void*) share);
    pthread_create(&jerryID, &attr, jerryTurn, (void*) share);
    pthread_create(&tomID, &attr, tomTurn, (void*) share);
        
    pthread_join(startID, NULL);
    pthread_join(jerryID, NULL);
    pthread_join(tomID, NULL);
    return 0;
}
