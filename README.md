Arrowhead C++ client skeletons

The following Linux packages are prerequisities:
openssl, libgnutls28-dev, libgnutlsxx28, libssl1.1, libssl1.0-dev, libcurl3, libcurl3-gnutls, libcurl4-gnutls-dev, libcrypto++-dev, libcrypto++-utils, libcrypto++6, libgpg-error-dev, automake, texinfo, g++

The project uses libmicrohttpd-0.9.59 as well. Download, compile and install it from source with HTTPS support: https://ftp.gnu.org/gnu/libmicrohttpd/
  >tar -xvzf libmicrohttpd-0.9.59.tar.gz

  >./configure --with-gnutls

  >make

  >sudo make install

Create libmicrohttpd.so.12 file in /usr/lib or usr/local/lib directory (or where the ”ldd ProviderExample” command points):
  >cd /usr/lib

  >sudo ln –s /usr/local/lib/libmicrohttpd.so.12.46.0 libmicrohttpd.so.12
