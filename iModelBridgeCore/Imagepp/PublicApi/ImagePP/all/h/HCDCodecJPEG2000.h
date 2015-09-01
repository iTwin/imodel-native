//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecJPEG2000.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecJPEG2000
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecErMapperSupported.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecJPEG2000 : public HCDCodecErMapperSupported
    {
    HDECLARE_CLASS_ID(HCDCodecId_JPEG2000, HCDCodecErMapperSupported)

public:
    // primary methods
    IMAGEPP_EXPORT                 HCDCodecJPEG2000();
    IMAGEPP_EXPORT                 HCDCodecJPEG2000(const HCDCodecJPEG2000& pi_rObj);
    IMAGEPP_EXPORT virtual         ~HCDCodecJPEG2000();

    virtual HCDCodec* Clone() const override;
    };


#if 0 //MST DONT DELETE - Developped for the JPEG 2000 codec prototype
//which purposes was to verify the feasibility of supporting
//JPEG 2000 in iTiff. This code would likely be used eventually
//to replace the file creation code from HUTExportToFile.cpp

class CMyIOStream2;
class CMyIOFileStream;
class JPEG2000Codec;
class HFCBinStream;

class HCDCodecJPEG2000 : public HCDCodecErMapperSupported
    {
    HDECLARE_CLASS_ID(HCDCodecId_JPEG2000, HCDCodecErMapperSupported)
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

    // primary methods
    IMAGEPP_EXPORT                 HCDCodecJPEG2000();
    IMAGEPP_EXPORT                 HCDCodecJPEG2000(uint32_t pi_Width,
                                            uint32_t pi_Height,
                                            uint32_t pi_BitsPerPixel);
    IMAGEPP_EXPORT                 HCDCodecJPEG2000(const HCDCodecJPEG2000& pi_rObj);
    IMAGEPP_EXPORT virtual         ~HCDCodecJPEG2000();

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

    virtual bool   IsBitsPerPixelSupported(uint32_t pi_Bits) const;

    // added method

    IMAGEPP_EXPORT void            SetColorMode(ColorModes pi_Mode);
    IMAGEPP_EXPORT ColorModes      GetColorMode() const;

    void SetJP2Stream(HFCBinStream* pi_pTiffFile,
                      uint32_t      pi_JP2StreamOffset,
                      uint64_t&     pi_rStreamSize);
    void SetStripInd(uint32_t pi_StripInd);


    uint32_t                m_ResHeight;
    uint32_t                m_StripHeight;

protected:

    void OpenJP2Stream();

    HAutoPtr<CMyIOStream2>    m_pStream;
    HAutoPtr<CMyIOFileStream> m_pCompressedStream;

    HAutoPtr<JPEG2000Codec> m_pCodec;

    unsigned short         m_CellType;
    UINT16                  m_NbBands;

    HFCBinStream*           m_pJP2Stream;
    uint32_t                m_JP2StreamOffset;
    uint32_t                m_StripInd;
    uint64_t               m_JP2StreamSize;

private:

    void InitObject();
    void DeepCopy();
    void DeepDelete();

    HAutoPtr<Byte>     m_pFileView;      //CNCSJP2FileView
    ColorModes          m_ColorMode;

    };
#endif

END_IMAGEPP_NAMESPACE