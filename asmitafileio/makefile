BUILDHOST=`uname -n`
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
	make mountasmitafs
	echo "Asmita app " > /home/asmita/OSLab2/asmitafileio/localroot/foo
	cd /home/asmita/OSLab2/asmitafileio/mountdir; ../asmitaapp

asmitaremotefs: asmitaremotetest
	echo "asmitaremotefs"
	make mountasmitafs
	echo "Asmita remote test coming from local machine" > /home/asmita/OSLab2/asmitafileio/localroot/foo
	echo "Asmita remote test coming from remote machine" > /tmp/remotejunk
	scp /tmp/remotejunk master0:/home/asmita/lab2exports/foo
	cd /home/asmita/OSLab2/asmitafileio/mountdir; ../asmitaremotetest



mountasmitafs:
	rm -f /home/asmita/OSLab2/asmitafileio/bbfs.log
	/home/asmita/OSLab2/asmitafileio/asmfs master0 192.168.10.223 /home/asmita/OSLab2/asmitafileio/localroot /home/asmita/OSLab2/asmitafileio/mountdir
	
unmountasmitafs:
	fusermount -u /home/asmita/OSLab2/asmitafileio/mountdir

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



