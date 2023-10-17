/*


	This file is derived from the the Big Brother File System, a template and logs are reused
	from this file. 

  Big Brother File System
  Copyright (C) 2012 Joseph J. Pfeiffer, Jr., Ph.D. <pfeiffer@cs.nmsu.edu>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  This code is derived from function prototypes found /usr/include/fuse/fuse.h
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  His code is licensed under the LGPLv2.
  A copy of that code is included in the file fuse.h

  The point of this FUSE filesystem is to provide an introduction to
  FUSE.  It was my first FUSE filesystem as I got to know the
  software; hopefully, the comments in this code will help people who
  follow later to get a gentler introduction.

  This might be called a no-op filesystem:  it doesn't impose
  filesystem semantics on top of any other existing structure.  It
  simply reports the requests that come in, and passes them to an
  underlying filesystem.  The information is saved in a logfile named
  bbfs.log, in the directory from which you run bbfs.
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

#ifdef HAVE_SYS_XATTR_H
#include <sys/xattr.h>
#endif

#include "log.h"
#include "remotescp.h"




static void relativeToAbsolute(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, PRIVATE_DATA->rootdir);
    strncat(fpath, path, PATH_MAX);

    log_msg("    relativeToAbsolute:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n",
            PRIVATE_DATA->rootdir, path, fpath);
}






int asmfs_getattr(const char *path, struct stat *statbuf)
{
    int retstat;
    char fpath[PATH_MAX];

    log_msg("\nasmfs_getattr(path=\"%s\", statbuf=0x%08x)\n",
            path, statbuf);
    relativeToAbsolute(fpath, path);

    statbuf -> st_uid = getuid() ;
    statbuf -> st_gid = getgid() ;
    statbuf -> st_atime = time(NULL) ;
    statbuf -> st_mtime = time(NULL) ;

    if( strcmp(path, "/") == 0 ) {

        statbuf -> st_mode = S_IFDIR | 0755 ;
        statbuf -> st_nlink  = 2 ;
        return 0 ;

    } else if ( strcmp(path, "/asmfsinfo.txt") == 0 ) {
        statbuf -> st_mode = S_IFREG | 0644 ;
        statbuf -> st_nlink  = 1 ;
        statbuf -> st_size = 1024 ;
        return 0 ;

    } else if ( strcmp(path, "/OSLab2-utaustin") == 0 ) {
        statbuf -> st_mode = S_IFDIR | 0755 ;
        statbuf -> st_nlink  = 1 ;
        return 0 ;

    } else if ( strcmp(path, "/foo1") == 0 ) {

        statbuf -> st_mode = S_IFREG | 0644 ;
        statbuf -> st_nlink  = 1 ;
        statbuf -> st_size = 1024 ;

        retstat = log_syscall("lstat", lstat("/tmp/foo1", statbuf), 0);
        return retstat;

    } else {
        retstat = log_syscall("lstat", lstat(fpath, statbuf), 0);
        return retstat;
    }

    retstat = log_syscall("lstat", lstat(fpath, statbuf), 0);

    // TODO
    //log_stat(statbuf);

    return retstat;
}

/** Read the target of a symbolic link
 *
 * The buffer should be filled with a null terminated string.  The
 * buffer size argument includes the space for the terminating
 * null character.  If the linkname is too long to fit in the
 * buffer, it should be truncated.  The return value should be 0
 * for success.
 */
// Note the system readlink() will truncate and lose the terminating
// null.  So, the size passed to to the system readlink() must be one
// less than the size passed to bb_readlink()
// bb_readlink() code by Bernardo F Costa (thanks!)





int bb_readlink(const char *path, char *link, size_t size)
{
    int retstat;
    char fpath[PATH_MAX];

    log_msg("\nbb_readlink(path=\"%s\", link=\"%s\", size=%d)\n",
            path, link, size);
    relativeToAbsolute(fpath, path);

    retstat = log_syscall("readlink", readlink(fpath, link, size - 1), 0);
    if (retstat >= 0) {
        link[retstat] = '\0';
        retstat = 0;
        log_msg("    link=\"%s\"\n", link);
    }

    return retstat;
}









