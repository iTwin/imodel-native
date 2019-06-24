/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "libjpeg-turbo/turbojpeg.h"

#include <stdlib.h>

#include "BeJpeg/BeJpeg.h"

#include <Bentley/bvector.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static int getTjPixelType(BeJpegPixelType pixelType)
    {
    switch (pixelType)
        {
        case BE_JPEG_PIXELTYPE_Rgb: return TJPF_RGB;
        case BE_JPEG_PIXELTYPE_RgbA: return TJPF_RGBA;
        case BE_JPEG_PIXELTYPE_Bgr: return TJPF_BGR;
        case BE_JPEG_PIXELTYPE_BgrA: return TJPF_BGRA;
        case BE_JPEG_PIXELTYPE_Gray:return TJPF_GRAY;
        }

    BeAssert(false);
    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeJpegDecompressor::BeJpegDecompressor()
    {
    m_jpegImpl = tjInitDecompress();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeJpegDecompressor::~BeJpegDecompressor()
    {
    tjDestroy(m_jpegImpl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeJpegDecompressor::ReadHeader(uint32_t& width, uint32_t& height, Byte const* jpegBuffer, size_t jpegBufferSize)
    {
    int tjWidth, tjHeight, tjjpegSubsamp;

    if (0 == tjDecompressHeader2(m_jpegImpl, const_cast<Byte*>(jpegBuffer), (unsigned long)jpegBufferSize, &tjWidth, &tjHeight, &tjjpegSubsamp))
        {
        width = (uint32_t)tjWidth;
        height = (uint32_t)tjHeight;
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeJpegDecompressor::Decompress(Byte* pOutBuffer, size_t outBufferSize, Byte const* jpegBuffer, size_t jpegBufferSize, BeJpegPixelType pixelType, BeJpegBottomUp bottomUp)
    {
    int tjPixeltype = getTjPixelType(pixelType);
    if (-1 == tjPixeltype)
        return ERROR;

    return tjDecompress2(m_jpegImpl, const_cast<Byte*>(jpegBuffer), (unsigned long)jpegBufferSize, pOutBuffer, 0, 0, 0, tjPixeltype, BeJpegBottomUp::No==bottomUp ? 0 : TJFLAG_BOTTOMUP) ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeJpegCompressor::BeJpegCompressor()
    {
    m_jpegImpl = tjInitCompress();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BeJpegCompressor::~BeJpegCompressor()
    {
    tjDestroy(m_jpegImpl);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  12/2015
//----------------------------------------------------------------------------------------
BentleyStatus BeJpegCompressor::Compress(ByteStream& outBuf, uint8_t const* srcBuffer, uint32_t width, uint32_t height, BeJpegPixelType pixelType, int quality, BeJpegBottomUp bottomUp)
    {
    return Compress(outBuf, srcBuffer, 0/*pitch*/, width, height, pixelType, quality, bottomUp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus BeJpegCompressor::Compress(ByteStream& outBuf, uint8_t const* srcBuffer, size_t pitch, uint32_t width, uint32_t height, BeJpegPixelType pixelType, int quality, BeJpegBottomUp bottomUp)
    {
    int tjPixeltype = getTjPixelType(pixelType);
    if (-1 == tjPixeltype)
        return ERROR;

    int flags = TJFLAG_FASTDCT; 

    if (BeJpegBottomUp::Yes==bottomUp)
        flags |= TJFLAG_BOTTOMUP;

    int subsamp = TJSAMP_444;

    unsigned long jpegSize = 0;
    Byte* jpegBuf = nullptr;

    if (0 != tjCompress2(m_jpegImpl, const_cast<Byte*>(srcBuffer), width, (unsigned long)pitch, height, tjPixeltype, &jpegBuf, &jpegSize, subsamp, quality, flags) || nullptr == jpegBuf)
        return BSIERROR;

    outBuf.SaveData(jpegBuf, jpegSize);

    tjFree(jpegBuf);

    return BSISUCCESS;
    }
