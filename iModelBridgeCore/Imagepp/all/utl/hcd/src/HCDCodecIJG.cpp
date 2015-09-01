//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecIJG.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecIJG
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecIJG.h>

#include "HCDCodecIJG8Bits.h"       // The order is important
#include "HCDCodecIJG12Bits.h"      // here

//#define VALIDATE_ABBREVIATION_MODE 1    // Turn ON to validate that abbreviation mode doesn't add DHT and DQT.

/*
 * JPEG markers consist of one or more 0xFF bytes, followed by a marker
 * code byte (which is not an FF).  Here are the marker codes of interest
 * in this program.  (See jdmarker.c for a more complete list.)
 */

#define M_SOF0  0xC0		/* Start Of Frame N */
#define M_SOF1  0xC1		/* N indicates which compression process */
#define M_SOF2  0xC2		/* Only SOF0-SOF2 are now in common use */
#define M_SOF3  0xC3
#define M_SOF5  0xC5		/* NB: codes C4 and CC are NOT SOF markers */
#define M_SOF6  0xC6
#define M_SOF7  0xC7
#define M_SOF9  0xC9
#define M_SOF10 0xCA
#define M_SOF11 0xCB
#define M_SOF13 0xCD
#define M_SOF14 0xCE
#define M_SOF15 0xCF
#define M_SOI   0xD8		/* Start Of Image (beginning of datastream) */
#define M_EOI   0xD9		/* End Of Image (end of datastream) */
#define M_SOS   0xDA		/* Start Of Scan (begins compressed data) */
#define M_COM   0xFE		/* COMment */
#define M_DHT   0xC4        // HUffman table
#define M_DQT   0xDB        // Quantization table


