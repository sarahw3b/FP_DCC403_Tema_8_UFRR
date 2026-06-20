#include <pthread.h>
#include <stdio.h>

int contador = 0;

void* writer(void* arg) {
    contador = contador + 1;
    return NULL;
}

void* reader(void* arg) {
    printf("%d\n", contador);
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
