all:
	gcc -fPIC -shared -o libnss_sheep.so.2 -Wl,-soname,libnss_sheep.so.2 nss_sheep.c
	gcc -fPIC -DPIC -shared -rdynamic -o pam_sheep.so pam_sheep.c
install:
	install -m 0644 libnss_sheep.so.2 /lib
	install -m 0644 pam_sheep.so /lib/x86_64-linux-gnu/security
	/sbin/ldconfig -n /lib /usr/lib
clean:
	/bin/rf -rf libnss_sheep.so.2
