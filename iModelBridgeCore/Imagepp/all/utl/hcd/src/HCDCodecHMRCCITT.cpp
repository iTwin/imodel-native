//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecHMRCCITT.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecHMRCCITT
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecHMRCCITT.h>



bool HCDCodecHMRCCITT::m_TablesBuilded = false;

void build_horiz_mode_tables();
void build_uncomp_mode_tables();
void build_null_mode_tables();

/*
 * Compression+decompression state blocks are
 * derived from this ``base state'' block.
 */
typedef struct
    {
    Byte  data;       // short   data;        /* current i/o byte             */
    Byte  bit;        // short   bit          /* current i/o bit in byte      */
    Byte  white;      // short   white;       /* value of the color ``white'' */
    uint32_t rowbytes;   // u_long  rowbytes;    /* XXX maybe should be a long?  */
    uint32_t rowpixels;  // u_long  rowpixels;   /* XXX maybe should be a long?  */
    enum
        {   /* decoding/encoding mode */
        G3_1D,          /* basic 1-d mode */
        G3_2D           /* optional 2-d mode */
        } tag;

    Byte* bitmap; //u_char    *bitmap;        /* bit reversal table */
    Byte* refline; //u_char   *refline;       /* reference line for 2d decoding */
    } Fax3BaseState;