int asmfs_mknod(const char *path, mode_t mode, dev_t dev)
{
    int retstat;
    char fpath[PATH_MAX];

    log_msg("\nasmfs_mknod(path=\"%s\", mode=0%3o, dev=%lld)\n",
            path, mode, dev);
    relativeToAbsolute(fpath, path);

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





/** Create a directory */
int bb_mkdir(const char *path, mode_t mode)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_mkdir(path=\"%s\", mode=0%3o)\n",
            path, mode);
    relativeToAbsolute(fpath, path);

    return log_syscall("mkdir", mkdir(fpath, mode), 0);
}









int bb_unlink(const char *path)
{
    char fpath[PATH_MAX];

    log_msg("bb_unlink(path=\"%s\")\n",
            path);
    relativeToAbsolute(fpath, path);

    return log_syscall("unlink", unlink(fpath), 0);
}









int bb_rmdir(const char *path)
{
    char fpath[PATH_MAX];

    log_msg("bb_rmdir(path=\"%s\")\n",
            path);
    relativeToAbsolute(fpath, path);

    return log_syscall("rmdir", rmdir(fpath), 0);
}








/* 
** Create a symbolic link 
*/

// similar to symlink system call - symbolic link <link> Points to <path>

int bb_symlink(const char *path, const char *link)
{
    char flink[PATH_MAX];

    log_msg("\nbb_symlink(path=\"%s\", link=\"%s\")\n", path, link);

    relativeToAbsolute(flink, link);

    return log_syscall("symlink", symlink(path, flink), 0);
}

/** Rename a file */
// both  paths are relative to mountpoint root
// rename oldname:<path> to newname:<newpath>


int bb_rename(const char *path, const char *newpath)
{
    char fpath[PATH_MAX];
    char fnewpath[PATH_MAX];

    log_msg("\nbb_rename(fpath=\"%s\", newpath=\"%s\")\n",
            path, newpath);

    relativeToAbsolute(fpath, path);
    relativeToAbsolute(fnewpath, newpath);

    return log_syscall("rename", rename(fpath, fnewpath), 0);
}









/** Create a hard link to a file */
// similar to link system call - hardlink newpath:<link> Pointsto:<path>


int bb_link(const char *path, const char *newpath)
{
    char fpath[PATH_MAX], fnewpath[PATH_MAX];

    log_msg("\nbb_link(path=\"%s\", newpath=\"%s\")\n",
            path, newpath);

    relativeToAbsolute(fpath, path);
    relativeToAbsolute(fnewpath, newpath);

    return log_syscall("link", link(fpath, fnewpath), 0);
}





int bb_chmod(const char *path, mode_t mode)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_chmod(fpath=\"%s\", mode=0%03o)\n",
            path, mode);
    relativeToAbsolute(fpath, path);

    return log_syscall("chmod", chmod(fpath, mode), 0);
}




int bb_chown(const char *path, uid_t uid, gid_t gid)

{
    char fpath[PATH_MAX];

    log_msg("\nbb_chown(path=\"%s\", uid=%d, gid=%d)\n",
            path, uid, gid);
    relativeToAbsolute(fpath, path);

    return log_syscall("chown", chown(fpath, uid, gid), 0);
}





/** Change the size of a file */
int bb_truncate(const char *path, off_t newsize)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_truncate(path=\"%s\", newsize=%lld)\n",
            path, newsize);
    relativeToAbsolute(fpath, path);

    return log_syscall("truncate", truncate(fpath, newsize), 0);
}






int bb_utime(const char *path, struct utimbuf *ubuf)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_utime(path=\"%s\", ubuf=0x%08x)\n",
            path, ubuf);
    relativeToAbsolute(fpath, path);

    return log_syscall("utime", utime(fpath, ubuf), 0);
}








int bb_open(const char *path, struct fuse_file_info *fi)
{
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

    log_msg("\nbb_open(path\"%s\", fi=0x%08x)\n", path, fi);

    relativeToAbsolute(fpath, path);

    //TODO: change this later


    sprintf(fullremoteuri,"scp://%s@%s/~/asmfsexports%s", PRIVATE_DATA->remoteUser,PRIVATE_DATA->remotehostname, path);

    sprintf(pathintemp, "/tmp%s", path);
    scpreadf(uptr,tptr);



    // if the open call succeeds, my retstat is the file descriptor,
    // else it's -errno.  I'm making sure that in that case the saved
    // file descriptor is exactly -1.



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

    log_fi(fi);

    return retstat;
}










int asmfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    int bytesRemaining =0;
    int actualOffset = offset ;

    char *asmfsInfo  =  "This is the builtin information file of asmfs... Lab project 2" ;
    char *goodMorningInfo  =  "Good Morning every one " ;
    char *info = ""  ;


    log_msg("\nasmfs_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
            path, buf, size, offset, fi);
    log_fi(fi);


    /*
    read for following files implemented ( foo comes from remote machine )

    "foo"
    "asmfsinfo.txt"
    "OSLab2-utaustin"
    */


    if( strcmp(path, "/asmfsinfo.txt") == 0 ) {
        info = asmfsInfo ;
        memcpy(buf, info+offset, size);
        return strlen(info) - offset ;

    } else if ( strcmp( path, "/OSLab2-utaustin") == 0 ) {
        info = goodMorningInfo ;
        memcpy(buf, info+offset, size);
        return strlen(info) - offset ;
    } else if ( strcmp(path, "/foo") == 0 ) {
        log_msg(" special processing for reading foo");


    } else
        return -1 ;


    /* foo file */





    return log_syscall("pread", pread(fi->fh, buf, size, offset), 0);
}

int bb_write(const char *path, const char *buf, size_t size, off_t offset,
             struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("\nbb_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
            path, buf, size, offset, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    log_fi(fi);

    return log_syscall("pwrite", pwrite(fi->fh, buf, size, offset), 0);
}
















int bb_statfs(const char *path, struct statvfs *statv)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nbb_statfs(path=\"%s\", statv=0x%08x)\n",
            path, statv);
    relativeToAbsolute(fpath, path);

    // get stats for underlying filesystem
    retstat = log_syscall("statvfs", statvfs(fpath, statv), 0);

    log_statvfs(statv);

    return retstat;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */
// this is a no-op in BBFS.  It just logs the call and returns success



int bb_flush(const char *path, struct fuse_file_info *fi)
{
    log_msg("\nbb_flush(path=\"%s\", fi=0x%08x)\n", path, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    log_fi(fi);

    return 0;
}









int asmfs_release(const char *path, struct fuse_file_info *fi)
{
    log_msg("\nasmfs_release(path=\"%s\", fi=0x%08x)\n",
            path, fi);
    log_fi(fi);



    fsync(fi->fh);

    int retvalue = log_syscall("close", close(fi->fh), 0 );

    char rmcmd[300];
    char fullremoteuri[5000];
    char pathintemp[5000];
    char *uptr = fullremoteuri ;
    char *tptr = pathintemp ;


    sprintf(fullremoteuri,"scp://%s@%s/~/asmfsexports%s", PRIVATE_DATA->remoteUser,PRIVATE_DATA->remotehostname, path);

    sprintf(pathintemp, "/tmp%s", path);
    scpwritef(tptr,uptr);

    sprintf(rmcmd,"rm -f /tmp%s",path);
    log_msg("\nBBRELEASE rmcmd %s",rmcmd);
    system(rmcmd);





    return retvalue ;
}







int bb_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    log_msg("\nbb_fsync(path=\"%s\", datasync=%d, fi=0x%08x)\n",
            path, datasync, fi);
    log_fi(fi);

    // some unix-like systems (notably freebsd) don't have a datasync call
#ifdef HAVE_FDATASYNC
    if (datasync)
        return log_syscall("fdatasync", fdatasync(fi->fh), 0);
    else
#endif
        return log_syscall("fsync", fsync(fi->fh), 0);
}






#ifdef HAVE_SYS_XATTR_H
/** Note that my implementations of the various xattr functions use
    the 'l-' versions of the functions (eg bb_setxattr() calls
    lsetxattr() not setxattr(), etc).  This is because it appears any
    symbolic links are resolved before the actual call takes place, so
    I only need to use the system-provided calls that don't follow
    them */

/** Set extended attributes */
int bb_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_setxattr(path=\"%s\", name=\"%s\", value=\"%s\", size=%d, flags=0x%08x)\n",
            path, name, value, size, flags);
    relativeToAbsolute(fpath, path);

    return log_syscall("lsetxattr", lsetxattr(fpath, name, value, size, flags), 0);
}

