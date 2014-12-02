/*--------------------------------------------------------------------------------------+
|
|     $Source: Tools/ToolSubs/nonport/winnt/setseed/setseed.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#define STRICT
#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <CommDlg.h>
#include <dlgs.h>

#pragma comment (lib, "shell32")
#pragma comment (lib, "user32")
#pragma comment (lib, "gdi32")
#pragma comment (lib, "comdlg32")
#pragma comment (lib, "shlwapi")

#include <setsids.h>
//
//In NewMicroStationDGN.reg
//   REGEDIT4
//
//   [HKEY_CLASSES_ROOT\.dgn\ShellNew]
//   "Command"="C:\\Test\\CopyTime\\CreateDgn.exe %2"
///////////////////////////////
////
//cl C:\Test\CopyTime\CreateDgn.cpp -Z7 -Yd -Od -link -pdb:none -debug

#define DIM(a)      (  (sizeof(a)) / sizeof *(a)  )
#define ___         (0)
#define Public

#define     debug   0
#include <miregistry.h>


// needed because PRG has old header files
typedef struct tagOFN
    {
    DWORD         lStructSize;
    HWND          hwndOwner;
    HINSTANCE     hInstance;
    LPCTSTR       lpstrFilter;
    LPTSTR        lpstrCustomFilter;
    DWORD         nMaxCustFilter;
    DWORD         nFilterIndex;
    LPTSTR        lpstrFile;
    DWORD         nMaxFile;
    LPTSTR        lpstrFileTitle;
    DWORD         nMaxFileTitle;
    LPCTSTR       lpstrInitialDir;
    LPCTSTR       lpstrTitle;
    DWORD         Flags;
    WORD          nFileOffset;
    WORD          nFileExtension;
    LPCTSTR       lpstrDefExt;
    LPARAM        lCustData;
    LPOFNHOOKPROC lpfnHook;
    LPCTSTR       lpTemplateName;
    void *        pvReserved;
    DWORD         dwReserved;
    DWORD         FlagsEx;
    } OPENFILENAMEx, *LPOPENFILENAMEx;

#if ! defined (OFN_DONTADDTORECENT)
enum
  {
  OFN_DONTADDTORECENT = 0x2000000
  };
#endif

static char const cmdlineShowCreateDgnDialog[]     = "-ShowCreateDgnDialog";   // Command line switch

static char       szFilter[MAX_PATH*2] = {"MicroStation DGN Files (*.dgn)|*.dgn|All Files (*.*)|*.*||"};
static char       szUseShellNewFile[MAX_PATH*2] = {"Use the seed file selected from this dialog (currently %s)"};

static HINSTANCE   g_hInstance;                // the current instance
static BOOL        bIsW2KExplorer;             // Use resizable File Open Dialog?
static HWND        g_hDlg;                     // Our part of the file open dialog template.
static WNDPROC     g_lpfnSetSeedOKBtnWndProc;  // Original window prog for the OK button

typedef struct persistantdata
    {
    int     bShowDialog;
    int     bLaunchExplorer;
    int     bUseSeedFilenameFromMicroStation;
    DWORD   dwListBoxStyle;
    char    szSeedFilename[MAX_PATH];
    }  PersistantData;

static PersistantData  persistantData;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if  debug
static char *msgToStr
(
UINT msg
)
   {
    switch (msg)
        {
        case BM_GETCHECK:           return  "BM_GETCHECK (F0)";
        case BM_SETSTATE:           return  "BM_SETSTATE (F3)";
        case BM_SETSTYLE:           return  "BM_SETSTYLE (F4)";
        case WM_NULL:        return "WM_NULL";
        case WM_CREATE:      return "WM_CREATE";
        case WM_DESTROY:     return "WM_DESTROY";
        case WM_MOVE:            return "WM_MOVE";
        case WM_SIZE:                return "WM_SIZE";
        case WM_ACTIVATE:    return "WM_ACTIVATE";
        case WM_SETFOCUS:    return "WM_SETFOCUS";
        case WM_KILLFOCUS:        return "WM_KILLFOCUS";
        case WM_ENABLE:        return "WM_ENABLE";
        case WM_SETREDRAW:        return "WM_SETREDRAW";
        case WM_SETTEXT:        return "WM_SETTEXT";
        case WM_GETTEXT:        return "WM_GETTEXT";
        case WM_GETTEXTLENGTH:      return "WM_GETTEXTLENGTH";
        case WM_PAINT:        return "WM_PAINT";
        case WM_CLOSE:        return "WM_CLOSE";
        case WM_QUERYENDSESSION:        return "WM_QUERYENDSESSION";
        case WM_QUIT:        return "WM_QUIT";
        case WM_QUERYOPEN:        return "WM_QUERYOPEN";
        case WM_ERASEBKGND:        return "WM_ERASEBKGND";
        case WM_SYSCOLORCHANGE:        return "WM_SYSCOLORCHANGE";
        case WM_ENDSESSION:        return "WM_ENDSESSION";
        //     case WM_SYSTEMERROR:           return "WM_SYSTEMERROR";
        case WM_SHOWWINDOW:        return "WM_SHOWWINDOW";
        //     case WM_CTLCOLOR:        return "WM_CTLCOLOR";
        case WM_SETTINGCHANGE:        return "WM_SETTINGCHANGE";
        case WM_DEVMODECHANGE:        return "WM_DEVMODECHANGE";
        case WM_ACTIVATEAPP:        return "WM_ACTIVATEAPP";
        case WM_FONTCHANGE:        return "WM_FONTCHANGE";
        case WM_TIMECHANGE:        return "WM_TIMECHANGE";
        case WM_CANCELMODE:        return "WM_CANCELMODE";
        case WM_SETCURSOR:        return "WM_SETCURSOR";
        case WM_MOUSEACTIVATE:        return "WM_MOUSEACTIVATE";
        case WM_CHILDACTIVATE:        return "WM_CHILDACTIVATE";
        case WM_QUEUESYNC:        return "WM_QUEUESYNC";
        case WM_GETMINMAXINFO:        return "WM_GETMINMAXINFO";
        case WM_PAINTICON:        return "WM_PAINTICON";
        case WM_ICONERASEBKGND:        return "WM_ICONERASEBKGND";
        case WM_NEXTDLGCTL:        return "WM_NEXTDLGCTL";
        case WM_SPOOLERSTATUS:        return "WM_SPOOLERSTATUS";
        case WM_DRAWITEM:        return "WM_DRAWITEM";
        case WM_MEASUREITEM:        return "WM_MEASUREITEM";
        case WM_DELETEITEM:        return "WM_DELETEITEM";
        case WM_VKEYTOITEM:        return "WM_VKEYTOITEM";
        case WM_CHARTOITEM:        return "WM_CHARTOITEM";
        case WM_SETFONT:        return "WM_SETFONT";
        case WM_GETFONT:        return "WM_GETFONT";
        case WM_SETHOTKEY:        return "WM_SETHOTKEY";
        case WM_GETHOTKEY:        return "WM_GETHOTKEY";
        case WM_QUERYDRAGICON:        return "WM_QUERYDRAGICON";
        case WM_COMPAREITEM:        return "WM_COMPAREITEM";
        //     case WM_GETOBJECT:        return "WM_GETOBJECT";
        case WM_COMPACTING:        return "WM_COMPACTING";
        case WM_COMMNOTIFY:        return "WM_COMMNOTIFY";
        case WM_WINDOWPOSCHANGING:        return "WM_WINDOWPOSCHANGING";
        case WM_WINDOWPOSCHANGED:        return "WM_WINDOWPOSCHANGED";
        case WM_POWER:        return "WM_POWER";
        case WM_COPYDATA:        return "WM_COPYDATA";
        case WM_CANCELJOURNAL:        return "WM_CANCELJOURNAL";
        case WM_NOTIFY:        return "WM_NOTIFY";
        case WM_INPUTLANGCHANGEREQUEST:        return "WM_INPUTLANGCHANGEREQUEST";
        case WM_INPUTLANGCHANGE:        return "WM_INPUTLANGCHANGE";
        case WM_TCARD:        return "WM_TCARD";
        case WM_HELP:        return "WM_HELP";
        case WM_USERCHANGED:        return "WM_USERCHANGED";
        case WM_NOTIFYFORMAT:        return "WM_NOTIFYFORMAT";
        case WM_CONTEXTMENU:        return "WM_CONTEXTMENU";
        case WM_STYLECHANGING:        return "WM_STYLECHANGING";
        case WM_STYLECHANGED:        return "WM_STYLECHANGED";
        case WM_DISPLAYCHANGE:        return "WM_DISPLAYCHANGE";
        case WM_GETICON:        return "WM_GETICON";
        case WM_SETICON:        return "WM_SETICON";
        case WM_NCCREATE:        return "WM_NCCREATE";
        case WM_NCDESTROY:        return "WM_NCDESTROY";
        case WM_NCCALCSIZE:        return "WM_NCCALCSIZE";
        case WM_NCHITTEST:        return "WM_NCHITTEST";
        case WM_NCPAINT:        return "WM_NCPAINT";
        case WM_NCACTIVATE:        return "WM_NCACTIVATE";
        case WM_GETDLGCODE:        return "WM_GETDLGCODE";
        case WM_NCMOUSEMOVE:        return "WM_NCMOUSEMOVE";
        case WM_NCLBUTTONDOWN:        return "WM_NCLBUTTONDOWN";
        case WM_NCLBUTTONUP:        return "WM_NCLBUTTONUP";
        case WM_NCLBUTTONDBLCLK:        return "WM_NCLBUTTONDDLCLK";
        case WM_NCRBUTTONDOWN:        return "WM_NCRBUTTONDOWN";
        case WM_NCRBUTTONUP:        return "WM_NCRBUTTONUP";
        case WM_NCRBUTTONDBLCLK:        return "WM_NCRBUTTONDBLCLK";
        case WM_NCMBUTTONDOWN:        return "WM_NCMBUTTONDOWN";
        case WM_NCMBUTTONUP:        return "WM_NCMBUTTONUP";
        case WM_NCMBUTTONDBLCLK:        return "WM_NCMBUTTONDBLCLK";
        case WM_KEYDOWN:        return "WM_KEYDOWN";
        case WM_KEYUP:        return "WM_KEYUP";
        case WM_CHAR:        return "WM_CHAR";
        case WM_DEADCHAR:        return "WM_DEADCHAR";
        case WM_SYSKEYDOWN:        return "WM_SYSKEYDOWN";
        case WM_SYSKEYUP:        return "WM_SYSKEYUP";
        case WM_SYSCHAR:        return "WM_SYSCHAR";
        case WM_SYSDEADCHAR:        return "WM_SYSDEADCHAR";
        case WM_KEYLAST:        return "WM_KEYLAST";
        case WM_INITDIALOG:        return "WM_INITDIALOG";
        case WM_COMMAND:        return "WM_COMMAND";
        case WM_SYSCOMMAND:        return "WM_SYSCOMMAND";
        case WM_TIMER:        return "WM_TIMER";
        case WM_HSCROLL:        return "WM_HSCROLL";
        case WM_VSCROLL:        return "WM_VSCROLL";
        case WM_INITMENU:        return "WM_INITMENU";
        case WM_INITMENUPOPUP:        return "WM_INITMENUPOPUP";
        case WM_MENUSELECT:        return "WM_MENUSELECT";
        case WM_MENUCHAR:        return "WM_MENUCHAR";
        case WM_ENTERIDLE:        return "WM_ENTERIDLE";
        //     case WM_MENURBUTTONUP:        return "WM_MENURBUTTONUP";
        //     case WM_MENUDRAG:        return "WM_MENUDRAG";
        //     case WM_MENUGETOBJECT:        return "WM_MENUGETOBJECT";
        //     case WM_UNINITMENUPOPUP:        return "WM_UNINITMENUPOPUP";
        //     case WM_MENUCOMMAND:        return "WM_MENUCOMMAND";
        //     case WM_CHANGEUISTATE:        return "WM_CHANGEUISTATE";
        //     case WM_UPDATEUISTATE:        return "WM_UPDATEUISTATE";
        //     case WM_QUERYUISTATE:        return "WM_QUERYUISTATE";
        case WM_CTLCOLORMSGBOX:        return "WM_CTLCOLORMSGBOX";
        case WM_CTLCOLOREDIT:        return "WM_CTLCOLOREDIT";
        case WM_CTLCOLORLISTBOX:        return "WM_CTLCOLORLISTBOX";
        case WM_CTLCOLORBTN:        return "WM_CTLCOLORBTN";
        case WM_CTLCOLORDLG:        return "WM_CTLCOLORDLG";
        case WM_CTLCOLORSCROLLBAR:        return "WM_CTLCOLORSCROLLBAR";
        case WM_CTLCOLORSTATIC:        return "WM_CTLCOLORSTATIC";
        case WM_MOUSEMOVE:        return "WM_MOUSEMOVE";
        case WM_LBUTTONDOWN:        return "WM_LBUTTONDOWN";
        case WM_LBUTTONUP:        return "WM_LBUTTONUP";
        case WM_LBUTTONDBLCLK:        return "WM_LBUTTONDBLCLK";
        case WM_RBUTTONDOWN:        return "WM_RBUTTONDOWN";
        case WM_RBUTTONUP:        return "WM_RBUTTONUP";
        case WM_RBUTTONDBLCLK:        return "WM_RBUTTONDBLCLK";
        case WM_MBUTTONDOWN:        return "WM_MBUTTONDOWN";
        case WM_MBUTTONUP:        return "WM_MBUTTONUP";
        case WM_MBUTTONDBLCLK:        return "WM_MBUTTONDBLCLK";
        //     case WM_MOUSEWHEEL:        return "WM_MOUSEWHEEL";
        case WM_PARENTNOTIFY:        return "WM_PARENTNOTIFY";
        case WM_ENTERMENULOOP:        return "WM_ENTERMENULOOP";
        case WM_EXITMENULOOP:        return "WM_EXITMENULOOP";
        case WM_NEXTMENU:        return "WM_NEXTMENU";
        case WM_SIZING:        return "WM_SIZING";
        case WM_CAPTURECHANGED:        return "WM_CAPTURECHANGED";
        case WM_MOVING:        return "WM_MOVING";
        case WM_POWERBROADCAST:        return "WM_POWERBROADCAST";
        case WM_DEVICECHANGE:        return "WM_DEVICECHANGE";
        case WM_IME_STARTCOMPOSITION:        return "WM_IME_STARTCOMPOSITION";
        case WM_IME_ENDCOMPOSITION:        return "WM_IME_ENDCOMPOSITION";
        case WM_IME_COMPOSITION:        return "WM_IME_COMPOSITION";
        case WM_IME_SETCONTEXT:        return "WM_IME_SETCONTEXT";
        case WM_IME_NOTIFY:        return "WM_IME_NOTIFY";
        case WM_IME_CONTROL:        return "WM_IME_CONTROL";
        case WM_IME_COMPOSITIONFULL:        return "WM_IME_COMPOSITIONFULL";
        case WM_IME_SELECT:        return "WM_IME_SELECT";
        case WM_IME_CHAR:        return "WM_IME_CHAR";
        //     case WM_IME_REQUEST:        return "WM_IME_REQUEST";
        case WM_IME_KEYDOWN:        return "WM_IME_KEYDOWN";
        case WM_IME_KEYUP:        return "WM_IME_KEYUP";
        case WM_MDICREATE:        return "WM_MDICREATE";
        case WM_MDIDESTROY:        return "WM_MDIDESTROY";
        case WM_MDIACTIVATE:        return "WM_MDIACTIVATE";
        case WM_MDIRESTORE:        return "WM_MDIRESTORE";
        case WM_MDINEXT:        return "WM_MDINEXT";
        case WM_MDIMAXIMIZE:        return "WM_MDIMAXIMIZE";
        case WM_MDITILE:        return "WM_MDITILE";
        case WM_MDICASCADE:        return "WM_MDICASCADE";
        case WM_MDIICONARRANGE:        return "WM_MDIICONARRANGE";
        case WM_MDIGETACTIVE:        return "WM_MDIGETACTIVE";
        case WM_MDISETMENU:        return "WM_MDISETMENU";
        case WM_ENTERSIZEMOVE:        return "WM_ENTERSIZEMOVE";
        case WM_EXITSIZEMOVE:        return "WM_EXITSIZEMOVE";
        case WM_DROPFILES:        return "WM_DROPFILES";
        case WM_MDIREFRESHMENU:        return "WM_MDIREFRESHMENU";
        //     case WM_MOUSEHOVER:        return "WM_MOUSEHOVER";
        //     case WM_MOUSELEAVE:        return "WM_MOUSELEAVE";
        case WM_CUT:        return "WM_CUT";
        case WM_COPY:        return "WM_COPY";
        case WM_PASTE:        return "WM_PASTE";
        case WM_CLEAR:        return "WM_CLEAR";
        case WM_UNDO:        return "WM_UNDO";
        case WM_RENDERFORMAT:        return "WM_RENDERFORMAT";
        case WM_RENDERALLFORMATS:        return "WM_RENDERALLFORMATS";
        case WM_DESTROYCLIPBOARD:        return "WM_DESTROYCLIPBOARD";
        case WM_DRAWCLIPBOARD:        return "WM_DRAWCLIPBOARD";
        case WM_PAINTCLIPBOARD:        return "WM_PAINTCLIPBOARD";
        case WM_VSCROLLCLIPBOARD:        return "WM_VSCROLLCLIPBOARD";
        case WM_SIZECLIPBOARD:        return "WM_SIZECLIPBOARD";
        case WM_ASKCBFORMATNAME:        return "WM_ASKCBFORMATNAME";
        case WM_CHANGECBCHAIN:        return "WM_CHANGECBCHAIN";
        case WM_HSCROLLCLIPBOARD:        return "WM_HSCROLLCLIPBOARD";
        case WM_QUERYNEWPALETTE:        return "WM_QUERYNEWPALETTE";
        case WM_PALETTEISCHANGING:        return "WM_PALETTEISCHANGING";
        case WM_PALETTECHANGED:        return "WM_PALETTECHANGED";
        case WM_HOTKEY:        return "WM_HOTKEY";
        case WM_PRINT:        return "WM_PRINT";
        case WM_PRINTCLIENT:        return "WM_PRINTCLIENT";
        case WM_HANDHELDFIRST:        return "WM_HANDHELDFIRST";
        case WM_HANDHELDLAST:        return "WM_HANDHELDLAST";
        case WM_PENWINFIRST:        return "WM_PENWINFIRST";
        case WM_PENWINLAST:        return "WM_PENWINLAST";
        //     case WM_COALESCE_FIRST:        return "WM_COALESCE_FIRST";
        //     case WM_COALESCE_LAST:      return "WM_COALESCE_LAST";
        case WM_DDE_INITIATE:        return "WM_DDE_INITIATE";
        case WM_DDE_TERMINATE:        return "WM_DDE_TERMINATE";
        case WM_DDE_ADVISE:        return "WM_DDE_ADVISE";
        case WM_DDE_UNADVISE:        return "WM_DDE_UNADVISE";
        case WM_DDE_ACK:        return "WM_DDE_ACK";
        case WM_DDE_DATA:        return "WM_DDE_DATA";
        case WM_DDE_REQUEST:        return "WM_DDE_REQUEST";
        case WM_DDE_POKE:        return "WM_DDE_POKE";
        case WM_DDE_EXECUTE:        return "WM_DDE_EXECUTE";
        case WM_APP:        return "WM_APP";
        case WM_USER:        return "WM_USER";
        default:
            {
            static char buf[100];
            _snprintf (buf, sizeof(buf), "%4x", msg);
            return buf;
            }
        }
    }
#endif      /* debug */

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*----------------------------------------------------------------------+
| name          setseed_centerWindowInWindow                            |
| author        MikeStratoti                            07/2002         |
+----------------------------------------------------------------------*/
static BOOL    setseed_centerWindowInWindow
(
HWND    const hwndChild,
HWND    const hwndParent
)
    {
    /*-------------------------------------------------------------------
     Center with respect to another window.  Specifying NULL for
     hwndParent centers hwndChild relative to the main screen.
    -------------------------------------------------------------------*/

    // Get the Height and Width of the child window.
    RECT    rcChild;
    GetWindowRect(hwndChild, &rcChild);
    int const cxChild = rcChild.right  - rcChild.left;
    int const cyChild = rcChild.bottom - rcChild.top;

    int     cxParent, cyParent;
    RECT    rcParent;
    if (hwndParent)
        {
        // Get the Height and Width of the parent window.
        GetWindowRect (hwndParent, &rcParent);
        cxParent = rcParent.right  - rcParent.left;
        cyParent = rcParent.bottom - rcParent.top;
        }
    else
        {
        cxParent = GetSystemMetrics (SM_CXSCREEN);
        cyParent = GetSystemMetrics (SM_CYSCREEN);
        rcParent.left   = 0;
        rcParent.top    = 0;
        rcParent.right  = cxParent;
        rcParent.bottom = cyParent;
        }

    // Get the display limits.
    HDC const hdc = GetDC(hwndChild);
    int const cxScreen = GetDeviceCaps(hdc, HORZRES);
    int const cyScreen = GetDeviceCaps(hdc, VERTRES);
    ReleaseDC (hwndChild, hdc);

    // Calculate new X position, then adjust for screen.
    int     xNew = rcParent.left + ((cxParent - cxChild) / 2);
    if (xNew < 0)
        {
        xNew = 0;
        }
    else if ((xNew + cxChild) > cxScreen)
        {
        xNew = cxScreen - cxChild;
        }

    // Calculate new Y position, then adjust for screen.
    int     yNew = rcParent.top  + ((cyParent - cyChild) / 2);
    if (yNew < 0)
        {
       yNew = 0;
        }
    else if ((yNew + cyChild) > cyScreen)
        {
        yNew = cyScreen - cyChild;
        }

    // Set it, and return.
    return SetWindowPos (hwndChild, ___,   xNew, yNew, ___, ___,  SWP_NOSIZE | SWP_NOZORDER);
    }


