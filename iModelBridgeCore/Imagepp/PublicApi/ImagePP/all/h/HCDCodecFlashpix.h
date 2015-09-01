//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecFlashpix.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecFlashpix
//-----------------------------------------------------------------------------
// Flashpix codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecIJG;

class HCDCodecFlashpix : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_Flashpix,HCDCodecImage )

public:

    enum ColorModes
        {
        PHOTOYCC,
        PHOTOYCC_OPACITY,
        RGB,
        RGB_OPACITY,
        MONOCHROME,
        MONOCHROME_OPACITY,
        OPACITY
        };


    // primary methods
    IMAGEPP_EXPORT                 HCDCodecFlashpix();
    IMAGEPP_EXPORT                 HCDCodecFlashpix(   size_t      pi_Width,
                                               size_t      pi_Height,
                                               ColorModes  pi_Mode);
    IMAGEPP_EXPORT                 HCDCodecFlashpix(const HCDCodecFlashpix& pi_rObj);
    IMAGEPP_EXPORT virtual         ~HCDCodecFlashpix();

    // overriden methods
    virtual HCDCodec* Clone() const override;
    virtual size_t  CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t pi_OutBufferSize);
    virtual size_t  DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize);

    virtual bool   IsBitsPerPixelSupported(size_t pi_Bits) const;
    virtual    void    SetDimensions(size_t pi_Width, size_t pi_Height);
    virtual void    SetLinePaddingBits(size_t pi_Bits);

    // added methods
    IMAGEPP_EXPORT void            SetQuality(Byte pi_Quality);
    IMAGEPP_EXPORT Byte          GetQuality() const;
    IMAGEPP_EXPORT void            SetTable(uint32_t pi_Table,
                                    Byte* pi_pTable,
                                    size_t pi_TableSize);
    IMAGEPP_EXPORT void            EnableInterleave(bool pi_Enable);
    IMAGEPP_EXPORT bool           IsInterleaveEnabled() const;
    IMAGEPP_EXPORT void            SetSubSampling(Byte pi_SubSampling);
    IMAGEPP_EXPORT Byte           GetSubSampling() const;
    IMAGEPP_EXPORT void            EnableColorConversion(bool pi_Enable);
    IMAGEPP_EXPORT bool           IsColorConversionEnabled() const;
    IMAGEPP_EXPORT ColorModes      GetColorMode() const;
    IMAGEPP_EXPORT void            SetColorMode(ColorModes pi_Mode);
    IMAGEPP_EXPORT uint32_t        GetEncoderTable() const;
    IMAGEPP_EXPORT uint32_t        GetTableCount() const;
    IMAGEPP_EXPORT const Byte*    GetTable(uint32_t pi_Index);
    IMAGEPP_EXPORT uint32_t        GetTableSize(uint32_t pi_Index);
    IMAGEPP_EXPORT void            SetCurrentTable(uint32_t pi_Index);
    IMAGEPP_EXPORT uint32_t        GetCurrentTable() const;
    IMAGEPP_EXPORT void            UpdateDefaultTable();

protected:

private:

    struct TableEntry
        {
        TableEntry() : pData(0), BufSize(0) {}
        Byte*   pData;
        size_t  BufSize;
        };

    typedef vector<TableEntry> Tables;

    uint32_t           m_LastTable;
    uint32_t           m_TableSelection;
    bool                m_DefaultTableUpdated;
    uint32_t           m_ColorMode;
    HFCPtr<HCDCodecIJG> m_pCodecJPEG;
    Tables              m_Tables;

    // NOT USED
    Byte           m_SubSampling;
    bool           m_InterleaveMode;
    bool           m_EnableColorConvert;

    void            DeepCopy(const HCDCodecFlashpix& pi_rObj);
    void            DeepDelete();
    void            InitObject( HCDCodecFlashpix::ColorModes pi_Mode,
                                size_t pi_Width,
                                size_t pi_Height);
    };

END_IMAGEPP_NAMESPACE