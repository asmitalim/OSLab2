#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

char buf[5000];


int doit(char *x);

int main() {
	doit("foo");
	doit("asmfsinfo.txt");
}


int doit(char *filename) {


   int fd1 = open(filename, O_RDONLY);
   if(fd1<0)
   {
	  fprintf(stderr,"Error reading %s\n\n",filename);
      perror("open");
      exit(-1);
   }
   int nb1 = read(fd1, buf, 100);
   close(fd1);

   printf("Printing the %s\n",filename);
   printf("Read %d bytes\n", nb1);

   for ( int x = 0 ; x < nb1 ; x++) {
   		fprintf(stderr,"%0x ,",buf[x]);
		if( x % 10 == 9) printf("\n");
   }

   return 0;
}
