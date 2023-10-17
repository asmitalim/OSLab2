#ifndef __REMOTESCP_H__
#define __REMOTESCP_H__
int scpreadf(char *remotefileuri, char *localfilename);
int scpwritef(char *localfilename, char *remotefileuri);
int remotestat(char* user, char* host, char* remotefilename);


#endif
