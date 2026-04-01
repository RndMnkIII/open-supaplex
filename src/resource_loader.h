#ifndef RESOURCE_LOADER_H
#define RESOURCE_LOADER_H

#include <stdint.h>
#include "pak_format.h"

typedef struct {
    void *pak_data;                    // Pointer to PAK data in memory
    uint32_t pak_size;
    SpaplexResourceHeader header;
    SpaplexResourceEntry *directory;   // Directory table
} SpaplexResourceManager;

// Initialize manager from PAK in memory
SpaplexResourceManager* rm_init(void *pak_ptr, uint32_t pak_size);

// Load resource from PAK
void* rm_load(SpaplexResourceManager *rm, const char *filename, uint32_t *out_size);

// Find specific entry
SpaplexResourceEntry* rm_find_entry(SpaplexResourceManager *rm, const char *filename);

// Clean up
void rm_destroy(SpaplexResourceManager *rm);

// Validate integrity
int rm_verify_crc(SpaplexResourceManager *rm, const char *filename);

#endif