//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecHMRCCITT::HCDCodecHMRCCITT()
    : HCDCodecCCITT()
    {
    m_pStateBlock = 0;

    m_yresolution = 0;
    m_resolutionunit = CCITT_RESUNIT_NONE;
    m_photometric = CCITT_PHOTOMETRIC_MINISWHITE;
    m_group3options = 0;
    m_options = CCITT_FAX3_CLASSF;
    m_bitrevtable = false;

    m_CCITT3 = true;
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecHMRCCITT::HCDCodecHMRCCITT(uint32_t pi_Width,
                                   uint32_t pi_Height)
    : HCDCodecCCITT(pi_Width,
                    pi_Height)
    {
    m_pStateBlock = 0;

    m_yresolution = 0;
    m_resolutionunit = CCITT_RESUNIT_NONE;
    m_photometric = CCITT_PHOTOMETRIC_MINISWHITE;
    m_group3options = 0;
    m_options = CCITT_FAX3_CLASSF;
    m_bitrevtable = false;

    m_CCITT3 = true;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecHMRCCITT::HCDCodecHMRCCITT(const HCDCodecHMRCCITT& pi_rObj)
    : HCDCodecCCITT(pi_rObj)
    {
    m_pStateBlock = 0;

    m_yresolution = pi_rObj.m_yresolution;
    m_resolutionunit = pi_rObj.m_resolutionunit;
    m_photometric = pi_rObj.m_photometric;
    m_group3options = pi_rObj.m_group3options;
    m_options = pi_rObj.m_options;
    m_bitrevtable = pi_rObj.m_bitrevtable;

    m_CCITT3 = pi_rObj.m_CCITT3;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecHMRCCITT::~HCDCodecHMRCCITT()
    {
    if(m_pStateBlock != 0)
        delete[] m_pStateBlock;
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecHMRCCITT::Clone() const
    {
    return new HCDCodecHMRCCITT(*this);
    }

typedef struct {
    Fax3BaseState b;
    Byte* wruns; //u_char    *wruns;
    Byte* bruns; //u_char    *bruns;
    short    k;            /* #rows left that can be 2d encoded */
    short    maxk;            /* max #rows that can be 2d encoded */
    } Fax3EncodeState;

#define    is2DEncoding() \
    (m_group3options & CCITT_GROUP3OPT_2DENCODING)

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecHMRCCITT::CompressSubset(const void* pi_pInData,
                                        size_t pi_InDataSize,
                                        void* po_pOutBuffer,
                                        size_t po_OutBufferSize)
    {
    if(!m_TablesBuilded)
        {
        build_null_mode_tables();
        build_uncomp_mode_tables();
        build_horiz_mode_tables();

        m_TablesBuilded = true;
        }

    size_t LinePaddingCompleteBytes = GetLinePaddingBits() / 8;

    m_prawcp = (Byte*)po_pOutBuffer;
    m_rawdatasize = po_OutBufferSize;
    m_rawcc = 0;

    size_t cc = (GetSubsetWidth() + 7) / 8 * GetSubsetHeight();

    // is it the first subset?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_COMPRESS);
        Fax3PreEncode();
        }

    Byte* pIn  = (Byte*)pi_pInData;

    Fax3EncodeState* sp = (Fax3EncodeState*)m_pStateBlock;

    while(cc > 0)
        {
        if(m_CCITT3)
            {
            Fax3PutEOL();

            if (is2DEncoding())
                {
                if (sp->b.tag == Fax3BaseState::G3_1D)
                    {
                    Fax3Encode1DRow(pIn, sp->b.rowpixels);

                    sp->b.tag = Fax3BaseState::G3_2D;
                    }
                else
                    {
                    Fax3Encode2DRow(pIn, sp->b.refline, sp->b.rowpixels);

                    sp->k--;
                    }
                if (sp->k == 0)
                    {
                    sp->b.tag = Fax3BaseState::G3_1D;
                    sp->k = sp->maxk-1;
                    }
                else
                    memcpy(sp->b.refline, pIn, sp->b.rowbytes);
                }
            else
                {
                Fax3Encode1DRow(pIn, sp->b.rowpixels);
                }
            }
        else
            {
            Fax3Encode2DRow(pIn, sp->b.refline, sp->b.rowpixels);
            memcpy(sp->b.refline, pIn, sp->b.rowbytes);
            }

        pIn += sp->b.rowbytes;
        pIn += LinePaddingCompleteBytes;

        cc -= sp->b.rowbytes;
        }

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    // is it the last subset?
    if(GetSubsetPosY() == GetHeight())
        {
        if(m_CCITT3)
            Fax3PostEncode();
        else
            Fax4PostEncode();

        Reset();
        }

    size_t OutDataSize = m_rawcc;

    return OutDataSize;
    }

typedef struct {
    Fax3BaseState b;
    } Fax3DecodeState;

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecHMRCCITT::DecompressSubset(const void* pi_pInData,
                                          size_t pi_InDataSize,
                                          void* po_pOutBuffer,
                                          size_t pi_OutBufferSize)
    {
    if(!m_TablesBuilded)
        {
        build_null_mode_tables();
        build_uncomp_mode_tables();
        build_horiz_mode_tables();

        m_TablesBuilded = true;
        }

    size_t LinePaddingCompleteBytes = GetLinePaddingBits() / 8;

    m_prawcp = (Byte*)pi_pInData;
    m_rawdatasize = pi_InDataSize;
    m_rawcc = pi_InDataSize;

    // is it the first subset?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        Fax3PreDecode();
        }

    Fax3DecodeState* sp = (Fax3DecodeState*)m_pStateBlock;

    Byte* pOut = (Byte*)po_pOutBuffer;

    size_t SubsetSize = (GetSubsetWidth() * GetBitsPerPixel()
                         + GetLinePaddingBits()) / 8 * GetSubsetHeight();

    HASSERT(SubsetSize <= pi_OutBufferSize);
    memset(pOut, 0, SubsetSize);

    for(uint32_t Line = 0; Line < GetSubsetHeight(); Line++)
        {
        if(m_CCITT3)
            {
            if (sp->b.tag == Fax3BaseState::G3_1D)
                Fax3Decode1DRow(pOut, sp->b.rowpixels);
            else
                Fax3Decode2DRow(pOut, sp->b.rowpixels);

            if (is2DEncoding()) {
                /*
                * Fetch the tag bit that indicates
                * whether the next row is 1d or 2d
                * encoded.  If 2d-encoded, then setup
                * the reference line from the decoded
                * scanline just completed.
                */
                sp->b.tag = nextbit() ? Fax3BaseState::G3_1D : Fax3BaseState::G3_2D;
                if (sp->b.tag == Fax3BaseState::G3_2D)
                    memcpy(sp->b.refline, pOut, sp->b.rowbytes);
                }
            }
        else
            {
            Fax3Decode2DRow(pOut, sp->b.rowpixels);

            memcpy(sp->b.refline, pOut, sp->b.rowbytes);
            }

        pOut += sp->b.rowbytes;

        pOut += LinePaddingCompleteBytes;
        }

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    // is it the last subset?
    if(GetSubsetPosY() == GetHeight())
        {
        Reset();
        }
    else
        {
        SetCompressedImageIndex(GetCompressedImageIndex() +
                                (pi_InDataSize - m_rawcc));
        }

    return SubsetSize;
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecHMRCCITT::HasLineAccess() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// public
// Reset
//-----------------------------------------------------------------------------
void HCDCodecHMRCCITT::Reset()
    {
    if(m_pStateBlock != 0)
        {
        delete[] m_pStateBlock;
        m_pStateBlock = 0;
        }

    HCDCodecCCITT::Reset();
    }

//-----------------------------------------------------------------------------
// public
// SetYResolution
//-----------------------------------------------------------------------------
void HCDCodecHMRCCITT::SetYResolution(float pi_Res)
    {
    m_yresolution = pi_Res;
    }

//-----------------------------------------------------------------------------
// public
// SetResolutionUnit
//-----------------------------------------------------------------------------
void HCDCodecHMRCCITT::SetResolutionUnit(unsigned short pi_Unit)
    {
    m_resolutionunit = pi_Unit;
    }

//-----------------------------------------------------------------------------
// public
// SetGroup3Options
//-----------------------------------------------------------------------------
void HCDCodecHMRCCITT::SetGroup3Options(int32_t pi_Options)
    {
    m_group3options = pi_Options;
    }

//-----------------------------------------------------------------------------
// public
// SetCCITT3
//-----------------------------------------------------------------------------
void HCDCodecHMRCCITT::SetCCITT3(bool pi_Enabled)
    {
    m_CCITT3 = pi_Enabled;

    if(!pi_Enabled)
        {
        SetGroup3Options(CCITT_GROUP3OPT_2DENCODING);
        SetOptions (CCITT_FAX3_NOEOL|CCITT_FAX3_CLASSF);
        }
    }


//-----------------------------------------------------------------------------
// public
// GetCCITT3
//-----------------------------------------------------------------------------
bool HCDCodecHMRCCITT::GetCCITT3() const
    {
    return (m_CCITT3);
    }

//-----------------------------------------------------------------------------
// public
// SetOptions
//-----------------------------------------------------------------------------
void HCDCodecHMRCCITT::SetOptions(int8_t pi_Options)
    {
    m_options = pi_Options;
    }

#define    EOL    0x001    /* EOL code value - 0000 0000 0000 1 */

#define    is2DEncoding() \
    (m_group3options & CCITT_GROUP3OPT_2DENCODING)

// Also used in HCDCodecCCITTFax4.cpp
unsigned char BitRevTable[256] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
    };

unsigned char NoBitRevTable[256] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
    0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
    0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    };

// Also used in HCDCodecCCITTFax4.cpp
Byte zeroruns[256] = {
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,    /* 0x00 - 0x0f */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,    /* 0x10 - 0x1f */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,    /* 0x20 - 0x2f */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,    /* 0x30 - 0x3f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    /* 0x40 - 0x4f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    /* 0x50 - 0x5f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    /* 0x60 - 0x6f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    /* 0x70 - 0x7f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0x80 - 0x8f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0x90 - 0x9f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0xa0 - 0xaf */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0xb0 - 0xbf */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0xc0 - 0xcf */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0xd0 - 0xdf */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0xe0 - 0xef */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0xf0 - 0xff */
    };

// Also used in HCDCodecCCITTFax4.cpp
Byte oneruns[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0x00 - 0x0f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0x10 - 0x1f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0x20 - 0x2f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0x30 - 0x3f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0x40 - 0x4f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0x50 - 0x5f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0x60 - 0x6f */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0x70 - 0x7f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    /* 0x80 - 0x8f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    /* 0x90 - 0x9f */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    /* 0xa0 - 0xaf */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,    /* 0xb0 - 0xbf */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,    /* 0xc0 - 0xcf */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,    /* 0xd0 - 0xdf */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,    /* 0xe0 - 0xef */
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 8,    /* 0xf0 - 0xff */
    };

//-----------------------------------------------------------------------------
// findspan: Find a span of ones or zeros using the supplied
//           table.  The byte-aligned start of the bit string
//           is supplied along with the start+end bit indices.
//           The table gives the number of consecutive ones or
//           zeros starting from the msb and is indexed by byte
//           value.
//-----------------------------------------------------------------------------
inline int HCDCodecHMRCCITT::findspan(Byte** bpp, int bs, int be, register Byte const* tab)
    {
    register Byte* bp = *bpp;
    register int bits = be - bs;
    register int n, span;

    /*
     * Check partial byte on lhs.
     */
    if (bits > 0 && (n = (bs & 7)))
        {
        span = tab[(*bp << n) & 0xff];
        if (span > 8-n)     /* table value too generous */
            span = 8-n;
        if (span > bits)    /* constrain span to bit range */
            span = bits;
        if (n+span < 8)     /* doesn't extend to edge of Byte */
            goto done;
        bits -= span;
        bp++;
        }
    else
        span = 0;
    /*
     * Scan full bytes for all 1's or all 0's.
     */
    while (bits >= 8)
        {
        n = tab[*bp];
        span += n;
        bits -= n;
        if (n < 8)      /* end of run */
            goto done;
        bp++;
        }
    /*
     * Check partial byte on rhs.
     */
    if (bits > 0) {
        n = tab[*bp];
        span += (n > bits ? bits : n);
        }
done:
    *bpp = bp;
    return (span);
    }

//-----------------------------------------------------------------------------
// finddiff: Return the offset of the next bit in the range
//           [bs..be] that is different from the specified
//           color.  The end, be, is returned if no such bit
//           exists.
//-----------------------------------------------------------------------------

inline int HCDCodecHMRCCITT::finddiff(Byte* cp, int bs, int be, int color)
    {
    cp += bs >> 3;            /* adjust Byte offset */
    return (bs + findspan(&cp, bs, be, color ? oneruns : zeroruns));
    }

/*
 * Reset encoding state at the start of a strip.
 */
void HCDCodecHMRCCITT::Fax3PreEncode()
    {
    Fax3EncodeState* sp = (Fax3EncodeState*)m_pStateBlock;

    if (sp == NULL) {
        Fax3SetupState(sizeof(Fax3EncodeState));

        sp = (Fax3EncodeState*)m_pStateBlock;

        if (!sp)
            return;
        if (sp->b.white == 0) {
            sp->wruns = zeroruns;
            sp->bruns = oneruns;
            }
        else {
            sp->wruns = oneruns;
            sp->bruns = zeroruns;
            }
        }
    sp->b.bit = 8;
    sp->b.data = 0;
    sp->b.tag = Fax3BaseState::G3_1D;
    /*
     * This is necessary for Group 4; otherwise it isn't
     * needed because the first scanline of each strip ends
     * up being copied into the refline.
     */
    if (sp->b.refline)
        memset(sp->b.refline, sp->b.white ? 0xff : 0x00, sp->b.rowbytes);

    if (is2DEncoding())
        {
        double res = m_yresolution;
        /*
         * The CCITT spec says that when doing 2d encoding, you
         * should only do it on K consecutive scanlines, where K
         * depends on the resolution of the image being encoded
         * (2 for <= 200 lpi, 4 for > 200 lpi).  Since the directory
         * code initializes td_yresolution to 0, this code will
         * select a K of 2 unless the YResolution tag is set
         * appropriately.  (Note also that we fudge a little here
         * and use 150 lpi to avoid problems with units conversion.)
         */
        if (m_resolutionunit == CCITT_RESUNIT_CENTIMETER)
            res = (res * .3937) / 2.54;    /* convert to inches */
        sp->maxk = (res > 150 ? 4 : 2);
        sp->k = sp->maxk-1;
        }
    else
        sp->k = sp->maxk = 0;
//    return (1);
    }

/*
 * Setup G3-related compression/decompression
 * state before data is processed.  This routine
 * is called once per image -- it sets up different
 * state based on whether or not 2D encoding is used.
 */
void HCDCodecHMRCCITT::Fax3SetupState(size_t space)
    {
    Fax3BaseState* sp;
    size_t cc = space;
    long rowbytes, rowpixels;

    rowbytes = static_cast<long>((GetSubsetWidth() * GetBitsPerPixel() + 7) / 8);
    rowpixels = static_cast<long>(GetSubsetWidth());

    if (is2DEncoding() || !m_CCITT3)
        cc += rowbytes + 1;

    m_pStateBlock = new Byte[cc];

    sp = (Fax3BaseState*)m_pStateBlock;
    sp->rowbytes = rowbytes;
    sp->rowpixels = rowpixels;

    sp->bitmap = m_bitrevtable ? BitRevTable : NoBitRevTable;

    sp->white = (m_photometric == CCITT_PHOTOMETRIC_MINISBLACK);

    if (is2DEncoding() || !m_CCITT3)
        {
        /*
         * 2d encoding/decoding requires a scanline
         * buffer for the ``reference line''; the
         * scanline against which delta encoding
         * is referenced.  The reference line must
         * be initialized to be ``white'' (done elsewhere).
         */
        sp->refline = m_pStateBlock + space + 1;
        /*
         * Initialize pixel just to the left of the
         * reference line to white.  This extra pixel
         * simplifies the edge-condition logic.
         */
        sp->refline[-1] = sp->white ? 0xff : 0x00;
        }
    else
        sp->refline = 0;
//    return (sp);
    }

static const CCITTtableentry horizcode =
    { 3, 0x1 };        /* 001 */
static const CCITTtableentry passcode =
    { 4, 0x1 };        /* 0001 */
static const CCITTtableentry vcodes[7] = {
        { 7, 0x03 },    /* 0000 011 */
        { 6, 0x03 },    /* 0000 11 */
        { 3, 0x03 },    /* 011 */
        { 1, 0x1 },        /* 1 */
        { 3, 0x2 },        /* 010 */
        { 6, 0x02 },    /* 0000 10 */
        { 7, 0x02 }        /* 0000 010 */
    };


/* status values returned instead of a run length */
#define    G3CODE_INVALID    -1
#define    G3CODE_INCOMP    -2
#define    G3CODE_EOL    -3
#define    G3CODE_EOF    -4

static const CCITTtableentry TIFFFaxWhiteCodes[] = {
        { 8, 0x35, 0 },    /* 0011 0101 */
        { 6, 0x7, 1 },    /* 0001 11 */
        { 4, 0x7, 2 },    /* 0111 */
        { 4, 0x8, 3 },    /* 1000 */
        { 4, 0xB, 4 },    /* 1011 */
        { 4, 0xC, 5 },    /* 1100 */
        { 4, 0xE, 6 },    /* 1110 */
        { 4, 0xF, 7 },    /* 1111 */
        { 5, 0x13, 8 },    /* 1001 1 */
        { 5, 0x14, 9 },    /* 1010 0 */
        { 5, 0x7, 10 },    /* 0011 1 */
        { 5, 0x8, 11 },    /* 0100 0 */
        { 6, 0x8, 12 },    /* 0010 00 */
        { 6, 0x3, 13 },    /* 0000 11 */
        { 6, 0x34, 14 },    /* 1101 00 */
        { 6, 0x35, 15 },    /* 1101 01 */
        { 6, 0x2A, 16 },    /* 1010 10 */
        { 6, 0x2B, 17 },    /* 1010 11 */
        { 7, 0x27, 18 },    /* 0100 111 */
        { 7, 0xC, 19 },    /* 0001 100 */
        { 7, 0x8, 20 },    /* 0001 000 */
        { 7, 0x17, 21 },    /* 0010 111 */
        { 7, 0x3, 22 },    /* 0000 011 */
        { 7, 0x4, 23 },    /* 0000 100 */
        { 7, 0x28, 24 },    /* 0101 000 */
        { 7, 0x2B, 25 },    /* 0101 011 */
        { 7, 0x13, 26 },    /* 0010 011 */
        { 7, 0x24, 27 },    /* 0100 100 */
        { 7, 0x18, 28 },    /* 0011 000 */
        { 8, 0x2, 29 },    /* 0000 0010 */
        { 8, 0x3, 30 },    /* 0000 0011 */
        { 8, 0x1A, 31 },    /* 0001 1010 */
        { 8, 0x1B, 32 },    /* 0001 1011 */
        { 8, 0x12, 33 },    /* 0001 0010 */
        { 8, 0x13, 34 },    /* 0001 0011 */
        { 8, 0x14, 35 },    /* 0001 0100 */
        { 8, 0x15, 36 },    /* 0001 0101 */
        { 8, 0x16, 37 },    /* 0001 0110 */
        { 8, 0x17, 38 },    /* 0001 0111 */
        { 8, 0x28, 39 },    /* 0010 1000 */
        { 8, 0x29, 40 },    /* 0010 1001 */
        { 8, 0x2A, 41 },    /* 0010 1010 */
        { 8, 0x2B, 42 },    /* 0010 1011 */
        { 8, 0x2C, 43 },    /* 0010 1100 */
        { 8, 0x2D, 44 },    /* 0010 1101 */
        { 8, 0x4, 45 },    /* 0000 0100 */
        { 8, 0x5, 46 },    /* 0000 0101 */
        { 8, 0xA, 47 },    /* 0000 1010 */
        { 8, 0xB, 48 },    /* 0000 1011 */
        { 8, 0x52, 49 },    /* 0101 0010 */
        { 8, 0x53, 50 },    /* 0101 0011 */
        { 8, 0x54, 51 },    /* 0101 0100 */
        { 8, 0x55, 52 },    /* 0101 0101 */
        { 8, 0x24, 53 },    /* 0010 0100 */
        { 8, 0x25, 54 },    /* 0010 0101 */
        { 8, 0x58, 55 },    /* 0101 1000 */
        { 8, 0x59, 56 },    /* 0101 1001 */
        { 8, 0x5A, 57 },    /* 0101 1010 */
        { 8, 0x5B, 58 },    /* 0101 1011 */
        { 8, 0x4A, 59 },    /* 0100 1010 */
        { 8, 0x4B, 60 },    /* 0100 1011 */
        { 8, 0x32, 61 },    /* 0011 0010 */
        { 8, 0x33, 62 },    /* 0011 0011 */
        { 8, 0x34, 63 },    /* 0011 0100 */
        { 5, 0x1B, 64 },    /* 1101 1 */
        { 5, 0x12, 128 },    /* 1001 0 */
        { 6, 0x17, 192 },    /* 0101 11 */
        { 7, 0x37, 256 },    /* 0110 111 */
        { 8, 0x36, 320 },    /* 0011 0110 */
        { 8, 0x37, 384 },    /* 0011 0111 */
        { 8, 0x64, 448 },    /* 0110 0100 */
        { 8, 0x65, 512 },    /* 0110 0101 */
        { 8, 0x68, 576 },    /* 0110 1000 */
        { 8, 0x67, 640 },    /* 0110 0111 */
        { 9, 0xCC, 704 },    /* 0110 0110 0 */
        { 9, 0xCD, 768 },    /* 0110 0110 1 */
        { 9, 0xD2, 832 },    /* 0110 1001 0 */
        { 9, 0xD3, 896 },    /* 0110 1001 1 */
        { 9, 0xD4, 960 },    /* 0110 1010 0 */
        { 9, 0xD5, 1024 },    /* 0110 1010 1 */
        { 9, 0xD6, 1088 },    /* 0110 1011 0 */
        { 9, 0xD7, 1152 },    /* 0110 1011 1 */
        { 9, 0xD8, 1216 },    /* 0110 1100 0 */
        { 9, 0xD9, 1280 },    /* 0110 1100 1 */
        { 9, 0xDA, 1344 },    /* 0110 1101 0 */
        { 9, 0xDB, 1408 },    /* 0110 1101 1 */
        { 9, 0x98, 1472 },    /* 0100 1100 0 */
        { 9, 0x99, 1536 },    /* 0100 1100 1 */
        { 9, 0x9A, 1600 },    /* 0100 1101 0 */
        { 6, 0x18, 1664 },    /* 0110 00 */
        { 9, 0x9B, 1728 },    /* 0100 1101 1 */
        { 11, 0x8, 1792 },    /* 0000 0001 000 */
        { 11, 0xC, 1856 },    /* 0000 0001 100 */
        { 11, 0xD, 1920 },    /* 0000 0001 101 */
        { 12, 0x12, 1984 },    /* 0000 0001 0010 */
        { 12, 0x13, 2048 },    /* 0000 0001 0011 */
        { 12, 0x14, 2112 },    /* 0000 0001 0100 */
        { 12, 0x15, 2176 },    /* 0000 0001 0101 */
        { 12, 0x16, 2240 },    /* 0000 0001 0110 */
        { 12, 0x17, 2304 },    /* 0000 0001 0111 */
        { 12, 0x1C, 2368 },    /* 0000 0001 1100 */
        { 12, 0x1D, 2432 },    /* 0000 0001 1101 */
        { 12, 0x1E, 2496 },    /* 0000 0001 1110 */
        { 12, 0x1F, 2560 },    /* 0000 0001 1111 */
        { 12, 0x1, G3CODE_EOL },    /* 0000 0000 0001 */
        { 9, 0x1, G3CODE_INVALID },    /* 0000 0000 1 */
        { 10, 0x1, G3CODE_INVALID },    /* 0000 0000 01 */
        { 11, 0x1, G3CODE_INVALID },    /* 0000 0000 001 */
        { 12, 0x0, G3CODE_INVALID },    /* 0000 0000 0000 */
    };

static const CCITTtableentry TIFFFaxBlackCodes[] = {
        { 10, 0x37, 0 },    /* 0000 1101 11 */
        { 3, 0x2, 1 },    /* 010 */
        { 2, 0x3, 2 },    /* 11 */
        { 2, 0x2, 3 },    /* 10 */
        { 3, 0x3, 4 },    /* 011 */
        { 4, 0x3, 5 },    /* 0011 */
        { 4, 0x2, 6 },    /* 0010 */
        { 5, 0x3, 7 },    /* 0001 1 */
        { 6, 0x5, 8 },    /* 0001 01 */
        { 6, 0x4, 9 },    /* 0001 00 */
        { 7, 0x4, 10 },    /* 0000 100 */
        { 7, 0x5, 11 },    /* 0000 101 */
        { 7, 0x7, 12 },    /* 0000 111 */
        { 8, 0x4, 13 },    /* 0000 0100 */
        { 8, 0x7, 14 },    /* 0000 0111 */
        { 9, 0x18, 15 },    /* 0000 1100 0 */
        { 10, 0x17, 16 },    /* 0000 0101 11 */
        { 10, 0x18, 17 },    /* 0000 0110 00 */
        { 10, 0x8, 18 },    /* 0000 0010 00 */
        { 11, 0x67, 19 },    /* 0000 1100 111 */
        { 11, 0x68, 20 },    /* 0000 1101 000 */
        { 11, 0x6C, 21 },    /* 0000 1101 100 */
        { 11, 0x37, 22 },    /* 0000 0110 111 */
        { 11, 0x28, 23 },    /* 0000 0101 000 */
        { 11, 0x17, 24 },    /* 0000 0010 111 */
        { 11, 0x18, 25 },    /* 0000 0011 000 */
        { 12, 0xCA, 26 },    /* 0000 1100 1010 */
        { 12, 0xCB, 27 },    /* 0000 1100 1011 */
        { 12, 0xCC, 28 },    /* 0000 1100 1100 */
        { 12, 0xCD, 29 },    /* 0000 1100 1101 */
        { 12, 0x68, 30 },    /* 0000 0110 1000 */
        { 12, 0x69, 31 },    /* 0000 0110 1001 */
        { 12, 0x6A, 32 },    /* 0000 0110 1010 */
        { 12, 0x6B, 33 },    /* 0000 0110 1011 */
        { 12, 0xD2, 34 },    /* 0000 1101 0010 */
        { 12, 0xD3, 35 },    /* 0000 1101 0011 */
        { 12, 0xD4, 36 },    /* 0000 1101 0100 */
        { 12, 0xD5, 37 },    /* 0000 1101 0101 */
        { 12, 0xD6, 38 },    /* 0000 1101 0110 */
        { 12, 0xD7, 39 },    /* 0000 1101 0111 */
        { 12, 0x6C, 40 },    /* 0000 0110 1100 */
        { 12, 0x6D, 41 },    /* 0000 0110 1101 */
        { 12, 0xDA, 42 },    /* 0000 1101 1010 */
        { 12, 0xDB, 43 },    /* 0000 1101 1011 */
        { 12, 0x54, 44 },    /* 0000 0101 0100 */
        { 12, 0x55, 45 },    /* 0000 0101 0101 */
        { 12, 0x56, 46 },    /* 0000 0101 0110 */
        { 12, 0x57, 47 },    /* 0000 0101 0111 */
        { 12, 0x64, 48 },    /* 0000 0110 0100 */
        { 12, 0x65, 49 },    /* 0000 0110 0101 */
        { 12, 0x52, 50 },    /* 0000 0101 0010 */
        { 12, 0x53, 51 },    /* 0000 0101 0011 */
        { 12, 0x24, 52 },    /* 0000 0010 0100 */
        { 12, 0x37, 53 },    /* 0000 0011 0111 */
        { 12, 0x38, 54 },    /* 0000 0011 1000 */
        { 12, 0x27, 55 },    /* 0000 0010 0111 */
        { 12, 0x28, 56 },    /* 0000 0010 1000 */
        { 12, 0x58, 57 },    /* 0000 0101 1000 */
        { 12, 0x59, 58 },    /* 0000 0101 1001 */
        { 12, 0x2B, 59 },    /* 0000 0010 1011 */
        { 12, 0x2C, 60 },    /* 0000 0010 1100 */
        { 12, 0x5A, 61 },    /* 0000 0101 1010 */
        { 12, 0x66, 62 },    /* 0000 0110 0110 */
        { 12, 0x67, 63 },    /* 0000 0110 0111 */
        { 10, 0xF, 64 },    /* 0000 0011 11 */
        { 12, 0xC8, 128 },    /* 0000 1100 1000 */
        { 12, 0xC9, 192 },    /* 0000 1100 1001 */
        { 12, 0x5B, 256 },    /* 0000 0101 1011 */
        { 12, 0x33, 320 },    /* 0000 0011 0011 */
        { 12, 0x34, 384 },    /* 0000 0011 0100 */
        { 12, 0x35, 448 },    /* 0000 0011 0101 */
        { 13, 0x6C, 512 },    /* 0000 0011 0110 0 */
        { 13, 0x6D, 576 },    /* 0000 0011 0110 1 */
        { 13, 0x4A, 640 },    /* 0000 0010 0101 0 */
        { 13, 0x4B, 704 },    /* 0000 0010 0101 1 */
        { 13, 0x4C, 768 },    /* 0000 0010 0110 0 */
        { 13, 0x4D, 832 },    /* 0000 0010 0110 1 */
        { 13, 0x72, 896 },    /* 0000 0011 1001 0 */
        { 13, 0x73, 960 },    /* 0000 0011 1001 1 */
        { 13, 0x74, 1024 },    /* 0000 0011 1010 0 */
        { 13, 0x75, 1088 },    /* 0000 0011 1010 1 */
        { 13, 0x76, 1152 },    /* 0000 0011 1011 0 */
        { 13, 0x77, 1216 },    /* 0000 0011 1011 1 */
        { 13, 0x52, 1280 },    /* 0000 0010 1001 0 */
        { 13, 0x53, 1344 },    /* 0000 0010 1001 1 */
        { 13, 0x54, 1408 },    /* 0000 0010 1010 0 */
        { 13, 0x55, 1472 },    /* 0000 0010 1010 1 */
        { 13, 0x5A, 1536 },    /* 0000 0010 1101 0 */
        { 13, 0x5B, 1600 },    /* 0000 0010 1101 1 */
        { 13, 0x64, 1664 },    /* 0000 0011 0010 0 */
        { 13, 0x65, 1728 },    /* 0000 0011 0010 1 */
        { 11, 0x8, 1792 },    /* 0000 0001 000 */
        { 11, 0xC, 1856 },    /* 0000 0001 100 */
        { 11, 0xD, 1920 },    /* 0000 0001 101 */
        { 12, 0x12, 1984 },    /* 0000 0001 0010 */
        { 12, 0x13, 2048 },    /* 0000 0001 0011 */
        { 12, 0x14, 2112 },    /* 0000 0001 0100 */
        { 12, 0x15, 2176 },    /* 0000 0001 0101 */
        { 12, 0x16, 2240 },    /* 0000 0001 0110 */
        { 12, 0x17, 2304 },    /* 0000 0001 0111 */
        { 12, 0x1C, 2368 },    /* 0000 0001 1100 */
        { 12, 0x1D, 2432 },    /* 0000 0001 1101 */
        { 12, 0x1E, 2496 },    /* 0000 0001 1110 */
        { 12, 0x1F, 2560 },    /* 0000 0001 1111 */
        { 12, 0x1, G3CODE_EOL },    /* 0000 0000 0001 */
        { 9, 0x1, G3CODE_INVALID },    /* 0000 0000 1 */
        { 10, 0x1, G3CODE_INVALID },    /* 0000 0000 01 */
        { 11, 0x1, G3CODE_INVALID },    /* 0000 0000 001 */
        { 12, 0x0, G3CODE_INVALID },    /* 0000 0000 0000 */
    };

/*
 * 2d-encode a row of pixels.  Consult the CCITT
 * documentation for the algorithm.
 */
void HCDCodecHMRCCITT::Fax3Encode2DRow(Byte* bp, Byte* rp, int32_t bits)
    {
#define    PIXEL(buf,ix)    ((((buf)[(ix)>>3]) >> (7-((ix)&7))) & 1)
    short white = ((Fax3BaseState*)m_pStateBlock)->white;
    int a0 = 0;
    int a1 = (PIXEL(bp, 0) != white ? 0 : finddiff(bp, 0, bits, white));
    int b1 = (PIXEL(rp, 0) != white ? 0 : finddiff(rp, 0, bits, white));
    int a2, b2;

    for (;;) {
        b2 = finddiff(rp, b1, bits, PIXEL(rp,b1));
        if (b2 >= a1) {
            int d = b1 - a1;
            if (!(-3 <= d && d <= 3)) {    /* horizontal mode */
                a2 = (a1 >= bits) ? a1 : finddiff(bp, a1, bits, PIXEL(bp,a1));
                putcode(&horizcode);
                if (a0+a1 == 0 || PIXEL(bp, a0) == white) {
                    putspan(a1-a0, TIFFFaxWhiteCodes);
                    putspan(a2-a1, TIFFFaxBlackCodes);
                    }
                else {
                    putspan(a1-a0, TIFFFaxBlackCodes);
                    putspan(a2-a1, TIFFFaxWhiteCodes);
                    }
                a0 = a2;
                }
            else {              /* vertical mode */
                putcode(&vcodes[d+3]);
                a0 = a1;
                }
            }
        else {                  /* pass mode */
            putcode(&passcode);
            a0 = b2;
            }
        if (a0 >= bits)
            break;
        a1 = finddiff(bp, a0, bits, PIXEL(bp,a0));
        b1 = finddiff(rp, a0, bits, !PIXEL(bp,a0));
        b1 = finddiff(rp, b1, bits, PIXEL(bp,a0));
        }
#undef PIXEL
    }


/*
 * Write a code to the output stream.
 */
void HCDCodecHMRCCITT::putcode(CCITTtableentry const* te)
    {
    putbits(te->code, te->length);
    }

/*
 * Write the sequence of codes that describes
 * the specified span of zero's or one's.  The
 * appropriate table that holds the make-up and
 * terminating codes is supplied.
 */
void HCDCodecHMRCCITT::putspan(int span, CCITTtableentry const* tab)
    {
    while (span >= 2624) {
        CCITTtableentry const* te = &tab[63 + (2560>>6)];
        putcode(te);
        span -= te->runlen;
        }
    if (span >= 64) {
        CCITTtableentry const* te = &tab[63 + (span>>6)];
        HASSERT(te->runlen == 64*(span>>6));
        putcode(te);
        span -= te->runlen;
        }
    putcode(&tab[span]);
    }

#define    Fax3FlushBits(sp) {            \
    *m_prawcp++ = (sp)->bitmap[(sp)->data];    \
    m_rawcc++;                \
    (sp)->data = 0;                    \
    (sp)->bit = 8;                    \
}