/*---------------------------------------------------------------------------------**//**
* JpegMarkerReader
* Read Jpeg header until SOS or EOI is reached. SOF, DQT, DHT and SOS position and lenght 
* are collected.
*
* Implementation is based on rdjpgcom.c and wrjpgcom.c from libjpeg.
* @bsiclass                                                    Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct JpegMarkerReader
    {
    struct Marker
        {
        Byte const* pData;          // A pointer to the marker, including 0xFFnn Marker.
        int         length;         // The total length of the marker. Including jpegMarker(0xFFnn) and length header.
        };

    struct JpegException
        {
        JpegException(WCharCP message)
            {
            m_message = message;
            }
        WString m_message;
        };

    JpegMarkerReader(Byte const* jpegData, size_t jpegDataSize)
        {
        m_pData = jpegData;
        m_Offset = 0;
        m_DataSize = jpegDataSize;
        }    

    int NEXTBYTE() 
        {
        if(m_Offset >= m_DataSize)
            return EOF; 

        return m_pData[m_Offset++];
        }

    /* Read one byte, testing for EOF */
    int read_1_byte (void)
        {
        int c;
        c = NEXTBYTE();
        if (c == EOF)
            throw JpegException(L"Premature EOF in JPEG file"); 
        
        return c;
        }

    /* Read 2 bytes, convert to unsigned int */
    /* All 2-byte quantities in JPEG markers are MSB first */
    unsigned int read_2_bytes (void)
        {
        int c1, c2;

        c1 = NEXTBYTE();
        if (c1 == EOF)
            throw JpegException(L"Premature EOF in JPEG file"); 
        c2 = NEXTBYTE();
        if (c2 == EOF)
            throw JpegException(L"Premature EOF in JPEG file"); 
        return (((unsigned int) c1) << 8) + ((unsigned int) c2);
        }

    /*
    * Find the next JPEG marker and return its marker code.
    * We expect at least one FF byte, possibly more if the compressor used FFs
    * to pad the file.  (Padding FFs will NOT be replicated in the output file.)
    * There could also be non-FF garbage between markers.  The treatment of such
    * garbage is unspecified; we choose to skip over it but emit a warning msg.
    * NB: this routine must not be used after seeing SOS marker, since it will
    * not deal correctly with FF/00 sequences in the compressed image data...
    */
    int next_marker (void)
        {
        int c;
        int discarded_bytes = 0;

        /* Find 0xFF byte; count and skip any non-FFs. */
        c = read_1_byte();
        while (c != 0xFF) {
            discarded_bytes++;
            c = read_1_byte();
            }
        /* Get marker code byte, swallowing any duplicate FF bytes.  Extra FFs
        * are legal as pad bytes, so don't count them in discarded_bytes.
        */
        do {
            c = read_1_byte();
            } while (c == 0xFF);

//         if (discarded_bytes != 0) {
//             fprintf(stderr, "Warning: garbage data found in JPEG file\n");
//             }

        return c;
        }

    /*
    * Read the initial marker, which should be SOI.
    * For a JFIF file, the first two bytes of the file should be literally
    * 0xFF M_SOI.  To be more general, we could use next_marker, but if the
    * input file weren't actually JPEG at all, next_marker might read the whole
    * file and then return a misleading error message...
    */
    int first_marker (void)
        {
        int c1, c2;

        c1 = NEXTBYTE();
        c2 = NEXTBYTE();
        if (c1 != 0xFF || c2 != M_SOI)
            throw JpegException(L"Not a JPEG file"); 
        return c2;
        }

    /*
     * Most types of marker are followed by a variable-length parameter segment.
     * This routine skips over the parameters for any marker we don't otherwise
     * want to process.
     * Note that we MUST skip the parameter segment explicitly in order not to
     * be fooled by 0xFF bytes that might appear within the parameter segment;
     * such bytes do NOT introduce new markers.
     */
    unsigned int skip_variable (void)
        /* Skip over an unknown or uninteresting variable-length marker */
        {
        unsigned int length, lengthRet;

        /* Get the marker parameter length count */
        length = lengthRet = read_2_bytes();
        /* Length includes itself, so must be at least 2 */
        if (length < 2)
            throw JpegException(L"Erroneous JPEG marker length");
        length -= 2;
        /* Skip over the remaining bytes */
        while (length > 0) {
            (void) read_1_byte();
            length--;
            }

        return lengthRet;
        }

    /*
     * Parse the marker stream until SOS or EOI is seen.
     */
    int scan_JPEG_header(vector<Marker>& DQT, Marker& SOF, vector<Marker>& DHT, Marker& SOS)
        {
        DQT.clear();
        DHT.clear();
        SOF.length = SOS.length = 0;
        SOF.pData = SOS.pData = NULL;

        int marker;

        /* Expect SOI at start of file */
        if (first_marker() != M_SOI)
             throw JpegException(L"Expected SOI marker first");

        /* Scan miscellaneous markers until we reach SOFn. */
        for (;;)
            {
            marker = next_marker();
            switch (marker) 
                {
                // Note that marker codes 0xC4, 0xC8, 0xCC are not, and must not be,  treated as SOFn. C4 in particular is actually DHT.
                case M_SOF0:        /* Baseline */
                case M_SOF1:        //* Extended sequential, Huffman */
                case M_SOF2:        //* Progressive, Huffman */
                case M_SOF3:        //* Lossless, Huffman */
                case M_SOF5:        //* Differential sequential, Huffman */
                case M_SOF6:        //* Differential progressive, Huffman */
                case M_SOF7:        //* Differential lossless, Huffman */
                case M_SOF9:        //* Extended sequential, arithmetic */
                case M_SOF10:       /* Progressive, arithmetic */
                case M_SOF11:       /* Lossless, arithmetic */
                case M_SOF13:       /* Differential sequential, arithmetic */
                case M_SOF14:       /* Differential progressive, arithmetic */
                case M_SOF15:       /* Differential lossless, arithmetic */
                    {
                    if(0 != SOF.pData)
                        throw JpegException(L"More than one SOFn");  

                    SOF.pData = m_pData + (m_Offset - 2);
                    SOF.length = skip_variable() + 2; // include marker
                    break;
                    }

                case M_SOS:
                    {
                    if(0 == SOF.pData)
                        throw JpegException(L"SOS without prior SOFn");      

                    SOS.pData = m_pData + (m_Offset - 2);
                    SOS.length = skip_variable() + 2; // include marker

  /*EXIT*/          return marker;
                    }

                case M_EOI:     /* in case it's a tables-only JPEG stream */
  /*EXIT*/          return marker;
            
                case M_DHT:
                    {
                    Marker dhtMarker;
                    dhtMarker.pData = m_pData + (m_Offset - 2);
                    dhtMarker.length = skip_variable() + 2; // include marker 
                    DHT.push_back(dhtMarker);
                    break;
                    }

                case M_DQT:
                    {
                    Marker dqtMarker;
                    dqtMarker.pData = m_pData + (m_Offset - 2);
                    dqtMarker.length = skip_variable() + 2; // include marker 
                    DQT.push_back(dqtMarker);
                    break;
                    }

                default:        /* Anything else just gets skipped */
                    skip_variable();        /* we assume it has a parameter count... */
                    break;
                }
            }
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                   Mathieu.Marchand  05/2013
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus ReadMarkers(vector<Marker>& DQT, Marker& SOF, vector<Marker>& DHT, Marker& SOS)
        {
        try
            {
            int marker = scan_JPEG_header(DQT, SOF, DHT, SOS);

            if(M_SOS != marker && M_EOI != marker)
                return BSIERROR;
            }
        catch(...)
            {
            return BSIERROR;
            }

        return BSISUCCESS;
        }

private:
    Byte const* m_pData;
    size_t      m_Offset;
    size_t      m_DataSize;
};

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecIJG::HCDCodecIJG()
    : HCDCodecJPEG()
    {
    m_pJpegCodec8Bits = new HCDCodecIJG_8bits();

    HPOSTCONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecIJG::HCDCodecIJG(size_t pi_Width,
                         size_t pi_Height,
                         size_t pi_BitsPerPixel)
    : HCDCodecJPEG(pi_Width, pi_Height, pi_BitsPerPixel)
    {
    // if the pixelType is 48 (RGB) or 64(RGBA), 16 bits by sample, we consider to use JPEG 12bits
    if (pi_BitsPerPixel == 48 || pi_BitsPerPixel == 64)
        m_pJpegCodec12Bits = new HCDCodecIJG_12bits(pi_Width, pi_Height, pi_BitsPerPixel);
    else
        m_pJpegCodec8Bits = new HCDCodecIJG_8bits(pi_Width, pi_Height, pi_BitsPerPixel);

    HPOSTCONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecIJG::HCDCodecIJG(const HCDCodecIJG& pi_rObj)
    : HCDCodecJPEG(pi_rObj)
    {
    if (pi_rObj.m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits = new HCDCodecIJG_12bits(*pi_rObj.m_pJpegCodec12Bits);
    else
        m_pJpegCodec8Bits = new HCDCodecIJG_8bits(*pi_rObj.m_pJpegCodec8Bits);

    HPOSTCONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecIJG::~HCDCodecIJG()
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecIJG::Clone() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    return new HCDCodecIJG(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecIJG::CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t po_OutBufferSize)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->CompressSubset(pi_pInData, pi_InDataSize, po_pOutBuffer, po_OutBufferSize);
    else
        return m_pJpegCodec8Bits->CompressSubset(pi_pInData, pi_InDataSize, po_pOutBuffer, po_OutBufferSize);
    }

//-----------------------------------------------------------------------------
// public
// CreateTables
//-----------------------------------------------------------------------------
uint32_t HCDCodecIJG::CreateTables(void* po_pOutBuffer, uint32_t pi_OutBufferSize)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->CreateTables(po_pOutBuffer, pi_OutBufferSize);
    else
        return m_pJpegCodec8Bits->CreateTables(po_pOutBuffer, pi_OutBufferSize);
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecIJG::DecompressSubset(const void*  pi_pInData,
                                     size_t pi_InDataSize,
                                     void*  po_pOutBuffer,
                                     size_t pi_OutBufferSize)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->DecompressSubset(pi_pInData, pi_InDataSize, po_pOutBuffer, pi_OutBufferSize);
    else
        return m_pJpegCodec8Bits->DecompressSubset(pi_pInData, pi_InDataSize, po_pOutBuffer, pi_OutBufferSize);
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecIJG::HasLineAccess() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->HasLineAccess();
    else
        return m_pJpegCodec8Bits->HasLineAccess();
    }



//-----------------------------------------------------------------------------
// public
// GetMinimumSubsetSize
//-----------------------------------------------------------------------------
size_t HCDCodecIJG::GetMinimumSubsetSize() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetMinimumSubsetSize();
    else
        return m_pJpegCodec8Bits->GetMinimumSubsetSize();
    }


//-----------------------------------------------------------------------------
// public
// Reset
//-----------------------------------------------------------------------------
void HCDCodecIJG::Reset()
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->Reset();
    else
        m_pJpegCodec8Bits->Reset();
    }

