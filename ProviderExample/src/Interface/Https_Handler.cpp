
#include "Https_Handler.hpp"
#include <cstring>


size_t Https_Handler::httpsResponseCallback(char *ptr, size_t size)
{
	return size;
}

size_t httpsResponseHandler(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	return ((Https_Handler *)userdata)->httpsResponseCallback(ptr, size*nmemb);
}


int Https_Handler::SendHttpsRequest(string pdata, string paddr, string pmethod)
{
     printf("SendHttpsRequest: %s\n", paddr.c_str());

     int http_code = 0;
     CURLcode res;
  	CURL *curl;
	struct curl_slist *headers = NULL;
  	string agent;

	curl_global_init(CURL_GLOBAL_ALL);

  	curl = curl_easy_init();

  	if(curl){
		agent = "libcurl/"+string(curl_version_info(CURLVERSION_NOW)->version);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, agent.c_str());

		headers = curl_slist_append(headers, "Expect:");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		if(pmethod == "POST")
			curl_easy_setopt(curl, CURLOPT_POST, true);
		else
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, pmethod.c_str());


          //---------------HTTPS SECTION--------------------------------------------------------
          //--verbose
          //if ( curl_easy_setopt(curl, CURLOPT_VERBOSE,        1L)            != CURLE_OK)
          // printf("error: CURLOPT_VERBOSE\n");
          //--insecure
          if ( curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L)            != CURLE_OK)
               printf("error: CURLOPT_SSL_VERIFYPEER\n");
          if ( curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L)            != CURLE_OK)
               printf("error: CURLOPT_SSL_VERIFYHOST\n");
          //--cert
          //if ( curl_easy_setopt(curl, CURLOPT_SSLCERT,        "keys/tempsensor.testcloud1.publicCert.pem")  != CURLE_OK)
		  if ( curl_easy_setopt(curl, CURLOPT_SSLCERT,        "keys2/tempsensor.testcloud2.clcert.pem")  != CURLE_OK)
               printf("error: CURLOPT_SSLCERT\n");
          //--cert-type
          if ( curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE,    "PEM")         != CURLE_OK)
               printf("error: CURLOPT_SSLCERTTYPE\n");
          //--key
          //if ( curl_easy_setopt(curl, CURLOPT_SSLKEY,         "keys/tempsensor.testcloud1.private.key") != CURLE_OK)
		  if ( curl_easy_setopt(curl, CURLOPT_SSLKEY,         "keys2/tempsensor.testcloud2.privkey.pem") != CURLE_OK)
               printf("error: CURLOPT_SSLKEY\n");
          //--key-type
          if ( curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE,     "PEM")         != CURLE_OK)
               printf("error: CURLOPT_SSLKEYTYPE\n");
          //--pass
          if ( curl_easy_setopt(curl, CURLOPT_KEYPASSWD,      "123456")       != CURLE_OK)
               printf("error: CURLOPT_KEYPASSWD\n");
          //--cacert
          //if ( curl_easy_setopt(curl, CURLOPT_CAINFO,         "keys/tempsensor.testcloud1.caCert.pem")  != CURLE_OK)
		  if ( curl_easy_setopt(curl, CURLOPT_CAINFO,         "keys2/tempsensor.testcloud2.caCert.pem")  != CURLE_OK)
               printf("error: CURLOPT_CAINFO\n");
          //
          //---------------END OF HTTPS SECTION-------------------------------------------------

          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pdata.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &httpsResponseHandler);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

		curl_easy_setopt(curl, CURLOPT_URL, paddr.c_str());

		res = curl_easy_perform(curl);

		if(res != CURLE_OK)
			printf("Error: curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

          curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
  	}
     curl_global_cleanup();
     return http_code;
}

static long get_file_size (const char *filename)
{
  FILE *fp;

  fp = fopen (filename, "rb");
  if (fp)
    {
      long size;

      if ((0 != fseek (fp, 0, SEEK_END)) || (-1 == (size = ftell (fp))))
        size = 0;

      fclose (fp);

      return size;
    }
  else
    return 0;
}

static char *load_file (const char *filename)
{
  FILE *fp;
  char *buffer;
  long size;

  size = get_file_size (filename);
  if (size == 0)
    return NULL;

  fp = fopen (filename, "rb");
  if (!fp)
    return NULL;

  buffer = (char *)malloc (size);
  if (!buffer)
    {
      fclose (fp);
      return NULL;
    }

  if (size != fread (buffer, 1, size, fp))
    {
      free (buffer);
      buffer = NULL;
    }

  fclose (fp);
  return buffer;
}
// This responds to HTTPS GET!
// can be Overloaded in child class!
int Https_Handler::httpsGETCallback(const char *Id, string *pData_str, string sToken, string sSignature, string clientDistName)
{
    *pData_str = "1234";
    return 1;
}

typedef pair<const char*, string> str_param_t;

static int http_str_param_iter(void *cls, enum MHD_ValueKind kind, const char *key, const char *value){
    str_param_t *p = (str_param_t*)cls;
    if(!strcmp(p->first, key)){
	p->second = value;
	return MHD_NO;
    }
    else{
	return MHD_YES; //continue iteration
    }

}

