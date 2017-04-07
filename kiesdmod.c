#include "kiesdmod.h"
#include "SysToolX.h"
#include "unpgzmod.h"
// replaced Cryptographic Service Provider from <wincrypt.h>
// with "aes256b.h" - now it works even under Windows 98
// because Microsoft Enhanced RSA and AES Cryptographic Provider
// available only on operating systems since Windows XP and newer
#include "aes256b/aes256.h"

#define CRYPT_BLOCK_LEN 0x110
#define CRYPT_BLOCK_RAW 0x100

// key
const static CCHAR s_key[] = "epovviwlx,dirwq;sor0-fvksz,erwog";
// initialize vector
const static CCHAR  s_iv[] = "afie,crywlxoetka";

// another code replace to drop msvcrt.dll dependency
void *memmove(void *dst, const void *src, size_t count) {
void *ret;
  ret = dst;
  if ((dst <= src) || ((char *)dst >= ((char *)src + count))) {
    while (count--) {
      *(char *)dst = *(char *)src;
      dst = (char *)dst + 1;
      src = (char *)src + 1;
    }
  } else {
    dst = (char *)dst + count - 1;
    src = (char *)src + count - 1;
    while (count--) {
      *(char *)dst = *(char *)src;
      dst = (char *)dst - 1;
      src = (char *)src - 1;
    }
  }
  return(ret);
}

BOOL IsKiesFile(TCHAR *filename) {
WIN32_FIND_DATA dFind;
HANDLE hFind;
  hFind = FindFirstFile(filename, &dFind);
  if (hFind != INVALID_HANDLE_VALUE) {
    FindClose(hFind);
  } else {
    dFind.nFileSizeLow = 0;
  }
  return(((dFind.nFileSizeLow > (CRYPT_BLOCK_LEN*2)) && ((dFind.nFileSizeLow % CRYPT_BLOCK_LEN) == 0)) ? TRUE : FALSE);
}

void AES256DecryptKeyIV(BYTE *p, DWORD sz, BYTE *key, BYTE *iv) {
aes256_context ctx;
BYTE x[32], i;
  if (p && sz && key) {
    if (iv) { CopyMemory(x, iv, 16); }
    aes256_init(&ctx, key);
    for (sz /= 16; sz; sz--) {
      if (iv) { CopyMemory(&x[16], p, 16); }
      aes256_decrypt_ecb(&ctx, p);
      if (iv) {
        for (i = 0; i < 16; i++) { p[i] ^= x[i]; }
        CopyMemory(x, &x[16], 16);
      }
      p += 16;
    }
    aes256_done(&ctx);
  }
}

BOOL AES256Decode(BYTE *buf, DWORD sz, BYTE *key, BYTE *iv) {
BOOL result;
  result = FALSE;
  // sanity check
  if (buf && sz && key && ((sz % CRYPT_BLOCK_LEN) == 0)) {
    for (sz /= CRYPT_BLOCK_LEN; sz; sz--) {
      AES256DecryptKeyIV(buf, CRYPT_BLOCK_LEN, key, iv);
      buf += CRYPT_BLOCK_LEN;
    }
    result = TRUE;
  }
  return(result);
}

DWORD DecodeKiesFile(TCHAR *cryptfile, TCHAR *plainfile) {
BYTE *p, *u;
DWORD i, sz, r;
HANDLE fl;
  r = 1; // cant open
  p = NULL;
  sz = 0;
  fl = CreateFile(cryptfile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (fl != INVALID_HANDLE_VALUE) {
    r = 2; // invalid format
    sz = GetFileSize(fl, NULL);
    if ((sz > CRYPT_BLOCK_LEN*2) && ((sz % CRYPT_BLOCK_LEN) == 0)) {
      r = 3; // not enough memory
      p = (BYTE *) GetMem(sz);
      if (p) {
        ReadFile(fl, p, sz, &i, NULL);
      }
    }
    CloseHandle(fl);
  }
  if (p) {
    r = 4; // decrypt failed
    if (AES256Decode(p, sz, (BYTE *) s_key, (BYTE *) s_iv)) {
      // remove padding
      sz /= CRYPT_BLOCK_LEN;
      for (i = 1; i < sz; i++) {
        MoveMemory(&p[i*CRYPT_BLOCK_RAW], &p[i*CRYPT_BLOCK_LEN], CRYPT_BLOCK_RAW);
      }
      r = 5; // unpacking failed
      sz -= 2;
      u = GZUnpack(&p[CRYPT_BLOCK_RAW*2], sz * CRYPT_BLOCK_RAW);
      if (u) {
        r = 6; // cant create
        fl = CreateFile(plainfile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (fl != INVALID_HANDLE_VALUE) {
          WriteFile(fl, &u[4], *((DWORD *) u), &i, NULL);
          CloseHandle(fl);
          r = 0; // let's hope it was alright
        }
        FreeMem(u);
      }
    }
    FreeMem(p);
  }
  return(r);
}
