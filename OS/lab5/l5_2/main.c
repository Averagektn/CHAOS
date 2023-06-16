#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define ERROR (-1)
#define SRC 1
#define SORT 2
#define DST 3
#define ARGUMENTS 4
#define SORT_BY_SIZE 1
#define SORT_BY_ALPHABET 2

typedef struct {
    char path[PATH_MAX];
    char name[FILENAME_MAX];
    int  size;
} file_t;

file_t *f_list;
int f_list_len = 0;

int cmpName(file_t file1, file_t file2) {
    return (strcmp(file1.name, file2.name) > 0);
}

int cmpSize(file_t file1, file_t file2) {
    return ((file1.size - file2.size) < 0);
}

char* slashAdd(char* path) {
    if (path[strlen(path) - 1] != '/')
        strcat(path, "/");
    return path;
}

int dirPass(const char *dir_name) {
    DIR *directory;
    struct dirent *dir_item;

    directory = opendir(dir_name);

    while((dir_item = readdir(directory)) != NULL) {
        char next_item[PATH_MAX];
        strcpy(next_item, dir_name);
        strcat(next_item, dir_item->d_name);
        if (strcmp(".", dir_item->d_name) != 0 && strcmp("..", dir_item->d_name) != 0) {
            struct stat statBuf;
            lstat(next_item, &statBuf);
            if (S_ISDIR(statBuf.st_mode)) {
                dirPass(slashAdd(next_item));
            } else if (S_ISREG(statBuf.st_mode)) {
                file_t file_tmp;
                strcpy(file_tmp.name, dir_item->d_name);
                strcpy(file_tmp.path, next_item);
                file_tmp.size = (int) statBuf.st_size;
                f_list = realloc(f_list, (++f_list_len) * sizeof(file_t));
                f_list[f_list_len - 1] = file_tmp;
            }
        }
    }
    closedir(directory);
}

void sort(int cmpFunc(file_t, file_t)) {
    for (int i = 0; i < f_list_len; i++) {
        for (int j = 0; j < f_list_len - i - 1; j++) {
            if (cmpFunc(f_list[j], f_list[j + 1])) {
                file_t file_tmp = f_list[j];
                f_list[j] = f_list[j + 1];
                f_list[j + 1] = file_tmp;
            }
        }
    }
}

int main(int argc, char const *argv[]) {
    char dest_file[PATH_MAX + 1], dest_path[PATH_MAX], srcDir[PATH_MAX];
    int sort_opt = atoi(argv[SORT]);

    realpath(argv[DST], dest_path);
    slashAdd(dest_path);

    if (argc != ARGUMENTS) {
        printf("ERROR: Wrong number of arguments\n");
        return ERROR;
    }

    if (sort_opt != SORT_BY_SIZE && sort_opt != SORT_BY_ALPHABET) {
        printf("ERROR: Wrong sort option\n");
        return ERROR;
    }

    strcpy(srcDir, argv[SRC]);
    slashAdd(srcDir);
    dirPass(srcDir);

    if (sort_opt == SORT_BY_ALPHABET)
        sort(cmpName);
    else
        sort(cmpSize);

    mkdir(dest_path, 0777);

    for (int i = 0; i < f_list_len; i++) {
        strcpy(dest_file, dest_path);
        strcat(dest_file, f_list[i].name);
        if (link(f_list[i].path, dest_file) == ERROR)
            printf("File already exists %s\n", f_list[i].name);
        else
            printf("ADDED\n FILE: %s\n SIZE: %d\n", f_list[i].name, f_list[i].size);
    }

    return 0;
}