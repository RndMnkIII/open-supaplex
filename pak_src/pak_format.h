#ifndef PAK_FORMAT_H
#define PAK_FORMAT_H

#include <stdint.h>

#define PAK_MAGIC           0x4B415053  // "SPAK" little-endian
#define PAK_VERSION         2           // Versión 2 con soporte de rutas
#define PAK_MAX_ENTRIES     512         // Aumentado para subdirectorios
#define PAK_FILENAME_SIZE   128         // Aumentado para rutas completas

#pragma pack(1)

typedef struct {
    uint32_t magic;           // "SPAK" (0x4B415053)
    uint32_t version;         // 2 (con rutas)
    uint32_t num_entries;     // Número de archivos
    uint32_t dir_offset;      // Offset a la tabla de directorio
    uint32_t total_size;      // Tamaño total del PAK
    uint32_t reserved[3];     // Espacio futuro
} SpaplexResourceHeader;

typedef struct {
    char filename[PAK_FILENAME_SIZE];  // Ahora incluye ruta relativa: "audio/music.ogg"
    uint32_t offset;
    uint32_t size;
    uint32_t compressed_size;
    uint32_t crc32;
    uint8_t  compression_type;
    uint8_t  reserved[3];
} SpaplexResourceEntry;

#pragma pack()

#define PAK_HEADER_SIZE     sizeof(SpaplexResourceHeader)
#define PAK_ENTRY_SIZE      sizeof(SpaplexResourceEntry)

#endif