#include "resource_loader.h"
#include "crc32.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

SpaplexResourceManager* rm_init(void *pak_ptr, uint32_t pak_size) {
    if (!pak_ptr || pak_size < PAK_HEADER_SIZE) {
        fprintf(stderr, "Error: invalid PAK\n");
        return NULL;
    }

    SpaplexResourceManager *rm = malloc(sizeof(SpaplexResourceManager));
    if (!rm) {
        return NULL;
    }
    rm->pak_data = pak_ptr;
    rm->pak_size = pak_size;

    // Copy header
    memcpy(&rm->header, pak_ptr, PAK_HEADER_SIZE);

    // Validate magic
    if (rm->header.magic != PAK_MAGIC) {
        fprintf(stderr, "Error: invalid PAK magic (0x%08x)\n", rm->header.magic);
        free(rm);
        return NULL;
    }

    // Validate directory offset
    if (rm->header.dir_offset >= pak_size) {
        fprintf(stderr, "Error: PAK directory offset out of bounds\n");
        free(rm);
        return NULL;
    }

    // Point to directory
    rm->directory = (SpaplexResourceEntry*)((char*)pak_ptr + rm->header.dir_offset);

    return rm;
}

SpaplexResourceEntry* rm_find_entry(SpaplexResourceManager *rm, const char *filename) {
    if (!rm || !filename) {
        return NULL;
    }
    for (uint32_t i = 0; i < rm->header.num_entries; i++) {
        if (strcmp(rm->directory[i].filename, filename) == 0) {
            return &rm->directory[i];
        }
    }
    return NULL;
}

void* rm_load(SpaplexResourceManager *rm, const char *filename, uint32_t *out_size) {
    SpaplexResourceEntry *entry = rm_find_entry(rm, filename);
    if (!entry) {
        if (out_size) *out_size = 0;
        return NULL;
    }

    if (out_size) {
        *out_size = entry->size;
    }

    return (char*)rm->pak_data + entry->offset;
}

int rm_verify_crc(SpaplexResourceManager *rm, const char *filename) {
    SpaplexResourceEntry *entry = rm_find_entry(rm, filename);
    if (!entry) {
        return -1;
    }

    uint8_t *data = (uint8_t*)rm->pak_data + entry->offset;
    uint32_t calculated_crc = crc32_calculate(data, entry->size);

    return (calculated_crc == entry->crc32) ? 0 : 1;  // 0=OK, 1=ERROR
}

void rm_destroy(SpaplexResourceManager *rm) {
    if (rm) {
        free(rm);
    }
}