/*----------------------------------------------------------------------+
| name          setseed_loadSeedFileFilter                              |
| author        MikeStratoti                            03/2002         |
+----------------------------------------------------------------------*/
static void    setseed_loadSeedFileFilter
(
void
)
    {
    /*-------------------------------------------------------------------
     "LoadString" will stop at the first '\0' even though the resource compiler
     put the proper strings into the resource.
     The loop below replaces occurrences of '|' string separator with '\0'.
    -------------------------------------------------------------------*/
    LoadString (g_hInstance, IDS_DgnFileFilterString,  szFilter, sizeof(szFilter));

    for (int i=0; szFilter[i]; i++)
        {
        if (szFilter[i] == '|')
            szFilter[i] = '\0';
        }
    }


/*----------------------------------------------------------------------+
| name          setseed_createDgnByDialogDlgHookProc                    |
| author        MikeStratoti                            03/2002         |
+----------------------------------------------------------------------*/
UINT CALLBACK   setseed_createDgnByDialogDlgHookProc
(
HWND    /*const*/ hDlg,
UINT    /*const*/ uMsg,
WPARAM  /*const*/ wParam,
LPARAM  /*const*/ lParam
)
    {
    switch (uMsg)
        {
        case WM_NOTIFY:
            {
            OFNOTIFY  const * const pofn = (LPOFNOTIFY) lParam;
            /*-----------------------------------------------------------
             These WM_NOTIFY messages supersede the FILEOKSTRING, LBSELCHSTRING, SHAREVISTRING, and HELPMSGSTRING
             registered messages used by previous versions of the Open and Save As dialog boxes. However, the hook
             procedure also receives the superseded message after the WM_NOTIFY message if the WM_NOTIFY processing
             does not use SetWindowLong to set a nonzero DWL_MSGRESULT value.
            -----------------------------------------------------------*/
            SetWindowLong (hDlg, DWL_MSGRESULT, 1);

            switch (pofn->hdr.code)
                {
                case CDN_INITDONE:
                    setseed_centerWindowInWindow (GetParent (hDlg), NULL);
                    return  ___;        // The return value is ignored except for notification messages that specify otherwise.
                    break;
                }
            }
        }

    // Return Values
    //  If the hook procedure returns zero, the default dialog box procedure processes the message.
    //  If the hook procedure returns a nonzero value, the default dialog box procedure ignores the message.
    return 0;
    }


