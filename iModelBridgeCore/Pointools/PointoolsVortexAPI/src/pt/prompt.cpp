/**
 @file prompt.cpp
 
 @author Morgan McGuire, morgan@graphics3d.com 
 @cite Windows dialog interface by Max McGuire, mmcguire@ironlore.com
 @cite Font setting code by Kurt Miller, kurt@flipcode.com

 @created 2000-08-26
 @edited  2003-09-13
 */

#include "PointoolsVortexAPIInternal.h"

#ifdef NEEDS_WORK_VORTEX_DGNDB
#include <pt/prompt.h>

#include <stdio.h>

#ifdef WIN32
    #include <windows.h>
    #include <strstream>
    #include <conio.h>
	#include <tchar.h>	
#else
    #define _getch getchar
#endif

#ifdef __APPLE__
    #import <AppKit/AppKit.h>
#endif

namespace pt {

#ifdef WIN32

namespace _internal {
/**
 Generic Win32 dialog facility.
 @author Max McGuire
 */
class DialogTemplate {
public:

  DialogTemplate(LPCTSTR caption, DWORD style, 
                 int x, int y, int w, int h,
                 LPCTSTR font = NULL, WORD fontSize = 8) {
    
      usedBufferLength = sizeof(DLGTEMPLATE);
      totalBufferLength = usedBufferLength;

      dialogTemplate = (DLGTEMPLATE*)malloc(totalBufferLength);

      dialogTemplate->style = style;
        
      if (font != NULL) {
        dialogTemplate->style |= DS_SETFONT;
      }
        
      dialogTemplate->x     = x;
      dialogTemplate->y     = y;
      dialogTemplate->cx    = w;
      dialogTemplate->cy    = h;
      dialogTemplate->cdit  = 0;
        
      dialogTemplate->dwExtendedStyle = 0;

      // The dialog box doesn't have a menu or a special class
      AppendData((void*)"\0", 2);
      AppendData((void*)"\0", 2);

      // Add the dialog's caption to the template

      AppendString(caption);

      if (font != NULL) {
          AppendData(&fontSize, sizeof(WORD));
          AppendString(font);
      }
    }

  void AddComponent(LPCTSTR type, LPCTSTR caption, DWORD style, DWORD exStyle, int x, int y, int w, int h, WORD id) {

      DLGITEMTEMPLATE item;

      item.style = style;
      item.x     = x;
      item.y     = y;
      item.cx    = w;
      item.cy    = h;
      item.id    = id;

      item.dwExtendedStyle = exStyle;

      AppendData(&item, sizeof(DLGITEMTEMPLATE));
        
      AppendString(type);
      AppendString(caption);

      WORD creationDataLength = 0;
      AppendData(&creationDataLength, sizeof(WORD));

      // Increment the component count
      dialogTemplate->cdit++;

    }


  void AddButton(LPCTSTR caption, DWORD style, DWORD exStyle, int x, int y, int w, int h, WORD id) {

      AddStandardComponent(0x0080, caption, style, exStyle, x, y, w, h, id);

      WORD creationDataLength = 0;
      AppendData(&creationDataLength, sizeof(WORD));

    }


  void AddEditBox(LPCTSTR caption, DWORD style, DWORD exStyle, int x, int y, int w, int h, WORD id) {

      AddStandardComponent(0x0081, caption, style, exStyle, x, y, w, h, id);
    
      WORD creationDataLength = 0;
      AppendData(&creationDataLength, sizeof(WORD));
    
    }


  void AddStatic(LPCTSTR caption, DWORD style, DWORD exStyle, int x, int y, int w, int h, WORD id) {

      AddStandardComponent(0x0082, caption, style, exStyle, x, y, w, h, id);
    
      WORD creationDataLength = 0;
      AppendData(&creationDataLength, sizeof(WORD));
    
    }


