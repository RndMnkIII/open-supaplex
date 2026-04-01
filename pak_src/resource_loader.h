#ifndef RESOURCE_LOADER_H
#define RESOURCE_LOADER_H

#include <stdint.h>
#include "pak_format.h"

typedef struct {
    void *pak_data;                    // Puntero a memoria del PAK
    uint32_t pak_size;
    SpaplexResourceHeader header;
    SpaplexResourceEntry *directory;   // Tabla de directorios
} SpaplexResourceManager;

// Inicializar manager desde PAK en memoria
SpaplexResourceManager* rm_init(void *pak_ptr, uint32_t pak_size);

// Cargar recurso desde PAK
void* rm_load(SpaplexResourceManager *rm, const char *filename, uint32_t *out_size);

// Buscar entrada específica
SpaplexResourceEntry* rm_find_entry(SpaplexResourceManager *rm, const char *filename);

// Limpiar
void rm_destroy(SpaplexResourceManager *rm);

// Validar integridad
int rm_verify_crc(SpaplexResourceManager *rm, const char *filename);

#endif