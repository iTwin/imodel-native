//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hut/src/HUTImportFromFileExportToFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class: HUTImportFromFileExportToFile
// ----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>




#include <Imagepp/all/h/HUTImportFromFileExportToFile.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HUTExportToFile.h>
#include <Imagepp/all/h/HRFCalsFile.h>
#include <Imagepp/all/h/HRFLRDFile.h>
#include <Imagepp/all/h/HRFIntergraphFile.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFSLOStripAdapter.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFRasterFileBlockAdapter.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HRSObjectStore.h>
#include <Imagepp/all/h/HGFHMRStdWorldCluster.h>
#include <Imagepp/all/h/HRFPageFileFactory.h>
#include <Imagepp/all/h/HRFRasterFilePageDecorator.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HFCGrid.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HCPGeoTiffKeys.h>

//-----------------------------------------------------------------------------
// Constructor
// Creation and destruction interface
//-----------------------------------------------------------------------------
HUTImportFromFileExportToFile::HUTImportFromFileExportToFile(const HFCPtr<HGF2DWorldCluster>& pi_pWorldCluster,
                                                             bool                            pi_BestMatchTag)
    : HRFImportExport(pi_pWorldCluster),
      m_Resampling(HGSResampling::AVERAGE)
    {
    m_pWorldCluster         = pi_pWorldCluster;
    m_BestMatchTag          = pi_BestMatchTag;
    m_NbColorsIfIndexed     = 0;    // MaxEntries by default

    HASSERT(CountExportFileFormat() > 0);
    SelectExportFileFormatByIndex(0);
    }

//-----------------------------------------------------------------------------
// Destructor
// Creation and destruction interface
//-----------------------------------------------------------------------------
HUTImportFromFileExportToFile::~HUTImportFromFileExportToFile()
    {
    }

//-----------------------------------------------------------------------------
// SelectImportFilename
// Export File name interface
//-----------------------------------------------------------------------------
void HUTImportFromFileExportToFile::SelectImportFilename(const HFCPtr<HFCURL>& pi_rpURLPath)
    {
    m_pSelectedImportFilename = pi_rpURLPath;

    // Open the import file (the file is close before the export, so we do not
    // need to change it's access mode).
    m_pSelectedImportFile = HRFRasterFileFactory::GetInstance()->OpenFile(m_pSelectedImportFilename, true);

    InitImportRasterFile();
    }

