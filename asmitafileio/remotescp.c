#include <stdio.h>
#include <curl/curl.h>
//scpreadf takes in remotfilename which should have complete uri and localfilename should have relative path
//without root at beginning
//returns -1 if there's an error
//returns 0 on success
int scpreadf(char *remotefileuri, char *localfilename)
{
    CURL *curl;
    CURLcode res;
    static char remoteuribuffer[2000];
    static char localfilebuffer[2000];
    sprintf(localfilebuffer, "/tmp/%s", localfilename);

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
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            return -1;
        }
        else
        {
            printf("Successful!");
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

int main(void)
{   
    scpreadf("scp://asmita@master0/etc/hosts","asmijunk");

    /*
    CURL *curl;
    CURLcode res;
    
    curl = curl_easy_init();
    FILE *fp = fopen("./asmitatempscptest", "wb");
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "scp://asmita@master0/etc/hosts");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        else
        {
            printf("Successful!");
        }

        curl_easy_cleanup(curl);
    }
    */

    return 0;
}