#include <windows.h>
#include <commctrl.h>
#include "SysToolX.h"
#include "resource/kiesconv.h"
#include "kiesdmod.h"

TCHAR *OpenSaveDialogEx(HWND wnd, DWORD msk, TCHAR *name, int savedlg) {
TCHAR buf[1024], *result, *s;
DWORD i, l;
  result = NULL;
  l = 0;
  buf[0] = 0;
  // only 4 bits
  msk &= 0xF;
  // any files always present
  msk |= 1 << (IDS_MSK_ANY - 1);
  // add strings
  for (i = 0; msk; msk >>= 1) {
    i++;
    // bit is set?
    if (msk & 1) {
      s = LangLoadString(i);
      if (s) {
        lstrcpyn(&buf[l], s, 1024 - l);
        l += lstrlen(s);
        FreeMem(s);
      }
    }
    // buf string too short
    if (l >= 1024) { break; }
  }
  // replace '|' with nulls
  for (s = buf; *s; s++) {
    if (*s == TEXT('|')) {
       *s = 0;
    }
  }
  // get default extension
  for (s = buf; *s; s++);
  for (s++; *s; s++) {
    // no multiextension
    if (*s == ';') {
      s = NULL;
      break;
    }
  }
  if (s) {
    for (s--; *s; s--) {
      if (*s == TEXT('.')) {
        break;
      }
      if ((*s == TEXT('*')) || (*s == TEXT('?')) || (*s == TEXT(';'))) {
        s = NULL;
        break;
      }
    }
    if (s) { s++; }
  }
  result = OpenSaveDialog(wnd, buf, s, name, savedlg);
  return(result);
}

TCHAR *BaseName(TCHAR *name) {
TCHAR *s;
  // sanity check
  if (name) {
    for (s = name; *s; s++) {
      if ((*s == TEXT('\\')) || (*s == TEXT('/'))) {
        name = &s[1];
      }
    }
  }
  return(name);
}

void ChangeFileExt(TCHAR *name, TCHAR *ext) {
TCHAR *s;
  // sanity check
  if (name && ext) {
    // basename
    name = BaseName(name);
    // files with the dot in a start of a name
    for (; (*name == TEXT('.')); name++);
    for (s = NULL; *name; name++) {
      if (*name == TEXT('.')) {
        s = name;
      }
    }
    lstrcpy(s ? s : name, ext);
  }
}