/*
 * Write a variable-length bit-value to
 * the output stream.  Values are
 * assumed to be at most 16 bits.
 */
void HCDCodecHMRCCITT::putbits(unsigned short bits, unsigned short length)
    {
    Fax3BaseState* sp = (Fax3BaseState*)m_pStateBlock;
    static const int mask[9] =
        { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };

    while (length > sp->bit) {
        sp->data |= bits >> (length - sp->bit);
        length -= sp->bit;
        Fax3FlushBits(sp);
        }
    sp->data |= (bits & mask[length]) << (sp->bit - length);
    sp->bit = (Byte)(sp->bit - length);
    if (sp->bit == 0)
        Fax3FlushBits(sp);
    }

/*
 * Write an EOL code to the output stream.  The zero-fill
 * logic for byte-aligning encoded scanlines is handled
 * here.  We also handle writing the tag bit for the next
 * scanline when doing 2d encoding.
 */
void HCDCodecHMRCCITT::Fax3PutEOL()
    {
    Fax3BaseState* sp = (Fax3BaseState*)m_pStateBlock;

    if (m_group3options & CCITT_GROUP3OPT_FILLBITS) {
        /*
         * Force bit alignment so EOL will terminate on
         * a byte boundary.  That is, force the bit alignment
         * to 16-12 = 4 before putting out the EOL code.
         */
        unsigned short align = 8 - 4;
        if (align != sp->bit) {
            if (align > sp->bit)
                align = sp->bit + (8 - align);
            else
                align = sp->bit - align;
            putbits(0, align);
            }
        }
    putbits(EOL, 12);

    if (is2DEncoding())
        putbits(sp->tag == Fax3BaseState::G3_1D, 1);
    }


