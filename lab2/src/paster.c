#include "paster.h"
#include "png_op.h"


#define NUM_STRIPS 50

const char *servers[] = {
    "http://ece252-1.uwaterloo.ca:2520",
    "http://ece252-2.uwaterloo.ca:2520",
    "http://ece252-3.uwaterloo.ca:2520"
};

int downloaded[NUM_STRIPS] = {0};
int total_download = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


void *fetch_worker(void *arg) {
    int img_num = *((int*) arg);

    CURL *curl_handle = curl_easy_init();
    RECV_BUF recv_buf;
    recv_buf_init(&recv_buf, BUF_SIZE);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_cb_curl3);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &recv_buf);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, header_cb_curl);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, &recv_buf);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    while (1) {
        pthread_mutex_lock(&lock);
        if(total_download >= 50) {
            pthread_mutex_unlock(&lock);
            break;
        }
        pthread_mutex_unlock(&lock);

        int sidx = rand() % 3;
        char url[256];
        sprintf(url, "%s/image?img=%d", servers[sidx], img_num);
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);

        recv_buf.size = 0;
        recv_buf.seq = -1;

        CURLcode res = curl_easy_perform(curl_handle);
        if (res != CURLE_OK || recv_buf.seq < 0 || recv_buf.seq >= 50) {
            continue;
        }

        pthread_mutex_lock(&lock);
        if(!downloaded[recv_buf.seq]) {
            char fname[64];
            sprintf(fname, "output/output_%d.png", recv_buf.seq);
            write_file(fname, recv_buf.buf, recv_buf.size);
            downloaded[recv_buf.seq] = 1;
            total_download ++;
            //printf("Got strip %d (%d/50)\n", recv_buf.seq, total_download);
        }
        pthread_mutex_unlock(&lock);
        //usleep(10000);
    }

    curl_easy_cleanup(curl_handle);
    recv_buf_cleanup(&recv_buf);
    pthread_exit(NULL);
}

size_t write_cb_curl3(char *p_recv, size_t size, size_t nmemb, void *p_userdata)
{
    size_t realsize = size * nmemb;
    RECV_BUF *p = (RECV_BUF *)p_userdata;
 
    if (p->size + realsize + 1 > p->max_size) {/* hope this rarely happens */ 
        /* received data is not 0 terminated, add one byte for terminating 0 */
        size_t new_size = p->max_size + max(BUF_INC, realsize + 1);   
        char *q = realloc(p->buf, new_size);
        if (q == NULL) {
            perror("realloc"); /* out of memory */
            return -1;
        }
        p->buf = q;
        p->max_size = new_size;
    }

    memcpy(p->buf + p->size, p_recv, realsize); /*copy data from libcurl*/
    p->size += realsize;
    p->buf[p->size] = 0;

    return realsize;
}

size_t header_cb_curl(char *p_recv, size_t size, size_t nmemb, void *userdata)
{
    int realsize = size * nmemb;
    RECV_BUF *p = userdata;
    
    if (realsize > strlen(ECE252_HEADER) &&
	strncmp(p_recv, ECE252_HEADER, strlen(ECE252_HEADER)) == 0) {

        /* extract img sequence number */
	p->seq = atoi(p_recv + strlen(ECE252_HEADER));

    }
    return realsize;
}

int recv_buf_init(RECV_BUF *ptr, size_t max_size)
{
    void *p = NULL;
    
    if (ptr == NULL) {
        return 1;
    }

    p = malloc(max_size);
    if (p == NULL) {
	return 2;
    }
    
    ptr->buf = p;
    ptr->size = 0;
    ptr->max_size = max_size;
    ptr->seq = -1;              /* valid seq should be non-negative */
    return 0;
}

int recv_buf_cleanup(RECV_BUF *ptr)
{
    if (ptr == NULL) {
	return 1;
    }
    
    free(ptr->buf);
    ptr->size = 0;
    ptr->max_size = 0;
    return 0;
}

int write_file(const char *path, const void *in, size_t len) {
    FILE *fp = NULL;

    if (path == NULL) {
        fprintf(stderr, "write_file: file name is null!\n");
        return -1;
    }

    if (in == NULL) {
        fprintf(stderr, "write_file: input data is null!\n");
        return -1;
    }

    fp = fopen(path, "wb");
    if (fp == NULL) {
        perror("fopen");
        return -2;
    }

    if (fwrite(in, 1, len, fp) != len) {
        fprintf(stderr, "write_file: imcomplete write!\n");
        return -3; 
    }
    return fclose(fp);
}

int main(int argc, char** argv) {
    mkdir("output", 0777); 
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
        int rc = pthread_create(&threads[i], NULL, fetch_worker, &n);
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

    cat_png((const char **)filenames, 50, "all.png");
    system("rm -rf output");

    return 0;
}
