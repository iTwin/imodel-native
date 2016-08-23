//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/HTIFFFileCoDec.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HTIFFFileCoDec
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <Imagepp/all/h/HTIFFFile.h>
#include <ImagePP/all/h/HTIFFTag.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecCCITTRLE.h>
#include <Imagepp/all/h/HCDCodecCCITTFax4.h>
#include <Imagepp/all/h/HCDPacket.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecLZW.h>

#include <zlib/zlib.h>



//-----------------------------------------------------------------------------
// private
// SetQuality ; Set the Quality if possible on the current Codec
//-----------------------------------------------------------------------------
void HTIFFFile::SetQualityToCodec()
    {
    HFCMonitor Monitor(m_Key);

    // Used the default
    if (m_CompressionQuality != -1)
        {
        HFCPtr<HCDCodec> pCodec = m_pPacket->GetCodec();

        if (pCodec->GetClassID() == HCDCodecIJG::CLASS_ID)
            {
            ((HFCPtr<HCDCodecIJG>&)pCodec)->SetQuality((Byte)m_CompressionQuality);
            }
        else if (pCodec->GetClassID() == HCDCodecFlashpix::CLASS_ID)
            {
            ((HFCPtr<HCDCodecFlashpix>&)pCodec)->SetQuality((Byte)m_CompressionQuality);
            }
        }
    }


//-----------------------------------------------------------------------------
// private
// SetNoneAlgo ; This method set the callback m_pCompressFunc and
//                                            m_pUncompressFunc
//-----------------------------------------------------------------------------
void HTIFFFile::SetNoneAlgo()
    {
    HFCMonitor Monitor(m_Key);

    m_pCompressFunc     = &HTIFFFile::CompressBlock;
    m_pUncompressFunc   = &HTIFFFile::UncompressBlock;
    m_pSetHeightFunc    = 0;

    m_pPacket->SetCodec(new HCDCodecIdentity(m_StripTileSize));
    }

//-----------------------------------------------------------------------------
// private
// SetDeflateAlgo ; This method set the callback m_pCompressFunc and
//                                               m_pUncompressFunc
//-----------------------------------------------------------------------------

void HTIFFFile::SetDeflateAlgo(uint32_t pi_BitsPerPixel, uint16_t pi_Predictor, uint32_t pi_SamplesPerPixel)
    {
    HFCMonitor Monitor(m_Key);

    if (2 == pi_Predictor)
        {
        uint32_t Width;
        uint32_t Height;
        ExtractWidthHeight(&Width, &Height);

        m_pPacket->SetCodec(new HCDCodecZlib(m_StripTileSize, Width, pi_BitsPerPixel, pi_Predictor, pi_SamplesPerPixel));
        }
    else
        {
        HASSERT(1 == pi_Predictor); // Strange, need investigation why we pass here.

        m_pPacket->SetCodec(new HCDCodecZlib(m_StripTileSize));
        }

    m_pCompressFunc     = &HTIFFFile::CompressBlock;
    m_pUncompressFunc   = &HTIFFFile::UncompressBlock;
    m_pSetHeightFunc    = 0;
    }

//-----------------------------------------------------------------------------
// private
// SetLZWAlgo ; This method set the callback m_pCompressFunc and
//                                               m_pUncompressFunc
//-----------------------------------------------------------------------------

