/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/IDwgContext.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Interface that provides context information when translating from and to DWG text. Applications can use this interface to modify parameters during conversion between and to DWG text to and from a textblock.
// @bsiclass                                                    Venkat.Kalyan   11/04
//=======================================================================================
struct  IDwgContext
    {
    //! Converts the input dwg color index, usually found it the markup to DGN Color.
    //! @param[in]  dwgColorIndex   input DWG color index
    //! @return output DGN color index
    public: virtual UInt32 DwgColorToDgnColor (UInt32 dwgColorIndex) const = 0;

    //! Converts DGN color to DWG color index.
    //! @param[in]  dgnColorIndex   input DGN color index
    //! @return output dwg color index
    public: virtual UInt32 DgnColorToDwgColor (UInt32 dgnColorIndex) const = 0;

    //! Extracts the default DGN color index
    //! @return output default DGN color index
    public: virtual UInt32 GetDefaultDGNColor () const = 0;

    public: virtual UInt32 GetDefaultByBlockColor () const = 0;

    }; // IDwgContext

END_BENTLEY_DGNPLATFORM_NAMESPACE