void HCDCodecHMRCCITT::Fax4PostEncode()
    {
    Fax3BaseState* sp = (Fax3BaseState*)m_pStateBlock;

    putbits(EOL, 12);
    putbits(EOL, 12);

    if (sp->bit != 8)
        Fax3FlushBits(sp);
    }

/*
 * Setup state for decoding a strip.
 */
void HCDCodecHMRCCITT::Fax3PreDecode()
    {
    Fax3DecodeState* sp = (Fax3DecodeState*)m_pStateBlock;

    if (sp == NULL) {
        Fax3SetupState(sizeof (*sp));

        sp = (Fax3DecodeState*)m_pStateBlock;

        if (!sp)
            return; //return(0);
        }
    sp->b.bit = 0;            /* force initial read */
    sp->b.data = 0;
    sp->b.tag = Fax3BaseState::G3_1D;
    if (sp->b.refline)
        memset(sp->b.refline, sp->b.white ? 0xff : 0x00, sp->b.rowbytes);
    /*
     * If image has EOL codes, they precede each line
     * of data.  We skip over the first one here so that
     * when we decode rows, we can use an EOL to signal
     * that less than the expected number of pixels are
     * present for the scanline.
     */
    if ((m_options & CCITT_FAX3_NOEOL) == 0) {
        skiptoeol(0);
        if (is2DEncoding())
            /* tag should always be 1D! */
            sp->b.tag = nextbit() ? Fax3BaseState::G3_1D : Fax3BaseState::G3_2D;
        }
    }

