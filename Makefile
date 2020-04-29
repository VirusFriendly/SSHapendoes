all:
	gcc -fPIC -shared -o libnss_canary.so.2 -Wl,-soname,libnss_canary.so.2 nss_canary.c
	gcc -fPIC -DPIC -shared -rdynamic -o pam_canary.so pam_canary.c
install:
	install -m 0644 libnss_canary.so.2 /lib
	install -m 0644 pam_canary.so /lib/x86_64-linux-gnu/security
	/sbin/ldconfig -n /lib /usr/lib
clean:
	/bin/rf -rf libnss_canary.so.2