//-----------------------------------------------------------------------------
// public
// SetOptimizeCoding
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetOptimizeCoding(bool pi_Enable)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetOptimizeCoding(pi_Enable);
    else
        m_pJpegCodec8Bits->SetOptimizeCoding(pi_Enable);
    }

//-----------------------------------------------------------------------------
// public
// SetQuality
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetQuality(Byte pi_Percentage)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetQuality(pi_Percentage);
    else
        m_pJpegCodec8Bits->SetQuality(pi_Percentage);
    }

//-----------------------------------------------------------------------------
// public
// GetOptimizeCoding
//-----------------------------------------------------------------------------
bool HCDCodecIJG::GetOptimizeCoding() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetOptimizeCoding();
    else
        return m_pJpegCodec8Bits->GetOptimizeCoding();
    }

//-----------------------------------------------------------------------------
// public
// GetQuality
//-----------------------------------------------------------------------------
Byte HCDCodecIJG::GetQuality() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetQuality();
    else
        return m_pJpegCodec8Bits->GetQuality();
    }


//-----------------------------------------------------------------------------
// public
// SetProgressiveMode
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetProgressiveMode(bool pi_Enable)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetProgressiveMode(pi_Enable);
    else
        m_pJpegCodec8Bits->SetProgressiveMode(pi_Enable);
    }

