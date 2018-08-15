
#include "Http_Handler.hpp"
#include <cstring>

#include <vector>
#include <string>

#define POSTBUFFERSIZE  512
#define MAXNAMESIZE     20
#define MAXANSWERSIZE   512

#define POST            1

struct connection_info_struct
{
    char *answerstring;
    struct MHD_PostProcessor *postprocessor;
};

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

		if (strcmp(pmethod.c_str(), "PUT") == 0)
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
          else if( strcmp( pmethod.c_str(), "POST") == 0)
               curl_easy_setopt(curl, CURLOPT_POST, true);

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

int Http_Handler::httpPOSTCallback(const char *url, const char *payload)
{
     return 1;
}

inline int iterate_post(void *coninfo_cls,
                        enum MHD_ValueKind kind,
                        const char *key,
                        const char *filename,
                        const char *content_type,
                        const char *transfer_encoding,
                        const char *data,
                        uint64_t off,
                        size_t size)
{
     struct connection_info_struct *con_info = (connection_info_struct *) coninfo_cls;

     if (0 == strcmp(key, "name")){
          if ((size > 0) && (size <= MAXNAMESIZE)){
               char *answerstring;
               answerstring = (char *) malloc(MAXANSWERSIZE);
               if (!answerstring){
                    return MHD_NO;
               }

               snprintf(answerstring, MAXANSWERSIZE, "{\"name\": %s}", data);
               con_info->answerstring = answerstring;
          }
          else{
               con_info->answerstring = NULL;
          }

          return MHD_NO;
     }

     return MHD_YES;
}

char szContentType[] = "application/x-www-form-urlencoded\0";

// static C function (not member of class!)
extern "C" int MHD_Callback(void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data,
          size_t *upload_data_size,
          void **con_cls)
{
	static int aptr;
	struct MHD_Response *response;
     string cb_data;
     int value;
     int ret = MHD_YES;

     if(strcmp(method, "GET") == 0)
     {
          if (&aptr != *con_cls)
          {
               // do never respond on first call
               *con_cls = &aptr;
               return MHD_YES;
          }
          *con_cls = NULL;                  // reset when done

          value = ((Http_Handler *)cls)->httpGETCallback(url, &cb_data);
          if (value)
          {
               response = MHD_create_response_from_buffer(cb_data.length(), (void *)cb_data.c_str(), MHD_RESPMEM_MUST_COPY);
               MHD_add_response_header (response, "Content-Type", "application/json");
               ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
               MHD_destroy_response(response);
          }
     }
     else if(strcmp(method, "POST") == 0)
     {
          //cast to application/x-www-form-urlencoded
          MHD_replace_connection_value (connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_CONTENT_TYPE, szContentType);

          if (NULL == *con_cls){
               struct connection_info_struct *con_info = (connection_info_struct *) malloc(sizeof(struct connection_info_struct));
               if (NULL == con_info){
                    return MHD_NO;
               }
               con_info->answerstring = NULL;

               if (0 == strcmp(method, "POST")){
                    con_info->postprocessor = MHD_create_post_processor(connection, 2048, iterate_post, (void *) con_info);

                    if (NULL == con_info->postprocessor){
                         free(con_info);
                         return MHD_NO;
                    }
               }

               *con_cls = (void *) con_info;

               return MHD_YES;
          }

          struct connection_info_struct *con_info = (connection_info_struct *) *con_cls;

          if (*upload_data_size != 0){
               MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size);

               value = ((Http_Handler *)cls)->httpPOSTCallback(url /*const char * */, upload_data /*const char * */);
               *upload_data_size = 0;

               return MHD_YES;
          }
          else{
               string valasz = "\"Result\":\"OK\"";
               response = MHD_create_response_from_buffer(valasz.length(), (void *)valasz.c_str(), MHD_RESPMEM_MUST_COPY);
               MHD_add_response_header (response, "Content-Type", "application/json");
               ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
               MHD_destroy_response(response);
          }
     }
     else{
          string valasz = "\"Result\":\"Unknown method\"";
               response = MHD_create_response_from_buffer(valasz.length(), (void *)valasz.c_str(), MHD_RESPMEM_MUST_COPY);
               MHD_add_response_header (response, "Content-Type", "application/json");
               ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
               MHD_destroy_response(response);
     }

     return ret;
}


void request_completed(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe)
{
    struct connection_info_struct *con_info = (connection_info_struct *) *con_cls;

    if (NULL == con_info)
    {
        return;
    }

    MHD_destroy_post_processor(con_info->postprocessor);
    if (con_info->answerstring)
    {
        free(con_info->answerstring);
    }

    free(con_info);
    *con_cls = NULL;
}

int Http_Handler::MakeServer(unsigned short listen_port)
{
	pmhd = MHD_start_daemon(
			MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG | MHD_USE_DUAL_STACK,
			listen_port,
			NULL,
			NULL,
			&MHD_Callback,
			this,
			MHD_OPTION_NOTIFY_COMPLETED,
			request_completed,
			NULL,
			MHD_OPTION_CONNECTION_TIMEOUT,
			(unsigned int) 120,
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