void HTIFFFile::SetLZWAlgo(uint32_t pi_BitsPerPixel, unsigned short pi_Predictor, uint32_t pi_SamplesPerPixel)
    {
    HFCMonitor Monitor(m_Key);

    uint32_t Width;
    uint32_t Height;
    ExtractWidthHeight (&Width, &Height);
    Byte LinePaddingBits = (Byte)((8 - ((Width * pi_BitsPerPixel) % 8)) % 8);

    //Special case to fix TFS 88368.
    if (1 == pi_SamplesPerPixel && 2 == pi_Predictor && 32 == pi_BitsPerPixel)
        {
        m_pPacket->SetCodec(new HCDCodecLZWPredicateExt(Width, Height,pi_BitsPerPixel, pi_Predictor, pi_SamplesPerPixel));
        }
    else
        {
        m_pPacket->SetCodec(new HCDCodecLZW(Width, Height,pi_BitsPerPixel, pi_Predictor));
        }        

    if (LinePaddingBits != 0)
        ((HFCPtr<HCDCodecLZW>&)(m_pPacket->GetCodec()))->SetLinePaddingBits(LinePaddingBits);

    m_pCompressFunc     = &HTIFFFile::CompressBlock;
    m_pUncompressFunc   = &HTIFFFile::UncompressBlock;
    m_pSetHeightFunc    = 0;
    }

//-----------------------------------------------------------------------------
// private
// SetPackBitsAlgo ; This method set the callback m_pCompressFunc and
//                                               m_pUncompressFunc
//-----------------------------------------------------------------------------

void HTIFFFile::SetPackBitsAlgo(uint32_t pi_BitsPerPixel)
    {
    HFCMonitor Monitor(m_Key);
    uint32_t Width;
    uint32_t Height;
    ExtractWidthHeight (&Width, &Height);
    Byte LinePaddingBits = (Byte)((8 - ((Width * pi_BitsPerPixel) % 8)) % 8);


    m_pPacket->SetCodec(new HCDCodecHMRPackBits(Width, Height, pi_BitsPerPixel));

    if (LinePaddingBits != 0)
        ((HFCPtr<HCDCodecHMRPackBits>&)(m_pPacket->GetCodec()))->SetLinePaddingBits(LinePaddingBits);
    m_pCompressFunc     = &HTIFFFile::CompressBlock;
    m_pUncompressFunc   = &HTIFFFile::UncompressBlock;
    m_pSetHeightFunc    = &HTIFFFile::SetHeight;
    }


//-----------------------------------------------------------------------------
// private
// SetRLE1Algo ; This method set the callback m_pCompressFunc and
//                                               m_pUncompressFunc
//-----------------------------------------------------------------------------

void HTIFFFile::SetRLE1Algo()
    {
    HFCMonitor Monitor(m_Key);
    uint32_t Width;
    uint32_t Height;
    ExtractWidthHeight (&Width, &Height);

    m_pPacket->SetCodec(new HCDCodecHMRRLE1(Width, Height));

    ((HFCPtr<HCDCodecHMRRLE1>&)(m_pPacket->GetCodec()))->EnableLineIndexesTable(true);

    // Set padding line pixel
    if ((Width % 8) != 0)
        ((HFCPtr<HCDCodecHMRRLE1>&)(m_pPacket->GetCodec()))->SetLinePaddingBits(8-(Width % 8));

    m_pCompressFunc     = &HTIFFFile::CompressBlockRLE1;
    m_pUncompressFunc   = &HTIFFFile::UncompressBlockRLE1;
    m_pSetHeightFunc    = &HTIFFFile::SetHeight;
    }

//-----------------------------------------------------------------------------
// private
// SetJPEGAlgo ; This method set the callback m_pCompressFunc and
//                                             m_pUncompressFunc
//-----------------------------------------------------------------------------

