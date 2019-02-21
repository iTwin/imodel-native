/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BentleyManagedUtil/StringInterop.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#pragma warning (disable:4400)

#if !defined (_MANAGED)
#error This header is intended to be included only in a managed region of a /CLR compiland.
#endif

#include <gcroot.h>

#pragma unmanaged
#include <BaseTsd.h>            // Windows Platform SDK header file, defines INT_PTR
#pragma managed

#if defined (__cplusplus_cli)
#define STRINGH System::String^
#define _NULLPTR_ nullptr
#else
#define STRINGH System::String*
#define _NULLPTR_ NULL
#endif


namespace Bentley { namespace Interop {
/*=================================================================================**//**
*
* ScopedString.
* Do not use this class merely to get a native string ptr, but instead use
* pin_ptr<const wchar_t> nativeString = PtrToStringChars(managedString);
* wherever possible
*
* @bsiclass                                                     Bern.McCarty   05/04
+===============+===============+===============+===============+===============+======*/

#if defined (__cplusplus_cli)
ref class ScopedString
#else
class ScopedString
#endif
    {
private:
    bool                isNull;
#if defined (__cplusplus_cli)
    STRINGH             managedString;
#else
    gcroot<STRINGH>     managedString;
#endif
    INT_PTR             nativeAnsiString;
    wchar_t*            nativeUCS2String;


#if defined (__cplusplus_cli)
ScopedString () {};
#endif

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BernMcCarty   04/04
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedString (STRINGH managedString)
    {
    nativeAnsiString = 0;
    nativeUCS2String = NULL;

    if (_NULLPTR_ == managedString)
        {
        isNull = true;
        }
    else
        {
        isNull = false;
        this->managedString = managedString;
        }
    }

#if defined (__cplusplus_cli)
/*---------------------------------------------------------------------------------**//**
* User-defined copy constructor required for ref class
* @bsimethod                                                    CaseyMullen   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedString (const ScopedString^ scopedString)
    {
    nativeAnsiString = 0; // Do not copy any Ansi or Uni strings that have been generated. Who would free them?
    nativeUCS2String = NULL;
    isNull           = scopedString->isNull;
    managedString    = scopedString->managedString;
    }

/*---------------------------------------------------------------------------------**//**
* User-defined copy constructor required for ref class
* @bsimethod                                                    CaseyMullen   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ScopedString (const ScopedString% scopedString)
    {
    nativeAnsiString = 0; // Do not copy any Ansi or Uni strings that have been generated. Who would free them?
    nativeUCS2String = NULL;
    isNull           = scopedString.isNull;
    managedString    = scopedString.managedString;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BernMcCarty   04/04
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ~ScopedString()
    {
    if (isNull)
        return;

    if (this->nativeAnsiString)
        System::Runtime::InteropServices::Marshal::FreeHGlobal(static_cast<System::IntPtr>(this->nativeAnsiString));

    if (this->nativeUCS2String)
        System::Runtime::InteropServices::Marshal::FreeHGlobal(static_cast<System::IntPtr>(this->nativeUCS2String));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Trefz   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
UINT32 GetLength(void)
    {
    if (isNull)
        return 0;

    return managedString->Length;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BernMcCarty   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
System::IntPtr AnsiAsIntPtr(void)
    {
    if (isNull)
        return static_cast <System::IntPtr>(0);

    if (0 == this->nativeAnsiString)
        this->nativeAnsiString = static_cast<INT_PTR>(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(managedString));

    return static_cast<System::IntPtr>(this->nativeAnsiString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BernMcCarty   04/04
+---------------+---------------+---------------+---------------+---------------+------*/
char * Ansi(void)
    {
    if (isNull)
        return NULL;

    return static_cast<char *>(this->AnsiAsIntPtr().ToPointer());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BernMcCarty   04/04
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t * Uni(void)
    {
    if (isNull)
        return NULL;

    if (NULL == this->nativeUCS2String)
        this->nativeUCS2String = static_cast<wchar_t*>(System::Runtime::InteropServices::Marshal::StringToHGlobalUni(managedString).ToPointer());

    return this->nativeUCS2String;
    }

};

} }

