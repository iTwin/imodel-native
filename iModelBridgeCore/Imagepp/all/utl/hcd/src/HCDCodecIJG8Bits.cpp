//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecIJG8Bits.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecIJG
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDException.h>

// Enable this to have MergeDQT_DHT support. Right now libjpeg-turbo is not exposing this service but it could.
// #define HAVE_TRANSUPP

#ifdef JPEGLIB_SUPPORT_12BITS
    // jpegturbo cannot handle 12 bits. Use the libjpeg from out external source.
    #include "../../../../ext/jpeg/jpeglib.h"
    #ifdef HAVE_TRANSUPP
        #include "../../../../ext/jpeg/transupp.h"
    #endif
    #include "HCDCodecIJG12Bits.h"
    #define JPEGLIB_SUPPORT_12BITS          // HCDCodecIJG12bits.h undefines the symbol
#else
    #include <libjpeg-turbo/jpeglib.h>
    #ifdef HAVE_TRANSUPP
        #include <libjpeg-turbo/transupp.h>
    #endif
    #include "HCDCodecIJG8Bits.h"
#endif

//When the size of the subset is very small it is possible that the safety factor of
//2 may be not enough. So add an offset that should be increased as required.
#define MAX_COMPRESSED_SIZE_SAFETY_OFFSET 768


// To support Jpeg compression 8 and 12 bits, we need to remove any references from the .h file.
#define cinfocomp ((struct jpeg_compress_struct*)m_cinfocomp)
#define cinfodec  ((struct jpeg_decompress_struct*)m_cinfodec)


static void
s_jpeg_mem_dest (j_compress_ptr cinfo, Byte* pi_pData, size_t pi_DataSize );
static void
s_jpeg_mem_src (j_decompress_ptr cinfo, Byte* pi_pData, size_t pi_DataSize);

typedef struct {
    struct jpeg_destination_mgr pub;  // public fields.

    Byte*   m_pData;                // target stream.
    size_t    m_DataSize;                // start of buffer
    } my_destination_mgr;
typedef my_destination_mgr* my_dest_ptr;


typedef struct {
    struct jpeg_source_mgr pub;        // public fields

    Byte* m_pData;                    // source stream
    size_t m_DataSize;                // start of buffer

    } my_source_mgr;

typedef my_source_mgr* my_src_ptr;

// NaturalOrderIndexes[i] is the natural-order position of the i'th element
// of a zigzag-ordered matrix.

static const uint16_t s_NaturalOrderIndexes[64] =
    {
    0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63,
    };

//-----------------------------------------------------------------------------
// Friend
// JPEGErrorExit
//-----------------------------------------------------------------------------
METHODDEF (void) HCDJpegErrorExit(void* cinfo)
    {
    IJG12BITS(HCDCodecIJG)::HCDJpegFileErrorManager* pErrorManager;

    // The IJG JPEG Library wants us to display the error message
    // and to exit.
    // Instead, we will longjump back to the execution error and
    // throw an exception

    // cinfo->err really points to a HRFJpegFileErrorManager, so coerce pointer
    pErrorManager = (IJG12BITS(HCDCodecIJG)::HCDJpegFileErrorManager*) ((j_common_ptr)cinfo)->err;

    // call a Jpeg exception
    throw HCDCorruptedPackbitsDataException();
    }


