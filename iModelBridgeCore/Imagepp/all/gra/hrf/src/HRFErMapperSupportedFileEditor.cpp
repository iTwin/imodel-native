//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFErMapperSupportedFileEditor.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFErMapperSupportedFileEditor
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#if defined(IPP_HAVE_ERMAPPER_SUPPORT) 

#include <Imagepp/all/h/HRFErMapperSupportedFile.h>
#include <Imagepp/all/h/HRFErMapperSupportedFileEditor.h>

// Includes from the ERMapper SDK
#include <ErMapperEcw/NCSECWClient.h>
#include <ErMapperEcw/NCSTypes.h>


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
static const unsigned short m_LinePadBits = 32;

enum Jpeg2000VisibleBand
    {
    RED_BAND = 0,
    GREEN_BAND,
    BLUE_BAND,
    ALPHA_EXT_BAND,
    };

//-----------------------------------------------------------------------------
// Private types
//-----------------------------------------------------------------------------
typedef uint32_t Jpeg2000Band;

//-----------------------------------------------------------------------------
// Private functions declaration
//-----------------------------------------------------------------------------
uint64_t   s_RemapXChannelsToRGB                              (Byte* const       po_pBIPOutputBuffer,
                                                                const Byte* const pi_pBILInputBuffer,
                                                                const uint64_t     pi_InputLength,
                                                                const uint32_t       pi_InputChannelQty,
                                                                const Jpeg2000Band  pi_Red,
                                                                const Jpeg2000Band  pi_Green,
                                                                const Jpeg2000Band  pi_Blue);

