#!/bin/bash

openssl pkcs12 -in client1.testcloud1.p12 -out cacert.pem -cacerts -nokeys
openssl pkcs12 -in client1.testcloud1.p12 -out clcert.pem -clcerts -nokeys
openssl pkcs12 -in client1.testcloud1.p12 -out privkey.pem -nocerts
