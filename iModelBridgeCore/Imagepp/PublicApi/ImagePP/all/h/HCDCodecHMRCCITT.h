//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecHMRCCITT.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecHMRCCITT
//-----------------------------------------------------------------------------
// HMRCCITT codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecCCITT.h"

BEGIN_IMAGEPP_NAMESPACE
/*
 * CCITT T.4 1D Huffman runlength codes and
 * related definitions.  Given the small sizes
 * of these tables it might does not seem
 * worthwhile to make code & length 8 bits.
 */
typedef struct cciitt3tableentry {
    unsigned short length;    /* bit length of g3 code */
    unsigned short code;    /* g3 code */
    short    runlen;        /* run length in bits */
    } CCITTtableentry;

#define        CCITT_RESUNIT_NONE        1    /* no meaningful units */
#define        CCITT_RESUNIT_INCH        2    /* english */
#define        CCITT_RESUNIT_CENTIMETER        3    /* metric */

#define        CCITT_GROUP3OPT_2DENCODING        0x1    /* 2-dimensional coding */
#define        CCITT_GROUP3OPT_UNCOMPRESSED    0x2    /* data not compressed */
#define        CCITT_GROUP3OPT_FILLBITS        0x4    /* fill to Byte boundary */

/* the following are for use by Compression=2, 32771, and 4 (T.6) algorithms */
#define     CCITT_FAX3_CLASSF       0x1
#define     CCITT_FAX3_NOEOL        0x2    /* no EOL code at end of row */
#define     CCITT_FAX3_BYTEALIGN    0x4    /* force Byte alignment at end of row */
#define     CCITT_FAX3_WORDALIGN    0x8    /* force word alignment at end of row */

class HCDCodecHMRCCITT : public HCDCodecCCITT
    {
    HDECLARE_CLASS_ID(HCDCodecId_CCITTHMR, HCDCodecCCITT)

public:

    IMAGEPP_EXPORT                 HCDCodecHMRCCITT();

    IMAGEPP_EXPORT                 HCDCodecHMRCCITT(uint32_t pi_Width,
                                            uint32_t pi_Height);


    HCDCodecHMRCCITT(const HCDCodecHMRCCITT& pi_rObj);

    ~HCDCodecHMRCCITT();

    size_t          CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t pi_OutBufferSize);

    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void* po_pOutBuffer,
                                     size_t pi_OutBufferSize);

    bool           HasLineAccess() const;

    void            Reset();

    virtual HCDCodec* Clone() const override;

    IMAGEPP_EXPORT void            SetYResolution(float pi_Res);

    IMAGEPP_EXPORT void            SetResolutionUnit(unsigned short pi_Unit);

    IMAGEPP_EXPORT void            SetGroup3Options(int32_t pi_Options);

    IMAGEPP_EXPORT void            SetCCITT3(bool pi_Enabled);
    IMAGEPP_EXPORT bool           GetCCITT3() const;

    IMAGEPP_EXPORT void            SetOptions(int8_t pi_Options);

protected:

private:

    Byte*         m_pStateBlock;

    void            Fax3SetupState(size_t space);

    void            Fax3PreEncode();

    void            Fax3Encode2DRow(Byte* bp, Byte* rp, int32_t bits);

    int             finddiff(Byte* cp, int bs, int be, int color);

    void            putcode(CCITTtableentry const* te);

    void            putspan(int span, CCITTtableentry const* tab);

    int             findspan(Byte** bpp, int bs, int be, register Byte const* tab);

    void            putbits(unsigned short bits, unsigned short length);

    void            Fax4PostEncode();

    void            Fax3PutEOL();

    void            Fax3PreDecode();

    void            skiptoeol(int len);

    int             nextbit();

    int             Fax3Decode2DRow(Byte* buf, int npels);

    void            fillspan(Byte* cp, int x, int count);

    int             decode_white_run();

    int             decode_black_run();

    int             decode_uncomp_code();

    int             Fax3PostEncode();

    int             Fax3Encode1DRow(Byte* bp, int bits);

    int             Fax3Decode1DRow(Byte* buf, int npels);

    float          m_yresolution;
    unsigned short m_resolutionunit;
    int32_t        m_group3options;
    int8_t         m_options;

    bool           m_CCITT3;

    Byte*         m_prawcp;
    size_t          m_rawdatasize;
    size_t          m_rawcc;


    static bool    m_TablesBuilded;
    };

END_IMAGEPP_NAMESPACE