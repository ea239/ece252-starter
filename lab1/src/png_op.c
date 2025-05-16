#include "png_op.h"

static char** result_list = NULL;
static int result_index = 0;

int end_with_png(const char *filename) {
    const char *exd = strrchr(filename, '.');
    return exd && strcmp(exd, ".png") == 0;
}

int check_png(const char *path) {
    FILE* fp = fopen(path, "rb");
    if(!fp) {
        perror("Error opening png file\n");
        return 1;
    }
    U8 header[8];
    fread(header, 1, 8, fp);
    fclose(fp);
    return is_png(header, 8);
}

void search_dir(const char *path) {
    DIR* dir = opendir(path);
    if (!dir) {
        perror("Error opening dir\n");
        return;
    }

    struct dirent* entry;
    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath),"%s/%s", path, entry->d_name);

        struct stat st;
        if(stat(fullpath, &st) == -1) continue;

        if(S_ISDIR(st.st_mode)) {
            search_dir(fullpath);
        } else if(S_ISREG(st.st_mode) && end_with_png(entry->d_name)) {
            if (check_png(fullpath)) result_list[result_index++] = strdup(fullpath);
        }

        if(result_index >= MAX_FILES - 1) break;
    }
    closedir(dir);
}


char** findpng(const char* path) {
    result_list = malloc(sizeof(char*) * MAX_FILES);
    result_index = 0;
    search_dir(path);
    result_list[result_index] = NULL;
    printf("result_index: %d\n", result_index);
    return result_list;
}