BOOL CALLBACK DlgPrc(HWND wnd, UINT msg, WPARAM wparm, LPARAM lparm) {
BOOL result;
DWORD i;
TCHAR *s, *d, ext[4];
DRAWITEMSTRUCT *dis;
  result = FALSE;
  switch (msg) {
    case WM_INITDIALOG:
      // add icons
      SendMessage(wnd, WM_SETICON, ICON_BIG  , (LPARAM) LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICN)));
      SendMessage(wnd, WM_SETICON, ICON_SMALL, (LPARAM) LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICN)));
      // diable buttons
      DialogEnableWindow(wnd, IDC_SAVE, FALSE);
      DialogEnableWindow(wnd, IDOK, FALSE);
      // set focus (because _DEF_PUSHBUTTON
      // also must be Post not Send or didn't work at all
      // https://blogs.msdn.microsoft.com/oldnewthing/20040802-00/?p=38283
      //PostMessage(wnd, WM_NEXTDLGCTL, (WPARAM) GetDlgItem(wnd, IDC_OPEN), TRUE);
      // file from command line?
      // this message must be after focus selecting
      s = (TCHAR *) lparm;
      if (s) {
        SetDlgItemText(wnd, IDC_FSRC, s);
        // add message to window message queue - emulate "Open..." click
        // note that lparam (handle of control) must be null - used as flag
        PostMessage(wnd, WM_COMMAND, MAKELONG(IDC_OPEN, BN_CLICKED), 0);
      }
      // update controls
      result = TRUE;
      break;
    case WM_COMMAND:
      if (HIWORD(wparm) == BN_CLICKED) {
        result = TRUE;
        switch (LOWORD(wparm)) {
          case IDC_OPEN:
            // lparam are zero if got here from WM_INITDIALOG
            s = lparm ? OpenSaveDialogEx(wnd, 1|2|4, NULL, 0) : GetWndText(GetDlgItem(wnd, IDC_FSRC));
            // init edit control
            if (!lparm) { SetDlgItemText(wnd, IDC_FSRC, (TCHAR *) &lparm); }
            if (s) {
              if (IsKiesFile(s)) {
                // set dialog text
                SetDlgItemText(wnd, IDC_FSRC, s);
                // scroll to the end so the filename can be visible
                SendDlgItemMessage(wnd, IDC_FSRC, EM_SETSEL, 0, -1);
                // also generate and fill output filename
                // cur extension (if any)
                i = TEXT('.');
                ChangeFileExt(s, (TCHAR *)&i);
                SetDlgItemText(wnd, IDC_FDST, s);
                // select all (carret = end)
                SendDlgItemMessage(wnd, IDC_FDST, EM_SETSEL, 0, -1);
                // unselect all (carret stays intact)
                SendDlgItemMessage(wnd, IDC_FDST, EM_SETSEL, -1, 0);
                // add new extension
                ext[0] = TEXT('x');
                ext[1] = TEXT('m');
                ext[2] = TEXT('l');
                ext[3] = 0;
                SendDlgItemMessage(wnd, IDC_FDST, EM_REPLACESEL, (WPARAM) FALSE, (LPARAM) ext);
                // and scroll too
                SendDlgItemMessage(wnd, IDC_FDST, EM_SETSEL, 0, -1);
                // enable buttons
                DialogEnableWindow(wnd, IDC_SAVE, TRUE);
                DialogEnableWindow(wnd, IDOK, TRUE);
                // set focus to "Convert" button if goes from the commandline
                if (!lparm) {
                  PostMessage(wnd, WM_NEXTDLGCTL, (WPARAM) GetDlgItem(wnd, IDOK), TRUE);
                }
               } else {
                // show error message
                MsgBox(wnd, MAKEINTRESOURCE(IDS_ERR_BAD), MB_ICONERROR);
              }
              FreeMem(s);
            }
            break;
          case IDC_SAVE:
            d = GetWndText(GetDlgItem(wnd, IDC_FDST));
            s = OpenSaveDialogEx(wnd, 8, d, 1);
            if (d) { FreeMem(d); }
            if (s) {
              // set dialog text
              SetDlgItemText(wnd, IDC_FDST, s);
              // scroll to the end so the filename can be visible
              SendDlgItemMessage(wnd, IDC_FDST, EM_SETSEL, 0, -1);
              FreeMem(s);
            }
            break;
          case IDOK:
            s = GetWndText(GetDlgItem(wnd, IDC_FSRC));
            d = GetWndText(GetDlgItem(wnd, IDC_FDST));
            if (s && d && *s && *d) {
              i = DecodeKiesFile(s, d);
              MsgBox(wnd, MAKEINTRESOURCE(i + IDS_ERR_OCS), i ? MB_ICONERROR : MB_ICONINFORMATION);
            }
            if (d) { FreeMem(d); }
            if (s) { FreeMem(s); }
            break;
          // exit from program
          case IDCANCEL:
            EndDialog(wnd, 0);
            break;
          case IDC_SITE:
            // get control text
            s = GetWndText(GetDlgItem(wnd, LOWORD(wparm)));
            if (s) {
              // save original pointer
              d = s;
              // find link splitter
              for (; *s; s++) {
                // found it
                if (*s == TEXT('|')) {
                  break;
                }
              }
              // found?
              if (*s == TEXT('|')) {
                // remove space if any
                for (s++; *s == TEXT(' '); s++);
              }
              // open link
              if (*s) {
                URLOpenLink(wnd, s);
              }
              // free memory
              FreeMem(d);
            }
            break;
        }
      }
      break;
    // IDC_SITE
    case WM_DRAWITEM:
      dis = (DRAWITEMSTRUCT *) lparm;
      if (dis && (dis->CtlID == IDC_SITE)) {
        s = GetWndText(dis->hwndItem);
        if (s) {
          SetTextColor(dis->hDC, RGB(0, 0, 0xFF));
          SetBkMode(dis->hDC, TRANSPARENT);
          DrawText(dis->hDC, s, -1, &dis->rcItem, DT_LEFT | DT_TOP | DT_SINGLELINE);
          FreeMem(s);
          result = TRUE;
        }
      }
      break;
    case WM_SETCURSOR:
      if (((HWND) wparm) == GetDlgItem(wnd, IDC_SITE)) {
        SetCursor(LoadCursor(NULL, IDC_HAND));
        SetWindowLongPtr(wnd, DWLP_MSGRESULT, TRUE);
        result = TRUE;
      }
      break;
  }
  return(result);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
TCHAR **argv, *openfile;
DWORD argc;
  InitCommonControls();
  openfile = NULL;
  argv = GetCmdLineArgs(&argc);
  if (argv) {
    if (argc == 2) { openfile = GetFullFilePath(argv[1]); }
    FreeMem(argv);
  }
  DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DLG), 0, &DlgPrc, (LPARAM) openfile);
  if (openfile) { FreeMem(openfile); }
  ExitProcess(0);
  return(0);
}