/*----------------------------------------------------------------------+
| name          setseed_createDgnByDialog                               |
| author        MikeStratoti                            03/2002         |
+----------------------------------------------------------------------*/
static DWORD   setseed_createDgnByDialog
(
char const * const  lpCmdLineIn
)
    {
    char    szSelectSeedDialogTitle[MAX_PATH*2] = {"Select a MicroStation DGN Seed File"};
    LoadString (g_hInstance, IDS_SelectSeedDialogTitle,  szSelectSeedDialogTitle, sizeof(szSelectSeedDialogTitle));

    setseed_loadSeedFileFilter();

    /*-------------------------------------------------------------------
     Show the dialog
    -------------------------------------------------------------------*/
    {
    OPENFILENAMEx       ofn;
    memset (&ofn, 0, sizeof(ofn));
    ofn.lStructSize     = bIsW2KExplorer ? sizeof (OPENFILENAMEx) : sizeof (OPENFILENAME);
    ofn.lpstrFilter     = szFilter;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFile       = persistantData.szSeedFilename;
    ofn.nMaxFile        = sizeof(persistantData).szSeedFilename;
    ofn.lpstrTitle      = szSelectSeedDialogTitle;
    ofn.hInstance       = g_hInstance;
    ofn.lpfnHook        = setseed_createDgnByDialogDlgHookProc;
    ofn.Flags           = OFN_ENABLEHOOK | OFN_DONTADDTORECENT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_ENABLESIZING;

    BOOL const ok = GetOpenFileName ((OPENFILENAME *)&ofn);
#if (debug)
        printf (" Got %d for '%s'\n", ok,  persistantData.szSeedFilename);
#endif
    if ( ! ok )
        return CommDlgExtendedError ();
    }

    DWORD status = SHSetValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValSeedFileName,  REG_SZ,    persistantData.szSeedFilename,      strlen(persistantData.szSeedFilename));

    /*-------------------------------------------------------------------
     Copy the file.
    -------------------------------------------------------------------*/
    char const * const fileNameFromExplorer = strstr (lpCmdLineIn, cmdlineShowCreateDgnDialog) + sizeof(cmdlineShowCreateDgnDialog);
