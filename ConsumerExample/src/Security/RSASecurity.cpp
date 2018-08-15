
#include "RSASecurity.h"

RSA* RSASecurity::createPrivateRSA(std::string key) {
	RSA *rsa = NULL;
	const char* c_string = key.c_str();
	BIO * keybio = BIO_new_mem_buf((void*)c_string, -1);

	if (keybio==NULL) {
		return 0;
	}

	rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa,NULL, NULL);
	return rsa;
}

RSA* RSASecurity::createPublicRSA(std::string key) {
	RSA *rsa = NULL;
	BIO *keybio;
	const char* c_string = key.c_str();
	keybio = BIO_new_mem_buf((void*)c_string, -1);
	if (keybio==NULL) {
		return 0;
	}
	rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa,NULL, NULL);
	return rsa;
}

bool RSASecurity::RSAVerifySignature( RSA* rsa,
                     unsigned char* MsgHash,
                     size_t MsgHashLen,
                     const char* Msg,
                     size_t MsgLen,
                     bool* Authentic) {

	*Authentic = false;
	EVP_PKEY* pubKey  = EVP_PKEY_new();
	EVP_PKEY_assign_RSA(pubKey, rsa);
	EVP_MD_CTX *m_RSAVerifyCtx = EVP_MD_CTX_create();

	if (EVP_DigestVerifyInit(m_RSAVerifyCtx,NULL, EVP_sha256(),NULL,pubKey)<=0) {
          return false;
	}

	if (EVP_DigestVerifyUpdate(m_RSAVerifyCtx, Msg, MsgLen) <= 0) {
          return false;
	}

	int AuthStatus = EVP_DigestVerifyFinal(m_RSAVerifyCtx, MsgHash, MsgHashLen);

	if (AuthStatus==1) {
          *Authentic = true;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
          EVP_MD_CTX_cleanup(m_RSAVerifyCtx);
#else
          EVP_MD_CTX_free(m_RSAVerifyCtx);
#endif
          return true;
	}
	else if(AuthStatus==0){
          *Authentic = false;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
          EVP_MD_CTX_cleanup(m_RSAVerifyCtx);
#else
          EVP_MD_CTX_free(m_RSAVerifyCtx);
#endif
          return true;
	}
	else{
          *Authentic = false;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
          EVP_MD_CTX_cleanup(m_RSAVerifyCtx);
#else
          EVP_MD_CTX_free(m_RSAVerifyCtx);
#endif
          return false;
	}
}

bool RSASecurity::verifySignature(std::string plainText, char* signatureBase64) {

	FILE *keyfile = fopen(AUTHORIZERPUBLICKEYPATH, "r");

	if(!keyfile){
	    printf("Error: Cannot open authorizer public key\n");
	    return 0;
	}

	RSA *publicRSA = PEM_read_RSA_PUBKEY(keyfile, NULL, NULL, NULL);
	fclose(keyfile);

	std::string sencMessage = base64_decode((std::string)signatureBase64);

	unsigned char* encMessage = (unsigned char*)sencMessage.c_str();
	size_t encMessageLength = sencMessage.length();
	bool authentic;

	bool result = RSAVerifySignature(publicRSA, encMessage, encMessageLength, plainText.c_str(), plainText.length(), &authentic);
	return result & authentic;
}

inline int pem_password_callback(char *buf, int max_len, int flag, void *ctx){
    const char* PASSWD = "12345";
    int len = strlen(PASSWD);

    if(len > max_len)
        return 0;

    memcpy(buf, PASSWD, len);
    return len;
}

std::string RSASecurity::getDecryptedToken(){

    std::string b64DecodedRSAEncryptedToken = base64_decode(sB64EncodedRSAEncryptedToken);

    FILE *keyfile = fopen(privateKeyPath.c_str(), "r");

    if(!keyfile){
        printf("Error: Cannot open provider private key: %s\n", privateKeyPath.c_str());
        return 0;
    }

     RSA *rsa_privatekey = PEM_read_RSAPrivateKey(keyfile, NULL, pem_password_callback, NULL);

     if(rsa_privatekey == NULL){
          printf("Error: Cannot open provider RSA private key: %s\n");
          return (std::string)"error";
     }

     fclose(keyfile);

     unsigned char *decryptedToken = (unsigned char*) malloc(2000);

     RSA_private_decrypt(
          (int)RSA_size(rsa_privatekey),
          (const unsigned char*)b64DecodedRSAEncryptedToken.c_str(),
          decryptedToken,
          rsa_privatekey,
          RSA_PKCS1_PADDING
    );

     char* jsonEnd = (char *)"}";

     char *p = strstr((char*)decryptedToken, jsonEnd);
     if(p == NULL){
          return (std::string)"error";
     }
     p[1] = '\0';

     return (std::string)((const char*)decryptedToken);
}

bool RSASecurity::getVerificationResult(){
     std::string b64DecodedToken = base64_decode(sB64EncodedRSAEncryptedToken);

     return verifySignature(b64DecodedToken, (char *)sB64EncodedSignature.c_str());
}

