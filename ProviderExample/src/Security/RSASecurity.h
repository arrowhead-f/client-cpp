
#pragma once

#include <string>
#include <cstring>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <assert.h>

#include "base64.h"

#include <stdio.h>

#define AUTHORIZERPUBLICKEYPATH "keys/authorizerPublicKey.pem"

class RSASecurity{

    public:

	std::string privateKeyPath;
	std::string publicKeyPath;

	std::string sB64EncodedRSAEncryptedToken;
	std::string sB64EncodedSignature;

	void updateAuthPublicKeyFromFile(std::string filePath);

	RSA *createPrivateRSA(std::string key);
	RSA *createPublicRSA(std::string key);
	bool RSAVerifySignature(RSA *rsa, unsigned char *MsgHash, size_t MsgHashLen, const char *msg, size_t MsgLen, bool *authentic);
	bool verifySignature(std::string plainText, char *signatureBase64);

	std::string getDecryptedToken();
	bool getVerificationResult();

};
