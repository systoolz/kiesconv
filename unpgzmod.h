#ifndef __UNPGZMOD_H
#define __UNPGZMOD_H

#include <windows.h>

#define GZ_MAGIC 0x8B1F
#define GZ_DEFLATE 8

#pragma pack(push, 1)
typedef struct {
  WORD  magic; // 0x8B1F
  BYTE  ctype; // compression type: 8 - deflate
  BYTE  flags; // various flags
  DWORD mtime; // modification unix filetimestamp
  BYTE  flagx; // extra flag 4 - compressor used fastest algorithm
  BYTE  stype; // system type: 0 - FAT filesystem (MS-DOS, OS/2, NT/Win32)
} gzf_head;
// < ...packed data follows... >
typedef struct {
  DWORD crc32; // CRC32 of whole unpacked data
  DWORD usize; // unpacked data size
} gzf_tail;
#pragma pack(pop)

BYTE *GZUnpack(BYTE *p, DWORD sz);

#endif
