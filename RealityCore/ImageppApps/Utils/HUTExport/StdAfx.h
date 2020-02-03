//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/StdAfx.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__7D8B490D_A980_11D2_8400_0060082DE76F__INCLUDED_)
#define AFX_STDAFX_H__7D8B490D_A980_11D2_8400_0060082DE76F__INCLUDED_

#pragma once

#ifndef WINVER
#define WINVER 0x0501
#endif

#define VC_EXTRALEAN        // Exclude rarely-used stuff from Windows headers

#if defined(WIN32_LEAN_AND_MEAN)
#undef WIN32_LEAN_AND_MEAN      // atldef.h will define it
#endif

#pragma warning(push)
#pragma warning(disable:4266)   //  MFC doesn't pass this warning

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>            // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxtempl.h>
#pragma warning(pop)

// **************************
// NO! With a static lib configuration of HUTExportlib, it breaks our activeXPro regsvr32. 
// We might need this include if we ever create a dll version of HUTExportlib but right now we only have a static version.
// #include <afxdllx.h>
// **************************

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

// Platform SDK
#include <shlobj.h>

// Ipp
#include <ImagePP/h/ImageppAPI.h>
USING_NAMESPACE_IMAGEPP

#include <ImagePP/all/h/HRFImportExport.h>
#include <ImagePP/all/h/HTiffTag.h>


#endif // !defined(AFX_STDAFX_H__7D8B490D_A980_11D2_8400_0060082DE76F__INCLUDED_)
