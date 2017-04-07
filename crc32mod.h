#ifndef __CRC32MOD_H
#define __CRC32MOD_H

#include <windows.h>

DWORD CRC32Update(BYTE *p, DWORD sz, DWORD crc);
#define CRC32Buffer(p,s) CRC32Update((p),(s),0)

#endif