#if (debug)
        printf (" Copyfile from \"%s\"   to   \"%s\"\n", persistantData.szSeedFilename, fileNameFromExplorer);
#endif

    {
    SetLastError(0);
    BOOL const  copyOK  = CopyFile (persistantData.szSeedFilename, fileNameFromExplorer, false);
#if (debug)
    DWORD       gle     = GetLastError();
    printf (" Copyfile %d %d\n", copyOK, gle);
#endif
    }

    /*-------------------------------------------------------------------
     Launch the Explorer on the newly created file.
    -------------------------------------------------------------------*/
    if (persistantData.bLaunchExplorer)
        {
        char renameCmdBuf[3*MAX_PATH];
        _snprintf (renameCmdBuf, sizeof(renameCmdBuf), "Explorer.exe /e,/select,\"%s\"", fileNameFromExplorer);
#if (debug)
        printf ("Invoking: \"%s\"\n", renameCmdBuf);
#endif
        WinExec (renameCmdBuf, SW_SHOWNORMAL);
        }

    return  0;
    }


/*----------------------------------------------------------------------+
| name          resizeDivider                                           |
| author        MikeStratoti                            03/2002         |
+----------------------------------------------------------------------*/
static void    resizeDivider
(
HWND    const hDlg
)
    {
    // Match the width of the divider to the width of the "FileOpen" control
    HWND                    hwndDivider     = GetDlgItem (hDlg, IDC_Divider);
    WINDOWPLACEMENT         wpDivider       = {sizeof(wpDivider)};
    RECT            * const rpDivider       = &wpDivider.rcNormalPosition;

    HWND                    hwndFileOpenCtrl= GetParent(hDlg);
    WINDOWPLACEMENT         wpFileOpenCtrl  = {sizeof(wpFileOpenCtrl)};

    GetWindowPlacement (hwndFileOpenCtrl, &wpFileOpenCtrl);
    GetWindowPlacement (hwndDivider,      &wpDivider);

    LONG const baseUnits = LOWORD(GetDialogBaseUnits ()) + 3;
    rpDivider->left   =  baseUnits;
    rpDivider->right  = (wpFileOpenCtrl.rcNormalPosition.right - wpFileOpenCtrl.rcNormalPosition.left) - baseUnits;

    SetWindowPlacement (hwndDivider, &wpDivider);
    }