static inline int get_auth_dn(gnutls_x509_crt_t client_cert, char **buf) {
        size_t len = 0;
        int r;

        r = gnutls_x509_crt_get_dn(client_cert, NULL, &len);
        if (r != GNUTLS_E_SHORT_MEMORY_BUFFER) {
                printf("Error: gnutls_x509_crt_get_dn failed");
                return r;
        }

        *buf = (char*)malloc(len);

        gnutls_x509_crt_get_dn(client_cert, *buf, &len);
        return 0;
}

static inline void gnutls_x509_crt_deinitp(gnutls_x509_crt_t *p) {
        gnutls_x509_crt_deinit(*p);
}

static inline int get_client_cert(gnutls_session_t session, gnutls_x509_crt_t *client_cert) {
        const gnutls_datum_t *pcert;
        unsigned listsize;
        gnutls_x509_crt_t cert;
        int r;

        pcert = gnutls_certificate_get_peers(session, &listsize);
        if (!pcert || !listsize) {

                printf("Failed to retrieve certificate chain, pcert: %p, listsize: %d\n", pcert, listsize);
                return -EINVAL;
        }

        r = gnutls_x509_crt_init(&cert);
        if (r < 0) {
                printf("Failed to initialize client certificate");
                return r;
        }

        /* Note that by passing values between 0 and listsize here, you
           can get access to the CA's certs */
        r = gnutls_x509_crt_import(cert, &pcert[0], GNUTLS_X509_FMT_DER);
        if (r < 0) {
                printf("Failed to import client certificate");
                gnutls_x509_crt_deinit(cert);
                return r;
        }

        *client_cert = cert;
        return 0;
}

// static C function (not member of class!)
extern "C" int MHD_Callback_Https(void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size, void **ptr)
{
	static int aptr;
	struct MHD_Response *response;
	string cb_data;
	int value;
	int ret = MHD_YES;

	if (0 != strcmp (method, "GET"))
          return MHD_NO;              // unexpected method

	if (&aptr != *ptr){
		// do never respond on first call
		*ptr = &aptr;
		return MHD_YES;
	}
	*ptr = NULL;                  // reset when done

     /*Get URL parameters, token and signature*/
     str_param_t sp_token("token", "default_value");
     MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, &http_str_param_iter, &sp_token);
     string param_token = sp_token.second;

     str_param_t sp_signature("signature", "default_value");
     MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, &http_str_param_iter, &sp_signature);
     string param_signature = sp_signature.second;

     //Get CN from client cert
     const union MHD_ConnectionInfo *ci;
     gnutls_session_t session;
     gnutls_x509_crt_t client_cert = NULL;
     char *clientDistinguishedName = NULL;

     int r;

     ci = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_GNUTLS_SESSION);

     session = (gnutls_session_t)(ci->tls_session);

     r = get_client_cert(session, &client_cert);
     if(r<0)
          printf("Error: get_client_cert, %d, %s\n", r, gnutls_strerror_name(r));


     r = get_auth_dn(client_cert, &clientDistinguishedName);
     if(r<0)
          printf("Error: get_auth_dn return value: %d, %s\n", r, gnutls_strerror_name(r));

     if(clientDistinguishedName == NULL){
          clientDistinguishedName = (char *)"error";
     }

	// Cast callback to creators class (passing C++ in C functions...)
     value = ((Https_Handler *)cls)->httpsGETCallback(url, &cb_data, param_token, param_signature, (std::string)clientDistinguishedName);

     if (value)
     {
        response = MHD_create_response_from_buffer(cb_data.length(), (void *)cb_data.c_str(), MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header (response, "Content-Type", "application/json");
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
     }

     return ret;
}

int Https_Handler::MakeHttpsServer(unsigned short listen_port)
{
	key_pem     = load_file (SERVERKEYFILE);
	cert_pem    = load_file (SERVERCERTFILE);
     root_ca_pem = load_file (ROOTCACERTFILE);

	if ((key_pem == NULL) || (cert_pem == NULL))
	{
  		printf ("The key/certificate files could not be read.\n");
  		return 1;
	}
	pmhd = MHD_start_daemon(
			MHD_USE_SELECT_INTERNALLY | MHD_USE_SSL | MHD_USE_DEBUG | MHD_USE_DUAL_STACK,
			listen_port,
			NULL, NULL, &MHD_Callback_Https, this,
			MHD_OPTION_HTTPS_MEM_KEY, key_pem,
			MHD_OPTION_HTTPS_KEY_PASSWORD, "123456",
               MHD_OPTION_HTTPS_MEM_CERT, cert_pem,
               MHD_OPTION_HTTPS_MEM_TRUST, root_ca_pem,
			MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
			MHD_OPTION_END);
	if (pmhd == NULL){
		printf("Server creation failed!\n");
		return 1;
	}

return 0;
}

int Https_Handler::KillHttpsServer()
{
	if (pmhd)
	{
		MHD_stop_daemon(pmhd);
		free (key_pem);
		free (cert_pem);
		pmhd = NULL;
		return 0;
	}
return 1;
}
