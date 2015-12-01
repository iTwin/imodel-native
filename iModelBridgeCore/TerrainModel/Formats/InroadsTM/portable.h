//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* portable.h                                                                 */
/*----------------------------------------------------------------------------*/
/* This file contains standard MFC includes, standard C/C++ library includes, */
/* MicroStation MDL prototype includes, and ANSI function redefinitions.      */
/* Each StdAfx.h file in the product should include this file.                */
/*----------------------------------------------------------------------------*/
#pragma once

//-------------------------------------------------------------------
// Standard Bentley defines
// http://bsw-wiki.bentley.com/default.aspx/Development/CompilingMicroStationAppsWithVCPROJ.html
//-------------------------------------------------------------------

#undef _DEBUG

#ifndef _SECURE_SCL
#define _SECURE_SCL 0
#endif

//-------------------------------------------------------------------
// Standard C/C++ Header Files
//-------------------------------------------------------------------

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iosfwd>
#include <string>
#include <vector>
#include <map>
#include <time.h>
#include <mbstring.h>

#ifdef _AFXDLL
//-------------------------------------------------------------------
// MFC Header Files
//-------------------------------------------------------------------

#ifndef _UNICODE
#define _UNICODE                        // We are a UNICODE only app
#endif

#define VC_EXTRALEAN                    // Exclude rarely-used stuff from Windows headers

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif


#include <afxwin.h>                     // MFC core and standard components
#include <afxext.h>                     // MFC extensions
#include <afxtempl.h>                   // MFC template classes
#include <commctrl.h>                   // MFC common control classes
#include <afxcmn.h>                     // MFC common control classes
#include <dlgs.h>                       // Windows dialog control ids
#include <afxole.h>
#endif

//-------------------------------------------------------------------
// MicroStation Header Files
//-------------------------------------------------------------------

#include <Bentley\Bentley.h>
#include <Bentley\stg\guid.h>
#include <Geom/GeomApi.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/DateTime.h>
#include <Bentley/BeFilename.h>

using namespace Bentley;

//-------------------------------------------------------------------
// Standard product pragmas
//-------------------------------------------------------------------
#pragma warning(error:4311 4302)

//-------------------------------------------------------------------
// Standard product definitions
//-------------------------------------------------------------------

#ifndef DEFAULT_PREFERENCE_NAME
#define DEFAULT_PREFERENCE_NAME     L"Default"
#endif

#define DEFAULT_OBJECT_NAME DEFAULT_PREFERENCE_NAME

// ------------- DH Added -------------------
#ifndef _AFXDLL
typedef short WORD;
typedef long DWORD;
typedef unsigned int UINT;
typedef bool BOOL;
typedef bool boolean;
#define LPWSTR WCharP
#define LPCWSTR WCharCP
#define FALSE false
#define TRUE true
#define HRESULT long
#define LPVOID void*
#define S_OK 0
typedef unsigned long ULONG;
#define MB_OKCANCEL 1
#define MB_OK 0
#define ASSERT BeAssert
#define AFXAPI
typedef struct _SYSTEMTIME
    {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
    } SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

struct __POSITION {};
typedef __POSITION* POSITION;

struct CString : WString
    {
    CString ()
        { }
    CString (WCharCP l) : WString (l)
        {
        }
    CString (WStringCR l) : WString (l)
        { }
    bool IsEmpty ()
        {
        return empty ();
        }
    operator WCharCP() const throw()
        {
        return GetWCharCP ();
        }
    void LoadString (int)
        { }
    void Format (WCharCP format, ...)
        {
        va_list     ap;

        va_start (ap, format);
        VSprintf (format, ap);
        va_end (ap);
        }

    int CompareNoCase (WCharCP value)
        {
        return CompareToI (value);
        }

    };
template <class A, class AR, class B, class BR> class CMap : public bmap<A, B>
    {
    public: void InitHashTable (int) {}
            void RemoveAll () { clear (); }
            void SetAt (A key, B value)
                {
                (*this)[key] = value;
                }
            bool Lookup (A key, BR value) const
                {
                auto i = this->find (key);
                if (i == this->end ())
                    return false;
                value = i->second;
                return true;
                }
            void RemoveKey (A key)
                {
                this->erase (key);
                }
            int GetCount ()
                {
                return (int)size ();
                }
            void GetNextAssoc (
                POSITION& rNextPosition,
                A& rKey,
                B& rValue
                ) const
                {
                iterator* nextPos = (iterator*)rNextPosition;
                if ((*nextPos) == end ())
                    {
                    delete nextPos;
                    rNextPosition = nullptr;
                    }
                else
                    {
                    rKey = (*nextPos)->first;
                    rValue = (*nextPos)->second;
                    (*nextPos)++;
                    }
                }
            POSITION GetStartPosition ()
                {
                return (POSITION)new iterator (begin ());
                }
    };