/*----------------------------------------------------------------------+
| name          resizeUseShellNewFile                                   |
| author        MikeStratoti                            03/2002         |
+----------------------------------------------------------------------*/
static void    resizeUseShellNewFile
(
HWND    const hDlg
)
    {
    // Match the width of the Field to the width of the "FileOpen" control
    HWND                    hwndField     = GetDlgItem (hDlg, IDC_UseShellNewFile);
    WINDOWPLACEMENT         wpField       = {sizeof(wpField)};
    RECT            * const rpField       = &wpField.rcNormalPosition;

    HWND                    hwndFileOpenCtrl= GetParent(hDlg);
    WINDOWPLACEMENT         wpFileOpenCtrl  = {sizeof(wpFileOpenCtrl)};

    GetWindowPlacement (hwndFileOpenCtrl, &wpFileOpenCtrl);
    GetWindowPlacement (hwndField,        &wpField);

    LONG const baseUnits = LOWORD(GetDialogBaseUnits ()) + 3;
    rpField->right  = (wpFileOpenCtrl.rcNormalPosition.right - wpFileOpenCtrl.rcNormalPosition.left) - baseUnits;

    SetWindowPlacement (hwndField, &wpField);
    }


/*----------------------------------------------------------------------+
| name          setseed_maintainUseShellNewFile                         |
| author        MikeStratoti                            03/2002         |
+----------------------------------------------------------------------*/
void setseed_maintainUseShellNewFile
(
HWND    const hDlg
)
    {
    static int  readFromDialog;
    static char szFmt[MAX_PATH + sizeof(szUseShellNewFile)] = "";
    char        szPath[MAX_PATH]= "", *szBase=szPath;

    HWND    hwnd = GetDlgItem (hDlg, IDC_UseShellNewFile);
    if ( ! *szFmt)
        GetWindowText (hwnd, szFmt, sizeof(szFmt));

    GetFullPathName (persistantData.szSeedFilename, sizeof(szPath), szPath, &szBase);
    _snprintf (szUseShellNewFile, sizeof(szUseShellNewFile), szFmt, szBase);
    SetWindowText (hwnd, szUseShellNewFile);
    }