  void AddListBox(LPCTSTR caption, DWORD style, DWORD exStyle, int x, int y, int w, int h, WORD id) {

      AddStandardComponent(0x0083, caption, style, exStyle, x, y, w, h, id);
    
      WORD creationDataLength = sizeof(WORD) + 5 * sizeof(WCHAR);
      AppendData(&creationDataLength, sizeof(WORD));

      AppendString(_T("TEST"));
    
    }
    

  void AddScrollBar(LPCTSTR caption, DWORD style, DWORD exStyle, int x, int y, int w, int h, WORD id) {

      AddStandardComponent(0x0084, caption, style, exStyle, x, y, w, h, id);
    
      WORD creationDataLength = 0;
      AppendData(&creationDataLength, sizeof(WORD));
    
    }


  void AddComboBox(LPCTSTR caption, DWORD style, DWORD exStyle, int x, int y, int w, int h, WORD id) {

      AddStandardComponent(0x0085, caption, style, exStyle, x, y, w, h, id);
    
      WORD creationDataLength = 0;
      AppendData(&creationDataLength, sizeof(WORD));

    }


  /**
   *
   * Returns a pointer to the Win32 dialog template which the object
   * represents. This pointer may become invalid if additional components
   * are added to the template.
   *
   */
  operator const DLGTEMPLATE*() const {
      return dialogTemplate;
  }

  virtual ~DialogTemplate() {
      free(dialogTemplate);
  }

protected:

  void AddStandardComponent(WORD type, LPCTSTR caption, DWORD style, DWORD exStyle, 
                            int x, int y, int w, int h, WORD id, LPTSTR font = NULL, WORD fontSize = 8) {

      DLGITEMTEMPLATE item;

      // DWORD align the beginning of the component data
        
      AlignData(sizeof(DWORD));

      item.style = style;
      if (font != NULL) {
          item.style |= DS_SETFONT;
      }
      item.x     = x;
      item.y     = y;
      item.cx    = w;
      item.cy    = h;
      item.id    = id;

      item.dwExtendedStyle = exStyle;

      AppendData(&item, sizeof(DLGITEMTEMPLATE));
        
      WORD preType = 0xFFFF;
        
      AppendData(&preType, sizeof(WORD));
      AppendData(&type, sizeof(WORD));

      AppendString(caption);

      if (font != NULL) {
          AppendData(&fontSize, sizeof(WORD));
          AppendString(font);
      }

      // Increment the component count
      dialogTemplate->cdit++;
  }

    
  void AlignData(int size) {

      int paddingSize = usedBufferLength % size;
        
      if (paddingSize != 0) {
          EnsureSpace(paddingSize);
          usedBufferLength += paddingSize;
      }

  }

  void AppendString(LPCTSTR string) {
#ifndef UNICODE
      int length = MultiByteToWideChar(CP_ACP, 0, string, -1, NULL, 0);

      WCHAR* wideString = (WCHAR*)malloc(sizeof(WCHAR) * length);
      MultiByteToWideChar(CP_ACP, 0, string, -1, wideString, length);

      AppendData(wideString, length * sizeof(WCHAR));
      free(wideString);
#else
      AppendData(const_cast<wchar_t*>(string), wcslen(string) * sizeof(WCHAR));
#endif

  }

  void AppendData(void* data, int dataLength) {

      EnsureSpace(dataLength);

      memcpy((char*)dialogTemplate + usedBufferLength, data, dataLength);
      usedBufferLength += dataLength;

  }

  void EnsureSpace(int length) {
      if (length + usedBufferLength > totalBufferLength) {
          totalBufferLength += length * 2;

          void* newBuffer = malloc(totalBufferLength);
          memcpy(newBuffer, dialogTemplate, usedBufferLength);
            
          free(dialogTemplate);
          dialogTemplate = (DLGTEMPLATE*)newBuffer;
      }
  }

private:

  DLGTEMPLATE*      dialogTemplate;

