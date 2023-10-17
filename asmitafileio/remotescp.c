#include <stdio.h>
#include <curl/curl.h>
#include <fuse.h>
#include <string.h>
#include <stdlib.h>

#include "remotescp.h"

#include "log.h"



#ifdef REMOTESCPDEBUG

#define log_msg printf

int main(void)
{
    int retvalue;

#ifdef buntz
    scpreadf("scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com:/etc/hosts","/tmp/asmijunk");
    scpreadf("scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/foo","/tmp/fooremote");
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

    scpwritef("/tmp/fooremote",  "scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/fooremoteremote");
    scpwritef("/tmp/lsaloutput", "scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/lsaloutput");
    scpwritef("/tmp/babsjunk",   "scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/babsjunk");

    scpreadf("scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com/~/asmfsexports/foo","/tmp/foo");



#else
    scpreadf("scp://asmita@master0/etc/hosts","/tmp/asmijunk");
#endif
}
#endif

//scpreadf takes in remotfilename which should have complete uri and localfilename should have relative path
//without root at beginning

//  the localfilename is required to have /tmp as prefix
//returns -1 if there's an error
//returns 0 on success

int scpreadf(char *remotefileuri, char *localfilename)
{
    CURL *curl;
    CURLcode res;
    const char *slashtmp = "/tmp" ;



    static char remoteuribuffer[5000];
    static char localfilebuffer[5000];

    log_msg("SCPREAD:Length of local file name = %ld\n",strlen(localfilename));
    log_msg("SCPREAD:4 letter prefix of local file name matches? = %s\n",strncmp(localfilename,slashtmp,4L)?"no":"yes");

    if ((strlen(localfilename) < 4) || (strncmp(localfilename,slashtmp, 4) != 0 )) {
        log_msg("SCPREAD:can not have Localfilename prefix other than /tmp");
        return -1 ;
    }

    sprintf(localfilebuffer, "%s", localfilename);
    sprintf(remoteuribuffer, "%s", remotefileuri);

    log_msg("SCPREAD:Local file path: %s \n", localfilebuffer);
    log_msg("SCPREAD:Remote file path: %s \n", remoteuribuffer);





    curl = curl_easy_init();
    FILE *fp = fopen(localfilebuffer, "wb");

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, remotefileuri);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK) {
            log_msg("SCPREAD:curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            return -1;
        } else {
            log_msg("SCPREAD:Successful!\n");
        }

        /* always cleanup */
        fflush(fp);
        fclose(fp);
        curl_easy_cleanup(curl);
        return 0;
    } else {
        return -1;
    }
}


//  the localfilename is required to have /tmp as prefix
//returns -1 if there's an error
//returns 0 on success

int scpwritef(char *localfilename, char *remotefileuri)
{


    char cmdbuffer[500] ;
    char *cmdptr = cmdbuffer ;



    const char *slashtmp = "/tmp" ;



    static char remoteuribuffer[5000];
    static char localfilebuffer[5000];

    log_msg("SCPWRITE:Length of local file name = %ld\n",strlen(localfilename));
    log_msg("SCPWRITE:4 letter prefix of local file name matches? = %s\n",strncmp(localfilename,slashtmp,4L)?"no":"yes");

    if ((strlen(localfilename) < 4) || (strncmp(localfilename,slashtmp, 4) != 0 )) {
        log_msg("SCPWRITE:can not have Localfilename prefix other than /tmp");
        return -1 ;
    }

    sprintf(localfilebuffer, "%s", localfilename);
    sprintf(remoteuribuffer, "%s", remotefileuri);

    log_msg("SCPWRITE:Local file path: %s \n", localfilebuffer);
    log_msg("SCPWRITE:Remote file path: %s \n", remoteuribuffer);


    sprintf(cmdptr, "scp %s %s", localfilename, remotefileuri);
    log_msg("SCPWRITE: command  %s\n", cmdptr);

    int retval;

    retval = system(cmdptr);

    if(retval == 0) {
        return 0 ;
    } else {
        return -1 ;
    }










    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    FILE *fp = fopen(localfilebuffer, "rb");

    if (curl) {
        //curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, remotefileuri);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_READDATA, fp);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK) {
            log_msg("SCPWRITE:curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            return -1;
        } else {
            log_msg("SCPWRITE:Successful!\n");
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
        fflush(fp);
        fclose(fp);
        return 0;
    } else {
        return -1;
    }
}








