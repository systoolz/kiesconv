#include "unpgzmod.h"
#include "crc32mod.h"
#include "SysToolX.h"
#include "zlibpuff/puff.h"

BYTE *GZUnpack(BYTE *p, DWORD sz) {
gzf_head *gh;
gzf_tail *gt;
DWORD us, ps;
BYTE *u;
int c;
  u = NULL;
  // minimal gz file headers size
  sz -= min(sz, (sizeof(gh[0]) + sizeof(gt[0])));
  gh = (gzf_head *) p;
  // sanity check
  if (p && sz && (gh->magic == GZ_MAGIC) && (gh->ctype == GZ_DEFLATE)) {
    // TODO: maybe add full support for gh->flags and gh->flagx (skip additional fields) ?
    p += sizeof(gh[0]);
    us = 0;
    ps = sz;
    c = puff(NULL, &us, p, &ps);
    gt = (gzf_tail *) &p[ps];
    // sanity check
    if ((c >= 0) && (ps <= sz) && (us > 0) && (gt->usize == us)) {
      u = (BYTE *) GetMem(us + sizeof(us));
      if (u) {
        *((DWORD *) u) = us;
        puff(&u[sizeof(us)], &us, p, &ps);
        // sanity check
        if (CRC32Buffer(&u[sizeof(us)], us) != gt->crc32) {
          FreeMem(u);
          u = NULL;
        }
      }
    }
  }
  return(u);
}
