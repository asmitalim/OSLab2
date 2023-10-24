/*


	ammifs.c



	credit: This file is baded on the Big Brother file system   "bbfs.c"


*/




#include "config.h"
#include "params.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"
#include "remotescp.h"

static void asm_fullpath(char fpath[PATH_MAX], const char *path) {
    strcpy(fpath, ASM_DATA->rootdir);
    strncat(fpath, path, PATH_MAX);

    log_msg("    asm_fullpath:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n",
            ASM_DATA->rootdir, path, fpath);
}





#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>


#define MAXCACHEFILESIZE 8192

#define TABLESIZE 256
#define MAXNAMESIZE 1024


int directoryIndex = -1;
char cachedDirectories[ TABLESIZE ][ MAXNAMESIZE ];
struct stat cachedDirStats[TABLESIZE] ;

int fileNameIndex = -1;
char cachedFileNames[ TABLESIZE ][ MAXNAMESIZE ];
struct stat cachedFileStats[TABLESIZE] ;

int  cachedFileIndex = -1;
char fileStuff[ TABLESIZE ][ MAXCACHEFILESIZE ];

typedef struct {
    int refcount ; // refcount
    int fds[10] ;  // open fds
    int rws[10] ;  // if RD or RDWR
    int fdcounts ; // valid numbers in open fds
} FileMetaData ;

FileMetaData  meta[TABLESIZE] ;



char dirStuffy[5000];

void addDirectoryEntry(const char *ptr, struct stat *statptr);
void addFileIntoCache( const char *ptr, struct stat *statptr);


void listAndRemoteDirNames() {
    int n = remotedirnames(ASM_DATA->remoteuser, ASM_DATA->remotehostname,"/", &dirStuffy[0]);

    char *str = dirStuffy ;
    struct stat statbuf ;

    if( n != 0 ) {
        int buflen = strlen(dirStuffy);
        char delim[] = "\n" ;

        char *ptr = strtok(str,delim);

        //log_msg("Getting all tokens split using slash n\n");

        while(ptr != NULL) {
            //log_msg("ptr[%s]\n",ptr);

            char remoteXName[300] ;
            sprintf(remoteXName,"/%s",ptr);


            //log_msg("REMOTEDIRNAME(): getting info for %s\n",ptr);
            int x = remotestat(ASM_DATA->remoteuser, ASM_DATA->remotehostname,ptr, &statbuf);
            //log_stat(&statbuf);


            if( x == 0 ) {
                if(statbuf.st_mode & S_IFREG )  {
                    log_msg("listAndRemoteDirNames():  %s is a file\n",ptr);
                    addFileIntoCache( ptr, &statbuf);
                } else if(statbuf.st_mode & S_IFDIR ) {
                    log_msg("listAndRemoteDirNames():  %s is a directory\n",ptr);
                    addDirectoryEntry(ptr, &statbuf);
                } else {
                    log_msg("listAndRemoteDirName():  %s is a unsupported file\n",ptr);
                }
            } else {

            }

            ptr = strtok(NULL, delim);
        }
    }
}





void addDirectoryEntry( const char *remoteDirName, struct stat *statptr ) {

    if(directoryIndex + 1 == TABLESIZE) return ;


    directoryIndex++;
    strcpy( cachedDirectories[ directoryIndex ], remoteDirName );

    memset(&cachedDirStats[ directoryIndex], 0, sizeof(struct stat));

    cachedDirStats[directoryIndex].st_mode = statptr->st_mode ;
    cachedDirStats[directoryIndex].st_atime = statptr->st_atime ;
    cachedDirStats[directoryIndex].st_mtime = statptr->st_mtime ;
    cachedDirStats[directoryIndex].st_size = statptr->st_size ;
    cachedDirStats[directoryIndex].st_blksize = statptr->st_blksize ;
    cachedDirStats[directoryIndex].st_uid = statptr->st_uid ;
    cachedDirStats[directoryIndex].st_gid = statptr->st_gid ;
    cachedDirStats[directoryIndex].st_nlink = statptr->st_nlink ;
}


/*
** Outstanding issues
**
**	open
		x implement
		x open with name.cached file , if already there , do not download again
		x if not there , download, create and update the remotestat

		- open read write keep handle in fuse.
		x second open read/write - error
		- second open readonly allowed - keep handle in cache.
		- when read or writ modify the direntry of the open file.
		- keep open reference count cache - fd and sequence of open called.

	read
		x implment pread to the local handle ( passthrough )

	write
		x implement write to the local handle ( pass through )

	close
		x for close without rw,  do not copy back
		x or for the close which is for the rw file, copy the file.
		- allow read to continue
		- if matching close do not copy back the file.
		x implement release
**
**
*/



