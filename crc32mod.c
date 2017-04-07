#include "crc32mod.h"

// http://create.stephan-brumme.com/crc32/#tableless

static DWORD ctab[256];

void CRC32BuildTable(void) {
DWORD i, j;
  for (i = 0; i < 256; i++) {
    ctab[i] = i;
    for (j = 0; j < 8; j++) {
      ctab[i] = (ctab[i] >> 1) ^ (0xEDB88320 * (ctab[i] & 1));
    }
  }
}

DWORD CRC32Update(BYTE *p, DWORD sz, DWORD crc) {
  // crc32 not initialized
  if (ctab[128] ^ 0xEDB88320) { CRC32BuildTable(); }
  crc = ~crc;
  while (sz--) {
    crc = (crc >> 8) ^ ctab[LOBYTE(crc) ^ *p++];
  }
  return(~crc);
}
