#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>

const int MAX_SIZE = 10000;
 
int main(int argc, char *argv[]) {
    int fd1to2, fd2to1;
    int file;
    char name1to2[] = "1to2.fifo";
    char name2to1[] = "2to1.fifo";
    (void)umask(0);
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
    return 0;
}
