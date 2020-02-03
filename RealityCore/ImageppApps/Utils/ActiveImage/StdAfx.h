/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/StdAfx.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef WINVER
#define WINVER 0x0501
#endif

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
 
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#include <afxdialogex.h>
#include <afxcontrolbars.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include <afxtempl.h>
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <atlimage.h>

#define WM_BUTTON_MOUSE_RCLICKED   WM_USER + 100

#undef GetObject

// Fix a runtime error truncated value
#undef GetGValue
#define GetGValue(rgb) (LOBYTE((rgb) >> 8))

#include <Imagepp/h/ImageppAPI.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFPageDescriptor.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRPMapFilters8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV32PRPhotoYCCA8.h>
#include <Imagepp/all/h/HGF2DGrid.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRARaster.h>
#include <Imagepp/all/h/HRAStoredRaster.h>
#include <Imagepp/all/h/HCDPacket.h>

#include <Imagepp/all/h/HFCCallback.h>
#include <Imagepp/all/h/HFCCallbacks.h>
#include <Imagepp/all/h/HRFPDFFile.h>
#include <Imagepp/all/h/HRFVirtualEarthFile.h>
#include <ImagePP/all/h/HRFCacheFileCreator.h>
#include <ImagePP/all/h/HRFImportExport.h>


USING_NAMESPACE_BENTLEY
USING_NAMESPACE_IMAGEPP



