CC=gcc
CFLAGS=-Wall -I polarssl-1.1.4/include/
OPTIMIZATION=-O2
POLARSSL=polarssl-1.1.4/library/

all: picosafe_aes picosafe_sign picosafe_genkey_rsa picosafe_genkey_aes

picosafe_genkey_rsa:
	$(CC) $(CFLAGS) picosafe_genkey_rsa.c $(OPTIMIZATION) $(POLARSSL)/rsa.c $(POLARSSL)/bignum.c $(POLARSSL)/md.c $(POLARSSL)/md_wrap.c $(POLARSSL)/sha1.c $(POLARSSL)/sha2.c $(POLARSSL)/sha4.c $(POLARSSL)/md5.c $(POLARSSL)/entropy.c $(POLARSSL)/havege.c $(POLARSSL)/timing.c $(POLARSSL)/entropy_poll.c $(POLARSSL)/ctr_drbg.c $(POLARSSL)/aes.c $(POLARSSL)/padlock.c -o picosafe_genkey_rsa

picosafe_sign:
	$(CC) $(CFLAGS) picosafe_sign.c $(OPTIMIZATION) $(POLARSSL)/rsa.c $(POLARSSL)/bignum.c $(POLARSSL)/md.c $(POLARSSL)/md_wrap.c $(POLARSSL)/sha1.c $(POLARSSL)/sha2.c $(POLARSSL)/sha4.c $(POLARSSL)/md5.c -o picosafe_sign

picosafe_aes:
	$(CC) $(CFLAGS) $(OPTIMIZATION) -I polarssl-1.1.4/include/ polarssl-1.1.4/library/aes.c polarssl-1.1.4/library/padlock.c slurp.c picosafe_aes.c -o picosafe_aes

picosafe_genkey_aes:
	$(CC) $(CFLAGS) $(OPTIMIZATION) picosafe_genkey_aes.c -o picosafe_genkey_aes

clean:
	rm -rf picosafe_aes picosafe_sign picosafe_genkey_rsa picosafe_genkey_aes

install:
	cp picosafe_genkey_aes /usr/bin
	cp picosafe_genkey_rsa /usr/bin
	cp picosafe_sign       /usr/bin
	cp picosafe_aes        /usr/bin
	cp picosafe_convert    /usr/bin
	chmod +x /usr/bin/picosafe_convert

uninstall:
	rm -rf /usr/bin/picosafe_genkey_rsa /usr/bin/picosafe_sign /usr/bin/picosafe_aes /usr/bin/picosafe_genkey_aes

.PHONY: picosafe_genkey_rsa picosafe_genkey_aes picosafe_sign picosafe_aes
