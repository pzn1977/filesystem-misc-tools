CFLAGS = -Wall -Werror -O2
CC = gcc
BINARIES = file-save-attrs file-ctime file-mtime file-atime file-size

all: $(BINARIES)

clean:
	rm -f *~

distclean: clean
	rm -f $(BINARIES)

install: all
	install file-save-attrs $(DESTDIR)/usr/local/bin/
	install file-ctime $(DESTDIR)/usr/local/bin/
	cd $(DESTDIR)/usr/local/bin/ && ln -s -f file-ctime file-atime
	cd $(DESTDIR)/usr/local/bin/ && ln -s -f file-ctime file-mtime
	cd $(DESTDIR)/usr/local/bin/ && ln -s -f file-ctime file-size

%: %.c
	$(CC) $(CFLAGS) -o $@ $^

file-atime: file-ctime
	ln -s $^ $@

file-mtime: file-ctime
	ln -s $^ $@

file-size: file-ctime
	ln -s $^ $@
