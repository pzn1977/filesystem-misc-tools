CFLAGS = -Wall -Werror -O2
CC = gcc
BINARIES = file-save-attrs file-ctime file-mtime file-atime file-size

all: $(BINARIES)

clean:
	rm -f *~

distclean: clean
	rm -f $(BINARIES)

%: %.c
	$(CC) $(CFLAGS) -o $@ $^

file-atime: file-ctime
	ln -s $^ $@

file-mtime: file-ctime
	ln -s $^ $@

file-size: file-ctime
	ln -s $^ $@
