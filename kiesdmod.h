#ifndef __KIESDMOD_H
#define __KIESDMOD_H

#include <windows.h>

BOOL IsKiesFile(TCHAR *filename);
DWORD DecodeKiesFile(TCHAR *cryptfile, TCHAR *plainfile);

#endif
