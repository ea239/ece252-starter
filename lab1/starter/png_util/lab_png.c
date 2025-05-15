#include "lab_png.h"

int is_png(U8 *buf, size_t n) {
    const U8 png_sign[8] = {0x89,'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    if(n < 16){
        return 0;
    }
    for(int i = 0; i < 8; ++i) {
        if(buf[i] != png_sign[i]) {
            return 0;
        }
    }
    return 1;
}

int get_png_data_IHDR(struct data_IHDR *out, FILE *fp, long offset, int whence) {
    if(fseek(fp, offset + 8, whence) != 0) {
        return 0;
    }

    int ok = 1;
    U32 buf;
    ok &= fread(buf, 1, 4, fp) == 4;
    out->height = ntohl(buf);
    ok &= fread(buf, 1, 4, fp) == 4;
    out->width = ntohl(buf);
    ok &= fread(&out->bit_depth, 1, 1, fp) == 1;
    ok &= fread(&out->color_type, 1, 1, fp) == 1;
    ok &= fread(&out->compression, 1, 1, fp) == 1;
    ok &= fread(&out->filter, 1, 1, fp) == 1;
    ok &= fread(&out->interlace, 1, 1, fp) == 1;

    return ok;
}

int get_png_height(struct data_IHDR *buf) {
    return buf->height;
}

int get_png_width(struct data_IHDR *buf) {
    return buf->width;
}

