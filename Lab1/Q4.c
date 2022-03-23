//  Compile me with:
//      cc -std=c99 -Wall -Werror -pedantic -o  Q4-soln  Q4-soln.c

//  This version is the same as Q2-soln.c
//  but gets its data from a large file, rather than generating random data.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>


/*  We use textual inclusion here to obtain the checksum files - saves
    typing, and the need for more complicated compilation and linking.

    DON'T DO THIS IN MORE SIGNIFICANT PROGRAMS.
 */

#include "./checksum_ccitt.c"
#include "./checksum_crc16.c"
#include "./checksum_internet.c"


//  VALUES DEFINING THE 'SIZE' OF OUR EXPERIMENTS
#define FRAMESIZE       100
#define NFRAMES         1000000


#define BIGFILE         "/usr/bin/crash"
int	data_fd;

//  CORRUPT A FRAME WITH A BURST ERROR
void corrupt_frame(unsigned char frame[], int length)
{
#define MIN_BURSTLENGTH         10
#define MAX_BURSTLENGTH         100
    int nbits           = (length * NBBY);
    while(true) {
        int     b0      = rand() % nbits;
        int     b1      = rand() % nbits;
	int	burst	= b1 - b0;

        if(burst >= MIN_BURSTLENGTH && burst <= MAX_BURSTLENGTH) {
            for(int b=b0 ; b<b1 ; ++b) {
                int     byte    = b / NBBY;
                int     bit     = b % NBBY;

                frame[byte]     = (frame[byte] | (1UL << bit));
            }
            break;
        }
    }
}

//  TIMES THE EXECUTION OF SECTIONS OF CODE IN MICROSECONDS
int64_t timing(bool start)
{
    static      struct timeval startw, endw;
    int64_t     usecs   = 0;

    if(start) {
        gettimeofday(&startw, NULL);
    }
    else {
        gettimeofday(&endw, NULL);
        usecs   =
               (endw.tv_sec  - startw.tv_sec)*1000000 +
               (endw.tv_usec - startw.tv_usec);
    }
    return usecs;
}


// PERFORM 'NFRAMES' TESTS OF THE PROVIDED CHECKSUM FUNCTION, REPORT RESULTS
void evaluate(unsigned short (*fn)(), char *name)
{
    unsigned char       frame[FRAMESIZE];
    int                 nfailures = 0, checksum;

/*  We must ensure that our tests for each checksum algorithm are performed
    with the same data - otherwise the comparison will be unfair.
 */

    lseek(data_fd, 0, SEEK_SET);			// rewind file

//  START TIMING
    timing(true);

//  PERFORM TESTS FOR A LARGE NUMBER OF FRAMES
    for(int n=0 ; n<NFRAMES ; ++n) {

//  POPULATE THE FRAME WITH BYTES FROM THE DATA FILE
//  IF WE DON'T GET ALL WE ASK FOR, WE HAVE REACHED THE END OF THE FILE
        while(read(data_fd, (char *)frame, FRAMESIZE) != FRAMESIZE) {
            lseek(data_fd, 0, SEEK_SET);
	}

//  CALCULATE THE CHECKSUM BEFORE "TRANSMISSION"
        checksum        = (*fn)(frame, FRAMESIZE);

//  CORRUPT THE FRAME (FAKE TRANSMISSION)
        corrupt_frame(frame, FRAMESIZE);

//  IF THE BEFORE AND AFTER CHECKSUMS MATCH, WE HAVE A FAILURE.
        if(checksum == (*fn)(frame, FRAMESIZE))
            ++nfailures;
    }

//  REPORT RESULTS AND THE TIME TAKEN
    printf("%18s: %5d failures (%6.4f%%), %6lld msecs\n",
	name, nfailures, nfailures*100.0 / NFRAMES, timing(false)/1000);
}


int main(void)
{
    extern void         srand(unsigned int seed);

    srand(getpid());

/*  We need a large stream of data from somewhere.
    We continually read from BIGFILE to provide an infinite stream of
    data for our tests.  Alternatively, we could generate some random data
    and fill a buffer with it.
 */

    if((data_fd = open(BIGFILE, O_RDONLY, 0)) < 0) {
        fprintf(stderr,"Cannot open %s\n", BIGFILE);
        exit(EXIT_FAILURE);
    }
    printf("Data from \"%s\" framesize=%d, #frames=%d\n",
		BIGFILE, FRAMESIZE, NFRAMES);

/*  The following 3 calls to evaluate() pass the address of a checksum function
    as their first parameter.  This "trick" is a bit advanced, so for a bit
    more typing, we could make a call to each checksum function for each frame.
 */
    evaluate(checksum_ccitt,    "checksum_ccitt");
    evaluate(checksum_crc16,    "checksum_crc16");
    evaluate((unsigned short (*)())checksum_internet,   "checksum_internet");

    close(data_fd);

    exit(EXIT_SUCCESS);
}
