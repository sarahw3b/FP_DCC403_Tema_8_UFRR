#include <pthread.h>

int contador = 0;
pthread_mutex_t mutex;

void* writer(void* arg) {
    pthread_mutex_lock(&mutex);
    contador++;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* reader(void* arg) {
    pthread_mutex_lock(&mutex);
    int x = contador;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, writer, NULL);
    pthread_create(&t2, NULL, reader, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
