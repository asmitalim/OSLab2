all: asmitaapp bbfs asmfs
	echo "Asmita app is built"

asmitaapp: asmitaapp.o 
	gcc -o asmitaapp asmitaapp.o

.c.o: 
	gcc -c $<

clean: 
	rm -f *.o
	rm -f asmitaapp
	rm -f bbfs
	rm -f asmfs

asmitafs: asmitaapp
	make mountasmitafs
	echo "Asmita app " > /home/asmita/OSLab2/asmitafileio/localroot/foo
	cd /home/asmita/OSLab2/asmitafileio/mountdir; ../asmitaapp

mountasmitafs:
	rm -f /home/asmita/OSLab2/asmitafileio/bbfs.log
	/home/asmita/OSLab2/asmitafileio/asmfs nandihill 192.168.10.223 /home/asmita/OSLab2/asmitafileio/localroot /home/asmita/OSLab2/asmitafileio/mountdir
	
unmountasmitafs:
	fusermount -u /home/asmita/OSLab2/asmitafileio/mountdir

bbfs: bbfs.o log.o
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2   -o bbfs bbfs.o log.o -lfuse -pthread

bbfs.o: bbfs.c
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2   -c bbfs.c -pthread

log.o: log.c
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2   -c log.c -pthread

asmfs: asmfs.o log.o
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2   -o asmfs asmfs.o log.o -lfuse -pthread

asmfs.o: asmfs.c
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2   -c asmfs.c -pthread






