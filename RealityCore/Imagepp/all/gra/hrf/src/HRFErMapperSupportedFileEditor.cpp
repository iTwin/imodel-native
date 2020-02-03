//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFErMapperSupportedFileEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFErMapperSupportedFile.h>

#if defined(IPP_HAVE_ERMAPPER_SUPPORT) 

#include <ImagePP/all/h/HRFErMapperSupportedFileEditor.h>
#include <ImagePP/all/h/HCPGCoordUtility.h>

// Includes from the ERMapper SDK
#include <ErdasEcwJpeg2000/NCSECWClient.h>
#include <ErdasEcwJpeg2000/NCSTypes.h>
#include <ErdasEcwJpeg2000/NCSUtil.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
static const uint16_t m_LinePadBits = 32;
static const uint64_t MAX_ECW_IMAGE_SIZE = 10000000000; //NCSEcw support 10Go uncompressed images

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
                                                               uint16_t       pi_Resolution,
                                                               HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                            pi_Page,
                            pi_Resolution,
                            pi_AccessMode)
    {
    HPRECONDITION(m_pRasterFile != 0);
    HPRECONDITION(pi_Resolution < ((HRFErMapperSupportedFile*) pi_rpRasterFile.GetPtr())->GetPageDescriptor(pi_Page)->CountResolutions());

    HRFErMapperSupportedFile* pRasterFile = (HRFErMapperSupportedFile*) pi_rpRasterFile.GetPtr();

    m_pRasterFile = pi_rpRasterFile;
    m_ResNb = pi_Resolution;

    m_pTileIDDesc = new HGFTileIDDescriptor(m_pResolutionDescriptor->GetWidth(),
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


    m_ChannelsQty = ((NCSFileViewFileInfoEx*) pRasterFile->GetFileViewFileInfoEx())->nBands;

    m_LineBuffer = new Byte[m_pResolutionDescriptor->GetBlockWidth()*m_ChannelsQty];



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

HSTATUS HRFErMapperSupportedFileEditor::ReadBlock(uint64_t    pi_PosBlockX,
                                                  uint64_t    pi_PosBlockY,
                                                  Byte*       po_pData)
    {
    HPRECONDITION(po_pData != 0);

    HSTATUS Status = H_ERROR;
    HRFErMapperSupportedFile* pRasterFile = (HRFErMapperSupportedFile*) m_pRasterFile.GetPtr();

    HRFErMapperSupportedFile::TilePool::iterator Itr(pRasterFile->m_TilePool.find(m_pTileIDDesc->
                                                                                  ComputeID(pi_PosBlockX,
                                                                                          pi_PosBlockY,
                                                                                          m_Resolution)));

    if (Itr != pRasterFile->m_TilePool.end())
        {
        memcpy(po_pData, Itr->second, m_pResolutionDescriptor->GetBlockSizeInBytes());
        delete [] Itr->second;
        pRasterFile->m_TilePool.erase(Itr);
        Status = H_SUCCESS;
        }
    else
        {
        Status = ReadBlock(pi_PosBlockX,
                             pi_PosBlockY,
                             BLOCK_WIDTH_ERMAPPER,
                             BLOCK_HEIGHT_ERMAPPER,
                             po_pData);
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFErMapperSupportedFileEditor::WriteBlock(uint64_t      pi_PosBlockX,
                                                   uint64_t      pi_PosBlockY,
                                                   const Byte*    pi_pData)
    {
    HASSERT(0); // not supported
    return H_ERROR;
    }

//-----------------------------------------------------------------------------
// protected
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFErMapperSupportedFileEditor::ReadBlock(uint64_t pi_PosBlockX,
                                                  uint64_t pi_PosBlockY,
                                                  uint64_t pi_BlockWidth,
                                                  uint64_t pi_BlockHeight,
                                                  Byte*   po_pData)
    {
    HSTATUS                   Status = H_SUCCESS;
    uint16_t            i;
    unsigned char*            pData;
    uint32_t             LineSize;
    uint32_t             BlockWidth;
    uint32_t             BlockHeight;
    HRFErMapperSupportedFile* pRasterFile = (HRFErMapperSupportedFile*) m_pRasterFile.GetPtr();
    uint32_t             PosX = (uint32_t) ((double) pi_PosBlockX / pRasterFile->GetRatio(m_ResNb));
    uint32_t             PosY = (uint32_t) ((double) pi_PosBlockY / pRasterFile->GetRatio(m_ResNb));
    uint32_t             StdViewWidth = (uint32_t) ((double) pi_BlockWidth / pRasterFile->GetRatio(m_ResNb));
    uint32_t             StdViewHeight = (uint32_t) ((double) pi_BlockHeight / pRasterFile->GetRatio(m_ResNb));

    uint32_t ViewWidth = MIN((uint32_t)pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth() - PosX,
                                  StdViewWidth);


    if (ViewWidth < StdViewWidth)
        BlockWidth = (uint32_t)(pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(m_ResNb)->GetWidth() - pi_PosBlockX);
    else
        BlockWidth = (uint32_t)pi_BlockWidth;

    uint32_t ViewHeight = MIN((uint32_t)pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight() - PosY,
                                   StdViewHeight);

    if (ViewHeight < StdViewHeight)
        BlockHeight = (uint32_t)(pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(m_ResNb)->GetHeight() - pi_PosBlockY);
    else
        BlockHeight = (uint32_t)pi_BlockHeight;

    if (NCScbmSetFileView((NCSFileView*) (pRasterFile->GetFileView()),
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

    // If the server is slow, need to wait for the data.
    NCSSetViewInfo *pInfo;
    if (NCSGetSetViewInfo((NCSFileView*) (pRasterFile->GetFileView()), &pInfo) == NCS_SUCCESS)
        {
        while (pInfo->nBlocksAvailable < pInfo->nBlocksInView)
            NCSSleep(100);
        }


    if (Status == H_SUCCESS)
        {
        pData = po_pData;

        // Read data
        if (m_ChannelsQty == 1) // Grayscale
            {
            LineSize = (uint32_t)pi_BlockWidth * 1;
            for (i = 0; i < BlockHeight; i++)
                {
                if (NCScbmReadViewLineBIL((NCSFileView*) (pRasterFile->GetFileView()), &pData) != 0)
                    {
                    Status = H_ERROR;
                    break;
                    }
                pData += LineSize;
                }
            }
        else if (m_ChannelsQty == 3 || HRFErMapperSupportedFile::GetSpecifiedBands().empty()) // RGB
            {
            LineSize = (uint32_t)pi_BlockWidth * 3;
            for (i = 0; i < BlockHeight; i++)
                {
                if (NCScbmReadViewLineRGB((NCSFileView*) (pRasterFile->GetFileView()), pData) != 0)
                    {
                    Status = H_ERROR;
                    break;
                    }
                pData += LineSize;
                }
            }
        else
            {
            LineSize = (uint32_t)pi_BlockWidth * m_ChannelsQty;

            if (3 == HRFErMapperSupportedFile::GetSpecifiedBands().size())
                {
                for (i = 0; i < BlockHeight; i++)
                    {
                    if (NCScbmReadViewLineBIL((NCSFileView*) (pRasterFile->GetFileView()), m_ppLineChannelsBuffers))
                        {
                        Status = H_ERROR;
                        break;
                        }

                    pData += s_RemapXChannelsToRGB(pData, m_LineBuffer, LineSize, m_ChannelsQty,
                        HRFErMapperSupportedFile::GetSpecifiedBands()[RED_BAND],
                        HRFErMapperSupportedFile::GetSpecifiedBands()[GREEN_BAND],
                        HRFErMapperSupportedFile::GetSpecifiedBands()[BLUE_BAND]);
                    }
                }
            else
                {
                HASSERT(4 == HRFErMapperSupportedFile::GetSpecifiedBands().size());

                for (i = 0; i < BlockHeight; i++)
                    {
                    if (NCScbmReadViewLineBIL((NCSFileView*) (pRasterFile->GetFileView()), m_ppLineChannelsBuffers))
                        {
                        Status = H_ERROR;
                        break;
                        }

                    pData += s_RemapXChannelsToRGBA(pData, m_LineBuffer, LineSize, m_ChannelsQty,
                        HRFErMapperSupportedFile::GetSpecifiedBands()[RED_BAND],
                        HRFErMapperSupportedFile::GetSpecifiedBands()[GREEN_BAND],
                        HRFErMapperSupportedFile::GetSpecifiedBands()[BLUE_BAND],
                        HRFErMapperSupportedFile::GetSpecifiedBands()[ALPHA_EXT_BAND]);
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

    const Byte*   pInRedChannel = &pi_pBILInputBuffer[pi_Red * PixelsQty];
    const Byte*   pInGreenChannel = &pi_pBILInputBuffer[pi_Green * PixelsQty];
    const Byte*   pInBlueChannel = &pi_pBILInputBuffer[pi_Blue * PixelsQty];
    Byte*         pOutData = po_pBIPOutputBuffer;

    for (uint64_t iPixel = 0; iPixel < PixelsQty; ++iPixel)
        {
        pOutData[RED_BAND] = *pInRedChannel++;
        pOutData[GREEN_BAND] = *pInGreenChannel++;
        pOutData[BLUE_BAND] = *pInBlueChannel++;

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

    const Byte*   pInRedChannel = &pi_pBILInputBuffer[pi_Red * PixelsQty];
    const Byte*   pInGreenChannel = &pi_pBILInputBuffer[pi_Green * PixelsQty];
    const Byte*   pInBlueChannel = &pi_pBILInputBuffer[pi_Blue * PixelsQty];
    const Byte*   pInAlphaChannel = &pi_pBILInputBuffer[pi_Alpha * PixelsQty];
    Byte*         pOutData = po_pBIPOutputBuffer;

    for (uint64_t iPixel = 0; iPixel < PixelsQty; ++iPixel)
        {
        pOutData[RED_BAND] = *pInRedChannel++;
        pOutData[GREEN_BAND] = *pInGreenChannel++;
        pOutData[BLUE_BAND] = *pInBlueChannel++;
        pOutData[ALPHA_EXT_BAND] = *pInAlphaChannel++;

        pOutData += OUTPUT_CHANNEL_QTY;
        }

    HPOSTCONDITION(pOutData == (po_pBIPOutputBuffer + (pi_InputLength / pi_InputChannelQty) * OUTPUT_CHANNEL_QTY));

    return static_cast<uint64_t>(pOutData - po_pBIPOutputBuffer);
    }


//-----------------------------------------------------------------------------
// Used by HutExportToFile.cpp

#ifdef __ECW_EXPORT_ENABLE__

#include <ImagePP/all/h/HUTExportProgressIndicator.h>
#include <ImagePP/all/h/HRFRasterFileExtender.h>
#include <ImagePP/all/h/HCDCodecErMapperSupported.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HRPPixelTypeV8Gray8.h>
#include <ImagePP/all/h/HGF2DTranslation.h>
#include <ImagePP/all/h/HRFException.h>
#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HRAPyramidRaster.h>
#include <ImagePP/all/h/HRSObjectStore.h>
#include <ImagePP/all/h/HRACopyFromOptions.h>

// Includes from the ERMapper  SDK
#include <ErdasEcwJpeg2000/NCSEcwCompressClient.h>
#include <ErdasEcwJpeg2000/NCSTypes.h>
//#include <ErdasEcwJpeg2000/NCSGDTLocation.h>
#include <ErdasEcwJpeg2000/NCSMalloc.h>

//----------------------------------------------------------------------------
// Structure to store client read data
struct ECWReadInfoStruct
    {
    ECWReadInfoStruct()
        : m_CancelExport(false)
        {
        };

    ~ECWReadInfoStruct()
        {
        };

    HFCPtr<HRARaster>               m_pSourceRaster;
    HFCPtr<HRAStoredRaster>         m_pDestinationRaster;
    HFCPtr<HRABitmap>               m_pDestinationBitmap;

    HFCPtr<HRFRasterFile>           m_pSourceFile;
    HFCPtr<HRFRasterFile>           m_pDestinationFile;
    HFCPtr<HGF2DTransfoModel>       m_pDestModel;           // Precompute the model to solve threading problem with Gcoord host.
#if 0 //DMx ECW SDK 5.0 support Geotiff Tag --> Version 3
    HFCPtr<HCPGeoTiffKeys>          m_pGeotiffKeys;
#endif

    HFCPtr<HGF2DTransfoModel>       m_pLineTranslateModel;
    HRACopyFromOptions              m_CopyFromOptions;

    uint32_t                          m_LineWidthInByte;

    // Buffer to store an error message in (1024 is an arbitrary size)
    char                            m_pErrorBuffer[1024];

    uint32_t                          m_StripHeight;
    uint32_t                          m_CurrentStripPos;

    // PDF export
    std::mutex                      m_cv_main_mutex;
    std::condition_variable         m_notifyMainThread;

    std::mutex                      m_cv_ecw_mutex;
    std::condition_variable         m_notifyEcwThread;

    bool                            m_CancelExport;
    Byte                            m_ECWVersion;
    };

void Export_ECW_Jpeg2000_Helper(ECWReadInfoStruct& pio_rReadInfo);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRFEcwCreator::_HandleExportToFile(HFCPtr<HRFRasterFile>& pDestinationFile, HFCPtr<HRAStoredRaster>& pDestinationRaster,
                                        HFCPtr<HRFRasterFile>& pSourceFile, HFCPtr<HRARaster>& pSourceRaster,
                                        HRACopyFromOptions const& copyFromOpts) const
    {
    // Initialize ECWReadInfoStruct
    ECWReadInfoStruct ReadInfoStruct;
    ReadInfoStruct.m_pSourceFile = pSourceFile;
    ReadInfoStruct.m_pSourceRaster = pSourceRaster;
    ReadInfoStruct.m_pDestinationFile = pDestinationFile;
    ReadInfoStruct.m_pDestinationRaster = pDestinationRaster;
    ReadInfoStruct.m_CopyFromOptions = copyFromOpts;

    HFCPtr<HRFRasterFile> pOriginalFile = pDestinationFile;
    if (pDestinationFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
        pOriginalFile = ((HFCPtr<HRFRasterFileExtender>&)pDestinationFile)->GetOriginalFile();
    ReadInfoStruct.m_ECWVersion = static_cast<HRFErMapperSupportedFile*>(pOriginalFile.GetPtr())->m_ECWVersion;

    // This code should be done in the method InitializeECWCallbackStruct, but we have a problem to call the GeoCoord Host in another thread, that is the
    // case here, for that reason, we translate the model here, instead of in the ECW export thread.
    // Set the transformation model
    ReadInfoStruct.m_pDestModel = (((HFCPtr<HRAStoredRaster> &)pDestinationRaster)->GetTransfoModel());

    // Translate the model units to geocoding units
    HFCPtr<HRFPageDescriptor>       pDstPageDesc = pDestinationFile->GetPageDescriptor(0);
    GeoCoordinates::BaseGCSCP pBaseGCS = pDstPageDesc->GetGeocodingCP();

    //If some geocoding information is available, get the transformation in the units specified by the geocoding information
    if (pBaseGCS != nullptr && pBaseGCS->IsValid()) 
        {
#if 0 //DMx ECW SDK 5.0 support Geotiff Tag --> Version 3
        ReadInfoStruct.m_pGeotiffKeys = new HCPGeoTiffKeys();
        baseGCS->SetGeoTiffKeys(ReadInfoStruct.m_pGeotiffKeys);
#endif
        ReadInfoStruct.m_pDestModel = HCPGCoordUtility::TranslateFromMeter(ReadInfoStruct.m_pDestModel, 1.0 / pBaseGCS->UnitsFromMeters(), 0);
        }

    Export_ECW_Jpeg2000_Helper(ReadInfoStruct);     // in HRFErMapperSupportedFileEditor.cpp
    return true; // handled.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRFJpeg2000Creator::_HandleExportToFile(HFCPtr<HRFRasterFile>& pDestinationFile, HFCPtr<HRAStoredRaster>& pDestinationRaster,
                                        HFCPtr<HRFRasterFile>& pSourceFile, HFCPtr<HRARaster>& pSourceRaster,
                                        HRACopyFromOptions const& copyFromOpts) const
    {
    return HRFEcwCreator::GetInstance()->_HandleExportToFile(pDestinationFile, pDestinationRaster, pSourceFile, pSourceRaster, copyFromOpts);
    }


// this class is use to initialize the ErMapper library only once.
// The library will be initializ ed on the first Init() call and will be
// shutdown by the destructor.
class ErMapperLibrary
    {
    public:
        ErMapperLibrary()
            : m_ErMapperInitalized(false)
            {
            };

        ~ErMapperLibrary()
            {
            if (m_ErMapperInitalized)
                NCSecwShutdown();
            };

        void Init()
            {
            if (!m_ErMapperInitalized)
                {
                NCSecwInit();           
                                        // SDK 5.3 key
                NCSEcwCompressSetOEMKey("Bentley Systems Incorporated", "0f6a3bf2bea4eaa2fa16cabe9b5f98bc261fb6b5cc4994da536b1bcce7e0f0d8f04cde9825f57b0b9128000f48c6ed7a183937bbe41830b28cc48aabd7208f9d");
                m_ErMapperInitalized = true;
                }
            };

        void Term()
            {
            // do nothing, the library will be shutdown by the destructor
            };

    private:
        bool m_ErMapperInitalized;
    };

// singleton on ErMapperLibrary.
// we can use a static variable to shutdown the library because ErMapper library's is static
static ErMapperLibrary s_ErMapperLibrary;


//----------------------------------------------------------------------------
// Callback Functions for ECW special export.
//
static BOOLEAN ReadCallback(NCSEcwCompressClient* pClient, uint32_t nNextLine, IEEE4** ppOutputBandBufferArray);
static BOOLEAN CancelCallback(NCSEcwCompressClient* pClient);

//----------------------------------------------------------------------------------------
// @bsimethod                                                  
//----------------------------------------------------------------------------------------
class ERMapperExporter
    {
    public:
        //----------------------------------------------------------------------------------------
        // @bsimethod                                                  
        //----------------------------------------------------------------------------------------
        ERMapperExporter(ECWReadInfoStruct& pi_rCompressReadInfo)
            : m_rCompressReadInfo(pi_rCompressReadInfo),
              m_ExportCompleted(false),
              m_LastError(NCS_SUCCESS)
            {
            };

        ~ERMapperExporter(){};

        //----------------------------------------------------------------------------------------
        // @bsimethod                                                  
        //----------------------------------------------------------------------------------------
        void operator()()
            {
            s_ErMapperLibrary.Init();

            if (InitializeECWCallbackStruct())
                {
                // Open the compression
                m_LastError = NCSEcwCompressOpen(m_pCompressClient, false);
                if (m_LastError == NCS_SUCCESS)
                    {
                    // Start progress indicator.
                    // Iteration occurs in StatusCallback(...), except for the last one that occurs
                    // after all the "hard work" (See a couples of lines below).
                uint64_t NbBlocks((uint64_t)ceil((double)(m_pCompressClient->nInOutSizeY)/(double)(m_rCompressReadInfo.m_StripHeight)));
                    HUTExportProgressIndicator::GetInstance()->Restart(NbBlocks);

#if 0 //DMx ECW SDK 5.0 support Geotiff Tag --> Version 3
                    GeoCoordinates::IGeoTiffKeysList::GeoKeyItem GeoTiffKey;
                    if (m_rCompressReadInfo.m_pGeotiffKeys->GetFirstKey(&GeoTiffKey) == true)
                        {
                        do  {
                            switch (GeoTiffKey.KeyDataType)
                                {
                                    case GeoCoordinates::IGeoTiffKeysList::ASCII:
                                        NCSCompressSetGeotiffKey(m_pCompressClient, (geokey_t) GeoTiffKey.KeyID, TYPE_ASCII, 1, GeoTiffKey.KeyValue.StringVal);
                                        break;

                                    case GeoCoordinates::IGeoTiffKeysList::DOUBLE:
                                        NCSCompressSetGeotiffKey(m_pCompressClient, (geokey_t) GeoTiffKey.KeyID, TYPE_DOUBLE, 1, GeoTiffKey.KeyValue.DoubleVal);
                                        break;

                                    case GeoCoordinates::IGeoTiffKeysList::LONG:
                                        NCSCompressSetGeotiffKey(m_pCompressClient, (geokey_t) GeoTiffKey.KeyID, TYPE_SHORT, 1, (int16_t) GeoTiffKey.KeyValue.LongVal);
                                        break;
                                }
                            } while (m_rCompressReadInfo.m_pGeotiffKeys->GetNextKey(&GeoTiffKey));

                            NCSCompressSetGeotiffKey(m_pCompressClient, GTRasterTypeGeoKey, TYPE_SHORT, 1, 1/*RasterPixelIsArea*/);
                            HFCMatrix<3, 3> TheMatrix;
                            double         aFileMatrix[16];

                            TheMatrix = m_rCompressReadInfo.m_pDestModel->GetMatrix();

                            // a-b-c-d
                            aFileMatrix[0] = TheMatrix[0][0];
                            aFileMatrix[1] = TheMatrix[0][1];
                            aFileMatrix[2] = 0.0;
                            aFileMatrix[3] = TheMatrix[0][2];

                            // e-f-g-h
                            aFileMatrix[4] = TheMatrix[1][0];
                            aFileMatrix[5] = TheMatrix[1][1];
                            aFileMatrix[6] = 0.0;
                            aFileMatrix[7] = TheMatrix[1][2];

                            // i-j-k-l
                            aFileMatrix[8] = 0.0;
                            aFileMatrix[9] = 0.0;
                            aFileMatrix[10] = 1.0;
                            aFileMatrix[11] = 0.0;

                            // m-n-o-p
                            aFileMatrix[12] = TheMatrix[2][0];
                            aFileMatrix[13] = TheMatrix[2][1];
                            aFileMatrix[14] = 0.0;
                            aFileMatrix[15] = TheMatrix[2][2];

                            NCSCompressSetGeotiffTag(m_pCompressClient, GTIFF_TRANSMATRIX, 16, aFileMatrix);
                        }
#endif

                    // Compress
                    m_LastError = NCSEcwCompress(m_pCompressClient);

                    if ((m_LastError == NCS_SUCCESS) ||
                        (m_LastError == NCS_USER_CANCELLED_COMPRESSION))
                        {
                        // Free client
                        if (NCSEcwCompressClose(m_pCompressClient) == NCS_SUCCESS)
                            m_ExportCompleted = true;
                        HDEBUGCODE(else HASSERT(false));
                        }
                    HDEBUGCODE(else HASSERT(false););
                    }
                else
                    {
                    //NCSEcwCompressOpen seems to return NCS_COULDNT_OPEN_COMPRESSION error when the input size is too big
                    //while the appropriate code should be NCS_INPUT_SIZE_EXCEEDED for an input exceding the SDK limit (10Go.)
                if (MAX_ECW_IMAGE_SIZE < (uint64_t)m_pCompressClient->nInOutSizeX * 
                                         (uint64_t)m_pCompressClient->nInOutSizeY * 
                                         (uint64_t)m_pCompressClient->nInputBands)
                        {
                        m_LastError = NCS_INPUT_SIZE_EXCEEDED;
                        HASSERT(false);
                        }
                    HDEBUGCODE(else HASSERT(m_LastError == NCS_INPUT_SIZE_TOO_SMALL););
                    }

                //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                // Free allocated memory
                if (m_pCompressClient)
                    {
                    NCSEcwCompressFreeClient(m_pCompressClient);
                    m_pCompressClient = 0;
                    }
                }
            HDEBUGCODE(else HASSERT(false););

            s_ErMapperLibrary.Term();

            HASSERT(!m_pCompressClient);

            m_ExportCompleted = true;   // This is how the main thread know that we are done. It doesn't mean we have a success.
            m_rCompressReadInfo.m_notifyMainThread.notify_all();     // notify we have completed.
            };

        //----------------------------------------------------------------------------------------
        // @bsimethod                                                  
        //----------------------------------------------------------------------------------------
        NCSError GetLastError()
            {
            return m_LastError;
            }

        //----------------------------------------------------------------------------------------
        // Initialize the callback structure for ErMapper API.
        // @bsimethod                                                  
        //----------------------------------------------------------------------------------------
        bool InitializeECWCallbackStruct()
            {
            bool CreationStatus = false;

            // Dynamic Load the ECW library
            if (m_pCompressClient = NCSCompressAllocClientW())
                {
                Utf8String FileName(((HFCPtr<HFCURLFile>&)m_rCompressReadInfo.m_pDestinationFile->GetURL())->GetAbsoluteFileName());
                
                WString FileNameW(FileName.c_str(), BentleyCharEncoding::Utf8);

                // Copy in string (unicode) to buffer (ansi)
                wcscpy_s(m_pCompressClient->uOutputFileName.wszOutputFileName, MAX_PATH, FileNameW.c_str());    // V5.0

                HFCPtr<HRFPageDescriptor>       pDstPageDesc = m_rCompressReadInfo.m_pDestinationFile->GetPageDescriptor(0);
                HFCPtr<HRFResolutionDescriptor> pDstResDesc = pDstPageDesc->GetResolutionDescriptor(0);

                // Set up the input dimensions
                HASSERT(pDstResDesc->GetWidth() <= UINT32_MAX);
                HASSERT(pDstResDesc->GetHeight() <= UINT32_MAX);

            m_pCompressClient->nInOutSizeX = (uint32_t)pDstResDesc->GetWidth();
            m_pCompressClient->nInOutSizeY = (uint32_t)pDstResDesc->GetHeight();

                // Specify the callbacks and client data ptr
                m_pCompressClient->pReadCallback = ReadCallback;
                m_pCompressClient->pStatusCallback = 0;
                m_pCompressClient->pCancelCallback = CancelCallback;

                m_pCompressClient->nFormatVersion = m_rCompressReadInfo.m_ECWVersion;
#if 1 //DMx ECW SDK 5.0 support Geotiff Tag --> Version 2

                if (m_rCompressReadInfo.m_pDestModel->IsStretchable())
                    {
                    double           ScaleFactorX;
                    double           ScaleFactorY;
                    HGF2DDisplacement Translation;

                    m_rCompressReadInfo.m_pDestModel->GetStretchParams(&ScaleFactorX, &ScaleFactorY, &Translation);

                    // Specifying the transformation model.
                    m_pCompressClient->fCellIncrementX = ScaleFactorX;
                    m_pCompressClient->fCellIncrementY = ScaleFactorY;

                    m_pCompressClient->fOriginX = Translation.GetDeltaX();
                    m_pCompressClient->fOriginY = Translation.GetDeltaY();

                    // TR 188117 (ECW lib problem)
                    char* pszProjection = 0;
                    char* pszDatum = 0;
                    bool IsRefCoordSysFound = false;
                    uint32_t EPSGCode = 0;
                    GeoCoordinates::BaseGCSCP baseGCS = pDstPageDesc->GetGeocodingCP();
 
                    if (baseGCS != nullptr && baseGCS->IsValid() && baseGCS->GetEPSGCode() != 0)
                        {
                        EPSGCode = baseGCS->GetEPSGCode();

                        // szProjection : ER Mapper style Projection name string, e.g. "RAW" or "GEODETIC".  Never NULL
                        // szDatum : Mapper style Datum name string, e.g. "RAW" or "NAD27".  Never NULL
                        if (NCS_SUCCESS == NCSGetProjectionAndDatum(EPSGCode, &pszProjection, &pszDatum))
                            {
                            if(/*IsProjected*/baseGCS->GetProjectionCode() != GeoCoordinates::BaseGCS::ProjectionCodeValue::pcvUnity)
                                {
                                // TR 265837 - Sometimes the return status is SUCCESS but the pszProjection is NULL (In this case the datum is correctly set)
                                // We will consider this identical to a not found status. A typical test case is EPSG:21892 a deprectated Bogot CRS replaced
                                // by EPSG:21897 ... It is not up to us nor ERMapper to determine what replacement should be set.
                                if (pszProjection != NULL)
                                    IsRefCoordSysFound = true;
                                }
                            else
                                {
                                IsRefCoordSysFound = true;
                                }
                            }
                        }

                    if (IsRefCoordSysFound)
                        {
                        strcpy(m_pCompressClient->szProjection, pszProjection);
                        strcpy(m_pCompressClient->szDatum, pszDatum);
                        NCSFree(pszProjection);
                        NCSFree(pszDatum);
                        }
                    else if (EPSGCode != 0) // the function NCSGetProjectionAndDatum returns an error now
                        {                   // as a workaround, ECW support suggest to do that.
                        char defaultStr[64];
                        sprintf_s(defaultStr, 63, "epsg:%d", EPSGCode);
                        strcpy(m_pCompressClient->szProjection, defaultStr);
                        strcpy(m_pCompressClient->szDatum, defaultStr);
                        }

                    // Specify the units in which world cell sizes are specified, e.g. meters, feet
                    if ((baseGCS != 0) && baseGCS->IsValid() && (baseGCS->GetEPSGUnitCode() == TIFFGeo_Linear_Foot_US_Survey))
                        m_pCompressClient->eCellSizeUnits = ECW_CELL_UNITS_FEET;
                    else
                        m_pCompressClient->eCellSizeUnits = ECW_CELL_UNITS_METERS;
                    }
                else

                    {
                    // Specifying the transformation model.
                    m_pCompressClient->fCellIncrementX = 1.0;
                    m_pCompressClient->fCellIncrementY = 1.0;

                    m_pCompressClient->fOriginX = 0.0;
                    m_pCompressClient->fOriginY = 0.0;

                    // Specify the units in which world cell sizes are specified, e.g. meters, feet
                    m_pCompressClient->eCellSizeUnits = ECW_CELL_UNITS_METERS;
                    }
#endif

                //Set the compression ratio
                HFCPtr<HRFRasterFile> pOriginalDestRaster(m_rCompressReadInfo.m_pDestinationFile);

                //Not the original file
                if (pOriginalDestRaster->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
                    pOriginalDestRaster = ((HFCPtr<HRFRasterFileExtender> &)m_rCompressReadInfo.m_pDestinationFile)->GetOriginalFile();

                HFCPtr<HRFResolutionDescriptor> pOriRasterResDesc = pOriginalDestRaster->GetPageDescriptor(0)->GetResolutionDescriptor(0);

                const HFCPtr<HCDCodec>& pCodec = pOriRasterResDesc->GetCodec();
                HASSERT(pCodec->IsCompatibleWith(HCDCodecErMapperSupported::CLASS_ID));
                m_pCompressClient->fTargetCompression = (IEEE4) ((HFCPtr<HCDCodecErMapperSupported>&)pCodec)->GetCompressionRatio();

                // Set up client data for read callback
                m_rCompressReadInfo.m_pDestinationBitmap  = HRABitmap::Create(m_pCompressClient->nInOutSizeX,
                                                                          pDstResDesc->GetBlockHeight(),
                                                                          m_rCompressReadInfo.m_pDestinationRaster->GetTransfoModel(),
                                                                          m_rCompressReadInfo.m_pDestinationRaster->GetCoordSys(),
                                                                          pDstResDesc->GetPixelType());

                m_rCompressReadInfo.m_StripHeight = pDstResDesc->GetBlockHeight();

                if (m_rCompressReadInfo.m_pDestinationBitmap->GetPacket()->GetBufferAddress() == 0)
                    {
                m_rCompressReadInfo.m_pDestinationBitmap = HRABitmap::Create(m_pCompressClient->nInOutSizeX,
                                                                             1,
                                                                             m_rCompressReadInfo.m_pDestinationRaster->GetTransfoModel(),
                                                                             m_rCompressReadInfo.m_pDestinationRaster->GetCoordSys(),
                                                                             pDstResDesc->GetPixelType());

                    m_rCompressReadInfo.m_StripHeight = 1;
                    }

                if (m_rCompressReadInfo.m_pDestinationBitmap->GetPacket()->GetBufferAddress() != 0)
                    {
                    m_rCompressReadInfo.m_CurrentStripPos = 0;

                    // ECW support only GrayScale or RGB
                    HPRECONDITION(pDstResDesc->GetPixelType()->CountValueBits() == 8 || pDstResDesc->GetPixelType()->CountValueBits() == 24);
                    m_rCompressReadInfo.m_LineWidthInByte = m_pCompressClient->nInOutSizeX * pDstResDesc->GetPixelType()->CountValueBits() / 8;

                    m_rCompressReadInfo.m_pErrorBuffer[0] = 0;

                    // Set up the default pixel type according the src pixel type.
                    if (pDstResDesc->GetPixelType()->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID))
                        {
                        m_pCompressClient->nInputBands = 1;
                        m_pCompressClient->eCompressFormat = COMPRESS_UINT8;
                        }
                    else
                        {
                        m_pCompressClient->nInputBands = 3;
                        m_pCompressClient->eCompressFormat = COMPRESS_RGB;
                        }

                    // Create a Translation transformation model to move the m_pDestinationBitmap
                    // over the SourceRaster physical line by physical line into our ReadCallback.
                    m_rCompressReadInfo.m_pLineTranslateModel = new HGF2DTranslation(
                        HGF2DDisplacement(0, m_rCompressReadInfo.m_StripHeight));
                    m_pCompressClient->pClientData = &m_rCompressReadInfo;
                    CreationStatus = true;
                    }
                }
            return CreationStatus;
            };

        //----------------------------------------------------------------------------------------
        // @bsimethod
        //----------------------------------------------------------------------------------------
        bool ExportCompleted() const
            {
            return m_ExportCompleted;
            }

        //----------------------------------------------------------------------------------------
        // @bsimethod                                                   
        //--------------------------------------------------------------------------------------
        void CancelExport()
            {
            m_rCompressReadInfo.m_CancelExport = true;
            }

        //private:

        bool                    m_ExportCompleted;
        ECWReadInfoStruct&      m_rCompressReadInfo;
        NCSEcwCompressClient*   m_pCompressClient;
        NCSError                m_LastError;
    };


//---------------------------------------------------------------------------
// Read callback function - called once for each input line
//---------------------------------------------------------------------------

static BOOLEAN ReadCallback(NCSEcwCompressClient* pClient,
                            uint32_t                nNextLine,
                            IEEE4**               ppOutputBandBufferArray)
    {
    HPRECONDITION(((ECWReadInfoStruct*) (pClient->pClientData))->m_pSourceRaster != 0);

    ECWReadInfoStruct* pReadInfo = (ECWReadInfoStruct*) pClient->pClientData;

    if (nNextLine == 0)
        {
        // Copy data from the source raster into the destination one.
        pReadInfo->m_pDestinationBitmap->Clear();
        // signal the event, the main thread will execute the CopyFrom...
        pReadInfo->m_notifyMainThread.notify_all();
        // wait until the main signal that the CopyFrom is terminate
        std::unique_lock<std::mutex> lk(pReadInfo->m_cv_ecw_mutex);
        pReadInfo->m_notifyEcwThread.wait(lk);
        }
    else if (nNextLine >= pReadInfo->m_CurrentStripPos + pReadInfo->m_StripHeight)
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Move the destination raster OVER the source file at the right position.
        HFCPtr<HGF2DTransfoModel> pLineModel = pReadInfo->m_pDestinationBitmap->GetTransfoModel();

        pLineModel = pReadInfo->m_pLineTranslateModel->ComposeInverseWithDirectOf(*pLineModel);
        pReadInfo->m_pDestinationBitmap->SetTransfoModel(*pLineModel, pReadInfo->m_pDestinationBitmap->GetCoordSys());

        // Copy data from the source raster into the destination one.
        pReadInfo->m_pDestinationBitmap->Clear();
        // signal the event, the main thread will execute the CopyFrom...
        pReadInfo->m_notifyMainThread.notify_all();
        // wait until the main signal that the CopyFrom is terminate
        std::unique_lock<std::mutex> lk(pReadInfo->m_cv_ecw_mutex);
        pReadInfo->m_notifyEcwThread.wait(lk);

        pReadInfo->m_CurrentStripPos = pReadInfo->m_CurrentStripPos + pReadInfo->m_StripHeight;
        }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Then convert our buffer into the ECW format.

    const Byte* pData = pReadInfo->m_pDestinationBitmap->GetPacket()->GetBufferAddress() + (nNextLine - pReadInfo->m_CurrentStripPos) * pReadInfo->m_LineWidthInByte;

    for (uint32_t Channel = 0; Channel < pClient->nInputBands; Channel++)
        {
        IEEE4* pOutputValue = ppOutputBandBufferArray[Channel];
        uint32_t ValueIndex = Channel;

        for (uint32_t PixelIndex = 0; PixelIndex < pClient->nInOutSizeX; PixelIndex++)
            {
            // Be sure to remain inbound.
            HASSERT(ValueIndex < pReadInfo->m_pDestinationBitmap->GetPacket()->GetBufferSize());

            // Compression needs input to be IEEE4
            pOutputValue[PixelIndex] = *(pData + ValueIndex);
            ValueIndex += pClient->nInputBands;
            }
        }
    return true;
    }

//---------------------------------------------------------------------------
// Cancel callback function
//---------------------------------------------------------------------------
static BOOLEAN CancelCallback(NCSEcwCompressClient* pClient)
    {
    HPRECONDITION(((ECWReadInfoStruct*) (pClient->pClientData))->m_pSourceRaster != 0);

    ECWReadInfoStruct* pReadInfo = (ECWReadInfoStruct*) pClient->pClientData;

    return pReadInfo->m_CancelExport;
    }


void Export_ECW_Jpeg2000_Helper(ECWReadInfoStruct& pio_rReadInfo)
    {
    // TR #194882
    // PDF library must be run into a single thread, this means that the CopyFrom() must be run into the main thread.
    // When we export to ECW, the ReadCallback() function is called by another thread than the main thread. For this
    // reason, we create a new thread, the ECW API is initialized into the new thread, the ReadCallback() signal an
    // event for the main thread
    // Create the thread
    // This thread will initialize ECW API and call NCSEcwCompress()
    // The callback function ReadCallback will be called by ECW API for each line.
    // When the PDF data is needed, ReadCallback() will signal m_ComputeStripEvent.
    
    //DMx ------------------------------------------------------------------------------------------------
#if 0
    s_ErMapperLibrary.Init();
    ExportThread.InitializeECWCallbackStruct();

    NCSCompressClient *pClient;
    pClient = NCSCompressAllocClientW();

    /*
    ** Set up the input dimensions
    */

    // To make an opacity channel for rgb - 
    // 1. set TEST_NR_BANDS to 4 and
    // 2. set pClient->eCompressFormat to COMPRESS_RGB
    // To make a normal rgb set set TEST_NR_BANDS to 3
    pClient->nInputBands = 3;
    pClient->nInOutSizeX = 500;
    pClient->nInOutSizeY = 500;

    /*
    ** Set format and ratio based on # of input bands
    **
    ** Format can be:
    ** COMPRESS_UINT8 (Grayscale)
    ** COMPRESS_RGB   (RGB)
    ** COMPRESS_MULTI (Multi band grayscale)
    ** COMPRESS_YUV	  (YUV)
    */
    if (pClient->nInputBands == 1) {
        pClient->eCompressFormat = NCS_COMPRESS_UINT8;
        pClient->fTargetCompression = 20.0f;
        }
    else if (pClient->nInputBands == 4) {
        pClient->eCompressFormat = NCS_COMPRESS_RGB;
        pClient->fTargetCompression = 10.0f;
        }
    else {
        pClient->eCompressFormat = NCS_COMPRESS_MULTI;
        pClient->fTargetCompression = 20.0f;
        }

    /*
    ** Give output filename
    */
    //		strncpy(pClient->uOutputFileName.szOutputFileName, argv[1], MAX_PATH);      // ori
    wcscpy_s(pClient->uOutputFileName.wszOutputFileName, MAX_PATH, "k:\\tmp\\test2.ecw");    // V5.0

    /*
    ** Specify the callbacks and client data ptr
    */
    pClient->pReadCallback = ReadCallback;
    //		pClient->pStatusCallback = StatusCallback;
    pClient->pCancelCallback = CancelCallback;
    pClient->pClientData = pClient;



    NCSError LastError;
    LastError = NCSEcwCompressOpen(pClient /*ExportThread.m_pCompressClient*/, false);
#endif
    //DMx ------------------------------------------------------------------------------------------------
    // Create work and start thread. Use std::ref to share 'ECWExporter' with both threads.
    ERMapperExporter ECWExporter(pio_rReadInfo); 
    std::thread ecwThread(std::ref(ECWExporter));

    while(1)
        {
        std::unique_lock<std::mutex> lk(pio_rReadInfo.m_cv_main_mutex);
        pio_rReadInfo.m_notifyMainThread.wait(lk);

        // If the export completed, exited on error or whatever.
        if(ECWExporter.ExportCompleted())   
            break;

        // Execute the CopyFrom (from the this thread).  The ecwThread is waiting for us.
        pio_rReadInfo.m_pDestinationBitmap->CopyFrom(*pio_rReadInfo.m_pSourceRaster, pio_rReadInfo.m_CopyFromOptions);
             
        // Notify caller and cancel if requested.
        if(!HUTExportProgressIndicator::GetInstance()->ContinueIteration(pio_rReadInfo.m_pDestinationFile, 0))
            ECWExporter.CancelExport();    // flag ecw to cancel. we don't know when it will happen so we keep going until ExportCompleted().

        // give control back to ecw thread
        pio_rReadInfo.m_notifyEcwThread.notify_all();
        }

    ecwThread.join();   // Wait until the thread exit.

    if ((ECWExporter.GetLastError() != NCS_SUCCESS) &&
        (ECWExporter.GetLastError() != NCS_USER_CANCELLED_COMPRESSION))
        {
        if (ECWExporter.GetLastError() == NCS_INPUT_SIZE_TOO_SMALL)
            {
            throw HRFTooSmallForEcwCompressionException(pio_rReadInfo.m_pDestinationFile->GetURL()->GetURL());
            }
        else if (ECWExporter.GetLastError() == NCS_INPUT_SIZE_EXCEEDED)
            {
            throw HRFTooBigForEcwCompressionException(pio_rReadInfo.m_pDestinationFile->GetURL()->GetURL());
            }
        else
            {
            throw HFCFileNotCreatedException(pio_rReadInfo.m_pDestinationFile->GetURL()->GetURL());
            }
        }

    //Sub-resolution computation is done by the ErMapper library above.
    if (pio_rReadInfo.m_pDestinationRaster->GetClassID() == HRAPyramidRaster::CLASS_ID)
        {
        ((HFCPtr<HRAPyramidRaster>&)pio_rReadInfo.m_pDestinationRaster)->EnableSubImageComputing(false);
        }
    }

#endif // __ECW_EXPORT_ENABLE__

#endif // IPP_HAVE_ERMAPPER_SUPPORT 
