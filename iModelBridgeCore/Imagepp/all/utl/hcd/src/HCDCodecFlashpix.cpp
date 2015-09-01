//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecFlashpix.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecFlashpix
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HFCMath.h>
#include <Imagepp/all/h/HCDCodecIJG.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

static const uint32_t s_MaxTables     = 256;
static const uint32_t s_EncoderTable  = 255;  // the last table

#define HCD_CODEC_NAME     L"Flashpix"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecFlashpix::HCDCodecFlashpix()
    : HCDCodecImage(HCD_CODEC_NAME),
      m_Tables(s_MaxTables)
    {
    InitObject(RGB, 0, 0);
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecFlashpix::HCDCodecFlashpix(size_t pi_Width,
                                   size_t pi_Height,
                                   HCDCodecFlashpix::ColorModes pi_Mode)
    : HCDCodecImage(HCD_CODEC_NAME),
      m_Tables(s_MaxTables)
    {
    InitObject(pi_Mode, pi_Width, pi_Height);
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecFlashpix::HCDCodecFlashpix(const HCDCodecFlashpix& pi_rObj)
    : HCDCodecImage(pi_rObj),
      m_Tables(s_MaxTables)
    {
    InitObject((HCDCodecFlashpix::ColorModes)pi_rObj.m_ColorMode, pi_rObj.GetWidth(), pi_rObj.GetHeight());

    DeepCopy(pi_rObj);

    SetCurrentTable(GetCurrentTable());
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecFlashpix::~HCDCodecFlashpix()
    {
    DeepDelete();
    }


//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecFlashpix::Clone() const
    {
    return new HCDCodecFlashpix(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecFlashpix::CompressSubset(    const void* pi_pInData,
                                            size_t pi_InDataSize,
                                            void* po_pOutBuffer,
                                            size_t pi_OutBufferSize)
    {
    Byte* pIn;

    if (m_TableSelection != 0)
        m_pCodecJPEG->CopyTablesFromDecoderToEncoder();

    // if we are in NIF RGB with OPACITY, invert the RGB channels
    HAutoPtr<Byte> pTmpData;
    if(m_ColorMode == RGB_OPACITY)
        {
        size_t BytesCount = (GetSubsetWidth() * GetBitsPerPixel() + GetLinePaddingBits()) / 8
                            * GetSubsetHeight();

        pTmpData = new Byte[BytesCount];
        memcpy(pTmpData, (Byte*)pi_pInData, BytesCount);

        Byte* pData = pTmpData;

        Byte TmpData[3];

        while(BytesCount != 0)
            {
            // invert the RGBs
            TmpData[0] = 255 - pData[0];
            TmpData[1] = 255 - pData[1];
            TmpData[2] = 255 - pData[2];

            // convert from RGB to YCC
            pData[0] = (Byte)(MAX(0, MIN(255, TmpData[0] * 0.29900 + TmpData[1] * 0.58700 + TmpData[2] * 0.11400)));
            pData[1] = (Byte)(MAX(0, MIN(255, TmpData[0] * (- 0.16874) + TmpData[1] * (- 0.33126) + TmpData[2] * 0.5 + 127.5)));
            pData[2] = (Byte)(MAX(0, MIN(255, TmpData[0] * 0.5 + TmpData[1] * (-0.41869) + TmpData[2] * (-0.08131) + 127.5)));

            BytesCount -= 4;
            pData += 4;
            }

        pIn = pTmpData;
        }
    else
        {
        pIn = (Byte*)pi_pInData;
        }

    if(!m_DefaultTableUpdated && (m_TableSelection == s_EncoderTable))
        UpdateDefaultTable();

    if (m_TableSelection == 0)
        SetQuality(GetQuality());

    // compress the data
    size_t Result = m_pCodecJPEG->CompressSubset( pIn,
                                                  pi_InDataSize,


                                                  po_pOutBuffer,
                                                  pi_OutBufferSize);

    return (Result);
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecFlashpix::DecompressSubset(  const void* pi_pInData,
                                            size_t pi_InDataSize,
                                            void* po_pOutBuffer,
                                            size_t pi_OutBufferSize)
    {
    // decompress the data
    size_t SubsetSize = m_pCodecJPEG->DecompressSubset( pi_pInData,
                                                        pi_InDataSize,
                                                        po_pOutBuffer,
                                                        pi_OutBufferSize);


    // if we are in NIF RGB with OPACITY, invert the RGB channels
    if(m_ColorMode == RGB_OPACITY)
        {
        Byte* pData = (Byte*)po_pOutBuffer;

        size_t BytesCount = SubsetSize;

        Byte TmpData[3];

        while(BytesCount != 0)
            {
            // convert from YCC to RGB
            TmpData[0] = (Byte)(MAX(0, MIN(255, pData[0] + 1.40200 * pData[2] - 178.55)));
            TmpData[1] = (Byte)(MAX(0, MIN(255, pData[0] - 0.34313 * pData[1] - 0.71414 * pData[2] + 134.9307)));
            TmpData[2] = (Byte)(MAX(0, MIN(255, pData[0] + 1.77200 * pData[1] - 225.93)));

            // invert the RGBs
            pData[0] = 255 - TmpData[0];
            pData[1] = 255 - TmpData[1];
            pData[2] = 255 - TmpData[2];

            HFCMath (*pQuotients) (HFCMath::GetInstance());

            // reaclculate the premultiplication because the JPEG is lossy
            // and thw two alphas are not necesseraly equal

            if(pData[3] != 0)
                {
                pData[0] = pQuotients->DivideBy255ToByte(MIN(pData[0] * 255 / pData[3], 255) * pData[3]);
                pData[1] = pQuotients->DivideBy255ToByte(MIN(pData[1] * 255 / pData[3], 255) * pData[3]);
                pData[2] = pQuotients->DivideBy255ToByte(MIN(pData[2] * 255 / pData[3], 255) * pData[3]);
                }
            else
                {
                pData[0] = 0;
                pData[1] = 0;
                pData[2] = 0;
                }

            BytesCount -= 4;
            pData += 4;
            }
        }

    return SubsetSize;
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecFlashpix::IsBitsPerPixelSupported(size_t pi_Bits) const
    {
    if(pi_Bits == 8 || pi_Bits == 16 || pi_Bits == 24 || pi_Bits == 32)
        return true;
    else
        return false;
    }

//-----------------------------------------------------------------------------
// private
// DeepCopy
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::DeepCopy(const HCDCodecFlashpix& pi_rObj)
    {
    m_ColorMode = pi_rObj.m_ColorMode;

    m_pCodecJPEG = (HCDCodecIJG*)(pi_rObj.m_pCodecJPEG)->Clone();

    // copy the tables
    m_LastTable = pi_rObj.m_LastTable;
    m_TableSelection = pi_rObj.m_TableSelection;
    m_DefaultTableUpdated = pi_rObj.m_DefaultTableUpdated;
    for (uint32_t i = 0; i < s_MaxTables; i++)
        {
        // Initialize
        m_Tables[i].pData   = 0;
        m_Tables[i].BufSize = 0;

        // Copy from source
        if (pi_rObj.m_Tables[i].pData != 0)
            SetTable(i, pi_rObj.m_Tables[i].pData, pi_rObj.m_Tables[i].BufSize);
        }

    // HLX... NOT USED
    m_InterleaveMode        = pi_rObj.m_InterleaveMode;
    m_SubSampling           = pi_rObj.m_SubSampling;
    m_EnableColorConvert    = pi_rObj.m_EnableColorConvert;
    }


//-----------------------------------------------------------------------------
// private
// DeepDelete
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::DeepDelete()
    {
    for (size_t i = 0; i < s_MaxTables; i++)
        {
        delete[] m_Tables[i].pData;
        m_Tables[i].pData   = 0;
        m_Tables[i].BufSize = 0;
        }
    }

//-----------------------------------------------------------------------------
// public
// GetColorMode
//-----------------------------------------------------------------------------
HCDCodecFlashpix::ColorModes HCDCodecFlashpix::GetColorMode() const
    {
    return (ColorModes)m_ColorMode;
    }

//-----------------------------------------------------------------------------
// private
// InitObject
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::InitObject(HCDCodecFlashpix::ColorModes pi_Mode,
                                  size_t pi_Width,
                                  size_t pi_Height)
    {
    m_pCodecJPEG = new HCDCodecIJG();

    // Initialize items in the tables vector
    for (uint32_t i = 0; i < s_MaxTables; i++)
        {
        m_Tables[i].pData   = 0;
        m_Tables[i].BufSize = 0;
        }
    m_LastTable         = ULONG_MAX;
    m_TableSelection    = 0;

    //HLX.. Not used Set default settings
    m_InterleaveMode   = false;
    m_SubSampling       = 0x22;
    m_EnableColorConvert = true;

    SetDimensions(pi_Width, pi_Height);
    SetColorMode(pi_Mode);

    m_DefaultTableUpdated = false;
    }

//-----------------------------------------------------------------------------
// public
// SetColorMode
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::SetColorMode(HCDCodecFlashpix::ColorModes pi_Mode)
    {
    m_ColorMode = pi_Mode;

    uint32_t BitsPerPixel;
    HCDCodecIJG::ColorModes ColorMode;
    switch(pi_Mode)
        {
        case PHOTOYCC:
            ColorMode = HCDCodecIJG::YCC;
            BitsPerPixel = 24;
            break;
        case PHOTOYCC_OPACITY:
            ColorMode = HCDCodecIJG::UNKNOWN;
            BitsPerPixel = 32;
            break;
        case RGB_OPACITY:
            ColorMode = HCDCodecIJG::UNKNOWN;
            BitsPerPixel = 32;
            break;
        case MONOCHROME:
            ColorMode = HCDCodecIJG::GRAYSCALE;
            BitsPerPixel = 8;
            break;
        case MONOCHROME_OPACITY:
            ColorMode = HCDCodecIJG::UNKNOWN;
            BitsPerPixel = 16;
            break;

        case RGB:
        default:
            HASSERT(RGB == pi_Mode);
            ColorMode = HCDCodecIJG::RGB;
            BitsPerPixel = 24;
            break;
        }

    SetBitsPerPixel(BitsPerPixel);
    m_pCodecJPEG->SetBitsPerPixel(BitsPerPixel);
    m_pCodecJPEG->SetColorMode(ColorMode);

    m_DefaultTableUpdated = false;
    }

//-----------------------------------------------------------------------------
// public
// SetDeimensions
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::SetDimensions(size_t pi_Width, size_t pi_Height)
    {
    m_pCodecJPEG->SetDimensions(pi_Width, pi_Height);

    HCDCodecImage::SetDimensions(pi_Width, pi_Height);

    m_DefaultTableUpdated = false;
    }

//-----------------------------------------------------------------------------
// public
// SetTable
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::SetTable(uint32_t pi_Table,
                                Byte* pi_pTable,
                                size_t pi_TableSize)
    {
    HPRECONDITION(pi_pTable != 0);
    HPRECONDITION(pi_TableSize > 0);
    HPRECONDITION(pi_Table < s_MaxTables);

    // If the table to set is the last table used,
    // then invalidate the last table
    if (pi_Table == m_LastTable)
        m_LastTable = ULONG_MAX;

    // destroy the current table data if availabe
    delete[] m_Tables[pi_Table].pData;
    m_Tables[pi_Table].pData   = 0;
    m_Tables[pi_Table].BufSize = 0;

    // Allocate the data for the table
    m_Tables[pi_Table].pData = new Byte[pi_TableSize];
    HASSERT(m_Tables[pi_Table].pData != 0);

    // Copy the table data and set the size
    memcpy(m_Tables[pi_Table].pData, pi_pTable, pi_TableSize);
    m_Tables[pi_Table].BufSize = pi_TableSize;

    if(pi_Table == s_EncoderTable)
        m_DefaultTableUpdated = true;
    }

//-----------------------------------------------------------------------------
// public
// GetQuality
//-----------------------------------------------------------------------------
Byte HCDCodecFlashpix::GetQuality() const
    {
    return m_pCodecJPEG->GetQuality();
    }

//-----------------------------------------------------------------------------
// public
// SetQuality
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::SetQuality(Byte pi_Percentage)
    {
    HPRECONDITION(pi_Percentage <= 100);

    m_pCodecJPEG->SetQuality(pi_Percentage);

    m_DefaultTableUpdated = false;
    }

//-----------------------------------------------------------------------------
// public
// SetLinePaddingBits
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::SetLinePaddingBits(size_t pi_Bits)
    {
    HCDCodecImage::SetLinePaddingBits(pi_Bits);

    m_pCodecJPEG->SetLinePaddingBits(pi_Bits);
    }

//-----------------------------------------------------------------------------
// public
// EnableInterleave
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::EnableInterleave(bool pi_Enable)
    {
    // HLX... NOT USED
    m_InterleaveMode = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// IsInterleaveEnabled
//-----------------------------------------------------------------------------
bool HCDCodecFlashpix::IsInterleaveEnabled() const
    {
    // HLX.. NOT USED
    return m_InterleaveMode;
    }

//-----------------------------------------------------------------------------
// public
// SetSubsampling
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::SetSubSampling(Byte pi_SubSampling)
    {
    // HLX... NOT USED
    m_SubSampling = pi_SubSampling;
    }

//-----------------------------------------------------------------------------
// public
// GetSubsampling
//-----------------------------------------------------------------------------
Byte HCDCodecFlashpix::GetSubSampling() const
    {
    // HLX... NOT USED
    return m_SubSampling;
    }

//-----------------------------------------------------------------------------
// public
// EnableColorConversion
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::EnableColorConversion(bool pi_Enable)
    {
    // HLX... NOT USED
    m_EnableColorConvert = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// IsColorConversionEnabled
//-----------------------------------------------------------------------------
bool HCDCodecFlashpix::IsColorConversionEnabled() const
    {
    // HLX... NOT USED
    return m_EnableColorConvert;
    }

//-----------------------------------------------------------------------------
// public
// GetEncoderTable
//-----------------------------------------------------------------------------
uint32_t HCDCodecFlashpix::GetEncoderTable() const
    {
    return (s_EncoderTable);
    }


//-----------------------------------------------------------------------------
// public
// GetTableCount
//-----------------------------------------------------------------------------
uint32_t HCDCodecFlashpix::GetTableCount() const
    {
    return (s_MaxTables);
    }


//-----------------------------------------------------------------------------
// public
// GetTable
//-----------------------------------------------------------------------------
const Byte* HCDCodecFlashpix::GetTable(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < s_MaxTables);

    if(pi_Index == s_EncoderTable && !m_DefaultTableUpdated)
        UpdateDefaultTable();

    return (m_Tables[pi_Index].pData);
    }


//-----------------------------------------------------------------------------
// public
// GetTableSize
//-----------------------------------------------------------------------------
uint32_t HCDCodecFlashpix::GetTableSize(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < s_MaxTables);

    if(pi_Index == s_EncoderTable && !m_DefaultTableUpdated)
        UpdateDefaultTable();

    return (uint32_t)m_Tables[pi_Index].BufSize;
    }

//-----------------------------------------------------------------------------
// public
// SetCurrentTable
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::SetCurrentTable(uint32_t pi_Index)
    {
    HPRECONDITION(pi_Index < s_MaxTables);

    if(pi_Index == s_EncoderTable && !m_DefaultTableUpdated)
        UpdateDefaultTable();

    m_TableSelection = pi_Index;

    // if the table ID is 0, it means that the JPEG header is
    // part of the tile data.  Otherwise, it is in the table list.
    if (m_TableSelection != 0)
        {
        // Verify if the last table used is the same as this one
        if (m_LastTable != m_TableSelection)
            {
            // Set the last table id
            m_LastTable = m_TableSelection;

            // read the header
            m_pCodecJPEG->ReadHeader(m_Tables[m_TableSelection].pData,
                                     (uint32_t)m_Tables[m_TableSelection].BufSize);


            m_pCodecJPEG->CopyTablesFromDecoderToEncoder();
            }

        m_pCodecJPEG->SetAbbreviateMode(true);
        }
    else
        {
        m_pCodecJPEG->SetAbbreviateMode(false);
        }
    }


//-----------------------------------------------------------------------------
// public
// GetCurrentTable
//-----------------------------------------------------------------------------
uint32_t HCDCodecFlashpix::GetCurrentTable() const
    {
    return (m_TableSelection);
    }


//-----------------------------------------------------------------------------
// private
// UpdateDefaultTable
//-----------------------------------------------------------------------------
void HCDCodecFlashpix::UpdateDefaultTable()
    {
    // Create the encoder table based on the new settings
    Byte Table[1024];
    size_t TableSize = m_pCodecJPEG->CreateTables((Byte*)Table, 1024);
    SetTable(s_EncoderTable, (Byte*)Table, TableSize);

    m_DefaultTableUpdated = true;
    }