int ifCachedDir( const char *path ) {
    if( directoryIndex == -1) return 0 ;

    char *ptr = (char *)path ;

    for ( int ii = 0; ii <= directoryIndex; ii++ )
        if ( strcmp( ptr+1, cachedDirectories[ ii ] ) == 0 )
            return 1;

    return 0;
}


void addFileIntoCache( const char *filename, struct stat *statptr ) {
    if(fileNameIndex+1 == TABLESIZE ) return ;

    fileNameIndex++;


    strcpy( cachedFileNames[ fileNameIndex ], filename );
    memset(&cachedFileStats[ fileNameIndex], 0, sizeof(struct stat));

    cachedFileStats[fileNameIndex].st_mode = statptr->st_mode ;
    cachedFileStats[fileNameIndex].st_atime = statptr->st_atime ;
    cachedFileStats[fileNameIndex].st_mtime = statptr->st_mtime ;
    cachedFileStats[fileNameIndex].st_size = statptr->st_size ;
    cachedFileStats[fileNameIndex].st_blksize = statptr->st_blksize ;
    cachedFileStats[fileNameIndex].st_uid = statptr->st_uid ;
    cachedFileStats[fileNameIndex].st_gid = statptr->st_gid ;
    cachedFileStats[fileNameIndex].st_nlink = statptr->st_nlink ;

    meta[fileNameIndex].fdcounts = 0 ;
    meta[fileNameIndex].refcount = 0 ;



    cachedFileIndex++;
    strcpy( fileStuff[ cachedFileIndex ], "" );
}

int isCachedFileName( const char *path ) {

    for ( int i = 0; i <= fileNameIndex; i++ )
        if ( strcmp( path+1, cachedFileNames[ i ] ) == 0 ) return 1;

    return 0;
}

int getFileIndex( const char *path ) {

    for ( int i = 0; i <= fileNameIndex; i++ )
        if ( strcmp( path+1, cachedFileNames[ i ] ) == 0 )
            return i;

    return -1;
}


int getDirectoryIndex( const char *path ) {

    for ( int i = 0; i <= directoryIndex; i++ )
        if ( strcmp( path+1, cachedDirectories[ i ] ) == 0 )
            return i;

    return -1;
}


size_t writeFile( const char *path, const char *stuff, size_t size ) {

    int fileIndex = getFileIndex( path );
    log_msg("writeFile:%s,size = %ld\n",size);

    if ( fileIndex == -1 ) return -1;


    if(size == 0 ) return 0 ;

    //strcpy( fileStuff[ fileIndex ], stuff );
    char *dptr = fileStuff[fileIndex] ;
    char *sptr = stuff ;

    size_t n = size ;

    for( size_t xx = 0 ; xx < size ; xx++) {
        *dptr++ = *sptr++ ;
    }


    return n ;

}


static int do_getattr( const char *path, struct stat *st ) {
    st->st_uid = getuid();
    st->st_gid = getgid();
    st->st_atime = time( NULL );
    st->st_mtime = time( NULL );

    if ( strcmp( path, "/" ) == 0 ) { // || ifCachedDir( path ) == 1 )
        st->st_mode = S_IFDIR | 0755;
        st->st_nlink = 2;
    } else if ( isCachedFileName( path ) == 1 ) {
        int fileIndex =  getFileIndex( path );

        if ( fileIndex == -1 ) {
            return -ENOENT;
        } else {
            st->st_mode = cachedFileStats[fileIndex].st_mode ;
            st->st_nlink = cachedFileStats[fileIndex].st_mode ;
            st->st_size = cachedFileStats[fileIndex].st_mode ;
            st->st_blksize = cachedFileStats[fileIndex].st_blksize ;
            st->st_size = cachedFileStats[fileIndex].st_size ;
            st->st_nlink = cachedFileStats[fileIndex].st_nlink ;
        }

    } else if ( ifCachedDir( path ) == 1 ) {
        int dirIndex =  getDirectoryIndex( path );

        if ( dirIndex == -1 ) {
            return -ENOENT;
        }
        st->st_mode = cachedDirStats[dirIndex].st_mode ;
        st->st_nlink = cachedDirStats[dirIndex].st_mode ;
        st->st_size = cachedDirStats[dirIndex].st_mode ;
        st->st_blksize = cachedDirStats[dirIndex].st_blksize ;
        st->st_size = cachedDirStats[dirIndex].st_size ;
        st->st_nlink = cachedDirStats[dirIndex].st_nlink ;
    } else {
        return -ENOENT;
    }

    return 0;
}


