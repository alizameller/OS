#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <ctype.h> 
#include <limits.h>

int main() {
// Read the pattern from file
char *file = "test.txt";
int file_fd; 
int size;
struct stat st;
void *file_memory;


if ((file_fd = open(file, O_RDONLY)) == -1) {
    fprintf(stderr,"Error: could not open %s for reading: %s\n", file, strerror(errno));
    return -1; 
}

// Get size of file
if (fstat(file_fd, &st) == -1) {
    fprintf(stderr,"Error: could not get size of %s for mmap: %s\n", file, strerror(errno));
    return -1; 
}
size = st.st_size;

if ((file_memory = mmap(NULL, size, PROT_READ,MAP_SHARED, file_fd, 0)) == MAP_FAILED) {
    close(file_fd); 
    fprintf(stderr,"Error: could not mmap %s: %s\n", file, strerror(errno));
    return -1;
}

close(file_fd); 
}