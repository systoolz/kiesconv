#include <setjmp.h>

/*
  simply _setjmp/longjmp implementation
  since there are no puff() like decompressors which can return output buffer size
  forced to use this one, but this rely on _setjmp/longjmp and it's a msvcrt() dependency
  all this needed because gzip format output buffer size located at the end of the input
  packed buffer, but there are no packed buffer size, in other words - if you have something
  (like padding) at the end of gzip file you don't have any other options except to unpack
  the whole stream to get the output buffer size and pray that it wasn't corrupted in any way
*/

const static unsigned char x_setjmp[] = {
  // mov   edx, [esp+04h]
  0x8B, 0x54, 0x24, 0x04,
  // mov   [edx], ebp
  0x89, 0x2A,
  // mov   [edx+04h], ebx
  0x89, 0x5A, 0x04,
  // mov   [edx+08h], edi
  0x89, 0x7A, 0x08,
  // mov   [edx+0Ch], esi
  0x89, 0x72, 0x0C,
  // mov   [edx+10h], esp
  0x89, 0x62, 0x10,
  // mov   eax, [esp]
  0x8B, 0x04, 0x24,
  // mov   [edx+14h], eax
  0x89, 0x42, 0x14,
  // xor   eax, eax
  0x31, 0xC0,
  // retn
  0xC3
};

const static unsigned char xlongjmp[] = {
  // mov   eax, [esp+08h]
  0x8B, 0x44, 0x24, 0x08,
  // cmp   eax, 1
  0x83, 0xF8, 0x01,
  // adc   eax, 0
  0x83, 0xD0, 0x00,
  // mov   edx, [esp+04h]
  0x8B, 0x54, 0x24, 0x04,
  // mov   ebp, [edx]
  0x8B, 0x2A,
  // mov   ebx, [edx+04h]
  0x8B, 0x5A, 0x04,
  // mov   edi, [edx+08h]
  0x8B, 0x7A, 0x08,
  // mov   esi, [edx+0Ch]
  0x8B, 0x72, 0x0C,
  // mov   esp, [esp+10h]
  0x8B, 0x64, 0x24, 0x10,
  // jmp   dword [edx+14h]
  0xFF, 0x62, 0x14
};

#undef setjmp
typedef int (*LP_SETJMP)(jmp_buf jmpb);
#define setjmp ((LP_SETJMP) x_setjmp)

#undef longjmp
typedef void (*LPLONGJMP)(jmp_buf jmpb, int retval);
#define longjmp ((LPLONGJMP) xlongjmp)

#include "zlibpuff/puff.c"