//-----------------------------------------------------------------------------
// public
// IsProgressive
//-----------------------------------------------------------------------------
bool HCDCodecIJG::IsProgressive() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->IsProgressive();
    else
        return m_pJpegCodec8Bits->IsProgressive();
    }

//-----------------------------------------------------------------------------
// public
// SetBitsPerPixel
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetBitsPerPixel(size_t pi_BitsPerPixel)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetBitsPerPixel(pi_BitsPerPixel);
    else
        m_pJpegCodec8Bits->SetBitsPerPixel(pi_BitsPerPixel);
    }

//-----------------------------------------------------------------------------
// public
// SetColorMode
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetColorMode(ColorModes pi_Mode)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetColorMode(pi_Mode);
    else
        m_pJpegCodec8Bits->SetColorMode(pi_Mode);
    }

//-----------------------------------------------------------------------------
// public
// GetColorMode
//-----------------------------------------------------------------------------
HCDCodecIJG::ColorModes HCDCodecIJG::GetColorMode() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetColorMode();
    else
        return m_pJpegCodec8Bits->GetColorMode();
    }

//-----------------------------------------------------------------------------
// public
// ReadHeader
//-----------------------------------------------------------------------------
void HCDCodecIJG::ReadHeader(const void* pi_pInData, size_t pi_InDataSize)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->ReadHeader(pi_pInData, pi_InDataSize);
    else
        m_pJpegCodec8Bits->ReadHeader(pi_pInData, pi_InDataSize);
    }

//-----------------------------------------------------------------------------
// public
// SetDimensions
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetDimensions(size_t pi_Width, size_t pi_Height)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetDimensions(pi_Width, pi_Height);
    else
        m_pJpegCodec8Bits->SetDimensions(pi_Width, pi_Height);
    }

//-----------------------------------------------------------------------------
// public
// GetHeight
//-----------------------------------------------------------------------------
size_t HCDCodecIJG::GetHeight() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetHeight();
    else
        return m_pJpegCodec8Bits->GetHeight();
    }


//-----------------------------------------------------------------------------
// public
// GetWidth
//-----------------------------------------------------------------------------
size_t HCDCodecIJG::GetWidth() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetWidth();
    else
        return m_pJpegCodec8Bits->GetWidth();
    }

//-----------------------------------------------------------------------------
// public
// GetLinePaddingBits
//-----------------------------------------------------------------------------
size_t  HCDCodecIJG::GetLinePaddingBits() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetLinePaddingBits();
    else
        return m_pJpegCodec8Bits->GetLinePaddingBits();
    }

//-----------------------------------------------------------------------------
// public
// SetAbbreviateMode
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetLinePaddingBits(size_t pi_Bits)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetLinePaddingBits(pi_Bits);
    else
        m_pJpegCodec8Bits->SetLinePaddingBits(pi_Bits);
    }

//-----------------------------------------------------------------------------
// public
// GetDataSize
//-----------------------------------------------------------------------------
size_t HCDCodecIJG::GetDataSize() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetDataSize();
    else
        return m_pJpegCodec8Bits->GetDataSize();
    }

