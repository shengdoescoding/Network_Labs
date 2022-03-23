#include <stdlib.h> // Standard Library
#include <stdint.h> // Typedef int32_t and more
#include <fcntl.h> // Imports open() function
//#include <unistd.h> // Defines miscellaneous symbolic constants and types

int main(int argc, char const *argv[])
{
    int  fd = open("Q1.datafile", O_CREAT|O_WRONLY, 0600);

    if(fd >= 0) {        //  IFF FILE OPENED SUCCESSFULLY
        for(int32_t i = -50 ; i<50 ; i++) {
            write(fd, &i, sizeof(i));
        }
        close(fd);
    }
    
    return 0;
}