/** Get extended attributes */
int bb_getxattr(const char *path, const char *name, char *value, size_t size)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nbb_getxattr(path = \"%s\", name = \"%s\", value = 0x%08x, size = %d)\n",
            path, name, value, size);
    relativeToAbsolute(fpath, path);

    retstat = log_syscall("lgetxattr", lgetxattr(fpath, name, value, size), 0);
    if (retstat >= 0)
        log_msg("    value = \"%s\"\n", value);

    return retstat;
}

/** List extended attributes */
int bb_listxattr(const char *path, char *list, size_t size)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    char *ptr;

    log_msg("\nbb_listxattr(path=\"%s\", list=0x%08x, size=%d)\n",
            path, list, size);
    relativeToAbsolute(fpath, path);

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

/** Remove extended attributes */
int bb_removexattr(const char *path, const char *name)
{
    char fpath[PATH_MAX];

    log_msg("\nbb_removexattr(path=\"%s\", name=\"%s\")\n",
            path, name);
    relativeToAbsolute(fpath, path);

    return log_syscall("lremovexattr", lremovexattr(fpath, name), 0);
}
#endif




























int bb_opendir(const char *path, struct fuse_file_info *fi)
{
    DIR *dp;
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nbb_opendir(path=\"%s\", fi=0x%08x)\n",
            path, fi);
    relativeToAbsolute(fpath, path);

    // since opendir returns a pointer, takes some custom handling of
    // return status.
    dp = opendir(fpath);
    log_msg("    opendir returned 0x%p\n", dp);
    if (dp == NULL)
        retstat = log_error("bb_opendir opendir");

    fi->fh = (intptr_t)dp;

    log_fi(fi);

    return retstat;
}











int asmfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                  struct fuse_file_info *fi)
{
    int retstat = 0;
    DIR *dp;
    struct dirent *de;

    log_msg("\nasmfs_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n",
            path, buf, filler, offset, fi);




    dp = (DIR *)(uintptr_t)fi->fh;


    de = readdir(dp);
    log_msg("    readdir returned 0x%p\n", de);


    if (de == 0) {
        retstat = log_error("asmfs_readdir readdir");
        return retstat;
    }


    log_msg("....asmfs getting the directory entries need to mask these .....\n");


#ifdef JUNK
    filler(buf,".", NULL, 0);
    filler(buf,"..", NULL, 0);


    if (strcmp(path, "/") == 0) {
        filler(buf,"foo", NULL, 0);
        filler(buf,"asmfsinfo.txt",NULL,0);
        filler(buf,"OSLab2-utaustin",NULL,0);
    }
    log_fi(fi);
	return 0 ;
#endif


    do
    {
        log_msg("calling filler with name %s\n", de->d_name);
        if (filler(buf, de->d_name, NULL, 0) != 0)
        {
            log_msg("    ERROR asmfs_readdir filler:  buffer full");
            return -ENOMEM;
        }
    } while ((de = readdir(dp)) != NULL);

    log_fi(fi);

    return retstat;
}








int bb_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("\nbb_releasedir(path=\"%s\", fi=0x%08x)\n",
            path, fi);
    log_fi(fi);

    closedir((DIR *)(uintptr_t)fi->fh);

    return retstat;
}









int bb_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("\nbb_fsyncdir(path=\"%s\", datasync=%d, fi=0x%08x)\n",
            path, datasync, fi);
    log_fi(fi);

    return retstat;
}














void *asmfs_init(struct fuse_conn_info *conn)
{
    log_msg("\nasmfs_init()\n");
    log_conn(conn);

    log_fuse_context(fuse_get_context());

    return PRIVATE_DATA;
}



void bb_destroy(void *userdata)
{
    log_msg("\nbb_destroy(userdata=0x%08x)\n", userdata);
}






int bb_access(const char *path, int mask)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nbb_access(path=\"%s\", mask=0%o)\n",
            path, mask);
    relativeToAbsolute(fpath, path);

    retstat = access(fpath, mask);

    if (retstat < 0)
        retstat = log_error("bb_access access");

    return retstat;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
// Not implemented.  I had a version that used creat() to create and
// open the file, which it turned out opened the file write-only.

/**
 * Change the size of an open file
 *
 * This method is called instead of the truncate() method if the
 * truncation was invoked from an ftruncate() system call.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the truncate() method will be
 * called instead.
 *
 * Introduced in version 2.5
 */