static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ) {
    filler( buffer, ".", NULL, 0 );
    filler( buffer, "..", NULL, 0 );

    if ( strcmp( path, "/" ) == 0 ) {
        for ( int i = 0; i <= directoryIndex; i++ )
            filler( buffer, cachedDirectories[ i ], &cachedDirStats[i], 0 );

        for ( int i = 0; i <= fileNameIndex; i++ )
            filler( buffer, cachedFileNames[ i ], &cachedFileStats[i], 0 );
    }

    return 0;
}

static int do_readinmem( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi ) {
    int fileIndex ;

    fileIndex = getFileIndex( path );
    size_t  canserveSize = size ;
    size_t  existingFileSize ;


    if ( fileIndex == -1 ) return -1;


    char *stuff = fileStuff[ fileIndex ];
    struct stat *statPtr = &cachedFileStats[fileIndex] ;

    if(offset >= MAXCACHEFILESIZE ) {
        canserveSize = 0 ;
    } else if(offset+size >= MAXCACHEFILESIZE ) {
        canserveSize = MAXCACHEFILESIZE - offset;
    } else {
        canserveSize = size ;
    }





    existingFileSize = statPtr->st_size ;


    size_t finalSize ;
    if(offset >= existingFileSize ) {
        finalSize = 0 ;
    } else if( offset+canserveSize >=existingFileSize) {
        finalSize = existingFileSize - offset ;
    } else {
        finalSize = canserveSize ;
    }

    log_msg("do_readinmem(): fileIndex:%d path: %s size:%ld offset:%ld \n",fileIndex,path, size, offset);
    //log_fi(fi);
    log_msg("stat->st_size = %ld\n",statPtr->st_size );

    log_msg("Serving size = %ld\n",canserveSize);
    log_msg("Final Serve size = %ld\n",finalSize);


    if ( finalSize == 0 ) return 0 ;




    /*

    	for ( int t = 0 ; t < MAXCACHEFILESIZE ; t++) {
    		stuff[t] = '\0' ;
    		switch(t) {
    			case 0 :
    				stuff[t] = 'x';
    				break ;
    			case 1 :
    				stuff[t] = 'y';
    				break ;
    			case 2 :
    				stuff[t] = 'z';
    				break ;
    			default:
    				//stuff[t] = 'A' + (t-3)%26 ;
    				stuff[t] = t%256 ;
    				break ;
    		}
    	}

    */

    /*
    memcpy( buffer, stuff + offset, size );
    return strlen( stuff ) - offset;
    */

    memcpy(buffer, stuff+offset,finalSize);

    /*
    	int n = (canserveSize >= 3 )? 3 : canserveSize ;

    	char *cptr = buffer ;
    	strncpy(buffer,"PHL",n);
    */



    return (int) finalSize ;
}

static int do_mkdir( const char *path, mode_t mode ) {
    struct stat st ;
    st.st_mode = 0755 | S_IFDIR ;
    st.st_size = 0 ;
    st.st_blksize = 4096 ;
    st.st_uid = getuid() ;
    st.st_gid = getgid() ;
    st.st_atime = time(NULL) ;
    st.st_mtime = time(NULL) ;
    st.st_nlink = 2 ;

    addDirectoryEntry( path+1, &st );
    return 0;
}

static int do_mknod( const char *path, mode_t mode, dev_t rdev ) {
    struct stat st ;
    st.st_mode = 0644 | S_IFREG ;  // replace with mode
    st.st_size = 0 ;
    st.st_blksize = 4096 ;
    st.st_uid = getuid() ;
    st.st_gid = getgid() ;
    st.st_atime = time(NULL) ;
    st.st_mtime = time(NULL) ;
    st.st_nlink = 1 ;

    addFileIntoCache( path+1,&st );

    return 0;
}