//-----------------------------------------------------------------------------
// InitImportRasterFile
//-----------------------------------------------------------------------------
void HUTImportFromFileExportToFile::InitImportRasterFile()
    {
    HASSERT(m_pSelectedImportFile != 0 && m_pSelectedImportFile->CountPages() > 0 &&
            m_pSelectedImportFile->GetPageDescriptor(0)->CountResolutions() > 0);

    // Adapt Scan Line Orientation (1 bit images)
    if (m_pSelectedImportFile->IsCompatibleWith(HRFIntergraphFile::CLASS_ID) ||
        m_pSelectedImportFile->IsCompatibleWith(HRFCalsFile::CLASS_ID      ) ||
        m_pSelectedImportFile->IsCompatibleWith(HRFLRDFile::CLASS_ID       ) )
        {
        if (HRFSLOStripAdapter::NeedSLOAdapterFor(m_pSelectedImportFile))
            {
            // Adapt only when the raster file has not a standard scan line orientation
            // i.e. with an upper left origin, horizontal scan line.
            m_pSelectedImportFile = HRFSLOStripAdapter::CreateBestAdapterFor(m_pSelectedImportFile);
            }
        }

    // Set Original dimension
    CHECK_HUINT64_TO_HDOUBLE_CONV(m_pSelectedImportFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth())
    CHECK_HUINT64_TO_HDOUBLE_CONV(m_pSelectedImportFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight())

    m_OriginalSize.SetX((double)m_pSelectedImportFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth());
    m_OriginalSize.SetY((double)m_pSelectedImportFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight());

    // Add the Decoration HGR or the TFW Page File
    if (HRFPageFileFactory::GetInstance()->HasFor(m_pSelectedImportFile))
        m_pSelectedImportFile= new HRFRasterFilePageDecorator(m_pSelectedImportFile, HRFPageFileFactory::GetInstance()->FindCreatorFor(m_pSelectedImportFile));

    // Look if source raster has a model and if this model is diferent from indentity.
    if  (m_pSelectedImportFile->GetPageDescriptor(0)->HasTransfoModel())
        {
        // Simplify the model
        HFCPtr<HGF2DTransfoModel> pSimplifiedModel = m_pSelectedImportFile->GetPageDescriptor(0)->GetTransfoModel()->CreateSimplifiedModel();
        if (pSimplifiedModel != 0)
            m_SourceHasTransfo = (pSimplifiedModel->GetClassID() != HGF2DIdentity::CLASS_ID);
        else
            m_SourceHasTransfo =  (m_pSelectedImportFile->GetPageDescriptor(0)->GetTransfoModel()->GetClassID() != HGF2DIdentity::CLASS_ID);

        }

    // Resample Size
    //
    // Load Raster, to extract logical dimension for Resampling option
    HPMPool                 Log(0, NULL);
    HFCPtr<HRSObjectStore>  pStore  = new HRSObjectStore (&Log,
                                                          m_pSelectedImportFile,
                                                          0,
                                                          m_pWorldCluster->GetCoordSysReference(m_pSelectedImportFile->GetWorldIdentificator()));
    HASSERT(pStore != 0);
    HFCPtr<HRAStoredRaster> pRaster = pStore->LoadRaster();

    // A raster must have been pumped from store
    HASSERT(pRaster != 0);

    // The raster must have a size
    HASSERT(!pRaster->GetEffectiveShape()->IsEmpty());

    HVEShape ClipShape(*(pRaster->GetEffectiveShape()));

    // Set the sources shape in HFRImportExport.
    m_ClipShape = ClipShape;

    ClipShape.ChangeCoordSys(m_pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));

    HGF2DExtent TmpExtentMin;
    HGF2DExtent TmpExtentMax;
    pRaster->GetPixelSizeRange(TmpExtentMin, TmpExtentMax);

    // The minimum pixel size must be defined and its width and height must be greater than 0.0
    HASSERT(TmpExtentMin.IsDefined());
    HASSERT(TmpExtentMin.GetWidth() != 0.0);
    HASSERT(TmpExtentMin.GetHeight() != 0.0);

    HGF2DExtent PixelSize(TmpExtentMin.CalculateApproxExtentIn(m_pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD)));

    // PixelSize Area.
    m_DefaultResampleScaleFactorX = MIN(PixelSize.GetWidth(), PixelSize.GetHeight());

    // The pixel size may not be 0.0 (exact compare)
    HASSERT(m_DefaultResampleScaleFactorX != 0.0);

    // ScaleFactor
    m_DefaultResampleScaleFactorX =  1.0 / m_DefaultResampleScaleFactorX;
    m_DefaultResampleScaleFactorY =  m_DefaultResampleScaleFactorX;

    // Exceptionally, we specify a precision. We don't want to create
    // pixels that are not useful. 0.01 is quite arbitrary ;-)
    HFCGrid ResampleGrid(0.0,
                         0.0,
                         ClipShape.GetExtent().GetWidth()  * m_DefaultResampleScaleFactorX,
                         ClipShape.GetExtent().GetHeight() * m_DefaultResampleScaleFactorY,
                         0.01);

    // Set resample size in HRFImportExport.
    CHECK_HSINT64_TO_HDOUBLE_CONV(ResampleGrid.GetWidth())
    CHECK_HSINT64_TO_HDOUBLE_CONV(ResampleGrid.GetHeight())

    m_DefaultResampleSize.SetX((double)ResampleGrid.GetWidth());
    m_DefaultResampleSize.SetY((double)ResampleGrid.GetHeight());

    // Recompute the scale factors to reflect the real dimensions
    // as computed by the ResampleGrid.
    m_DefaultResampleScaleFactorX = ResampleGrid.GetWidth() / ClipShape.GetExtent().GetWidth();
    m_DefaultResampleScaleFactorY = ResampleGrid.GetHeight() / ClipShape.GetExtent().GetHeight();

    SetResample(false);
    SetResampleIsForce(false);

    BestMatchSelectedValues();
    }

//-----------------------------------------------------------------------------
// SetImportRasterFile
//-----------------------------------------------------------------------------
void HUTImportFromFileExportToFile::SetImportRasterFile(const HFCPtr<HRFRasterFile>& pi_rpRasterFile)
    {
    m_pSelectedImportFile = pi_rpRasterFile;
    m_pSelectedImportFilename = 0;

    InitImportRasterFile();
    }

