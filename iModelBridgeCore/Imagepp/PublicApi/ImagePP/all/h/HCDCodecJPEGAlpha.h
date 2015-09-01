//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecJPEGAlpha.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecJPEGAlpha
//-----------------------------------------------------------------------------
// IJG codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecJPEG.h"
#include "HCDCodecIJG.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecRLE8;

class HCDCodecJPEGAlpha : public HCDCodecJPEG
    {
    HDECLARE_CLASS_ID(HCDCodecId_JPEGAlpha, HCDCodecJPEG);

public:
    //--------------------------------------
    // Declare Types
    //--------------------------------------

    // Available color modes for this codec
    enum ColorModes
        {
        RGB_OPACITY,
        GRAYSCALE_OPACITY,
        };


    //--------------------------------------
    // Contruction/Destruction
    //--------------------------------------

    IMAGEPP_EXPORT                 HCDCodecJPEGAlpha();
    IMAGEPP_EXPORT                 HCDCodecJPEGAlpha(uint32_t pi_Width,
                                             uint32_t pi_Height,
                                             uint32_t pi_BitsPerPixel);
    IMAGEPP_EXPORT                 HCDCodecJPEGAlpha(const HCDCodecJPEGAlpha& pi_rObj);
    IMAGEPP_EXPORT virtual         ~HCDCodecJPEGAlpha();


    //--------------------------------------
    // Overrides
    //--------------------------------------

    virtual HCDCodec* Clone() const override;

    // From HCDCodecIJG
    virtual size_t  CompressSubset  (const void* pi_pInData,
                                     size_t      pi_InDataSize,
                                     void*       po_pOutBuffer,
                                     size_t      pi_OutBufferSize);
    virtual size_t  DecompressSubset(const void* pi_pInData,
                                     size_t      pi_InDataSize,
                                     void*       po_pOutBuffer,
                                     size_t      pi_OutBufferSize);
    virtual bool   HasLineAccess() const;
    virtual size_t  GetMinimumSubsetSize() const;
    virtual void    Reset();
    virtual    void    SetBitsPerPixel(size_t pi_BitsPerPixel);
    virtual size_t  GetSubsetMaxCompressedSize() const;

    // added method.
    void            SetColorMode(ColorModes pi_Mode);
    ColorModes      GetColorMode() const;

    // added method
    void            SetQuality(Byte pi_Percentage);
    Byte          GetQuality() const;
    void            SetOptimizeCoding(bool pi_Enable);
    bool           GetOptimizeCoding() const;
    uint32_t        CreateTables(void* po_pOutBuffer, uint32_t pi_OutBufferSize);
    void            SetProgressiveMode(bool pi_Enable);
    bool           IsProgressive() const;
    IMAGEPP_EXPORT void    ReadHeader(const void* pi_pInData, size_t pi_InDataSize);
    IMAGEPP_EXPORT void    SetAbbreviateMode(bool pi_Enable);
    IMAGEPP_EXPORT bool    GetAbbreviateMode() const;
    IMAGEPP_EXPORT void    CopyTablesFromDecoderToEncoder();
    void            SetQuantizationTable(int pi_Slot, const unsigned int* pi_pTable);
    HCDCodecIJG::SubsamplingModes
    GetSubsamplingMode() const;
    void            SetSubsamplingMode(HCDCodecIJG::SubsamplingModes pi_Mode);


protected:
private:

    // The Codecs
    HFCPtr<HCDCodecIJG>
    m_pCodecJpeg;
    HFCPtr<HCDCodecRLE8>
    m_pCodecRLE8;
    ColorModes      m_ColorMode;

    // private methods
    void            InitObject();
    void            DeepCopy(const HCDCodecJPEGAlpha& pi_rObj);
    void            DeepDelete();
    };


END_IMAGEPP_NAMESPACE