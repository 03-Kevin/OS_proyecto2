#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>

#define DEVNAME "/dev/mydev"

void read_and_print(int fd) {
    char text[4096];
    int status = read(fd, text, 4096);
    if(status < 0) {
        perror("read");
        return;
    }
    printf("READ: I got %d bytes with '%s'\n", status, text);
}

int main(int argc, char **argv) {
    int fd, status, offset;
    void *ptr;

    if(argc < 2) {
        printf("Usage: %s [m,r,w,p] {data}\n", argv[0]);
        return 0;
    }

    fd = open(DEVNAME, O_RDWR);
    if(fd < 0) {
        perror("open");
        return 1;
    }

    switch(argv[1][0]) {
        case 'r':
            read_and_print(fd);
            break;
        // Other cases omitted for brevity
    }

    close(fd);
    return 0;
}
