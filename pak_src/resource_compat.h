#ifndef RESOURCE_COMPAT_H
#define RESOURCE_COMPAT_H

// Este módulo proporciona funciones compatibles con el código existente
// Reemplaza fopen/fread/fclose para recursos PAK

#include <stdio.h>
#include <stdint.h>

/**
 * Estructura compatible con FILE para PAK
 */
typedef struct {
    const void *data;      // Puntero a datos del PAK
    uint32_t position;     // Posición actual de lectura
    uint32_t size;         // Tamaño total
    int is_pak;            // Flag: es recurso PAK
} PAK_FILE;

/**
 * Abrir recurso (puede ser PAK o archivo tradicional)
 */
PAK_FILE* pak_fopen(const char *filename, const char *mode);

/**
 * Leer desde recurso
 */
size_t pak_fread(void *ptr, size_t size, size_t nmemb, PAK_FILE *stream);

/**
 * Buscar en recurso
 */
int pak_fseek(PAK_FILE *stream, long offset, int whence);

/**
 * Obtener posición
 */
long pak_ftell(PAK_FILE *stream);

/**
 * Cerrar recurso
 */
int pak_fclose(PAK_FILE *stream);

/**
 * MACROS para reemplazar en código existente
 * Descomentar para usar en lugar de fopen/fread/fclose
 */
/*
#define fopen(filename, mode)  pak_fopen(filename, mode)
#define fread(ptr, size, nmemb, stream)  pak_fread(ptr, size, nmemb, stream)
#define fseek(stream, offset, whence)  pak_fseek(stream, offset, whence)
#define ftell(stream)  pak_ftell(stream)
#define fclose(stream)  pak_fclose(stream)
*/

#endif