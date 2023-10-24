#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


#include <time.h>

#define MAXNUMBER 100000000L
#define MAXNUMBER     50000L
#define MAXNUMBER   5000000L

#define NBYTES		   4096
#define NBLOCKS         100

// 400K
// 4000K - 4M
// 400MB
#define NBLOCKS1 	   100
#define NBLOCKS2 	  1000
#define NBLOCKS3 	100000


int
randomtestsmallsize(long maxnumber,int readpercentage, int tasknumber);
int 
sequentialsmallsize(long maxnumber,int readpercentage, int tasknumber);

void doRandomTask0(int fd, char *charbuf, int rw, int idx) ;
void doRandomTask1(int fd, char *charbuf, int rw, int idx) ;
void doSequentialTask0(int fd, char *charbuf, int rw, int idx) ;



char readbuf[NBYTES];
char writebuf[NBYTES];


int globalNBlocks = NBLOCKS ; 



int pickupreadorwrite(int readpercentage)
{
    /* read 0 */
    /* write 1 */

    int randval = rand()%100 ;
    if( randval < readpercentage)
        return 0 ;
    else
        return 1 ;
}

int randompickindex()
{
    int idx = rand() % globalNBlocks ;
    return idx ;
}

int getnextindex()
{
	static int idx = 0 ; 
    return (idx++%globalNBlocks) ;
}



double timespent(clock_t endtick, clock_t starttick) {
	return 1.0*(endtick-starttick)/(double)CLOCKS_PER_SEC ; 
}
	

int blocks[] = { NBLOCKS1, NBLOCKS2, NBLOCKS3 } ;
int N = 3; 

int main(int argc, char *argv[]) {	
	clock_t tick0 ;
	clock_t tick1 ;
	double casetime ; 


	for ( int loopcount = 0 ; loopcount <  N ; loopcount ++) {	

				int currentNBlock = blocks[loopcount] ;

				globalNBlocks = currentNBlock ; 
				double  currentFileSize = currentNBlock * NBYTES /1000000.0 ; 

					/*
					tick0 = clock(); randomtestsmallsize(MAXNUMBER,10,0); tick1 = clock();
					casetime= timespent(tick1,tick0);
					printf("Random, filesize:%5.1lf Mbytes, task0 ,%ld, %d, blocks:%d, time:%5.3lf\n",currentFileSize,MAXNUMBER, 10, globalNBlcoks, casetime);
					printf("\n");


					tick0 = clock(); randomtestsmallsize(MAXNUMBER,50,0); tick1 = clock();
					casetime = timespent(tick1,tick0);
					printf("Random, filesize:%5.1lf MBytes, task0, %ld, %d, blocks:%d, time:%5.3lf\n",currentFileSize,MAXNUMBER, 50, globalNBlcoks, casetime);
					printf("\n");

					tick0 = clock(); randomtestsmallsize(MAXNUMBER,90,0); tick1 = clock();
					casetime = timespent(tick1,tick0);
					printf("Random, filesize:%5.1lf MBytes, task0, %ld, %d, blocks:%d, time:%5.3lf\n",currentFileSize,MAXNUMBER, 70, globalNBlcoks, casetime);
					printf("\n");
					*/

					tick0 = clock(); randomtestsmallsize(MAXNUMBER,0,1); tick1 = clock();
					casetime = timespent(tick1,tick0);
					printf("Random, filesize:%5.1lf MBytes,  task1, %ld, %d, blocks %d, time:%5.3lf\n",currentFileSize,MAXNUMBER, 0, globalNBlocks, casetime);
					printf("\n");

					tick0 = clock(); randomtestsmallsize(MAXNUMBER,50,1); tick1 = clock();
					casetime = timespent(tick1,tick0);
					printf("Random, filesize:%5.1lf MBytes, task1, %ld, %d, blocks:%d, time:%5.3lf\n",currentFileSize,MAXNUMBER, 50,  globalNBlocks, casetime);
					printf("\n");

					tick0 = clock(); 
					randomtestsmallsize(MAXNUMBER,100,1); tick1 = clock();
					casetime = timespent(tick1,tick0);
					printf("Random, filesize:%5.1lf MBytes, task1, %ld, %d, blocks:%d, time:%5.3lf\n",currentFileSize,MAXNUMBER, 100, globalNBlocks, casetime);
					printf("\n");
					

					tick0 = clock(); sequentialsmallsize(MAXNUMBER,0,0); tick1 = clock();
					casetime = timespent(tick1,tick0);
					printf("Sequential, filesize:%5.1lf MBytes, task0, %ld, %d, blocks:%d, time:%5.3lf\n",currentFileSize,MAXNUMBER, 0, globalNBlocks, casetime);
					printf("\n");

					tick0 = clock(); sequentialsmallsize(MAXNUMBER,100,0); tick1 = clock();
					casetime = timespent(tick1,tick0);
					printf("Sequential, filesize:%5.1lf MBytes, task0, %ld, %d, blocks:%d, time:%5.3lf\n",currentFileSize,MAXNUMBER, 100, globalNBlocks, casetime);
					printf("\n");
	}
}

int indexc[NBLOCKS3] ;

