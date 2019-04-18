/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/ByteStream.h> 

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
// @bsiclass                                                    
//=======================================================================================
enum BeJpegPixelType
    {
    BE_JPEG_PIXELTYPE_Rgb,
    BE_JPEG_PIXELTYPE_RgbA,
    BE_JPEG_PIXELTYPE_Bgr,
    BE_JPEG_PIXELTYPE_BgrA,
    BE_JPEG_PIXELTYPE_Gray
    };

enum class BeJpegBottomUp {No=0, Yes=1};

//=======================================================================================
// @bsiclass                                                   
//=======================================================================================
struct BeJpegDecompressor
    {
private:
    void* m_jpegImpl;

public:
    BeJpegDecompressor();

    ~BeJpegDecompressor();

    //! Retrieve information about a JPEG image without decompressing it.
    //! @param[out] width      The width in pixels.
    //! @param[out] height     The height in pixels.
    //! @param jpegBuffer      Buffer containing a JPEG image.
    //! @param jpegBufferSize  The size of the JPEG buffer in bytes.
    //! @return SUCCESS if successful, or ERROR if an error occurred.
    BentleyStatus ReadHeader(uint32_t& width, uint32_t& height, Byte const* jpegBuffer, size_t jpegBufferSize);

    //! Decompress a JPEG Image.
    //! @param[out] pOutBuffer       Destination buffer that will hold uncompress pixels.
    //! @param[out] outBufferSize    The size of the destination buffer in bytes.
    //! @param jpegBuffer            Buffer containing a JPEG image.
    //! @param jpegBufferSize        The size of the JPEG buffer in bytes.
    //! @param pixelType             The destination pixel type. See BeJpegPixelType.
    //! @param bottomUp Whether the image is flipped in Y direction
    //! @return SUCCESS if successful, or ERROR if an error occurred.
    BentleyStatus Decompress(Byte* pOutBuffer, size_t outBufferSize, Byte const* jpegBuffer, size_t jpegBufferSize, BeJpegPixelType pixelType, BeJpegBottomUp bottomUp=BeJpegBottomUp::No);
    };

//=======================================================================================
// @bsiclass                                                   
//=======================================================================================
struct BeJpegCompressor
    {
private:
    void* m_jpegImpl;

public:
    //! Constructs a JPEG compressor
    BeJpegCompressor();

    //! Destroys a JPEG compressor
    ~BeJpegCompressor();

    //! Compress a pixel buffer to JPEG.
    //! @param[out] outBuf              Destination buffer that will hold jpeg data.
    //! @param srcBuffer                Buffer containing uncompressed pixels. Must be in RGB format.
    //! @param width                    The width in pixels.
    //! @param height                   The height in pixels.
    //! @param pixelType                The source pixel type. See BeJpegPixelType.
    //! @param quality                  Compression quality, 1 = worst, 100 = best. 
    //! @param bottomUp Whether the image is flipped in Y direction
    //! @return SUCCESS if successful, or ERROR if an error occurred.
    //! @remarks Always uses 4:2:2 chrominance subsampling.
    BentleyStatus Compress(ByteStream& outBuf, uint8_t const* srcBuffer, uint32_t width, uint32_t height, BeJpegPixelType pixelType, int quality, BeJpegBottomUp bottomUp=BeJpegBottomUp::No);

    //! Compress a pixel buffer to JPEG.
    //! @param[out] outBuf              Destination buffer that will hold jpeg data.
    //! @param srcBuffer                Buffer containing uncompressed pixels. Must be RGB format.
    //! @param srcPitch                 Bytes per line of srcBuffer. A value of 0 will default to width*height*BytePerPixel.
    //! @param width                    The width in pixels.
    //! @param height                   The height in pixels.
    //! @param pixelType                The source pixel type. See BeJpegPixelType.
    //! @param quality                  Compression quality, 1 = worst, 100 = best. 
    //! @param bottomUp Whether the image is flipped in Y direction
    //! @return SUCCESS if successful, or ERROR if an error occurred.
    //! @remarks Always uses 4:2:2 chrominance subsampling..
    BentleyStatus Compress(ByteStream& outBuf, uint8_t const* srcBuffer, size_t srcPitch, uint32_t width, uint32_t height, BeJpegPixelType pixelType, int quality, BeJpegBottomUp bottomUp=BeJpegBottomUp::No);
    };

END_BENTLEY_NAMESPACE
