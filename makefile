# kiesconv GCC makefile
CC=gcc
LD=ld
EXEC_FILE=kiesconv.exe
LIB_PATH?=.
CFLAGS= -fwritable-strings -mno-stack-arg-probe -Os -Wall -pedantic -mwindows -I.
LDFLAGS= --strip-all --subsystem windows -L $(LIB_PATH) -l kernel32 -l user32 -l ole32 -l comctl32 -l comdlg32 -l shell32 -l gdi32 -nostdlib --exclude-libs msvcrt.a -e_WinMain@16

OBJ_EXT=.o
OBJS=kiesconv${OBJ_EXT} SysToolX${OBJ_EXT} crc32mod${OBJ_EXT} puffjump${OBJ_EXT} unpgzmod${OBJ_EXT} kiesdmod${OBJ_EXT} aes256b/aes256${OBJ_EXT}

all: $(EXEC_FILE)

$(EXEC_FILE): $(OBJS)
	${LD} ${OBJS} resource/kiesconv.res -o $@ ${LDFLAGS}


o=${OBJ_EXT}

wipe:
	@rm -rf *${OBJ_EXT}

clean:	wipe all
