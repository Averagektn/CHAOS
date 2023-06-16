#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>

#define LEN 255

typedef struct {
    char arg_1[LEN];
    char arg_2[LEN];
}ARGV;

void *func(void *tmp){
    execlp(((ARGV *)tmp)->arg_1, ((ARGV *)tmp)->arg_1, ((ARGV *)tmp)->arg_2, NULL);
}

int main() {
    ARGV ops;
    pthread_t pthread;
    do {
        scanf("%s", ops.arg_1);
        scanf("%s", ops.arg_2);
        if (strcmp(ops.arg_1, "0") != 0) {
            pthread_create(&pthread, NULL, func, &ops);
            pthread_join(pthread, NULL);
        }
    } while (strcmp(ops.arg_1, "0") != 0);
    return 0;
}