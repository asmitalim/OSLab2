#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


#include <time.h>

#define MAXNUMBER 100000000L
#define MAXNUMBER  5000000L

int
randomtestsmallsize(long maxnumber,int readpercentage, int tasknumber);
int 
sequentialsmallsize(long maxnumber,int readpercentage, int tasknumber);

void doRamdomTask0(int fd, char *charbuf, int rw, int idx) ;
void doRamdomTask1(int fd, char *charbuf, int rw, int idx) ;
void doSequentialTask0(int fd, char *charbuf, int rw, int idx) ;

char buf[100000];

char tempbuf[100] ;
char *bufblocks = &buf[0] ;



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
    int idx = rand() % 100 ;
    return idx ;
}

int getnextindex()
{
	static int idx = 0 ; 
    return (idx++%100) ;
}



double timespent(clock_t endtick, clock_t starttick) {
	return 1.0*(endtick-starttick)/(double)CLOCKS_PER_SEC ; 
}
	

int main() {	
	clock_t tick0 ;
	clock_t tick1 ;
	double casetime ; 
	fprintf(stderr,"running test\n");
	fprintf(stderr,"simple test\n");

	/*
	tick0 = clock(); randomtestsmallsize(MAXNUMBER,10,0); tick1 = clock();
	casetime= timespent(tick1,tick0);
	printf("Random, Small, samelocation, task0 ,%ld, %d, %5.3lf\n",MAXNUMBER, 10, casetime);


	tick0 = clock(); randomtestsmallsize(MAXNUMBER,50,0); tick1 = clock();
	casetime = timespent(tick1,tick0);
	printf("Random, Small, samelocation, task0, %ld, %d, %5.3lf\n",MAXNUMBER, 50, casetime);

	tick0 = clock(); randomtestsmallsize(MAXNUMBER,90,0); tick1 = clock();
	casetime = timespent(tick1,tick0);
	printf("Random, Small, samelocation, task0, %ld, %d, %5.3lf\n",MAXNUMBER, 70, casetime);

	tick0 = clock(); randomtestsmallsize(MAXNUMBER,10,1); tick1 = clock();
	casetime = timespent(tick1,tick0);
	printf("Random, Small, samelocation, task1, %ld, %d, %5.3lf\n",MAXNUMBER, 10, casetime);

	tick0 = clock(); randomtestsmallsize(MAXNUMBER,50,1); tick1 = clock();
	casetime = timespent(tick1,tick0);
	printf("Random, Small, samelocation, task1, %ld, %d, %5.3lf\n",MAXNUMBER, 50, casetime);

	tick0 = clock(); 
	randomtestsmallsize(MAXNUMBER,90,1); tick1 = clock();
	casetime = timespent(tick1,tick0);
	printf("Random, Small, samelocation, task1, %ld, %d, %5.3lf\n",MAXNUMBER, 70, casetime);
	*/

	tick0 = clock(); sequentialsmallsize(MAXNUMBER,0,0); tick1 = clock();
	casetime = timespent(tick1,tick0);
	printf("Sequential, Small, samelocation, task0, %ld, %d, %5.3lf\n",MAXNUMBER, 0, casetime);

	tick0 = clock(); sequentialsmallsize(MAXNUMBER,100,0); tick1 = clock();
	casetime = timespent(tick1,tick0);
	printf("Sequential, Small, samelocation, task0, %ld, %d, %5.3lf\n",MAXNUMBER, 100, casetime);
}


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
    int indexc[100] ;

	for(int m =0 ; m < 2 ; m++) rwc[m] = 0 ;
	for(int m =0 ; m < 100 ; m++) indexc[m] = 0 ;




	int nb1, nb2 ; 

	int fd1 ;
	

    // seeding so that each time you get a variation
    srand( (unsigned )time(&t));

    fd1 = open("foo", O_RDWR);
    if(fd1<0) {
        perror("Error in opening the file fd1");
        exit(-1);
    }


	for( xx = 0 ; xx < 100 ; xx ++) {
		buf[xx] = xx ; 
	}
    for( int yy =0 ; yy < 100 ; yy++) {
        for( xx = 0 ; xx < 100 ; xx ++) {
            bufblocks[yy*100+xx] = yy ;
        }
    }

    lseek(fd1,0L,SEEK_SET);
    nb2 = write(fd1,tempbuf,100);
	fprintf(stderr,"Initial bytes written %d\n",nb2);

    for( x = 0 ; x < maxnumber ; x++) {

		//if( x %10000 == 9999) fprintf(stderr,".");
		if ( x % 1000000 == 999999) fprintf(stderr,"%5.2lf %%\n",(double)100.0 *x/maxnumber);
        i = randompickindex();
        rw = pickupreadorwrite(readpercentage) ;
        indexc[i] += 1 ;
        rwc[rw] += 1 ;



		if( tasknumber == 0 ) 
			doRamdomTask0(fd1, tempbuf, rw, i) ;
		if( tasknumber == 1 ) 
			doRamdomTask1(fd1, tempbuf, rw, i) ;

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
    int indexc[100] ;

	for(int m =0 ; m < 2 ; m++) rwc[m] = 0 ;
	for(int m =0 ; m < 100 ; m++) indexc[m] = 0 ;



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


	for( xx = 0 ; xx < 100 ; xx ++) {
		buf[xx] = xx ; 
	}
    for( int yy =0 ; yy < 100 ; yy++) {
        for( xx = 0 ; xx < 100 ; xx ++) {
            bufblocks[yy*100+xx] = yy ;
        }
    }

    lseek(fd1,0L,SEEK_SET);
    nb2 = write(fd1,tempbuf,100);
	fprintf(stderr,"Initial bytes written %d\n",nb2);

    for( x = 0 ; x < maxnumber ; x++) {

		//if( x %10000 == 9999) fprintf(stderr,".");
		if ( x % 1000000 == 999999) fprintf(stderr,"%5.2lf %%\n",(double)100.0 *x/maxnumber);

		i = getnextindex();
        indexc[i] += 1 ;
        rwc[rw] += 1 ;



		if( tasknumber == 0 ) 
			doSequentialTask0(fd1, tempbuf, rw, i) ;
		if( tasknumber == 1 ) 
			doSequentialTask0(fd1, tempbuf, rw, i) ; // add task for read only
    }


    fprintf(stderr,"r:%5.2f  w:%5.2f  13th index: %5.3f\n",100.0*rwc[0]/maxnumber, 100.0*rwc[1]/maxnumber, 100.0*indexc[13]/(maxnumber));

    close(fd1);
    return 0;
}








