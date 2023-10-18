#include <stdio.h>
#include <curl/curl.h>
#include <fuse.h>
#include <string.h>
#include <stdlib.h>

#include "remotescp.h"

#include "log.h"


char dirStuff[5000] ; 


#ifdef REMOTESCPDEBUG

#define log_msg printf

int main(void)
{
    int retvalue;
	struct stat statbuff ;

#ifdef buntz
    remotestat("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com", "foo", &statbuff);
    //remotestat("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com", "foo1", &statbuff);
    remotedir("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com","/", &dirStuff[0]);
	remotedirnames("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com","/", &dirStuff[0]);

    return 0;



    scpreadf("scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com:/etc/hosts", "/tmp/asmijunk");
    scpreadf("scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/foo", "/tmp/fooremote");
    /*
    retvalue = system("ls -al /tmp  > /tmp/lsaloutput");
    if( retvalue < 0 ) {
        perror("Can not execute ls -al");
        exit(1);
    }

    retvalue = system("cp /etc/hosts  /tmp/babshosts");
    if( retvalue < 0 ) {
        perror("Can not execute ls -al");
        exit(1);
    }
    */

    scpwritef("/tmp/fooremote", "scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/fooremoteremote");
    scpwritef("/tmp/lsaloutput", "scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/lsaloutput");
    scpwritef("/tmp/babsjunk", "scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/babsjunk");

    scpreadf("scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/foo", "/tmp/foo");

#else
    remotestat("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com", "foo", &statbuff);
    //remotestat("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com", "foo1", &statbuff);
    remotedir("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com","/", &dirStuff[0]);
	remotedirnames("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com","/", &dirStuff[0]);

    return 0;

    //remotestat("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com", "foo", &statbuff);
    //remotestat("ubiqadmin", "nandihill.centralindia.cloudapp.azure.com", "foo1", &statbuff);

    return 0;
    // scpreadf("scp://asmita@master0/etc/hosts","/tmp/asmijunk");
    scpreadf("scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com:/etc/hosts", "/tmp/asmijunk");
    scpreadf("scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/foo", "/tmp/fooremote");
    /*
    retvalue = system("ls -al /tmp  > /tmp/lsaloutput");
    if( retvalue < 0 ) {
        perror("Can not execute ls -al");
        exit(1);
    }

    retvalue = system("cp /etc/hosts  /tmp/babshosts");
    if( retvalue < 0 ) {
        perror("Can not execute ls -al");
        exit(1);
    }
    */

    scpwritef("/tmp/fooremote", "scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/fooremoteremote");
    scpwritef("/tmp/lsaloutput", "scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/lsaloutput");
    scpwritef("/tmp/babsjunk", "scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/babsjunk");

    scpreadf("scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/foo", "/tmp/foo");

#endif
}
#endif

// scpreadf takes in remotfilename which should have complete uri and localfilename should have relative path
// without root at beginning

//  the localfilename is required to have /tmp as prefix
// returns -1 if there's an error
// returns 0 on success

int scpreadf(char *remotefileuri, char *localfilename)
{
    CURL *curl;
    CURLcode res;
    const char *slashtmp = "/tmp";

    static char remoteuribuffer[5000];
    static char localfilebuffer[5000];

    log_msg("SCPREAD:Length of local file name = %ld\n", strlen(localfilename));
    log_msg("SCPREAD:4 letter prefix of local file name matches? = %s\n", strncmp(localfilename, slashtmp, 4L) ? "no" : "yes");

    if ((strlen(localfilename) < 4) || (strncmp(localfilename, slashtmp, 4) != 0))
    {
        log_msg("SCPREAD:can not have Localfilename prefix other than /tmp");
        return -1;
    }

    sprintf(localfilebuffer, "%s", localfilename);
    sprintf(remoteuribuffer, "%s", remotefileuri);

    log_msg("SCPREAD:Local file path: %s \n", localfilebuffer);
    log_msg("SCPREAD:Remote file path: %s \n", remoteuribuffer);

    curl = curl_easy_init();
    FILE *fp = fopen(localfilebuffer, "wb");

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, remotefileuri);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK)
        {
            log_msg("SCPREAD:curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            return -1;
        }
        else
        {
            log_msg("SCPREAD:Successful!\n");
        }

        /* always cleanup */
        fflush(fp);
        fclose(fp);
        curl_easy_cleanup(curl);
        return 0;
    }
    else
    {
        return -1;
    }
}

