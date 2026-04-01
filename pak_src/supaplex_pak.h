#ifndef SUPAPLEX_PAK_H
#define SUPAPLEX_PAK_H

#include <stdint.h>
#include "pak_format.h"
#include "resource_loader.h"

// ============================================================================
// GESTOR GLOBAL DE RECURSOS PAK
// ============================================================================

typedef struct {
    SpaplexResourceManager *rm;
    int is_initialized;
    const char *pak_filename;
} PakResourceSystem;

// Variable global
extern PakResourceSystem g_pak_system;

// ============================================================================
// FUNCIONES DE INICIALIZACIÓN
// ============================================================================

/**
 * Inicializar sistema de recursos PAK
 * @param pak_file Ruta al archivo PAK
 * @param pak_data Puntero a datos del PAK en memoria (si está precargado)
 * @param pak_size Tamaño del archivo PAK
 * @return 0 si exitoso, -1 si error
 */
int pak_system_init(const char *pak_file, void *pak_data, uint32_t pak_size);

/**
 * Limpiar sistema de recursos
 */
void pak_system_shutdown(void);

/**
 * Verificar si el sistema está inicializado
 */
int pak_system_is_ready(void);

// ============================================================================
// FUNCIONES DE CARGA DE RECURSOS (Reemplazan fopen/fread)
// ============================================================================

/**
 * Cargar recurso desde PAK
 * @param filename Nombre del recurso (ej: "GFX.DAT" o "audio/music.ogg")
 * @param out_size Puntero para recibir el tamaño
 * @return Puntero a datos, NULL si no existe
 */
void* pak_load_resource(const char *filename, uint32_t *out_size);

/**
 * Cargar recurso y copiar a buffer (para compatibilidad con código existente)
 * @param filename Nombre del recurso
 * @param buffer Buffer destino
 * @param max_size Tamaño máximo del buffer
 * @return Bytes copiados, 0 si error
 */
uint32_t pak_load_resource_into(const char *filename, void *buffer, uint32_t max_size);

/**
 * Buscar recurso sin cargarlo
 * @param filename Nombre del recurso
 * @return Puntero a entry, NULL si no existe
 */
SpaplexResourceEntry* pak_find_resource(const char *filename);

/**
 * Verificar integridad de recurso
 * @param filename Nombre del recurso
 * @return 0=OK, 1=CRC error, -1=no encontrado
 */
int pak_verify_resource(const char *filename);

/**
 * Listar todos los recursos que matcheen un patrón
 * @param pattern Patrón glob (ej: "audio/*", "level*.dat")
 * @param entries Array para recibir entradas
 * @param max_entries Máximo de entradas
 * @return Número de recursos encontrados
 */
int pak_list_resources(const char *pattern, SpaplexResourceEntry **entries, 
                       int max_entries);

#endif