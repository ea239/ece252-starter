#define _GNU_SOURCE
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include "lab_png.h"


#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#define MAX_FILES 4096


char** findpng(const char *path);

int end_with_png(const char *filename);

void search_dir(const char *path);

int check_png(const char *path);
  


