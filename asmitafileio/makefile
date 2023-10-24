BUILDHOST=`uname -n`
MAKE_ASMFS_HOST=${ASMFS_HOST}
MAKE_ASMFS_USER=${ASMFS_USER}

all: asmitaapp asmfs remotescp ammifs 
	@echo "............................."
	@echo "Building on:$(BUILDHOST)"
	@echo "Asmita app is built"
	cd test ; make -f makefile.test 




asmitaapp: asmitaapp.o 
	echo "asmitaapp"
	gcc -o asmitaapp asmitaapp.o

codingstandard: 
	astyle --style=google ammifs.c
	astyle --style=google ammireserve.c
	astyle --style=google log.c
	astyle --style=google remotescp.c
	astyle --style=google asmitaapp.c


.c.o: 
	gcc -c $<

clean: 
	rm -f *.o
	rm -f asmitaapp
	rm -f bbfs
	rm -f asmfs
	rm -f ammifs
	rm -f remotescp
	cd test ; make -f makefile.test testclean
	rm -f *.orig

asmitafs: asmitaapp 
	echo "mounting and running the UT IO test"
	make mountasmfs
	echo "Asmita app " > ./localroot/foo
	cd ./mountdir; ../asmitaapp

asmitaremotefs: asmitaremotetest
	echo "asmitaremotefs"
	make mountasmfs
	#echo "User Message coming from local machine" > ./localroot/foo
	#echo "User Message coming from remote machine" > /tmp/remotejunk
	#scp /tmp/remotejunk "${ASMFS_USER}@${ASMFS_HOST}:~/asmfsexports/foo"
	#rm -f /tmp/remotejunk
	cd ./mountdir; ../asmitaremotetest


memonlocalroot:
	mkdir -p /tmp/localroot
	sudo chmod 777 /tmp/localroot
	sudo mount -t tmpfs -o size=1024M localroot /tmp/localroot

unmountlocalroot:
	sudo umount /tmp/localroot

mount:
	rm -f ./asmfs.log
	./ammifs ${ASMFS_HOST} ${ASMFS_USER} /tmp/localroot mountdir


umount:
	fusermount -u mountdir


mountasmfs:
	rm -f ./asmfs.log
	./asmfs ${ASMFS_HOST} ${ASMFS_USER} localroot mountdir
	
umountasmfs:
	fusermount -u mountdir

ammifs: ammifs.o log.o remotescp.o ammireserve.o
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2  -D$(BUILDHOST)  -o ammifs ammifs.o log.o ammireserve.o remotescp.o -lfuse -lcurl 

ammifs.o: ammifs.c
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2  -D$(BUILDHOST)  -c ammifs.c 

ammireserve.o:	ammireserve.o
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2  -D$(BUILDHOST)  -c ammireserve.c 

log.o: log.c
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2  -D$(BUILDHOST)  -c log.c -pthread

asmfs: asmfs.o log.o remotescp.o
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2   -o asmfs asmfs.o log.o remotescp.o -lfuse -lcurl 

asmfs.o: asmfs.c
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2  -D$(BUILDHOST)  -c asmfs.c 

remotescp.o: remotescp.c
	gcc -D_FILE_OFFSET_BITS=64 -D$(BUILDHOST)  -I/usr/include/fuse -g -O2   -c remotescp.c 

remotescp_debug.o: remotescp.c
	gcc -D_FILE_OFFSET_BITS=64 -D$(BUILDHOST) -g -DREMOTESCPDEBUG -I/usr/include/fuse -g -O2 -o remotescp_debug.o  -c remotescp.c -pthread

remotescp:  remotescp_debug.o
	gcc  -D_FILE_OFFSET_BITS=64 -D$(BUILDHOST) -g -DREMOTESCPDEBUG -I/usr/include/fuse -g -O2 -o remotescp  remotescp_debug.o -pthread -lcurl




