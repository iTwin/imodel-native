/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ImageUtilities.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/BeFile.h>
#include <Bentley/bvector.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/16
//=======================================================================================
struct RgbImageInfo
{
    enum class BottomUp {No=0, Yes=1};

    uint32_t m_height = 0; //!< height of image in pixels
    uint32_t m_width = 0; //!< width of image in pixels
    bool m_hasAlpha = false; //!< If true, each pixel also has an alpha value (so that the image has 4 bytes per pixel).
    bool m_isBGR = false;    //!< If true, the image data is in BGR format; else RGB format. Always false for an image read from PNG.
    bool m_isTopDown = true; //!< If true, the image data is in top-down row order; else bottom-up order. Always true for an image read from PNG.

/** @name PNG Support for the PNG format */
/** @{ */
    //! Extract an RGB[A] image from a stream of bytes in the PNG format.
    //! @param[out] image   Image data read from PNG .
    //! @param[in]  inputBuffer The PNG definition data
    //! @param[in]  inputBufferSize The number of bytes in \a inputBuffer
    //! @return non-zero if the image could not be read.
    //! @remarks The image data written to \a imageData is always in top-down row order. It is in RGB format if \a hasAlpha is \a false, else RGBA. It is always 8-bit color.
    DGNPLATFORM_EXPORT BentleyStatus ReadImageFromPngBuffer(Render::Image& image, Byte const* inputBuffer, size_t inputBufferSize);  

    //! Extract an RGB[A] image from a file in the PNG format.
    //! @param[out] imageData   Image data read from PNG file.
    //! @param[in]  pngFile     The PNG file to read. Must be open and positioned at the start of the PNG data.
    //! @return non-zero if the image could not be read.
    //! @remarks The image data written to \a imageData is always in top-down row order. It is in RGB format if \a hasAlpha is \a false, else RGBA. It is always 8-bit color.
    //! The PNG data is in compressed format with 8-bit color.
    DGNPLATFORM_EXPORT BentleyStatus ReadImageFromPngFile(ByteStream& imageData, BeFile& pngFile);

    //! Write an image in RGB or RGBA format to PNG format.
    //! @param[out] pngData      The image in PNG format.
    //! @param[in,out] imageData The image data to be written to the PNG file. The data may be modified in place.
    //! @return non-zero error status if the data could not be written.
    //! @note The data in \a imageData may be modified. If the data is in BGR order, it is converted to RGB. If the data contains alpha values that are 0,
    //! they are set to 0xff.
    //! @remarks The PNG data is written in compressed format with 8-bit color.
    DGNPLATFORM_EXPORT BentleyStatus WriteImageToPngFormat(ByteStream& pngData, ByteStream& imageData) const;

    //! Write an image in RGB or RGBA format to a PNG file.
    //! @param[in] pngFile      The file to write to. Must be open for write.
    //! @param[in,out] imageData The image data to be written to the PNG file. The data may be modified in place.
    //! @return non-zero error status if the data could not be written.
    //! @note The data in \a imageData may be modified. If the data is in BGR order, it is converted to RGB. If the data contains alpha values that are 0,
    //! they are set to 0xff.
    //! @remarks The PNG data is written in compressed format with 8-bit color.
    DGNPLATFORM_EXPORT BentleyStatus WriteImageToPngFile(BeFile& pngFile, ByteStream& imageData) const;
/** @} */

/** @name JPEG Support for the JPEG format */
/** @{ */
    //! Read an image in RGB format from a JPEG file
    //! @param[out] image Image data read from JPEG file.
    //! @param[in] jpegBuffer The JPEG definition data
    //! @param[in] jpegBufferSize The number of bytes in \a inputBuffer
    //! @param[in] bottomUp Whether the image is flipped in y direction
    //! @return non-zero if the image could not be read.
    DGNPLATFORM_EXPORT BentleyStatus ReadImageFromJpgBuffer(Render::Image& image, Byte const* jpegBuffer, size_t jpegBufferSize, BottomUp bottomUp=BottomUp::No);

    //! Read an image in RGB format from a JPEG file
    //! @param[out] jpegData   The image in JPEG format.
    //! @param[in] rgbBuffer   Image data to be converted to JPEG format.
    //! @param[in] quality     The image quality to preserve. A value in the range 0-100, inclusive. 100 is loss-less. Any value less than 100 is lossy. 
    //! @param[in] bottomUp Whether the image is flipped in y direction
    //! @return non-zero if the image could not be written.
    //! @note the data in \a rgbBuffer must be in RGB format, have no alpha. 
    DGNPLATFORM_EXPORT BentleyStatus WriteImageToJpgBuffer(ByteStream& jpegData, ByteStream const& rgbBuffer, int quality, BottomUp bottomUp=BottomUp::No) const;
/** @} */
};

END_BENTLEY_DGN_NAMESPACE
