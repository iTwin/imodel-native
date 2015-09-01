//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecIJG.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecIJG
//-----------------------------------------------------------------------------
// IJG codec class.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodecJPEG.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecIJG_8bits;
class HCDCodecIJG_12bits;

class HCDCodecIJG : public HCDCodecJPEG
    {
    HDECLARE_CLASS_ID(HCDCodecId_IJGJPEG, HCDCodecJPEG)

public:

    enum ColorModes
        {
        YCC,
        RGB,
        BGR,
        GRAYSCALE,
        CMYK,
        RGBA,
        UNKNOWN
        };

    enum SubsamplingModes
        {
        SNONE,
        S411,
        S422
        };

    // primary methods
    IMAGEPP_EXPORT                 HCDCodecIJG();
    IMAGEPP_EXPORT                 HCDCodecIJG(size_t pi_Width,
                                       size_t pi_Height,
                                       size_t pi_BitsPerPixel);
    IMAGEPP_EXPORT                 HCDCodecIJG(const HCDCodecIJG& pi_rObj);
    IMAGEPP_EXPORT virtual         ~HCDCodecIJG();


    // overriden methods
    virtual HCDCodec* Clone() const override;

    virtual size_t  CompressSubset( const void* pi_pInData,
                                    size_t pi_InDataSize,
                                    void* po_pOutBuffer,
                                    size_t pi_OutBufferSize);
    virtual size_t  DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize);
    virtual bool   HasLineAccess() const;
    virtual size_t  GetMinimumSubsetSize() const;
    virtual void    Reset();
    virtual void    SetBitsPerPixel(size_t pi_BitsPerPixel);
    virtual void    SetDimensions(size_t pi_Width, size_t pi_Height);
    virtual size_t  GetLinePaddingBits() const;
    virtual size_t  GetWidth() const;
    virtual size_t  GetHeight() const;
    virtual void    SetLinePaddingBits(size_t pi_Bits);
    virtual size_t  GetDataSize() const;
    virtual void    SetSubsetSize(size_t pi_Size);
    virtual size_t  GetSubsetWidth() const;
    virtual size_t  GetSubsetHeight() const;
    virtual size_t  GetSubsetPosY() const;
    virtual void    SetSubset(  size_t pi_Width,
                                size_t pi_Height,
                                size_t pi_PosX = 0,
                                size_t pi_PosY = 0);
    virtual void   SetSubsetPosY(size_t pi_PosY);
    virtual size_t GetCompressedImageIndex() const;


    // added method
    IMAGEPP_EXPORT void            SetQuality(Byte pi_Percentage);
    IMAGEPP_EXPORT Byte          GetQuality() const;
    IMAGEPP_EXPORT void            SetOptimizeCoding(bool pi_Enable);
    IMAGEPP_EXPORT bool           GetOptimizeCoding() const;
    IMAGEPP_EXPORT uint32_t        CreateTables(void* po_pOutBuffer, uint32_t pi_OutBufferSize);
    IMAGEPP_EXPORT void            SetProgressiveMode(bool pi_Enable);
    IMAGEPP_EXPORT bool           IsProgressive() const;
    IMAGEPP_EXPORT void            SetColorMode(ColorModes pi_Mode);
    IMAGEPP_EXPORT ColorModes      GetColorMode() const;
    IMAGEPP_EXPORT void            ReadHeader(const void* pi_pInData, size_t pi_InDataSize);
    IMAGEPP_EXPORT void            SetAbbreviateMode(bool pi_Enable);
    IMAGEPP_EXPORT bool           GetAbbreviateMode() const;
    IMAGEPP_EXPORT void            CopyTablesFromDecoderToEncoder();

    // Standard JPG color mode has been store in YCbCr form. (YCbCr is defined per CCIR 601-1)
    // The jpeg_set_defaults function may choose wrong colorspace, eg YCbCr if input is RGB.
    IMAGEPP_EXPORT void            SetSourceColorMode(ColorModes pi_Mode);
    IMAGEPP_EXPORT ColorModes      GetSourceColorMode() const;

    // third parameter applies only for IJL, internal use :)
    IMAGEPP_EXPORT void            SetQuantizationTable(int pi_Slot, const unsigned int* pi_pTable, bool pi_UnZigZag = true);

    // There is 3 kinds of jpeg stream:
    //  1) (default) Interchange format. A standalone jpeg stream.
    //  2) Abbreviated format for compressed image data. Same as (1) but without quantization and huffman tables.
    //  3) Abbreviated format for table-specification data. The quantization and huffman tables to used with a (2) stream.
    // MakeInterchangeFormat will inject quantization and huffman tables(3) in the jpeg header(2) without altering the compressed pixels.
    // The result will be a standalone jpeg stream(1). 0 is returned to indicate an ERROR.
    IMAGEPP_EXPORT size_t MakeInterchangeFormat(const Byte* pi_pInData, size_t pi_InDataSize, Byte* po_pOutBuffer, size_t pi_OutBufferSize) const;

    Byte const* GetAbbreviateTableHeader(size_t& headerSize) const;
    IMAGEPP_EXPORT SubsamplingModes
    GetSubsamplingMode() const;
    IMAGEPP_EXPORT void            SetSubsamplingMode(SubsamplingModes pi_Mode);

    virtual size_t         GetSubsetMaxCompressedSize() const;

protected:
private:
    HAutoPtr<HCDCodecIJG_8bits>     m_pJpegCodec8Bits;
    HAutoPtr<HCDCodecIJG_12bits>    m_pJpegCodec12Bits;
    };


END_IMAGEPP_NAMESPACE