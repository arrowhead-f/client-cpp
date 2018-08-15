
#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include <string.h>
#include "curl/curl.h"

size_t httpResponseHandler(char *ptr, size_t size, size_t nmemb, void *userdata);
int SendRequest(char *pdata, char *paddr, char *pmethod);

#endif
