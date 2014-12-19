//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecFlashpixOLDForMSI10.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecFlashpix
//-----------------------------------------------------------------------------
// Flashpix codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

class HCDCodecIJG;

class HCDCodecFlashpixOLDForMSI10 : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(1299, HCDCodecImage)

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
    _HDLLu                 HCDCodecFlashpixOLDForMSI10();
    _HDLLu                 HCDCodecFlashpixOLDForMSI10(   size_t      pi_Width,
                                                          size_t      pi_Height,
                                                          ColorModes  pi_Mode);
    _HDLLu                 HCDCodecFlashpixOLDForMSI10(const HCDCodecFlashpixOLDForMSI10& pi_rObj);
    _HDLLu virtual         ~HCDCodecFlashpixOLDForMSI10();

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
    void            SetQuality(Byte pi_Quality);
    Byte          GetQuality() const;
    _HDLLu void            SetTable(uint32_t pi_Table,
                                    Byte* pi_pTable,
                                    uint32_t pi_TableSize);
    _HDLLu void            EnableInterleave(bool pi_Enable);
    bool           IsInterleaveEnabled() const;
    _HDLLu void            SetSubSampling(Byte pi_SubSampling);
    Byte           GetSubSampling() const;
    _HDLLu void            EnableColorConversion(bool pi_Enable);
    bool           IsColorConversionEnabled() const;
    ColorModes      GetColorMode() const;
    void            SetColorMode(ColorModes pi_Mode);
    uint32_t        GetEncoderTable() const;
    uint32_t        GetTableCount() const;
    const Byte*    GetTable(uint32_t pi_Index);
    uint32_t        GetTableSize(uint32_t pi_Index);
    _HDLLu void            SetCurrentTable(uint32_t pi_Index);
    uint32_t        GetCurrentTable() const;
    void            UpdateDefaultTable();

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

    void            DeepCopy(const HCDCodecFlashpixOLDForMSI10& pi_rObj);
    void            DeepDelete();
    void            InitObject( HCDCodecFlashpixOLDForMSI10::ColorModes pi_Mode,
                                size_t pi_Width,
                                size_t pi_Height);
    };

