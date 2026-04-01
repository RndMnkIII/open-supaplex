// ============================================================================
// CAMBIOS EN main() de supaplex.c
// ============================================================================

// AÑADIR AL INICIO:
#include "supaplex_pak.h"
#include "resource_compat.h"

// EN LA FUNCIÓN main(), REEMPLAZAR INICIALIZACIÓN DE RECURSOS:

// ANTES (carga tradicional):
/*
int main(int argc, char *argv[]) {
    // ... código anterior ...
    
    // Cargar recursos individuales
    FILE *file = fopen("resources/GFX.DAT", "rb");
    fread(gfx_buffer, 1, 32001, file);
    fclose(file);
    
    file = fopen("resources/MENU.DAT", "rb");
    fread(menu_buffer, 1, 32000, file);
    fclose(file);
    
    // ... etc
}
*/

// DESPUÉS (con PAK):
int main(int argc, char *argv[]) {
    // Parsear argumentos
    parseCommandLineOptions(argc, argv);
    initializeLogging();
    initializeSystem();
    initializeVideo();
    initializeControllers();
    initializeAudio();
    
    // ✅ NUEVO: Inicializar sistema PAK
    printf("\n🔧 Cargando recursos...\n");
    if (pak_system_init("SUPAPLEX.pak", NULL, 0) != 0) {
        fprintf(stderr, "❌ Error al cargar PAK. Intentando modo tradicional...\n");
        // Fallback: intentar cargar recursos individuales del directorio
        // (mantener para compatibilidad)
    }
    printf("\n");
    
    // Ahora usar pak_load_resource_into() en lugar de fopen/fread/fclose
    
    // Ejemplo 1: Cargar GFX.DAT
    uint32_t gfx_size = pak_load_resource_into("GFX.DAT", gfx_buffer, 32001);
    if (gfx_size != 32001) {
        fprintf(stderr, "❌ Error cargando GFX.DAT\n");
        return -1;
    }
    
    // Ejemplo 2: Cargar MENU.DAT
    uint32_t menu_size = pak_load_resource_into("MENU.DAT", menu_buffer, 32000);
    if (menu_size != 32000) {
        fprintf(stderr, "❌ Error cargando MENU.DAT\n");
        return -1;
    }
    
    // Ejemplo 3: Cargar archivos de niveles con ruta
    uint32_t level_size = pak_load_resource_into("levels/LEVEL.L01", level_buffer, MAX_LEVEL_SIZE);
    if (level_size == 0) {
        fprintf(stderr, "❌ Error cargando nivel\n");
        return -1;
    }
    
    // Ejemplo 4: Cargar audio (si existe)
    void *music_data = pak_load_resource("audio/music.ogg", &audio_size);
    if (music_data) {
        play_music(music_data, audio_size);
    }
    
    // Resto del inicialización
    initializeGameStateData();
    
    // Main loop
    int game_running = 1;
    while (game_running) {
        readLevels();
        initializeGameInfo();
        runLevel();
        slideDownGameDash();
    }
    
    // Limpieza
    destroyVideo();
    destroyAudio();
    
    // ✅ NUEVO: Limpiar sistema PAK
    pak_system_shutdown();
    
    return 0;
}

// ============================================================================
// FUNCIÓN ALTERNATIVA: CARGA COMPATIBLE CON CÓDIGO EXISTENTE
// ============================================================================

/**
 * Esta función permite mantener compatibilidad con código existente
 * sin cambiar llamadas a fopen/fread/fclose
 */
void enable_pak_compatibility_mode(void) {
    // Descomentar las macros en resource_compat.h para reemplazar
    // globalmente fopen/fread/fclose
    
    // Ventaja: casi ningún cambio en código existente
    // Desventaja: puede afectar otras partes del código
}

// ============================================================================
// FUNCIÓN AUXILIAR: CARGAR TODOS LOS RECURSOS
// ============================================================================

int load_all_game_resources(void) {
    printf("📦 Cargando todos los recursos...\n\n");
    
    // Estructura de datos para auditar carga
    typedef struct {
        const char *filename;
        void *buffer;
        uint32_t max_size;
        uint32_t *actual_size;
        const char *description;
    } ResourceToLoad;
    
    // Define todos los recursos necesarios
    uint32_t gfx_size, menu_size, title_size, back_size, panel_size;
    uint32_t moving_size, fixed_size, palettes_size, controls_size;
    
    ResourceToLoad resources[] = {
        // Gráficos
        {"GFX.DAT", gfx_buffer, 32001, &gfx_size, "Gráficos principales"},
        {"MENU.DAT", menu_buffer, 32000, &menu_size, "Menú"},
        {"TITLE.DAT", title_buffer, 32001, &title_size, "Pantalla título"},
        {"BACK.DAT", back_buffer, 32000, &back_size, "Fondo"},
        {"PANEL.DAT", panel_buffer, 3840, &panel_size, "Panel HUD"},
        {"MOVING.DAT", moving_buffer, 73920, &moving_size, "Sprites animados"},
        {"FIXED.DAT", fixed_buffer, 5120, &fixed_size, "Elementos fijos"},
        {"PALETTES.DAT", palettes_buffer, 256, &palettes_size, "Paletas"},
        {"CONTROLS.DAT", controls_buffer, 32001, &controls_size, "Pantalla controles"},
    };
    
    int num_resources = sizeof(resources) / sizeof(ResourceToLoad);
    int loaded = 0;
    int failed = 0;
    
    for (int i = 0; i < num_resources; i++) {
        ResourceToLoad *res = &resources[i];
        uint32_t size = pak_load_resource_into(res->filename, res->buffer, res->max_size);
        
        if (size > 0) {
            *res->actual_size = size;
            printf("✓ %s (%u bytes) - %s\n", res->filename, size, res->description);
            loaded++;
        } else {
            printf("✗ %s - FALLO - %s\n", res->filename, res->description);
            failed++;
        }
    }
    
    printf("\n📊 Resumen:\n");
    printf("   Recursos cargados: %d/%d\n", loaded, num_resources);
    
    if (failed > 0) {
        printf("   ⚠️  Fallos: %d\n", failed);
        return -1;
    }
    
    printf("   ✅ Todos los recursos cargados correctamente\n\n");
    return 0;
}

// ============================================================================
// VERSIÓN ALTERNATIVA: USO CON PAK_FILE (sin necesidad de cambiar código)
// ============================================================================

/**
 * Ejemplo de cómo usar pak_fopen/pak_fread/pak_fclose
 * si prefieres mantener la API FILE* existente
 */
int load_resources_compat_mode(void) {
    // Cargar GFX.DAT usando interfaz compatible
    PAK_FILE *gfx_file = pak_fopen("GFX.DAT", "rb");
    if (!gfx_file) {
        fprintf(stderr, "Error al abrir GFX.DAT\n");
        return -1;
    }
    
    pak_fread(gfx_buffer, 1, 32001, gfx_file);
    pak_fclose(gfx_file);
    
    // El resto del código NO necesita cambios
    printf("✓ GFX.DAT cargado exitosamente\n");
    
    return 0;
}