//-----------------------------------------------------------------------------
// GetSelectedImportFilename
// Export File name interface
//-----------------------------------------------------------------------------
const HFCPtr<HFCURL>& HUTImportFromFileExportToFile::GetSelectedImportFilename() const
    {
    return m_pSelectedImportFilename;
    }

//-----------------------------------------------------------------------------
// GetImportRasterFile
// return the source raster file. Must be call after SelectImportFilename
// or SetImportRasterFile.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFile>& HUTImportFromFileExportToFile::GetImportRasterFile() const
    {
    return m_pSelectedImportFile;
    }

//-----------------------------------------------------------------------------
// StartExport
// Export interface
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HUTImportFromFileExportToFile::StartExport()
    {
    HFCPtr<HRFRasterFile> pDstRasterFile;

    try
        {
        // Create the export file
        pDstRasterFile  = CreateFileFromSelectedValues();

        bool IsCompressedImage = false;

        ValidateUncompressedExportSize(pDstRasterFile, &IsCompressedImage);

        // Get the block acces of the raster file, since if it is random we don't
        // need to add an adapter.
        HRFBlockAccess WriterBlockAccess;
        WriterBlockAccess = pDstRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWriterBlockAccess();

        // Get the height of the source blocks.
        uint32_t Src_BlockHeight = m_pSelectedImportFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockHeight();

        // If blockType is Image, set HRFImportExport_ADAPT_HEIGHT otherwise the CanAdapt below can fail.
        if (m_pSelectedImportFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetBlockType() == HRFBlockType::IMAGE)
            Src_BlockHeight = HRFImportExport_ADAPT_HEIGHT;

        uint32_t AdaptHeight = MAX(HRFImportExport_ADAPT_HEIGHT, Src_BlockHeight);

        // Adapt the destination raster file to strip.
        // The strip adapter is the fastest access that we can use when we export a file.
        // The strip adapter is the only one that guarantee that all data can be write
        // correctly without the use of a cache.
        HRFRasterFileBlockAdapter::BlockDescriptorMap BlockDescMap;
        HRFRasterFileBlockAdapter::BlockDescriptor    BlockDesc;

        HASSERT(pDstRasterFile->CountPages() == 1);
        BlockDesc.m_BlockType   = HRFBlockType(HRFBlockType::STRIP);
        BlockDesc.m_BlockWidth  = GetImageWidth();
        BlockDesc.m_BlockHeight = AdaptHeight;
        BlockDescMap.insert(HRFRasterFileBlockAdapter::BlockDescriptorMap::value_type(0, BlockDesc));

        if (HRFRasterFileBlockAdapter::CanAdapt(pDstRasterFile, BlockDescMap))
            pDstRasterFile = new HRFRasterFileBlockAdapter(pDstRasterFile, BlockDescMap);

        if (WriterBlockAccess == HRFBlockAccess::SEQUENTIAL && pDstRasterFile->GetPageDescriptor(0)->CountResolutions() > 1)
            pDstRasterFile  = new HRFRasterFileCache(pDstRasterFile , HRFiTiffCacheFileCreator::GetInstance(), true);

        // If we do not resample, scale factor must be 1.0.
        HASSERT((!GetResample() && GetScaleFactorX() == 1.0 && GetScaleFactorX() == 1.0) ||
                GetResample());

        // Export the raster file.
        HAutoPtr<HUTExportToFile> pExport;

        if (m_pSelectedImportFilename)
            {
            // Close the input file since the export will open it itself.
            m_pSelectedImportFile = 0;

            pExport = new HUTExportToFile(m_pSelectedImportFilename,
                                          pDstRasterFile,
                                          m_pWorldCluster,
                                          GetScaleFactorX(),
                                          GetScaleFactorY(),
                                          GetResample(),
                                          m_pResamplingFilter);
            }
        else
            {
            pExport = new HUTExportToFile(m_pSelectedImportFile,
                                          pDstRasterFile,
                                          m_pWorldCluster,
                                          GetScaleFactorX(),
                                          GetScaleFactorY(),
                                          GetResample(),
                                          m_pResamplingFilter);
            }

        pExport->SetResamplingMode(m_Resampling);

        if (m_NbColorsIfIndexed != 0)
            pExport->SetNumberOfColorDestination(m_NbColorsIfIndexed);

        if (m_UseDestinationPaletteIfIndexed)
            pExport->SetUseDestinationPaletteIfIndexed(true);

        pExport->SetExportSizeEstimation(IsCompressedImage);

        pExport->Export();
        }
    catch (...)
        {
        //TR 193681 - Ensure that never written block during the export
        //process won't be considered as empty and written during the save
        //operation occurring at the destruction of the destination raster file.
        if (pDstRasterFile != 0)
            {
            pDstRasterFile->CancelCreate();
            }

        throw;
        }
    return pDstRasterFile;
    }

