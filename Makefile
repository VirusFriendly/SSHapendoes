all:
	gcc -fPIC -shared -o libnss_catchall.so.2 -Wl,-soname,libnss_catchall.so.2 nss_catchall.c
	gcc -fPIC -DPIC -shared -rdynamic -o pam_catchall.so pam_catchall.c
install:
	install -m 0644 libnss_catchall.so.2 /lib
	install -m 0644 pam_catchall.so /lib/x86_64-linux-gnu/security
	/sbin/ldconfig -n /lib /usr/lib
clean:
	/bin/rf -rf libnss_catchall.so.2
