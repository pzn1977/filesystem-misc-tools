/* file-save-attrs -- save file attributes recursively
 * (C) 2006 Pedro Zorzenon Neto - Licence GPL2.0 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <time.h>
#include <stdint.h>

#define MAXDEPTH 64
#define MAXREADLINK 8192

int opt_xdev = 0;
dev_t start_dev = 0;

char file_type (mode_t m) {
  if (S_ISREG(m)) return 'f';
  if (S_ISDIR(m)) return 'd';
  if (S_ISCHR(m)) return 'c';
  if (S_ISBLK(m)) return 'b';
  if (S_ISFIFO(m)) return 'p';
  if (S_ISLNK(m)) return 'l';
  if (S_ISSOCK(m)) return 's';
  return '!';
}

char path_type (char * path) {
  struct stat st;
  if (lstat(path,&st) == 0) {
    return file_type(st.st_mode);
  }
  return '!';
}

int scan_recursive (char * dir);

void get_start_dev (char * file) {
  struct stat st;
  if (lstat(file,&st) == 0) {
    start_dev = st.st_dev;
  }
}

int describe_link (char * file, uintmax_t dev, uintmax_t ino) {
  int ret = 0;
  char s[MAXREADLINK+1];
  int i;
  i = readlink(file,s,MAXREADLINK);
  if (i > MAXREADLINK) i = MAXREADLINK;
  if (i > 0) {
    s[i] = 0;
    printf("L %ju %ju %s -> %s\n", dev, ino, file, s);
  } else {
    printf("L %ju %ju %s -> [readlink-failed]\n", dev, ino, file);
    ret |= 2;
  }
  return ret;
}

int describe (char * file) {
  int ret = 0;
  struct stat st;
  char f;
  if (lstat(file,&st) == 0) {
    if ((opt_xdev) && (start_dev != st.st_dev)) return ret;
    f = file_type(st.st_mode);
    printf("%c %ju %ju %u %ju %ju %u %u %ju %ju %ju %ju %s\n",
           f, (uintmax_t) st.st_dev, (uintmax_t) st.st_ino,
           (~S_IFMT) & st.st_mode, (uintmax_t) st.st_nlink,
	   (uintmax_t) st.st_size, st.st_uid, st.st_gid, st.st_rdev,
           (uintmax_t) st.st_atime, (uintmax_t) st.st_mtime,
           (uintmax_t) st.st_ctime, file);
    if (f == 'd') {
      ret |= scan_recursive(file);
    }
    if (f == 'l') {
      ret |= describe_link (file,(uintmax_t) st.st_dev,(uintmax_t) st.st_ino);
    }
  } else {
    char msg[8192];
    sprintf(msg, "could not stat file '%s'", file);
    perror(msg);
    ret |= 1;
  }
  return ret;
}

int scan_recursive (char * dir) {
  static int depth = 0;
  DIR * d;
  struct dirent * dent;
  char file[8192];
  int ret = 0;

  if (path_type(dir) != 'd') {
    ret |= describe(dir);
    return ret;
  }

  depth++;
  if(depth >= MAXDEPTH) {
    fprintf(stderr,"max depth reached, will not scan '%s'\n",dir);
    depth--;
    ret |= 4;
    return ret;
  }
  d = opendir(dir);
  if (d == NULL) {
    char msg[8192];
    sprintf(msg, "could not open '%s'", dir);
    perror(msg);
    ret |= 1;
  } else {
    do {
      dent = readdir(d);
      if (dent != NULL) {
	if ((strcmp(dent->d_name,".")!=0) &&
	    (strcmp(dent->d_name,"..")!=0)) {
	  sprintf(file, "%s/%s", dir, dent->d_name);
	  ret |= describe(file);
	}
      }
    } while (dent != NULL);
    closedir(d);
  }
  depth--;
  return ret;
}

int main (int argc, char *argv[]) {

  printf("#type dev ino mode nlink size uid gid rdev atime mtime ctime file\n");
  printf("#types: f(file) d(directory) c(chardev) b(blockdev) p(pipe) l(link) s(sock)\n");
  printf("#type 'L' is used to describe link destination\n");

  if ((argc > 1) &&
      (strcmp(argv[1],"--xdev")==0)) {
    opt_xdev = 1;
  }

  if (((opt_xdev) && (argc == 2)) ||
      (argc < 2)) {
    get_start_dev (".");
    return describe(".");
  } else {
    get_start_dev(argv[argc-1]);
    return describe(argv[argc-1]);
  }
}
