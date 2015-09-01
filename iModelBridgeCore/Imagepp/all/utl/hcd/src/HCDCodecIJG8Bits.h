//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecIJG8Bits.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecIJG
//-----------------------------------------------------------------------------
// IJG codec class.
//-----------------------------------------------------------------------------

#ifndef HCDCodecIJG8Bits_H           // Don't use #pragma once here, See HCDCodecIJG12Bits.h and .cpp
#define HCDCodecIJG8Bits_H

#include <Imagepp/all/h/HCDCodecIJG.h>

BEGIN_IMAGEPP_NAMESPACE

#ifndef IJG12BITS
#define IJG12BITS(x)    x##_8bits
#endif

class IJG12BITS(HCDCodecIJG) : public HCDCodecJPEG
    {
#ifdef JPEGLIB_SUPPORT_12BITS
    HDECLARE_CLASS_ID(HCDCodecId_IJG12bits, HCDCodecJPEG)         // version 12 bits
#else
    HDECLARE_CLASS_ID(HCDCodecId_IJG8bits, HCDCodecJPEG)          // version 8 bits
#endif

public:

    typedef vector<uint32_t, allocator<uint32_t> >
    HUINTVector;

    typedef vector< HUINTVector, allocator< HUINTVector > >
    HUINTVectorVector;

    // primary methods
                    IJG12BITS(HCDCodecIJG)();
                    IJG12BITS(HCDCodecIJG)(  size_t pi_Width,
                                                    size_t pi_Height,
                                                    size_t pi_BitsPerPixel);
                    IJG12BITS(HCDCodecIJG)(const IJG12BITS(HCDCodecIJG)& pi_rObj);
    virtual         ~IJG12BITS(HCDCodecIJG)();


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

    // added method
    void            SetQuality(Byte pi_Percentage);
    Byte          GetQuality() const;
    void            SetOptimizeCoding(bool pi_Enable);
    bool           GetOptimizeCoding() const;
    uint32_t        CreateTables(void* po_pOutBuffer, uint32_t pi_OutBufferSize);
    void            SetProgressiveMode(bool pi_Enable);
    bool           IsProgressive() const;
    void            SetColorMode(HCDCodecIJG::ColorModes pi_Mode);
    HCDCodecIJG::ColorModes GetColorMode() const;
    void            ReadHeader(const void* pi_pInData, size_t pi_InDataSize);
    void            SetAbbreviateMode(bool pi_Enable);
    bool           GetAbbreviateMode() const;
    void            CopyTablesFromDecoderToEncoder();

    // Standard JPG color mode has been store in YCbCr form. (YCbCr is defined per CCIR 601-1)
    // The jpeg_set_defaults function may choose wrong colorspace, eg YCbCr if input is RGB.
    void            SetSourceColorMode(HCDCodecIJG::ColorModes pi_Mode);
    HCDCodecIJG::ColorModes      GetSourceColorMode() const;

    // third parameter applies only for IJL, internal use :)
    void            SetQuantizationTable(int pi_Slot, const unsigned int* pi_pTable, bool pi_UnZigZag = true);

    Byte const* GetAbbreviateTableHeader(size_t& headerSize) const;

    HCDCodecIJG::SubsamplingModes
    GetSubsamplingMode() const;
    void            SetSubsamplingMode(HCDCodecIJG::SubsamplingModes pi_Mode);

    virtual size_t  GetSubsetMaxCompressedSize() const;

    static void HCDJpegErrorExit(void* cinfo);

    // This structure is used by the error handling functions
    // of the IJG JPEG library
    typedef struct
        {
        Byte* pub;
        } HCDJpegFileErrorManager;

    size_t MergeDQT_DHT(const Byte* pi_pInData,
                                              size_t pi_InDataSize,
                                              Byte* po_pOutBuffer,
                                              size_t po_OutBufferSize);


private:

    // IJG only
    void*                   m_cinfocomp;
    void*                   m_cinfodec;
    HCDJpegFileErrorManager m_ErrorManager;

    // common
    Byte                  m_Quality;
    bool                   m_OptimizeCoding;
    bool                   m_ProgressiveMode;
    HCDCodecIJG::ColorModes m_ColorMode;
    HCDCodecIJG::ColorModes m_StoredColorMode;

    bool                   m_AbbreviateMode;
    HArrayAutoPtr<Byte>   m_pHeader;
    size_t                  m_HeaderSize;
    HCDCodecIJG::SubsamplingModes        m_SubsamplingMode;
    bool                   m_ExternalQuantizationTablesUse;
    HUINTVectorVector       m_QuantizationTables;

    // private methods
    void            InitObject();
    void            DeepCopy(const IJG12BITS(HCDCodecIJG)& pi_rObj);
    void            DeepDelete();
    };

END_IMAGEPP_NAMESPACE

#endif      // HCDCodecIJG8Bits_H