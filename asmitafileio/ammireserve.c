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



#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>


#define MAXCACHEFILESIZE 8192

#define TABLESIZE 256
#define MAXNAMESIZE 1024










static void asm_fullpath(char fpath[PATH_MAX], const char *path) {
   strcpy(fpath, ASM_DATA->rootdir);
   strncat(fpath, path, PATH_MAX);
   
   log_msg("    asm_fullpath:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n",
   		ASM_DATA->rootdir, path, fpath);
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

    //log_fi(fi);


    retstat =  log_syscall("pread", pread(fi->fh, buf, size, offset), 0);

    // Alternate way by seek followed by read or directly calling pread
    //retstat = log_syscall("lseek", lseek(fi->fh, offset,SEEK_SET), 0);
    //retstat = log_syscall("read",  read(fi->fh, buf, size),0);


    return retstat ;
}







int asm_write(const char *path, const char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
    int retstat = 0;

    log_msg("\nasm_write(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n",
            path, buf, size, offset, fi);
    log_fi(fi);

    retstat = log_syscall("pwrite", pwrite(fi->fh, buf, size, offset), 0);

    return retstat ;
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





/*
**  NOT TO BE USED
*/
int asm_release(const char *path, struct fuse_file_info *fi) {
    log_msg("\nasm_release(path=\"%s\", fi=0x%08x)\n",
            path, fi);
    log_fi(fi);



    //fsync(fi->fh);

    int retvalue = log_syscall("close", close(fi->fh), 0 );

    char rmcmd[300];
    char fullremoteuri[5000];
    char pathintemp[5000];
    char *uptr = fullremoteuri ;
    char *tptr = pathintemp ;


    sprintf(fullremoteuri,"scp://%s@%s/~/asmfsexports%s", ASM_DATA->remoteuser,ASM_DATA->remotehostname, path);

    sprintf(pathintemp, "/tmp/localroot%s", path);
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
    /* TODO getxattr not implemented yet*/
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




