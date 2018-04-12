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



    while ((*jerryArgs).stonesAvailable > 0) {
        pthread_mutex_lock(&(*jerryArgs).lock);

        while((*jerryArgs).turn == 1) {
            pthread_cond_wait(&(*jerryArgs).jerry, &(*jerryArgs).lock);
        }
        if ((*jerryArgs).stonesAvailable <= 0) {
            printf("Jerry is the winner!\n");
            pthread_exit(0);
        }
        (*jerryArgs).stonesAvailable -= 1;
        printf("Jerry picks up 1 stone, %d left\n\n",(*jerryArgs).stonesAvailable);
        (*jerryArgs).turn = 1;
        pthread_cond_signal(&(*jerryArgs).tom);
        //sleep(1);
        pthread_mutex_unlock(&(*jerryArgs).lock);
    }

    pthread_exit(0);

}

void* tomTurn(void* arg) {
    struct threadArgs *tomArgs = (struct threadArgs*)arg;

    int stones;

    while ((*tomArgs).stonesAvailable > 0) {
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
            printf("Tom picks up %d stones, %d left\n\n",stones, (*tomArgs).stonesAvailable);
        } else if ((*tomArgs).stonesAvailable < stones) {
            printf("Tom picks up %d stones, %d left\n\n",(*tomArgs).stonesAvailable, 0);
            (*tomArgs).stonesAvailable -= (*tomArgs).stonesAvailable;
        }
        (*tomArgs).turn = 0;
        pthread_cond_signal(&(*tomArgs).jerry);
        //sleep(1);
        pthread_mutex_unlock(&(*tomArgs).lock);
    }

    pthread_exit(0);

}


int main(int argc, char *argv[]) {
    struct threadArgs *share;
    share = malloc(sizeof(struct threadArgs));
    srand(time(NULL));
    (*share).stonesAvailable = rand() % 20 + 80;

    printf("amount of stones: %d\n", (*share).stonesAvailable);

    (*share).turn = 1;

    pthread_mutex_init(&(*share).lock, NULL);
    pthread_cond_init(&(*share).jerry, NULL);
    pthread_cond_init(&(*share).tom, NULL);

    pthread_t jerryID, tomID;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&jerryID, &attr, jerryTurn, (void*) share);
    pthread_create(&tomID, &attr, tomTurn, (void*) share);

    pthread_join(jerryID, NULL);
    pthread_join(tomID, NULL);
    

}
