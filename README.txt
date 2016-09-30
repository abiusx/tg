Build on mac:
export CFLAGS="-I/usr/local/include -I/usr/local/Cellar/readline/6.3.8/include -I/usr/local/Cellar/openssl/1.0.2h_1/include"
export LDFLAGS="-L/usr/local/lib -L/usr/local/Cellar/readline/6.3.8/lib -L/usr/local/Cellar/openssl/1.0.2h_1/lib"
 ./configure && make