/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/IRasterSourceFileQuery.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/** @addtogroup RasterElements */
/** @beginGroup */

//=======================================================================================
//! Interface that provides methods to query raster file information.
// @bsiclass                                                       Marc.Bedard    07/2010
struct  IRasterSourceFileQuery : public IRefCounted
{
private:

protected:
    DGNPLATFORM_EXPORT virtual uint64_t         _GetBitmapHeight () const = 0;
    DGNPLATFORM_EXPORT virtual uint64_t         _GetBitmapWidth  () const = 0;
    DGNPLATFORM_EXPORT virtual BentleyStatus    _ReadToRGBA (Byte *RgbaBuffer, size_t maxBufferLength, bool useBgIfNoAlpha) const=0;

    //__PUBLISH_CLASS_VIRTUAL__
public:

    //! Query bitmap height in pixels.
    //! @return bitmap height in pixels.
    DGNPLATFORM_EXPORT uint64_t            GetBitmapHeight () const;

    //! Query bitmap width in pixels.
    //! @return bitmap width in pixels.
    DGNPLATFORM_EXPORT uint64_t            GetBitmapWidth  () const;

    //! Fill a buffer with RGBA data read from a raster file. If the file is not RGBA, original data read will be converted to RGBA
    //! @param[in,out] RgbaBuffer       The buffer to be filled. Caller must allocate the buffer before calling the method.
    //! @param[in]     maxBufferLength  The size of the input buffer.
    //! @param[in]     useBgIfNoAlpha   If True and there is no alpha channel present in file, will create alpha based on background color. Use first pixel as background color if no background defined in file.
    //! @return BSISUCCESS or BSIERROR.
    DGNPLATFORM_EXPORT BentleyStatus       ReadToRGBA (Byte *RgbaBuffer, size_t maxBufferLength, bool useBgIfNoAlpha) const;
};

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