  int               totalBufferLength;
  int               usedBufferLength;

};


struct PromptParams {
    const TCHAR* message;
    const TCHAR* title;
};

/**
 * Constants for controls.
 */
#define IDC_MESSAGE     1000
#define IDC_BUTTON0     2000

INT_PTR CALLBACK PromptDlgProc(HWND hDlg, UINT msg,
                               WPARAM wParam, LPARAM lParam) {
  switch(msg) {
    case WM_INITDIALOG:
      {
        PromptParams *params = (PromptParams*)lParam;
        ::SetWindowText(::GetDlgItem(hDlg, IDC_MESSAGE), params->message);

        ::SetFocus(::GetDlgItem(hDlg, IDC_BUTTON0));

        SetWindowText(hDlg, params->title);

        HFONT hfont =
            CreateFont(16, 0, 0, 0, FW_NORMAL,
                       FALSE, FALSE, FALSE, 
                       ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                        PROOF_QUALITY, FIXED_PITCH | FF_MODERN, _T("Courier New"));
                        
        SendDlgItemMessage(hDlg, IDC_MESSAGE, WM_SETFONT, (WPARAM)hfont, MAKELPARAM(TRUE,0));


        break;
      }
    case WM_COMMAND:
      {
          int choiceNumber = LOWORD(wParam) - IDC_BUTTON0;
          if ((choiceNumber >= 0) && (choiceNumber < 10)) {
              EndDialog(hDlg, choiceNumber);
              return TRUE;
          }
      }

      break;

    case WM_NCDESTROY:
        // Under SDL 1.2.6 we get a NCDESTROY message for no reason and the
        // window is immediately closed.  This is here to debug the problem.
        {int x = 1;}
        break;

    }

    return FALSE;
}

}; // namespace _internal


using namespace _internal;

/**
 * Show a dialog prompt.
 */
static int guiPrompt(
    const TCHAR*         windowTitle,
    const TCHAR*         prompt,
    const TCHAR**        choice,
    int                 numChoices) {

    int width = 280;
    int height = 128;

    const int buttonSpacing = 2;
    const int buttonWidth = 
        (width - buttonSpacing * 2 - 
          buttonSpacing * (numChoices - 1)) / numChoices;
    const int buttonHeight = 13;


    DialogTemplate dialogTemplate(
        windowTitle,
        WS_CAPTION | DS_CENTER | WS_SYSMENU,
        10, 10, width, height,
        _T("Tahoma"));

    dialogTemplate.AddEditBox(
        _T("Edit"), WS_VISIBLE | ES_READONLY | ES_OEMCONVERT | ES_MULTILINE | WS_TABSTOP, WS_EX_STATICEDGE,
        2, 2, width - 4, height - buttonHeight - 7, IDC_MESSAGE);

    int i;
    for (i = 0; i < numChoices; i++) {

        int x = buttonSpacing + i * (buttonWidth + buttonSpacing);
        int y = height - buttonHeight - buttonSpacing;

        dialogTemplate.AddButton(choice[i], WS_VISIBLE | WS_TABSTOP, 0,
                           x, y, buttonWidth, buttonHeight, IDC_BUTTON0 + i);
        
    }

    PromptParams params;
    params.message  = prompt;
    params.title    = windowTitle;

    HMODULE module = GetModuleHandle(0);
    int ret = DialogBoxIndirectParam(module, dialogTemplate, NULL, (DLGPROC) PromptDlgProc, (LPARAM)&params);


    /*
     For debugging when DialogBoxIndirectParam fails:

        // The last error value.  (Which is preserved across the call).
        DWORD lastErr = GetLastError();
    
        // The decoded message from FormatMessage
        LPCTSTR formatMsg = NULL;

        if (NULL == formatMsg) {
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                          FORMAT_MESSAGE_IGNORE_INSERTS |
                          FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            lastErr,
                            0,
                            (LPCTSTR)&formatMsg,
                            0,
                            NULL);
        }

        // Make sure the message got translated into something.
        LPCTSTR realLastErr;
        if (NULL != formatMsg) {
            realLastErr = formatMsg;
        } else {
            realLastErr = "Last error code does not exist.";
        }

        // Get rid of the allocated memory from FormatMessage.
        if (NULL != formatMsg) {
            LocalFree((LPVOID)formatMsg);
        }
        */

    return ret;
}

#endif


/**
 * Show a prompt on stdout 
 */
static int textPrompt(
    const TCHAR*         windowTitle,
    const TCHAR*         prompt,
    const TCHAR**        choice,
    int                 numChoices) {

    _tprintf(_T("\n___________________________________________________\n"));
	_tprintf(_T("%s\n"), windowTitle);
    _tprintf(_T("%s"), prompt);

    if (numChoices > 10) {
        numChoices = 10;
    }

    int c = -1;
    if (numChoices > 1) {
        _tprintf(_T("\n"));
        _tprintf(_T("Choose an option by number:"));

        while ((c < 0) || (c >= numChoices)) { 
            _tprintf(_T("\n"));

            for (int i = 0; i < numChoices; i++) {
                if (numChoices <= 3) {
                    _tprintf(_T("  (%d) %s "), i, choice[i]);
                } else {
                    _tprintf(_T("  (%d) %s\n"), i, choice[i]);
                }
            }

            _tprintf(_T("\n> "));
            c = _getch() - _T('0');

            if ((c < 0) || (c >= numChoices)) {
                _tprintf(_T("'%d' is not a valid choice."), c);
            } else {
                _tprintf(_T("%d"), c);
            }
        }
    
    } else if (numChoices == 1) {
        
        _tprintf(_T("\nPress any key for '%s'..."), choice[0]);
        _getch();
        c = 0;

    } else {

        _tprintf(_T("\nPress any key..."));
        _getch();
        c = 0;
    }

    _tprintf(_T("\n___________________________________________________\n"));
    return c;
}

#ifdef __APPLE__
static int guiPrompt(const char*         windowTitle,
                     const char*         prompt,
                     const char**        choice,
                     int                 numChoices) 
{
        NSAlert* alert;
        alert = [[NSAlert alloc] init];
        
        if (numChoices > 4){
                //If the # of choices is > 4, use standard text mode for now b/c of OS X bug:
                [alert setInformativeText: @"Prompt with more than 4 options, see the console..."];
                [alert addButtonWithTitle:@"Ok"];
                [alert runModal];
                [alert release];
                return textPrompt(windowTitle, prompt, choice, numChoices);
        }

        //Set Title:
        [alert setInformativeText: [NSString stringWithUTF8String:windowTitle]];
        
        //Set prompt:
        [alert setMessageText: [NSString stringWithUTF8String:prompt]];
        
        //Set the buttons:
        for (int i = 0; i < numChoices; i++){
                NSLog(@"About to add button number %d which is called %@", i, [NSString stringWithUTF8String:choice[i]]); 
                [[alert addButtonWithTitle:[NSString stringWithUTF8String:choice[i]]] setTag: i];
        }
        int toReturn = [alert runModal];
        NSLog(@"Returning : %d", toReturn);
        [alert release];
        return toReturn;
}
#endif

int prompt(
    const TCHAR*      windowTitle,
    const TCHAR*      prompt, 
    const TCHAR**     choice,
    int              numChoices,
    bool             useGui) {

    #ifdef WIN32
        if (useGui) {
            // Build the message box
            return guiPrompt(windowTitle, prompt, choice, numChoices);
        }
    #endif
    
        #ifdef __APPLE__
                if (useGui){
                        //Will default to text prompt if numChoices > 4
                        return guiPrompt(windowTitle, prompt, choice, numChoices);
                }
        #endif
    return textPrompt(windowTitle, prompt, choice, numChoices);
}

#ifdef UNICODE
void msgBox(
    const std::wstring& message,
    const std::wstring& title) {

    const wchar_t *choice[] = {L"Ok"};
    prompt(title.c_str(), message.c_str(), choice, 1, true); 
}
#else
void msgBox(
    const std::string& message,
    const std::string& title) {

    const CHAR *choice[] = {"Ok"};
    prompt(title.c_str(), message.c_str(), choice, 1, true); 
}
#endif
#ifndef WIN32
    #undef _getch
#endif

};// namespace

#endif