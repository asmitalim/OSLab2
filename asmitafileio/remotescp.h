#include "fuse.h"
#ifndef __REMOTESCP_H__
#define __REMOTESCP_H__
int scpreadf(char *remotefileuri, char *localfilename);
int scpwritef(char *localfilename, char *remotefileuri);
int remotestat(char* user, char* host, char* remotefilename, struct stat *statbuf);

int parseMode(char *rwxStr) ;
int remotedir(char *u, char *h, char *d, char *dirbuff);
int remotedirnames(char *user, char *host, char *dir, char *dirbuffer);



#endif
