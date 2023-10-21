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

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"
#include "remotescp.h"

static void asm_fullpath(char fpath[PATH_MAX], const char *path) {
    strcpy(fpath, ASM_DATA->rootdir);
    strncat(fpath, path, PATH_MAX); // ridiculously long paths will
    // break here

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



char dirStuff[5000];

void addDirectoryEntry(const char *ptr, struct stat *statptr);
void addFileIntoCache( const char *ptr, struct stat *statptr);


void listAndRemoteDirNames() {
    int n = remotedirnames("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com","/", &dirStuff[0]);

    //int n = remotedirnames(ASM_DATA->remoteIP, ASM_DATA->remotehostname, path, dirbuffer);

    char *str = dirStuff ;
    struct stat statbuf ;

    if( n != 0 ) {
        int buflen = strlen(dirStuff);
        char delim[] = "\n" ;

        char *ptr = strtok(str,delim);

        log_msg("Getting all tokens split using slash n\n");

        while(ptr != NULL) {
            log_msg("ptr[%s]\n",ptr);

            char remoteXName[300] ;
            sprintf(remoteXName,"/%s",ptr);


            log_msg("REMOTEDIRNAME(): getting info for %s\n",ptr);
            int x = remotestat("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com",ptr, &statbuf);
            log_stat(&statbuf);


            if( x == 0 ) {
                if(statbuf.st_mode & S_IFREG )  {
                    log_msg("list:REMOTE:  %s is a file\n",ptr);
                    addFileIntoCache( ptr, &statbuf);
                } else if(statbuf.st_mode & S_IFDIR ) {
                    log_msg("list:REMOTE:  %s is a directory\n",ptr);
                    addDirectoryEntry(ptr, &statbuf);
                } else {
                    log_msg("list:REMOTE:  %s is a unsupported file\n",ptr);
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



size_t writeFile( const char *path, const char *stuff ) {

    int fileIndex = getFileIndex( path );

    if ( fileIndex == -1 ) return;

    strcpy( fileStuff[ fileIndex ], stuff );
}

// ... //

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

    if ( fileIndex == -1 ) return -1;

    char *stuff = fileStuff[ fileIndex ];
	struct stat *statPtr = &cachedFileStats[fileIndex] ; 

	if(offset+size >= MAXCACHEFILESIZE ) {
		canserveSize = MAXCACHEFILESIZE - offset;
	} else {
		canserveSize = size ;

	}


	log_msg("do_readinmem(): fileIndex:%d path: %s size:%ld offset:%ld \n",fileIndex,path, size, offset);
	//log_fi(fi);
	log_msg("stat->st_size = %ld\n",statPtr->st_size );

	log_msg("Serving size = %ld\n",canserveSize);

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

	/*
    memcpy( buffer, stuff + offset, size );
    return strlen( stuff ) - offset;
	*/

	memcpy(buffer, stuff+offset,canserveSize);

	int n = (canserveSize >= 3 )? 3 : canserveSize ; 

	char *cptr = buffer ; 
	strncpy(buffer,"PHL",n);
	return canserveSize ; 
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

	if(offset+size >= MAXCACHEFILESIZE ) {
		canWriteSize = MAXCACHEFILESIZE - offset;
		newFileSize = MAXCACHEFILESIZE ;
	} else {
		canWriteSize = size ;
		newFileSize = offset+size ; 
	}

	log_msg("stat->st_size = %ld\n",statPtr->st_size );

	log_msg("Can write size = %ld\n",canWriteSize);

	statPtr->st_size = newFileSize ; 
	statPtr->st_mtime = time(NULL);
	statPtr->st_atime = time(NULL);

	/*
    memcpy( buffer, stuff + offset, size );
    return strlen( stuff ) - offset;
	*/

	memcpy(stuff+offset, buffer, canWriteSize);
    writeFile( path, buffer );

	return canWriteSize ; 
}










int asm_getattr(const char *path, struct stat *statbuf) {
    int retstat;
    char fpath[PATH_MAX];

    /*
    log_msg("\nasm_getattr(path=\"%s\", statbuf=0x%08x)\n",
            path, statbuf);
    */
    asm_fullpath(fpath, path);
    log_msg("\nasm_getattr:%s\n",path);

    /*if(strcmp(path,"/foo")==0)
    {
        retstat=remotestat("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com", "foo", statbuf);
    }
    //retstat=remotestat(ASM_DATA->remotehostname, ASM_DATA->remoteuser, )
    else*/
    {
        //retstat=log_syscall("lstat", lstat(fpath, statbuf), 0);
        retstat=lstat(fpath, statbuf);
    }


    /*
    log_stat(statbuf);
    */

    return retstat;
}

int bb_readlink(const char *path, char *link, size_t size) {
    int retstat;
    char fpath[PATH_MAX];

    log_msg("\nbb_readlink(path=\"%s\", link=\"%s\", size=%d)\n",
            path, link, size);
    asm_fullpath(fpath, path);

    retstat = log_syscall("readlink", readlink(fpath, link, size - 1), 0);
    if (retstat >= 0) {
        link[retstat] = '\0';
        retstat = 0;
        log_msg("    link=\"%s\"\n", link);
    }

    return retstat;
}

int bb_mknod(const char *path, mode_t mode, dev_t dev) {
    int retstat;
    char fpath[PATH_MAX];

    log_msg("\nbb_mknod(path=\"%s\", mode=0%3o, dev=%lld)\n",
            path, mode, dev);
    asm_fullpath(fpath, path);

    if (S_ISREG(mode)) {
        retstat = log_syscall("open", open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode), 0);
        if (retstat >= 0)
            retstat = log_syscall("close", close(retstat), 0);
    } else if (S_ISFIFO(mode))
        retstat = log_syscall("mkfifo", mkfifo(fpath, mode), 0);
    else
        retstat = log_syscall("mknod", mknod(fpath, mode, dev), 0);

    return retstat;
}

int bb_mkdir(const char *path, mode_t mode) {
    char fpath[PATH_MAX];

    log_msg("\nbb_mkdir(path=\"%s\", mode=0%3o)\n",
            path, mode);
    asm_fullpath(fpath, path);

    return log_syscall("mkdir", mkdir(fpath, mode), 0);
}

int bb_unlink(const char *path) {
    char fpath[PATH_MAX];

    log_msg("bb_unlink(path=\"%s\")\n",
            path);
    asm_fullpath(fpath, path);

    return log_syscall("unlink", unlink(fpath), 0);
}

int bb_rmdir(const char *path) {
    char fpath[PATH_MAX];

    log_msg("bb_rmdir(path=\"%s\")\n",
            path);
    asm_fullpath(fpath, path);

    return log_syscall("rmdir", rmdir(fpath), 0);
}

int bb_symlink(const char *path, const char *link) {
    char flink[PATH_MAX];

    log_msg("\nbb_symlink(path=\"%s\", link=\"%s\")\n",
            path, link);
    asm_fullpath(flink, link);

    return log_syscall("symlink", symlink(path, flink), 0);
}

int bb_rename(const char *path, const char *newpath) {
    char fpath[PATH_MAX];
    char fnewpath[PATH_MAX];

    log_msg("\nbb_rename(fpath=\"%s\", newpath=\"%s\")\n",
            path, newpath);
    asm_fullpath(fpath, path);
    asm_fullpath(fnewpath, newpath);

    return log_syscall("rename", rename(fpath, fnewpath), 0);
}

int bb_link(const char *path, const char *newpath) {
    char fpath[PATH_MAX], fnewpath[PATH_MAX];

    log_msg("\nbb_link(path=\"%s\", newpath=\"%s\")\n",
            path, newpath);
    asm_fullpath(fpath, path);
    asm_fullpath(fnewpath, newpath);

    return log_syscall("link", link(fpath, fnewpath), 0);
}

int bb_chmod(const char *path, mode_t mode) {
    char fpath[PATH_MAX];

    log_msg("\nbb_chmod(fpath=\"%s\", mode=0%03o)\n",
            path, mode);
    asm_fullpath(fpath, path);

    return log_syscall("chmod", chmod(fpath, mode), 0);
}

int bb_chown(const char *path, uid_t uid, gid_t gid)

{
    char fpath[PATH_MAX];

    log_msg("\nbb_chown(path=\"%s\", uid=%d, gid=%d)\n",
            path, uid, gid);
    asm_fullpath(fpath, path);

    return log_syscall("chown", chown(fpath, uid, gid), 0);
}

int bb_truncate(const char *path, off_t newsize) {
    char fpath[PATH_MAX];

    log_msg("\nbb_truncate(path=\"%s\", newsize=%lld)\n",
            path, newsize);
    asm_fullpath(fpath, path);

    return log_syscall("truncate", truncate(fpath, newsize), 0);
}

int bb_utime(const char *path, struct utimbuf *ubuf) {
    char fpath[PATH_MAX];

    log_msg("\nbb_utime(path=\"%s\", ubuf=0x%08x)\n",
            path, ubuf);
    asm_fullpath(fpath, path);

    return log_syscall("utime", utime(fpath, ubuf), 0);
}

/** File open operation
 */
int asm_open(const char *path, struct fuse_file_info *fi) {
    int retstat = 0;
    int fd;
    int fdtemp;
    char fpath[PATH_MAX];
    char pathintemp[5000];
    char fullremoteuri[5000];
    char* tptr;
    char* uptr;

    tptr=&pathintemp[0];
    uptr=&fullremoteuri[0];

    /*
    log_msg("\nasm_open(path\"%s\", fi=0x%08x)\n", path, fi);
    */

    asm_fullpath(fpath, path);



    sprintf(fullremoteuri,"scp://%s@%s/~/asmfsexports%s", ASM_DATA->remoteuser,ASM_DATA->remotehostname, path);

    sprintf(pathintemp, "/tmp%s", path);
    scpreadf(uptr,tptr);




    /*
    fd = log_syscall("open", open(fpath, fi->flags), 0);
    if (fd < 0)
        retstat = log_error("open");

    fi->fh = fd;
    */


    fdtemp = log_syscall("open", open(pathintemp, fi->flags), 0);
    if (fdtemp < 0)
        retstat = log_error("open");

    fi->fh = fdtemp;

    //log_fi(fi);

    return retstat;
}

int asm_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int retstat = 0;

    log_msg("\nasm_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
            path, buf, size, offset, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    /*
    log_fi(fi);
    */


    //return log_syscall("pread", pread(fi->fh, buf, size, offset), 0);

    retstat = log_syscall("lseek", lseek(fi->fh, offset,SEEK_SET), 0);
    retstat = log_syscall("read",  read(fi->fh, buf, size),0);

    //retstat = pread(fi->fh, buf,size,offset) ;
    return retstat ;
}

int asm_write(const char *path, const char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
    int retstat = 0;

    /*
    log_msg("\nasm_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
            path, buf, size, offset, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    log_fi(fi);

    return log_syscall("pwrite", pwrite(fi->fh, buf, size, offset), 0);
    */
    return pwrite(fi->fh, buf, size, offset);
}

int bb_statfs(const char *path, struct statvfs *statv) {
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nbb_statfs(path=\"%s\", statv=0x%08x)\n",
            path, statv);
    asm_fullpath(fpath, path);

    // get stats for underlying filesystem
    retstat = log_syscall("statvfs", statvfs(fpath, statv), 0);

    log_statvfs(statv);

    return retstat;
}

int bb_flush(const char *path, struct fuse_file_info *fi) {
    /*
    log_msg("\nbb_flush(path=\"%s\", fi=0x%08x)\n", path, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    log_fi(fi);
    */

    return 0;
}

/** Release an open file
 */
int asm_release(const char *path, struct fuse_file_info *fi) {
    log_msg("\nasm_release(path=\"%s\", fi=0x%08x)\n",
            path, fi);
    /*
    log_fi(fi);
    */



    fsync(fi->fh);

    int retvalue = log_syscall("close", close(fi->fh), 0 );

    char rmcmd[300];
    char fullremoteuri[5000];
    char pathintemp[5000];
    char *uptr = fullremoteuri ;
    char *tptr = pathintemp ;


    sprintf(fullremoteuri,"scp://%s@%s/~/asmfsexports%s", ASM_DATA->remoteuser,ASM_DATA->remotehostname, path);

    sprintf(pathintemp, "/tmp%s", path);
    scpwritef(tptr,uptr);

    sprintf(rmcmd,"rm -f /tmp%s",path);
    log_msg("\nBBRELEASE rmcmd %s",rmcmd);
    int cmdretval = system(rmcmd);





    return retvalue ;
}

int bb_fsync(const char *path, int datasync, struct fuse_file_info *fi) {
    log_msg("\nbb_fsync(path=\"%s\", datasync=%d, fi=0x%08x)\n",
            path, datasync, fi);
    //log_fi(fi);

#ifdef HAVE_FDATASYNC
    if (datasync)
        return log_syscall("fdatasync", fdatasync(fi->fh), 0);
    else
#endif
        return log_syscall("fsync", fsync(fi->fh), 0);
}

#ifdef HAVE_SYS_XATTR_H

int bb_setxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
    char fpath[PATH_MAX];

    log_msg("\nbb_setxattr(path=\"%s\", name=\"%s\", value=\"%s\", size=%d, flags=0x%08x)\n",
            path, name, value, size, flags);
    asm_fullpath(fpath, path);

    return log_syscall("lsetxattr", lsetxattr(fpath, name, value, size, flags), 0);
}

int bb_getxattr(const char *path, const char *name, char *value, size_t size) {
    int retstat = 0;
    char fpath[PATH_MAX];
    /* TODO */
    return -1 ;

    log_msg("\nbb_getxattr(path = \"%s\", name = \"%s\", value = 0x%08x, size = %d)\n",
            path, name, value, size);
    asm_fullpath(fpath, path);

    retstat = log_syscall("lgetxattr", lgetxattr(fpath, name, value, size), 0);
    if (retstat >= 0)
        log_msg("    value = \"%s\"\n", value);

    return retstat;
}

int bb_listxattr(const char *path, char *list, size_t size) {
    int retstat = 0;
    char fpath[PATH_MAX];
    char *ptr;

    log_msg("\nbb_listxattr(path=\"%s\", list=0x%08x, size=%d)\n",
            path, list, size);
    asm_fullpath(fpath, path);

    retstat = log_syscall("llistxattr", llistxattr(fpath, list, size), 0);
    if (retstat >= 0) {
        log_msg("    returned attributes (length %d):\n", retstat);
        if (list != NULL)
            for (ptr = list; ptr < list + retstat; ptr += strlen(ptr) + 1)
                log_msg("    \"%s\"\n", ptr);
        else
            log_msg("    (null)\n");
    }

    return retstat;
}

int bb_removexattr(const char *path, const char *name) {
    char fpath[PATH_MAX];

    log_msg("\nbb_removexattr(path=\"%s\", name=\"%s\")\n",
            path, name);
    asm_fullpath(fpath, path);

    return log_syscall("lremovexattr", lremovexattr(fpath, name), 0);
}
#endif

int bb_opendir(const char *path, struct fuse_file_info *fi) {
    DIR *dp;
    int retstat = 0;
    char fpath[PATH_MAX];

    /*
    log_msg("\nbb_opendir(path=\"%s\", fi=0x%08x)\n",
            path, fi);
    */
    asm_fullpath(fpath, path);

    dp = opendir(fpath);
    //log_msg("    opendir returned 0x%p\n", dp);
    if (dp == NULL)
        retstat = log_error("bb_opendir opendir");

    fi->fh = (intptr_t)dp;

    //log_fi(fi);

    return retstat;
}


int asm_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                struct fuse_file_info *fi) {
    int retstat = 0;
    DIR *dp;
    struct dirent *de;

    /*
    log_msg("\nasm_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n",
            path, buf, filler, offset, fi);
    // once again, no need for fullpath -- but note that I need to cast fi->fh

    */


    dp = (DIR *)(uintptr_t)fi->fh;

    de = readdir(dp);



    //log_msg("    readdir returned 0x%p\n", de);
    if (de == 0) {
        retstat = log_error("asm_readdir readdir");
        return retstat;
    }

    /*if(strcmp(path, "/")==0)
    {
        filler(buf, "foo", NULL, 0);
    }*/
    do {
        //log_msg("calling filler with name %s\n", de->d_name);
        if (filler(buf, de->d_name, NULL, 0) != 0) {
            log_msg("    ERROR asm_readdir filler:  buffer full");
            return -ENOMEM;
        }
    } while ((de = readdir(dp)) != NULL);

    //log_fi(fi);

    return retstat;
}

int bb_releasedir(const char *path, struct fuse_file_info *fi) {
    int retstat = 0;

    log_msg("\nbb_releasedir(path=\"%s\", fi=0x%08x)\n",
            path, fi);
    /*
    log_fi(fi);
    */

    closedir((DIR *)(uintptr_t)fi->fh);

    return retstat;
}

int bb_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi) {
    int retstat = 0;

    log_msg("\nbb_fsyncdir(path=\"%s\", datasync=%d, fi=0x%08x)\n",
            path, datasync, fi);
    //log_fi(fi);

    return retstat;
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

int bb_access(const char *path, int mask) {
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nbb_access(path=\"%s\", mask=0%o)\n",
            path, mask);
    asm_fullpath(fpath, path);

    retstat = access(fpath, mask);

    if (retstat < 0)
        retstat = log_error("bb_access access");

    return retstat;
}


/**
 * Change the size of an open file
 */
int bb_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi) {
    int retstat = 0;

    log_msg("\nbb_ftruncate(path=\"%s\", offset=%lld, fi=0x%08x)\n",
            path, offset, fi);
    //log_fi(fi);

    retstat = ftruncate(fi->fh, offset);
    if (retstat < 0)
        retstat = log_error("bb_ftruncate ftruncate");

    return retstat;
}

/**
 * Get attributes from an open file
 *
 */
int bb_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi) {
    int retstat = 0;

    log_msg("\nbb_fgetattr(path=\"%s\", statbuf=0x%08x, fi=0x%08x)\n",
            path, statbuf, fi);
    //log_fi(fi);

    if (!strcmp(path, "/"))
        return asm_getattr(path, statbuf);

    retstat = fstat(fi->fh, statbuf);
    if (retstat < 0)
        retstat = log_error("bb_fgetattr fstat");

    log_stat(statbuf);

    return retstat;
}



struct fuse_operations bb_oper = {
    .getattr = asm_getattr,
    .mknod = bb_mknod,
    .mkdir = bb_mkdir,
    .open = asm_open,
    .read = asm_read,
    .write = asm_write,
    /*
    .release = asm_release,
    .init = asm_init,
    .destroy = asm_destroy,
    */
};



static struct fuse_operations ammiops = {
    .init 		 = asm_init,
    .mkdir      = do_mkdir,

    .mknod      = do_mknod,
    .getattr    = do_getattr,
    .readdir    = do_readdir,

    .read       = do_readinmem,
    .write      = do_writeinmem,
	/*
    .read       = asm_read,
    .write      = asm_write,
	*/
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


    fprintf(stderr, "argc %d\n", argc);
    for( int x =  0 ; x < argc ; x++) {
        fprintf(stderr, "Arguments [%d] is %s\n",x,argv[x]);
    }


    fprintf(stderr, "about to call fuse_main\n");
    //fuse_stat = fuse_main(argc, argv, &bb_oper, asm_data);
    fuse_stat = fuse_main(argc, argv, &ammiops, asm_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

    return fuse_stat;
}
