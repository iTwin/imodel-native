/**
  @file debugAssert.h
 
  debugAssert(expression);
  debugAssertM(expression, message);
 
  @cite
     John Robbins, Microsoft Systems Journal Bugslayer Column, Feb 1999.
     <A HREF="http://msdn.microsoft.com/library/periodic/period99/feb99_BUGSLAYE_BUGSLAYE.htm">
     http://msdn.microsoft.com/library/periodic/period99/feb99_BUGSLAYE_BUGSLAYE.htm</A>
 
  @cite 
     Douglas Cox, An assert() Replacement, Code of The Day, flipcode, Sept 19, 2000
     <A HREF="http://www.flipcode.com/cgi-bin/msg.cgi?showThread=COTD-AssertReplace&forum=cotd&id=-1">
     http://www.flipcode.com/cgi-bin/msg.cgi?showThread=COTD-AssertReplace&forum=cotd&id=-1</A>
 
  @maintainer Morgan McGuire, matrix@graphics3d.com
 
  @created 2001-08-26
  @edited  2004-01-02

 Copyright 2000-2004, Morgan McGuire.
 All rights reserved.
 */

#ifndef PT_DEBUGASSERT_H
#define PT_DEBUGASSERT_H


#ifndef NEEDS_WORK_VORTEX_DGNDB

#define debugAssert(exp)            BeAssert(exp)
#define debugAssertM(exp, msg)      BeAssert(msg && exp);
#define alwaysAssertM(exp, msg)     BeAssert(msg && exp);

#else


#include <string>
#include <pt/ptunicode.h>
#include <pt/classes.h>

#ifdef __linux__
    // Needed so we can define a global display
    // pointer for debugAssert.
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/Xatom.h>
#endif

/**
 @def debugBreak()
 
 Break at the current location (i.e. don't push a procedure stack frame
 before breaking).
 */

/**
  @def debugAssert(exp)
  Breaks if the expression is false. If G3D_DEBUG_NOGUI is defined, prompts at
  the console, otherwise pops up a dialog.  The user may then break (debug), 
  ignore, ignore always, or halt the program.
 
  The assertion is also posted to the clipboard under Win32.
 */

/**
  @def debugAssertM(exp, msg)
  Breaks if the expression is false and displays a message. If G3D_DEBUG_NOGUI 
  is defined, prompts at the console, otherwise pops up a dialog.  The user may
  then break (debug), ignore, ignore always, or halt the program.
 
  The assertion is also posted to the clipboard under Win32.
 */

namespace pt {
typedef bool (*AssertionHook)(
    const TCHAR* _expression,
    const std::tstring& message,
    const TCHAR* filename,
    int lineNumber,
    bool& ignoreAlways,
    bool useGuiPrompt);


/** 
  Allows customization of the global function invoked when a debugAssert fails.
  The initial value is pt::_internal::_handleDebugAssert_.  G3D will invoke
  rawBreak if the hook returns true.  If NULL, assertions are not handled.
*/
void setAssertionHook(AssertionHook hook);

namespace _internal {
    extern AssertionHook CCLASSES_API _debugHook;
} // internal
} // G3D

/**
 @def __debugPromptShowDialog__
 @internal
 */

#ifdef _DEBUG

    #ifndef __APPLE__
        #ifdef _MSC_VER
			#ifdef _WIN64
				#define rawBreak() assert(false);
			#else
				#define rawBreak()  _asm { int 3 }
			#endif
		#else
            #define rawBreak() __asm__ __volatile__ ( "int $3" ); 
        #endif
    #else
        #define rawBreak() assert(false); /* No breakpoints on OS X yet */
    #endif


    #define debugBreak() pt::_internal::_releaseInputGrab_(); rawBreak(); pt::_internal::_restoreInputGrab_();
    #define debugAssert(exp) debugAssertM(exp, _T("Debug assertion failure"))

    #ifdef PT_DEBUG_NOGUI
        #define __debugPromptShowDialog__ false
    #else
        #define __debugPromptShowDialog__ true
    #endif
#ifndef _M_X64
    #define debugAssertM(exp, message) { \
        static bool __debugAssertIgnoreAlways__ = false; \
        if (!__debugAssertIgnoreAlways__ && !(exp)) { \
            pt::_internal::_releaseInputGrab_(); \
            if ((pt::_internal::_debugHook != NULL) && \
                pt::_internal::_debugHook(_T(#exp), message, _T(__FILE__), __LINE__, __debugAssertIgnoreAlways__, __debugPromptShowDialog__)) { \
                 rawBreak(); \
            } \
            pt::_internal::_restoreInputGrab_(); \
        } \
    }
#else
	#define debugAssertM(exp, message) assert(exp); 
#endif
    #define alwaysAssertM debugAssertM

#else  // Release
    #ifdef PT_DEBUG_NOGUI
        #define __debugPromptShowDialog__ false
    #else
        #define __debugPromptShowDialog__ true
    #endif

    // In the release build, just define away assertions.
    #define rawBreak()
    #define debugAssert(exp)
    #define debugAssertM(exp, message)
    #define debugBreak()

    // But keep the always assertions
    #define alwaysAssertM(exp, message) { \
        if (!(exp)) { \
            pt::_internal::_releaseInputGrab_(); \
            pt::_internal::_handleErrorCheck_(_T(#exp), message, _T(__FILE__), __LINE__, __debugPromptShowDialog__); \
            exit(-1); \
        } \
    }

#endif  // if debug



namespace pt {  namespace _internal {

#ifdef __linux__
    /**
     A pointer to the X11 display.  Initially NULL.  If set to a
     non-null value (e.g. by SDLWindow), debugAssert attempts to use
     this display to release the mouse/input grab when an assertion
     fails.
     */
    extern Display*      x11Display;

    /**
     A pointer to the X11 window.  Initially NULL.  If set to a
     non-null value (e.g. by SDLWindow), debugAssert attempts to use
     this window to release the mouse/input grab when an assertion
     fails.
     */
    extern Window        x11Window;
#endif

/**
 Pops up an assertion dialog or prints an assertion

 ignoreAlways      - return result of pressing the ignore button.
 useGuiPrompt      - if true, shows a dialog
 */
bool CCLASSES_API _handleDebugAssert_(
    const TCHAR* expression,
    const std::tstring& message,
    const TCHAR* filename,
    int         lineNumber,
    bool&       ignoreAlways,
    bool        useGuiPrompt);

void CCLASSES_API _handleErrorCheck_(
    const TCHAR* expression,
    const std::tstring& message,
    const TCHAR* filename,
    int         lineNumber,
    bool        useGuiPrompt);

/** Attempts to give the user back their mouse and keyboard if they 
    were locked to the current window.  
    @internal*/
void CCLASSES_API _releaseInputGrab_();

/** Attempts to restore the state before _releaseInputGrab_.  
    @internal*/
void CCLASSES_API  _restoreInputGrab_();

}; }; // namespace

#endif

#endif
