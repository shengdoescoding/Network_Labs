//  written by Chris.McDonald@uwa.edu.au
//
//  Compile me with:
//      cc -std=c99 -Wall -Werror -pedantic -o Q2-soln Q2-soln.c

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <time.h>
#include <sys/time.h>
#include <sys/param.h>          // provides number-of-bits-per-byte  NBBY

//  VALUES DEFINING THE PARAMETERS OF OUR EXPERIMENTS
#define FRAMELEN                100
#define NFRAMES                 1000000

#define MIN_BURSTLEN            15
#define MAX_BURSTLEN            50


//  WE USE TEXTUAL INCLUSION HERE TO OBTAIN THE CHECKSUM FUNCTIONS -
//  SAVES THE NEED FOR MORE COMPLICATED COMPILATION AND LINKING.
//
//  DON'T DO THIS IN MORE SIGNIFICANT PROGRAMS!

#include "./checksum_ccitt.c"
#include "./checksum_crc16.c"
#include "./checksum_internet.c"

//  ---------------------------------------------------------------------

//  CORRUPT A FRAME WITH A BURST ERROR
//  WE EMPLOY THE setbit() MACRO FROM  <sys/param.h>
//  cf:  https://code.woboq.org/qt5/include/sys/param.h.html
void corrupt_frame(unsigned char frame[], int framelen, int burstlen)
{
    int nbits           = (framelen * NBBY);
    int firstbit        = rand() % (nbits - burstlen);  // 1st bit of burst

    for(int b=0 ; b<burstlen ; ++b) {
        setbit(frame, firstbit+b);
    }
}

//  TIMES THE EXECUTION OF SECTIONS OF CODE IN MICROSECONDS
int64_t microseconds(void)
{
    struct timeval now;

    gettimeofday(&now, NULL);
    return now.tv_sec*1000000 + now.tv_usec;
}

//  PERFORM 'NFRAMES' CALLS OF THE PROVIDED CHECKSUM FUNCTION
void evaluate(unsigned short (*function)(), char *name)
{
//  WE MUST ENSURE THAT OUR TESTS FOR EACH CHECKSUM ALGORITHM ARE PERFORMED
//  WITH THE SAME DATA - OTHERWISE THE COMPARISON WILL BE UNFAIR.
//  WE DO THIS BE RE-INITIALIZING THE RANDOM NUMBER GENERATOR EACH TIME.

    srand(getpid());

    unsigned char       frame[FRAMELEN];
    int nfailures = 0;

//  START TIMING
    int64_t start       = microseconds();

//  PERFORM TESTS FOR A LARGE NUMBER OF FRAMES
    for(int n=0 ; n<NFRAMES ; ++n) {

//  POPULATE THE FRAME WITH RANDOM BYTES
        for(int i=0 ; i<sizeof(frame) ; ++i) {
            frame[i] = rand() % 254;
        }

//  CALCULATE THE CHECKSUM BEFORE "TRANSMISSION"
        int checksum_before     = (*function)(frame, sizeof(frame));

//  CORRUPT THE FRAME (FAKE THE TRANSMISSION)
        int burstlen    = MIN_BURSTLEN + rand() % (MAX_BURSTLEN - MIN_BURSTLEN);
        corrupt_frame(frame, sizeof(frame), burstlen);

//  IF THE BEFORE AND AFTER CHECKSUMS MATCH, WE HAVE A FAILURE.
        int checksum_after      = (*function)(frame, sizeof(frame));

        if(checksum_before == checksum_after) {
            ++nfailures;
        }
    }

//  REPORT RESULTS AND THE TIME TAKEN
    printf("Generating random data, framelen=%i, #frames=%i\n",
                FRAMELEN, NFRAMES);
    printf("%18s: %5d failures (%6.4f%%), %6ld msecs\n",
                name, nfailures, nfailures*100.0 / NFRAMES,
                (microseconds() - start)/1000);
}


int main(void)
{
//  THE 3 CALLS TO evaluate() PASS THE ADDRESS OF A CHECKSUM FUNCTION
//  AS THEIR FIRST PARAMETER.  FOR A BIT MORE TYPING WE COULD, ALTERNATIVELY,
//  MAKE A CALL TO EACH FUNCTION FOR EACH FRAME.

    evaluate(checksum_ccitt,    "checksum_ccitt");

    evaluate(checksum_crc16,    "checksum_crc16");

    evaluate((unsigned short (*)())checksum_internet,   "checksum_internet");

    exit(EXIT_SUCCESS);
}
