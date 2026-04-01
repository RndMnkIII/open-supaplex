#include "supaplex_pak.h"
#include "resource_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_FNMATCH
#include <fnmatch.h>
#endif

// Global variable
PakResourceSystem g_pak_system = {
    NULL,   // rm
    0,      // is_initialized
    NULL    // pak_filename
};

// ============================================================================
// INITIALIZATION
// ============================================================================

int pak_system_init(const char *pak_file, void *pak_data, uint32_t pak_size) {
    if (g_pak_system.is_initialized) {
        return 0;  // Already initialized
    }

    if (!pak_file) {
        fprintf(stderr, "PAK: pak_file is NULL\n");
        return -1;
    }

    int data_needs_free = 0;

    // If no pre-loaded data, read from file
    if (!pak_data) {
        FILE *f = fopen(pak_file, "rb");
        if (!f) {
            return -1;  // PAK not available – silent fallback to filesystem
        }

        fseek(f, 0, SEEK_END);
        pak_size = (uint32_t)ftell(f);
        fseek(f, 0, SEEK_SET);

        pak_data = malloc(pak_size);
        if (!pak_data) {
            fclose(f);
            return -1;
        }

        if (fread(pak_data, 1, pak_size, f) != pak_size) {
            free(pak_data);
            fclose(f);
            return -1;
        }

        fclose(f);
        data_needs_free = 1;
    }

    // Initialize resource manager
    g_pak_system.rm = rm_init(pak_data, pak_size);
    if (!g_pak_system.rm) {
        if (data_needs_free) {
            free(pak_data);
        }
        return -1;
    }

    g_pak_system.pak_filename = pak_file;
    g_pak_system.is_initialized = 1;

    return 0;
}

void pak_system_shutdown(void) {
    if (g_pak_system.rm) {
        // Free the pak_data buffer that we allocated in pak_system_init
        free(g_pak_system.rm->pak_data);
        rm_destroy(g_pak_system.rm);
        g_pak_system.rm = NULL;
    }
    g_pak_system.is_initialized = 0;
    g_pak_system.pak_filename = NULL;
}

int pak_system_is_ready(void) {
    return g_pak_system.is_initialized && g_pak_system.rm != NULL;
}

// ============================================================================
// RESOURCE LOADING
// ============================================================================

void* pak_load_resource(const char *filename, uint32_t *out_size) {
    if (!pak_system_is_ready()) {
        if (out_size) *out_size = 0;
        return NULL;
    }

    return rm_load(g_pak_system.rm, filename, out_size);
}

uint32_t pak_load_resource_into(const char *filename, void *buffer, uint32_t max_size) {
    if (!buffer || max_size == 0) {
        return 0;
    }

    uint32_t resource_size;
    void *resource_data = pak_load_resource(filename, &resource_size);

    if (!resource_data) {
        return 0;
    }

    if (resource_size > max_size) {
        fprintf(stderr, "PAK: resource %s too large (%u > %u)\n",
                filename, resource_size, max_size);
        return 0;
    }

    memcpy(buffer, resource_data, resource_size);
    return resource_size;
}

SpaplexResourceEntry* pak_find_resource(const char *filename) {
    if (!pak_system_is_ready()) {
        return NULL;
    }

    return rm_find_entry(g_pak_system.rm, filename);
}

int pak_verify_resource(const char *filename) {
    if (!pak_system_is_ready()) {
        return -1;
    }

    return rm_verify_crc(g_pak_system.rm, filename);
}

#ifdef HAVE_FNMATCH
int pak_list_resources(const char *pattern, SpaplexResourceEntry **entries,
                       int max_entries) {
    if (!pak_system_is_ready()) {
        return 0;
    }

    int count = 0;
    for (uint32_t i = 0; i < g_pak_system.rm->header.num_entries && count < max_entries; i++) {
        SpaplexResourceEntry *entry = &g_pak_system.rm->directory[i];

        if (fnmatch(pattern, entry->filename, 0) == 0) {
            entries[count++] = entry;
        }
    }

    return count;
}
#endif
