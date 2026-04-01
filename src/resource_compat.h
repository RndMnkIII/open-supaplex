#ifndef RESOURCE_COMPAT_H
#define RESOURCE_COMPAT_H

// This module provides functions compatible with existing code.
// It replaces fopen/fread/fclose for PAK resources.

#include <stdio.h>
#include <stdint.h>

/**
 * FILE-compatible structure for PAK resources
 */
typedef struct {
    const void *data;      // Pointer to PAK data
    uint32_t position;     // Current read position
    uint32_t size;         // Total size
    int is_pak;            // Flag: resource came from PAK
} PAK_FILE;

/**
 * Open resource (from PAK or from filesystem)
 */
PAK_FILE* pak_fopen(const char *filename, const char *mode);

/**
 * Read from resource
 */
size_t pak_fread(void *ptr, size_t size, size_t nmemb, PAK_FILE *stream);

/**
 * Seek within resource
 */
int pak_fseek(PAK_FILE *stream, long offset, int whence);

/**
 * Get current position
 */
long pak_ftell(PAK_FILE *stream);

/**
 * Close resource
 */
int pak_fclose(PAK_FILE *stream);

#endif
