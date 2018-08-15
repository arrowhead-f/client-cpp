#!/bin/bash

openssl pkcs12 -in authorization.testcloud1.p12 -out autho_cacert.pem  -cacerts -nokeys
openssl pkcs12 -in authorization.testcloud1.p12 -out autho_clcert.pem  -clcerts -nokeys
openssl pkcs12 -in authorization.testcloud1.p12 -out autho_privkey.pem -nocerts
openssl rsa -in autho_privkey.pem -pubout       -out autho_pubkey.pem