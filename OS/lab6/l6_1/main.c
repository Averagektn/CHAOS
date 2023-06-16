#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

#define LEN 255

void input(char **str, int size){
    char tmp[size][LEN];

    printf("Enter 0 in 1 argument to finish\n\n");

    for (int i = 0; i < size - 1; i++) {
        printf("Enter %d argument\n", i + 1);
        scanf("%s", tmp[i]);
    }
    for (int i = 0; i < size - 1; i++){
        str[i] = tmp[i];
    }

    str[size - 1] = NULL;
}

int main() {
    char **argv = NULL;
    int stat, size;
    pid_t pid;

    do {
        scanf("%d", &size);
        size++;

        argv = (char **)malloc(size * sizeof(char *));
        input(argv, size);

        if (strcmp(argv[0], "0") != 0){
            pid = fork();
            if (pid == 0){
                printf("Child %d\n", getpid());
                printf("Parent %d\n", getppid());
                execvp(argv[0], argv);
            } else if (pid > 0){
                waitpid(pid, &stat, WNOHANG | WUNTRACED);
            } else {
                printf("ERROR\n");
            }
        }
    } while (strcmp(argv[0], "0") != 0);
    return 0;
}