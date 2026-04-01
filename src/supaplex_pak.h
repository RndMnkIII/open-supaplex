#ifndef SUPAPLEX_PAK_H
#define SUPAPLEX_PAK_H

#include <stdint.h>
#include "pak_format.h"
#include "resource_loader.h"

// ============================================================================
// GLOBAL PAK RESOURCE MANAGER
// ============================================================================

typedef struct {
    SpaplexResourceManager *rm;
    int is_initialized;
    const char *pak_filename;
} PakResourceSystem;

// Global variable
extern PakResourceSystem g_pak_system;

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

/**
 * Initialize PAK resource system
 * @param pak_file  Path to the PAK file
 * @param pak_data  Pointer to pre-loaded PAK data in memory (may be NULL)
 * @param pak_size  Size of the PAK file
 * @return 0 on success, -1 on error
 */
int pak_system_init(const char *pak_file, void *pak_data, uint32_t pak_size);

/**
 * Shut down PAK resource system
 */
void pak_system_shutdown(void);

/**
 * Check whether the system is initialized
 */
int pak_system_is_ready(void);

// ============================================================================
// RESOURCE LOADING (replaces fopen/fread)
// ============================================================================

/**
 * Load resource from PAK
 * @param filename  Resource name (e.g. "GFX.DAT" or "audio/music.ogg")
 * @param out_size  Receives the data size
 * @return Pointer to data, or NULL if not found
 */
void* pak_load_resource(const char *filename, uint32_t *out_size);

/**
 * Load resource and copy into buffer
 * @param filename  Resource name
 * @param buffer    Destination buffer
 * @param max_size  Maximum buffer size
 * @return Bytes copied, 0 on error
 */
uint32_t pak_load_resource_into(const char *filename, void *buffer, uint32_t max_size);

/**
 * Find a resource without loading it
 * @param filename  Resource name
 * @return Pointer to entry, or NULL if not found
 */
SpaplexResourceEntry* pak_find_resource(const char *filename);

/**
 * Verify resource integrity
 * @param filename  Resource name
 * @return 0=OK, 1=CRC error, -1=not found
 */
int pak_verify_resource(const char *filename);

#ifdef HAVE_FNMATCH
/**
 * List all resources matching a glob pattern
 * @param pattern     Glob pattern (e.g. "audio/*", "level*.dat")
 * @param entries     Array to receive entries
 * @param max_entries Maximum number of entries
 * @return Number of resources found
 */
int pak_list_resources(const char *pattern, SpaplexResourceEntry **entries,
                       int max_entries);
#endif

#endif
