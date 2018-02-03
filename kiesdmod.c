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
// v1.2
const static CCHAR s_ver[] = "<?xml\0<HeaderData\0<version\0";
const static CCHAR s_typ[] = "<?xml\0<HeaderData\0<zipType\0";
const static CCHAR s_gzp[] = "GZip";

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

// v1.2
DWORD CmpXMLNode(CCHAR *s, DWORD z, CCHAR *d, DWORD x) {
DWORD r;
CCHAR a, b;
  r = 0;
  if (s && d && z && x) {
    r = x;
    while (x < z) {
      a = *s;
      b = *d;
      a |= ((a >= 'A') && (a <= 'Z')) ? 0x20 : 0;
      b |= ((b >= 'A') && (b <= 'Z')) ? 0x20 : 0;
      if (a != b) { break; }
      s++;
      z--;
      d++;
      x--;
    }
    if ((!x) && (z) && ((*s == '>') || (*s == ' ') || (*s == '\t') || (*s == '\n') || (*s == '\r'))) {
      while ((z) && (*s != '>')) {
        s++;
        z--;
        r++;
      }
      if ((z) && (*s == '>')) { r++; }
    } else {
      r = 0;
    }
  }
  return(r);
}

// v1.2
void GetXMLValue(CCHAR *s, DWORD z, CCHAR *f, CCHAR *v, DWORD l) {
DWORD r, x;
  if (v && l) {
    *v = 0;
    if (s && z && f) {
      x = lstrlenA(f);
      while (z > x) {
        r = CmpXMLNode(s, z, f, x);
        if (r) {
          f += x + 1;
          s += r;
          z -= r;
          if (*f) {
            GetXMLValue(s, z, f, v, l);
          } else {
            l = (z < l) ? z : l;
            if (l) {
              CopyMemory(v, s, l * sizeof(v[0]));
              v[l - 1] = 0;
              for (l = 0; v[l]; l++) {
                if (v[l] == '<') {
                  v[l] = 0;
                  break;
                }
              }
            }
          }
          break;
        }
        s++;
        z--;
      }
    }
  }
}

BOOL DumpToFile(TCHAR *filename, void *p, DWORD sz) {
HANDLE fl;
DWORD dw;
BOOL r;
  r = FALSE;
  if (filename) {
    fl = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (fl != INVALID_HANDLE_VALUE) {
      if (p && sz) {
        WriteFile(fl, p, sz, &dw, NULL);
      }
      CloseHandle(fl);
      r = TRUE;
    }
  }
  return(r);
}

DWORD DecodeKiesFile(TCHAR *cryptfile, TCHAR *plainfile) {
BYTE *p, *u;
CCHAR v[8];
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
      // v1.2
      // check XML version
      GetXMLValue((CCHAR*) p, CRYPT_BLOCK_RAW, (CCHAR *) s_ver, v, 8);
      // .SPB 1.x: "1.x"; .SPB 2.x: "2.x"; .SSC 2.x: "Version:2.0"
      if (v[0]) {
        // calculate version and header size
        i = (v[0] == '1') ? 1 : 2;
        // blocks
        sz -= i;
        // check if unpacking required
        GetXMLValue((CCHAR*) p, CRYPT_BLOCK_RAW*i, (CCHAR *) s_typ, v, 8);
        if (!lstrcmpiA(v, (CCHAR *) s_gzp)) {
          r = 5; // unpacking failed
          u = GZUnpack(&p[CRYPT_BLOCK_RAW*i], sz * CRYPT_BLOCK_RAW);
          if (u) {
            // cant create
            r = DumpToFile(plainfile, &u[4], *((DWORD *) u)) ? 0 : 6;
            FreeMem(u);
          }
        } else {
          // probably plain .XML file
          r = DumpToFile(plainfile, &p[CRYPT_BLOCK_RAW*i], sz * CRYPT_BLOCK_RAW) ? 0 : 6;
        }
      }
    }
    FreeMem(p);
  }
  return(r);
}