void HTIFFFile::SetJPEGAlgo(uint32_t pi_BitsPerPixel)
    {
    HFCMonitor Monitor(m_Key);
    uint32_t Width;
    uint32_t Height;
    ExtractWidthHeight (&Width, &Height);

    m_pCompressFunc     = &HTIFFFile::CompressBlock;
    m_pUncompressFunc   = &HTIFFFile::UncompressBlock;
    m_pSetHeightFunc    = &HTIFFFile::SetHeight;

    // if the pixelType is 48 (RGB) or 64(RGBA), 16 bits by sample, we will use JPEG 12bits
    m_pPacket->SetCodec(new HCDCodecIJG(Width, Height, pi_BitsPerPixel));

    unsigned short ShortVal;
    if(GetField (PHOTOMETRIC, &ShortVal))
        {
        if (ShortVal == PHOTOMETRIC_MINISBLACK || ShortVal == PHOTOMETRIC_MINISWHITE)
            {
            ((HFCPtr<HCDCodecIJG>&)(m_pPacket->GetCodec()))->SetColorMode(HCDCodecIJG::GRAYSCALE);
            }
        else
            ((HFCPtr<HCDCodecIJG>&)(m_pPacket->GetCodec()))->SetColorMode(HCDCodecIJG::RGB);
        }

    // !!!! HChkSebG !!!!
    // Must take care if jpeg data source has been stored as RGB (should'nt but happen..) or YCbCr...
    if (pi_BitsPerPixel == 24 && ShortVal == PHOTOMETRIC_RGB)
        {
        ((HFCPtr<HCDCodecIJG>&)(m_pPacket->GetCodec()))->SetSourceColorMode(HCDCodecIJG::RGB);
        }

    unsigned short HorizontalSub;
    unsigned short VerticalSub;

    if (((HFCPtr<HCDCodecIJG>&)(m_pPacket->GetCodec()))->GetColorMode() != HCDCodecIJG::GRAYSCALE)
        {
        if (GetField(YCBCRSUBSAMPLING, &HorizontalSub, &VerticalSub))
            {
            if(HorizontalSub == 1 && VerticalSub == 1)
                ((HFCPtr<HCDCodecIJG>&)(m_pPacket->GetCodec()))->SetSubsamplingMode(HCDCodecIJG::SNONE);
            else if(HorizontalSub == 2 && VerticalSub == 2)
                // This case is the default.
                ((HFCPtr<HCDCodecIJG>&)(m_pPacket->GetCodec()))->SetSubsamplingMode(HCDCodecIJG::S411);
            else
                ((HFCPtr<HCDCodecIJG>&)(m_pPacket->GetCodec()))->SetSubsamplingMode(HCDCodecIJG::S422);
            }
        }

    // optimize coding
    if(GetField (COMPRESSION_JPEGOPTIMIZECODING, &ShortVal))
        ((HFCPtr<HCDCodecIJG>&)(m_pPacket->GetCodec()))->SetOptimizeCoding(true);
    else
        ((HFCPtr<HCDCodecIJG>&)(m_pPacket->GetCodec()))->SetOptimizeCoding(false);

    // get the field of the JPEG table if there is one
    Byte* pTable;
    uint32_t CodecTableSize;
    if(GetField (JPEGTABLES, &CodecTableSize, &pTable) && CodecTableSize > 0)
        {
        ((HFCPtr<HCDCodecIJG>&)(m_pPacket->GetCodec()))->ReadHeader(pTable, CodecTableSize);
        ((HFCPtr<HCDCodecIJG>&)(m_pPacket->GetCodec()))->CopyTablesFromDecoderToEncoder();
        ((HFCPtr<HCDCodecIJG>&)(m_pPacket->GetCodec()))->SetAbbreviateMode(true);
        }
    else
        SetQualityToCodec();
    }

//-----------------------------------------------------------------------------
// private
// SetFlashpixAlgo ; This method set the callback m_pCompressFunc and
//                                             m_pUncompressFunc
//-----------------------------------------------------------------------------

