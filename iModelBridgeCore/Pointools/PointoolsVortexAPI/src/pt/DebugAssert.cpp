/**
 @file debugAssert.cpp

 Windows implementation of assertion routines.

 @maintainer Morgan McGuire, graphics3d.com
 
 @created 2001-08-26
 @edited  2004-09-16

  @feb 07 - unicode support added by Faraz Ravi
 */

#include "PointoolsVortexAPIInternal.h"

#ifdef NEEDS_WORK_VORTEX_DGNDB

#include <pt/ptUnicode.h>
#include <pt/debugAssert.h>
#include <pt/format.h>
#include <pt/prompt.h>
#include <pt/debugPrintf.h>
#include <pt/Log.h>

using namespace std;

namespace pt { namespace _internal {

AssertionHook _debugHook = _handleDebugAssert_;

#ifdef __linux__
    Display*      x11Display = NULL;
    Window        x11Window  = 0;
#endif


#ifdef _WIN32
static void postToClipboard(const TCHAR *text) {
    if (OpenClipboard(NULL)) {
        HGLOBAL hMem = GlobalAlloc(GHND | GMEM_DDESHARE, _tcslen(text) + 1);
        if (hMem) {
            TCHAR *pMem = (TCHAR*)GlobalLock(hMem);
            _tcscpy(pMem, text);
            GlobalUnlock(hMem);

            EmptyClipboard();
            SetClipboardData(CF_TEXT, hMem);
        }

        CloseClipboard();
        GlobalFree(hMem);
    }
}
#endif

/**
 outTitle should be set before the call
 */
static void createErrorMessage(
    const TCHAR*         expression,
    const std::tstring&  message,
    const TCHAR*         filename,
    int                 lineNumber,
    std::tstring&        outTitle,
    std::tstring&        outMessage) {

    std::tstring le = _T("");
    TCHAR* newline = _T("\n");

    #ifdef _WIN32
        newline = _T("\r\n");

        // The last error value.  (Which is preserved across the call).
        DWORD lastErr = GetLastError();
    
        // The decoded message from FormatMessage
        LPTSTR formatMsg = NULL;

        if (NULL == formatMsg) {
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_IGNORE_INSERTS |
                          FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            lastErr,
                            0,
                            (LPTSTR)&formatMsg,
                            0,
                            NULL);
        }

        // Make sure the message got translated into something.
        LPTSTR realLastErr;
		TCHAR formatBuffer[256];
        if (NULL != formatMsg) {
            realLastErr = formatMsg;
        } else {
            realLastErr = _T("Last error code does not exist.");
        }

		if (lastErr != 0) {
	        _stprintf(formatBuffer, _T("Last Error (0x%08X): %s\r\n\r\n"), lastErr, (LPCTSTR)realLastErr);
			le = formatBuffer;
		}

        // Get rid of the allocated memory from FormatMessage.
        if (NULL != formatMsg) {
            LocalFree((LPVOID)formatMsg);
        }

        TCHAR modulePath[MAX_PATH];
        GetModuleFileName(NULL, modulePath, MAX_PATH);

        const TCHAR* moduleName = _tcsrchr(modulePath, '\\');
        outTitle = outTitle + tstring(_T(" - ")) + tstring(moduleName ? (moduleName + 1) : modulePath);

    #endif

