#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


/* simple test for opening a file if it exists in remote machine */
/* file exists return the number of bytes in the buffer */
/* if file do not exist return err */

/* now multiple  times read until eof */


char buf[100];

int
main(int argc, char **argv) {
    int fd ;
    int bytesRead ;

    if( argc < 2 ) {
        printf("Usage: %s <filename>\n",argv[0]);
        exit(1);
    }

    fd = open(argv[1], O_RDONLY);
    if( fd < 0 ) {
        perror("open():");
        exit(2);
    }


    size_t offsetIndex = 0 ;
    while((bytesRead=read(fd,buf,100)) > 0) {

        //printf("Bytes read successfull %d\n", bytesRead);
        for( int x = 0 ; x < bytesRead; x++) {

            if( (x+10) % 10 == 0 ) printf("\n");

            printf("%ld-%x ,",offsetIndex+x,buf[x]);
        }
        printf("\n");
        offsetIndex += bytesRead ;
    }

    printf("Total Bytes Read = %ld \n",offsetIndex);

    if(bytesRead <  0 ) {
        perror("read()");
        exit(3);
    }
    if(bytesRead ==  0 ) {
        perror("read():reached eof\n");

        exit(3);
    }



    close(fd);
    exit(0);





    int fd0,fd1 ;
    int nb0, nb1 ;
    long ls = lseek(fd0,0L,SEEK_SET);


    printf("seeked to %ld\n",ls);
    nb0 = write(fd0, buf, 100);
    printf("Wrote %d, then xx  bytes\n", nb0);
    close(fd0);
    lseek(fd1,100L,SEEK_SET);
    nb1 = read(fd1, buf, 100);
    assert(buf[0] == 9);
    assert(buf[1] == 81);
    assert(buf[2] == 'A');
    assert(buf[3] == 'q');
    assert(buf[4] == '0');
    nb1 = read(fd1, buf, 100);
    close(fd1);
    printf("Wrote %d, then %d bytes\n", nb0, nb1);
    return 0;
}
