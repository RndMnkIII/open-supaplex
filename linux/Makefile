src = $(wildcard ../src/*.c) $(wildcard ../src/sdl2/*.c) $(wildcard ../src/sdl_common/*.c) ../src/lib/ini/ini.c ../src/null/virtualKeyboard.c
obj = $(src:.c=.o)

LDFLAGS += -lSDL2_mixer `sdl2-config --libs` -lm

CFLAGS += `sdl2-config --cflags` -DHAVE_SDL2

# PAK resource system: enable by default on this platform.
# To disable (use individual resource files instead), pass: make USE_PAK_RESOURCES=0
ifneq ($(USE_PAK_RESOURCES),0)
CFLAGS += -DUSE_PAK_RESOURCES -DHAVE_FMEMOPEN -DHAVE_FNMATCH
endif

PAK_TOOL = pak_tool
PAK_FILE = SUPAPLEX.pak
RESOURCES_DIR = ../resources

opensupaplex: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)

$(PAK_TOOL): ../pak_src/pak_tool_v2.c ../src/crc32.c
	$(CC) $(CFLAGS) -I../src -o $@ $^

pak: $(PAK_TOOL)
	./$(PAK_TOOL) create $(RESOURCES_DIR) $(PAK_FILE)

clean:
	rm -f $(obj) opensupaplex $(PAK_TOOL) $(PAK_FILE)