void HTIFFFile::SetFlashpixAlgo(uint32_t pi_BitsPerPixel)
    {
    HFCMonitor Monitor(m_Key);
    uint32_t Width;
    uint32_t Height;
    ExtractWidthHeight (&Width, &Height);

    unsigned short ShortVal;

    HCDCodecFlashpix::ColorModes ColorMode;
    if(GetField (PHOTOMETRIC, &ShortVal) && ShortVal == PHOTOMETRIC_YCBCR)
        {
        if(pi_BitsPerPixel == 32)
            ColorMode = HCDCodecFlashpix::PHOTOYCC_OPACITY;
        else
            ColorMode = HCDCodecFlashpix::PHOTOYCC;
        }
    else
        {
        if(pi_BitsPerPixel == 32)
            ColorMode = HCDCodecFlashpix::RGB_OPACITY;
        else if(pi_BitsPerPixel == 24)
            ColorMode = HCDCodecFlashpix::RGB;
        else if(pi_BitsPerPixel == 16)
            ColorMode = HCDCodecFlashpix::MONOCHROME_OPACITY;
        else
            ColorMode = HCDCodecFlashpix::MONOCHROME;
        }

    m_pCompressFunc     = &HTIFFFile::CompressBlock;
    m_pUncompressFunc   = &HTIFFFile::UncompressBlock;
    m_pSetHeightFunc    = &HTIFFFile::SetHeight;

    // get the field of the JPEG table if there is one
    Byte* pTable;
    uint32_t CodecTableSize;

    if(GetField (JPEGTABLES, &CodecTableSize, &pTable) && CodecTableSize > 0)
        {
        HASSERT(pTable != 0);

        m_pPacket->SetCodec(new HCDCodecFlashpix(Width, Height, ColorMode));

        // Copy the TIFF JPEG table to table 1
        ((HFCPtr<HCDCodecFlashpix>&)(m_pPacket->GetCodec()))->SetTable(1, pTable, CodecTableSize);
        ((HFCPtr<HCDCodecFlashpix>&)(m_pPacket->GetCodec()))->SetCurrentTable(1);
        }
    }


//-----------------------------------------------------------------------------
// private
// SetCCITT3Algo ; This method set the callback m_pCompressFunc and
//                                              m_pUncompressFunc
//-----------------------------------------------------------------------------

void HTIFFFile::SetCCITTAlgo(uint32_t pi_CompressMode, bool pi_BitRev)
    {
    HFCMonitor Monitor(m_Key);
    uint32_t Width;
    uint32_t Height;
    ExtractWidthHeight (&Width, &Height);

    switch (pi_CompressMode)
        {
        case COMPRESSION_CCITTFAX3:
            m_pPacket->SetCodec(new HCDCodecHMRCCITT(Width, Height));
            break;

        case COMPRESSION_CCITTFAX4:
            m_pPacket->SetCodec(new HCDCodecCCITTFax4(Width, Height));
            break;

        case COMPRESSION_CCITTRLE:
            m_pPacket->SetCodec(new HCDCodecCCITTRLE(Width, Height));
            break;

        case COMPRESSION_CCITTRLEW:
            m_pPacket->SetCodec(new HCDCodecHMRCCITT(Width, Height));
            ((HFCPtr<HCDCodecHMRCCITT>&)m_pPacket->GetCodec())->SetOptions (CCITT_FAX3_NOEOL|CCITT_FAX3_WORDALIGN);
            break;
        }

    HFCPtr<HCDCodecCCITT> pCCITTCodec ((HFCPtr<HCDCodecHMRCCITT>&)m_pPacket->GetCodec());

    // Set padding line pixel
    if ((Width % 8) != 0)
        pCCITTCodec->SetLinePaddingBits(8-(Width % 8));

    pCCITTCodec->SetBitRevTable(pi_BitRev);

    // Check for special setting
    unsigned short ShortVal;
    uint32_t    LongVal;
    RATIONAL    RationalVal;

    if (GetField (PHOTOMETRIC, &ShortVal))
        {
        switch(ShortVal)
            {
            case PHOTOMETRIC_MINISBLACK:
                pCCITTCodec->SetPhotometric(CCITT_PHOTOMETRIC_MINISBLACK);
                break;

            case PHOTOMETRIC_MINISWHITE:
            default:
                pCCITTCodec->SetPhotometric(CCITT_PHOTOMETRIC_MINISWHITE);
                break;
            }
        }

    // Only HMR CCITT codec support these methods.
    if(pCCITTCodec->IsCompatibleWith(HCDCodecHMRCCITT::CLASS_ID))
        {
        HFCPtr<HCDCodecHMRCCITT> pHMRCCITTCodec ((HFCPtr<HCDCodecHMRCCITT>&)pCCITTCodec);

        if (GetField (RESOLUTIONUNIT, &ShortVal))
            {
            switch(ShortVal)
                {
                case RESUNIT_NONE:
                    pHMRCCITTCodec->SetResolutionUnit(CCITT_RESUNIT_NONE);
                    break;
                case RESUNIT_INCH:
                    pHMRCCITTCodec->SetResolutionUnit(CCITT_RESUNIT_INCH);
                    break;
                case RESUNIT_CENTIMETER:
                    pHMRCCITTCodec->SetResolutionUnit(CCITT_RESUNIT_CENTIMETER);
                    break;
                }
            }

        if (GetField (YRESOLUTION, &RationalVal))
            {
            pHMRCCITTCodec->SetYResolution((float)RationalVal.Value);
            }

        if (GetField (GROUP3OPTIONS, &LongVal))
            {
            int32_t Opt = 0;
            if (LongVal & GROUP3OPT_2DENCODING)
                Opt |= CCITT_GROUP3OPT_2DENCODING;
            if (LongVal & GROUP3OPT_UNCOMPRESSED)
                Opt |= CCITT_GROUP3OPT_UNCOMPRESSED;
            if (LongVal & GROUP3OPT_FILLBITS)
                Opt |= CCITT_GROUP3OPT_FILLBITS;

            pHMRCCITTCodec->SetGroup3Options(Opt);
            }
        }

    m_pCompressFunc     = &HTIFFFile::CompressBlock;
    m_pUncompressFunc   = &HTIFFFile::UncompressBlock;
    m_pSetHeightFunc    = &HTIFFFile::SetHeight;
    }