class CMapPtrToPtr : public CMap < void*, void*&, void*, void*&>
    {
    };

class CMapStringToPtr : public CMap < CString, CString&, void*, void*&>
    {
    };

class CMapStringToString : public CMap<CString, CString&, CString, CString&>
    {
    };

template <class A> class CArray : public bvector<A>
    {
    public:
        A GetAt (int i)
            {
            return (*this)[i];
            }
        void RemoveAt (int i)
            {
            this->erase (begin()+i);
            }
        void Add (A value)
            {
            this->push_back (value);
            }
        int GetCount ()
            {
            return (int)this->size ();
            }
    };

class CPtrArray :public CArray<void*>
    { };

#define _tcscpy wcscpy
#define _T(a) L a
#define TEXT(a) a
#define LCID long
inline void GetUserNameW (WCharP l, unsigned long* size)
    {
    WString name;
    BeGetUserName (name);
    BeStringUtilities::Wcsncpy (l, 256, name.GetWCharCP ());
    *size = (int)name.size ();
    }
inline void GetLocalTime (SYSTEMTIME* pSystemTime)
    {
    Int64 now = BeTimeUtilities::GetCurrentTimeAsUnixMillis ();
    tm nowTM;
    BeTimeUtilities::ConvertUnixMillisToLocalTime (nowTM, now);
    pSystemTime->wYear = (WORD)(nowTM.tm_year + 1900);
    pSystemTime->wMonth = (WORD)(nowTM.tm_mon + 1);
    pSystemTime->wDayOfWeek = (WORD)nowTM.tm_wday;
    pSystemTime->wDay = (WORD)nowTM.tm_mday;
    pSystemTime->wHour = (WORD)nowTM.tm_hour;
    pSystemTime->wMinute = (WORD)nowTM.tm_min;
    pSystemTime->wSecond = (WORD)nowTM.tm_sec;
    pSystemTime->wMilliseconds = (WORD)(now % 1000);
    }
inline void GetTimeFormatW (LCID lcid, DWORD l, const SYSTEMTIME* t, void*, WCharP cTime, int size)
    {
    tm tm;
    tm.tm_year = t->wYear;
    tm.tm_mon = t->wMonth;
    tm.tm_wday = t->wDayOfWeek;
    tm.tm_mday = t->wDay;
    tm.tm_hour = t->wHour;
    tm.tm_min = t->wMinute;
    tm.tm_sec = t->wSecond;
    WString timeString;
    BeTimeUtilities::UnixMillisToString (&timeString, nullptr, BeTimeUtilities::ConvertTmToUnixMillisDouble (tm));
    BeStringUtilities::Wcsncpy (cTime, size, timeString.GetWCharCP ());
    }

inline void GetDateFormatW (LCID lcid, DWORD l, const SYSTEMTIME* t, void*, WCharP cDate, int size)
    {
    tm tm;
    tm.tm_year = t->wYear;
    tm.tm_mon = t->wMonth;
    tm.tm_wday = t->wDayOfWeek;
    tm.tm_mday = t->wDay;
    tm.tm_hour = t->wHour;
    tm.tm_min = t->wMinute;
    tm.tm_sec = t->wSecond;
    WString dateString;
    BeTimeUtilities::UnixMillisToString (nullptr, &dateString, BeTimeUtilities::ConvertTmToUnixMillisDouble (tm));
    BeStringUtilities::Wcsncpy (cDate, size, dateString.GetWCharCP ());
    }

#define FILE_ATTRIBUTE_READONLY 0

inline DWORD GetFileAttributes (WCharCP file)
    {
    if (BeFileNameStatus::Success != BeFileName::CheckAccess (file, BeFileNameAccess::Write))
        return FILE_ATTRIBUTE_READONLY;
    return 0;
    }
#endif

#define MAX_LEVEL_NAME_BYTES 1024
#define MAX_MaterialName 30
#define MAX_FONT_NAME_BYTES 1024
#define MAX_TEXTSTYLE_NAME_LENGTH 512

#define ACTIONBUTTON_CANCEL 4
#define ACTIONBUTTON_OK 3
#define ACTIONBUTTON_NO 7

#define MAX_VERTICES 5000

inline double mdlVec_distanceXY (DPoint3d* v1, DPoint3d* v2)
    {
    return v1->DistanceXY (*v2);
    }

inline double mdlVec_distance (DPoint3d* v1, DPoint3d* v2)
    {
    return v1->Distance (*v2);
    }