┌─────────────────────────────────────────────────────────────┐
│                    SUPAPLEX STARTUP                          │
└─────────────────────────────────────────────────────────────┘
                            ↓
        ┌───────────────────────────────────────┐
        │  pak_system_init("SUPAPLEX.pak")      │
        └───────────────┬───────────────────────┘
                        ↓
        ┌───────────────────────────────────────┐
        │  Cargar PAK en memoria                │
        │  Mapear directorio de recursos        │
        │  Verificar integridad (magic + CRC)   │
        └───────────────┬───────────────────────┘
                        ↓
        ┌───────────────────────────────────────┐
        │  load_all_game_resources()            │
        ├───────────────────────────────────────┤
        │  pak_load_resource("GFX.DAT")         │
        │  pak_load_resource("MENU.DAT")        │
        │  pak_load_resource("levels/L01.DAT")  │
        │  pak_load_resource("audio/music.ogg") │
        └───────────────┬───────────────────────┘
                        ↓
        ┌───────────────────────────────────────┐
        │  initializeGameStateData()            │
        │  Main Game Loop                       │
        └───────────────┬───────────────────────┘
                        ↓
        ┌───────────────────────────────────────┐
        │  pak_system_shutdown()                │
        │  Limpiar memoria                      │
        └───────────────────────────────────────┘


# 1. Copiar archivos
cp pak_format.h supaplex/
cp pak_tool_v2.c supaplex/
cp crc32.c crc32.h supaplex/
cp supaplex_pak.c supaplex_pak.h supaplex/
cp resource_compat.c resource_compat.h supaplex/
cp resource_loader.c resource_loader.h supaplex/
cp Makefile_with_pak supaplex/Makefile

# 2. Ejecutar setup
cd supaplex
bash setup_pak.sh

# 3. Jugar
./supaplex