uint64_t   s_RemapXChannelsToRGBA                             (Byte* const       po_pBIPOutputBuffer,
                                                                const Byte* const pi_pBILInputBuffer,
                                                                const uint64_t     pi_InputLength,
                                                                const uint32_t       pi_InputChannelQty,
                                                                const Jpeg2000Band  pi_Red,
                                                                const Jpeg2000Band  pi_Green,
                                                                const Jpeg2000Band  pi_Blue,
                                                                const Jpeg2000Band  pi_Alpha);

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFErMapperSupportedFileEditor::HRFErMapperSupportedFileEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                               uint32_t              pi_Page,
                                                               unsigned short       pi_Resolution,
                                                               HFCAccessMode         pi_AccessMode)
    :   HRFResolutionEditor(pi_rpRasterFile,
                            pi_Page,
                            pi_Resolution,
                            pi_AccessMode)
    {
    HPRECONDITION(m_pRasterFile != 0);
    HPRECONDITION(pi_Resolution < ((HRFErMapperSupportedFile*)pi_rpRasterFile.GetPtr())->GetPageDescriptor(pi_Page)->CountResolutions());

    HRFErMapperSupportedFile* pRasterFile = (HRFErMapperSupportedFile*)pi_rpRasterFile.GetPtr();

    m_pRasterFile   = pi_rpRasterFile;
    m_ResNb         = pi_Resolution;

    m_pTileIDDesc   = new HGFTileIDDescriptor(m_pResolutionDescriptor->GetWidth(),
                                              m_pResolutionDescriptor->GetHeight(),
                                              m_pResolutionDescriptor->GetBlockWidth(),
                                              m_pResolutionDescriptor->GetBlockHeight());

    // TR#280888 Editor is only used for read access. At this point, when a file has no read
    // access, it will not be opened. Because it is not opened, required information for the
    // remaining editor initialization is unavailable. This explains the  Also, because HRARaster are created
    // in HUTExport by default even when we're only in Create/Write, we don't want to throw
    // any exception here.
    if (!pi_rpRasterFile->GetAccessMode().m_HasReadAccess)
        return;


    m_ChannelsQty   = ((NCSFileViewFileInfoEx*)pRasterFile->GetFileViewFileInfoEx())->nBands;

    m_LineBuffer    = new Byte[m_pResolutionDescriptor->GetBlockWidth()*m_ChannelsQty];



    // Adapt our line buffer to a channels buffers list in order to be able to
    // read BIL line using ECW API.
    m_ppLineChannelsBuffers = new Byte*[m_ChannelsQty];

    uint64_t iBufferOffset = 0;
    for (uint32_t iChannel = 0; iChannel < m_ChannelsQty; ++iChannel)
        {
        m_ppLineChannelsBuffers[iChannel] = m_LineBuffer + iBufferOffset;
        iBufferOffset += m_pResolutionDescriptor->GetBlockWidth();
        }
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFErMapperSupportedFileEditor::~HRFErMapperSupportedFileEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------

HSTATUS HRFErMapperSupportedFileEditor::ReadBlock(uint32_t pi_PosBlockX,
                                                  uint32_t pi_PosBlockY,
                                                  Byte* po_pData,
                                                  HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);

    HSTATUS Status = H_ERROR;
    HRFErMapperSupportedFile* pRasterFile = (HRFErMapperSupportedFile*)m_pRasterFile.GetPtr();

    HRFErMapperSupportedFile::TilePool::iterator Itr(pRasterFile->m_TilePool.find(m_pTileIDDesc->
                                                                                  ComputeID(pi_PosBlockX,
                                                                                          pi_PosBlockY,
                                                                                          m_Resolution)));

    if (Itr != pRasterFile->m_TilePool.end())
        {
        memcpy(po_pData, Itr->second, m_pResolutionDescriptor->GetBlockSizeInBytes());
        delete[] Itr->second;
        pRasterFile->m_TilePool.erase(Itr);
        Status = H_SUCCESS;
        }
    else
        {
        Status = ReadBlock(pi_PosBlockX,
                           pi_PosBlockY,
                           BLOCK_WIDTH_ERMAPPER,
                           BLOCK_HEIGHT_ERMAPPER,
                           po_pData,
                           pi_pSisterFileLock);
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFErMapperSupportedFileEditor::WriteBlock(uint32_t     pi_PosBlockX,
                                                   uint32_t     pi_PosBlockY,
                                                   const Byte* pi_pData,
                                                   HFCLockMonitor const* pi_pSisterFileLock)
    {
    HASSERT(0); // not supported
    return H_ERROR;
    }

//-----------------------------------------------------------------------------
// protected
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFErMapperSupportedFileEditor::ReadBlock(uint32_t              pi_PosBlockX,
                                                  uint32_t              pi_PosBlockY,
                                                  uint32_t              pi_BlockWidth,
                                                  uint32_t              pi_BlockHeight,
                                                  Byte*                po_pData,
                                                  HFCLockMonitor const* pi_pSisterFileLock)
    {
    HSTATUS                   Status = H_SUCCESS;
    unsigned short            i;
    unsigned char*            pData;
    unsigned long             LineSize;
    unsigned long             BlockWidth;
    unsigned long             BlockHeight;
    HRFErMapperSupportedFile* pRasterFile = (HRFErMapperSupportedFile*)m_pRasterFile.GetPtr();
    unsigned long             PosX = (unsigned long)((double)pi_PosBlockX / pRasterFile->GetRatio(m_ResNb));
    unsigned long             PosY = (unsigned long)((double)pi_PosBlockY / pRasterFile->GetRatio(m_ResNb));
    unsigned long             StdViewWidth = (unsigned long)((double)pi_BlockWidth / pRasterFile->GetRatio(m_ResNb));
    unsigned long             StdViewHeight = (unsigned long)((double)pi_BlockHeight / pRasterFile->GetRatio(m_ResNb));

    unsigned long ViewWidth = min((uint32_t)pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth() - PosX,
                                  StdViewWidth);


    if (ViewWidth < StdViewWidth)
        BlockWidth = (uint32_t)pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(m_ResNb)->GetWidth() - pi_PosBlockX;
    else
        BlockWidth = pi_BlockWidth;

    unsigned long ViewHeight = min((uint32_t)pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight() - PosY,
                                   StdViewHeight);

    if (ViewHeight < StdViewHeight)
        BlockHeight = (uint32_t)pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(m_ResNb)->GetHeight() - pi_PosBlockY;
    else
        BlockHeight = pi_BlockHeight;

    if (NCScbmSetFileView((NCSFileView*)(pRasterFile->GetFileView()),
                          m_ChannelsQty,
                          pRasterFile->GetBandList(),
                          PosX,
                          PosY,
                          PosX + ViewWidth - 1,
                          PosY + ViewHeight - 1,
                          BlockWidth,
                          BlockHeight) != 0)
        {
        Status = H_ERROR;
        }

    if (Status == H_SUCCESS)
        {
        pData       = po_pData;

        // Read data
        if (m_ChannelsQty == 1) // Grayscale
            {
            LineSize = pi_BlockWidth * 1;
            for (i = 0; i < BlockHeight; i++)
                {
                if (NCScbmReadViewLineBIL((NCSFileView*)(pRasterFile->GetFileView()), &pData) != 0)
                    {
                    Status = H_ERROR;
                    break;
                    }
                pData += LineSize;
                }
            }
        else if (m_ChannelsQty == 3 || HRFErMapperSupportedFile::s_SpecifiedBands.empty()) // RGB
            {
            LineSize = pi_BlockWidth * 3;
            for (i = 0; i < BlockHeight; i++)
                {
                if (NCScbmReadViewLineRGB((NCSFileView*)(pRasterFile->GetFileView()), pData) != 0)
                    {
                    Status = H_ERROR;
                    break;
                    }
                pData += LineSize;
                }
            }
        else
            {
            LineSize = pi_BlockWidth * m_ChannelsQty;

            if (3 == HRFErMapperSupportedFile::s_SpecifiedBands.size())
                {
                for (i = 0; i < BlockHeight; i++)
                    {
                    if (NCScbmReadViewLineBIL((NCSFileView*)(pRasterFile->GetFileView()), m_ppLineChannelsBuffers))
                        {
                        Status = H_ERROR;
                        break;
                        }

                    pData += s_RemapXChannelsToRGB( pData, m_LineBuffer, LineSize, m_ChannelsQty,
                                                    HRFErMapperSupportedFile::s_SpecifiedBands[RED_BAND],
                                                    HRFErMapperSupportedFile::s_SpecifiedBands[GREEN_BAND],
                                                    HRFErMapperSupportedFile::s_SpecifiedBands[BLUE_BAND]);
                    }
                }
            else
                {
                HASSERT(4 == HRFErMapperSupportedFile::s_SpecifiedBands.size());

                for (i = 0; i < BlockHeight; i++)
                    {
                    if (NCScbmReadViewLineBIL((NCSFileView*)(pRasterFile->GetFileView()), m_ppLineChannelsBuffers))
                        {
                        Status = H_ERROR;
                        break;
                        }

                    pData += s_RemapXChannelsToRGBA(pData, m_LineBuffer, LineSize, m_ChannelsQty,
                                                    HRFErMapperSupportedFile::s_SpecifiedBands[RED_BAND],
                                                    HRFErMapperSupportedFile::s_SpecifiedBands[GREEN_BAND],
                                                    HRFErMapperSupportedFile::s_SpecifiedBands[BLUE_BAND],
                                                    HRFErMapperSupportedFile::s_SpecifiedBands[ALPHA_EXT_BAND]);
                    }
                }

            }
        }
    return Status;
    }


//-----------------------------------------------------------------------------
// Static Private
// Remap an array of X Channels composites to an array of RGB composites
//-----------------------------------------------------------------------------
uint64_t s_RemapXChannelsToRGB  (Byte* const       po_pBIPOutputBuffer,
                                const Byte* const pi_pBILInputBuffer,
                                const uint64_t     pi_InputLength,
                                const uint32_t       pi_InputChannelQty,
                                const Jpeg2000Band  pi_Red,
                                const Jpeg2000Band  pi_Green,
                                const Jpeg2000Band  pi_Blue)
    {
    static const uint32_t OUTPUT_CHANNEL_QTY = 3;

    HPRECONDITION(0 != po_pBIPOutputBuffer);
    HPRECONDITION(0 != pi_pBILInputBuffer);
    HPRECONDITION(0 == (pi_InputLength % pi_InputChannelQty));
    HPRECONDITION(pi_InputChannelQty > pi_Red);
    HPRECONDITION(pi_InputChannelQty > pi_Green);
    HPRECONDITION(pi_InputChannelQty > pi_Blue);

    const uint64_t PixelsQty           = pi_InputLength / pi_InputChannelQty;

    const Byte*   pInRedChannel       = &pi_pBILInputBuffer[pi_Red * PixelsQty];
    const Byte*   pInGreenChannel     = &pi_pBILInputBuffer[pi_Green * PixelsQty];
    const Byte*   pInBlueChannel      = &pi_pBILInputBuffer[pi_Blue * PixelsQty];
    Byte*         pOutData            = po_pBIPOutputBuffer;

    for (uint64_t iPixel = 0; iPixel < PixelsQty; ++iPixel)
        {
        pOutData[RED_BAND]      = *pInRedChannel++;
        pOutData[GREEN_BAND]    = *pInGreenChannel++;
        pOutData[BLUE_BAND]     = *pInBlueChannel++;

        pOutData += OUTPUT_CHANNEL_QTY;
        }

    HPOSTCONDITION(pOutData == (po_pBIPOutputBuffer + (pi_InputLength / pi_InputChannelQty) * OUTPUT_CHANNEL_QTY));

    return static_cast<uint64_t>(pOutData - po_pBIPOutputBuffer);
    }


//-----------------------------------------------------------------------------
// Static Private
// Remap an array of X Channels composites to an array of RGBA composites
//-----------------------------------------------------------------------------
uint64_t s_RemapXChannelsToRGBA (Byte* const       po_pBIPOutputBuffer,
                                const Byte* const pi_pBILInputBuffer,
                                const uint64_t     pi_InputLength,
                                const uint32_t       pi_InputChannelQty,
                                const Jpeg2000Band  pi_Red,
                                const Jpeg2000Band  pi_Green,
                                const Jpeg2000Band  pi_Blue,
                                const Jpeg2000Band  pi_Alpha)
    {
    static const uint32_t OUTPUT_CHANNEL_QTY = 4;

    HPRECONDITION(0 != po_pBIPOutputBuffer);
    HPRECONDITION(0 != pi_pBILInputBuffer);
    HPRECONDITION(0 == (pi_InputLength % pi_InputChannelQty));
    HPRECONDITION(pi_InputChannelQty > pi_Red);
    HPRECONDITION(pi_InputChannelQty > pi_Green);
    HPRECONDITION(pi_InputChannelQty > pi_Blue);
    HPRECONDITION(pi_InputChannelQty > pi_Alpha);

    const uint64_t PixelsQty           = pi_InputLength / pi_InputChannelQty;

    const Byte*   pInRedChannel       = &pi_pBILInputBuffer[pi_Red * PixelsQty];
    const Byte*   pInGreenChannel     = &pi_pBILInputBuffer[pi_Green * PixelsQty];
    const Byte*   pInBlueChannel      = &pi_pBILInputBuffer[pi_Blue * PixelsQty];
    const Byte*   pInAlphaChannel     = &pi_pBILInputBuffer[pi_Alpha * PixelsQty];
    Byte*         pOutData            = po_pBIPOutputBuffer;

    for (uint64_t iPixel = 0; iPixel < PixelsQty; ++iPixel)
        {
        pOutData[RED_BAND]          = *pInRedChannel++;
        pOutData[GREEN_BAND]        = *pInGreenChannel++;
        pOutData[BLUE_BAND]         = *pInBlueChannel++;
        pOutData[ALPHA_EXT_BAND]    = *pInAlphaChannel++;

        pOutData += OUTPUT_CHANNEL_QTY;
        }

    HPOSTCONDITION(pOutData == (po_pBIPOutputBuffer + (pi_InputLength / pi_InputChannelQty) * OUTPUT_CHANNEL_QTY));

    return static_cast<uint64_t>(pOutData - po_pBIPOutputBuffer);
    }

#endif // IPP_HAVE_ERMAPPER_SUPPORT 
