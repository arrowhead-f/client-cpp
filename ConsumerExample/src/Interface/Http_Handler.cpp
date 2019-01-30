
#include "Http_Handler.hpp"
#include <cstring>

#include <vector>
#include <string>

size_t Http_Handler::httpResponseCallback(char *ptr, size_t size)
{
	return size;
}

size_t httpResponseHandler(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	return ((Http_Handler *)userdata)->httpResponseCallback(ptr, size*nmemb);
}

int Http_Handler::SendRequest(string pdata, string paddr, string pmethod)
{
    int http_code = 0;
    CURLcode res;
  	CURL *curl;
	struct curl_slist *headers = NULL;
  	string agent;

	curl_global_init(CURL_GLOBAL_ALL);
  	curl = curl_easy_init();

  	if(curl)
  	{
		agent = "libcurl/"+string(curl_version_info(CURLVERSION_NOW)->version);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, agent.c_str());

		headers = curl_slist_append(headers, "Expect:");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		if (pmethod == "PUT")
                        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pdata.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &httpResponseHandler);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

		curl_easy_setopt(curl, CURLOPT_URL, paddr.c_str());

		// Perform the request, res will get the return code
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

// SERVER side
// This responds to HTTP GET!
// can be Overloaded in child class!
int Http_Handler::httpGETCallback(const char *Id, string *pData_str)
{
    *pData_str = "1234";
    return 1;
}


// static C function (not member of class!)
extern "C" int MHD_Callback(void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data,
          size_t *upload_data_size,
          void **ptr)
{
	static int aptr;
	struct MHD_Response *response;
    string cb_data;
    int value;
    int ret = MHD_YES;

	if (0 != strcmp (method, "GET"))
	  return MHD_NO;              // unexpected method
	if (&aptr != *ptr)
	{
		// do never respond on first call
		*ptr = &aptr;
		return MHD_YES;
	}
	*ptr = NULL;                  // reset when done

	// Cast callback to creators class (passing C++ in C functions...)

    value = ((Http_Handler *)cls)->httpGETCallback(url, &cb_data);

    if (value)
    {
        response = MHD_create_response_from_buffer(cb_data.length(), (void *)cb_data.c_str(), MHD_RESPMEM_MUST_COPY);
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
    }
return ret;
}


int Http_Handler::MakeServer(unsigned short listen_port){
	pmhd = MHD_start_daemon(
			MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG | MHD_USE_DUAL_STACK,
			listen_port,
			NULL, NULL, &MHD_Callback, this,
			MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
			MHD_OPTION_END);
	if (pmhd == NULL)
		return 1;

	return 0;
}

int Http_Handler::KillServer()
{
	if (pmhd)
	{
		MHD_stop_daemon(pmhd);
		pmhd = NULL;
		return 0;
	}

        return 1;
}