//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
IJG12BITS(HCDCodecIJG)::IJG12BITS(HCDCodecIJG)()
    : HCDCodecJPEG()
    {
    try
        {
        InitObject();
        }
    catch(...)
        {
        DeepDelete();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
IJG12BITS(HCDCodecIJG)::IJG12BITS(HCDCodecIJG)(size_t pi_Width,
                                               size_t pi_Height,
                                               size_t pi_BitsPerPixel)
    : HCDCodecJPEG(pi_Width, pi_Height, pi_BitsPerPixel)
    {
    try
        {
        InitObject();
        }
    catch(...)
        {
        DeepDelete();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
IJG12BITS(HCDCodecIJG)::IJG12BITS(HCDCodecIJG)(const IJG12BITS(HCDCodecIJG)& pi_rObj)
    : HCDCodecJPEG(pi_rObj)
    {
    try
        {
        DeepCopy(pi_rObj);
        }
    catch(...)
        {
        DeepDelete();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
IJG12BITS(HCDCodecIJG)::~IJG12BITS(HCDCodecIJG)()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* IJG12BITS(HCDCodecIJG)::Clone() const
    {
    return new IJG12BITS(HCDCodecIJG)(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t IJG12BITS(HCDCodecIJG)::CompressSubset(const void* pi_pInData,
                                              size_t pi_InDataSize,
                                              void* po_pOutBuffer,
                                              size_t po_OutBufferSize)
    {
    HPRECONDITION(pi_InDataSize < INT_MAX);
    HPRECONDITION(po_OutBufferSize < INT_MAX);

    ((my_destination_mgr*)(cinfocomp->dest))->pub.next_output_byte = (Byte*)po_pOutBuffer;
    ((my_destination_mgr*)(cinfocomp->dest))->pub.free_in_buffer = po_OutBufferSize;
    ((my_destination_mgr*)(cinfocomp->dest))->m_pData = (Byte*)po_pOutBuffer;
    ((my_destination_mgr*)(cinfocomp->dest))->m_DataSize = po_OutBufferSize;

    // is it the first subset?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_COMPRESS);

        if(!m_AbbreviateMode && !m_ExternalQuantizationTablesUse)
            SetQuality(m_Quality);

        jpeg_start_compress(cinfocomp, !m_AbbreviateMode);
        }

    JSAMPROW row_pointer[1];    /* pointer to a single row */
    size_t row_stride;            /* physical row width in buffer */

    row_stride = (GetSubsetWidth() * GetBitsPerPixel() + GetLinePaddingBits()) / 8;

    Byte* image_buffer = (Byte*)pi_pInData;

    // Calculate the ending position for the current subset. Normally,
    // this is the current subset position + the subset height. For the last
    // subset, we must stop at the end of the block...
    size_t EndPosY = MIN(GetSubsetPosY() + GetSubsetHeight(), GetHeight());

    while (cinfocomp->next_scanline < EndPosY) {
        row_pointer[0] = (JSAMPROW)&image_buffer[(cinfocomp->next_scanline - GetSubsetPosY())
                                                    * row_stride];
        jpeg_write_scanlines(cinfocomp, row_pointer, 1);
        }

    // The next subset pos is the same as the limit of the last subset.
    SetSubsetPosY(EndPosY);

    // is it the last packet?
    if(GetSubsetPosY() == GetHeight())
        {
        jpeg_finish_compress(cinfocomp);

        HCDCodecJPEG::Reset();
        }

    return (po_OutBufferSize - ((my_destination_mgr*)(cinfocomp->dest))->pub.free_in_buffer);
    }

//-----------------------------------------------------------------------------
// public
// CreateTables
//-----------------------------------------------------------------------------
uint32_t IJG12BITS(HCDCodecIJG)::CreateTables(void* po_pOutBuffer, uint32_t pi_OutBufferSize)
    {
    HPRECONDITION(po_pOutBuffer != 0);
    HPRECONDITION(pi_OutBufferSize > 0);
    HPRECONDITION(pi_OutBufferSize < INT_MAX);

    // If the Quantization tables come from an external source, either the
    // tables are set manually or read through an external header, we must
    // not set the quality as it will overwrite the previously allocated
    // quantization tables.
    if(!m_ExternalQuantizationTablesUse && m_HeaderSize == 0)
        SetQuality(m_Quality);

    ((my_destination_mgr*)(cinfocomp->dest))->pub.next_output_byte = (Byte*)po_pOutBuffer;
    ((my_destination_mgr*)(cinfocomp->dest))->pub.free_in_buffer = pi_OutBufferSize;
    ((my_destination_mgr*)(cinfocomp->dest))->m_pData = (Byte*)po_pOutBuffer;
    ((my_destination_mgr*)(cinfocomp->dest))->m_DataSize = pi_OutBufferSize;

    jpeg_write_tables(cinfocomp);

    return (uint32_t)(pi_OutBufferSize - ((my_destination_mgr*)(cinfocomp->dest))->pub.free_in_buffer);
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t IJG12BITS(HCDCodecIJG)::DecompressSubset(const void*  pi_pInData,
                                                size_t pi_InDataSize,
                                                void*  po_pOutBuffer,
                                                size_t pi_OutBufferSize)
    {
    HPRECONDITION(pi_InDataSize < INT_MAX);
    HPRECONDITION(pi_OutBufferSize < INT_MAX);
    
    ((my_source_mgr*)(cinfodec->src))->pub.bytes_in_buffer = pi_InDataSize;
    ((my_source_mgr*)(cinfodec->src))->pub.next_input_byte = (Byte*)pi_pInData;
    ((my_source_mgr*)(cinfodec->src))->m_DataSize = pi_InDataSize;
    ((my_source_mgr*)(cinfodec->src))->m_pData = (Byte*)pi_pInData;

    // is the first subset?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        jpeg_read_header(cinfodec, true);

        // HCHK HL Patch
        if(GetBitsPerPixel() != 32)
            {
            SetColorMode(m_ColorMode);

            // !!!! HChkSebG !!!!
            // Must take care if jpeg data source has been stored as RGB (should'nt but happen..) or YCbCr
            // wich is standard.
            if (m_StoredColorMode == HCDCodecIJG::RGB)
                cinfodec->jpeg_color_space = JCS_RGB;
            }
        else
            cinfodec->out_color_space = JCS_CMYK;

        // ne pas oublier que l'on peut consulter le size d'une image
        // a partir de ce header!!! -> width, height, bits per pixel

        jpeg_start_decompress(cinfodec);
        }

    JSAMPROW row_pointer[1];    /* pointer to a single row */
    size_t row_stride;            /* physical row width in buffer */

    row_stride = (GetSubsetWidth() * GetBitsPerPixel() + GetLinePaddingBits()) / 8;

    Byte* image_buffer = (Byte*)po_pOutBuffer;

    size_t Height = GetSubsetPosY() + GetSubsetHeight();

    while (cinfodec->output_scanline < Height)
        {
        // not really that...
        row_pointer[0] = (JSAMPROW)&image_buffer[(cinfodec->output_scanline - GetSubsetPosY()) * row_stride];
        if (jpeg_read_scanlines(cinfodec, row_pointer, 1) == 0)
            break;
        }


    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    // is it the last subset?
    if(GetSubsetPosY() == GetHeight())
        {

        //TR #161327 : we check if the EOI tab is present. If its not, the  jpeg_finish_decompress() function will never exit and MS will freeze.
        my_src_ptr pCheck = ((my_src_ptr)(cinfodec->src)) ;
        bool found = false;

        if (((my_source_mgr*)(cinfodec->src))->pub.bytes_in_buffer == 0)
            jpeg_finish_decompress(cinfodec);
        else
            {
            for(uint32_t i=0; i < ((my_source_mgr*)(cinfodec->src))->pub.bytes_in_buffer && found == false; i++)
                {
                if(pCheck->pub.next_input_byte[0] == 0xFF)
                    {
                    if(pCheck->pub.next_input_byte[1] == 0xD9)
                        {
                        jpeg_finish_decompress(cinfodec);
                        found = true;
                        }
                    }
                pCheck->pub.next_input_byte++;
                }
            }

        HCDCodecJPEG::Reset();
        }
    else
        {
        SetCompressedImageIndex(GetCompressedImageIndex() +
                                (pi_InDataSize - ((my_source_mgr*)(cinfodec->src))->pub.bytes_in_buffer));
        }

    return(row_stride * GetSubsetHeight());
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool IJG12BITS(HCDCodecIJG)::HasLineAccess() const
    {
    return false;
    }

//-----------------------------------------------------------------------------
// public
// GetMinimumSubsetSize
//-----------------------------------------------------------------------------
size_t IJG12BITS(HCDCodecIJG)::GetMinimumSubsetSize() const
    {
    size_t ImageWidthInBytes = (GetWidth() * GetBitsPerPixel() + GetLinePaddingBits() + 7) / 8;
    size_t MaxLineCount = (1024*1024) / ImageWidthInBytes;  // 1 meg is big enough

    return (GetHeight() < MaxLineCount)
           ? (ImageWidthInBytes * GetHeight())
           : (ImageWidthInBytes * MaxLineCount);
    }


//-----------------------------------------------------------------------------
// public
// InitObject
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::InitObject()
    {
    m_ExternalQuantizationTablesUse = false;

    m_cinfocomp = 0;
    m_cinfodec  = 0;
    m_ErrorManager.pub = 0;

    m_cinfocomp          = new struct jpeg_compress_struct;
    m_cinfodec           = new struct jpeg_decompress_struct;
    m_ErrorManager.pub   = (Byte*)new struct jpeg_error_mgr;

    ////////////////////////////////////////
    // Set a custom error handler
    ////////////////////////////////////////

    // Set custom error handlers in the compression/decompression
    // structure of the m_Jpeg member.  This is done to avoid using
    // the default handlers which use exit() when an error occurs

    // set the jpeg error manager to a standard
    cinfodec->err  = jpeg_std_error((jpeg_error_mgr*)m_ErrorManager.pub);
    cinfocomp->err = jpeg_std_error((jpeg_error_mgr*)m_ErrorManager.pub);

    ((jpeg_error_mgr*)(m_ErrorManager.pub))->error_exit = (void(*)(j_common_ptr))::HCDJpegErrorExit;
    jpeg_create_compress(cinfocomp);

    s_jpeg_mem_dest(  cinfocomp,
                    (Byte*)0,
                    0);

    jpeg_create_decompress(cinfodec);

    s_jpeg_mem_src(   cinfodec,
                    (Byte*)0,
                    0);

    // Default color mode for stored pixels.
    m_StoredColorMode = HCDCodecIJG::YCC;

    if(GetBitsPerPixel() == 8)
        {
        SetColorMode(HCDCodecIJG::GRAYSCALE);
        SetSubsamplingMode(HCDCodecIJG::SNONE);
        }
    else if (GetBitsPerPixel() == 32)
        {
        SetColorMode(HCDCodecIJG::RGBA);
        SetSubsamplingMode(HCDCodecIJG::S411);
        }
    else if (GetBitsPerPixel() == 48)       // 12 bits compression
        {
        SetColorMode(HCDCodecIJG::RGB);
        SetSubsamplingMode(HCDCodecIJG::S411);
        }
    else
        {
        SetColorMode(HCDCodecIJG::RGB);              // Default RGB (24 or 48)
        SetSubsamplingMode(HCDCodecIJG::S411);
        }

    cinfocomp->image_width = static_cast<JDIMENSION>(GetWidth()); // image width and height, in pixels
    cinfocomp->image_height = static_cast<JDIMENSION>(GetHeight());

    m_Quality = 100;
    SetQuality(m_Quality);

    SetOptimizeCoding(false);

    m_AbbreviateMode = false;
    m_HeaderSize = 0;
    m_pHeader = 0;
    SetSubsamplingMode(HCDCodecIJG::S411);
    m_ProgressiveMode = false;
    }

//-----------------------------------------------------------------------------
// private
// DeepCopy
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::DeepCopy(const IJG12BITS(HCDCodecIJG)& pi_rObj)
    {
    InitObject();

    m_StoredColorMode = pi_rObj.GetSourceColorMode();

    SetAbbreviateMode(pi_rObj.m_AbbreviateMode);
    SetQuality(pi_rObj.m_Quality);
    SetProgressiveMode(pi_rObj.m_ProgressiveMode);
    SetColorMode((HCDCodecIJG::ColorModes)(pi_rObj.m_ColorMode));
    SetOptimizeCoding(pi_rObj.m_OptimizeCoding);

    // copy the decompressor info
    if(pi_rObj.m_HeaderSize != 0)
        {
        ReadHeader(pi_rObj.m_pHeader, pi_rObj.m_HeaderSize);
        CopyTablesFromDecoderToEncoder();
        }

    // copy the compressor info
    m_ExternalQuantizationTablesUse = pi_rObj.m_ExternalQuantizationTablesUse;
    if(m_ExternalQuantizationTablesUse)
        {
        int tblno;

        m_QuantizationTables.resize(NUM_QUANT_TBLS , HUINTVector());

        /* Copy the source's quantization tables. */
        for (tblno = 0; tblno < NUM_QUANT_TBLS; tblno++)
            {
            if(pi_rObj.m_QuantizationTables[tblno].size() != 0)
                SetQuantizationTable(tblno, (const unsigned int*)&pi_rObj.m_QuantizationTables[tblno][0], false);
            else
                m_QuantizationTables[tblno].clear();
            }
        }
    SetSubsamplingMode(pi_rObj.m_SubsamplingMode);
    }

//-----------------------------------------------------------------------------
// private
// DeepDelete
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::DeepDelete()
    {
    if (cinfocomp)
        jpeg_destroy_compress(cinfocomp);
    if (cinfodec)
        jpeg_destroy_decompress(cinfodec);

    delete cinfocomp;
    delete cinfodec;

    delete m_ErrorManager.pub;
    }


//-----------------------------------------------------------------------------
// public
// Reset
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::Reset()
    {
    if (GetCurrentState() == STATE_COMPRESS)
        jpeg_abort_compress(cinfocomp);
    else if (GetCurrentState() == STATE_DECOMPRESS)
        jpeg_abort_decompress(cinfodec);

    HCDCodecJPEG::Reset();
    }

//-----------------------------------------------------------------------------
// public
// SetOptimizeCoding
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::SetOptimizeCoding(bool pi_Enable)
    {
    HASSERT(cinfocomp != 0);

    m_OptimizeCoding = pi_Enable;
    cinfocomp->optimize_coding = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// SetQuality
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::SetQuality(Byte pi_Percentage)
    {
    HPRECONDITION(pi_Percentage <= 100);
    HASSERT(cinfocomp != 0);

    m_Quality = pi_Percentage;

    if (m_ExternalQuantizationTablesUse)
        {
        for (int tblno = 0; tblno < NUM_QUANT_TBLS; tblno++)
            {
            if(m_QuantizationTables[tblno].size() != 0)
                {
                jpeg_add_quant_table (cinfocomp,
                                      tblno,
                                      &m_QuantizationTables[tblno][0],
                                      jpeg_quality_scaling(m_Quality),  // set to 50% to preserve table values
                                      true);
                }
            }
        }
    else
        jpeg_set_quality(cinfocomp, pi_Percentage, true);
    }

//-----------------------------------------------------------------------------
// public
// GetOptimizeCoding
//-----------------------------------------------------------------------------
bool IJG12BITS(HCDCodecIJG)::GetOptimizeCoding() const
    {
    return(m_OptimizeCoding);
    }

//-----------------------------------------------------------------------------
// public
// GetQuality
//-----------------------------------------------------------------------------
Byte IJG12BITS(HCDCodecIJG)::GetQuality() const
    {
    return m_Quality;
    }


//-----------------------------------------------------------------------------
// public
// SetProgressiveMode
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::SetProgressiveMode(bool pi_Enable)
    {
    if(pi_Enable)
        {
        jpeg_simple_progression(cinfocomp);
        }
    else
        {
        if(cinfocomp->scan_info != NULL)
            {
            free(const_cast<jpeg_scan_info*>(cinfocomp->scan_info));
            cinfocomp->scan_info = NULL;
            cinfocomp->num_scans = 0;
            }
        }

    m_ProgressiveMode = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// IsProgressive
//-----------------------------------------------------------------------------
bool IJG12BITS(HCDCodecIJG)::IsProgressive() const
    {
    return m_ProgressiveMode;
    }

//-----------------------------------------------------------------------------
// public
// SetBitsPerPixel
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::SetBitsPerPixel(size_t pi_BitsPerPixel)
    {
    HCDCodecJPEG::SetBitsPerPixel(pi_BitsPerPixel);

    if(pi_BitsPerPixel == 8)
        SetColorMode(HCDCodecIJG::GRAYSCALE);
    else if (pi_BitsPerPixel == 32)
        SetColorMode(HCDCodecIJG::RGBA);
    else
        SetColorMode(HCDCodecIJG::RGB);      // Default RGB (24 or 48)
    }

//-----------------------------------------------------------------------------
// public
// SetColorMode
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::SetColorMode(HCDCodecIJG::ColorModes pi_Mode)
    {
    m_ColorMode = pi_Mode;

    if(m_ColorMode == HCDCodecIJG::YCC)
        {
        cinfocomp->in_color_space = JCS_YCbCr;
        cinfodec->out_color_space = JCS_YCbCr;
        }
    else if(m_ColorMode == HCDCodecIJG::RGB)
        {
        cinfocomp->in_color_space = JCS_RGB;
        cinfodec->out_color_space = JCS_RGB;
        }
    else if(m_ColorMode == HCDCodecIJG::BGR)
        {
        cinfocomp->in_color_space = JCS_UNKNOWN;
        cinfodec->out_color_space = JCS_UNKNOWN;
        }
    else if(m_ColorMode == HCDCodecIJG::GRAYSCALE)
        {
        cinfocomp->in_color_space = JCS_GRAYSCALE;
        cinfodec->out_color_space = JCS_GRAYSCALE;
        }
    else if(m_ColorMode == HCDCodecIJG::CMYK)
        {
        cinfocomp->in_color_space = JCS_CMYK;
        cinfodec->out_color_space = JCS_CMYK;
        }
    else if(m_ColorMode == HCDCodecIJG::RGBA)
        {
        cinfocomp->in_color_space = JCS_UNKNOWN;
        cinfodec->out_color_space = JCS_UNKNOWN;
        }
    else if(m_ColorMode == HCDCodecIJG::UNKNOWN)
        {
        cinfocomp->in_color_space = JCS_UNKNOWN;
        cinfodec->out_color_space = JCS_UNKNOWN;
        }

#ifdef JPEGLIB_SUPPORT_12BITS
    cinfocomp->input_components = static_cast<int>(GetBitsPerPixel() / 16);
    cinfodec->output_components = static_cast<int>(GetBitsPerPixel() / 16);
#else
    cinfocomp->input_components = static_cast<int>(GetBitsPerPixel() / 8);
    cinfodec->output_components = static_cast<int>(GetBitsPerPixel() / 8);
#endif

    jpeg_set_defaults(cinfocomp);

    // !!!! HChkSebG !!!!
    // Must take care if jpeg data source has been stored as RGB (should'nt but happen..) or YCbCr
    // wich is standard.
    if (m_StoredColorMode == HCDCodecIJG::RGB)
        jpeg_set_colorspace(cinfocomp, JCS_RGB);

    if (m_ColorMode == HCDCodecIJG::GRAYSCALE)
        SetSubsamplingMode(HCDCodecIJG::SNONE);
    }

//-----------------------------------------------------------------------------
// public
// GetColorMode
//-----------------------------------------------------------------------------
HCDCodecIJG::ColorModes IJG12BITS(HCDCodecIJG)::GetColorMode() const
    {
    return (HCDCodecIJG::ColorModes)m_ColorMode;
    }

//-----------------------------------------------------------------------------
// public
// ReadHeader
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::ReadHeader(const void* pi_pInData, size_t pi_InDataSize)
    {
    HPRECONDITION(pi_pInData != 0);
    HPRECONDITION(pi_InDataSize > 0);
    HPRECONDITION(pi_InDataSize < INT_MAX);

    ((my_source_mgr*)(cinfodec->src))->pub.bytes_in_buffer = pi_InDataSize;
    ((my_source_mgr*)(cinfodec->src))->pub.next_input_byte = (Byte*)pi_pInData;
    ((my_source_mgr*)(cinfodec->src))->m_DataSize = pi_InDataSize;
    ((my_source_mgr*)(cinfodec->src))->m_pData = (Byte*)pi_pInData;

    jpeg_read_header(cinfodec, false);

    m_HeaderSize = (uint32_t)(pi_InDataSize - ((my_source_mgr*)(cinfodec->src))->pub.bytes_in_buffer);
    m_pHeader = new Byte[m_HeaderSize];
    memcpy(m_pHeader, pi_pInData, m_HeaderSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Byte const* IJG12BITS(HCDCodecIJG)::GetAbbreviateTableHeader(size_t& headerSize) const
    {
    headerSize = m_HeaderSize;
    return m_pHeader;
    }

//-----------------------------------------------------------------------------
// public
// SetDimensions
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::SetDimensions(size_t pi_Width, size_t pi_Height)
    {
    cinfocomp->image_width = static_cast<JDIMENSION>(pi_Width);
    cinfocomp->image_height = static_cast<JDIMENSION>(pi_Height);
    cinfodec->image_width = static_cast<JDIMENSION>(pi_Width);
    cinfodec->image_height = static_cast<JDIMENSION>(pi_Height);

    HCDCodecJPEG::SetDimensions(pi_Width, pi_Height);
    }

//-----------------------------------------------------------------------------
// public
// SetAbbreviateMode
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::SetAbbreviateMode(bool pi_Enable)
    {
    m_AbbreviateMode = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// GetAbbreviateMode
//-----------------------------------------------------------------------------
bool IJG12BITS(HCDCodecIJG)::GetAbbreviateMode() const
    {
    return m_AbbreviateMode;
    }

//-----------------------------------------------------------------------------
// public
// SetSubsamplingMode
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::SetSubsamplingMode(HCDCodecIJG::SubsamplingModes pi_Mode)
    {
    m_SubsamplingMode = pi_Mode;

    switch (m_SubsamplingMode)
        {
        case HCDCodecIJG::S422:
            cinfocomp->comp_info[0].h_samp_factor = 2;
            cinfocomp->comp_info[0].v_samp_factor = 2;
            cinfocomp->comp_info[1].h_samp_factor = 1;
            cinfocomp->comp_info[1].v_samp_factor = 2;
            cinfocomp->comp_info[2].h_samp_factor = 1;
            cinfocomp->comp_info[2].v_samp_factor = 2;
            break;
        case HCDCodecIJG::S411:
            cinfocomp->comp_info[0].h_samp_factor = 2;
            cinfocomp->comp_info[0].v_samp_factor = 2;
            cinfocomp->comp_info[1].h_samp_factor = 1;
            cinfocomp->comp_info[1].v_samp_factor = 1;
            cinfocomp->comp_info[2].h_samp_factor = 1;
            cinfocomp->comp_info[2].v_samp_factor = 1;
            break;
        case HCDCodecIJG::SNONE:
            cinfocomp->comp_info[0].h_samp_factor = 1;
            cinfocomp->comp_info[0].v_samp_factor = 1;
            cinfocomp->comp_info[1].h_samp_factor = 1;
            cinfocomp->comp_info[1].v_samp_factor = 1;
            cinfocomp->comp_info[2].h_samp_factor = 1;
            cinfocomp->comp_info[2].v_samp_factor = 1;
            break;
        }
    }

//-----------------------------------------------------------------------------
// public
// SetQuantizationTable
// PLEASE CALL FOR SLOT 0 BEFORE SLOT 1
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::SetQuantizationTable(int pi_Slot, const unsigned int* pi_pTable, bool pi_UnZigZag)
    {
    if(!m_ExternalQuantizationTablesUse)
        {
        m_QuantizationTables.resize(NUM_QUANT_TBLS, HUINTVector());
        m_ExternalQuantizationTablesUse = true;
        }
    jpeg_add_quant_table (cinfocomp,
                            pi_Slot,
                            pi_pTable,
                            jpeg_quality_scaling(m_Quality),  // set to 50% to preserve table values
                            true);

    // stock it
    m_QuantizationTables[pi_Slot].resize(64);
    memcpy(&m_QuantizationTables[pi_Slot][0], pi_pTable, 64 * sizeof(uint32_t));
    }

//-----------------------------------------------------------------------------
// public
// GetSubsamplingMode
//-----------------------------------------------------------------------------

HCDCodecIJG::SubsamplingModes IJG12BITS(HCDCodecIJG)::GetSubsamplingMode() const
    {
    return m_SubsamplingMode;
    }

//-----------------------------------------------------------------------------
// public
// SetSourceColorMode
//-----------------------------------------------------------------------------

void IJG12BITS(HCDCodecIJG)::SetSourceColorMode(HCDCodecIJG::ColorModes pi_Mode)
    {
    m_StoredColorMode = pi_Mode;
    }

//-----------------------------------------------------------------------------
// public
// GetSourceColorMode
//-----------------------------------------------------------------------------

HCDCodecIJG::ColorModes IJG12BITS(HCDCodecIJG)::GetSourceColorMode() const
    {
    return m_StoredColorMode;
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t IJG12BITS(HCDCodecIJG)::GetSubsetMaxCompressedSize() const
    {
    HPRECONDITION(cinfocomp->num_components > 0 && cinfocomp->num_components < 5);

    return (GetSubsetWidth() * cinfocomp->num_components) * GetSubsetHeight() * 2 + //SAFETY
           MAX_COMPRESSED_SIZE_SAFETY_OFFSET;
    }

//-----------------------------------------------------------------------------
// public
// CopyTablesFromDecoderToEncoder
//-----------------------------------------------------------------------------
void IJG12BITS(HCDCodecIJG)::CopyTablesFromDecoderToEncoder()
    {
    //    jpeg_copy_critical_parameters(cinfodec, cinfocomp);
    /* jpeg_set_defaults may choose wrong colorspace, eg YCbCr if input is RGB.
        * Fix it to get the right header markers for the image colorspace.
        */
    JQUANT_TBL** qtblptr;
    int tblno;

    /*jpeg_set_colorspace(cinfocomp, cinfodec->jpeg_color_space);
    cinfocomp->data_precision = cinfodec->data_precision;
    cinfocomp->CCIR601_sampling = cinfodec->CCIR601_sampling;
    */ /* Copy the source's quantization tables. */
    for (tblno = 0; tblno < NUM_QUANT_TBLS; tblno++)
        {
        if (cinfodec->quant_tbl_ptrs[tblno] != NULL)
            {
            qtblptr = & cinfocomp->quant_tbl_ptrs[tblno];
            if (*qtblptr == NULL)
                *qtblptr = jpeg_alloc_quant_table((j_common_ptr) cinfocomp);
            memcpy((*qtblptr)->quantval,
                    cinfodec->quant_tbl_ptrs[tblno]->quantval,
                    sizeof((*qtblptr)->quantval));
            (*qtblptr)->sent_table = false;
            }
        }
    /* Copy the source's per-component info.
        * Note we assume jpeg_set_defaults has allocated the dest comp_info array.
        */
    /*cinfocomp->num_components = cinfodec->num_components;
    if (cinfocomp->num_components < 1 || cinfocomp->num_components > MAX_COMPONENTS)
        ERREXIT2(cinfocomp, JERR_COMPONENT_COUNT, cinfocomp->num_components,
            MAX_COMPONENTS);
    for (ci = 0, incomp = cinfodec->comp_info, outcomp = cinfocomp->comp_info;
            ci < cinfocomp->num_components; ci++, incomp++, outcomp++)
    {
        outcomp->component_id = incomp->component_id;
        outcomp->h_samp_factor = incomp->h_samp_factor;
        outcomp->v_samp_factor = incomp->v_samp_factor;
        outcomp->quant_tbl_no = incomp->quant_tbl_no;
        /* Make sure saved quantization table for component matches the qtable
        * slot.  If not, the input file re-used this qtable slot.
        * IJG encoder currently cannot duplicate this.
        *//*
tblno = outcomp->quant_tbl_no;
if (tblno < 0 || tblno >= NUM_QUANT_TBLS ||
cinfodec->quant_tbl_ptrs[tblno] == NULL)
ERREXIT1(cinfocomp, JERR_NO_QUANT_TABLE, tblno);
slot_quant = cinfodec->quant_tbl_ptrs[tblno];
c_quant = incomp->quant_table;
if (c_quant != NULL)
{
for (coefi = 0; coefi < DCTSIZE2; coefi++)
{
if (c_quant->quantval[coefi] != slot_quant->quantval[coefi])
ERREXIT1(cinfocomp, JERR_MISMATCHED_QUANT_TABLE, tblno);
}
}*/
    /* Note: we do not copy the source's Huffman table assignments;
        * instead we rely on jpeg_set_colorspace to have made a suitable choice.
        */
    // }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IJG12BITS(HCDCodecIJG)::MergeDQT_DHT(const Byte* pi_pInData,
                                              size_t pi_InDataSize,
                                              Byte* po_pOutBuffer,
                                              size_t po_OutBufferSize)
    {
#ifdef HAVE_TRANSUPP
    // ******************************* IMPORTANT ************************************************
    // This is how we would merge DQT_DHT using jpeglib API. However it is slow so we implemented 
    // HCDCodecIJG::MakeInterchangeFormat that copy jpeg makers and insert DQT/DHT tables along the way.
    // We keep this code as a reference.
    // Implementation is based on jpegtran.c from libjpeg.

    struct jpeg_decompress_struct srcinfo;
    struct jpeg_compress_struct dstinfo;

    jvirt_barray_ptr * src_coef_arrays;
    jvirt_barray_ptr * dst_coef_arrays;

    /* Initialize the JPEG decompression object with default error handling. */
    srcinfo.err = jpeg_std_error((jpeg_error_mgr*)m_ErrorManager.pub);
    jpeg_create_decompress(&srcinfo);
    /* Initialize the JPEG compression object with default error handling. */
    dstinfo.err = jpeg_std_error((jpeg_error_mgr*)m_ErrorManager.pub);
    jpeg_create_compress(&dstinfo);

    jpeg_mem_src(&srcinfo, const_cast<Byte*>(m_pHeader.get()), (uint32_t)m_HeaderSize);

    /* Read file header no data*/
    jpeg_read_header(&srcinfo, false/*NoData*/);

    jpeg_mem_src(&srcinfo, const_cast<Byte*>(pi_pInData), (uint32_t)pi_InDataSize);

    /* Enable saving of extra markers that we want to copy */
    jcopy_markers_setup(&srcinfo, JCOPYOPT_ALL);

    jpeg_read_header(&srcinfo, true);

    /* Read source file as DCT coefficients */
    src_coef_arrays = jpeg_read_coefficients(&srcinfo);

    /* Initialize destination compression parameters from source values */
    jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

    /* Adjust destination parameters if required by transform options;
    * also find out which set of coefficient arrays will hold the output.
    */
    dst_coef_arrays = src_coef_arrays;

    //   /* Close input file, if we opened it.
    //    * Note: we assume that jpeg_read_coefficients consumed all input
    //    * until JPEG_REACHED_EOI, and that jpeg_finish_decompress will
    //    * only consume more while (! cinfo->inputctl->eoi_reached).
    //    * We cannot call jpeg_finish_decompress here since we still need the
    //    * virtual arrays allocated from the source object for processing.
    //    */
    //   if (fp != stdin)
    //     fclose(fp);


    /* Specify data destination for compression */
    unsigned long outBufferSize = (unsigned long)po_OutBufferSize;
    jpeg_mem_dest(&dstinfo, &po_pOutBuffer, &outBufferSize);

    /* Start compressor (note no image data is actually written here) */
    jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

    /* Copy to the output file any extra markers that we want to preserve */
    jcopy_markers_execute(&srcinfo, &dstinfo, JCOPYOPT_ALL);

    /* Finish compression and release memory */
    jpeg_finish_compress(&dstinfo);
    jpeg_destroy_compress(&dstinfo);
    jpeg_finish_decompress(&srcinfo);
    jpeg_destroy_decompress(&srcinfo);

    return outBufferSize;		
#else
    BeAssert(!"Do not have jpeglib transupp.h support");  // see HAVE_TRANSUPP
    return 0;
#endif
    }

//----------------------------------------------------------------------------
// DESTINATION MANAGER ***********************
// modified to be in memory instead of beeing in file
//----------------------------------------------------------------------------
/*
 * jdatadst.c
 *
 * Copyright (C) 1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains compression data destination routines for the case of
 * emitting JPEG data to a file (or any stdio stream).  While these routines
 * are sufficient for most applications, some will want to use a different
 * destination manager.
 * IMPORTANT: we assume that fwrite() will correctly transcribe an array of
 * JOCTETs into 8-bit-wide elements on external storage.  If char is wider
 * than 8 bits on your machine, you may need to do some tweaking.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */



/* Expanded data destination object for stdio output */

/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */

METHODDEF (void)
init_destination (j_compress_ptr cinfo)
    {
    my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

    dest->pub.next_output_byte = dest->m_pData;
    dest->pub.free_in_buffer = dest->m_DataSize;
    }


/*
 * Empty the output buffer --- called whenever buffer fills up.
 *
 * In typical applications, this should write the entire output buffer
 * (ignoring the current state of next_output_byte & free_in_buffer),
 * reset the pointer & count to the start of the buffer, and return true
 * indicating that the buffer has been dumped.
 *
 * In applications that need to be able to suspend compression due to output
 * overrun, a false return indicates that the buffer cannot be emptied now.
 * In this situation, the compressor will return to its caller (possibly with
 * an indication that it has not accepted all the supplied scanlines).  The
 * application should resume compression after it has made more room in the
 * output buffer.  Note that there are substantial restrictions on the use of
 * suspension --- see the documentation.
 *
 * When suspending, the compressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_output_byte & free_in_buffer
 * indicate where the restart point will be if the current call returns false.
 * Data beyond this point will be regenerated after resumption, so do not
 * write it out when emptying the buffer externally.
 */

METHODDEF (boolean)
empty_output_buffer (j_compress_ptr cinfo)
    {
    my_dest_ptr dest = (my_dest_ptr) cinfo->dest;

    HASSERT(!"Increase the number of the define MAX_COMPRESSED_SIZE_SAFETY_OFFSET");

    dest->pub.next_output_byte = dest->m_pData;
    dest->pub.free_in_buffer = dest->m_DataSize;

    return true;
    }


/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF (void)
term_destination (j_compress_ptr cinfo)
    {
//  my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
    }


/*
 * Prepare for output to a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing compression.
 */

static void
s_jpeg_mem_dest (j_compress_ptr cinfo, Byte* pi_pData, size_t pi_DataSize )
    {
    my_dest_ptr dest;

    /* The destination object is made permanent so that multiple JPEG images
     * can be written to the same file without re-executing jpeg_stdio_dest.
     * This makes it dangerous to use this manager and a different destination
     * manager serially with the same JPEG object, because their private object
     * sizes may be different.  Caveat programmer.
     */
    if (cinfo->dest == NULL) {    /* first time for this JPEG object? */
        cinfo->dest = (struct jpeg_destination_mgr*)
                      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                                  sizeof(my_destination_mgr));
        }

    dest = (my_dest_ptr) cinfo->dest;
    dest->pub.init_destination = init_destination;
    dest->pub.empty_output_buffer = empty_output_buffer;
    dest->pub.term_destination = term_destination;
    dest->m_pData = pi_pData;
    dest->m_DataSize = pi_DataSize;
    }


//----------------------------------------------------------------------------
// SOURCE MANAGER ***********************
// modified to be in memory instead of beeing in file
//----------------------------------------------------------------------------
/*
 * jdatasrc.c
 *
 * Copyright (C) 1994, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains decompression data source routines for the case of
 * reading JPEG data from a file (or any stdio stream).  While these routines
 * are sufficient for most applications, some will want to use a different
 * source manager.
 * IMPORTANT: we assume that fread() will correctly transcribe an array of
 * JOCTETs from 8-bit-wide elements on external storage.  If char is wider
 * than 8 bits on your machine, you may need to do some tweaking.
 */

/* this is not a core library module, so it doesn't define JPEG_INTERNALS */


/* Expanded data source object for stdio input */

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF (void)
init_source (j_decompress_ptr cinfo)
    {
//  my_src_ptr src = (my_src_ptr) cinfo->src;

    }


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return true
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a false return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns false.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF (boolean)
fill_input_buffer (j_decompress_ptr cinfo)
    {
    my_src_ptr src = (my_src_ptr) cinfo->src;

    src->pub.next_input_byte = src->m_pData;
    src->pub.bytes_in_buffer = src->m_DataSize;

    return true;
    }


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

METHODDEF (void)
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
    {
    my_src_ptr src = (my_src_ptr) cinfo->src;

    /* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
     * any trouble anyway --- large skips are infrequent.
     */
    if (num_bytes > 0) {
        while (num_bytes > (long) src->pub.bytes_in_buffer) {
            num_bytes -= (long) src->pub.bytes_in_buffer;
            (void) fill_input_buffer(cinfo);
            /* note we assume that fill_input_buffer will never return false,
             * so suspension need not be handled.
             */
            }
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
        }
    }


/*
 * An additional method that can be provided by data source modules is the
 * resync_to_restart method for error recovery in the presence of RST markers.
 * For the moment, this source module just uses the default resync method
 * provided by the JPEG library.  That method assumes that no backtracking
 * is possible.
 */


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF (void)
term_source (j_decompress_ptr cinfo)
    {
    /* no work necessary here */
    }


/*
 * Prepare for input from a stdio stream.
 * The caller must have already opened the stream, and is responsible
 * for closing it after finishing decompression.
 */

static void
s_jpeg_mem_src (j_decompress_ptr cinfo, Byte* pi_pData, size_t pi_DataSize)
    {
    my_src_ptr src = NULL;

    /* The source object and input buffer are made permanent so that a series
     * of JPEG images can be read from the same file by calling jpeg_stdio_src
     * only before the first one.  (If we discarded the buffer at the end of
     * one image, we'd likely lose the start of the next one.)
     * This makes it unsafe to use this manager and a different source
     * manager serially with the same JPEG object.  Caveat programmer.
     */
    if (cinfo->src == NULL) {    /* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr*)
                     (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                                 sizeof(my_source_mgr));
        src = (my_src_ptr) cinfo->src;
        }

    src->m_pData = pi_pData;
    src->m_DataSize = pi_DataSize;
    src = (my_src_ptr) cinfo->src;
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src->pub.term_source = term_source;
    src->pub.bytes_in_buffer = pi_DataSize; /* forces fill_input_buffer on first read */
    src->pub.next_input_byte = pi_pData; /* until buffer loaded */
    }


