/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeIconUtilities.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "Bentley.h"
#include "bvector.h"

BEGIN_BENTLEY_NAMESPACE

struct BeIconUtilities
{
//! Platform-specific icon
struct Icon {;};
typedef Icon* IconP;
//! Platform-specific icon source
struct IconSource {;};
typedef IconSource* IconSourceP;

// #pragmas are used here to ensure that the structure's packing in memory matches the packing of the EXE or DLL.
#pragma pack (push)
#pragma pack (2)

//! Platform-independent binary icon resource layout (originally copied from Wind32)
struct IconDirEntry
    {
    uint8_t bWidth;                 // Width, in pixels, of the image
    uint8_t bHeight;                // Height, in pixels, of the image
    uint8_t bColorCount;            // Number of colors in image (0 if >=8bpp)
    uint8_t bReserved;              // Reserved
    uint16_t wPlanes;                // Color Planes
    uint16_t wBitCount;              // Bits per pixel
    uint32_t dwBytesInRes;           // how many bytes in this resource?
    uint16_t nID;                    // the ID
    };
struct IconDir
    {
    uint16_t     idReserved;        // Reserved (must be 0)
    uint16_t     idType;            // Resource type (1 for icons)
    uint16_t     idCount;           // How many images?
    IconDirEntry idEntries[1];      // The entries for each image
    };
#pragma pack (pop) // #pragma pack (2)

//! Platform-independent bitmap layout (originally copied from Win32)
struct BitMapInfoHeader
    {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
    };

struct RGBQuad
    {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
    };

//! Searches through icon or cursor data for the icon or cursor that best fits the current display device.
//! @param iconDir  a list of icons
//! @param width    desired icon width
//! @param height   desired icon height
//! @return The index of the selected icon
BENTLEYDLL_EXPORT static int LookupIconIdFromDirectory (IconDir const* iconDir, int width, int height);

//! Create an icon from stored binary data
//! @param iconData the icon definition data to load
//! @param size     the number of bytes of icon definition data
//! @param width    desired icon width
//! @param height   desired icon height
//! @return an icon object or NULL if the input data is invalid
BENTLEYDLL_EXPORT static IconP CreateIconFromResource (uint8_t* iconData, int size, int width, int height);

//! Create an appropriate icon to represent a file based on its filename.
//! @param filename the filename for which an icon is being requested
//! @param width    the desired icon width
//! @param height   the desired icon height
//! @return an icon object, or NULL if no suitable icon could be found
BENTLEYDLL_EXPORT static IconP CreateIconForFileName (WCharCP filename, int width, int height);

//! Get the size and hotspot location of an icon
//! @param[out] hotspot the hotspot location
//! @param[out] size    the size
//! @return SUCCESS if the operation is supported
BENTLEYDLL_EXPORT static BentleyStatus GetIconSizeInfo (int32_t& hotspotx, int32_t& hotspoty, int32_t& sizex, int32_t& sizey, IconP icon);

//! Get the bits of an icon
//! @param[out] iconBits    the icon's bits
//! @param[out] iconMask    the icon's mask
//! @param[in]  icon        the icon
//! @param[in]  hdc         a device context for use in extraction
//! @return SUCCESS if the operation is supported
BENTLEYDLL_EXPORT static BentleyStatus GetDIBits (bvector<uint32_t>& iconBits, bvector<uint32_t>& iconMask, IconP icon, void* hdc);

//! Load an icon
//! @param hInstance    identifies the source where the icon is stored
//! @param rscId        identifies the icon resource within the source
//! @return an icon object or NULL if not found
BENTLEYDLL_EXPORT static IconP BeLoadIcon (IconSourceP hInstance, int rscId);

//! Load an icon
//! @param hInstance    identifies the source where the icon is stored
//! @param rscId        identifies the icon resource within the source
//! @param desiredWidth The width, in pixels, of the icon or cursor.
//! @param desiredHeight The height, in pixels, of the icon or cursor.
//! @return an icon object or NULL if not found
BENTLEYDLL_EXPORT static IconP BeLoadIcon (IconSourceP hInstance, int rscId, int desiredWidth, int desiredHeight);

//! Discard an icon
//! @param icon         the icon to be destroyed
//! @return SUCCESS if the operation is supported
BENTLEYDLL_EXPORT static BentleyStatus DestroyIcon (IconP icon);

}; // BeIconUtilities

//! Get an HINSTANCE for the specified DLL
BENTLEYDLL_EXPORT void* BeGetBinaryResourceSource (void* addr);

END_BENTLEY_NAMESPACE