#define    BITCASE(b)            \
    case b:                \
    code <<= 1;            \
    if (data & (1<<(7-b))) code |= 1;\
    len++;                \
    if (code > 0) { bit = b+1; break; }

#define    fetchByte(sp) \
    (m_rawcc--, (sp)->b.bitmap[*(Byte*)m_prawcp++])

/*
 * Skip over input until an EOL code is found.  The
 * value of len is passed as 0 except during error
 * recovery when decoding 2D data.  Note also that
 * we don't use the optimized state tables to locate
 * an EOL because we can't assume much of anything
 * about our state (e.g. bit position).
 */
void HCDCodecHMRCCITT::skiptoeol(int len)
    {
    Fax3DecodeState* sp = (Fax3DecodeState*)m_pStateBlock;
    register int bit = sp->b.bit;
    register int data = sp->b.data;
    int code = 0;

    /*
     * Our handling of ``bit'' is painful because
     * the rest of the code does not maintain it as
     * exactly the bit offset in the current data
     * byte (bit == 0 means refill the data byte).
     * Thus we have to be careful on entry and
     * exit to insure that we maintain a value that's
     * understandable elsewhere in the decoding logic.
     */
    if (bit == 0)            /* force refill */
        bit = 8;
    for (;;) {
        switch (bit) {
again:
                BITCASE(0);
                BITCASE(1);
                BITCASE(2);
                BITCASE(3);
                BITCASE(4);
                BITCASE(5);
                BITCASE(6);
                BITCASE(7);
            default:
                if (m_rawcc <= 0)
                    return;
                data = fetchByte(sp);
                goto again;
            }
        if (len >= 12 && code == EOL)
            break;
        code = len = 0;
        }
    sp->b.bit = (Byte)(bit > 7 ? 0 : bit);    /* force refill */
    sp->b.data = (Byte)data;
    }