    // Build the message.
    _stprintf(formatBuffer, _T("%s%s%sExpression: %s%s%s:%d%s%s%s"),
		/*pt::format(_T("%s%s%sExpression: %s%s%s:%d%s%s%s"),*/ 
                 message.c_str(), newline, newline, expression, newline, 
                 filename, lineNumber, newline, newline, le.c_str());
    outMessage = formatBuffer;
}


bool _handleDebugAssert_(
    const TCHAR*         expression,
    const std::tstring&  message,
    const TCHAR*         filename,
    int                 lineNumber,
    bool&               ignoreAlways,
    bool                useGuiPrompt) {

    std::tstring dialogTitle = _T("Assertion Failure");
    std::tstring dialogText = _T("");

    createErrorMessage(expression, message, filename, lineNumber, dialogTitle, dialogText);

    #ifdef _WIN32
        DWORD lastErr = GetLastError();
        postToClipboard(dialogText.c_str());
    #endif

    const int cBreak = 0;
    const int cIgnore = 1;
    const int cIgnoreAlways = 2;
    const int cAbort = 3;

    static TCHAR* choices[] = {_T("Debug"), _T("Ignore"), _T("Ignore Always"), _T("Exit")};

    // Log the error
    Log::common()->print(std::tstring(_T("\n**************************\n\n")) + dialogTitle + _T("\n") + dialogText);

    int result = pt::prompt(dialogTitle.c_str(), dialogText.c_str(), (const TCHAR**)choices, 4, useGuiPrompt);

    #ifdef _WIN32
        // Put the incoming last error back.
        SetLastError(lastErr);
    #endif

    switch (result) {
    // -1 shouldn't actually occur because it means 
    // that we're in release mode.
    case -1:
    case cBreak:
        return true;
        break;

    case cIgnore:
        return false;
        break;
   
    case cIgnoreAlways:
        ignoreAlways = true;
        return false;
        break;

    case cAbort:
        exit(-1);
        return false;
        break;
    default:
        // Shouldn't get here
        return false;
        break;
    }

}


void _handleErrorCheck_(    
    const TCHAR*         expression,
    const std::tstring&  message,
    const TCHAR*         filename,
    int                 lineNumber,
    bool                useGuiPrompt) {

    std::tstring dialogTitle = _T("Critical Error");
    std::tstring dialogText = _T("");

    createErrorMessage(expression, message, filename, lineNumber, dialogTitle, dialogText);

    // Log the error
    Log::common()->print(std::tstring(_T("\n**************************\n\n")) + dialogTitle + _T("\n") + dialogText);

    static TCHAR* choices[] = {_T("Ok")};

    std::tstring m = 
        std::tstring(_T("An internal error has occured in your program and it will now close.  Details about the error have been reported in \"")) +
            Log::getCommonLogFilename() + _T("\".");


    int result = pt::prompt(_T("Error"), m.c_str(), (const TCHAR**)choices, 1, useGuiPrompt);
    (void)result;
}


#ifdef _WIN32
static HCURSOR oldCursor;
static RECT    oldCursorRect;
static POINT   oldCursorPos;
static int     oldShowCursorCount;
#endif

void _releaseInputGrab_() {
    #ifdef _WIN32

        GetCursorPos(&oldCursorPos);

        // Stop hiding the cursor if the application hid it.
        oldShowCursorCount = ShowCursor(true) - 1;

        if (oldShowCursorCount < -1) {
            for (int c = oldShowCursorCount; c < -1; ++c) {
                ShowCursor(true);
            }
        }

        // Set the default cursor in case the application
        // set the cursor to NULL.
        oldCursor = GetCursor();
        SetCursor(LoadCursor(NULL, IDC_ARROW));

        // Allow the cursor full access to the screen
        GetClipCursor(&oldCursorRect);
        ClipCursor(NULL);
        
    #elif defined(__linux__)
        if (x11Display != NULL) {
            XUngrabPointer(x11Display, CurrentTime);
            XUngrabKeyboard(x11Display, CurrentTime);
            if (x11Window != 0) {
                //XUndefineCursor(x11Display, x11Window);
                // TODO: Note that we leak this cursor; it should be
                // freed in the restore code.
                Cursor c = XCreateFontCursor(x11Display, 68);
                XDefineCursor(x11Display, x11Window, c);
            }
            XSync(x11Display, false);           
            XAllowEvents(x11Display, AsyncPointer, CurrentTime);
            XFlush(x11Display);
        }
    #elif defined(__APPLE__)
        // TODO: OS X
    #endif
}


void _restoreInputGrab_() {
    #ifdef _WIN32

        // Restore the old clipping region
        ClipCursor(&oldCursorRect);

        SetCursorPos(oldCursorPos.x, oldCursorPos.y);

        // Restore the old cursor
        SetCursor(oldCursor);

        // Restore old visibility count
        if (oldShowCursorCount < 0) {
            for (int c = 0; c > oldShowCursorCount; --c) {
                ShowCursor(false);
            }
        }
        
    #elif defined(__linux__)
        // TODO: Linux
    #elif defined(__APPLE__)
        // TODO: OS X
    #endif
}


}; // internal namespace
 
void setAssertionHook(AssertionHook hook) {
	pt::_internal::_debugHook = hook;
}


}; // namespace


#endif