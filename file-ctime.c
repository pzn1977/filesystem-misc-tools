/* file-ctime, file-atime, file-mtime, file-size
 *    shows file times (or size) recursively
 * (C) 2006 Pedro Zorzenon Neto - Licence GPL2 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <time.h>

#define MAXDEPTH 64

int opt_human = 0;   /* time in human readable format */
int opt_time = 0;  /* use atime, ctime or mtime */

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

char * time_human (time_t t) {
  static char * ret = NULL;
  struct tm * lt;
  if (ret == NULL) {
    ret = malloc(64);
  }
  lt = localtime(&t);
  sprintf(ret,"%04d-%02d-%02d_%02d:%02d:%02d",
	  lt->tm_year+1900, lt->tm_mon+1, lt->tm_mday,
	  lt->tm_hour, lt->tm_min, lt->tm_sec);
  return ret;
}

int scan_recursive (char * dir);

int describe (char * file) {
  int ret = 0;
  struct stat st;
  char f;
  time_t t;
  if (lstat(file,&st) == 0) {
    switch (opt_time) {
    case 'c': /* ctime */
      t = st.st_ctime;
      break;
    case 'a': /* atime */
      t = st.st_atime;
      break;
    case 's': /* size */
      t = st.st_size;
      break;
    default: /* 'm' => mtime, or others */
      t = st.st_mtime;
    }
    f = file_type(st.st_mode);
    if (opt_human) {
      printf("%s %c %s\n", time_human(t), f, file);
    } else {
      printf("%lu %c %s\n", (unsigned long int) t, f, file);
    }
    if (f == 'd') {
      ret |= scan_recursive(file);
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
    describe(dir);
    return 0;
  }

  depth++;
  if(depth >= MAXDEPTH) {
    fprintf(stderr,"max depth reached, will not scan '%s'\n",dir);
    depth--;
    return 4;
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
	  describe(file);
	}
      }
    } while (dent != NULL);
    closedir(d);
  }
  depth--;
  return ret;
}

int main (int argc, char *argv[]) {
  int ret;
  int dir_argv = 1;
  int plen;

  assert(argc > 0);
  plen = strlen(argv[0]);
  if (strcasecmp(argv[0]+plen-5,"ctime") == 0) {
    opt_time = 'c';
  }
  if (strcasecmp(argv[0]+plen-5,"atime") == 0) {
    opt_time = 'a';
  }
  if (strcasecmp(argv[0]+plen-5,"mtime") == 0) {
    opt_time = 'm';
  }
  if (strcasecmp(argv[0]+plen-4,"size") == 0) {
    opt_time = 's';
  }
  if (opt_time == 0) {
    fprintf(stderr,
	    "Warn: program name does not end with mtime, ctime, atime or size\n"
	    "      will use default that is mtime!\n"
	    "      Please rename this executable file!\n");
  }

  if ((argc > 1) &&
      ((strcmp(argv[1],"-h")==0) ||
       (strcmp(argv[1],"--help")==0))) {
    printf("Usage: %s [-h|--help] [-t] [directory | file]\n"
	   "Description: reports file-%ctime, file-type, file-name and\n"
	   "             file-size recursivelly.\n"
	   "Options: -t = show timestamp in human readable format\n"
	   "file-type is f=regular-file d=directory c=char-device\n"
	   "             b=block-device p=fifo l=symlink\n"
	   "             s=socket !=unknown\n"
	   "Hint: rename this program to file-mtime, file-atime\n"
	   "      and file-mtime, it will work as expected :-)\n"
	   "(C) 2006 Pedro Zorzenon Neto <pzn@debian.org>\n"
	   "License: http://www.gnu.org/copyleft/gpl.html\n"
	   ,argv[0], opt_time);
    return 0;
  }

  if (argc > 1) {
    if (strcmp(argv[1],"-t") == 0) {
      if (opt_time == 's') {
        fprintf(stderr,"ERROR: file-size can not be used with option '-t'\n");
	return 1;
      }
      opt_human = 1;
      dir_argv++;
    }
  }
  
  if (argc > dir_argv) {
    ret = scan_recursive(argv[dir_argv]);
  } else {
    ret = scan_recursive(".");
  }
  return ret;
}