static int do_writeinmem( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info ) {

    int fileIndex ;

    fileIndex = getFileIndex( path );
    size_t  canWriteSize = size ;
    size_t  newFileSize ;

    log_msg("do_writeinmem(): fileIndex:%d path: %s size:%ld offset:%ld \n",fileIndex,path, size, offset);
    //log_fi(fi);

    if ( fileIndex == -1 ) return -1;

    char *stuff = fileStuff[ fileIndex ];
    struct stat *statPtr = &cachedFileStats[fileIndex] ;

    size_t existingFileSize = statPtr->st_size ;

    if(offset >= MAXCACHEFILESIZE ) {
        canWriteSize = 0 ;
        newFileSize = MAXCACHEFILESIZE ;
    } else if(offset+size >= MAXCACHEFILESIZE ) {
        canWriteSize = MAXCACHEFILESIZE - offset;
        newFileSize = MAXCACHEFILESIZE ;

    } else {
        canWriteSize = size ;
        newFileSize = offset+size ;
    }
    if(newFileSize < existingFileSize) {
        newFileSize = existingFileSize;
    }

    log_msg("do_writeinmem():existing stat->st_size = %ld\n",statPtr->st_size );
    log_msg("do_writeinmem():Can write size = %ld\n",canWriteSize);


    statPtr->st_size = newFileSize ;
    statPtr->st_mtime = time(NULL);
    statPtr->st_atime = time(NULL);

    log_msg("do_writeinmem():after modification stat->st_size = %ld\n",statPtr->st_size );

    /*
    memcpy( buffer, stuff + offset, size );
    return strlen( stuff ) - offset;
    */

    log_msg("do_writeinmem():Before copying offset:%ld size:%ld\n",offset,canWriteSize);

    if(canWriteSize == 0 ) return 0 ;

    memcpy(stuff+offset, buffer, canWriteSize);

    //int writtenSize = writeFile( path, buffer, canWriteSize );
    int writtenSize = canWriteSize ;

    return writtenSize ;
}



/*
**  do_open a preferred way of opening
*/
int do_open(const char *path, struct fuse_file_info *fi) {

    int retstat = 0; // the sys call return state
    int fd;
    int fdtemp;


    char fpath[PATH_MAX];
    char pathintemp[PATH_MAX];
    char* tptr = pathintemp;


    char fullremoteuri[PATH_MAX];
    char* uptr = fullremoteuri;


    log_msg("\ndo_open(path\"%s\", fi=0x%08x)\n", path, fi);
    //log_fi(fi);
	log_msg("do_open():flags = %x\n",fi->flags);



    asm_fullpath(fpath, path);



    sprintf(fullremoteuri,"scp://%s@%s/~/asmfsexports%s", ASM_DATA->remoteuser,ASM_DATA->remotehostname, path);
    sprintf(pathintemp, "%s.cached", fpath);

    //log_msg("do_open():cached content is in %s\n",pathintemp);
    //log_msg("do_open():remote content is in %s\n", fullremoteuri);

    // TODO  do_open() stat updated for the entry if copied from the remote machine


    // check if the cached file exists if yes then open and return ( do not copy using scp )
    //scpreadf(uptr,tptr);

    // open will fail if the remote file is not braught into the cache


    fdtemp = log_syscall("open", open(pathintemp, fi->flags), 0);
    //log_msg("do_open():file.cached : open(%s) return value %d\n",pathintemp,fdtemp);



    if (fdtemp < 0) {
        // this path for the case when .cached file is not there, but direntry is there ( otherwise open will not be called )
        //log_msg("do_open(): handling open failed with no <file>.cached \n");
        retstat = log_error("open(file.cached):");

        // one option is to call scp to get a copy from remote machine, handle the case where the remote file is not there, and cache file is there
        int scpOk = scpreadf(uptr,tptr);

        if( scpOk < 0 ) {
            // scp failed so return i suppose
            log_msg("do_open():scpread()");
        }


        // no file.cached and scp succeeded now we have some cached file so now we can try opening it.

        int secondfd = log_syscall("open", open(pathintemp, fi->flags), 0);
        if( secondfd < 0 )  {
            // strange case where thestrange when scp succeeded and we do not have .cached file");
            retstat = log_error("do_open(): scp succeeded and but  no .cached file");
        }

        // successfully scp, and open

        //log_msg("do_open(): (2) %s opened with %d\n",pathintemp,secondfd);

        fi->fh = secondfd ;
        //log_fi(fi);
		log_msg("do_open(): get .cached and then OPEN RETURNS %d\n",secondfd);

		if( secondfd == -1)
			return retstat ; 
		else 
			return 0 ; 



    } else {
        // Successfull as .cached file exists and opened
        // any further open will come here as .cached file existed.

        fi->fh = fdtemp;
        // log_msg("do_open():(first fd:%d) before returning\n",fdtemp);
        //log_fi(fi);
		//log_msg("do_open(): alreadyhave .cached OPEN RETURNS %d\n",fdtemp);
        return 0 ;
    }
    // unreachable but if reached so return -1
    return -1 ;
}







