/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeIconUtilities.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
    #include <windows.h>
    #include <shellapi.h>
#elif defined (__unix__)
#else
    #error unknown compiler
#endif

#include <Bentley/BeIconUtilities.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int BeIconUtilities::LookupIconIdFromDirectory (IconDir const* iconDir, int width, int height)
    {
#if defined (BENTLEY_WIN32)

    return ::LookupIconIdFromDirectoryEx ((Byte*)iconDir, true, width, height, LR_DEFAULTCOLOR);
    
#elif defined (BENTLEY_WINRT)

    // *** WIP_NONPORT
    return 0;

#elif defined (__unix__)

    // *** WIP_NONPORT
    return 0;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeIconUtilities::IconP BeIconUtilities::CreateIconFromResource (uint8_t* iconData, int size, int width, int height)
    {
#if defined (BENTLEY_WIN32)

    //  If the flag is set to LR_DEFAULTSIZE (and CreateIconFromResource uses this)
    //  CreateIconFromResourceEx may change the size
    return (IconP) ::CreateIconFromResourceEx (iconData, size, true, 0x00030000, width, height, 0);
    
#elif defined (BENTLEY_WINRT)

    // *** WIP_NONPORT
    return 0;

#elif defined (__unix__)

    // *** WIP_NONPORT
    return 0;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeIconUtilities::IconP BeIconUtilities::CreateIconForFileName (WCharCP filename, int width, int height)
    {
#if defined (BENTLEY_WIN32)

    IconP       iconP = 0;
    SHFILEINFOW shFileInfo;
    if (0 != ::SHGetFileInfoW (filename, FILE_ATTRIBUTE_NORMAL, &shFileInfo, sizeof (shFileInfo), SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_SMALLICON))
        iconP = (IconP)shFileInfo.hIcon;

    return iconP;

#else

    // *** WIP_NONPORT
    return 0;

#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeIconUtilities::IconP BeIconUtilities::BeLoadIcon (IconSourceP hInstance, int rscId)
    {
    return BeLoadIcon(hInstance, rscId, 0, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeIconUtilities::IconP BeIconUtilities::BeLoadIcon (IconSourceP hInstance, int rscId, int desiredWidth, int desiredHeight)
    {
#if defined (BENTLEY_WIN32)

    return (BeIconUtilities::IconP) ::LoadImage ((HINSTANCE)hInstance, MAKEINTRESOURCE(rscId), IMAGE_ICON, desiredWidth, desiredHeight, LR_CREATEDIBSECTION);

#elif defined (BENTLEY_WINRT)

    // *** WIP_NONPORT
    return 0;

#elif defined (__unix__)

    // *** WIP_NONPORT
    return 0;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeIconUtilities::DestroyIcon (IconP icon)
    {
#if defined (BENTLEY_WIN32)

    if (NULL != icon)
        ::DestroyIcon ((HICON)icon);

    return SUCCESS;

#elif defined (BENTLEY_WINRT)

    // *** WIP_NONPORT
    return ERROR;

#elif defined (__unix__)

    // *** WIP_NONPORT
    return ERROR;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeIconUtilities::GetIconSizeInfo (int32_t& hotSpotx, int32_t& hotSpoty, int32_t& sizex, int32_t& sizey, IconP icon)
    {
#if defined (BENTLEY_WIN32)

    ICONINFO    iconInfo;
    if (0 == ::GetIconInfo ((HICON)icon, &iconInfo))  // stupidly, 0 means error
        return ERROR;

    BITMAP  bitmap;
    if (0 == ::GetObject (iconInfo.hbmColor, sizeof(bitmap), &bitmap))
        {
        DeleteObject (iconInfo.hbmColor);
        DeleteObject (iconInfo.hbmMask);
        return ERROR;
        }

    hotSpotx = iconInfo.xHotspot;
    hotSpoty = iconInfo.yHotspot;
    sizex    = bitmap.bmWidth;
    sizey    = bitmap.bmHeight;

    DeleteObject (iconInfo.hbmColor);
    DeleteObject (iconInfo.hbmMask);

    return SUCCESS;

#elif defined (BENTLEY_WINRT)

    // *** WIP_NONPORT
    return ERROR;

#elif defined (__unix__)

    // *** WIP_NONPORT
    return ERROR;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeIconUtilities::GetDIBits (bvector<uint32_t>& iconBits, bvector<uint32_t>& iconMask, IconP icon, void* hdc)
    {
#if defined (BENTLEY_WIN32)

    ICONINFO    iconInfo;
    if ((NULL == hdc) || 0 == GetIconInfo ((HICON)icon, &iconInfo))  // 0 means error
        return ERROR;

    BITMAP  bitmap;
    if (0 == ::GetObject (iconInfo.hbmColor, sizeof(bitmap), &bitmap))
        {
        DeleteObject (iconInfo.hbmColor);
        DeleteObject (iconInfo.hbmMask);
        return ERROR;
        }

    int sizex    = bitmap.bmWidth;
    int sizey    = bitmap.bmHeight;

    BITMAPINFOHEADER  iconBmInfo;
    iconBmInfo.biSize          = sizeof iconBmInfo;
    iconBmInfo.biWidth         = sizex;
    iconBmInfo.biHeight        = -sizey;
    iconBmInfo.biPlanes        = 1;
    iconBmInfo.biBitCount      = 32;
    iconBmInfo.biCompression   = BI_RGB;
    iconBmInfo.biSizeImage     = 0;
    iconBmInfo.biXPelsPerMeter = 1024;
    iconBmInfo.biYPelsPerMeter = 1024;
    iconBmInfo.biClrUsed       = 0;
    iconBmInfo.biClrImportant  = 0;

    int   size = sizex * sizey;
    iconBits.resize (size);
    iconMask.resize (size);

    if (0 == ::GetDIBits ((HDC)hdc, iconInfo.hbmColor, 0, sizey, &iconBits[0], (LPBITMAPINFO) &iconBmInfo, DIB_RGB_COLORS))
        return  ERROR;

    if (0 == ::GetDIBits ((HDC)hdc, iconInfo.hbmMask, 0, sizey, &iconMask[0], (LPBITMAPINFO) &iconBmInfo, DIB_RGB_COLORS))
        return  ERROR;

    DeleteObject (iconInfo.hbmColor);
    DeleteObject (iconInfo.hbmMask);
    return  SUCCESS;

#elif defined (BENTLEY_WINRT)

    // *** WIP_NONPORT
    return ERROR;

#elif defined (__unix__)

    // *** WIP_NONPORT
    return ERROR;

#else
#error unknown runtime
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void* BentleyApi::BeGetBinaryResourceSource (void* addr)
    {
#if defined (BENTLEYCONFIG_OS_WINDOWS)
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery (addr, &mbi, sizeof mbi);
    return (void*)mbi.AllocationBase;
#elif defined (__unix__)
    // WIP_NONPORT
    return NULL;
#endif
    }

