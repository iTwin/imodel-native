//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecJBIG.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecIJG
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCBinStream.h>

#include <Imagepp/all/h/HCDCodecJBIG.h>

#include <Imagepp/all/h/D:\BSI\08.09.00.xx\src\imagepp\ext\jbigkit\libjbig\jbig.h>

#define HCD_CODEC_NAME L"JBIG"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecJBIG::HCDCodecJBIG()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    try
        {
        InitObject();
        }
    catch(...)
        {
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecJBIG::HCDCodecJBIG(uint32_t pi_Width,
                           uint32_t pi_Height)
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    try
        {
        InitObject();
        SetDimensions(pi_Width, pi_Height);
        SetBitsPerPixel(1);
        }
    catch(...)
        {
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecJBIG::HCDCodecJBIG(const HCDCodecJBIG& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecJBIG::~HCDCodecJBIG()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecJBIG::Clone() const
    {
    return new HCDCodecJBIG(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
void HCDCodecJBIG::output_bie(Byte* start, size_t len, void* file)
    {
    HCDCodecJBIG* pCodec = (HCDCodecJBIG*)file;

    memcpy(pCodec->m_pOutBuffer, start, len);
    pCodec->m_pOutBuffer += len;

    return;
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecJBIG::CompressSubset(const void* pi_pInData,
                                    size_t      pi_InDataSize,
                                    void*       po_pOutBuffer,
                                    size_t      po_OutBufferSize)
    {
    HPRECONDITION(pi_InDataSize < INT_MAX);
    HPRECONDITION(po_OutBufferSize < INT_MAX);

    struct jbg_enc_state se;

    m_pOutBuffer = (Byte*)po_pOutBuffer;

    jbg_enc_init(&se,
                 GetWidth(),
                 GetHeight(),
                 1,
                 (Byte**)(&pi_pInData),
                 output_bie,
                 this);              /* initialize encoder */

    jbg_enc_out(&se);                                    /* encode image */

    HASSERT((size_t)(m_pOutBuffer - (Byte*)po_pOutBuffer) <= po_OutBufferSize);

    jbg_enc_free(&se);                    /* release allocated resources */

    return (m_pOutBuffer - (Byte*)po_pOutBuffer);
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecJBIG::DecompressSubset(const void*  pi_pInData,
                                      size_t       pi_InDataSize,
                                      void*        po_pOutBuffer,
                                      size_t       pi_OutBufferSize)
    {
    HPRECONDITION(pi_InDataSize < INT_MAX);
    HPRECONDITION(pi_OutBufferSize < INT_MAX);

    struct jbg_dec_state sjd;

    jbg_dec_init(&sjd);
    int Result = jbg_dec_in(&sjd, (unsigned char*)pi_pInData, pi_InDataSize, NULL);

    HASSERT(Result == JBG_EOK);
    HASSERT(sjd.planes == 1);
    HASSERT(sjd.d == 0);

    size_t DecompressedImgSize = ((sjd.xd + 7) / 8) * sjd.yd;

    HASSERT(DecompressedImgSize <= pi_OutBufferSize);

    memcpy(po_pOutBuffer, sjd.lhp[0][0], DecompressedImgSize);

    jbg_dec_free(&sjd);

    return DecompressedImgSize;
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecJBIG::IsBitsPerPixelSupported(uint32_t pi_Bits) const
    {
    if(pi_Bits == 1)
        return true;
    else
        return false;
    }

//-----------------------------------------------------------------------------
// protected
// InitObject
//-----------------------------------------------------------------------------
void HCDCodecJBIG::InitObject()
    {
    m_pOutBuffer     = 0;
    }