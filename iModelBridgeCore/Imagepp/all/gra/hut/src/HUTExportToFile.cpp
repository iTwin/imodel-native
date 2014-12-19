//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hut/src/HUTExportToFile.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HUTExportFile.
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HIMStoredRasterEquivalentTransfo.h>
#include <Imagepp/all/h/HRADrawProgressIndicator.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRAPyramidRaster.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRAUpdateSubResProgressIndicator.h>
#include <Imagepp/all/h/HRFCalsFile.h>
#include <Imagepp/all/h/HRFHMRFile.h>
#include <Imagepp/all/h/HRFImportExport.h>
#include <Imagepp/all/h/HRFIntergraphFile.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFLRDFile.h>
#include <Imagepp/all/h/HRFPageFileFactory.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFRasterFileBlockAdapter.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HRFRasterFilePageDecorator.h>
#include <Imagepp/all/h/HRFRasterFileResBooster.h>
#include <Imagepp/all/h/HRFSloStripAdapter.h>
#include <Imagepp/all/h/HRPConvFiltersV24R8G8B8.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HUTExportProgressIndicator.h>
#include <Imagepp/all/h/HUTExportToFile.h>
#include <Imagepp/all/h/HVE2DUniverse.h>
#include <Imagepp/all/h/HRAClearOptions.h>
#include <Imagepp/all/h/HRACopyFromOptions.h>
#include <Imagepp/all/h/HVE2DRectangle.h>

#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HIMStripAdapter.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecErMapperSupported.h>

#ifdef __ECW_EXPORT_ENABLE__
//#error
// Includes from the ERMapper SDK
#include <ErMapperEcw/NCSEcwCompressClient.h>
#include <ErMapperEcw/NCSTypes.h>
#include <ErMapperEcw/NCSGDTLocation.h>
#include <ErMapperEcw/NCSMalloc.h>

#include <Imagepp/all/h/HFCThread.h>

// this class is use to initialize the ErMapper library only once.
// The library will be initialized on the first Init() call and will be
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
// Structure to store client read data
struct ECWReadInfoStruct
    {
    ECWReadInfoStruct()
        : m_ComputeStripEvent(false, false),
          m_StripComputedEvent(false, false),
          m_CancelExport(false)
        {
        };

    ~ECWReadInfoStruct()
        {
        };

    HFCPtr<HRAStoredRaster>         m_pSourceRaster;
    HFCPtr<HRAStoredRaster>         m_pDestinationRaster;
    HFCPtr<HRABitmap>               m_pDestinationBitmap;

    HFCPtr<HRFRasterFile>           m_pSourceFile;
    HFCPtr<HRFRasterFile>           m_pDestinationFile;

    HFCPtr<HGF2DTransfoModel>       m_pLineTranslateModel;
    HRACopyFromOptions              m_CopyFromOptions;

    UINT32                          m_LineWidthInByte;

    // Buffer to store an error message in (1024 is an arbitrary size)
    char                            m_pErrorBuffer[1024];

    UINT32                          m_StripHeight;
    UINT32                          m_CurrentStripPos;

    // PDF export
    HFCEvent                        m_ComputeStripEvent;
    HFCEvent                        m_StripComputedEvent;

    bool                           m_CancelExport;
    };


//----------------------------------------------------------------------------
// Callback Functions for ECW special export.
//
static BOOLEAN ReadCallback  (NCSEcwCompressClient* pClient, UINT32 nNextLine, IEEE4** ppOutputBandBufferArray);
static BOOLEAN CancelCallback(NCSEcwCompressClient* pClient);

