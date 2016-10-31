CFLAGS := -std=c11 -fPIC \
	-Wall -Wextra -pedantic \
	-Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes \
	-Wno-gnu-empty-initializer \
	-D_GNU_SOURCE \
	-ggdb3 -O0 \
	$(CFLAGS)

LDLIBS=

all: strip

nsstmp-ctl: nsstmp-ctl.o storage.o

strip: nsstmp-ctl libnss_tmp.so.2
	@strip nsstmp-ctl libnss_tmp.so.2

libnss_tmp.so.2: nss.o storage.o
	$(CC) -fPIC -Wall -shared -o libnss_tmp.so.2 -Wl,-soname,libnss_tmp.so.2 $^ $(LDLIBS)
clean:
	$(RM) nsstmp-ctl libnss_tmp.so.2 *.o

install: all
	cp libnss_tmp.so.2 /usr/local/lib/
	cp nsstmp-ctl /usr/local/bin
	ldconfig

uninstall:
	rm -f /usr/local/lib/libnss_tmp.so.2
	rm -f /usr/local/bin/nsstmp-ctl
	@ldconfig
	@echo "Uninstalled"

.PHONY: clean