/*----------------------------------------------------------------------+
| name          EnumChildProc                                           |
| author        MikeStratoti                            03/2002         |
+----------------------------------------------------------------------*/
#if debug
static BOOL CALLBACK EnumChildProc         // For Debugging only
(
HWND    hwndChild,
LPARAM  lParam
)
    {
    static int cnt;
    char title[512] = {'\0'};
    GetWindowText (hwndChild, title, sizeof(title));
    printf (" %2d.  %8x   %3d  '%s'\n", ++cnt, hwndChild, GetDlgCtrlID(hwndChild), title);
    return true;
    }
#endif


/*----------------------------------------------------------------------+
| name          setseed_openDlgHookProc                                 |
| author        MikeStratoti                            03/2002         |
+----------------------------------------------------------------------*/
LONG CALLBACK   setseed_subClassFuncSetSeedOKBtn
(
HWND    hWnd,
UINT    message,
WPARAM  wParam,
LONG    lParam
)
    {
#if debug
    static int cnt;
    printf ("%5d. %#x %20s  %8x  %8x\n", cnt++, hWnd,  msgToStr(message), wParam, lParam);
#endif

    /*-------------------------------------------------------------------
     This routine is needed to handle the OK button.  The FileOpen dialog
     normally requires a file name for OK to close the dialog.  However,
     in this case it is proper to close the dialog without a file name
     if other radio buttons are selected.
    -------------------------------------------------------------------*/
    HWND const  hwndOKBtn = hWnd;
    static int  isButtonDown;

    switch (message)
        {
        case BM_SETSTATE:
            isButtonDown = (int) wParam;
            break;

        case  WM_LBUTTONUP:
            // Only close the dialog if either top radio buttons are checked.  Otherwise rely on the default processing that requires a file name
            if ( ! isButtonDown)
                break;

            HWND const hDlg = GetParent(hwndOKBtn);
            if (BST_CHECKED == IsDlgButtonChecked (g_hDlg, IDC_UseDialogNewFile)   ||  BST_CHECKED == IsDlgButtonChecked (g_hDlg, IDC_UseSeedFilenameFromMicroStation))
                EndDialog (hDlg, 1);        // OK to exit, not an error
            break;
        }

    return CallWindowProc (g_lpfnSetSeedOKBtnWndProc, hWnd, message, wParam, lParam);
    }

typedef enum
    {
    GET_SETTINGS    =   1,
    SET_SETTINGS
    } ControlListBoxDisplay;

static DWORD                   controlListBoxDisplay
(
ControlListBoxDisplay    const  operation,
HWND                     const  hDlg,
DWORD                  * const  pStyle
)
    {
    HWND    const hwndParent     = GetParent(hDlg);                             // Top dialog
    HWND    const hwndListParent = GetDlgItem (hwndParent, lst2);               // ..SHELLDLL_DefView
    HWND    const hwndList       = GetDlgItem (hwndListParent, 1);              // ....SysListView32

    if (! hwndParent ||  !hwndListParent ||  !hwndList)
        {
        __asm int 3;
        return *pStyle;
        }

    if (GET_SETTINGS == operation)
        return *pStyle = LVS_TYPEMASK & GetWindowLong (hwndList, GWL_STYLE) ;
    else
        {
        DWORD const currentStyle = GetWindowLong (hwndList, GWL_STYLE);                 // Get the list view's current style
        DWORD const rtn = SetWindowLong (hwndList, GWL_STYLE, (currentStyle & ~LVS_TYPEMASK) | (LVS_TYPEMASK & *pStyle));       // Change the style to the mode specified
//      SetWindowPos (hwndList, ___, ___, ___, ___, ___, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOSIZE |  SWP_NOZORDER);
        ListView_EnsureVisible (hwndList, 0, false);
        UpdateWindow (hwndList);
        return rtn;
        }
    }



