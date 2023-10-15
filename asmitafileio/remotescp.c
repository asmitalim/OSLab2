#include <stdio.h>
#include <curl/curl.h>
#include <fuse.h>
#include "remotescp.h"
#include "log.h"

//scpreadf takes in remotfilename which should have complete uri and localfilename should have relative path
//without root at beginning
//returns -1 if there's an error
//returns 0 on success



#ifdef REMOTESCPDEBUG

#define log_msg printf

int main(void) {   

#ifdef buntz
    scpreadf("scp://ubiqadmin@nandihill.centralindia.cloudapp.azure.com:/etc/hosts","/tmp/asmijunk");

#else
    scpreadf("scp://asmita@master0/etc/hosts","/tmp/asmijunk");
#endif
}
#endif

int scpreadf(char *remotefileuri, char *localfilename)
{
    CURL *curl;
    CURLcode res;
    static char remoteuribuffer[2000];
    static char localfilebuffer[2000];
    sprintf(localfilebuffer, "%s", localfilename);
    sprintf(remoteuribuffer, "%s", remotefileuri);

    log_msg("SCPREAD:Local file path: %s \n", localfilebuffer);
    log_msg("SCPREAD:Remote file path: %s \n", remoteuribuffer);





    curl = curl_easy_init();
    FILE *fp = fopen(localfilebuffer, "wb");

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, remotefileuri);
        /* example.com is redirected, so we tell libcurl to follow redirection */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK){
            log_msg("curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            return -1;
        }
        else
        {
            log_msg("SCPREAD:Successful!");
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
        return 0;
    }
    else
    {
        return -1;
    }
}

