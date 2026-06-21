#include <pthread.h>
#include <stdio.h>

int saldo = 0;
int estoque = 0;
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

/* Thread 1: pega mutex1 depois mutex2 */
void* thread1(void* arg) {
    pthread_mutex_lock(&mutex1);
    saldo++;
    pthread_mutex_lock(&mutex2);
    estoque++;
    pthread_mutex_unlock(&mutex2);
    pthread_mutex_unlock(&mutex1);
    return NULL;
}

/* Thread 2: pega mutex2 depois mutex1 */
void* thread2(void* arg) {
    pthread_mutex_lock(&mutex2);
    estoque++;
    pthread_mutex_lock(&mutex1);
    saldo++;
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    return NULL;
}

int main() {
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    return 0;
}
