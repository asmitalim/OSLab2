BUILDHOST=`uname -n`
MAKE_ASMFS_HOST=${ASMFS_HOST}
MAKE_ASMFS_USER=${ASMFS_USER}

all: asmitaapp bbfs asmfs asmitaremotetest remotescp
	@echo "............................."
	@echo "Building on:$(BUILDHOST)"
	@echo "Asmita app is built"
    


asmitaapp: asmitaapp.o 
	echo "asmitaapp"
	gcc -o asmitaapp asmitaapp.o

asmitaremotetest: asmitaremotetest.o 
	echo "asmitaremotehost"
	gcc -o asmitaremotetest asmitaremotetest.o


.c.o: 
	gcc -c $<

clean: 
	rm -f *.o
	rm -f asmitaapp
	rm -f bbfs
	rm -f asmfs
	rm -f asmitaremotetest
	rm -f remotescp

asmitafs: asmitaapp 
	echo "mounting and running the UT IO test"
	make mountasmfs
	echo "Asmita app " > ./localroot/foo
	cd ./mountdir; ../asmitaapp

asmitaremotefs: asmitaremotetest
	echo "asmitaremotefs"
	make mountasmfs
	echo "User Message coming from local machine" > ./localroot/foo
	echo "User Message coming from remote machine" > /tmp/remotejunk
	scp /tmp/remotejunk "${ASMFS_USER}@${ASMFS_HOST}:~/asmfsexports/foo"
	rm -f /tmp/remotejunk
	cd ./mountdir; ../asmitaremotetest



mountasmfs:
	rm -f ./asmfs.log
	./asmfs ${ASMFS_HOST} ${ASMFS_USER} localroot mountdir
	
umountasmfs:
	fusermount -u mountdir

bbfs: bbfs.o log.o
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2  -D$(BUILDHOST)  -o bbfs bbfs.o log.o -lfuse -pthread

bbfs.o: bbfs.c
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2  -D$(BUILDHOST)  -c bbfs.c -pthread

log.o: log.c
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2  -D$(BUILDHOST)  -c log.c -pthread

asmfs: asmfs.o log.o remotescp.o
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2   -o asmfs asmfs.o log.o remotescp.o -lfuse -lcurl -pthread 

asmfs.o: asmfs.c
	gcc -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -g -O2  -D$(BUILDHOST)  -c asmfs.c -pthread

remotescp.o: remotescp.c
	gcc -D_FILE_OFFSET_BITS=64 -D$(BUILDHOST)  -I/usr/include/fuse -g -O2   -c remotescp.c -pthread

remotescp_debug.o: remotescp.c
	gcc -D_FILE_OFFSET_BITS=64 -D$(BUILDHOST) -DREMOTESCPDEBUG -I/usr/include/fuse -g -O2 -o remotescp_debug.o  -c remotescp.c -pthread

remotescp:  remotescp_debug.o
	gcc  -D_FILE_OFFSET_BITS=64 -D$(BUILDHOST) -DREMOTESCPDEBUG -I/usr/include/fuse -g -O2 -o remotescp  remotescp_debug.o -pthread -lcurl



