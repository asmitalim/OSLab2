#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

char buf[5000];
int
main() {
   int fd1 = open("foo", O_RDONLY);
   if(fd1<0)
   {
      perror("Error in reading fd1");
      exit(-1);
   }
   int nb1 = read(fd1, buf, 100);
   close(fd1);
   printf("Read %d bytes\n", nb1);
   return 0;
}