/*----------------------------------------------------------------------+
| name          setseed_openDlgHookProc                                 |
| author        MikeStratoti                            03/2002         |
+----------------------------------------------------------------------*/
UINT CALLBACK   setseed_openDlgHookProc
(
HWND    /*const*/ hDlg,
UINT    /*const*/ uMsg,
WPARAM  /*const*/ wParam,
LPARAM  /*const*/ lParam
)
    {
    //{ static int cnt;     printf ("%4d.  %#x\n", cnt++, uMsg);    }

    static int     stdFileOpenDialogIDs [] =            // Disable these controls on the OpenFile dialog when not using a static seed filename
        {
#if 0
        1091,   // 'Look &in:'
        1137,   // Looking combo box
        1088,
        1184,
        1120,
        1121,
        1090,
        1148,
        1089,
        1136,
#endif
        1089,   // IDC_FileTypeStatic
        1090,   // IDC_FilenameStatic
        1136,   // IDC_FileTypeCombo
        1148,   // IDC_FilenameCombo         Win2K
        1152    // IDC_FilenameEdit          WinNT
        } ;

    switch (uMsg)
        {
        case WM_NOTIFY:
            {
            OFNOTIFY  const * const pofn = (LPOFNOTIFY) lParam;
            /*-----------------------------------------------------------
             These WM_NOTIFY messages supersede the FILEOKSTRING, LBSELCHSTRING, SHAREVISTRING, and HELPMSGSTRING
             registered messages used by previous versions of the Open and Save As dialog boxes. However, the hook
             procedure also receives the superseded message after the WM_NOTIFY message if the WM_NOTIFY processing
             does not use SetWindowLong to set a nonzero DWL_MSGRESULT value.
            -----------------------------------------------------------*/
            SetWindowLong (hDlg, DWL_MSGRESULT, 1);

            switch (pofn->hdr.code)
                {
                case CDN_INITDONE:
                    g_hDlg = hDlg;
                    resizeDivider (hDlg);
                    resizeUseShellNewFile (hDlg);
                    setseed_centerWindowInWindow (GetParent (hDlg), NULL);
//                    controlListBoxDisplay (SET_SETTINGS, hDlg, &persistantData.dwListBoxStyle);
                    return  ___;        // The return value is ignored except for notification messages that specify otherwise.
                    break;

                case CDN_SELCHANGE:
                    {
                    static bool          initialSetup;
                    if ( ! initialSetup)
                        {
                        controlListBoxDisplay (SET_SETTINGS, hDlg, &persistantData.dwListBoxStyle);     // Only once needed
                        initialSetup++;
                        }
                    }

                case CDN_FILEOK:
                    persistantData.dwListBoxStyle = controlListBoxDisplay (GET_SETTINGS, hDlg, &persistantData.dwListBoxStyle);
                    return 0;

                default:
                    break;
                }
            break;
            }

        case WM_WINDOWPOSCHANGED:
        case WM_WINDOWPOSCHANGING:
            resizeDivider (hDlg);
            resizeUseShellNewFile (hDlg);
            break;


        case WM_INITDIALOG:
            {
            setseed_maintainUseShellNewFile (hDlg);

            // Subclass the OK/Open button so that we can catch its press even though there may be no file specified
            HWND const hwndOKBtn = GetDlgItem(GetParent(hDlg), IDOK);
            g_lpfnSetSeedOKBtnWndProc = (WNDPROC)SetWindowLong (hwndOKBtn, GWL_WNDPROC, (DWORD) setseed_subClassFuncSetSeedOKBtn);

            // Change button text from "Open" to "OK"
            {
            char szCaption[100]="OK";
            LoadString (g_hInstance, IDS_OKBtnTitle,  szCaption, sizeof(szCaption));
            SendMessage (hwndOKBtn, WM_SETTEXT, ___, (LPARAM)szCaption);
            }

            // Make sure that only one radio button is selected
            int const checkMe = persistantData.bShowDialog ? IDC_UseDialogNewFile :
                (persistantData.bUseSeedFilenameFromMicroStation ? IDC_UseSeedFilenameFromMicroStation : IDC_UseShellNewFile);
            CheckRadioButton (hDlg,  IDC_UseShellNewFile, IDC_UseSeedFilenameFromMicroStation, checkMe);

            CheckDlgButton(hDlg, IDC_LaunchExplorer,  persistantData.bLaunchExplorer ? BST_CHECKED : BST_UNCHECKED);
            EnableWindow (GetDlgItem (hDlg, IDC_LaunchExplorer), persistantData.bShowDialog);

            // [En|Dis]able the standard file open dialog controls
            HWND const hwmdParent = GetParent(hDlg);
            BOOL const show       = ! persistantData.bShowDialog  &&  !persistantData.bUseSeedFilenameFromMicroStation;
            for (int i=0; i < DIM(stdFileOpenDialogIDs); i++)
                EnableWindow (GetDlgItem (hwmdParent, stdFileOpenDialogIDs[i]), show);

            break;
            }

        case WM_COMMAND:
            {
#if debug
            printf ("Child window IDs:\n");
            EnumChildWindows(GetParent(hDlg), EnumChildProc, ___);              // Needed to determine IDs.   DEBUG ONLY
#endif


            EnableWindow (GetDlgItem (hDlg, IDC_LaunchExplorer), (BST_CHECKED == IsDlgButtonChecked (hDlg, IDC_UseDialogNewFile)));
            // [En|Dis]able the standard file open dialog controls
            HWND const hwmdParent = GetParent(hDlg);
            BOOL const show       = (BST_CHECKED == IsDlgButtonChecked (hDlg, IDC_UseShellNewFile) );
            for (int i=0; i < DIM(stdFileOpenDialogIDs); i++)
                EnableWindow (GetDlgItem (hwmdParent, stdFileOpenDialogIDs[i]), show);
            break;
            }


        case WM_DESTROY:
            // NB: Only our additional controls exist at this point.  The standard one are gone.
            persistantData.bLaunchExplorer                  = (BST_CHECKED == IsDlgButtonChecked (hDlg, IDC_LaunchExplorer)   );
            persistantData.bShowDialog                      = (BST_CHECKED == IsDlgButtonChecked (hDlg, IDC_UseDialogNewFile) );
            persistantData.bUseSeedFilenameFromMicroStation = (BST_CHECKED == IsDlgButtonChecked (hDlg, IDC_UseSeedFilenameFromMicroStation) );
            break;
        }

    // Return Values
    //  If the hook procedure returns zero, the default dialog box procedure processes the message.
    //  If the hook procedure returns a nonzero value, the default dialog box procedure ignores the message.
    return 0;
    }

