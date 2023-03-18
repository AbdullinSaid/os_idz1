#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>

const int MAX_SIZE = 10000;

int main(int argc, char *argv[]) {
    int fd1to2[2], fd2to3[2], result;
    int file;
    int n = atoi(argv[3]);

    if (pipe(fd1to2) < 0) {
        printf("Can\'t open pipe 1to2\n");
        exit(-1);
    }
    if (pipe(fd2to3) < 0) {
        printf("Can\'t open pipe 2to3\n");
        exit(-1);
    }

    result = fork();
    if (result < 0) {
        printf("Can\'t fork processing\n");
        exit(-1);
    } else if (result > 0) { /* Input process */
        if (close(fd1to2[0]) < 0) {
            printf("input: Can\'t close reading side of pipe 1to2\n");
            exit(-1);
        }

        char s[MAX_SIZE];
        if ((file = open(argv[1], O_RDONLY)) < 0){
            printf("Can\'t open input file\n");
            exit(-1);
        }

        ssize_t s_size = read(file, s, MAX_SIZE);
        ssize_t size = write(fd1to2[1], s, s_size);
        if (size != s_size) {
            printf("input: Can\'t write all string to pipe 1to2\n");
            exit(-1);
        }
        if (close(fd1to2[1]) < 0) {
            printf("input: Can\'t close writing side of pipe 1to2\n");
            exit(-1);
        }
    } else {
        int result2 = fork();
        if (result2 < 0) {
           printf("Can\'t fork output\n");
           exit(-1);
        } else if (result2 > 0) {  /* data processing process */
            char s[MAX_SIZE];
            if (close(fd1to2[1]) < 0) {
                printf("processing: Can\'t close writing side of pipe 1to2\n");
                exit(-1);
            }
            ssize_t size = read(fd1to2[0], s, MAX_SIZE);
            int current = 1;
            char answer[n];
            bool is_found = false;
            if (n == 1 && size > 0) {
                answer[0] = s[0];
                is_found = true;
            } else {
                for (int i = 1; i < size; ++i) {
                    if (s[i] < s[i - 1]) {
                        current++;
                    } else {
                        current = 1;
                    }
                    if (current == n) {
                        for (int j = i + 1 - n; j <= i; ++j) {
                            answer[j - i - 1 + n] = s[j];
                        }
                        is_found = true;
                        break;
                    }
                }
            }
            if (close(fd1to2[0]) < 0) {
                printf("processing: Can\'t close reading side of pipe 1to2\n");
                exit(-1);
            }
            if (close(fd2to3[0]) < 0) {
                printf("processing: Can\'t close reading side of pipe 2to3\n");
                exit(-1);
            }
            if (is_found) {
                size = write(fd2to3[1], answer, n);
                if (size != n) {
                    printf("processing: Can\'t write all string to pipe 2to3\n");
                    exit(-1);
                }
            } else {
                char not_found_message[] = "no decreasing substr with that length";
                size = write(fd2to3[1], not_found_message, sizeof(not_found_message) - 1);
                if (size != sizeof(not_found_message) - 1) {
                    printf("processing: Can\'t write all string to pipe 2to3\n");
                    exit(-1);
                }
            }
            if (close(fd2to3[1]) < 0) {
                printf("processing: Can\'t close writing side of pipe 2to3\n");
                exit(-1);
            }
        } else { /* Output process */
            if (close(fd2to3[1]) < 0) {
                printf("output: Can\'t close writing side of pipe 2to3\n");
                exit(-1);
            }
            char s[MAX_SIZE];
            ssize_t size = read(fd2to3[0], s, MAX_SIZE);
            if ((file = open(argv[2], O_WRONLY)) < 0){
                printf("output: Can\'t open output file\n");
                exit(-1);
            }

            write(file, s, size);
            if (close(fd2to3[0]) < 0) {
                printf("output: Can\'t close reading side of pipe 2to3\n");
                exit(-1);
            }
        }
    }
    return 0;
}