int bb_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("\nbb_ftruncate(path=\"%s\", offset=%lld, fi=0x%08x)\n",
            path, offset, fi);
    log_fi(fi);

    retstat = ftruncate(fi->fh, offset);
    if (retstat < 0)
        retstat = log_error("bb_ftruncate ftruncate");

    return retstat;
}







int bb_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("\nbb_fgetattr(path=\"%s\", statbuf=0x%08x, fi=0x%08x)\n",
            path, statbuf, fi);
    log_fi(fi);

    if (!strcmp(path, "/"))
        return asmfs_getattr(path, statbuf);

    retstat = fstat(fi->fh, statbuf);
    if (retstat < 0)
        retstat = log_error("bb_fgetattr fstat");

    log_stat(statbuf);

    return retstat;
}







struct fuse_operations bb_oper = {
    .getattr = asmfs_getattr,
    .readlink = bb_readlink,
    // no .getdir -- that's deprecated
    .getdir = NULL,
    .mknod = asmfs_mknod,
    .mkdir = bb_mkdir,
    .unlink = bb_unlink,
    .rmdir = bb_rmdir,
    .symlink = bb_symlink,
    .rename = bb_rename,
    .link = bb_link,
    .chmod = bb_chmod,
    .chown = bb_chown,
    .truncate = bb_truncate,
    .utime = bb_utime,
    .open = bb_open,
    .read = asmfs_read,
    .write = bb_write,
    /** Just a placeholder, don't set */ // huh???
    .statfs = bb_statfs,
    .flush = bb_flush,
    .release = asmfs_release,
    .fsync = bb_fsync,

#ifdef HAVE_SYS_XATTR_H
    .setxattr = bb_setxattr,
    .getxattr = bb_getxattr,
    .listxattr = bb_listxattr,
    .removexattr = bb_removexattr,
#endif

    .opendir = bb_opendir,
    .readdir = asmfs_readdir,
    .releasedir = bb_releasedir,
    .fsyncdir = bb_fsyncdir,
    .init = asmfs_init,
    .destroy = bb_destroy,
    .access = bb_access,
    .ftruncate = bb_ftruncate,
    .fgetattr = bb_fgetattr
};



void asmfs_usage()
{
    fprintf(stderr, "Usage:  asmfs remotehostname remoteuser rootDir mountPoint\n");
    abort();
}




int main(int argc, char *argv[])
{
    int fuse_stat;
    struct asmfs_state *priavate_data;

    if ((getuid() == 0) || (geteuid() == 0)) {
        fprintf(stderr, "can run only as your own uid and gid , root and other users are not supported \n");
        return 1;
    }

    fprintf(stderr, "Fuse library version %d.%d\n", FUSE_MAJOR_VERSION, FUSE_MINOR_VERSION);

    if ((argc < 5) || (argv[argc - 2][0] == '-') || (argv[argc - 1][0] == '-'))
        asmfs_usage();

    priavate_data = malloc(sizeof(struct asmfs_state));
    if (priavate_data == NULL) {
        perror("main calloc");
        abort();
    }

    //remotehostname remoteuser rootdir mountdir
    //argc-4 argc-3 argc-2 argc-1



    priavate_data->rootdir = realpath(argv[argc - 2], NULL);
    argv[argc - 2] = argv[argc - 1];
    argv[argc - 1] = NULL;
    argc--;

    sprintf(priavate_data->remoteUser, "%s", argv[argc - 2]);
    argv[argc - 2] = argv[argc - 1];
    argv[argc - 1] = NULL;
    argc--;

    sprintf(priavate_data->remotehostname, "%s", argv[argc - 2]);
    argv[argc - 2] = argv[argc - 1];
    argv[argc - 1] = NULL;
    argc--;

    priavate_data->logfile = log_open();




    fprintf(stderr, "parameters fuse_main\n");
    fprintf(stderr, "remoteUser:%s\n", priavate_data->remoteUser);
    fprintf(stderr, "remoteHost:%s\n", priavate_data->remotehostname);


    fprintf(stderr, "argc %d\n", argc);
    for( int x =  0 ; x < argc ; x++) {
        fprintf(stderr, "Arguments [%d] is %s\n",x,argv[x]);
    }


    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    fuse_stat = fuse_main(argc, argv, &bb_oper, priavate_data);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

    return fuse_stat;
}