class PDFExporterThread : public HFCThread
    {
public:
    PDFExporterThread(ECWReadInfoStruct& pi_rCompressReadInfo)
        : HFCThread(),
          m_rCompressReadInfo(pi_rCompressReadInfo),
          m_ExportCompleted(false),
          m_LastError(NCS_SUCCESS)
        {
        };

    ~PDFExporterThread()
        {
        WaitUntilSignaled();    // wait until the thread is terminated
        };

    void Go()
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
            HDEBUGCODE(else HASSERT(m_LastError == NCS_INPUT_SIZE_TOO_SMALL););

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
        };

    NCSError GetLastError()
        {
        return m_LastError;
        }

    // Build a strip
    // This method is called by the ReadCallback function. All information need to copy the strip was
    // calculated by ReadCallback().
    void ComputeStrip()
        {
        m_rCompressReadInfo.m_pDestinationBitmap->CopyFrom((HFCPtr<HRARaster>&)m_rCompressReadInfo.m_pSourceRaster, m_rCompressReadInfo.m_CopyFromOptions);
        m_rCompressReadInfo.m_StripComputedEvent.Signal();
        };

    // Initialize the callback structure for ErMapper API.
    bool InitializeECWCallbackStruct()
        {
        bool CreationStatus = false;

        // Dynamic Load the ECW library
        if (m_pCompressClient = NCSEcwCompressAllocClient())
            {
            WString  FileName(((HFCPtr<HFCURLFile>&)m_rCompressReadInfo.m_pDestinationFile->GetURL())->GetHost());

            FileName += L"\\";
            FileName += ((HFCPtr<HFCURLFile>&)m_rCompressReadInfo.m_pDestinationFile->GetURL())->GetPath();

            // Copy in string (unicode) to buffer (ansi)
            BeStringUtilities::WCharToCurrentLocaleChar(m_pCompressClient->szOutputFilename,FileName.c_str(),MAX_PATH);
            HFCPtr<HRFPageDescriptor>       pDstPageDesc = m_rCompressReadInfo.m_pDestinationFile->GetPageDescriptor(0);
            HFCPtr<HRFResolutionDescriptor> pDstResDesc  = pDstPageDesc->GetResolutionDescriptor(0);

            // Set up the input dimensions
            HASSERT(pDstResDesc->GetWidth() <= ULONG_MAX);
            HASSERT(pDstResDesc->GetHeight() <= ULONG_MAX);

            m_pCompressClient->nInOutSizeX = (uint32_t)pDstResDesc->GetWidth();
            m_pCompressClient->nInOutSizeY = (uint32_t)pDstResDesc->GetHeight();

            // Specify the callbacks and client data ptr
            m_pCompressClient->pReadCallback   = ReadCallback;
            m_pCompressClient->pStatusCallback = 0;
            m_pCompressClient->pCancelCallback = CancelCallback;

            // Set the transformation model
            HFCPtr<HGF2DTransfoModel> pDestModel(((HFCPtr<HRAStoredRaster> &)m_rCompressReadInfo.
                                                  m_pDestinationRaster)->
                                                 GetTransfoModel());

            // Translate the model units to geocoding units
            IRasterBaseGcsPtr baseGCS = pDstPageDesc->GetGeocoding();

            //If some geocoding information is available, get the transformation in the units specified
            //by the geocoding information
            if (baseGCS != 0 && baseGCS->IsValid())
                {
                pDestModel = HCPGeoKeys::TranslateFromMeter(baseGCS,
                                                            pDestModel,
                                                            true,
                                                            false,
                                                            0,
                                                            baseGCS);
                }

            if (pDestModel->IsStretchable())
                {
                double           ScaleFactorX;
                double           ScaleFactorY;
                HGF2DDisplacement Translation;

                pDestModel->GetStretchParams(&ScaleFactorX, &ScaleFactorY, &Translation);

                // Specifying the transformation model.
                m_pCompressClient->fCellIncrementX = ScaleFactorX;
                m_pCompressClient->fCellIncrementY = ScaleFactorY;

                m_pCompressClient->fOriginX        = Translation.GetDeltaX();
                m_pCompressClient->fOriginY        = Translation.GetDeltaY();

                // TR 188117 (ECW lib problem)
                char* pszProjection      = 0;
                char* pszDatum           = 0;
                bool IsRefCoordSysFound = false;

                if ((baseGCS != 0) && (baseGCS->IsValid() && (baseGCS->IsProjected()) && (baseGCS->GetEPSGCode() != 0))
                    {
                    uint32_t EPSGCode = baseGCS->GetEPSGCode();

                    // szProjection : ER Mapper style Projection name string, e.g. "RAW" or "GEODETIC".  Never NULL
                    // szDatum : Mapper style Datum name string, e.g. "RAW" or "NAD27".  Never NULL
                    if (NCS_SUCCESS == NCSGetProjectionAndDatumEx(EPSGCode, &pszProjection, &pszDatum, true))
                        {
                        // TR 265837 - Sometimes the return status is SUCCESS but the pszProjection is NULL (In this case the datum is correctly set)
                        // We will consider this identical to a not found status. A typical test case is EPSG:21892 a deprectated Bogot CRS replaced
                        // by EPSG:21897 ... It is not up to us nor ERMapper to determine what replacement should be set.
                        if (pszProjection != NULL)
                            IsRefCoordSysFound = true;
                        }
                    }
                else if ((baseGCS != 0) && (baseGCS->IsValid() && (!baseGCS->IsProjected()) && (baseGCS->GetEPSGCode() != 0))
                    {
                    uint32_t EPSGCode = baseGCS->GetEPSGCode();

                    // szProjection : ER Mapper style Projection name string, e.g. "RAW" or "GEODETIC".  Never NULL
                    // szDatum : Mapper style Datum name string, e.g. "RAW" or "NAD27".  Never NULL
                    if (NCS_SUCCESS == NCSGetProjectionAndDatumEx(EPSGCode, &pszProjection, &pszDatum, true))
                        {
                        IsRefCoordSysFound = true;
                        }
                    }

                if (IsRefCoordSysFound == true)
                    {
                    strcpy(m_pCompressClient->szProjection, pszProjection);
                    strcpy(m_pCompressClient->szDatum     , pszDatum);
                    NCSFree (pszProjection);
                    NCSFree (pszDatum);
                    }

                // Specify the units in which world cell sizes are specified, e.g. meters, feet
                m_pCompressClient->eCellSizeUnits = ECW_CELL_UNITS_METERS;
                }
            else
                {
                // Specifying the transformation model.
                m_pCompressClient->fCellIncrementX = 1.0;
                m_pCompressClient->fCellIncrementY = 1.0;

                m_pCompressClient->fOriginX        = 0.0;
                m_pCompressClient->fOriginY        = 0.0;

                // Specify the units in which world cell sizes are specified, e.g. meters, feet
                m_pCompressClient->eCellSizeUnits = ECW_CELL_UNITS_METERS;
                }

            //Set the compression ratio
            HFCPtr<HRFRasterFile> pOriginalDestRaster(m_rCompressReadInfo.m_pDestinationFile);

            //Not the original file
            if (pOriginalDestRaster->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
                pOriginalDestRaster = ((HFCPtr<HRFRasterFileExtender> &)m_rCompressReadInfo.m_pDestinationFile)->GetOriginalFile();

            HFCPtr<HRFResolutionDescriptor> pOriRasterResDesc = pOriginalDestRaster->GetPageDescriptor(0)->GetResolutionDescriptor(0);

            const HFCPtr<HCDCodec>& pCodec = pOriRasterResDesc->GetCodec();
            HASSERT(pCodec->IsCompatibleWith(HCDCodecErMapperSupported::CLASS_ID));
            m_pCompressClient->fTargetCompression = (IEEE4)((HFCPtr<HCDCodecErMapperSupported>&)pCodec)->GetCompressionRatio();

            // Set up client data for read callback
            m_rCompressReadInfo.m_pDestinationBitmap  = new HRABitmap(m_pCompressClient->nInOutSizeX,
                                                                      pDstResDesc->GetBlockHeight(),
                                                                      m_rCompressReadInfo.m_pDestinationRaster->GetTransfoModel(),
                                                                      m_rCompressReadInfo.m_pDestinationRaster->GetCoordSys(),
                                                                      pDstResDesc->GetPixelType());

            m_rCompressReadInfo.m_StripHeight = pDstResDesc->GetBlockHeight();

            if (m_rCompressReadInfo.m_pDestinationBitmap->GetPacket()->GetBufferAddress() == 0)
                {
                m_rCompressReadInfo.m_pDestinationBitmap = new HRABitmap(m_pCompressClient->nInOutSizeX,
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
                m_rCompressReadInfo.m_LineWidthInByte       = m_pCompressClient->nInOutSizeX * pDstResDesc->GetPixelType()->CountValueBits() / 8;

                m_rCompressReadInfo.m_pErrorBuffer[0]       = 0;

                // Set up the default pixel type according the src pixel type.
                if (pDstResDesc->GetPixelType()->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID))
                    {
                    m_pCompressClient->nInputBands     = 1;
                    m_pCompressClient->eCompressFormat = COMPRESS_UINT8;
                    }
                else
                    {
                    m_pCompressClient->nInputBands     = 3;
                    m_pCompressClient->eCompressFormat = COMPRESS_RGB;
                    }

                // Create a Translation transformation model to move the m_pDestinationBitmap
                // over the SourceRaster physical line by physical line into our ReadCallback.
                m_rCompressReadInfo.m_pLineTranslateModel = new HGF2DTranslation(
                    HGF2DDisplacement(0, m_rCompressReadInfo.m_StripHeight));
                m_pCompressClient->pClientData = (void*)&m_rCompressReadInfo;
                CreationStatus = true;
                }
            }
        return CreationStatus;
        };

    bool ExportCompleted() const
        {
        return m_ExportCompleted;
        }

    void CancelExport()
        {
        m_rCompressReadInfo.m_CancelExport = true;
        }

private:

    bool                   m_ExportCompleted;
    ECWReadInfoStruct&      m_rCompressReadInfo;
    NCSEcwCompressClient*   m_pCompressClient;
    NCSError                m_LastError;

    };

#endif // __ECW_EXPORT_ENABLE__

USING_NAMESPACE_IMAGEPP

//-----------------------------------------------------------------------------
// Public
// Constructor used to export the given source file into the specified
// destination one.
// IMPORTANT:
//      The source and destination raster files must be tiled.
//-----------------------------------------------------------------------------
HUTExportToFile::HUTExportToFile(const HFCPtr<HFCURL>&            pi_rpSourceURL,
                                 HFCPtr<HRFRasterFile>&           pi_rpDestinationFile,
                                 const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster,
                                 double                          pi_ScaleX,
                                 double                          pi_ScaleY,
                                 bool                            pi_Resample,
                                 const HFCPtr<HRPFilter>&         pi_rpResamplingFilter)
    : m_pDestinationFile(pi_rpDestinationFile), m_pClusterWorld(pi_rpWorldCluster),
      m_SourcePool(1024, HPMPool::KeepLastBlock),
      m_Resampling(HGSResampling::AVERAGE)
    {
    HPRECONDITION(m_pDestinationFile != 0);
    HPRECONDITION(pi_ScaleX != 0.0 && pi_ScaleY != 0.0);

    // Initialise the members (don't initialise m_SrcBlockAccess since
    // OpenSourceFile do it).
    m_AlreadyExported   = false;
    m_HasClipShape      = false;
    m_SrcNeedCache      = false;
    m_NbColorsIfIndexed = 0;    // MaxEntries Default value.
    m_UseDestinationPaletteIfIndexed = false;
    m_Resample          = pi_Resample;
    m_pResamplingFilter = pi_rpResamplingFilter;

    // TR #201780
    // if the destination has an Alpha channel, the alpha channel must be not blended
    HFCPtr<HRPPixelType> pDstPixelType = m_pDestinationFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType();
    m_BlendAlpha = pDstPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) == HRPChannelType::FREE;

    m_HasRepPalSamplingOptions = false;

    m_ScaleFactorX      = pi_ScaleX;
    m_ScaleFactorY      = pi_ScaleY;

    m_EstimateExportSize = false;

    // Open the source file.
    OpenSourceFile(pi_rpSourceURL);




    HASSERT(m_pSourceFile != 0);
    }

//-----------------------------------------------------------------------------
// Public
// Constructor used to export the given source file into the specified
// destination one.
// IMPORTANT:
//      The source and destination raster files must be tiled.
//-----------------------------------------------------------------------------
HUTExportToFile::HUTExportToFile(const HFCPtr<HRFRasterFile>&     pi_rpRasterFile,
                                 HFCPtr<HRFRasterFile>&           pi_rpDestinationFile,
                                 const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster,
                                 double                          pi_ScaleX,
                                 double                          pi_ScaleY,
                                 bool                            pi_Resample,
                                 const HFCPtr<HRPFilter>&         pi_rpResamplingFilter)
    : m_pDestinationFile(pi_rpDestinationFile), m_pClusterWorld(pi_rpWorldCluster),
      m_SourcePool(1024, HPMPool::KeepLastBlock),
      m_Resampling(HGSResampling::AVERAGE)
    {
    HPRECONDITION(m_pDestinationFile != 0);
    HPRECONDITION(pi_ScaleX != 0.0 && pi_ScaleY != 0.0);

    // Initialise the members (don't initialise m_SrcBlockAccess since
    // OpenSourceFile do it).
    m_AlreadyExported   = false;
    m_HasClipShape      = false;
    m_SrcNeedCache      = false;
    m_NbColorsIfIndexed = 0;    // MaxEntries Default value.
    m_UseDestinationPaletteIfIndexed = false;
    m_Resample          = pi_Resample;
    m_pResamplingFilter = pi_rpResamplingFilter;
    // TR #201780
    // if the destination has an Alpha channel, the alpha channel must be not blended
    HFCPtr<HRPPixelType> pDstPixelType = m_pDestinationFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType();
    m_BlendAlpha = pDstPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) == HRPChannelType::FREE;
    m_HasRepPalSamplingOptions = false;

    m_ScaleFactorX      = pi_ScaleX;
    m_ScaleFactorY      = pi_ScaleY;

    // Open the source file.
    m_pSourceFile = pi_rpRasterFile;
    InitSourceFile();

    HASSERT(m_pSourceFile != 0);
    }