int do_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    /*
    **  this is the write function to be used
    */

    int retstat = 0;

	/*
    log_msg("\ndo_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
            path, buf, size, offset, fi);
	*/

    //log_fi(fi);

    //return log_syscall("pread", pread(fi->fh, buf, size, offset), 0);

    return  pread(fi->fh, buf, size, offset);


}



int do_write(const char *path, const char *buf, size_t size, off_t offset,
             struct fuse_file_info *fi) {


    /*
    **  this is the write function to be used
    **
    **	am i supposed to logged in
    */
    int retstat = 0;

	/*
    log_msg("\ndo_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
            path, buf, size, offset, fi);
	*/
    //log_fi(fi);

    //retstat = log_syscall("pwrite", pwrite(fi->fh, buf, size, offset), 0);
    // return retstat ;

    return pwrite(fi->fh, buf, size, offset) ;

}



int do_release(const char *path, struct fuse_file_info *fi) {
    int retvalue ;
    static int  backnumber = 0 ;
    char fullremoteuri[PATH_MAX];
    char *uptr = fullremoteuri ;


    char pathintemp[PATH_MAX];
    char *pathInTempPtr = pathintemp ;
    char *tptr = pathintemp ;


    char rmcmd[PATH_MAX];

    char fpath[PATH_MAX];

    /*
    *
    *    This is the one chosen
    */

    //todo if opened for read, do not writeback the file using scp
    // todo if opened for write, the writeback the file using scp

    // avoid multiple write


    log_msg("\ndo_release(path=\"%s\", fi=0x%08x)\n",
            path, fi);

    //log_fi(fi);
	log_msg("		flags = %x, handle = %d fh_old %d\n",fi->flags, fi->fh, fi->fh_old );
    asm_fullpath(fpath, path);


    // keep ready the paths for writing it back if needed.
    sprintf(fullremoteuri,"scp://%s@%s/~/asmfsexports%s", ASM_DATA->remoteuser,ASM_DATA->remotehostname, path);
    sprintf(pathInTempPtr, "%s.cached", fpath);



    //log_msg("flags = %x : O_RDWR:%x, O_RDONLY:%x O_WRONLY:%x O_TRUNC:%x, O_CREAT:%x\n",fi->flags,O_RDWR,  O_RDONLY, O_WRONLY, O_TRUNC, O_CREAT);


    if( (fi->flags & 0xff )  == O_RDONLY ) {
        // readonly so just call the close
        log_msg("do_release():flags = %x, releasing the fd with RDONLY\n",fi->flags);
        retvalue = log_syscall("close", close(fi->fh), 0 );


    } else if( (fi->flags & 0xff) == O_RDWR )  {
        log_msg("do_release():flags = %x, releasing the fd with RDWR\n",fi->flags);

        // sync the buffers back
        fsync(fi->fh);
        fsync(fi->fh);

        // close the file
        retvalue = log_syscall("close", close(fi->fh), 0 );


        // prepare the copy path
        sprintf(fullremoteuri,"scp://%s@%s/~/asmfsexports%s", ASM_DATA->remoteuser,ASM_DATA->remotehostname, path);
        sprintf(pathInTempPtr, "%s.cached", fpath);

        log_msg("do_release(): Writing back src:%s back to dst:%s\n",pathintemp, fullremoteuri);

        // carry out the scp  from "/temp/.. (tptr)  to "remote" (uptr)
        int scpretval = scpwritef(tptr,uptr);
        if(scpretval < 0 ) {
            log_error("scpwritef failed");
        }


    } else if( (fi->flags & 0xff) == O_WRONLY )  {
        log_msg("release():flags = %x, releasing the fd with WRONLY\n",fi->flags);
        fsync(fi->fh);
        fsync(fi->fh);


        retvalue = log_syscall("close", close(fi->fh), 0 );

        // prepare the copy path
        sprintf(fullremoteuri,"scp://%s@%s/~/asmfsexports%s", ASM_DATA->remoteuser,ASM_DATA->remotehostname, path);
        sprintf(pathInTempPtr, "%s.cached", fpath);

        log_msg("release: Writing back src:%s back to dst:%s\n",pathintemp, fullremoteuri);

        // carry out the scp  from "/temp/.. (tptr)  to "remote" (uptr)
        int scpretval = scpwritef(tptr,uptr);
        if(scpretval < 0 ) {
            log_error("scpwritef failed");
        }

    } else {
        log_msg("relesae(): handling case which is none of Readonly, writeonly or readwrite\n");
        fsync(fi->fh);
        fsync(fi->fh);

        // TODO do_release(): non read, non writeonly and non readwrite case : implement writeback

        retvalue = log_syscall("close", close(fi->fh), 0 );

    }

    // TODO do_release: need to implement unlink the .cached file on last reference removed. ( or last close or can we just keep it? )

    if( 0 ) {

        //if we need to get some back of the cached file.
        //sprintf(rmcmd,"cp %s %s.back%06d ; rm -f %s",pathInTempPtr, pathInTempPtr,backnumber,pathInTempPtr); backnumber++ ;

        //log_msg("\ndo_release(): file is removed with %s",rmcmd);
        sprintf(rmcmd,"rm -f %s",pathInTempPtr); // typically /tmp/localroot/<file>.cached
        int cmdretval = system(rmcmd);
    }

    // TODO do_release: can sync up directory entry with remotestat




    // finally return the syscall return value
    return retvalue ;

    /* ------------------------- Done -------------------------------- */


}





