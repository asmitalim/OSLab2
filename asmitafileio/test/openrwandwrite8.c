#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>


/* simple test for opening a file if it exists in remote machine */
/* file exists return the number of bytes in the buffer */
/* if file do not exist return err */

/* now multiple  times read until eof */


char buf[100];

int
main(int argc, char **argv) {
    int fd ;
    int bytesRead ;
	long n ;
	char *strPtr = buf ; 

    if( argc < 2 ) {
        printf("Usage: %s <filename>\n",argv[0]);
        exit(1);
    }

    fd = open(argv[1], O_RDWR);
    if( fd < 0 ) {
        perror("open():");
        exit(2);
    }


	sprintf(buf, "asmitali");

	n = write(fd,buf,strlen(strPtr));
	if( n < 0) {
		perror("write()");
		exit(1);
	}


    printf("Total Bytes written = %ld \n",n);




    close(fd);
    exit(0);

}