//-----------------------------------------------------------------------------
// Public
// Constructor used to export the given stored raster into the specfied
// destination file.
// IMPORTANT:
//      The destination raster file must be tiled.
//-----------------------------------------------------------------------------
HUTExportToFile::HUTExportToFile(HFCPtr<HRARaster>&               pi_rpSourceRaster,
                                 HFCPtr<HRFRasterFile>&           pi_rpDestinationFile,
                                 const HFCPtr<HGF2DWorldCluster>& pi_rpWorldCluster,
                                 double                          pi_ScaleX,
                                 double                          pi_ScaleY,
                                 bool                            pi_Resample,
                                 const HFCPtr<HRPFilter>&         pi_rpResamplingFilter)
    : m_pSourceRaster(pi_rpSourceRaster), m_pDestinationFile(pi_rpDestinationFile),
      m_pClusterWorld(pi_rpWorldCluster),
      m_SourcePool(1024, HPMPool::KeepLastBlock),
      m_Resampling(HGSResampling::AVERAGE)
    {
    HPRECONDITION(m_pSourceRaster != 0);
    HPRECONDITION(m_pDestinationFile != 0);
    HPRECONDITION(pi_ScaleX != 0.0 && pi_ScaleY != 0.0);

    // If pi_Resample is false, we must obtain the "equivalent to StoredRaster" transformation
    HDEBUGCODE(HIMStoredRasterEquivalentTransfo SRETransfo(m_pSourceRaster));
    HPRECONDITION(pi_Resample || SRETransfo.EquivalentTransfoCanBeComputed());

    // Initialise the members.
    m_AlreadyExported   = false;
    m_HasClipShape      = false;
    m_SrcNeedCache      = false;
    m_NbColorsIfIndexed = 0;    // MaxEntries Default value.
    m_UseDestinationPaletteIfIndexed = false;
    m_Resample          = pi_Resample;
    m_pResamplingFilter = pi_rpResamplingFilter;
    m_SrcBlockAccess    = HRFBlockAccess::RANDOM;
    // TR #201780
    // if the destination has an Alpha channel, the alpha channel must be not blended
    HFCPtr<HRPPixelType> pDstPixelType = m_pDestinationFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType();
    m_BlendAlpha = pDstPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) == HRPChannelType::FREE;
    m_HasRepPalSamplingOptions = false;

    m_ScaleFactorX      = pi_ScaleX;
    m_ScaleFactorY      = pi_ScaleY;

    }

//-----------------------------------------------------------------------------
// Public
// Destructor.
//-----------------------------------------------------------------------------
HUTExportToFile::~HUTExportToFile()
    {
    CleanUp();
    }

//-----------------------------------------------------------------------------
// Private
// This method is used to clean up all reference of the current object in the
// right order.
//
// IMPORTANT:
//     This method can't throw any error since it can be call from the
//     class destructor.
//-----------------------------------------------------------------------------
void HUTExportToFile::CleanUp()
    {
    // IMPORTANT:
    // We need to flush the reference on the source and destination raster if
    // we want to delete the object log (in the case of a local one) without
    // any danger.
    m_pDestinationRaster = 0;
    m_pSourceRaster      = 0;

    // Flush the source and destination raster files.
    m_pSourceFile      = 0;
    m_pDestinationFile = 0;

    //Ensure that the update sub resolution and draw are re-enabled.
    HRAUpdateSubResProgressIndicator::GetInstance()->Restart(0);
    HRADrawProgressIndicator::GetInstance()->Restart(0);
    }

//-----------------------------------------------------------------------------
// Public
// This method is used to set a clip shape in the export process.  The clip
// shape must be expressed in Logical of Physical CoordSys of the source.
// (The clip shape will represent the destination physical shape).
//-----------------------------------------------------------------------------
void HUTExportToFile::SetClipShape(const HVEShape& pi_rClipShape)
    {
    // The provided shape may not be empty
    HPRECONDITION(!pi_rClipShape.IsEmpty());

    // Get the shape and set the logical CoordSys to the CoordSys of the shape.
    m_HasClipShape     = true;
    m_ClipShape        = pi_rClipShape;
    }


//-----------------------------------------------------------------------------
// Public
// This method is used to reduce the number of colors in the palette, for exemple
// compress to 32 colors in a palette of 256 colors.
//-----------------------------------------------------------------------------
void HUTExportToFile::SetNumberOfColorDestination(uint32_t pi_NbColors)
    {
    m_NbColorsIfIndexed = pi_NbColors;
    }

//-----------------------------------------------------------------------------
// Public
// This method is used to reduce the number of colors in the palette, for exemple
// compress to 32 colors in a palette of 256 colors.
//-----------------------------------------------------------------------------
void HUTExportToFile::SetUseDestinationPaletteIfIndexed(bool pi_UseDestinationPalette)
    {
    m_UseDestinationPaletteIfIndexed = pi_UseDestinationPalette;
    }

//-----------------------------------------------------------------------------
// Public
// This method is used to set how to copy the alpha channel.
//-----------------------------------------------------------------------------
void HUTExportToFile::SetBlendAlpha(bool pi_BlendAlpha)
    {
    m_BlendAlpha = pi_BlendAlpha;
    }

//-----------------------------------------------------------------------------
// Public
// This method enable/disable export final size estimation.
//-----------------------------------------------------------------------------
void HUTExportToFile::SetExportSizeEstimation(bool pi_IsEnable)
    {
    m_EstimateExportSize = pi_IsEnable;
    }

//-----------------------------------------------------------------------------
// public
// SetResamplingMode
//-----------------------------------------------------------------------------
void HUTExportToFile::SetResamplingMode(const HGSResampling& pi_rResampling)
    {
    m_Resampling = pi_rResampling;
    }

//-----------------------------------------------------------------------------
// public
// GetRepresentativePaletteSamplingOptions
//-----------------------------------------------------------------------------
HRASamplingOptions HUTExportToFile::GetRepresentativePaletteSamplingOptions() const
    {
    return m_RepPalSamplingOptions;
    }

//-----------------------------------------------------------------------------
// public
// SetRepresentativePaletteSamplingOptions
//-----------------------------------------------------------------------------
void HUTExportToFile::SetRepresentativePaletteSamplingOptions(const HRASamplingOptions& pi_rRepPalSamplingOptions)
    {
    m_HasRepPalSamplingOptions = true;
    m_RepPalSamplingOptions = pi_rRepPalSamplingOptions;
    }

//-----------------------------------------------------------------------------
// Private
// This method is used to prepare all members that must be initialized for
// the Export() method to work (all members except m_pDestinationRaster).
//-----------------------------------------------------------------------------
void HUTExportToFile::PrepareMembers()
    {
    // Check if we have a source raster file, if not we need put the source
    // file into a stored raster so that we can pull a HRARaster afterwards.
    if (m_pSourceRaster == 0)
        {
        HFCPtr<HGF2DCoordSys> pSrcCoordSys = m_pClusterWorld->GetCoordSysReference(m_pSourceFile->GetWorldIdentificator());

        // For now, we don't use the specified object log. We simply create
        // a small one. It will always keep at least one source block in
        // memory, and we don't need more than that...
        m_pSourceStore = new HRSObjectStore(&m_SourcePool, m_pSourceFile, 0, pSrcCoordSys);

        // Get the raster from the store.
        m_pSourceRaster = m_pSourceStore->LoadRaster();

        // Change the coord sys of the clip shape into the new logical coord sys.
        if (m_HasClipShape)
            m_ClipShape.ChangeCoordSys(m_pSourceStore->GetPhysicalCoordSys());
        }

    // Setup the clip shape and the logical shape.
    if (m_HasClipShape)
        {
        // We have a clip shape, we need to set the logical shape to the clip shape.
        m_LogicalShape = m_ClipShape;
        }
    else if (m_pDestinationFile->GetCapabilities()->GetCapabilityOfType(HRFClipShapeCapability::CLASS_ID, HFC_CREATE_ONLY) == 0)
        {
        // Here we don't have any clip shape and the destination don't support shape.
        // We need to initialise the clip shape and the logical shape to the source effective shape.
        HVEShape ClipShape(*(m_pSourceRaster->GetEffectiveShape()));
        ClipShape.ChangeCoordSys(m_pSourceRaster->GetCoordSys());
        SetClipShape(ClipShape);
        m_LogicalShape = m_ClipShape;
        }
    else
        {
        HASSERT(m_pSourceRaster->IsStoredRaster());

        // We don't have any shape and the destination file support shape, so we
        // initialise the clip shape to physical shape of the source and the logical
        // shape to the logical shape of the source.

        // Get the logical shape of the source.
        m_LogicalShape = m_pSourceRaster->GetShape();

        // Get the physical shape of the source
        HVEShape Shape(((HFCPtr<HRAStoredRaster>&)m_pSourceRaster)->GetPhysicalExtent());

        // Change the coordsys of the shape to the source coordsys (logical).
        Shape.ChangeCoordSys(m_pSourceRaster->GetCoordSys());
        SetClipShape(Shape);
        }
    }


