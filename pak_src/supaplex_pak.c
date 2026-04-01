#include "supaplex_pak.h"
#include "resource_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>

// Variable global
PakResourceSystem g_pak_system = {
    .rm = NULL,
    .is_initialized = 0,
    .pak_filename = NULL
};

// ============================================================================
// INICIALIZACIÓN
// ============================================================================

int pak_system_init(const char *pak_file, void *pak_data, uint32_t pak_size) {
    if (g_pak_system.is_initialized) {
        fprintf(stderr, "⚠️  Sistema PAK ya inicializado\n");
        return 0;  // Ya está inicializado
    }
    
    if (!pak_file) {
        fprintf(stderr, "❌ Error: pak_file es NULL\n");
        return -1;
    }
    
    printf("🔧 Inicializando sistema PAK...\n");
    printf("   Archivo: %s\n", pak_file);
    
    // Si no hay datos precargados, leer del archivo
    if (!pak_data) {
        FILE *f = fopen(pak_file, "rb");
        if (!f) {
            perror("❌ Error al abrir PAK");
            return -1;
        }
        
        // Obtener tamaño del archivo
        fseek(f, 0, SEEK_END);
        pak_size = (uint32_t)ftell(f);
        fseek(f, 0, SEEK_SET);
        
        // Leer en memoria
        pak_data = malloc(pak_size);
        if (fread(pak_data, 1, pak_size, f) != pak_size) {
            perror("❌ Error al leer PAK");
            free(pak_data);
            fclose(f);
            return -1;
        }
        
        fclose(f);
        printf("   Tamaño: %u bytes (%.2f MB)\n", pak_size, pak_size / (1024.0 * 1024.0));
    }
    
    // Inicializar resource manager
    g_pak_system.rm = rm_init(pak_data, pak_size);
    if (!g_pak_system.rm) {
        fprintf(stderr, "❌ Error al inicializar resource manager\n");
        free(pak_data);
        return -1;
    }
    
    g_pak_system.pak_filename = pak_file;
    g_pak_system.is_initialized = 1;
    
    printf("✅ Sistema PAK inicializado exitosamente\n");
    printf("   Recursos disponibles: %u\n\n", g_pak_system.rm->header.num_entries);
    
    return 0;
}

void pak_system_shutdown(void) {
    if (g_pak_system.rm) {
        rm_destroy(g_pak_system.rm);
        g_pak_system.rm = NULL;
    }
    g_pak_system.is_initialized = 0;
}

int pak_system_is_ready(void) {
    return g_pak_system.is_initialized && g_pak_system.rm != NULL;
}

// ============================================================================
// CARGA DE RECURSOS
// ============================================================================

void* pak_load_resource(const char *filename, uint32_t *out_size) {
    if (!pak_system_is_ready()) {
        fprintf(stderr, "❌ Sistema PAK no inicializado\n");
        if (out_size) *out_size = 0;
        return NULL;
    }
    
    void *data = rm_load(g_pak_system.rm, filename, out_size);
    if (!data) {
        fprintf(stderr, "⚠️  Recurso no encontrado: %s\n", filename);
        return NULL;
    }
    
    return data;
}

uint32_t pak_load_resource_into(const char *filename, void *buffer, uint32_t max_size) {
    if (!buffer || max_size == 0) {
        fprintf(stderr, "❌ Buffer inválido\n");
        return 0;
    }
    
    uint32_t resource_size;
    void *resource_data = pak_load_resource(filename, &resource_size);
    
    if (!resource_data) {
        return 0;
    }
    
    if (resource_size > max_size) {
        fprintf(stderr, "❌ Recurso %s es demasiado grande (%u > %u)\n",
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

int pak_list_resources(const char *pattern, SpaplexResourceEntry **entries, 
                       int max_entries) {
    if (!pak_system_is_ready()) {
        return 0;
    }
    
    int count = 0;
    for (uint32_t i = 0; i < g_pak_system.rm->header.num_entries && count < max_entries; i++) {
        SpaplexResourceEntry *entry = &g_pak_system.rm->directory[i];
        
        // Comparar con patrón glob
        if (fnmatch(pattern, entry->filename, 0) == 0) {
            entries[count++] = entry;
        }
    }
    
    return count;
}