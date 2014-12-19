//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecFlashpix.h $
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

class HCDCodecFlashpix : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(1237,HCDCodecImage )

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
    _HDLLu                 HCDCodecFlashpix();
    _HDLLu                 HCDCodecFlashpix(   size_t      pi_Width,
                                               size_t      pi_Height,
                                               ColorModes  pi_Mode);
    _HDLLu                 HCDCodecFlashpix(const HCDCodecFlashpix& pi_rObj);
    _HDLLu virtual         ~HCDCodecFlashpix();

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
    _HDLLu void            SetQuality(Byte pi_Quality);
    _HDLLu Byte          GetQuality() const;
    _HDLLu void            SetTable(uint32_t pi_Table,
                                    Byte* pi_pTable,
                                    size_t pi_TableSize);
    _HDLLu void            EnableInterleave(bool pi_Enable);
    _HDLLu bool           IsInterleaveEnabled() const;
    _HDLLu void            SetSubSampling(Byte pi_SubSampling);
    _HDLLu Byte           GetSubSampling() const;
    _HDLLu void            EnableColorConversion(bool pi_Enable);
    _HDLLu bool           IsColorConversionEnabled() const;
    _HDLLu ColorModes      GetColorMode() const;
    _HDLLu void            SetColorMode(ColorModes pi_Mode);
    _HDLLu uint32_t        GetEncoderTable() const;
    _HDLLu uint32_t        GetTableCount() const;
    _HDLLu const Byte*    GetTable(uint32_t pi_Index);
    _HDLLu uint32_t        GetTableSize(uint32_t pi_Index);
    _HDLLu void            SetCurrentTable(uint32_t pi_Index);
    _HDLLu uint32_t        GetCurrentTable() const;
    _HDLLu void            UpdateDefaultTable();

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