void doRamdomTask0(int fd1, char *tbuf, int rw, int idx) {
	int nb1 , nb2;
	if(rw == 0 ) {
		lseek(fd1,1000L,SEEK_SET);
		nb1 = read(fd1, tbuf, 100);
		//fprintf(stderr,"Read %d bytes\n", nb1);
		//assert(nb1 == 100);
	}
	else {
		lseek(fd1,1000L,SEEK_SET);
		buf[0] = 51 ;
		buf[1] = 9  ; 
		buf[2] = 'A' ;
		buf[3] = 's' ;
		buf[4] = 'm' ;
		nb2 = write(fd1,tbuf,100);
		//fprintf(stderr,"Write %d bytes\n", nb2);
		//assert(nb2 == 100);
	}
}

void doRamdomTask1(int fd1, char *tbuf, int rw, int idx) {
	int nb1 , nb2;
	if(rw == 0 ) {
		lseek(fd1,100*idx+0L,SEEK_SET);
		nb1 = read(fd1, tbuf, 100L);
		//assert(nb1 == 100);
	} else {
		lseek(fd1,100*idx+0L,SEEK_SET);
		nb2 = write(fd1,&bufblocks[100*idx],100L);
		//assert(nb2 == 100);
	}
}

void doSequentialTask0(int fd1, char *tbuf, int rw, int idx) {
	int nb1 , nb2;
	if(rw == 0 ) {
		lseek(fd1,100*idx+0L,SEEK_SET);
		nb1 = read(fd1, tbuf, 100L);
		//assert(nb1 == 100);
	} else {
		lseek(fd1,100*idx+0L,SEEK_SET);
		nb2 = write(fd1,&bufblocks[100*idx],100L);
		//assert(nb2 == 100);
	}
}
