
#include "Http_Handler.h"
#include <string.h>
#include <stdio.h>

size_t httpResponseHandler(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    return size*nmemb;
}

int SendRequest(char *pdata, char *paddr, char *pmethod)
{
    int http_code = 0;
    CURLcode res;
    CURL *curl;
    struct curl_slist *headers = NULL;
    char agent[128];

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl)
    {
	strcpy(agent, "libcurl/");
	strcat(agent, curl_version_info(CURLVERSION_NOW)->version);

	curl_easy_setopt(curl, CURLOPT_USERAGENT, agent);

	headers = curl_slist_append(headers, "Expect:");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	if ( strcmp(pmethod,"PUT") == 0 )
	    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pdata);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &httpResponseHandler);
	curl_easy_setopt(curl, CURLOPT_URL, paddr);

	res = curl_easy_perform(curl);

	if(res != CURLE_OK)
	    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return http_code;
}