//-----------------------------------------------------------------------------
// public
// SetSubsetSize
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetSubsetSize(size_t pi_Size)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetSubsetSize(pi_Size);
    else
        m_pJpegCodec8Bits->SetSubsetSize(pi_Size);
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetWidth
//-----------------------------------------------------------------------------
size_t HCDCodecIJG::GetSubsetWidth() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetSubsetWidth();
    else
        return m_pJpegCodec8Bits->GetSubsetWidth();
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetHeight
//-----------------------------------------------------------------------------
size_t HCDCodecIJG::GetSubsetHeight() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetSubsetHeight();
    else
        return m_pJpegCodec8Bits->GetSubsetHeight();
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetPosY
//-----------------------------------------------------------------------------
size_t HCDCodecIJG::GetSubsetPosY() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetSubsetPosY();
    else
        return m_pJpegCodec8Bits->GetSubsetPosY();
    }

//-----------------------------------------------------------------------------
// public
// SetSubset
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetSubset(size_t pi_Width,
                            size_t pi_Height,
                            size_t pi_PosX,
                            size_t pi_PosY)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetSubset(pi_Width, pi_Height, pi_PosX, pi_PosY);
    else
        m_pJpegCodec8Bits->SetSubset(pi_Width, pi_Height, pi_PosX, pi_PosY);
    }

//-----------------------------------------------------------------------------
// public
// SetSubsetPosY
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetSubsetPosY(size_t pi_PosY)

    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetSubsetPosY(pi_PosY);
    else
        m_pJpegCodec8Bits->SetSubsetPosY(pi_PosY);
    }

//-----------------------------------------------------------------------------
// public
// GetCompressedImageIndex
//-----------------------------------------------------------------------------
size_t HCDCodecIJG::GetCompressedImageIndex() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetCompressedImageIndex();
    else
        return m_pJpegCodec8Bits->GetCompressedImageIndex();
    }

//-----------------------------------------------------------------------------
// public
// SetAbbreviateMode
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetAbbreviateMode(bool pi_Enable)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetAbbreviateMode(pi_Enable);
    else
        m_pJpegCodec8Bits->SetAbbreviateMode(pi_Enable);
    }

//-----------------------------------------------------------------------------
// public
// GetAbbreviateMode
//-----------------------------------------------------------------------------
bool HCDCodecIJG::GetAbbreviateMode() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetAbbreviateMode();
    else
        return m_pJpegCodec8Bits->GetAbbreviateMode();
    }

//-----------------------------------------------------------------------------
// public
// SetSubsamplingMode
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetSubsamplingMode(SubsamplingModes pi_Mode)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetSubsamplingMode(pi_Mode);
    else
        m_pJpegCodec8Bits->SetSubsamplingMode(pi_Mode);
    }

//-----------------------------------------------------------------------------
// public
// SetQuantizationTable
// PLEASE CALL FOR SLOT 0 BEFORE SLOT 1
//-----------------------------------------------------------------------------
void HCDCodecIJG::SetQuantizationTable(int pi_Slot, const unsigned int* pi_pTable, bool pi_UnZigZag)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetQuantizationTable(pi_Slot, pi_pTable, pi_UnZigZag);
    else
        m_pJpegCodec8Bits->SetQuantizationTable(pi_Slot, pi_pTable, pi_UnZigZag);
    }

//-----------------------------------------------------------------------------
// public
// GetSubsamplingMode
//-----------------------------------------------------------------------------

HCDCodecIJG::SubsamplingModes HCDCodecIJG::GetSubsamplingMode() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetSubsamplingMode();
    else
        return m_pJpegCodec8Bits->GetSubsamplingMode();
    }

//-----------------------------------------------------------------------------
// public
// SetSourceColorMode
//-----------------------------------------------------------------------------

void HCDCodecIJG::SetSourceColorMode(ColorModes pi_Mode)
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->SetSourceColorMode(pi_Mode);
    else
        m_pJpegCodec8Bits->SetSourceColorMode(pi_Mode);
    }

//-----------------------------------------------------------------------------
// public
// GetSourceColorMode
//-----------------------------------------------------------------------------

HCDCodecIJG::ColorModes HCDCodecIJG::GetSourceColorMode() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetSourceColorMode();
    else
        return m_pJpegCodec8Bits->GetSourceColorMode();
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecIJG::GetSubsetMaxCompressedSize() const
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        return m_pJpegCodec12Bits->GetSubsetMaxCompressedSize();
    else
        return m_pJpegCodec8Bits->GetSubsetMaxCompressedSize();
    }