//#pragma optimize("g", off)


//-----------------------------------------------------------------------------
// Public
// This method export the given raster (or raster file) into the specified
// destination file.
//
// IMPORTANT:
//    This method can only be call once, it's illegal to call it more than one
//    time, since at the end of the export we flush all reference that we have
//    on the source and destination raster (and file).
//-----------------------------------------------------------------------------
void HUTExportToFile::Export()
    {
    // Check that the first time that do the export.
    HPRECONDITION(m_AlreadyExported == false);

    HGF2DDisplacement               Displacement;
    bool                           DstSupportsTransfo;
    HFCPtr<HRFPageDescriptor>       pDstPageDesc;
    HFCPtr<HRFResolutionDescriptor> pDstResDesc;
    HFCPtr<HRPHistogram>            pHistogram;
    HFCPtr<HGF2DTransfoModel>       pPhysicalToLogicalCS;

    HFCPtr<HRFRasterFile>           pOriginalDestRaster = m_pDestinationFile;
    HAutoPtr<HPMPool>                pDstObjectLog;

    try
        {
        // Initialize the members.
        PrepareMembers();

        HPRECONDITION(m_HasClipShape);
        HPRECONDITION(m_pDestinationFile != 0);
        HPRECONDITION(m_pSourceRaster != 0);

        // Get the first page descriptor of the destination file.
        pDstPageDesc = m_pDestinationFile->GetPageDescriptor(0);

        // Get the first resolution descriptor of the destination page descriptor.
        pDstResDesc = pDstPageDesc->GetResolutionDescriptor(0);

        // Get the logical transformation model from the physical coordsys.
        pPhysicalToLogicalCS = ComputeTransformation(pDstPageDesc, pDstResDesc);

#ifdef __ECW_EXPORT_ENABLE__
        if (m_pDestinationFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
            pOriginalDestRaster = ((HFCPtr<HRFRasterFileExtender> &)m_pDestinationFile)->GetOriginalFile();

        // Is the destination is the COM ECW(ERMapper) (1429) OR Is the destination is COM Jpeg2000File (1477)
        // Simplify the transformation model to meet their restricted capabillities.
        if (!m_Resample &&
            ((pOriginalDestRaster->GetClassID() == HRFEcwFile::CLASS_ID) || (pOriginalDestRaster->GetClassID() == HRFJpeg2000File::CLASS_ID)))
            {
            double           ScaleFactorX;
            double           ScaleFactorY;
            HGF2DDisplacement Translation;

            pPhysicalToLogicalCS->GetStretchParamsAt(&ScaleFactorX,
                                                     &ScaleFactorY,
                                                     &Translation,
                                                     pDstResDesc->GetWidth() / 2.0,
                                                     pDstResDesc->GetHeight() / 2.0);

            // ECW support the raster origin, the size of dataset cells in world units (scalling),
            // and the type of linear units used, (rotation and shear are unsupported)
            if ((pOriginalDestRaster->GetClassID() == HRFEcwFile::CLASS_ID) || (pOriginalDestRaster->GetClassID() == HRFJpeg2000File::CLASS_ID)) // ECW or J2K
                {
                pPhysicalToLogicalCS = new HGF2DStretch(Translation, ScaleFactorX, ScaleFactorY);
                }
            }
#endif // __ECW_EXPORT_ENABLE__

        // If we don't resample we need to check that the destination file
        // support the transformation.
        if (!m_Resample && pPhysicalToLogicalCS != 0 &&  pPhysicalToLogicalCS->GetClassID() != HGF2DIdentity::CLASS_ID)
            {
            HFCPtr<HRFCapability> pTransfoCapability = new HRFTransfoModelCapability(HFC_CREATE_ONLY,
                                                                                     pPhysicalToLogicalCS->GetClassID());

            HASSERT(m_pDestinationFile->GetCapabilities()->Supports(pTransfoCapability));
            }

        // Build the TransfoModel capability
        HFCPtr<HRFCapability> pTransfoCapability = new HRFTransfoModelCapability(HFC_CREATE_ONLY,
                                                                                 pPhysicalToLogicalCS->GetClassID());

        // Check if the destination file support this kind of TransfoModel
        DstSupportsTransfo = m_pDestinationFile->GetCapabilities()->Supports(pTransfoCapability);
        if (DstSupportsTransfo)
            pDstPageDesc->SetTransfoModel(*pPhysicalToLogicalCS);

        // If the source file have a sequential access and we don't have an identiy
        // model between the source and destination, we need to create a cache over
        // the source file.
        if (m_SrcBlockAccess == HRFBlockAccess::SEQUENTIAL && m_pSourceFile != 0)
            {
            HFCPtr<HGF2DTransfoModel> pDstPhysToLogModel;
            HFCPtr<HGF2DCoordSys>     pDstWorld;
            HFCPtr<HGF2DTransfoModel> pPhysicalSrcToPhysicalDst;
            HFCPtr<HGF2DTransfoModel> pSrcPhysToLogModel;
            HFCPtr<HGF2DTransfoModel> pSrcToDstWorld;
            HFCPtr<HGF2DCoordSys>     pSrcWorld;

            // Get the world for the source and destination files.
            pSrcWorld      = m_pClusterWorld->GetCoordSysReference(m_pSourceFile->GetWorldIdentificator());
            pDstWorld      = m_pClusterWorld->GetCoordSysReference(m_pDestinationFile->GetWorldIdentificator());
            pSrcToDstWorld = pSrcWorld->GetTransfoModelTo(pDstWorld);

            // Get the transformation model of the source file.
            if (m_pSourceFile->GetPageDescriptor(0)->HasTransfoModel())
                pSrcPhysToLogModel = m_pSourceFile->GetPageDescriptor(0)->GetTransfoModel();
            else
                pSrcPhysToLogModel = new HGF2DIdentity();

            // Get the transformation model of the destination file.
            if (pDstPageDesc->HasTransfoModel())
                pDstPhysToLogModel = pDstPageDesc->GetTransfoModel();
            else
                pDstPhysToLogModel = new HGF2DIdentity();

            // Compose the two models.
            pPhysicalSrcToPhysicalDst = pSrcPhysToLogModel->ComposeInverseWithDirectOf(*pSrcToDstWorld);
            pPhysicalSrcToPhysicalDst = pPhysicalSrcToPhysicalDst->ComposeInverseWithInverseOf(*pDstPhysToLogModel);

            if (pPhysicalSrcToPhysicalDst->IsIdentity() == false)
                {
                // Indicate that we need a cache on the source file.
                m_SrcNeedCache = true;

                // Before putting a cache over the source file we need to close it since
                // we have alreay pumped a raster from it.
                HFCPtr<HFCURL> pSourceURL(m_pSourceFile->GetURL());
                CloseSourceFile();
                OpenSourceFile(pSourceURL);
                PrepareMembers();
                }
            }

        HFCPtr<HRPPixelType> pDstPixelType;

        // get pixeltype from rasterfile
        pDstPixelType = pDstResDesc->GetPixelType();

        if (pDstPixelType->CountIndexBits() && !m_UseDestinationPaletteIfIndexed)
            {
            if (!pDstPixelType->GetPalette().IsReadOnly())
                {
                // test if we have the same kind of palette, if yes, simply copy the palette
                if((m_NbColorsIfIndexed == 0) &&
                   (((HFCPtr<HRAStoredRaster>&)m_pSourceRaster)->GetPixelType()->GetClassID() ==
                    pDstPixelType->GetClassID()))
                    {
                    // Copy the palette
                    HRPPixelPalette& rPalette = pDstPixelType->LockPalette();
                    const HRPPixelPalette  SrcPalette = ((HFCPtr<HRAStoredRaster>&)m_pSourceRaster)->GetPixelType()->GetPalette();

                    HASSERT(rPalette.GetChannelOrg() == SrcPalette.GetChannelOrg());
                    rPalette = SrcPalette;

                    pDstPixelType->UnlockPalette();
                    }
                else
                    {
                    // otherwise, get the representative palette
                    HRASamplingOptions RepPalOptions;

                    if(m_HasRepPalSamplingOptions)
                        RepPalOptions = m_RepPalSamplingOptions;

                    HRARepPalParms RepPalParms(pDstPixelType, RepPalOptions, false, m_NbColorsIfIndexed);

                    if (m_pSourceRaster->IsStoredRaster())
                        m_pSourceRaster->GetRepresentativePalette(&RepPalParms);
                    else
                        {
                        // Create a pixeltype with the same channels as the source,
                        // but without index bits.
                        HFCPtr<HRPPixelType> pStripPixelType = HRPPixelTypeFactory::GetInstance()->Create(m_pSourceRaster->GetPixelType()->GetChannelOrg(), 0);

                        if (pStripPixelType == 0)
                            pStripPixelType = new HRPPixelTypeV32R8G8B8A8();

                        HFCPtr<HRARaster> pAdapter(new HIMStripAdapter(m_pSourceRaster, pStripPixelType));
                        ((HFCPtr<HIMStripAdapter>&)pAdapter)->ClipStripsBasedOnSource(true);
                        if (RepPalOptions.GetPyramidImageSize() != 0)
                            ((HFCPtr<HIMStripAdapter>&)pAdapter)->SetQualityFactor(RepPalOptions.GetPyramidImageSize() / 100.0);

                        pAdapter->GetRepresentativePalette(&RepPalParms);
                        }

                    // If the destination file have sequential access we reopen it since
                    // GetRepresentativePalette() have already read it.
                    if (m_pSourceFile != 0 && m_SrcBlockAccess == HRFBlockAccess::SEQUENTIAL)
                        {
                        HFCPtr<HFCURL> pSourceURL(m_pSourceFile->GetURL());
                        CloseSourceFile();
                        OpenSourceFile(pSourceURL);
                        PrepareMembers();
                        }
                    }
                // copy the palette to the HRF
                HRPPixelPalette Palette = pDstPixelType->GetPalette();
                HAutoPtr<HRFResolutionEditor> pDstResolutionEditor;
                for (unsigned short i = 0; i < pDstPageDesc->CountResolutions(); i++)
                    {
                    pDstResolutionEditor = m_pDestinationFile->CreateResolutionEditor(0, i, HFC_CREATE_ONLY);
                    pDstResolutionEditor->SetPalette(Palette);
                    }
                }
            }

        // Create a Destination POOL
        //
        // Calculate dimension of one block
        uint32_t PoolSizeKB = 0;
        bool  Is1BitRLE=false;
        for (uint32_t ResNumber=0; ResNumber < m_pDestinationFile->GetPageDescriptor(0)->CountResolutions(); ResNumber++)
            {
            uint32_t ResSize  = m_pDestinationFile->GetPageDescriptor(0)->GetResolutionDescriptor((unsigned short)ResNumber)->GetBlockSizeInBytes();
            if (m_pDestinationFile->GetPageDescriptor(0)->GetResolutionDescriptor((unsigned short)ResNumber)->GetPixelType()->CountPixelRawDataBits() == 1)
                {
                ResSize  *= 16; // Compression RLE1 bad case
                Is1BitRLE = true;
                }
            else
                ResSize  = (uint32_t)(ResSize * 1.25); // For security only

            PoolSizeKB += ResSize;
            }
        // Convert in KB
        PoolSizeKB /= 1024;

        // Memory Mgr setting
        //  We disable it completely for the 1bit image, because the packet size is never the same.(rle compression)
        //  We disable it partially for destRaster not stripped
        HPMPool::MemoryMgrType SelectedMemMgr = HPMPool::KeepLastBlock;
        if (Is1BitRLE)
            SelectedMemMgr = HPMPool::None;         // Disable Mgr for 1 bit output
        else if (m_pDestinationFile->GetPageDescriptor(0)->GetResolutionDescriptor((unsigned short)0)->GetBlockType() == HRFBlockType::STRIP)
            SelectedMemMgr = HPMPool::ExportRaster; // Optimize - Stripped output

        // Disable memory manager if 1bitRLE,  because the editor replace the buffer in the packet...
        pDstObjectLog = new HPMPool((uint32_t)PoolSizeKB, SelectedMemMgr);

        // Convert the destination file to a raster object using an HRSObjectStore.
        // Put the HRF raster in a store so that we can pull a HRARaster afterwards.
        HFCPtr<HGF2DCoordSys> pDstCoordSys;
        pDstCoordSys        = m_pClusterWorld->GetCoordSysReference(m_pDestinationFile->GetWorldIdentificator());
        m_pDestinationStore = new HRSObjectStore(pDstObjectLog, m_pDestinationFile, 0, pDstCoordSys);
        m_pDestinationStore->SetThrowable(true);

        // Get the raster from the store.
        m_pDestinationRaster = m_pDestinationStore->LoadRaster();

        // if we are in RasterToFile or we resample, move the destination
        if (m_pSourceFile == 0 || m_Resample)
            {
            // Get the extent of the clip shape and change it to the logical CoordSys
            // of the destination.
            HFCPtr<HVEShape> pTmpShape = new HVEShape(m_ClipShape);
            pTmpShape->ChangeCoordSys(m_pDestinationRaster->GetPhysicalCoordSys());
            HGF2DExtent    ClipExtent(pTmpShape->GetExtent());
            HVE2DRectangle aRectangle(ClipExtent.GetXMin(), ClipExtent.GetYMin(),
                                      ClipExtent.GetXMax(), ClipExtent.GetYMax(),
                                      pTmpShape->GetCoordSys());

            pTmpShape = new HVEShape (aRectangle);

            pTmpShape->ChangeCoordSys(m_pDestinationRaster->GetCoordSys());
            ClipExtent = pTmpShape->GetExtent();

            // Get the extent of the destination shape.
            pTmpShape = new HVEShape(*m_pDestinationRaster->GetEffectiveShape());
            pTmpShape->ChangeCoordSys(m_pDestinationRaster->GetCoordSys());
            HGF2DExtent ClipExtentDst(pTmpShape->GetExtent());

            // If the destination file don't support transformation model
            // when need to set the scale factor on the raster.
            if (!DstSupportsTransfo)
                {
                m_pDestinationRaster->Scale(1.0/m_ScaleFactorX, 1.0/m_ScaleFactorY,
                                            HGF2DLocation(ClipExtent.GetCoordSys()));
                }

            // Set the translation of the destination raster to position
            // it over the source one.
            Displacement.SetDeltaX(ClipExtent.GetXMin() - ClipExtentDst.GetXMin());
            Displacement.SetDeltaY(ClipExtent.GetYMin() - ClipExtentDst.GetYMin());
            m_pDestinationRaster->Move(Displacement);

            m_pDestinationRaster->SetShape(m_LogicalShape);
            }

        HRACopyFromOptions CopyFromOptions;

        if (m_Resample)
            {
            CopyFromOptions.SetResamplingMode(m_Resampling);
            }

        if (m_Resample && m_pResamplingFilter != 0)
            {
            // Specify Cubic interpolation
            HASSERT(false);
            }

        CopyFromOptions.SetAlphaBlend(m_BlendAlpha);

        HFCPtr<HVEShape> pSrcExtent;
        HVEShape* pCopyShape = 0;
        // if we are in FileToFile without resample and the destination support shape
        // we remove the shape and copy all the file, the shape will be add after the
        // copy on the destination raster
        if (m_pSourceFile != 0 && !m_Resample &&
            m_pDestinationFile->GetCapabilities()->GetCapabilityOfType(HRFClipShapeCapability::CLASS_ID, HFC_WRITE_ONLY) != 0)
            {
            // FileToFile without resample
            // because we have a source file, we have necessary a HRAStoredRaster
            HPRECONDITION(m_pSourceRaster->IsCompatibleWith(HRAStoredRaster::CLASS_ID));

            CHECK_HUINT64_TO_HDOUBLE_CONV(m_pSourceFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth())
            CHECK_HUINT64_TO_HDOUBLE_CONV(m_pSourceFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight())

            pSrcExtent = new HVEShape(0.0,
                                      0.0,
                                      (double)m_pSourceFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth(),
                                      (double)m_pSourceFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight(),
                                      ((HFCPtr<HRAStoredRaster>&)m_pSourceRaster)->GetPhysicalCoordSys());
            pCopyShape = pSrcExtent;
            }
        else
            {
            pCopyShape = &m_LogicalShape;
            }

        // Copy the image data by block, since HRA don't give us any guarantee about
        // the order in which the data is copied (especially when we dealing with mosaic).
        uint64_t BlockPosX;
        uint64_t BlockPosY;

        HUTExportProgressIndicator::GetInstance()->SetExportedFile(m_pDestinationFile);

        HFCPtr<HRFRasterFile> pOriginalFile = m_pDestinationFile;
        if (m_pDestinationFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
            {
            pOriginalFile = ((HFCPtr<HRFRasterFileExtender>&)m_pDestinationFile)->GetOriginalFile();
            }

#ifdef __ECW_EXPORT_ENABLE__
        // Is the destination is the COM ECW(ERMapper) (1429) OR Is the destination is COM Jpeg2000File (1477)
        if ((pOriginalDestRaster->GetClassID() == HRFEcwFile::CLASS_ID) || (pOriginalDestRaster->GetClassID() == HRFJpeg2000File::CLASS_ID))
            {

            // Initialize ECWReadInfoStruct
            ECWReadInfoStruct ReadInfoStruct;
            ReadInfoStruct.m_pSourceFile = m_pSourceFile;
            ReadInfoStruct.m_pSourceRaster = (HFCPtr<HRAStoredRaster>&)m_pSourceRaster;
            ReadInfoStruct.m_pDestinationFile = m_pDestinationFile;
            ReadInfoStruct.m_pDestinationRaster = (HFCPtr<HRAStoredRaster>&)m_pDestinationRaster;
            ReadInfoStruct.m_CopyFromOptions = CopyFromOptions;

            // TR #194882
            // PDF library must be run into a single thread, this means that the CopyFrom() must be run into the main thread.
            // When we export to ECW, the ReadCallback() function is called by another thread then the main thread. For this
            // reason, we create a new thread, the ECW API is initialized into the new thread, the ReadCallback() signal an
            // event for the main thread
            // Create the thread
            // This thread will initialize ECW API and call NCSEcwCompress()
            // The callback function ReadCallback will be called by ECW API for each line.
            // When the PDF data is needed, ReadCallback() will signal m_ComputeStripEvent.
            PDFExporterThread ExportThread(ReadInfoStruct);
            ExportThread.StartThread();

            HFCSynchroContainer Synchros;
            Synchros.AddSynchro (&ExportThread);    // ExportThread will be signaled when the thread stop
            Synchros.AddSynchro (&ReadInfoStruct.m_ComputeStripEvent);

            if (0 != HFCSynchro::WaitForMultipleObjects (Synchros, false))
                {
                do
                    {
                    // execute the CopyFrom
                    ExportThread.ComputeStrip();

                    }
                while (0 != HFCSynchro::WaitForMultipleObjects (Synchros, false) && HUTExportProgressIndicator::GetInstance()->ContinueIteration(m_pDestinationFile, 0));
                }

            if (!ExportThread.ExportCompleted())
                {
                ExportThread.CancelExport();
                //Wait until the library cancels the export
                do
                    {
                    ReadInfoStruct.m_StripComputedEvent.Signal();
                    }
                while (!ExportThread.WaitUntilSignaled(100));
                }

            if ((ExportThread.GetLastError() != NCS_SUCCESS) &&
                (ExportThread.GetLastError() != NCS_USER_CANCELLED_COMPRESSION))
                {
                if (ExportThread.GetLastError() == NCS_INPUT_SIZE_TOO_SMALL)
                    {
                    throw HRFException(HRF_TOO_SMALL_FOR_ECW_COMPRESSION_EXCEPTION, m_pDestinationFile->GetURL()->GetURL());
                    }
                else
                    {
                    throw HFCFileException(HFC_FILE_NOT_CREATED_EXCEPTION, m_pDestinationFile->GetURL()->GetURL());
                    }
                }

            //Sub-resolution computation is done by the ErMapper library above.
            if (m_pDestinationRaster->GetClassID() == HRAPyramidRaster::CLASS_ID)
                {
                ((HFCPtr<HRAPyramidRaster>&)m_pDestinationRaster)->EnableSubImageComputing(false);
                }
            }
        else
#endif // __ECW_EXPORT_ENABLE__
            {
            HFCPtr<ExportSizeEstimator> pExportSizeEstimator = 0;


            HFCPtr<HFCProgressEvaluator> pExportSizeEstimation;

            if (m_EstimateExportSize == true)
                {
                pExportSizeEstimation = new HFCProgressEvaluator(HFCProgressEvaluator::COMPRESSED_DATA_ESTIMATED_SIZE);
                pExportSizeEstimation->SetValue(0);
                HUTExportProgressIndicator::GetInstance()->AddEvaluator(pExportSizeEstimation);

                pExportSizeEstimator = new ExportSizeEstimator(m_pDestinationFile,
                                                               pOriginalFile,
                                                               1 * CLOCKS_PER_SEC);

                }

            uint64_t TotalNbBlocks = 0;

            HRFPageDescriptor* pPageDescriptor = m_pDestinationFile->GetPageDescriptor(0);

            for (unsigned short ResInd = 0; ResInd < pPageDescriptor->CountResolutions(); ResInd++)
                {
                TotalNbBlocks += pPageDescriptor->GetResolutionDescriptor(ResInd)->CountBlocks();
                }

            HUTExportProgressIndicator::GetInstance()->Restart(TotalNbBlocks);

            HRAClearOptions ClearOptions;
            ClearOptions.SetLoadingData(true);          // force loading the data
            ClearOptions.SetApplyRasterClipping(false); // the destination block shape will be set, we
            // want to clear the entire block

            // The last progress iteration will be executed at the end of the method
            for (uint32_t Index = 0 ; ((Index < pDstResDesc->CountBlocks()) && !HUTExportProgressIndicator::GetInstance()->IsIterationStopped()) ; Index++)
                {
                //TR 223786 : When the destination pool is very big (especially for RLE) and
                //the resampling complex (warping), it can take a long time before a block is
                //written to the file, thus making the progress bar sticking to 0.
                //So ContinueIteration is also called here to update the progress bar based
                //on some factors.
                HUTExportProgressIndicator::GetInstance()->ContinueIteration(m_pDestinationFile,
                                                                             0,
                                                                             true);
                // Get the position of the current block.
                pDstResDesc->ComputeBlockPosition(Index, &BlockPosX, &BlockPosY);

                CHECK_HUINT64_TO_HDOUBLE_CONV( BlockPosX + pDstResDesc->GetBlockWidth())
                CHECK_HUINT64_TO_HDOUBLE_CONV(BlockPosY + pDstResDesc->GetBlockHeight())

                // Create a shape that represent the block to copy.
                HVEShape BlockShape(HVE2DRectangle((double)BlockPosX,
                                                   (double)BlockPosY,
                                                   (double)BlockPosX + pDstResDesc->GetBlockWidth(),
                                                   (double)BlockPosY + pDstResDesc->GetBlockHeight(),
                                                   m_pDestinationRaster->GetPhysicalCoordSys()));

                // Clear the destination to ensure that all parts of the destination are initialized.
                // Force load to ensure that the tile is really cleared, not simply flagged to be cleared.

                ClearOptions.SetShape(&BlockShape);

                m_pDestinationRaster->Clear(ClearOptions);

                // Intersect the block shape with the source raster shape.
                BlockShape.Intersect(*pCopyShape);
                CopyFromOptions.SetDestShape(&BlockShape);

                if (!(BlockShape.IsEmpty()))
                    {
                    // Copy the data from the source raster into the destination one.
                    if (m_pSourceRaster->HasLookAhead())
                        m_pSourceRaster->SetLookAhead(BlockShape.GetExtent(), 0);

                    // Copy the source raster in the destination
                    m_pDestinationRaster->CopyFrom((HFCPtr<HRARaster>&)m_pSourceRaster, CopyFromOptions);
                    }

                // Remove the shape pointer because it's a local...
                CopyFromOptions.SetDestShape(0);

                if (m_EstimateExportSize == true)
                    {
                    pExportSizeEstimator->Estimate(pExportSizeEstimation);
                    }
                }
            }

        // Update SubRes if it is a pyramid
        if (m_pDestinationRaster->GetClassID() == HRAPyramidRaster::CLASS_ID)
            ((HFCPtr<HRAPyramidRaster>&)m_pDestinationRaster)->UpdateSubResolution();

        // Restore the shape of the source raster to it's old one and set the
        // destination one to the logical shape.
        m_LogicalShape.ChangeCoordSys(m_pDestinationRaster->GetCoordSys());
        m_pDestinationRaster->SetShape(m_LogicalShape);

        if (pOriginalFile->IsCompatibleWith(HRFHMRFile::CLASS_ID))
            {
            HRASamplingOptions  SamplingOptions;
            HFCPtr<HRPPixelType> pPixelType;
            if (m_pDestinationRaster->GetPixelType()->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID))
                pPixelType = new HRPPixelTypeI8R8G8B8(); // by default, the palette is gray scale
            else
                pPixelType = m_pDestinationRaster->GetPixelType();

            HRAHistogramOptions HistogramOptions(pPixelType);
            SamplingOptions.SetPixelsToScan(100);
            SamplingOptions.SetTilesToScan(100);
            SamplingOptions.SetPyramidImageSize(100);
            HistogramOptions.SetSamplingOptions(SamplingOptions);
            m_pDestinationRaster->ComputeHistogram(&HistogramOptions);
            pHistogram = HistogramOptions.GetHistogram();
            }

        // Save the destination raster file.
        m_pDestinationRaster->Save();

        // If the destination raster file support hystogram, we set the computed one.
        if ((m_pDestinationFile->GetCapabilities()->GetCapabilityOfType(HRFHistogramCapability::CLASS_ID, HFC_CREATE_ONLY) != 0) &&
            (pHistogram != 0))
            {
            pDstPageDesc->SetHistogram(*pHistogram);
            }

        // Indicate that we have done an export.
        m_AlreadyExported = true;

        // Clean up the object to flush all modification to the raster.
        CleanUp();

        HUTExportProgressIndicator::GetInstance()->RemoveEvaluator(HFCProgressEvaluator::COMPRESSED_DATA_ESTIMATED_SIZE);
        HUTExportProgressIndicator::GetInstance()->SetExportedFile(0);
        }
    catch(HFCException& rException)
        {
        HUTExportProgressIndicator::GetInstance()->RemoveEvaluator(HFCProgressEvaluator::COMPRESSED_DATA_ESTIMATED_SIZE);
        HUTExportProgressIndicator::GetInstance()->SetExportedFile(0);

        if (m_pDestinationStore != 0)
            m_pDestinationStore->ForceReadOnly(true);

        CleanUp();

        throw rException;
        }
    catch(...)
        {
        HUTExportProgressIndicator::GetInstance()->RemoveEvaluator(HFCProgressEvaluator::COMPRESSED_DATA_ESTIMATED_SIZE);
        HUTExportProgressIndicator::GetInstance()->SetExportedFile(0);

        if (m_pDestinationStore != 0)
            m_pDestinationStore->ForceReadOnly(true);

        CleanUp();

        throw;
        }
    }

//#pragma optimize("", on)


//-----------------------------------------------------------------------------
// Private
// This method "improve" the source file for our purpose.
//-----------------------------------------------------------------------------
void HUTExportToFile::InitSourceFile()
    {
    // Adapt Scan Line Orientation (1 bit images)
    if (m_pSourceFile->IsCompatibleWith(HRFIntergraphFile::CLASS_ID) ||
        m_pSourceFile->IsCompatibleWith(HRFCalsFile::CLASS_ID)       ||
        m_pSourceFile->IsCompatibleWith(HRFLRDFile::CLASS_ID)        )
        {
        if (HRFSLOStripAdapter::NeedSLOAdapterFor(m_pSourceFile))
            {
            // Adapt only when the raster file has not a standard scan line orientation
            // i.e. with an upper left origin, horizontal scan line.
            m_pSourceFile = HRFSLOStripAdapter::CreateBestAdapterFor(m_pSourceFile);
            }
        }

    // Add the Decoration HGR or the TFW Page File
    if (HRFPageFileFactory::GetInstance()->HasFor(m_pSourceFile))
        m_pSourceFile= new HRFRasterFilePageDecorator(m_pSourceFile, HRFPageFileFactory::GetInstance()->FindCreatorFor(m_pSourceFile));

    // Get the block heights
    uint32_t Dst_BlockHeight = m_pDestinationFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockHeight();
    uint32_t Src_BlockHeight = m_pSourceFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockHeight();

    // Destination is already adapted to Strip or Image. If our blocks are of the same height
    // (tiles or strips), we're fine. Otherwise, strip at the same height as the destination.
    if (Dst_BlockHeight != Src_BlockHeight ||
        m_pSourceFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType()->CountPixelRawDataBits() == 1)
        {
        uint32_t AdaptHeight = max(HRFImportExport_ADAPT_HEIGHT, Dst_BlockHeight);

        // Adapt the source raster file to srip.
        // The strip adapter is the fastest access that we can use when we export a file.
        HASSERT(m_pSourceFile->CountPages() == 1);
        HRFRasterFileBlockAdapter::BlockDescriptorMap BlockDescMap;
        HRFRasterFileBlockAdapter::BlockDescriptor    BlockDesc;
        BlockDesc.m_BlockType   = HRFBlockType(HRFBlockType::STRIP);

        HASSERT(m_pSourceFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth() <= ULONG_MAX);
        BlockDesc.m_BlockWidth  = (uint32_t)m_pSourceFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth();
        BlockDesc.m_BlockHeight = AdaptHeight;
        BlockDescMap.insert(HRFRasterFileBlockAdapter::BlockDescriptorMap::value_type(0, BlockDesc));

        if (HRFRasterFileBlockAdapter::CanAdapt(m_pSourceFile, BlockDescMap))
            m_pSourceFile = new HRFRasterFileBlockAdapter(m_pSourceFile, BlockDescMap);
        }

    // Don't use the Cache if possible, to increase the quality on the output Image.
    //
    // Check if we need to put a cache on the source file.
    if (m_SrcNeedCache)
        {
        // Check if we already have a cache for the source file, if so we open it!
        if (HRFiTiffCacheFileCreator::GetInstance()->HasCacheFor(m_pSourceFile))
            {
            HFCPtr<HRFRasterFile> pCacheFile;
            bool                 IsResBooster;

            // Get the cache and check if it is a resolution booster (when the cache don't have
            // the same number of resolutions than the source file).
            pCacheFile   = HRFiTiffCacheFileCreator::GetInstance()->GetCacheFileFor(m_pSourceFile);
            IsResBooster = m_pSourceFile->GetPageDescriptor(0)->CountResolutions() != pCacheFile->GetPageDescriptor(0)->CountResolutions();

            // Flush the reference on the cache.
            pCacheFile = 0;

            // If we have a resolution booster we open it, else we only open a cache file.
            if (IsResBooster)
                m_pSourceFile = new HRFRasterFileResBooster(m_pSourceFile, HRFiTiffCacheFileCreator::GetInstance());
            else
                m_pSourceFile = new HRFRasterFileCache(m_pSourceFile, HRFiTiffCacheFileCreator::GetInstance(), false);
            }
        else
            // Add a Cache
            m_pSourceFile = new HRFRasterFileCache(m_pSourceFile, HRFiTiffCacheFileCreator::GetInstance(), true);
        }

    // Check the access type of the source raster.
    m_SrcBlockAccess = m_pSourceFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetReaderBlockAccess();
    }

//-----------------------------------------------------------------------------
// Private
// This method open the specified file as the source file.  This method
// do not attach any raster to the file.
//-----------------------------------------------------------------------------
void HUTExportToFile::OpenSourceFile(const HFCPtr<HFCURL>& pi_rpSourceURL)
    {
    HPRECONDITION(pi_rpSourceURL != 0);

    // Open the source file.
    m_pSourceFile = HRFRasterFileFactory::GetInstance()->OpenFile(pi_rpSourceURL, true);

    InitSourceFile();
    }

//-----------------------------------------------------------------------------
// Private
// This method force the destructor of the source file.
//-----------------------------------------------------------------------------

void HUTExportToFile::CloseSourceFile()
    {
    // To force the destruction of the source file we also need to destroy
    // the raster attached to it.
    m_pSourceRaster = 0;
    m_pSourceFile   = 0;
    }

//-----------------------------------------------------------------------------
// Private
// This method computed the transformation to apply to the destination file.
//-----------------------------------------------------------------------------

HFCPtr<HGF2DTransfoModel> HUTExportToFile::ComputeTransformation(HFCPtr<HRFPageDescriptor>       pi_pDstPageDesc,
                                                                 HFCPtr<HRFResolutionDescriptor> pi_pDstResDesc)
    {
    HFCPtr<HGF2DTransfoModel> pPhysicalToLogicalCS;

    // Check if we need to resample, if not we need to set the transformation
    // model of the destination file.
    if (m_Resample)
        {
        if (pi_pDstPageDesc->HasTransfoModel())
            pPhysicalToLogicalCS = pi_pDstPageDesc->GetTransfoModel();
        else
            pPhysicalToLogicalCS = new HGF2DIdentity();

        HFCPtr<HGF2DCoordSys> pPhysicalWorld;
        HRFScanlineOrientation SLO = pi_pDstResDesc->GetScanlineOrientation();
        if (SLO == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
            pPhysicalWorld =  m_pClusterWorld->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD);
        else if (SLO == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
            pPhysicalWorld = m_pClusterWorld->GetCoordSysReference(HGF2DWorld_HMRWORLD);
        else
            {
            HASSERT(0);
            }

        HFCPtr<HGF2DCoordSys> pLogicalCS(new HGF2DCoordSys(*pPhysicalToLogicalCS, pPhysicalWorld));

        // The scale was calculated with the physical world, for this reason we need to extract the model between
        // the real WorldID of the destination and physical world
        HFCPtr<HGF2DCoordSys>     pDstWorld;
        HFCPtr<HGF2DTransfoModel> pDstToLogicalCS;
        pDstWorld   = m_pClusterWorld->GetCoordSysReference(m_pDestinationFile->GetWorldIdentificator());
        pDstToLogicalCS = pDstWorld->GetTransfoModelTo(pLogicalCS);

        // Compose the two models.
        pPhysicalToLogicalCS = pPhysicalToLogicalCS->ComposeInverseWithInverseOf(*pDstToLogicalCS);
        }

    // We don't resample, check if we have the source raster file.
    else if (m_pSourceFile)
        {
        HFCPtr<HGF2DCoordSys>     pSrcWorld;
        HFCPtr<HGF2DCoordSys>     pDstWorld;
        HFCPtr<HGF2DTransfoModel> pSrcToDstWorld;

        // Get the transformation between the source file and the destination
        // file world CoordSys.
        pSrcWorld      = m_pClusterWorld->GetCoordSysReference(m_pSourceFile->GetWorldIdentificator());
        pDstWorld      = m_pClusterWorld->GetCoordSysReference(m_pDestinationFile->GetWorldIdentificator());
        pSrcToDstWorld = pSrcWorld->GetTransfoModelTo(pDstWorld);

        // Get the transformation model of the source file.
        if (m_pSourceFile->GetPageDescriptor(0)->HasTransfoModel())
            pPhysicalToLogicalCS = m_pSourceFile->GetPageDescriptor(0)->GetTransfoModel();
        else
            pPhysicalToLogicalCS = new HGF2DIdentity();

        // Compose the two models.
        pPhysicalToLogicalCS = pPhysicalToLogicalCS->ComposeInverseWithDirectOf(*pSrcToDstWorld);
        }
    else
        {
        HFCPtr<HGF2DCoordSys>     pDstWorld;
        HFCPtr<HGF2DTransfoModel> pSrcToDstWorld;

        // Get the transformation between the source raster and the destination
        // file world CoordSys.
        pDstWorld      = m_pClusterWorld->GetCoordSysReference(m_pDestinationFile->GetWorldIdentificator());
        pSrcToDstWorld = m_pSourceRaster->GetCoordSys()->GetTransfoModelTo(pDstWorld);

        HIMStoredRasterEquivalentTransfo SRETransfo(m_pSourceRaster);

        pPhysicalToLogicalCS = SRETransfo.GetEquivalentTransfoModel();

        // Compose the two models.
        pPhysicalToLogicalCS = pPhysicalToLogicalCS->ComposeInverseWithDirectOf(*pSrcToDstWorld);
        }

    HFCPtr<HGF2DCoordSys> pLogicalCS;
    HFCPtr<HGF2DCoordSys> pPhysicalCS;

    // Retreive the coordinate system for the raster.
    pLogicalCS  = m_pClusterWorld->GetCoordSysReference(m_pDestinationFile->GetWorldIdentificator());
    pPhysicalCS = new HGF2DCoordSys(*pPhysicalToLogicalCS, pLogicalCS);

    HGF2DStretch Scale(HGF2DDisplacement(), 1.0/m_ScaleFactorX,
                       1.0/m_ScaleFactorY);
    pPhysicalToLogicalCS = Scale.ComposeInverseWithDirectOf(*pPhysicalCS->GetTransfoModelTo(pLogicalCS));

    // Set the new transformation model (only if the destination file support
    // transformation model).
    // IMPORTANT:
    //    - This must be done before extracting the raster from the file.
    //    - If the file don't support transformation model we need to set the
    //      scale factor when the raster is created.

    // Simplify the model
    HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pPhysicalToLogicalCS->CreateSimplifiedModel();
    if (pSimplifiedModel != 0)
        pPhysicalToLogicalCS = pSimplifiedModel;

    return pPhysicalToLogicalCS;
    }

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#ifdef __ECW_EXPORT_ENABLE__

//---------------------------------------------------------------------------
// Read callback function - called once for each input line
//---------------------------------------------------------------------------

static BOOLEAN ReadCallback(NCSEcwCompressClient* pClient,
                            UINT32                nNextLine,
                            IEEE4**               ppOutputBandBufferArray)
    {
    HPRECONDITION(((ECWReadInfoStruct*)(pClient->pClientData))->m_pSourceRaster   != 0);

    ECWReadInfoStruct* pReadInfo = (ECWReadInfoStruct*)pClient->pClientData;

    if (nNextLine == 0)
        {
        // Copy data from the source raster into the destination one.
        pReadInfo->m_pDestinationBitmap->Clear();
        // signal the event, the main thread will execute the CopyFrom...
        pReadInfo->m_ComputeStripEvent.Signal();
        // wait until the main signal that the CopyFrom is terminate
        pReadInfo->m_StripComputedEvent.WaitUntilSignaled();
        }
    else if (nNextLine >= pReadInfo->m_CurrentStripPos + pReadInfo->m_StripHeight)
        {
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Move the destination raster OVER the source file at the right position.
        HFCPtr<HGF2DTransfoModel> pLineModel = pReadInfo->m_pDestinationBitmap->GetTransfoModel();

        pLineModel = pReadInfo->m_pLineTranslateModel->ComposeInverseWithDirectOf (*pLineModel);
        pReadInfo->m_pDestinationBitmap->SetTransfoModel(*pLineModel, pReadInfo->m_pDestinationBitmap->GetCoordSys());

        // Copy data from the source raster into the destination one.
        pReadInfo->m_pDestinationBitmap->Clear();
        // signal the event, the main thread will execute the CopyFrom...
        pReadInfo->m_ComputeStripEvent.Signal();
        // wait until the main signal that the CopyFrom is terminate
        pReadInfo->m_StripComputedEvent.WaitUntilSignaled();

        pReadInfo->m_CurrentStripPos = pReadInfo->m_CurrentStripPos + pReadInfo->m_StripHeight;
        }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Then convert our buffer into the ECW format.

    const Byte* pData = pReadInfo->m_pDestinationBitmap->GetPacket()->GetBufferAddress() + (nNextLine - pReadInfo->m_CurrentStripPos) * pReadInfo->m_LineWidthInByte;

    unsigned int ValueIndex;
    IEEE4* pOutputValue;

    for(unsigned int Channel = 0; Channel < pClient->nInputBands; Channel++)
        {
        pOutputValue = ppOutputBandBufferArray[Channel];
        ValueIndex   = Channel;

        for (unsigned int PixelIndex = 0; PixelIndex < pClient->nInOutSizeX; PixelIndex++)
            {
            // Be sure to remain inbound.
            HASSERT(ValueIndex < pReadInfo->m_pDestinationBitmap->GetPacket()->GetBufferSize());

            // Compression needs input to be IEEE4
            pOutputValue[PixelIndex] =  *(pData + ValueIndex);
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
    HPRECONDITION(((ECWReadInfoStruct*)(pClient->pClientData))->m_pSourceRaster != 0);

    ECWReadInfoStruct* pReadInfo = (ECWReadInfoStruct*)pClient->pClientData;

    return pReadInfo->m_CancelExport;
    }

#endif // __ECW_EXPORT_ENABLE__


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HUTExportToFile::ExportSizeEstimator::ExportSizeEstimator(HFCPtr<HRFRasterFile>& pi_prDestinationFile,
                                                          HFCPtr<HRFRasterFile>& pi_prOriginalFile,
                                                          clock_t                 pi_EstimateInterval)
    {
    HPRECONDITION(pi_prDestinationFile != 0);
    HPRECONDITION(pi_prOriginalFile != 0);
    HPRECONDITION(pi_EstimateInterval >= CLOCKS_PER_SEC / 2);

    m_pOriginalRasterFile = pi_prOriginalFile;
    m_EstimateInterval      = pi_EstimateInterval;

    HRFPageDescriptor* pPageDescriptor = pi_prDestinationFile->GetPageDescriptor(0);
    m_TotalNbOfPixelsAllRes = 0;
    m_NbRes = pPageDescriptor->CountResolutions();

    m_NbPixelsPerBlock.resize(pPageDescriptor->CountResolutions(), 0);

    for (unsigned short ResInd = 0; ResInd < pPageDescriptor->CountResolutions(); ResInd++)
        {
        m_TotalNbOfPixelsAllRes += pPageDescriptor->GetResolutionDescriptor(ResInd)->GetWidth() *
                                   pPageDescriptor->GetResolutionDescriptor(ResInd)->GetHeight();

        m_NbPixelsPerBlock[ResInd] = pPageDescriptor->GetResolutionDescriptor(ResInd)->GetBlockWidth() *
                                     pPageDescriptor->GetResolutionDescriptor(ResInd)->GetBlockHeight();
        }

    m_LastCheckTime = clock();
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HUTExportToFile::ExportSizeEstimator::~ExportSizeEstimator()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Estimate the final export at some periodic intervals.
//-----------------------------------------------------------------------------
void HUTExportToFile::ExportSizeEstimator::Estimate(HFCProgressEvaluator* pi_pProgressEvaluator)
    {
    if ((clock() > (m_LastCheckTime + m_EstimateInterval)))
        {
        uint64_t EstimatedFileSize = EstimateCompressedFileSize();

        pi_pProgressEvaluator->SetValue((double)EstimatedFileSize);

        m_LastCheckTime = clock();
        }
    }

//-----------------------------------------------------------------------------
// Private
// This method estimates the size of a compressed file from the block already
// written.
//-----------------------------------------------------------------------------
uint64_t HUTExportToFile::ExportSizeEstimator::EstimateCompressedFileSize()
    {
    HPRECONDITION(m_TotalNbOfPixelsAllRes != 0);

    uint64_t EstimatedCompressedFileSize = 0;

    if (HUTExportProgressIndicator::GetInstance()->GetProcessedIteration() == 0)
        {
        EstimatedCompressedFileSize = 0;
        }
    else
        {
        uint64_t NbExportedPixels = 0;

        for (int ResInd = 0; ResInd < m_NbRes; ResInd++)
            {
            NbExportedPixels += HUTExportProgressIndicator::GetInstance()->GetNbExportedBlocks(ResInd) *
                                m_NbPixelsPerBlock[ResInd];
            }

        EstimatedCompressedFileSize = (uint64_t)((double)m_pOriginalRasterFile->GetFileCurrentSize() / NbExportedPixels * m_TotalNbOfPixelsAllRes);
        }

    return EstimatedCompressedFileSize;
    }