/*----------------------------------------------------------------------+
| name          WinMain                                                 |
| author        MikeStratoti                            03/2002         |
+----------------------------------------------------------------------*/
extern "C" Public int WINAPI WinMain
(
HINSTANCE   hInstance,          // handle to current instance
HINSTANCE   hPrevInstance,      // handle to previous instance
LPSTR       lpCmdLineIn,        // pointer to command line
int         nCmdShow            // show state of window
)
    {
    /*-------------------------------------------------------------------
     Initialize.
    -------------------------------------------------------------------*/
    g_hInstance =  hInstance;

    {
    OSVERSIONINFO   osvi = {sizeof(osvi)};
    GetVersionEx (&osvi);
    bIsW2KExplorer =  (osvi.dwMajorVersion > 4)  ||  ( (osvi.dwMajorVersion == 4)  &&  (osvi.dwMinorVersion > 0) ) ;
    }

    /*-------------------------------------------------------------------
     Create a console output window for out debugging messages.
    -------------------------------------------------------------------*/
#if debug
    AllocConsole();                  // Allocate console window
    SetConsoleTitle ("CreateMicroStation DGN magic");

    freopen("CONIN$",  "a", stdin);  // Redirect to console
    freopen("CONOUT$", "a", stdout); // Redirect to console
    freopen("CONOUT$", "a", stderr); // Redirect to console
#endif

    /*-------------------------------------------------------------------
     Get the saved parameters if available.
    -------------------------------------------------------------------*/
    {
    DWORD   regType, len, status;
    len = sizeof(persistantData).szSeedFilename;                     status = SHGetValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValSeedFileName,                    &regType, persistantData.szSeedFilename, &len);
    len = sizeof(persistantData).bShowDialog;                        status = SHGetValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValShowDialog,                      &regType, &persistantData.bShowDialog, &len);
    len = sizeof(persistantData).bLaunchExplorer;                    status = SHGetValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValLaunchExplorer,                  &regType, &persistantData.bLaunchExplorer, &len);
    len = sizeof(persistantData).dwListBoxStyle;                     status = SHGetValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValListViewStyle,                    &regType, &persistantData.dwListBoxStyle, &len);
    len = sizeof(persistantData).bUseSeedFilenameFromMicroStation;   status = SHGetValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValUseSeedFilenameFromMicroStation, &regType, &persistantData.bUseSeedFilenameFromMicroStation, &len);

    if (debug)
        printf (" Loaded seed last filename '%s' (%d)\n", persistantData.szSeedFilename, status);
    }

    setseed_loadSeedFileFilter();

    /*-------------------------------------------------------------------
     Creat the DGN file from a dialog.
    -------------------------------------------------------------------*/
    if (strstr (lpCmdLineIn, cmdlineShowCreateDgnDialog))
        {
        return setseed_createDgnByDialog (lpCmdLineIn);
        }

    char szSetSeedDialogTitle[MAX_PATH*2] = {"Select MicroStation DGN Seed File for Explorer->New File"};
    LoadString (g_hInstance, IDS_SetSeedDialogTitle,  szSetSeedDialogTitle, sizeof(szSetSeedDialogTitle));

    /*-------------------------------------------------------------------
     Show the dialog
    -------------------------------------------------------------------*/
    {
    OPENFILENAMEx       ofn;
    memset (&ofn, 0, sizeof(ofn));
    ofn.lStructSize     = bIsW2KExplorer ? sizeof (OPENFILENAMEx) : sizeof (OPENFILENAME);
    ofn.lpstrFilter     = szFilter;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFile       = persistantData.szSeedFilename;
    ofn.nMaxFile        = sizeof(persistantData).szSeedFilename;
    ofn.lpstrTitle      = szSetSeedDialogTitle;
    ofn.hInstance       = g_hInstance;
    ofn.lpfnHook        = setseed_openDlgHookProc;
    ofn.lpTemplateName  = MAKEINTRESOURCE (IDD_OpenDialogTemplate);
    ofn.Flags           = OFN_DONTADDTORECENT | OFN_SHAREAWARE | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NOCHANGEDIR
                        | OFN_ENABLEHOOK | OFN_ENABLETEMPLATE |  OFN_ENABLESIZING;

    BOOL const ok = GetOpenFileName ((OPENFILENAME *)&ofn);
#if (debug)
        printf (" Got %d for '%s'\n", ok,  persistantData.szSeedFilename);
#endif
    if ( ! ok )
        return CommDlgExtendedError ();
    }

    /*-------------------------------------------------------------------
     Save the seed directory for later use.
    -------------------------------------------------------------------*/
    {
    DWORD status;

    // Save our persistant info
    status = SHSetValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValSeedFileName,                   REG_SZ,     persistantData.szSeedFilename,  strlen(persistantData.szSeedFilename));
    status = SHSetValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValShowDialog,                     REG_DWORD, &persistantData.bShowDialog,     sizeof(persistantData).bShowDialog);
    status = SHSetValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValLaunchExplorer,                 REG_DWORD, &persistantData.bLaunchExplorer, sizeof(persistantData).bLaunchExplorer);
    status = SHSetValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValListViewStyle,                  REG_DWORD, &persistantData.dwListBoxStyle,  sizeof(persistantData).dwListBoxStyle);
    status = SHSetValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValUseSeedFilenameFromMicroStation,REG_DWORD, &persistantData.bUseSeedFilenameFromMicroStation, sizeof(persistantData).bUseSeedFilenameFromMicroStation);

    // Fixup the Explorer's  menu
    if (persistantData.bShowDialog)
        {
        char    thisExe[MAX_PATH*3] = {"\""};
        GetModuleFileName (___, thisExe+1, sizeof(thisExe) - 2);                 // For \" we will add next
        strcat (thisExe, "\" ");
        strcat (thisExe, cmdlineShowCreateDgnDialog);
        strcat (thisExe, " %1");
        status = SHSetValue    (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValCommand,    REG_SZ,     thisExe,                         strlen(thisExe));
        status = SHDeleteValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValNewFileName);
        }
    else
        {
        status = SHSetValue    (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValNewFileName, REG_SZ,    persistantData.szSeedFilename,   strlen(persistantData.szSeedFilename));
        status = SHDeleteValue (HKEY_CLASSES_ROOT, registrySaveKey, registrySaveValCommand);
        }

#if (debug)
        printf (" Saved seed last path '%s' (%d)\n", persistantData.szSeedFilename, status);
#endif
    }

    /*-------------------------------------------------------------------
     Out'a here!
    -------------------------------------------------------------------*/
#if (debug)
        printf("Press a key...");
        _getch();
#endif

    return 0;
    }