void *asm_init(struct fuse_conn_info *conn) {
    log_msg("\nasm_init()\n");

    log_conn(conn);
    log_fuse_context(fuse_get_context());

    listAndRemoteDirNames() ;

    return ASM_DATA;
}

void asm_destroy(void *userdata) {
    log_msg("\nasm_destroy(userdata=0x%08x)\n", userdata);
}



static struct fuse_operations ammiops = {
    .init 		 = asm_init,
    .mkdir      = do_mkdir,

    .mknod      = do_mknod,
    .getattr    = do_getattr,
    .readdir    = do_readdir,

    /*
    .read       = do_readinmem,
    .write      = do_writeinmem,
    .read       = asm_read,
    .write      = asm_write,
    */
    .read       = do_read,
    .write      = do_write,

    .open 		= do_open,
    .release    = do_release,
};

void asm_usage() {
    fprintf(stderr, "usage:  ammifs remotehost remoteuser rootDir mountPoint\n");
    abort();
}

int main(int argc, char *argv[]) {
    int fuse_stat;
    struct asm_state *asm_data;

    if ((getuid() == 0) || (geteuid() == 0)) {
        fprintf(stderr, "Running BBFS as root opens unnacceptable security holes\n");
        return 1;
    }

    fprintf(stderr, "Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);

    for ( int tt = 0 ; tt < argc ; tt++) {
        fprintf(stderr,"%s\n",argv[tt]);
    }

    if ((argc < 5) || (argv[argc - 2][0] == '-') || (argv[argc - 1][0] == '-'))
        asm_usage();

    asm_data = malloc(sizeof(struct asm_state));
    if (asm_data == NULL) {
        perror("main calloc");
        abort();
    }
    //remotehostname remoteip rootdir mountdir
    //argc-4 argc-3 argc-2 argc-1
    // Pull the rootdir out of the argument list and save it in my
    // internal data
    asm_data->rootdir = realpath(argv[argc - 2], NULL);
    argv[argc - 2] = argv[argc - 1];
    argv[argc - 1] = NULL;
    argc--;

    sprintf(asm_data->remoteuser, "%s", argv[argc - 2]);
    argv[argc - 2] = argv[argc - 1];
    argv[argc - 1] = NULL;
    argc--;

    sprintf(asm_data->remotehostname, "%s", argv[argc - 2]);
    argv[argc - 2] = argv[argc - 1];
    argv[argc - 1] = NULL;
    argc--;

    asm_data->logfile = log_open();




    fprintf(stderr, "parameters fuse_main\n");
    fprintf(stderr, "remoteuser:%s\n", asm_data->remoteuser);
    fprintf(stderr, "remoteHost:%s\n", asm_data->remotehostname);
    fprintf(stderr, "rootdir:%s\n", asm_data->rootdir);


    static char mkdirbuf[PATH_MAX] ;
    char *mkdirptr  = &mkdirbuf[0] ;

    // TODO main(): mkdir -p /tmp/localroot hardcoded , use passed argument instead
    sprintf(mkdirbuf, "mkdir -p %s","/tmp/localroot");
    fprintf(stderr,"executing %s\n", mkdirptr);
    system(mkdirptr) ;


    fprintf(stderr, "about to call fuse_main\n");
    //fuse_stat = fuse_main(argc, argv, &bb_oper, asm_data);
    fuse_stat = fuse_main(argc, argv, &ammiops, asm_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

    return fuse_stat;
}
