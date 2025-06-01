#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "png_op.h"
#include "main_write_header_cb.h"



#define NUM_STRIPS 50

const char *servers[] = {
    "http://ece252-1.uwaterloo.ca:2520",
    "http://ece252-2.uwaterloo.ca:2520",
    "http://ece252-3.uwaterloo.ca:2520"
};

int downloaded[NUM_STRIPS] = {0};
int total_download = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


/* --------------------------- fetch_worker --------------------------- */
void *fetch_worker(void *arg) {
    int img_num = *((int *)arg);
    free(arg);       

    unsigned int seed = (unsigned int)(time(NULL) ^ pthread_self());

    CURL *curl = curl_easy_init();
    if (!curl) pthread_exit(NULL);

    RECV_BUF recv_buf;
    recv_buf_init(&recv_buf, BUF_SIZE);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb_curl3);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &recv_buf);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_cb_curl);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &recv_buf);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2L); 
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); 

    int fail_count = 0;

    while (1) {
        pthread_mutex_lock(&lock);
        if (total_download >= NUM_STRIPS) {
            pthread_mutex_unlock(&lock);
            break;
        }
        pthread_mutex_unlock(&lock);

        int sidx = rand_r(&seed) % 3;
        char url[256];
        snprintf(url, sizeof(url), "%s/image?img=%d", servers[sidx], img_num);
        curl_easy_setopt(curl, CURLOPT_URL, url);

        recv_buf.size = 0;
        recv_buf.seq  = -1;

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK || recv_buf.seq < 0 || recv_buf.seq >= NUM_STRIPS) {
            if (++fail_count > 1000) break;
            continue;
        }  

        int need_write = 0;
        char fname[64];
        unsigned char *buf_copy = NULL;
        size_t size_copy = 0;

        pthread_mutex_lock(&lock);

        if (!downloaded[recv_buf.seq]) {
            downloaded[recv_buf.seq] = 1;
            total_download++;
            printf("Got strip %d, %d remaining\n",
             recv_buf.seq, NUM_STRIPS - total_download); 
            fail_count = 0;
            size_copy = recv_buf.size;
            buf_copy = malloc(size_copy);

            if (buf_copy) {
                memcpy(buf_copy, recv_buf.buf, size_copy);
                snprintf(fname, sizeof(fname), "output/output_%d.png", recv_buf.seq);
                need_write = 1;
            } else {
                downloaded[recv_buf.seq] = 0;
                total_download--;
            }

        } else {
            if (++fail_count > 1000) {
                pthread_mutex_unlock(&lock);
                break;
            }
        }
        pthread_mutex_unlock(&lock);

        if (need_write && buf_copy) {
            if (write_file(fname, buf_copy, size_copy) != 0)
                fprintf(stderr, "write_file failed for %s\n", fname);
            free(buf_copy);
        }
    }

    curl_easy_cleanup(curl);
    recv_buf_cleanup(&recv_buf);
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if (mkdir("output", 0777) && errno != EEXIST) return 1;
    int t = 1;
    int n = 1;

    if(argc == 3) {
        if(strcmp(argv[1], "-t") == 0) {
            t = atoi(argv[2]);
        } else {
            n = atoi(argv[2]);
        }
    } else if (argc == 5) {
        t = atoi(argv[2]);
        n = atoi(argv[4]);
    }

    pthread_t threads[t];

    for(int i = 0; i < t; ++i) {
        int* img_arg = malloc(sizeof(int));
        if (!img_arg) {
            perror("malloc");
            exit(1);
        }
        *img_arg = n; 
        int rc = pthread_create(&threads[i], NULL, fetch_worker, img_arg);
        if (rc) {
        fprintf(stderr, "Failed to create thread. Error code: %d\n", rc);
            return 1;
        }
    }

    for(int i = 0; i < t; ++i) {
        pthread_join(threads[i], NULL);
    }

    char *filenames[NUM_STRIPS];
    for (int i = 0; i < NUM_STRIPS; i++) {
        filenames[i] = malloc(32);
        if (filenames[i] == NULL) {
            perror("malloc");
            exit(1);
        }

        sprintf(filenames[i], "output/output_%d.png", i);
    }

    if (total_download != NUM_STRIPS) {
        fprintf(stderr, "Only got %d/%d strips, aborting cat_png.\n",
                total_download, NUM_STRIPS);
        exit(EXIT_FAILURE);
    }

    cat_png((const char **)filenames, 50, "all.png");
    system("rm -rf output");

    for (int i = 0; i < NUM_STRIPS; i++) {
        free(filenames[i]);
    }

    return 0;
}
