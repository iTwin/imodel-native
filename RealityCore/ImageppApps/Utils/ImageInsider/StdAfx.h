/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/StdAfx.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <Imagepp/h/ImageppAPI.h>

#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HFCException.h>
USING_NAMESPACE_IMAGEPP