//-----------------------------------------------------------------------------
// private
// CompressBlock and UncompressBlock callback method.
//-----------------------------------------------------------------------------
HSTATUS HTIFFFile::CompressBlock(const Byte* pi_pSrcBuffer,
                                 uint32_t      pi_SrcLen,
                                 const HFCPtr<HCDPacket>&
                                 pio_rpPacket)
    {
    HFCMonitor Monitor(m_Key);
    HFCPtr<HCDPacket> pCompressedPacket(pio_rpPacket);

    HCDPacket UncompressedPacket((Byte*)pi_pSrcBuffer, pi_SrcLen, pi_SrcLen);
    bool Status = UncompressedPacket.Compress(pCompressedPacket);

    if(Status)
        return H_SUCCESS;
    else
        return H_ERROR;
    }

HSTATUS HTIFFFile::UncompressBlock (const HFCPtr<HCDPacket>& pio_rpPacket,
                                    Byte*       po_pDstBuffer,
                                    uint32_t*       po_pDstLen)
    {
    HFCMonitor Monitor(m_Key);
    HFCPtr<HCDPacket> pCompressedPacket(pio_rpPacket);
    HFCPtr<HCDPacket> pUncompressedPacket(new HCDPacket(po_pDstBuffer, *po_pDstLen, 0));

    bool Status =  pCompressedPacket->Decompress(pUncompressedPacket);

    if(Status)
        return H_SUCCESS;
    else
        return H_ERROR;
    }