static Byte bitMask[8] =
    { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
#define    isBitSet(sp)    ((sp)->b.data & bitMask[(sp)->b.bit])

/*
 * Return the next bit in the input stream.  This is
 * used to extract 2D tag values and the color tag
 * at the end of a terminating uncompressed data code.
 */
int HCDCodecHMRCCITT::nextbit()
    {
    Fax3DecodeState* sp = (Fax3DecodeState*)m_pStateBlock;
    int bit;

    if (sp->b.bit == 0 && m_rawcc > 0)
        sp->b.data = fetchByte(sp);
    bit = isBitSet(sp);
    if (++(sp->b.bit) > 7)
        sp->b.bit = 0;
    return (bit);
    }

/*
 * G3 2D and G4 decoding modes.  Note that
 * the vertical modes are ordered so that
 * (mode - MODE_VERT_V0) gives the vertical
 * adjustment for the b1 parameter.
 */
#define MODE_NULL    0
#define MODE_PASS    1
#define MODE_HORIZ    2
#define MODE_VERT_VL3    3
#define MODE_VERT_VL2    4
#define MODE_VERT_VL1    5
#define MODE_VERT_V0    6
#define MODE_VERT_VR1    7
#define MODE_VERT_VR2    8
#define MODE_VERT_VR3    9
#define MODE_UNCOMP    10
#define MODE_ERROR    11
#define MODE_ERROR_1    12

/*
 * 2D uncompressed mode codes.  Note
 * that two groups of codes are arranged
 * so that the decoder can caluclate the
 * length of the run by subtracting the
 * code from a known base value.
 */
#define    UNCOMP_INCOMP    0
/* runs of [0]*1 */
#define    UNCOMP_RUN0    1
#define    UNCOMP_RUN1    2
#define    UNCOMP_RUN2    3
#define    UNCOMP_RUN3    4
#define    UNCOMP_RUN4    5
#define    UNCOMP_RUN5    6
#define    UNCOMP_RUN6    7
/* runs of [0]* w/ terminating color */
#define    UNCOMP_TRUN0    8
#define    UNCOMP_TRUN1    9
#define    UNCOMP_TRUN2    10
#define    UNCOMP_TRUN3    11
#define    UNCOMP_TRUN4    12
/* special code for unexpected EOF */
#define    UNCOMP_EOF    13
/* invalid code encountered */
#define    UNCOMP_INVALID    14

#define MAX_NULLPREFIX    200    /* max # of null-mode prefixes */
typedef    unsigned char NullModeTable[MAX_NULLPREFIX][256];
NullModeTable null_mode;
#define TIFFFax2DMode null_mode
NullModeTable null_mode_next_state;
#define TIFFFax2DNextState null_mode_next_state

#define UNCOMP_EXIT    UNCOMP_TRUN0

#define MAX_HORIZPREFIX    250    /* max # of incomplete 1-D prefixes */
typedef    unsigned char HorizModeTable[MAX_HORIZPREFIX][256];
HorizModeTable horiz_mode;
#define TIFFFax1DAction horiz_mode

HorizModeTable horiz_mode_next_state;
#define TIFFFax1DNextState horiz_mode_next_state

//-----------------------------------------------------------------------------
// fillspan: Fill a span with ones.
//-----------------------------------------------------------------------------

inline void HCDCodecHMRCCITT::fillspan(Byte* cp, int x, int count)
    {
    static const unsigned char masks[] =  { 0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

    if (count <= 0)
        return;

    cp += x>>3;
    if (x &= 7)             /* align to Byte boundary */
        {
        if (count < 8 - x)
            {
            *cp++ |= masks[count] >> x;
            return;
            }
        *cp++ |= 0xff >> x;
        count -= 8 - x;
        }

    while (count >= 8)
        {
        *cp++ = 0xff;
        count -= 8;
        }

    if (count > 0)
        *cp |= masks[count];
    }

/*
*
 * Process one row of 2d encoded data.
 */
int HCDCodecHMRCCITT::Fax3Decode2DRow(Byte* buf, int npels)
    {
#define    PIXEL(buf,ix)    ((((buf)[(ix)>>3]) >> (7-((ix)&7))) & 1)
    Fax3DecodeState* sp = (Fax3DecodeState*)m_pStateBlock;
    int a0 = -1;
    int b1, b2;
    int run1, run2;        /* for horizontal mode */
    short mode;
    short color = sp->b.white;
    static char module[] = "Fax3Decode2D";

    do {
        if (sp->b.bit == 0 || sp->b.bit > 7) {
            if (m_rawcc <= 0) {

                //TIFFError(module,
                //"%s: Premature EOF at scanline %d",
                //  tif->tif_name, tif->tif_row);
                return (0);
                }
            sp->b.data = fetchByte(sp);
            }
        mode = TIFFFax2DMode[sp->b.bit][sp->b.data];
        sp->b.bit = TIFFFax2DNextState[sp->b.bit][sp->b.data];
        switch (mode) {
            case MODE_NULL:
                break;
            case MODE_PASS:
                b2 = finddiff(sp->b.refline, a0, npels, !color);
                b1 = finddiff(sp->b.refline, b2, npels, color);
                b2 = finddiff(sp->b.refline, b1, npels, !color);
                if (color) {
                    if (a0 < 0)
                        a0 = 0;
                    fillspan((Byte*)buf, a0, b2 - a0);
                    }
                a0 = b2;
                break;
            case MODE_HORIZ:
                if (color == sp->b.white) {
                    run1 = decode_white_run();
                    run2 = decode_black_run();
                    }
                else {
                    run1 = decode_black_run();
                    run2 = decode_white_run();
                    }
                /*
                 * Do the appropriate fill.  Note that we exit
                 * this logic with the same color that we enter
                 * with since we do 2 fills.  This explains the
                 * somewhat obscure logic below.
                 */
                if (a0 < 0)
                    a0 = 0;
                if (a0 + run1 > npels)
                    run1 = npels - a0;
                if (color)
                    fillspan((Byte*)buf, a0, run1);
                a0 += run1;
                if (a0 + run2 > npels)
                    run2 = npels - a0;
                if (!color)
                    fillspan((Byte*)buf, a0, run2);
                a0 += run2;
                break;
            case MODE_VERT_V0:
            case MODE_VERT_VR1:
            case MODE_VERT_VR2:
            case MODE_VERT_VR3:
            case MODE_VERT_VL1:
            case MODE_VERT_VL2:
            case MODE_VERT_VL3:
                b2 = finddiff(sp->b.refline, a0, npels, !color);
                b1 = finddiff(sp->b.refline, b2, npels, color);
                b1 += mode - MODE_VERT_V0;
                if (color) {
                    if (a0 < 0)
                        a0 = 0;
                    fillspan((Byte*)buf, a0, b1 - a0);
                    }
                color = !color;
                a0 = b1;
                break;
            case MODE_UNCOMP:
                /*
                 * Uncompressed mode: select from the
                 * special set of code words.
                 */
                if (a0 < 0)
                    a0 = 0;
                do {
                    mode = (short)decode_uncomp_code();
                    switch (mode) {
                        case UNCOMP_RUN1:
                        case UNCOMP_RUN2:
                        case UNCOMP_RUN3:
                        case UNCOMP_RUN4:
                        case UNCOMP_RUN5:
                            run1 = mode - UNCOMP_RUN0;
                            fillspan((Byte*)buf, a0+run1-1, 1);
                            a0 += run1;
                            break;
                        case UNCOMP_RUN6:
                            a0 += 5;
                            break;
                        case UNCOMP_TRUN0:
                        case UNCOMP_TRUN1:
                        case UNCOMP_TRUN2:
                        case UNCOMP_TRUN3:
                        case UNCOMP_TRUN4:
                            run1 = mode - UNCOMP_TRUN0;
                            a0 += run1;
                            color = nextbit() ?
                                    !sp->b.white : sp->b.white;
                            break;
                        case UNCOMP_INVALID:
                            ///TIFFError(module,
                            //"%s: Bad uncompressed code word at scanline %d",
                            //        tif->tif_name, tif->tif_row);
                            goto bad;
                        case UNCOMP_EOF:
                            //    TIFFError(module,
                            //        "%s: Premature EOF at scanline %d",
                            //        tif->tif_name, tif->tif_row);
                            return (0);
                        }
                    }
                while (mode < UNCOMP_EXIT);
                break;
            case MODE_ERROR_1:
                if ((m_options & CCITT_FAX3_NOEOL) == 0) {
                    //TIFFWarning(module,
                    //  "%s: Premature EOL at scanline %d (x %d)",
                    // tif->tif_name, tif->tif_row, a0);
                    skiptoeol(7);    /* seen 7 0's already */
                    return (1);        /* try to synchronize */
                    }
                /* fall thru... */
            case MODE_ERROR:
                //TIFFError(module,
                //  "%s: Bad 2D code word at scanline %d",
                //tif->tif_name, tif->tif_row);
                goto bad;
            default:
                //TIFFError(module,
                //  "%s: Panic, bad decoding state at scanline %d",
                // tif->tif_name, tif->tif_row);
                return (0);
            }
        }
    while (a0 < npels);
bad:
    /*
     * Cleanup at the end of row.  We check for
     * EOL separately so that this code can be
     * reused by the Group 4 decoding routine.
     */
    if ((m_options & CCITT_FAX3_NOEOL) == 0)
        skiptoeol(0);
    return (a0 >= npels);
#undef    PIXEL
    }

/*
 * Fill a span with ones.
 */
/********************************************************** To be place elsewhre SEB...
void HCDCodecHMRCCITT::fillspan(Byte* cp, int x, int count)
{
    static const unsigned char masks[] =  { 0, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

    if (count <= 0)
        return;

    cp += x>>3;
    if (x &= 7)
    {
        if (count < 8 - x)
        {
            *cp++ |= masks[count] >> x;
            return;
        }
        *cp++ |= 0xff >> x;
        count -= 8 - x;
    }

    while (count >= 8)
    {
        *cp++ = 0xff;
        count -= 8;
    }
    *cp |= masks[count];
}
**************************************************************/

/*
* Decoding action values for horiz_mode.
*/
#define ACT_INCOMP    0        /* incompletely decoded code */
#define ACT_INVALID    1        /* invalide code */
#define    ACT_WRUNT    2        /* terminating white run code */
#define    ACT_WRUN    65        /* non-terminating white run code */
#define    ACT_BRUNT    106        /* terminating black run code */
#define    ACT_BRUN    169        /* non-terminating black run code */
#define ACT_EOL        210        /* end-of-line code */

#define RUNLENGTH(ix)    (TIFFFaxWhiteCodes[ix].runlen)

/*
* Decode a run of white.
*/
int HCDCodecHMRCCITT::decode_white_run()
    {
    Fax3DecodeState* sp = (Fax3DecodeState*)m_pStateBlock;
    short state = sp->b.bit;
    short action;
    int runlen = 0;

    for (;;) {
        if (sp->b.bit == 0) {
nextbyte:
            if (m_rawcc <= 0)
                return (G3CODE_EOF);
            sp->b.data = fetchByte(sp);
            }
        action = TIFFFax1DAction[state][sp->b.data];
        state = TIFFFax1DNextState[state][sp->b.data];
        if (action == ACT_INCOMP)
            goto nextbyte;
        if (action == ACT_INVALID)
            return (G3CODE_INVALID);
        if (action == ACT_EOL)
            return (G3CODE_EOL);
        sp->b.bit = (Byte)state;
        action = RUNLENGTH(action - ACT_WRUNT);
        runlen += action;
        if (action < 64)
            return (runlen);
        }
    /*NOTREACHED*/
    }

NullModeTable uncomp_mode;
NullModeTable uncomp_mode_next_state;
#define TIFFFaxUncompAction uncomp_mode
#define TIFFFaxUncompNextState uncomp_mode_next_state

/*
 * Decode a run of black.
 */
int HCDCodecHMRCCITT::decode_black_run()
    {
    Fax3DecodeState* sp = (Fax3DecodeState*)m_pStateBlock;
    short state = sp->b.bit + 8;
    short action;
    int runlen = 0;

    for (;;) {
        if (sp->b.bit == 0) {
nextbyte:
            if (m_rawcc <= 0)
                return (G3CODE_EOF);
            sp->b.data = fetchByte(sp);
            }
        action = TIFFFax1DAction[state][sp->b.data];
        state = TIFFFax1DNextState[state][sp->b.data];
        if (action == ACT_INCOMP)
            goto nextbyte;
        if (action == ACT_INVALID)
            return (G3CODE_INVALID);
        if (action == ACT_EOL)
            return (G3CODE_EOL);
        sp->b.bit = (Byte)state;
        action = RUNLENGTH(action - ACT_BRUNT);
        runlen += action;
        if (action < 64)
            return (runlen);
        state += 8;
        }
    /*NOTREACHED*/
    }

/*
 * Return the next uncompressed mode code word.
 */
int HCDCodecHMRCCITT::decode_uncomp_code()
    {
    Fax3DecodeState* sp = (Fax3DecodeState*)m_pStateBlock;
    short code;

    do {
        if (sp->b.bit == 0 || sp->b.bit > 7) {
            if (m_rawcc <= 0)
                return (UNCOMP_EOF);
            sp->b.data = fetchByte(sp);
            }
        code = TIFFFaxUncompAction[sp->b.bit][sp->b.data];
        sp->b.bit = TIFFFaxUncompNextState[sp->b.bit][sp->b.data];
        }
    while (code == ACT_INCOMP);
    return (code);
    }

int HCDCodecHMRCCITT::Fax3PostEncode()
    {
    Fax3BaseState* sp = (Fax3BaseState*)m_pStateBlock;

    if (sp->bit != 8)
        Fax3FlushBits(sp);
    return (1);
    }

/*
 * 1d-encode a row of pixels.  The encoding is
 * a sequence of all-white or all-black spans
 * of pixels encoded with Huffman codes.
 */
int HCDCodecHMRCCITT::Fax3Encode1DRow(Byte* bp, int bits)
    {
    Fax3EncodeState* sp = (Fax3EncodeState*)m_pStateBlock;
    int bs = 0, span;

    for (;;) {
        span = findspan(&bp, bs, bits, sp->wruns);    /* white span */
        putspan(span, TIFFFaxWhiteCodes);
        bs += span;
        if (bs >= bits)
            break;
        span = findspan(&bp, bs, bits, sp->bruns);    /* black span */
        putspan(span, TIFFFaxBlackCodes);
        bs += span;
        if (bs >= bits)
            break;
        }
    return (1);
    }

/*
 * Process one row of 1d Huffman-encoded data.
 */
int HCDCodecHMRCCITT::Fax3Decode1DRow(Byte* buf, int npels)
    {
    Fax3DecodeState* sp = (Fax3DecodeState*)m_pStateBlock;
    int x = 0;
    int runlen;
    //short action;
    short color = sp->b.white;
    static char module[] = "Fax3Decode1D";

    for (;;) {
        if (color == sp->b.white)
            runlen = decode_white_run();
        else
            runlen = decode_black_run();
        switch (runlen) {
            case G3CODE_EOF:
                //TIFFError(module,
                //  "%s: Premature EOF at scanline %d (x %d)",
                //tif->tif_name, tif->tif_row, x);
                return (0);
            case G3CODE_INVALID:    /* invalid code */
                /*
                 * An invalid code was encountered.
                 * Flush the remainder of the line
                 * and allow the caller to decide whether
                 * or not to continue.  Note that this
                 * only works if we have a G3 image
                 * with EOL markers.
                 */
                //TIFFError(module,
                // "%s: Bad code word at scanline %d (x %d)",
                //tif->tif_name, tif->tif_row, x);
                goto done;
            case G3CODE_EOL:    /* premature end-of-line code */
                //TIFFWarning(module,
                //  "%s: Premature EOL at scanline %d (x %d)",
//                tif->tif_name, tif->tif_row, x);
                return (1);    /* try to resynchronize... */
            }
        if (x+runlen > npels)
            runlen = npels-x;
        if (runlen > 0) {
            if (color)
                fillspan((Byte*)buf, x, runlen);
            x += runlen;
            if (x >= npels)
                break;
            }
        color = !color;
        }
done:
    /*
     * Cleanup at the end of the row.  This convoluted
     * logic is merely so that we can reuse the code with
     * two other related compression algorithms (2 & 32771).
     *
     * Note also that our handling of word alignment assumes
     * that the buffer is at least word aligned.  This is
     * the case for most all versions of malloc (typically
     * the buffer is returned longword aligned).
     */
    if ((m_options & CCITT_FAX3_NOEOL) == 0)
        skiptoeol(0);
    if (m_options & CCITT_FAX3_BYTEALIGN)
        sp->b.bit = 0;
    if ((m_options & CCITT_FAX3_WORDALIGN) && ((size_t)m_prawcp & 1))   
        (void) fetchByte(sp);
    return (x == npels);
    }

long    null_mode_prefix[MAX_NULLPREFIX];

/* number of prefixes or rows in the G4 decoding tables */
short    null_mode_prefix_count = 0;

static    unsigned char bit_mask[8] =
    { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

#define    DECLARE1(f,t1,a1)        f(t1 a1)

unsigned long
DECLARE1(append_0, unsigned long, prefix)
    {
    return (prefix + (1L<<16));
    }


unsigned long
DECLARE1(append_1, unsigned long, prefix)
    {
    static unsigned short prefix_bit[16] = {
        0x8000, 0x4000, 0x2000, 0x1000,
        0x0800, 0x0400, 0x0200, 0x0100,
        0x0080, 0x0040, 0x0020, 0x0010,
        0x0008, 0x0004, 0x0002, 0x0001
        };
    unsigned char len = (unsigned char)((prefix >> 16) & 0xf);
    return (append_0(prefix) + prefix_bit[len]);
    }

short
DECLARE1(null_mode_type, long, prefix)
    {
    switch (prefix) {
        case 0x18000L:
            return (MODE_VERT_V0);    /* 1 */
        case 0x36000L:
            return (MODE_VERT_VR1);    /* 011 */
        case 0x34000L:
            return (MODE_VERT_VL1);    /* 010 */
        case 0x32000L:
            return (MODE_HORIZ);        /* 001 */
        case 0x41000L:
            return (MODE_PASS);        /* 0001 */
        case 0x60C00L:
            return (MODE_VERT_VR2);    /* 0000 11 */
        case 0x60800L:
            return (MODE_VERT_VL2);    /* 0000 10 */
        case 0x70600L:
            return (MODE_VERT_VR3);    /* 0000 011 */
        case 0x70400L:
            return (MODE_VERT_VL3);    /* 0000 010 */
        case 0x80200L:
            return (MODE_ERROR);        /* 0000 0010 */
        case 0x90300L:
            return (MODE_ERROR);        /* 0000 0011 0 */
        case 0xA0380L:
            return (MODE_ERROR);        /* 0000 0011 10 */
        case 0xA03C0L:
            return (MODE_UNCOMP);    /* 0000 0011 11 */
            /*
             * Under the assumption that there are no
             * errors in the file, then this bit string
             * can only be the beginning of an EOL code.
             */
        case 0x70000L:
            return (MODE_ERROR_1);    /* 0000 000 */
        }
    return (-1);
    }

#define    BASESTATE(b)    ((unsigned char) ((b) & 0x7))

int verbose = false;


short
DECLARE1(find_null_mode_prefix, long, prefix)
    {
    short j1;

    if (prefix == 0L)
        return (0);
    for (j1 = 8; j1 < null_mode_prefix_count; j1++)
        if (prefix == null_mode_prefix[j1])
            return (j1);
    if (null_mode_prefix_count == MAX_NULLPREFIX) {
        fprintf(stderr, "ERROR: null mode prefix table overflow\n");
        exit(1);
        }
    if (verbose)
        fprintf(stderr, "adding null mode prefix[%d] 0x%lx\n",
                (int) null_mode_prefix_count, prefix);
    null_mode_prefix[null_mode_prefix_count++] = prefix;
    return (null_mode_prefix_count-1);
    }


void
build_null_mode_tables()
    {
    short prefix;

    /*
     * Note: the first eight entries correspond to
     * a null prefix and starting bit numbers 0-7.
     */
    null_mode_prefix_count = 8;
    for (prefix = 0; prefix < null_mode_prefix_count; prefix++) {
        short Byte;
        for (Byte = 0; Byte < 256; Byte++) {
            short firstbit;
            short bit;
            long curprefix;
            char found_code = false;

            if (prefix < 8) {
                curprefix = 0L;
                firstbit = prefix;
                }
            else {
                curprefix = null_mode_prefix[prefix];
                firstbit = 0;
                }
            for (bit = firstbit; bit < 8 && !found_code; bit++) {
                short mode;

                if (bit_mask[bit] & Byte)
                    curprefix = append_1(curprefix);
                else
                    curprefix = append_0(curprefix);
                switch (mode = null_mode_type(curprefix)) {
                    case MODE_PASS:
                    case MODE_HORIZ:
                    case MODE_VERT_V0:
                    case MODE_VERT_VR1:
                    case MODE_VERT_VR2:
                    case MODE_VERT_VR3:
                    case MODE_VERT_VL1:
                    case MODE_VERT_VL2:
                    case MODE_VERT_VL3:
                    case MODE_UNCOMP:
                    case MODE_ERROR:
                    case MODE_ERROR_1:
                        /*
                         * NOTE: if the bit number is 8, then the table
                         * entry will be zero, which indicates a new byte
                         * is to be fetched during the decoding process
                         */
                        found_code = true;
                        null_mode[prefix][Byte] = (unsigned char) mode;
                        null_mode_next_state[prefix][Byte] = BASESTATE(bit+1);
                        break;
                    }
                }
            if (!found_code) {
                null_mode_next_state[prefix][Byte] = (unsigned char)
                                                     find_null_mode_prefix(curprefix);
                /*
                 * This indicates to the decoder that
                 * no valid code has yet been identified.
                 */
                null_mode[prefix][Byte] = MODE_NULL;
                }
            }
        }
    }

long    horiz_mode_prefix[MAX_HORIZPREFIX];
short    horiz_mode_prefix_count = 0;
char    horiz_mode_color[MAX_HORIZPREFIX];

#define WHITE   0
#define BLACK    1

#define    DECLARE3(f,t1,a1,t2,a2,t3,a3)    f(t1 a1, t2 a2, t3 a3)

short
DECLARE3(search_table, unsigned long, prefix, CCITTtableentry const*, tab, int, n)
    {
    unsigned short len = (unsigned short)((prefix >> 16) & 0xf);
    unsigned short code = (unsigned short)((prefix & 0xffff) >> (16 - len));

    while (n-- > 0) {
        if (tab->length == len && tab->code == code)
            return ((short) tab->runlen);
        tab++;
        }
    return (G3CODE_INCOMP);
    }

#define    NCODES(a)    (sizeof (a) / sizeof (a[0]))

short
DECLARE1(white_run_length, unsigned long, prefix)
    {
    return (search_table(prefix, TIFFFaxWhiteCodes, NCODES(TIFFFaxWhiteCodes)));
    }

short
DECLARE1(horiz_mode_code_white, short, runlen)
    {
    return (runlen < 64 ? runlen + ACT_WRUNT : (runlen / 64) + ACT_WRUN);
    }

short
DECLARE1(black_run_length, unsigned long, prefix)
    {
    return (search_table(prefix, TIFFFaxBlackCodes, NCODES(TIFFFaxBlackCodes)));
    }

short
DECLARE1(horiz_mode_code_black, short, runlen)
    {
    return (runlen < 64 ? runlen + ACT_BRUNT : (runlen / 64) + ACT_BRUN);
    }

#define    DECLARE2(f,t1,a1,t2,a2)        f(t1 a1, t2 a2)

short
DECLARE2(find_horiz_mode_prefix, long, prefix, char, color)
    {
    short j1;

    for (j1 = 0; j1 < horiz_mode_prefix_count; j1++)
        if (prefix == horiz_mode_prefix[j1] && horiz_mode_color[j1] == color)
            return (j1);
    /*
     * It wasn't found, so add it to the tables, but first, is there room?
     */
    if (horiz_mode_prefix_count == MAX_HORIZPREFIX) {
        fprintf(stderr, "ERROR: 1D prefix table overflow\n");
        exit(1);
        }
    /* OK, there's room... */
    if (verbose)
        fprintf(stderr, "\nhoriz mode prefix %d, color %c = 0x%lx ",
                (int) horiz_mode_prefix_count, "WB"[color], prefix);
    horiz_mode_prefix[horiz_mode_prefix_count] = prefix;
    horiz_mode_color[horiz_mode_prefix_count] = color;
    horiz_mode_prefix_count++;
    return (horiz_mode_prefix_count - 1);
    }


void
build_horiz_mode_tables()
    {
    unsigned short Byte;
    short prefix;

    /*
     * The first 8 are for white,
     * the second 8 are for black,
     * beginning with bits 0-7.
     */
    horiz_mode_prefix_count = 16;
    for (prefix = 0; prefix < horiz_mode_prefix_count; prefix++)
        for (Byte = 0; Byte < 256; Byte++) {
            short bits_digested = 0;
            short bit;
            short firstbit;
            char color;
            unsigned long curprefix;

            if (prefix < 8) {
                color = WHITE;
                curprefix = 0L;
                firstbit = prefix;
                }
            else if (prefix < 16) {
                color = BLACK;
                curprefix = 0L;
                firstbit = prefix - 8;
                }
            else {
                color = horiz_mode_color[prefix];
                curprefix = horiz_mode_prefix[prefix];
                firstbit = 0;
                }
            for (bit = firstbit; bit < 8 && !bits_digested; bit++) {
                if (bit_mask[bit] & Byte)
                    curprefix = append_1(curprefix);
                else
                    curprefix = append_0(curprefix);
                /*
                 * The following conversion allows for arbitrary strings of
                 * zeroes to precede the end-of-line code 0000 0000 0001.
                 * It assumes no errors in the data, and is based on
                 * the assumption that the code replaced (12 consecutive
                 * zeroes) can only be "legally" encountered before the
                 * end-of-line code.  This assumption is valid only for
                 * a Group 3 image; the combination will never occur
                 * in horizontal mode in a proper Group 4 image.
                 */
                if (curprefix == 0xC0000L)
                    curprefix = 0xB0000L;
                if (color == WHITE) {
                    short runlength = white_run_length(curprefix);

                    if (runlength == G3CODE_INVALID) {
                        horiz_mode[prefix][Byte] = (unsigned char) ACT_INVALID;
                        horiz_mode_next_state[prefix][Byte] = (unsigned char) bit;
                        bits_digested = bit + 1;
                        }
                    else if (runlength == G3CODE_EOL) {   /* Group 3 only */
                        horiz_mode[prefix][Byte] = (unsigned char) ACT_EOL;
                        horiz_mode_next_state[prefix][Byte] = BASESTATE(bit+1);
                        bits_digested = bit + 1;
                        }
                    else if (runlength != G3CODE_INCOMP) {
                        horiz_mode[prefix][Byte] = (unsigned char)
                                                   horiz_mode_code_white(runlength);
                        horiz_mode_next_state[prefix][Byte] = BASESTATE(bit+1);
                        bits_digested = bit + 1;
                        }
                    }
                else {          /* color == BLACK */
                    short runlength = black_run_length(curprefix);

                    if (runlength == G3CODE_INVALID) {
                        horiz_mode[prefix][Byte] = (unsigned char) ACT_INVALID;
                        horiz_mode_next_state[prefix][Byte] = (unsigned char) (bit+8);
                        bits_digested = bit + 1;
                        }
                    else if (runlength == G3CODE_EOL) {   /* Group 3 only */
                        horiz_mode[prefix][Byte] = (unsigned char) ACT_EOL;
                        horiz_mode_next_state[prefix][Byte] = BASESTATE(bit+1);
                        bits_digested = bit + 1;
                        }
                    else if (runlength != G3CODE_INCOMP) {
                        horiz_mode[prefix][Byte] = (unsigned char)
                                                   horiz_mode_code_black(runlength);
                        horiz_mode_next_state[prefix][Byte] = BASESTATE(bit+1);
                        bits_digested = bit + 1;
                        }
                    }
                }
            if (!bits_digested) {    /* no codewords after examining Byte */
                horiz_mode[prefix][Byte] = (unsigned char) ACT_INCOMP;
                horiz_mode_next_state[prefix][Byte] = (unsigned char)
                                                      find_horiz_mode_prefix(curprefix, color);
                }
            }
    }

short    uncomp_mode_prefix_count = 0;
long    uncomp_mode_prefix[MAX_NULLPREFIX];

short
DECLARE1(uncomp_mode_type, long, prefix)
    {
    short code;
    short len;
    switch (prefix) {
        case 0x18000L:
            return (UNCOMP_RUN1);    /* 1 */
        case 0x24000L:
            return (UNCOMP_RUN2);    /* 01 */
        case 0x32000L:
            return (UNCOMP_RUN3);    /* 001 */
        case 0x41000L:
            return (UNCOMP_RUN4);    /* 0001 */
        case 0x50800L:
            return (UNCOMP_RUN5);    /* 0000 1 */
        case 0x60400L:
            return (UNCOMP_RUN6);    /* 0000 01 */
        case 0x70200L:
            return (UNCOMP_TRUN0);    /* 0000 001 */
        case 0x80100L:
            return (UNCOMP_TRUN1);    /* 0000 0001 */
        case 0x90080L:
            return (UNCOMP_TRUN2);    /* 0000 0000 1 */
        case 0xA0040L:
            return (UNCOMP_TRUN3);    /* 0000 0000 01 */
        case 0xB0020L:
            return (UNCOMP_TRUN4);    /* 0000 0000 001 */
        }
    code = (short)(prefix & 0xffffL);
    len = (short)((prefix >> 16) & 0xf);
    return ((code || len > 10) ? UNCOMP_INVALID : -1);
    }

short
DECLARE1(find_uncomp_mode_prefix, long, prefix)
    {
    short j1;

    if (prefix == 0L)
        return (0);
    for (j1 = 8; j1 < uncomp_mode_prefix_count; j1++)
        if (prefix == uncomp_mode_prefix[j1])
            return (j1);
    if (uncomp_mode_prefix_count == MAX_NULLPREFIX) {
        fprintf(stderr, "ERROR: uncomp mode prefix table overflow\n");
        exit(1);
        }
    if (verbose)
        fprintf(stderr, "adding uncomp mode prefix[%d] 0x%lx\n",
                (int) uncomp_mode_prefix_count, prefix);
    uncomp_mode_prefix[uncomp_mode_prefix_count++] = prefix;
    return (uncomp_mode_prefix_count-1);
    }


void
build_uncomp_mode_tables()
    {
    short prefix;

    /*
     * Note: the first eight entries correspond to
     * a null prefix and starting bit numbers 0-7.
     */
    uncomp_mode_prefix_count = 8;
    for (prefix = 0; prefix < uncomp_mode_prefix_count; prefix++) {
        short Byte;
        for (Byte = 0; Byte < 256; Byte++) {
            short firstbit;
            short bit;
            long curprefix;
            char found_code = false;

            if (prefix < 8) {
                curprefix = 0L;
                firstbit = prefix;
                }
            else {
                curprefix = uncomp_mode_prefix[prefix];
                firstbit = 0;
                }
            for (bit = firstbit; bit < 8 && !found_code; bit++) {
                short mode;

                if (bit_mask[bit] & Byte)
                    curprefix = append_1(curprefix);
                else
                    curprefix = append_0(curprefix);
                mode = uncomp_mode_type(curprefix);
                if (mode != -1) {
                    /*
                     * NOTE: if the bit number is 8, then the table
                     * entry will be zero, which indicates a new byte
                     * is to be fetched during the decoding process
                     */
                    found_code = true;
                    uncomp_mode[prefix][Byte] = (unsigned char) mode;
                    uncomp_mode_next_state[prefix][Byte] = BASESTATE(bit+1);
                    break;
                    }
                }
            if (!found_code) {
                uncomp_mode_next_state[prefix][Byte] = (unsigned char)
                                                       find_uncomp_mode_prefix(curprefix);
                /*
                 * This indicates to the decoder that
                 * no valid code has yet been identified.
                 */
                uncomp_mode[prefix][Byte] = UNCOMP_INCOMP;
                }
            }
        }
    }