//  the localfilename is required to have /tmp as prefix
// returns -1 if there's an error
// returns 0 on success

int scpwritef(char *localfilename, char *remotefileuri)
{

    char cmdbuffer[500];
    char *cmdptr = cmdbuffer;

    const char *slashtmp = "/tmp";

    static char remoteuribuffer[5000];
    static char localfilebuffer[5000];

    log_msg("SCPWRITE:Length of local file name = %ld\n", strlen(localfilename));
    log_msg("SCPWRITE:4 letter prefix of local file name matches? = %s\n", strncmp(localfilename, slashtmp, 4L) ? "no" : "yes");

    if ((strlen(localfilename) < 4) || (strncmp(localfilename, slashtmp, 4) != 0))
    {
        log_msg("SCPWRITE:can not have Localfilename prefix other than /tmp");
        return -1;
    }

    sprintf(localfilebuffer, "%s", localfilename);
    sprintf(remoteuribuffer, "%s", remotefileuri);

    log_msg("SCPWRITE:Local file path: %s \n", localfilebuffer);
    log_msg("SCPWRITE:Remote file path: %s \n", remoteuribuffer);

    sprintf(cmdptr, "scp %s %s", localfilename, remotefileuri);
    log_msg("SCPWRITE: command  %s\n", cmdptr);

    int retval;

    retval = system(cmdptr);

    if (retval == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    FILE *fp = fopen(localfilebuffer, "rb");

    if (curl)
    {
        // curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, remotefileuri);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_READDATA, fp);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK)
        {
            log_msg("SCPWRITE:curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            return -1;
        }
        else
        {
            log_msg("SCPWRITE:Successful!\n");
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
        fflush(fp);
        fclose(fp);
        return 0;
    }
    else
    {
        return -1;
    }
}


int remotedirnames(char *user, char *host, char *dir, char *dirbuffer) {
    static char cmd[2000];
	static char buf[2000];
	dirbuffer[0] = '\0' ;

    sprintf(cmd, "ssh %s@%s \"cd asmfsexports; ls -1 .%s \" 2>&1", user, host, dir);
	printf("Command is %s\n",cmd);
	FILE *fp = popen(cmd, "r");
	while (fgets(buf,sizeof(buf), fp) != NULL) {
		strcat(dirbuffer,buf);
		printf("The return value %s\n",buf);
	}
	printf("---------------------\n");
	printf("remotedirnames contents\n%s\n",dirbuffer);
}



int remotedir(char *user, char *host, char *dir, char *dirbuffer) {
    static char cmd[2000];
	static char buf[2000];
	dirbuffer[0] = '\0' ;

    sprintf(cmd, "ssh %s@%s \"cd asmfsexports; ls -l .%s | grep \\\"^-\\\"  | sed \\\"s/ \\\\+/ /g\\\" \" 2>&1", user, host, dir);
	printf("Command is %s\n",cmd);
	FILE *fp = popen(cmd, "r");
	while (fgets(buf,sizeof(buf), fp) != NULL) {
		strcat(dirbuffer,buf);
		printf("The return value %s\n",buf);
	}
	printf("---------------------\n");
	printf("remotedir contents\n%s\n",dirbuffer);
}


int remotestat(char* user, char* host, char* remotefilename,  struct stat *statbuf)
{   
    static char cmd[2000];
    //char* cmd= "\"cd asmfsexports; ls -al foo | sed \\\"s/ \\\\+/,/g\\\" \" ";
    sprintf(cmd, "ssh %s@%s \"cd asmfsexports; ls -al %s | sed \\\"s/ \\\\+/ /g\\\" \" 2>&1", user, host, remotefilename);
    //int retval=system(tmp);
    FILE* fp=popen(cmd, "r");
    static char buf[2000];
    fgets(buf, sizeof(buf),fp);
    log_msg("Output is %s", buf);
    if(strncmp(buf,"ls: cannot access",17)==0)
    {
        log_msg("No file called %s on the remote server \n", remotefilename);
        pclose(fp);
        return -1;
    }
    //printf("Retval for %s is: %d \n", remotefilename, retval);
    //return retval;
    static char retvalues[9][200];
    sscanf(buf,"%s %s %s %s %s %s %s %s %s",
	retvalues[0], // permission
    retvalues[1], // link
    retvalues[2], // uid
    retvalues[3], // gid
    retvalues[4], // size
    retvalues[5], // size
    retvalues[6], // month
    retvalues[7], // date
    retvalues[8]); //filename
    for(int i=0;i<9;i++)
    {
        log_msg("Value of retvalues %d is %s \n", i, retvalues[i]);
    }

    pclose(fp);


	int rwx = parseMode(retvalues[0]);
	log_msg("Permission is %o\n",rwx);

	long  sizeOfFile = 0 ; 
	sscanf(retvalues[4],"%ld",&sizeOfFile);
	int  numberOfLinks  ;
	sscanf(retvalues[1],"%d",&numberOfLinks);

	log_msg("size is %ld\n",sizeOfFile);
    log_msg("number of links %d\n",numberOfLinks);

	statbuf->st_mode = rwx ;
	statbuf->st_nlink =  numberOfLinks    ;
	statbuf->st_uid = getuid()     ;
	statbuf->st_gid = getgid()  ;
	statbuf->st_size = sizeOfFile ;




    return 0;

}

int parseMode(char *rwxStr) {

	unsigned long n = strlen(rwxStr);
	char dirOrReg ;
	int rwx = 0 ;

	log_msg("The rwx string is %s(%lu)\n",rwxStr,strlen(rwxStr));


	if (n == 10) {
		char *cp = &rwxStr[0] ;

		char dirOrReg = *cp++ ; 

		switch( dirOrReg ) {

			case 'd':
				rwx = S_IFDIR ;
				break ; 
				
			case '-':
				rwx = S_IFREG ;
				break ; 
				
			case 'p':
				rwx = S_IFDIR ;
				break ; 
				
			case 'c':
				rwx = S_IFCHR ;
				break ; 
				
			case 'b':
				rwx = S_IFBLK ;
				break ; 

			case 'l':
				rwx = S_IFLNK ;
				break ; 

			case 's':
				rwx = S_IFSOCK ;
				break ; 

			default:
				rwx = S_IFSOCK ;
				break ; 
		} 






		rwx |= *cp++ == 'r' ? 04  : 0 ;
		rwx |= *cp++ == 'w' ? 02  : 0 ;
		rwx |= *cp++ == 'x' ? 01  : 0 ;
		rwx <<= 3  ;
		//printf("the composed rwx is %o\n",rwx);

		rwx |= *cp++ == 'r' ? 04  : 0 ;
		rwx |= *cp++ == 'w' ? 02  : 0 ;
		rwx |= *cp++ == 'x' ? 01  : 0 ;
		rwx <<= 3  ;
		//printf("the composed rwx is %o\n",rwx);

		rwx |= *cp++ == 'r' ? 04  : 0 ;
		rwx |= *cp++ == 'w' ? 02  : 0 ;
		rwx |= *cp++ == 'x' ? 01  : 0 ;

		//printf("the composed rwx is %o\n",rwx);
		//printf("File or Directory: %c",dirOrReg);

		return rwx ; 
	}

	else 
		return 0755 ; 
}
