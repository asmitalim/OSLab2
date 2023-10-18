#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


#include <time.h>

char buf[10000];


/*
10 % read and
90 % write


100 block 100bytes each
randomly pick index of these 100 block

*/


int pickupreadorwrite()
{
    /* read 0 */
    /* write 1 */

    int randval = rand()%100 ;
    if( randval > 90)
        return 1 ;
    else
        return 0 ;
}

int randompickindex()
{
    int idx = rand() % 100 ;
    return idx ;
}



#define MAXNUMBER 100000000L


int
main()
{

    time_t t ;

    int xx ;
	unsigned long x ; 
    int i ;
    int rw;
    static int rwc[2] ;
    static int indexc[100] ;

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

	lseek(fd1,0L,SEEK_SET);
	nb2 = write(fd1,buf,100);

    for( x = 0 ; x < MAXNUMBER ; x++) {

		if( x %10000 == 9999) printf(".");
		if ( x % 1000000 == 999999) printf("%5.2lf %%\n",(double)100.0 *x/MAXNUMBER);
        i = randompickindex();
        rw = pickupreadorwrite() ;
        indexc[i] += 1 ;
        rwc[rw] += 1 ;




		if(rw == 0 ) {
			lseek(fd1,0L,SEEK_SET);
    		nb1 = read(fd1, buf, 100);
    		//printf("Read %d bytes\n", nb1);
			assert(nb1 == 100);
		}
		else {
			lseek(fd1,0L,SEEK_SET);
			nb2 = write(fd1,buf,100);
    		//printf("Write %d bytes\n", nb2);
			assert(nb2 == 100);
		}

    }


    printf("r:%5.2f  w:%5.2f  13th index: %5.3f\n",100.0*rwc[0]/MAXNUMBER, 100.0*rwc[1]/MAXNUMBER, 100.0*indexc[13]/(MAXNUMBER));



    close(fd1);
    exit(1);
    return 0;
}
