#ifndef __SYSTOOLX_H
#define __SYSTOOLX_H

#include <windows.h>

#define STR_ALLOC(x) ((TCHAR *) GetMem((x + 1) * sizeof(TCHAR)))

void FreeMem(void *block);
void *GetMem(DWORD dwSize);

TCHAR *LangLoadString(UINT sid);

TCHAR **GetCmdLineArgs(DWORD *argc);

TCHAR *GetFullFilePath(TCHAR *filename);
TCHAR *GetWndText(HWND wnd);

TCHAR *OpenSaveDialog(HWND wnd, TCHAR *filemask, TCHAR *defext, TCHAR *defname, int savedlg);

void URLOpenLink(HWND wnd, TCHAR *s);

int MsgBox(HWND wnd, TCHAR *lpText, UINT uType);
void DialogEnableWindow(HWND hdlg, int idControl, BOOL bEnable);

#endif