//-----------------------------------------------------------------------------
// BestMatchSelectedValues
// BestMatch interface
//-----------------------------------------------------------------------------
void HUTImportFromFileExportToFile::BestMatchSelectedValues()
    {
    if (m_pSelectedImportFile)
        {
        // BestMatch the pixel type with the import file
        HASSERT(m_pSelectedImportFile != 0 && m_pSelectedImportFile->CountPages() > 0 &&
                m_pSelectedImportFile->GetPageDescriptor(0)->CountResolutions() > 0);

        // Allow to change size and scale
        SetImageSizeIsLock(false);
        SetScaleFactorIsLock(false);
        SetMaintainAspectRatio(false);
        // If georeference format is in image and the selected format do not support
        // transfo model, we have to force resample.
        if ((m_SourceHasTransfo) &&
            (GetSelectedGeoreferenceFormat() == HRFGeoreferenceFormat::GEOREFERENCE_IN_IMAGE) &&
            !m_pSelectedFileFormatCapabilities->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID,
                                                                    HFC_CREATE_ONLY))
            {
            SetResample(true);
            SetResampleIsForce(true);

            SetImageWidth ((uint32_t)GetDefaultResampleSize().GetX());
            SetImageHeight((uint32_t)GetDefaultResampleSize().GetY());
            SetScaleFactorX(GetDefaultResampleScaleFactorX());
            SetScaleFactorY(GetDefaultResampleScaleFactorY());
            }
        else
            {
            SetResample(false);
            SetResampleIsForce(false);
            SetMaintainAspectRatio(true);
            }

        HFCPtr<HRPPixelType> pImportPixelType;
        pImportPixelType = m_pSelectedImportFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType();
        bool SamePixelTypeFound = false;
        HFCPtr<HRPPixelType> pPixelTypeSameNumberOfBits;
        bool                 SameNumberOfBitsFound = false;

        // check if we have color or grayscale pixel types
        bool ImportPixelTypeIsGray = false;
        if ((pImportPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::GRAY, 0) != HRPChannelType::FREE) ||
            (pImportPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::PREMULTIPLIED_GRAY, 0) != HRPChannelType::FREE))
            ImportPixelTypeIsGray = true;

        // Try to find the same PixelType
        for (uint32_t Index = 0; (Index < CountPixelType()) && (!SamePixelTypeFound); Index++)
            {
            HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
            pPixelTypeCapability  = ((HFCPtr<HRFPixelTypeCapability>&)m_pListOfValidPixelType->GetCapability(Index));

            if (pPixelTypeCapability->GetPixelTypeClassID() == pImportPixelType->GetClassID())
                {
                SelectPixelType(pImportPixelType->GetClassID());

                if(CountSubResPixelType())
                    SelectSubResPixelType(pImportPixelType->GetClassID());

                SamePixelTypeFound = true;
                }
            }

        if (!SamePixelTypeFound)
            {
            HFCPtr<HRPPixelType> pSameChannelOrgPixelType;

            // Try to find the PixelType with the same number and type of channels
            for (uint32_t Index = 0; (Index < CountPixelType()) && (!SamePixelTypeFound); Index++)
                {
                HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
                pPixelTypeCapability  = ((HFCPtr<HRFPixelTypeCapability>&)m_pListOfValidPixelType->GetCapability(Index));
                // Create the selected pixel type
                HFCPtr<HRPPixelType> pPixelType = HRPPixelTypeFactory::GetInstance()->Create(pPixelTypeCapability->GetPixelTypeClassID());
                bool                HasPalette       = (pPixelType->CountIndexBits() != 0);
                bool                ImportHasPalette = (pImportPixelType->CountIndexBits() != 0);

                if ((pPixelType->GetChannelOrg().CountChannels() ==
                     pImportPixelType->GetChannelOrg().CountChannels()) &&
                    (HasPalette == ImportHasPalette))
                    {
                    const HRPChannelOrg&  rChannelOrg       = pPixelType->GetChannelOrg();
                    const HRPChannelOrg&  rImportChannelOrg = pImportPixelType->GetChannelOrg();
                    uint32_t              ChannelInd        = 0;

                    for (; ChannelInd < rChannelOrg.CountChannels(); ChannelInd++)
                        {
                        const HRPChannelType* pChannelType(rChannelOrg.
                                                           GetChannelPtr(ChannelInd));
                        const HRPChannelType* pImportChannelType(rImportChannelOrg.
                                                                 GetChannelPtr(ChannelInd));

                        if ((pChannelType->GetRole() != pImportChannelType->GetRole()) ||
                            (pChannelType->GetDataType() != pImportChannelType->GetDataType()))
                            {
                            break;
                            }
                        }

                    if (ChannelInd == rChannelOrg.CountChannels())
                        {
                        if (pSameChannelOrgPixelType != 0)
                            {
                            HASSERT(pPixelType->CountPixelRawDataBits() !=
                                    pSameChannelOrgPixelType->CountPixelRawDataBits());

                            if (pPixelType->CountPixelRawDataBits() >
                                pSameChannelOrgPixelType->CountPixelRawDataBits())
                                {
                                pSameChannelOrgPixelType = pPixelType;
                                }
                            }
                        else
                            {
                            pSameChannelOrgPixelType = pPixelType;
                            }
                        }
                    }
                }

            if (pSameChannelOrgPixelType != 0)
                {
                SelectPixelType(pSameChannelOrgPixelType->GetClassID());

                if(CountSubResPixelType())
                    SelectSubResPixelType(pSameChannelOrgPixelType->GetClassID());

                SamePixelTypeFound = true;
                }
            }

        if (!SamePixelTypeFound)
            {
            // Try to find the PixelType with the same number of bits
            for (uint32_t Index = 0; (Index < CountPixelType()) && (!SamePixelTypeFound); Index++)
                {
                HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
                pPixelTypeCapability  = ((HFCPtr<HRFPixelTypeCapability>&)m_pListOfValidPixelType->GetCapability(Index));
                // Create the selected pixel type
                HFCPtr<HRPPixelType> pPixelType = HRPPixelTypeFactory::GetInstance()->Create(pPixelTypeCapability->GetPixelTypeClassID());

                if (pPixelType->CountPixelRawDataBits() == pImportPixelType->CountPixelRawDataBits())
                    {
                    // check if we have color or grayscale pixel types
                    // When we have color in source we prefer to select
                    // an upper pixeltype in bits to a gray scale pixeltype
                    bool PixelTypeIsGray = false;
                    if ((pPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::GRAY, 0) != HRPChannelType::FREE) ||
                        (pPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::PREMULTIPLIED_GRAY, 0) != HRPChannelType::FREE))
                        PixelTypeIsGray = true;

                    if (ImportPixelTypeIsGray == PixelTypeIsGray)
                        {
                        SelectPixelType(pPixelType->GetClassID());
                        if(CountSubResPixelType())
                            SelectSubResPixelType(pPixelType->GetClassID());
                        SamePixelTypeFound = true;
                        }
                    else
                        {
                        pPixelTypeSameNumberOfBits = pPixelType;
                        SameNumberOfBitsFound = true;
                        }
                    }
                }
            }
        if (!SamePixelTypeFound)
            {
            // Try to find the PixelType over and under in bits
            HFCPtr<HRPPixelType> pPixelTypeOver;
            HFCPtr<HRPPixelType> pPixelTypeUnder;

            // Try to find the near PixelType in bits
            for (uint32_t Index = 0; (Index < CountPixelType()) && (!SamePixelTypeFound); Index++)
                {
                HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
                pPixelTypeCapability  = ((HFCPtr<HRFPixelTypeCapability>&)m_pListOfValidPixelType->GetCapability(Index));
                // Create the selected pixel type
                HFCPtr<HRPPixelType> pPixelType = HRPPixelTypeFactory::GetInstance()->Create(pPixelTypeCapability->GetPixelTypeClassID());

                if (pPixelType->CountPixelRawDataBits() > pImportPixelType->CountPixelRawDataBits())
                    {
                    // Find the pixel type over in bits
                    if (pPixelTypeOver == 0)
                        pPixelTypeOver = pPixelType;
                    else if (pPixelType->CountPixelRawDataBits() < pPixelTypeOver->CountPixelRawDataBits())
                        pPixelTypeOver = pPixelType;
                    }
                else
                    {
                    // Find the pixel type over in bits
                    if (pPixelTypeUnder == 0)
                        pPixelTypeUnder = pPixelType;
                    else if (pPixelType->CountPixelRawDataBits() > pPixelTypeUnder->CountPixelRawDataBits())
                        pPixelTypeUnder = pPixelType;
                    }
                }
            if (pPixelTypeOver == 0)
                {
                if (SameNumberOfBitsFound)
                    {
                    // Set the best Pixel Type
                    SelectPixelType(pPixelTypeSameNumberOfBits->GetClassID());
                    if(CountSubResPixelType())
                        SelectSubResPixelType(pPixelTypeSameNumberOfBits->GetClassID());
                    SamePixelTypeFound = true;
                    }
                else
                    {
                    // Set the best Pixel Type
                    SelectPixelType(pPixelTypeUnder->GetClassID());
                    if(CountSubResPixelType())
                        SelectSubResPixelType(pPixelTypeUnder->GetClassID());
                    SamePixelTypeFound = true;
                    }
                }
            else
                {
                // check if we have color or grayscale pixel types
                // When we have color in source we prefer to select
                // an upper pixeltype in bits to a gray scale pixeltype
                bool PixelTypeOverIsGray = false;
                if ((pPixelTypeOver->GetChannelOrg().GetChannelIndex(HRPChannelType::GRAY, 0) != HRPChannelType::FREE) ||
                    (pPixelTypeOver->GetChannelOrg().GetChannelIndex(HRPChannelType::PREMULTIPLIED_GRAY, 0) != HRPChannelType::FREE))
                    PixelTypeOverIsGray = true;

                if ((PixelTypeOverIsGray == ImportPixelTypeIsGray) || (!SameNumberOfBitsFound))
                    {
                    // Set the best Pixel Type
                    SelectPixelType(pPixelTypeOver->GetClassID());
                    if(CountSubResPixelType())
                        SelectSubResPixelType(pPixelTypeOver->GetClassID());
                    SamePixelTypeFound = true;
                    }
                else
                    {
                    // Set the best Pixel Type
                    SelectPixelType(pPixelTypeSameNumberOfBits->GetClassID());
                    if(CountSubResPixelType())
                        SelectSubResPixelType(pPixelTypeSameNumberOfBits->GetClassID());
                    SamePixelTypeFound = true;
                    }
                }
            }

        // Best match encoding method.
        HASSERT(CountEncoding() > 0);
        SelectEncodingByIndex(0);

        // Tag best match
        uint32_t Page=0;

        //TR 238731 - Allow an application to have the choice to match all tags or not
        if (m_BestMatchTag == true)
            {
            HPMAttributeSet::HPMASiterator TagIterator;

            // be sure to add new tag from an empty list.
            ClearTagList();

            for (TagIterator  = m_pSelectedImportFile->GetPageDescriptor(Page)->GetTags().begin();
                 TagIterator != m_pSelectedImportFile->GetPageDescriptor(Page)->GetTags().end(); TagIterator++)
                {
                HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

                if (HasTag(*pTag))
                    SetTag(pTag);
                }
            }

        SetRasterFileGeocoding(*(m_pSelectedImportFile->GetPageDescriptor(Page)->GetRasterFileGeocoding().Clone()));
        }
    }

//-----------------------------------------------------------------------------
// SetNumberOfColorDestination
//-----------------------------------------------------------------------------
void HUTImportFromFileExportToFile::SetNumberOfColorDestination(uint32_t pi_NbColors)
    {
    m_NbColorsIfIndexed = pi_NbColors;
    }

//-----------------------------------------------------------------------------
// GetNumberOfColorDestination
//-----------------------------------------------------------------------------
uint32_t HUTImportFromFileExportToFile::GetNumberOfColorDestination() const
    {
    return m_NbColorsIfIndexed;
    }


//-----------------------------------------------------------------------------
// public
// SetResamplingMode
//-----------------------------------------------------------------------------
void HUTImportFromFileExportToFile::SetResamplingMode(const HGSResampling& pi_rResampling)
    {
    m_Resampling = pi_rResampling;
    }

//-----------------------------------------------------------------------------
// public
// GetResamplingMode
//-----------------------------------------------------------------------------
const HGSResampling& HUTImportFromFileExportToFile::GetResamplingMode() const
    {
    return m_Resampling;
    }