//-----------------------------------------------------------------------------
// private
// CompressBlockRLE1 and UncompressBlockRLE1 callback method.
// special case for RLE1
//-----------------------------------------------------------------------------
HSTATUS HTIFFFile::CompressBlockRLE1(const Byte* pi_pSrcBuffer,
                                     uint32_t      pi_SrcLen,
                                     const HFCPtr<HCDPacket>&
                                     pio_rpPacket)
    {
    HPRECONDITION(((HFCPtr<HCDCodecHMRRLE1>&)(pio_rpPacket->GetCodec()))->HasLineIndexesTable());
    HFCMonitor Monitor(m_Key);

    HFCPtr<HCDPacket> pCompressedPacket(pio_rpPacket);

    size_t LineIndexesTableSize = ((HFCPtr<HCDCodecHMRRLE1>&)(pio_rpPacket->GetCodec()))->GetHeight() * sizeof(uint32_t);

    HCDPacket UncompressedPacket((Byte*)pi_pSrcBuffer, pi_SrcLen, pi_SrcLen);
    bool Status = UncompressedPacket.Compress(pCompressedPacket);

    if(Status)
        {
        // copy the line indexes table
        size_t NewBufferSize = pCompressedPacket->GetDataSize() + LineIndexesTableSize;

        Byte* pBuffer = new Byte[NewBufferSize];

        // copy the line indexes table at the beginning of the data
        memcpy(pBuffer, ((HFCPtr<HCDCodecHMRRLE1>&)(pio_rpPacket->GetCodec()))->GetLineIndexesTable(), LineIndexesTableSize);

        // copy the data
        memcpy(pBuffer + LineIndexesTableSize, pCompressedPacket->GetBufferAddress(), pCompressedPacket->GetDataSize());

        pCompressedPacket->SetBuffer(pBuffer, NewBufferSize);
        pCompressedPacket->SetDataSize(NewBufferSize);
        }

    if(Status)
        return H_SUCCESS;
    else
        return H_ERROR;
    }

HSTATUS HTIFFFile::UncompressBlockRLE1 (const HFCPtr<HCDPacket>& pi_rpPacket,
                                        Byte*       po_pDstBuffer,
                                        uint32_t*       po_pDstLen)
    {
    HFCMonitor Monitor(m_Key);
    size_t LineIndexesTableSize = ((HFCPtr<HCDCodecHMRRLE1>&)(pi_rpPacket->GetCodec()))->GetHeight() * sizeof(uint32_t);

    size_t NewSize = ((HFCPtr<HCDCodecHMRRLE1>&)(pi_rpPacket->GetCodec()))->DecompressSubset (   pi_rpPacket->GetBufferAddress() + LineIndexesTableSize,
                     (pi_rpPacket->GetDataSize() - LineIndexesTableSize),
                     po_pDstBuffer,
                     m_StripTileSize);

    if (NewSize == 0)
        {
        *po_pDstLen += *po_pDstLen;         // Double the Buffer
        return H_NOT_ENOUGH_MEMORY;
        }
    else
        {
        HASSERT_X64(NewSize < ULONG_MAX);
        *po_pDstLen = (uint32_t)NewSize;
        return H_SUCCESS;
        }
    }

size_t HTIFFFile::GetSubsetMaxCompressedSize (size_t pi_CurrentSize)
    {
    HFCMonitor Monitor(m_Key);

    if (m_pPacket->GetCodec() == 0)
        return pi_CurrentSize;
    else
        return m_pPacket->GetCodec()->GetSubsetMaxCompressedSize();
    }


HSTATUS HTIFFFile::SetHeight (uint32_t pi_Height)
    {
    HFCMonitor Monitor(m_Key);
    uint32_t Width;
    uint32_t Height;

    if (((HFCPtr<HCDCodecImage>&)(m_pPacket->GetCodec()))->GetHeight() != pi_Height)
        {
        ExtractWidthHeight (&Width, &Height);
        ((HFCPtr<HCDCodecImage>&)(m_pPacket->GetCodec()))->SetDimensions (Width, pi_Height);
        }

    return H_SUCCESS;
    }

//--------------------------------------------------- Privates

void HTIFFFile::ExtractWidthHeight (uint32_t* po_pWidth, uint32_t* po_pHeight)
    {
    HFCMonitor Monitor(m_Key);

    if (IsTiled())
        {
        GetField (TILEWIDTH,  po_pWidth);
        GetField (TILELENGTH, po_pHeight);
        }
    else
        {
        // Strip
        *po_pWidth = m_ImageWidth;
        *po_pHeight= m_RowsByStrip;
        }

    }
