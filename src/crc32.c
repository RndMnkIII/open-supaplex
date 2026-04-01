#include "crc32.h"

static uint32_t crc32_table[256];
static int crc32_table_computed = 0;

static void crc32_make_table(void) {
    uint32_t c;
    int n, k;

    for (n = 0; n < 256; n++) {
        c = (uint32_t) n;
        for (k = 0; k < 8; k++) {
            if (c & 1) {
                c = 0xedb88320L ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        crc32_table[n] = c;
    }
    crc32_table_computed = 1;
}

uint32_t crc32_calculate(const uint8_t *data, size_t length) {
    uint32_t crc = 0xffffffffL;
    size_t i;

    if (!crc32_table_computed) {
        crc32_make_table();
    }

    for (i = 0; i < length; i++) {
        crc = crc32_table[(crc ^ data[i]) & 0xff] ^ (crc >> 8);
    }

    return crc ^ 0xffffffffL;
}
