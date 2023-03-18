#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>

const int MAX_SIZE = 10000;
 

int main(int argc, char *argv[]) {
    int fd1to2, fd2to1, result;
    int file;
    int n = atoi(argv[3]);
    char name1to2[] = "1to2.fifo";
    char name2to1[] = "2to1.fifo";
    (void)umask(0);

    mknod(name1to2, S_IFIFO | 0666, 0);
    mknod(name2to1, S_IFIFO | 0666, 0);
 
    result = fork();
    if (result < 0) {
        printf("Can\'t fork processing\n");
        exit(-1);
    } else if (result > 0) { /* Input and output process */
        char s[MAX_SIZE];
        if ((file = open(argv[1], O_RDONLY)) < 0){
            printf("Can\'t open input file\n");
            exit(-1);
        }

        ssize_t s_size = read(file, s, MAX_SIZE);
        if ((fd1to2 = open(name1to2, O_WRONLY)) < 0) {
            printf("Can't open FIFO for writing");
        }
        ssize_t size = write(fd1to2, s, s_size);
        if (size != s_size) {
            printf("input: Can\'t write all string to pipe 1to2\n");
            exit(-1);
        }
        if ((fd2to1 = open(name2to1, O_RDONLY)) < 0) {
            printf("Can't open FIFO for reading");
        }
        char s2[MAX_SIZE];
        ssize_t size2 = read(fd2to1, s2, MAX_SIZE);
        if ((file = open(argv[2], O_WRONLY)) < 0){
            printf("output: Can\'t open output file\n");
            exit(-1);
        }
        write(file, s2, size2);
    } else { /* data processing process */
        char s[MAX_SIZE];
        if ((fd1to2 = open(name1to2, O_RDONLY)) < 0) {
            printf("Can't open FIFO for reading");
        }
        ssize_t size = read(fd1to2, s, MAX_SIZE);
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
        if ((fd2to1 = open(name2to1, O_WRONLY)) < 0) {
            printf("Can't open FIFO for writing");
        }
        if (is_found) {
            size = write(fd2to1, answer, n);
            if (size != n) {
                printf("processing: Can\'t write all string to pipe 2to1\n");
                exit(-1);
            }
        } else {
            char not_found_message[] = "no decreasing substr with that length";
            size = write(fd2to1, not_found_message, sizeof(not_found_message) - 1);
            if (size != sizeof(not_found_message) - 1) {
                printf("processing: Can\'t write all string to pipe 2to1\n");
                exit(-1);
            }
        }
    }
    return 0;
}