//-----------------------------------------------------------------------------
// public
// CopyTablesFromDecoderToEncoder
//-----------------------------------------------------------------------------
void HCDCodecIJG::CopyTablesFromDecoderToEncoder()
    {
    HPRECONDITION (m_pJpegCodec8Bits == 0 || m_pJpegCodec12Bits == 0);

    if (m_pJpegCodec8Bits == 0)
        m_pJpegCodec12Bits->CopyTablesFromDecoderToEncoder();
    else
        m_pJpegCodec8Bits->CopyTablesFromDecoderToEncoder();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Byte const* HCDCodecIJG::GetAbbreviateTableHeader(size_t& headerSize) const
    {
    if (m_pJpegCodec12Bits != NULL)
        return m_pJpegCodec12Bits->GetAbbreviateTableHeader(headerSize);

    return m_pJpegCodec8Bits->GetAbbreviateTableHeader(headerSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t HCDCodecIJG::MakeInterchangeFormat(const Byte* pi_pInData, size_t pi_InDataSize,
                                          Byte* po_pOutBuffer, size_t pi_OutBufferSize) const
    {
    size_t tableHeaderSize = 0;
    Byte const* pTablesHeader = GetAbbreviateTableHeader(tableHeaderSize);
    if(pTablesHeader == NULL || tableHeaderSize == 0)
        return 0;
    
    JpegMarkerReader tableReader(pTablesHeader, tableHeaderSize);
    vector<JpegMarkerReader::Marker> header_DQT;
    JpegMarkerReader::Marker __SOF;
    vector<JpegMarkerReader::Marker> header_DHT;
    JpegMarkerReader::Marker __SOS;
    if(BSISUCCESS != tableReader.ReadMarkers(header_DQT, __SOF, header_DHT, __SOS))
        return 0; 
    
    // Not sure what could come from the header or not?
    if(header_DQT.empty() || header_DHT.empty())
        return 0;

    JpegMarkerReader reader(pi_pInData, pi_InDataSize);
    vector<JpegMarkerReader::Marker> DQT;
    JpegMarkerReader::Marker SOF;
    vector<JpegMarkerReader::Marker> DHT;
    JpegMarkerReader::Marker SOS;
    if(BSISUCCESS != reader.ReadMarkers(DQT, SOF, DHT, SOS))
        return 0; 

    // Validate SOF SOS maker.
    if(NULL == SOF.pData || NULL == SOS.pData || SOF.pData > SOS.pData)
        return 0;

    // Copy up to SOF
    size_t outBufferOffset = SOF.pData-pi_pInData;
    memcpy(po_pOutBuffer, pi_pInData, outBufferOffset);
            
    // Insert DQT tables if not present.
    if(DQT.empty() && !header_DQT.empty())
        {
        for(size_t i=0; i < header_DQT.size(); ++i)
            {
            memcpy(po_pOutBuffer + outBufferOffset, header_DQT[i].pData, header_DQT[i].length);
            outBufferOffset += header_DQT[i].length;
            }
        }

    // copy from SOF til SOS
    memcpy(po_pOutBuffer + outBufferOffset, SOF.pData, SOS.pData-SOF.pData);
    outBufferOffset += SOS.pData-SOF.pData;

    // Insert DHT tables if not present.
    if(DHT.empty() && !header_DHT.empty())
        {
        for(size_t i=0; i < header_DHT.size(); ++i)
            {
            memcpy(po_pOutBuffer + outBufferOffset, header_DHT[i].pData, header_DHT[i].length);
            outBufferOffset +=header_DHT[i].length;
            }
        }

    // Copy remainder of file
    memcpy(po_pOutBuffer + outBufferOffset, SOS.pData, (pi_pInData+pi_InDataSize)-SOS.pData);
    outBufferOffset += (pi_pInData+pi_InDataSize)-SOS.pData;

// DEBUG Code to compare our version with the one created with MergeDQT_DHT.
//     HArrayAutoPtr<byte> pTempBuf(new byte[pi_OutBufferSize]);
// 
//     size_t tempSize = 0;
//     if (m_pJpegCodec12Bits != 0)
//         tempSize = m_pJpegCodec12Bits->MergeDQT_DHT(pi_pInData, pi_InDataSize, pTempBuf, pi_OutBufferSize);
//     
//     tempSize = m_pJpegCodec8Bits->MergeDQT_DHT(pi_pInData, pi_InDataSize, pTempBuf, pi_OutBufferSize);
// 
//     HASSERT(tempSize == outBufferOffset);
//     if(tempSize == outBufferOffset)
//         {
//         HASSERT(0 == memcmp(pTempBuf, po_pOutBuffer, outBufferOffset));
//         }
    
    return outBufferOffset;
    }