/*  maxnumber is loop count, readpercentage in int 0-100 */
int
randomtestsmallsize(long maxnumber,int readpercentage, int tasknumber)
{

    time_t t ;

    int xx ;
	unsigned long x ; 
    int i ;
    int rw;
    int rwc[2] ;

	for(int m =0 ; m < 2 ; m++) rwc[m] = 0 ;
	for(int m =0 ; m < NBLOCKS3 ; m++) indexc[m] = 0 ;


	


	int nb1, nb2 ; 

	int fd1 ;
	

    // seeding so that each time you get a variation
    srand( (unsigned )time(&t));

    fd1 = open("foo", O_RDWR);
    if(fd1<0) {
        perror("Error in opening the file fd1");
        exit(-1);
    }


	for( int bufindex = 0 ; bufindex < NBYTES ; bufindex ++) {
		readbuf[bufindex] = 0x0 ; 
		writebuf[bufindex] = bufindex%256 ; 
	}

    lseek(fd1,0L,SEEK_SET);
    nb2 = write(fd1,writebuf,NBYTES);
	//fprintf(stderr,"Initial bytes written %d\n",nb2);

    for( x = 0 ; x < maxnumber ; x++) {

		if ( (x+1) % 1000000 == 0) fprintf(stderr,"%5.2lf %%\n",(double)100.0 *x/maxnumber);
        i = randompickindex();
        rw = pickupreadorwrite(readpercentage) ;
        indexc[i] += 1 ;
        rwc[rw] += 1 ;



		if( tasknumber == 0 ) 
			doRandomTask0(fd1, writebuf, rw, i) ;
		if( tasknumber == 1 ) 
			doRandomTask1(fd1, writebuf, rw, i) ;

    }


    fprintf(stderr,"r:%5.2f  w:%5.2f  13th index: %5.3f\n",100.0*rwc[0]/maxnumber, 100.0*rwc[1]/maxnumber, 100.0*indexc[13]/(maxnumber));

    close(fd1);
    return 0;
}


int
sequentialsmallsize(long maxnumber,int readpercentage, int tasknumber)
{

    time_t t ;

    int xx ;
	unsigned long x ; 
    int i ;
    int rw;
    int rwc[2] ;
    //int indexc[NBLOCKS3] ;

	for(int m =0 ; m < 2 ; m++) rwc[m] = 0 ;
	for(int m =0 ; m < NBLOCKS3 ; m++) indexc[m] = 0 ;



	if (readpercentage > 50) 
		rw = 0 ;
	else 
		rw = 1 ;
	


	int nb1, nb2 ; 

	int fd1 ;
	

    // seeding so that each time you get a variation
    srand( (unsigned )time(&t));

    fd1 = open("foo", O_RDWR);
    if(fd1<0) {
        perror("Error in opening the file fd1");
        exit(-1);
    }


	for( int bufindex = 0 ; bufindex < NBYTES ; bufindex ++) {
		readbuf[bufindex] = 0 ;
		writebuf[bufindex] = bufindex%256 ; 
	}


    lseek(fd1,0L,SEEK_SET);
    nb2 = write(fd1,writebuf,NBYTES);
	//fprintf(stderr,"Initial bytes written %d\n",nb2);

    for( x = 0 ; x < maxnumber ; x++) {

		//if( x %10000 == 9999) fprintf(stderr,".");
		if ( (x+1) % 1000000 == 0) fprintf(stderr,"%5.2lf %%\n",(double)100.0 *x/maxnumber);

		i = getnextindex();
        indexc[i] += 1 ;
        rwc[rw] += 1 ;



		if( tasknumber == 0 ) 
			doSequentialTask0(fd1, writebuf, rw, i) ;
		if( tasknumber == 1 ) 
			doSequentialTask0(fd1, writebuf, rw, i) ; // add task for read only
    }


    fprintf(stderr,"r:%5.2f  w:%5.2f  13th index: %5.3f\n",100.0*rwc[0]/maxnumber, 100.0*rwc[1]/maxnumber, 100.0*indexc[13]/(maxnumber));

    close(fd1);
    return 0;
}








void doRandomTask0(int fd1, char *tbuf, int rw, int idx) {
	int nb1 , nb2;
	if(rw == 0 ) {
		lseek(fd1,1000L,SEEK_SET);
		nb1 = read(fd1, tbuf, NBYTES);
		//fprintf(stderr,"Read %d bytes\n", nb1);
		//assert(nb1 == NBYTES);
	}
	else {
		lseek(fd1,1000L,SEEK_SET);
		tbuf[0] = 51 ;
		tbuf[1] = 9  ; 
		tbuf[2] = 'A' ;
		tbuf[3] = 's' ;
		tbuf[4] = 'm' ;
		nb2 = write(fd1,tbuf,NBYTES);
		//fprintf(stderr,"Write %d bytes\n", nb2);
		//assert(nb2 == NBYTES);
	}
}

void doRandomTask1(int fd1, char *tbuf, int rw, int idx) {
	int nb1 , nb2;
	if(rw == 0 ) {
		lseek(fd1,NBYTES*idx+0L,SEEK_SET);
		nb1 = read(fd1, tbuf, NBYTES);
		//assert(nb1 == NBYTES);
	} else {
		lseek(fd1,NBYTES*idx+0L,SEEK_SET);
		nb2 = write(fd1,tbuf, NBYTES);
		//assert(nb2 == NBYTES);
	}
}

void doSequentialTask0(int fd1, char *tbuf, int rw, int idx) {
	int nb1 , nb2;
	if(rw == 0 ) {
		lseek(fd1,NBYTES*idx+0L,SEEK_SET);
		nb1 = read(fd1, tbuf, NBYTES);
		//assert(nb1 == NBYTES);
	} else {
		lseek(fd1,NBYTES*idx+0L,SEEK_SET);
		nb2 = write(fd1,tbuf,NBYTES);
		//assert(nb2 == NBYTES);
	}
}
