#include <stdlib.h> // Standard Library
#include <stdint.h> // Typedef int32_t and more
#include <fcntl.h> // Imports open() function
#include <unistd.h> // Defines miscellaneous functions/types (write(), close())
#include <stdio.h> // For printf()

int main(int argc, char const *argv[])
{
    // Open (Q1.datafile, read only, permissions(0, since opening an existing file doesn't make sense to attribute a permissiosn)
    int  fd = open("Q1.datafile", O_RDONLY, 0);

    if(fd >= 0) {        //  IF FILE OPENED SUCCESSFULLY
        for(int32_t i = -50 ; i<50 ; i++) {
            int32_t value;

            read(fd, &value, sizeof(value));
            // printf formatting: %i == integer, \t == tab space, "0x", %08x (just %8 means theres 8 spaces before the 'x'. The x means print in lower case hexadecimal, the 0 replaces the spaces with 0) 
            printf("%i\t0x%08x\t%i\t0x%08x\n", i, i, value, value);
        }
        close(fd);
    }
}
