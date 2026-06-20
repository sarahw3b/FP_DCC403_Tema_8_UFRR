#include <pthread.h>

int a = 0;
int b = 0;
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

void* thread1(void* arg) {
    pthread_mutex_lock(&mutex1);
    a++;
    pthread_mutex_unlock(&mutex1);
    return NULL;
}

void* thread2(void* arg) {
    pthread_mutex_lock(&mutex2);
    b++;
    pthread_mutex_unlock(&mutex2);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
