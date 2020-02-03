/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageHunter/HuntTools.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageHunter/HuntTools.cpp,v 1.2 2010/08/27 18:54:32 Jean.Lalande Exp $
//-----------------------------------------------------------------------------
// Methods for class HuntTools
//-----------------------------------------------------------------------------

#include "Stdafx.h"
#include "HuntTools.h"

using namespace System; 
using namespace System::Collections;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jean.Lalande      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void HuntTools::MarshalString( String ^ s, WString& os ) 
    {
    using namespace Runtime::InteropServices;
    wchar_t* chars = (wchar_t*)(Marshal::StringToHGlobalUni(s)).ToPointer();

    os = chars;

    Marshal::FreeHGlobal((IntPtr)chars);
    }