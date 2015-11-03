//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFMrSIDFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFMrSIDFile
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLFile.h>

#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFMrSIDFile.h>
#include <Imagepp/all/h/HRFMrSIDEditor.h>

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DAffine.h>

#include <Imagepp/all/h/HVETileIDIterator.h>
#include <Imagepp/all/h/HTIFFTag.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#if defined(IPP_HAVE_MRSID_SUPPORT) 

#include <MrSid/lt_base.h>
#include <MrSid/lt_fileSpec.h>
#include <MrSid/lt_types.h>
#include <MrSid/lti_navigator.h>
#include <MrSid/lti_geoCoord.h>
#include <MrSid/lti_Pixel.h>
#include <MrSid/lti_SceneBuffer.h>
#include <MrSid/lti_Navigator.h>
#include <MrSid/lti_metadataReader.h>
#include <MrSid/lti_metadataDatabase.h>
#include <MrSid/lti_metadataRecord.h>
#include <MrSid/lti_types.h>
#include <MrSid/lti_viewerImageFilter.h>
#include <MrSid/lti_imageReader.h>
#include <MrSid/MrSIDImageReader.h>
#include <MrSid/lti_dynamicRangeFilter.h>


using namespace LizardTech;

#ifndef HGLOBAL_EPSILON
#define HGLOBAL_EPSILON (0.0000001)
#endif

#ifndef HDOUBLE_EQUAL_EPSILON
#define HDOUBLE_EQUAL_EPSILON(v1,v2)  ((v1 <= (v2+HGLOBAL_EPSILON)) && (v1 >= (v2-HGLOBAL_EPSILON)))
#endif


//-----------------------------------------------------------------------------
// HRFMrSIDBlockCapabilities
//-----------------------------------------------------------------------------
class HRFMrSIDBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFMrSIDBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        // Tile Capability

        Add(new HRFTileCapability(HFC_READ_ONLY,         // AccessMode
                                  LONG_MAX,            // MaxSizeInBytes
                                  1,                   // MinWidth
                                  LONG_MAX,            // MaxWidth
                                  1,                   // WidthIncrement
                                  1,                   // MinHeight
                                  LONG_MAX,            // MaxHeight
                                  1,                   // HeightIncrement
                                  true));              // Not Square
        }
    };

//-----------------------------------------------------------------------------
// HRFMrSIDCodecIdentityCapabilities
//-----------------------------------------------------------------------------
class HRFMrSIDCodecIdentityCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFMrSIDCodecIdentityCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFMrSIDBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFMrSIDCapabilities
//-----------------------------------------------------------------------------
HRFMrSIDCapabilities::HRFMrSIDCapabilities()
    : HRFRasterFileCapabilities()
    {
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFMrSIDCodecIdentityCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFMrSIDCodecIdentityCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // MultiResolution Capability
    Add(new HRFMultiResolutionCapability(HFC_READ_ONLY));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DProjective::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DAffine::CLASS_ID));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDocumentName()));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDateTime()));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    //Since the Geocoding can be specified using a WKT, all GeoTIFF keys are
    //needed.
    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(PCSCitation);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(GTCitation);
    pGeocodingCapability->AddSupportedKey(Projection);
    pGeocodingCapability->AddSupportedKey(ProjCoordTrans);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);
    pGeocodingCapability->AddSupportedKey(GeographicType);
    pGeocodingCapability->AddSupportedKey(GeogCitation);
    pGeocodingCapability->AddSupportedKey(GeogGeodeticDatum);
    pGeocodingCapability->AddSupportedKey(GeogPrimeMeridian);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnits);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnitSize);
    pGeocodingCapability->AddSupportedKey(GeogAngularUnits);
    pGeocodingCapability->AddSupportedKey(GeogAngularUnitSize);
    pGeocodingCapability->AddSupportedKey(GeogEllipsoid);
    pGeocodingCapability->AddSupportedKey(GeogSemiMajorAxis);
    pGeocodingCapability->AddSupportedKey(GeogSemiMinorAxis);
    pGeocodingCapability->AddSupportedKey(GeogInvFlattening);
    pGeocodingCapability->AddSupportedKey(GeogAzimuthUnits);
    pGeocodingCapability->AddSupportedKey(GeogPrimeMeridianLong);
    pGeocodingCapability->AddSupportedKey(ProjStdParallel1);
    pGeocodingCapability->AddSupportedKey(ProjStdParallel2);
    pGeocodingCapability->AddSupportedKey(ProjNatOriginLong);
    pGeocodingCapability->AddSupportedKey(ProjNatOriginLat);
    pGeocodingCapability->AddSupportedKey(ProjFalseEasting);
    pGeocodingCapability->AddSupportedKey(ProjFalseNorthing);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginLong);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginLat);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginEasting);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginNorthing);
    pGeocodingCapability->AddSupportedKey(ProjCenterLong);
    pGeocodingCapability->AddSupportedKey(ProjCenterLat);
    pGeocodingCapability->AddSupportedKey(ProjCenterEasting);
    pGeocodingCapability->AddSupportedKey(ProjCenterNorthing);
    pGeocodingCapability->AddSupportedKey(ProjScaleAtNatOrigin);
    pGeocodingCapability->AddSupportedKey(ProjScaleAtCenter);
    pGeocodingCapability->AddSupportedKey(ProjAzimuthAngle);
    pGeocodingCapability->AddSupportedKey(ProjStraightVertPoleLong);
    pGeocodingCapability->AddSupportedKey(ProjRectifiedGridAngle);
    pGeocodingCapability->AddSupportedKey(VerticalCSType);
    pGeocodingCapability->AddSupportedKey(VerticalCitation);
    pGeocodingCapability->AddSupportedKey(VerticalDatum);
    pGeocodingCapability->AddSupportedKey(VerticalUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);
    }

HFC_IMPLEMENT_SINGLETON(HRFMrSIDCreator)

//-----------------------------------------------------------------------------
// Constructor
// Public (HRFMrSIDCreator)
// This is the creator to instantiate MrSID format
//-----------------------------------------------------------------------------
HRFMrSIDCreator::HRFMrSIDCreator()
    : HRFRasterFileCreator(HRFMrSIDFile::CLASS_ID)
    {
    //MrSID capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// GetLabel
// Public (HRFMrSIDCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFMrSIDCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_MrSid()); // MrSID File Format
    }

//-----------------------------------------------------------------------------
// GetSchemes
// Public (HRFMrSIDCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFMrSIDCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// GetExtensions
// Public (HRFMrSIDCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFMrSIDCreator::GetExtensions() const
    {
    return WString(L"*.sid");
    }

//-----------------------------------------------------------------------------
// Create
// Public (HRFMrSIDCreator)
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFMrSIDCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                              HFCAccessMode   pi_AccessMode,
                                              uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFMrSIDFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// IsKindOfFile
// Public (HRFMrSIDCreator)
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFMrSIDCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    
    if (!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    bool IsKindOf        = false;

    LT_STATUS sts = LT_STS_Uninit;

    AString filenameA;
    BeStringUtilities::WCharToCurrentLocaleChar (filenameA, static_cast<HFCURLFile*>(pi_rpURL.GetPtr())->GetAbsoluteFileName().c_str());

    const LTFileSpec FileSpec(filenameA.c_str());

    (const_cast<HRFMrSIDCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    MrSIDImageReader* pImageReader(MrSIDImageReader::create());
    sts = pImageReader->initialize(FileSpec);

    if (LT_SUCCESS(sts))
        {
        // Return true if the raster has a minimal dimension.
        lt_uint32 Width;
        lt_uint32 Height;
        pImageReader->getDimsAtMag( 1.0, Width, Height);

        //Limit the width, to protect against very large images.
        Width = MIN(10, Width);

        const LTIScene scene(0, 0, Width, 1,  1.0);

        LTIPixel pixelProps = pImageReader->getPixelProps();
        LTISceneBuffer bufData(pixelProps, Width, 1, 0);

        IsKindOf = LT_SUCCESS(pImageReader->read(scene, bufData)); // true;
        }

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFMrSIDCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFMrSIDCreator*>(this))->m_pSharingControl = 0;

    pImageReader->release();

    return IsKindOf;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public (HRFMrSIDCreator)
// Create or get the singleton capabilities of MrSID file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFMrSIDCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFMrSIDCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFMrSIDFile::HRFMrSIDFile(const HFCPtr<HFCURL>& pi_rURL,
                           HFCAccessMode         pi_AccessMode,
                           uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    //Read-Only format
    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException( pi_rURL->GetURL());
        }

    // The ancestor store the access mode
    m_IsOpen             = false;

    m_pImageReader          = 0;
    m_pSceneBuffer          = 0;

    m_ResCount              = 0;
    m_pStdViewWidth         = 0;
    m_pStdViewHeight        = 0;
    m_pRatio                = 0;

    if (GetAccessMode().m_HasReadAccess)
        {
        // Create Page and Res Descriptors.
        Open();
        CreateDescriptors();
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFMrSIDFile::HRFMrSIDFile(const HFCPtr<HFCURL>& pi_rURL,
                           HFCAccessMode         pi_AccessMode,
                           uint64_t             pi_Offset,
                           bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen     = false;
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFMrSIDFile::GetWorldIdentificator () const
    {
    HPRECONDITION(CountPages() > 0);

    HGF2DWorldIdentificator World = HGF2DWorld_UNKNOWNWORLD;

    // Check geotiff tags
    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
    RasterFileGeocoding const& fileGeocoding = pPageDescriptor->GetRasterFileGeocoding();

    HCPGeoTiffKeys const& geoKeyContainer = fileGeocoding.GetGeoTiffKeys();

    if (geoKeyContainer.HasKey(GTModelType))
        {
        World = HGF2DWorld_GEOTIFFUNKNOWN;

        // Change world id if GTModelType is ModelTypeGeographic
        uint32_t GeoLongValue;
        geoKeyContainer.GetValue(GTModelType, &GeoLongValue);

        switch (GeoLongValue)
            {
            case TIFFGeo_ModelTypeProjected:
                World = HGF2DWorld_HMRWORLD;
                break;

            case TIFFGeo_ModelTypeGeographic:
                World = HGF2DWorld_GEOGRAPHIC;
                break;

            case TIFFGeo_ModelTypeSpecialMSJ:
            case TIFFGeo_ModelTypeSpecialMSSE:
                World = HGF2DWorld_INTERGRAPHWORLD;
                break;
            }
        }

    return World;
    }

//-----------------------------------------------------------------------------
// Destructor
// Public
// Destroy MrSID file object
//-----------------------------------------------------------------------------
HRFMrSIDFile::~HRFMrSIDFile()
    {
    Close();
    }


//-----------------------------------------------------------------------------
// Close
// Public
// Close the MrSID file
//-----------------------------------------------------------------------------
void HRFMrSIDFile::Close()
    {
    if (m_IsOpen)
        {
        m_IsOpen = false;
        }

    if (m_pFileSpec != 0)
        {
        delete m_pFileSpec;
        m_pFileSpec = 0;
        }


    if (m_pImageReader != 0)
        {
        m_pImageReader->release();
        m_pImageReader = 0;
        }

    if (m_pStdViewWidth != 0)
        {
        delete m_pStdViewWidth;
        m_pStdViewWidth = 0;
        }

    if (m_pStdViewHeight != 0)
        {
        delete m_pStdViewHeight;
        m_pStdViewHeight = 0;
        }

    if (m_pRatio != 0)
        {
        delete m_pRatio;
        m_pRatio = 0;
        }

    if (m_pSceneBuffer != 0)
        {
        delete  m_pSceneBuffer;
        m_pSceneBuffer = 0;
        }

    if (m_pImageRawReader != 0)
        {
        m_pImageRawReader->release();
        m_pImageRawReader = 0;
        }

    TilePool::iterator Itr(m_TilePool.begin());
    while (Itr != m_TilePool.end())
        {
        delete[] Itr->second;
        Itr++;
        }
    m_TilePool.clear();

    }

//-----------------------------------------------------------------------------
// CreateResolutionEditor
// Public
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFMrSIDFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                          unsigned short pi_Resolution,
                                                          HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFMrSIDEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// AddPage
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFMrSIDFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    //Read-only file format
    HASSERT(false);

    return false;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFMrSIDFile::GetCapabilities () const
    {
    return (HRFMrSIDCreator::GetInstance()->GetCapabilities());
    }



//-----------------------------------------------------------------------------
//  This method must be called before all other calls since result is then unpredictable.
//  Application must call this method to set :
//    - A default unit. (By default, default unit is meter.)
//
//    - To impose an interpretation of ProjectedCSType and ProjLinearUnits.
//      Refer to the section concerning interpretation of geo-reference for details.
//      Our implementation exclude ProjLinearUnits if the ProjectedCSType != UserDefined.
//
//    - To impose an interpretation on the unit.
//      Refer to the section concerning interpretation of geo-reference for details.
//      Our implementation will exclude ProjLinearUnits and use INTERGRAPH coordsys only if unit
//      cannot be resolve from other settings and this flag is true.(see TR 97326)
//
//      This method doesn't set the status flag Changed on the TransfoModel in the PageDescriptor.
//
//  @param pi_RatioToMeter       Ratio used to translate the model to meter.
//  @param pi_InterpretUnit      true : The files will be opened using ProjLinearUnits, can
//                                      redefine the unit of ProjectedCSType.
//                               false: Our standard.(default)
//  @param pi_InterpretUnitINTGR true : We consider the file to be in the INTERGRAPH coord sys
//                                      is unit cannot be resolved (see TR 97326).
//                               false: Our standard.(default)
//-----------------------------------------------------------------------------

void HRFMrSIDFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                          uint32_t pi_Page,
                                          bool   pi_CheckSpecificUnitSpec,
                                          bool   pi_InterpretUnitINTGR)
    {
    HPRECONDITION(CountPages() == 1);
    HPRECONDITION(GetPageDescriptor(0) != 0);

    // Update the model in each page
    HFCPtr<HRFPageDescriptor>     pPageDescriptor     = GetPageDescriptor(0);
    GeoCoordinates::BaseGCSCP pBaseGCS = pPageDescriptor->GetGeocodingCP();

    // In addition to the georeference, for some weird reason the fact the original georeference possesed an indication that of
    // the model type is used int he creation of the transformation matrix below
    if (pBaseGCS != nullptr)
        {
        HFCPtr<HGF2DTransfoModel> pTransfoModel;
        bool                     DefaultUnitWasFound = false;

        RasterFileGeocodingCR fileGeocoding = pPageDescriptor->GetRasterFileGeocoding();

        bool hasGTModel(false);
        hasGTModel = fileGeocoding.GetGeoTiffKeys().HasKey(GTModelType);


        // TranfoModel
        BuildTransfoModelMatrix(hasGTModel, pTransfoModel);

        //Ensure that a new GeoCoord is created based on the pi_CheckSpecificUnitSpec
        //configuration.
        pTransfoModel = pPageDescriptor->GetRasterFileGeocoding().TranslateToMeter(pTransfoModel,
                                                                                   pi_RatioToMeter,
                                                                                   pi_CheckSpecificUnitSpec,
                                                                                   &DefaultUnitWasFound);

        SetUnitFoundInFile(DefaultUnitWasFound, 0);

        pPageDescriptor->SetTransfoModel(*pTransfoModel, true);
        pPageDescriptor->SetTransfoModelUnchanged();
        }
    }

//-----------------------------------------------------------------------------
// Open
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFMrSIDFile::Open()
    {
    HPRECONDITION(GetURL()->IsCompatibleWith(HFCURLFile::CLASS_ID));

    bool HasFileBeenOpened = false;

    // Don't try this if a file is already opened!
    if (m_IsOpen == false)
        {
        // Open the MrSid file
        LT_STATUS sts = LT_STS_Uninit;

        AString filenameA;
        BeStringUtilities::WCharToCurrentLocaleChar (filenameA, static_cast<HFCURLFile*>(GetURL().GetPtr())->GetAbsoluteFileName().c_str());

        m_pFileSpec = new LTFileSpec(filenameA.c_str());

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        if ((m_pImageRawReader = MrSIDImageReader::create()) != NULL)
            {
            sts = m_pImageRawReader->initialize(*m_pFileSpec);
            m_pImageReader = 0;

            if (LT_FAILURE(sts) == false)
                {
                LizardTech::LTIImageStage* pImgFilter = NULL; 
                if (CreateFilter_CreatePixelType(m_pImageRawReader, &pImgFilter))   
                    {
                    m_pImageReader = pImgFilter;
                    }
                else
                    {
                    m_pImageReader = m_pImageRawReader;
                    m_pImageRawReader = 0;
                    }

                m_pSceneBuffer     = new LTISceneBuffer(m_pImageReader->getPixelProps(), BLOCK_WIDTH, BLOCK_HEIGHT, 0);
                m_IsOpen = true;
                HasFileBeenOpened = true;

                }
            else
                m_pImageRawReader->release();
            }
        }

    return HasFileBeenOpened;
    }


//-----------------------------------------------------------------------------
// public
// SetLookAhead
//
// Sets the LookAhead for a shape
//-----------------------------------------------------------------------------
void HRFMrSIDFile::SetLookAhead(uint32_t               pi_Page,
                                unsigned short        pi_Resolution,
                                const HVEShape&        pi_rShape,
                                uint32_t               pi_ConsumerID,
                                bool                  pi_Async)
    {
    HPRECONDITION(pi_Page == 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(0)->CountResolutions());

    HFCMonitor Monitor(m_TilePoolKey);
    TilePool::iterator Itr(m_TilePool.begin());
    while (Itr != m_TilePool.end())
        {
        delete[] Itr->second;
        Itr++;
        }
    m_TilePool.clear();

    if (!pi_rShape.IsEmpty())
        {
        // Extract the needed blocks from the region

        HFCPtr<HRFResolutionDescriptor> pResDesc = GetPageDescriptor(0)->GetResolutionDescriptor(pi_Resolution);

        uint32_t BlockWidth  = pResDesc->GetBlockWidth();
        uint32_t BlockHeight = pResDesc->GetBlockHeight();

        HGFTileIDDescriptor TileDesc(pResDesc->GetWidth(),
                                     pResDesc->GetHeight(),
                                     BlockWidth,
                                     BlockHeight);

        uint32_t MinX = ULONG_MAX;
        uint32_t MinY = ULONG_MAX;
        uint32_t MaxX = 0;
        uint32_t MaxY = 0;

        // Make sure the input shape will not be destroyed
        HVEShape* pShape = const_cast<HVEShape*>(&pi_rShape);
        pShape->IncrementRef();
            {
            HVETileIDIterator TileIterator(&TileDesc, HFCPtr<HVEShape>(pShape));

            uint64_t PosX;
            uint64_t PosY;
            uint64_t BlockIndex = TileIterator.GetFirstTileIndex();
            while (BlockIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
                {
                TileDesc.GetPositionFromIndex(BlockIndex, &PosX, &PosY);
                HASSERT(PosX <= ULONG_MAX);
                HASSERT(PosY <= ULONG_MAX);

                MinX = MIN(MinX, (uint32_t)PosX);
                MinY = MIN(MinY, (uint32_t)PosY);
                MaxX = MAX(MaxX, (uint32_t)PosX + BlockWidth);
                MaxY = MAX(MaxY, (uint32_t)PosY + BlockHeight);

                BlockIndex = TileIterator.GetNextTileIndex();
                }
            }
        // Decrement after the block because the HVETileIDIterator keeps an
        // HFCPtr to the input shape, so we want the Iterator to be destroyed
        // before we decrement.
        pShape->DecrementRef();

        uint32_t Width = MaxX - MinX;
        uint32_t Height = MaxY - MinY;
        HArrayAutoPtr<Byte> pData;

        uint32_t BytesPerPixel = m_pImageReader->getPixelProps().getNumBytes();
        pData = new Byte[Width * Height * BytesPerPixel];

        if (pData != 0)
            {
            try
                {
                LT_STATUS sts = LT_STS_Uninit;

                // Lock the file.
                HFCLockMonitor SisterFileLock (GetLockManager());

                delete m_pSceneBuffer;
                m_pSceneBuffer = new LTISceneBuffer(m_pImageReader->getPixelProps(), Width, Height, 0);

                const LTIScene scene(MinX, MinY, Width, Height,  m_pRatio[pi_Resolution]);
                sts = m_pImageReader->read(scene, *m_pSceneBuffer);

                if (LT_SUCCESS(sts))
                    {
                    sts = m_pSceneBuffer->exportData(pData,
                                                     m_pImageReader->getNumBands(),
                                                     Width * m_pImageReader->getNumBands(),
                                                     1);
                    if (LT_SUCCESS(sts))
                        {
                        uint32_t NbXTile = Width / BlockWidth;
                        uint32_t NbYTile = Height / BlockHeight;
                        uint32_t BytesPerBlockWidth = pResDesc->GetBytesPerBlockWidth();
                        uint32_t SrcWidth = Width * m_pImageReader->getPixelProps().getNumBytes();
                        Byte* pSrc;
                        Byte* pDst;
                        uint32_t BlockX;
                        uint32_t PosX;
                        uint32_t BlockY;
                        uint32_t PosY;
                        HArrayAutoPtr<Byte> pBlockData;
                        for (BlockX = 0, PosX = MinX; BlockX < NbXTile; ++BlockX, PosX += BlockWidth)
                            {
                            for (BlockY = 0, PosY = MinY; BlockY < NbYTile; ++BlockY, PosY += BlockHeight)
                                {
                                pBlockData = new Byte[pResDesc->GetBlockSizeInBytes()];
                                pDst = pBlockData.get();
                                pSrc = pData.get() + BlockY * BlockHeight * SrcWidth;
                                pSrc += BlockX * BytesPerBlockWidth;

                                for (uint32_t Line = 0; Line < BlockHeight; ++Line)
                                    {
                                    memcpy(pDst, pSrc, BytesPerBlockWidth);
                                    pDst += BytesPerBlockWidth;
                                    pSrc += SrcWidth;
                                    }
                                pair<TilePool::iterator, bool> InsertPair(m_TilePool.insert(TilePool::value_type(TileDesc.ComputeID(PosX, PosY, pi_Resolution),
                                                                                            pBlockData.release())));

                                HASSERT(InsertPair.second);
                                }
                            }
                        }
                    }
                }
            catch (...)
                {
                m_TilePool.clear();
                _ASSERT(false);
                }
            }
        }
    }


//-----------------------------------------------------------------------------
// public
// setLookAhead
//
// Sets the LookAhead for a list of blocks
//-----------------------------------------------------------------------------
void HRFMrSIDFile::SetLookAhead(uint32_t               pi_Page,
                                const HGFTileIDList&   pi_rBlocks,
                                uint32_t               pi_ConsumerID,
                                bool                  pi_Async)
    {
    HASSERT(0);
    }


//-----------------------------------------------------------------------------
// CreateDescriptors
// Protected
// Create Bmp File Descriptors
//-----------------------------------------------------------------------------
void HRFMrSIDFile::CreateDescriptors ()
    {
    lt_uint32 Width;
    lt_uint32 Height;
    unsigned int ResCount = 0;

    // Get pixel type
    HRPPixelType* pRawPixelType;
    CreateFilter_CreatePixelType(m_pImageReader, 0, &pRawPixelType);
    HFCPtr<HRPPixelType>  pPixelType(pRawPixelType);

    // Calculate resolution count
    m_pImageReader->getDimsAtMag (1.0, Width, Height);

    int ZoomRatio = (int)(1.0 / m_pImageReader->getMinMagnification());

    do
        {
        ZoomRatio >>= 1;
        ++ResCount;

        }
    while (ZoomRatio > 1);

    ResCount = MIN(ResCount, 254);  // don't use 255, 255 means Unlimited resolution.
    m_ResCount = ResCount;

    // Allocate memory for ReadBlock() utilities (optimization)
    m_pStdViewWidth  = new lt_uint32[m_ResCount];
    m_pStdViewHeight = new lt_uint32[m_ResCount];
    m_pRatio         = new double[m_ResCount];

    // Fill resolution descriptors
    unsigned int Denominator = 1;
    LT_STATUS    sts = LT_STS_Uninit;

    HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;

    HFCPtr<HRFResolutionDescriptor> pResolutionDesc;

    for(unsigned int Count=0; Count < m_ResCount; ++Count)
        {
        m_pRatio[Count]                  = 1.0 / (double)(Denominator);
        sts = m_pImageReader->getDimsAtMag (m_pRatio[Count], Width, Height);

        if (LT_SUCCESS(sts))
            {
            m_pStdViewWidth[Count]          = Width;
            m_pStdViewHeight[Count]         = Height;

            Denominator <<= 1;

            // Create Resolution Descriptor
            pResolutionDesc =  new HRFResolutionDescriptor(GetAccessMode(),            // AccessMode,
                                                           GetCapabilities(),             // Capabilities,
                                                           m_pRatio[Count],               // XResolutionRatio,
                                                           m_pRatio[Count],               // YResolutionRatio,
                                                           pPixelType,                    // PixelType,
                                                           new HCDCodecIdentity(),        // Codec,
                                                           HRFBlockAccess::RANDOM,        // RBlockAccess,
                                                           HRFBlockAccess::RANDOM,        // WBlockAccess,
                                                           HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
                                                           HRFInterleaveType::PIXEL,      // InterleaveType
                                                           0,                             // IsInterlace,
                                                           Width,                         // Width,
                                                           Height,                        // Height,
                                                           BLOCK_WIDTH,                   // BlockWidth,
                                                           BLOCK_HEIGHT                  // BlockHeight,
                                                          );

            ListOfResolutionDescriptor.push_back(pResolutionDesc);
            }
        else
            {
            m_ResCount = Count - 1;

            _ASSERT(false);
            break;
            }
        }

    HPMAttributeSet               TagList;

    
    // Geokeys are used as part of MrSID files (but apparently not always)
    RasterFileGeocodingPtr pFileGeocoding;
    GetFileInfo(TagList, pFileGeocoding);

    HFCPtr<HGF2DTransfoModel> pTransfoModel;
    bool                     DefaultUnitWasFound = false;

    bool hasGTModel(false);
    hasGTModel = pFileGeocoding->GetGeoTiffKeys().HasKey(GTModelType);

    BuildTransfoModelMatrix(hasGTModel, pTransfoModel);

    if ((pFileGeocoding->GetGeocodingCP() != 0) && (pFileGeocoding->GetGeocodingCP()->IsValid()))
        {
        // If file is a kind of "geotiff" then translate transfo model
        pTransfoModel = pFileGeocoding->TranslateToMeter(pTransfoModel,
                                                       1.0,
                                                       false,
                                                       &DefaultUnitWasFound);
        }

    SetUnitFoundInFile(DefaultUnitWasFound);

    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor (GetAccessMode(),
                                   GetCapabilities(),           // Capabilities,
                                   ListOfResolutionDescriptor,  // ResolutionDescriptor,
                                   0,                           // RepresentativePalette,
                                   0,                             // Histogram,
                                   0,                           // Thumbnail,
                                   0,                             // ClipShape,
                                   pTransfoModel,               // TransfoModel,
                                   0,                           // Filters
                                   &TagList,                    // Tag
                                   0);                          // Duration

    pPage->InitFromRasterFileGeocoding(*pFileGeocoding);

    m_ListOfPageDescriptor.push_back(pPage);
    }


//-----------------------------------------------------------------------------
// GetTagInfo
// Private
// Get a list of relevant tags found embedded in the file.
//-----------------------------------------------------------------------------
void HRFMrSIDFile::GetFileInfo(HPMAttributeSet&               po_rTagList,
                               RasterFileGeocodingPtr&        po_pFileGeocoding)
    {
    po_pFileGeocoding = RasterFileGeocoding::Create();

    HFCPtr<HCPGeoTiffKeys> po_rpGeoTiffKeys = new HCPGeoTiffKeys();

    try
        {
        // Tag information
        HFCPtr<HPMGenericAttribute> pTag;
        LTIMetadataDatabase         MyMetaDataReader = m_pImageReader->getMetadata();

        if (MyMetaDataReader.getIndexCount())
            {
            const LTIMetadataRecord* pMetaRecord = 0;
            const void*              pData = 0;
            const void**             ppData=0;
            lt_uint32                NumDims=0;
            const lt_uint32*         pDims = 0;

            // Geotiff info
            if (MyMetaDataReader.has("GEOTIFF_NUM::1024::GTModelTypeGeoKey"))
                {
                MyMetaDataReader.get("GEOTIFF_NUM::1024::GTModelTypeGeoKey", pMetaRecord);
                if (pMetaRecord)
                    {
                    HASSERT(pMetaRecord->getDataType() == LTI_METADATA_DATATYPE_UINT16);
                    pData = pMetaRecord->getScalarData();

                    po_rpGeoTiffKeys->AddKey(GTModelType, (uint32_t)*((unsigned short*)pData));
                    }
                }

            if (MyMetaDataReader.has("GEOTIFF_NUM::3072::ProjectedCSTypeGeoKey")) // unsigned short
                {
                MyMetaDataReader.get("GEOTIFF_NUM::3072::ProjectedCSTypeGeoKey", pMetaRecord);
                if (pMetaRecord)
                    {
                    HASSERT(pMetaRecord->getDataType() == LTI_METADATA_DATATYPE_UINT16);
                    pData = pMetaRecord->getScalarData();

                    po_rpGeoTiffKeys->AddKey(ProjectedCSType, (uint32_t)*((unsigned short*)pData));
                    }
                }

            if (MyMetaDataReader.has("GEOTIFF_NUM::3073::PCSCitationGeoKey")) // unsigned short
                {
                MyMetaDataReader.get("GEOTIFF_NUM::3073::PCSCitationGeoKey", pMetaRecord);
                if (pMetaRecord)
                    {
                    HASSERT(pMetaRecord->getDataType() == LTI_METADATA_DATATYPE_ASCII);
                    ppData = (const void**)pMetaRecord->getArrayData(NumDims, pDims);

                    po_rpGeoTiffKeys->AddKey(PCSCitation,(char*)*ppData);
                    }
                }

            if (MyMetaDataReader.has("GEOTIFF_NUM::3074::ProjectionGeoKey")) // unsigned short
                {
                MyMetaDataReader.get("GEOTIFF_NUM::3074::ProjectionGeoKey", pMetaRecord);
                if (pMetaRecord)
                    {
                    HASSERT(pMetaRecord->getDataType() == LTI_METADATA_DATATYPE_UINT16);
                    pData = pMetaRecord->getScalarData();

                    po_rpGeoTiffKeys->AddKey(Projection, (uint32_t)*((unsigned short*)pData));
                    }
                }

            if (MyMetaDataReader.has("GEOTIFF_NUM::4099::VerticalUnitsGeoKey")) // unsigned short
                {
                MyMetaDataReader.get("GEOTIFF_NUM::4099::VerticalUnitsGeoKey", pMetaRecord);
                if (pMetaRecord)
                    {
                    HASSERT(pMetaRecord->getDataType() == LTI_METADATA_DATATYPE_UINT16);
                    pData = pMetaRecord->getScalarData();

                    po_rpGeoTiffKeys->AddKey(VerticalUnits, (uint32_t)*((unsigned short*)pData));
                    }
                }

            if (MyMetaDataReader.has("GEOTIFF_NUM::2053::GeogLinearUnitSizeGeoKey"))
                {
                MyMetaDataReader.get("GEOTIFF_NUM::2053::GeogLinearUnitSizeGeoKey", pMetaRecord);
                if (pMetaRecord)
                    {
                    HASSERT(pMetaRecord->getDataType() == LTI_METADATA_DATATYPE_FLOAT64);
                    pData = pMetaRecord->getScalarData();

                    po_rpGeoTiffKeys->AddKey(GeogLinearUnitSize, *((double*)pData));
                    }
                }

            if (MyMetaDataReader.has("GEOTIFF_NUM::2054::GeogAngularUnitsGeoKey"))
                {
                MyMetaDataReader.get("GEOTIFF_NUM::2054::GeogAngularUnitsGeoKey", pMetaRecord);
                if (pMetaRecord)
                    {
                    HASSERT(pMetaRecord->getDataType() == LTI_METADATA_DATATYPE_UINT16);
                    pData = pMetaRecord->getScalarData();

                    po_rpGeoTiffKeys->AddKey(GeogAngularUnits, (uint32_t)*((unsigned short*)pData));
                    }
                }

            if (MyMetaDataReader.has("GEOTIFF_NUM::3076::ProjLinearUnitsGeoKey"))
                {
                MyMetaDataReader.get("GEOTIFF_NUM::3076::ProjLinearUnitsGeoKey", pMetaRecord);
                if (pMetaRecord)
                    {
                    HASSERT(pMetaRecord->getDataType() == LTI_METADATA_DATATYPE_UINT16);
                    pData = pMetaRecord->getScalarData();

                    po_rpGeoTiffKeys->AddKey(ProjLinearUnits, (uint32_t)*((unsigned short*)pData));
                    }
                }

            if (MyMetaDataReader.has("GEOTIFF_NUM::3077::ProjLinearUnitSizeGeoKey")) // unsigned short
                {
                MyMetaDataReader.get("GEOTIFF_NUM::3077::ProjLinearUnitSizeGeoKey", pMetaRecord);
                if (pMetaRecord)
                    {
                    HASSERT(pMetaRecord->getDataType() == LTI_METADATA_DATATYPE_FLOAT64);
                    pData = pMetaRecord->getScalarData();

                    po_rpGeoTiffKeys->AddKey(ProjLinearUnitSize, *((double*)pData));
                    }
                }

            // The original(s) file(s) name(s) from where the MrSid file has been created
            if (MyMetaDataReader.has("IMAGE::INPUT_NAME"))
                {
                MyMetaDataReader.get("IMAGE::INPUT_NAME", pMetaRecord);
                if (pMetaRecord)
                    {
                    HASSERT(pMetaRecord->getDataType() == LTI_METADATA_DATATYPE_ASCII);
                    ppData = (const void**)pMetaRecord->getArrayData(NumDims, pDims);
                    WString Name((char*)*ppData,false);
                    pTag = new HRFAttributeDocumentName(Name);
                    po_rTagList.Set(pTag);
                    }
                }

            // Date and Time of the creation of the MrSid file
            if (MyMetaDataReader.has("IMAGE::CREATION_DATE"))
                {
                MyMetaDataReader.get("IMAGE::CREATION_DATE", pMetaRecord);
                if (pMetaRecord)
                    {
                    HASSERT(pMetaRecord->getDataType() == LTI_METADATA_DATATYPE_ASCII);
                    ppData = (const void**)pMetaRecord->getArrayData(NumDims, pDims);
                    WString Name((char*)*ppData,false);
                    pTag = new HRFAttributeDateTime(Name);
                    po_rTagList.Set(pTag);
                    }
                }

            // Try creating the base GCS with geokeys
            po_pFileGeocoding = RasterFileGeocoding::Create(po_rpGeoTiffKeys.GetPtr());

            WString WKT;

            // TR 239138 - If a WKT is found in the file and a BaseGCS object cannot
            // be created with the GeoTIFF keys, try to create a BaseGCS object with the WKT
            // instead
            if (/*&&AR GCS review GCSServices->_IsAvailable() &&*/ 
                MyMetaDataReader.has("IMAGE::WKT"))
                {
                MyMetaDataReader.get("IMAGE::WKT", pMetaRecord);
                if (pMetaRecord)
                    {
                    HASSERT(pMetaRecord->getDataType() == LTI_METADATA_DATATYPE_ASCII);
                    ppData = (const void**)pMetaRecord->getArrayData(NumDims, pDims);

                    BeStringUtilities::CurrentLocaleCharToWChar( WKT,(char*)*ppData);
                    }

                if ((WKT != L"") && (po_rpGeoTiffKeys->GetNbKeys() > 0))
                    {
                    if (po_pFileGeocoding->GetGeocodingCP() == NULL || !(po_pFileGeocoding->GetGeocodingCP()->IsValid()))
                        {
                        //If a basegeocoord cannot be created with the GeoTIFF tags found
                        //and there is a WKT string, try with the WKT string

                        GeoCoordinates::BaseGCSPtr pBaseGcs = GeoCoordinates::BaseGCS::CreateGCS();
                        if(SUCCESS == pBaseGcs->InitFromWellKnownText (NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, WKT.c_str()))
                            po_pFileGeocoding = RasterFileGeocoding::Create(pBaseGcs.get()); 
                        }
                    }
                }
            }
        }
    catch(...)
        {
        //TR 208598 Sometime (randomly), the string array containing the CREATION_DATE
        //have unintelligible characters, for images that hasn't such tag according to mrsidinfo.exe.
        }
    }

//-----------------------------------------------------------------------------
// BuildTransfoModelMatrix
// Protected
// This method build the transformation matrix.
//-----------------------------------------------------------------------------
void HRFMrSIDFile::BuildTransfoModelMatrix(bool pi_HasModelType, HFCPtr<HGF2DTransfoModel>& po_prTranfoModel)
    {
    // isGeoCoordImplicit : true if and only if the geo information is not "real"
    if (!m_pImageReader->isGeoCoordImplicit())
        {
        const LTIGeoCoord& geo = m_pImageReader->getGeoCoord();

        double xu;
        double yu;
        double xres;
        double yres;
        double xrot;
        double yrot;

        // Get the data for the matrix.
        xu   = geo.getX();
        yu   = geo.getY();
        xres = geo.getXRes();
        yres = geo.getYRes();
        xrot = geo.getXRot();
        yrot = geo.getYRot();

        // SebG TR# TR 102989
        // Validate the matrix here...
        if (HDOUBLE_EQUAL_EPSILON(xres, 0.0)  || HDOUBLE_EQUAL_EPSILON(yres, 0.0))
            {
            // The scale factor is not valid, we can't know if other values
            // could be trust, change them all to "Identity".
            po_prTranfoModel = new HGF2DIdentity();
            }
        else
            {
            HFCMatrix<3, 3> TransfoMatrix;

            // Initialize the matrix.
            TransfoMatrix[0][0] = xres;
            TransfoMatrix[0][1] = yrot;
            TransfoMatrix[0][2] = xu;
            TransfoMatrix[1][0] = xrot;

            if (pi_HasModelType == true)
                {
                // TR #118017
                // GT TR# 81024
                TransfoMatrix[1][1] = yres;
                TransfoMatrix[1][2] = yu;
                }
            else
                {
                // SebG TR# 103717
                TransfoMatrix[1][1] = -yres;
                TransfoMatrix[1][2] = yu * -1;
                }
            TransfoMatrix[2][0] = 0.0;
            TransfoMatrix[2][1] = 0.0;
            TransfoMatrix[2][2] = 1.0;

            po_prTranfoModel = new HGF2DProjective(TransfoMatrix);

            // Get the simplest model possible.
            po_prTranfoModel = po_prTranfoModel->CreateSimplifiedModel();
            }
        }
    else
        {
        po_prTranfoModel = new HGF2DIdentity();
        }
    }

//-----------------------------------------------------------------------------
// Save
// Private
// This method saves the file.
//-----------------------------------------------------------------------------
void HRFMrSIDFile::Save()
    {
    //Read only file
    }


//-----------------------------------------------------------------------------
// CreateFilter_CreatePixelType
// Private
// This method returns true if a filter was requested and created to normalize pixeltype.
//     po_ImageFilter != null  --> Filter requested if needed
//
// Return pixeltype if requested, 
//     po_PixelType != null --> Return the corresponding Pixeltype. normally, Gary8 or RGB24) 
//-----------------------------------------------------------------------------
bool HRFMrSIDFile::CreateFilter_CreatePixelType(LizardTech::LTIImageStage* pi_pImage, LizardTech::LTIImageStage** po_ImageFilter, HRPPixelType** po_PixelType)
    {
    bool FilterRequiered = true;
    bool FilterCreated = false;

    LTIDataType datatype = pi_pImage->getDataType();
    switch (pi_pImage->getColorSpace())
        {
        case LTI_COLORSPACE_GRAYSCALE:
            if (datatype == LTI_DATATYPE_UINT8)
                {
                FilterRequiered = false;    

                if (po_PixelType != 0)
                    *po_PixelType = new HRPPixelTypeV8Gray8();
                }
            break;

        case LTI_COLORSPACE_RGB:
            switch(datatype)
                {
                case LTI_DATATYPE_UINT8:
                    FilterRequiered = false;    

                    if (po_PixelType != 0)
                        *po_PixelType = new HRPPixelTypeV24R8G8B8();
                    break;

                case LTI_DATATYPE_UINT16:
                    {
                    // Returns filter if requested.
                    if (po_ImageFilter != 0)
                        {
                        LTIDynamicRangeFilter* pImgFilter = LTIDynamicRangeFilter::create();
                        LTIPixel MinPixel(LTI_COLORSPACE_RGB, 3, LTI_DATATYPE_UINT16);
                        MinPixel.setSampleValuesToMin();
                        LTIPixel MaxPixel(LTI_COLORSPACE_RGB, 3, LTI_DATATYPE_UINT16);
                        MaxPixel.setSampleValuesUint16(256);
                        // Destination pixeltype 8bit --> Gray or RGB
                        if (LT_FAILURE(pImgFilter->initialize(m_pImageRawReader, &MinPixel, &MaxPixel, LTI_DATATYPE_UINT8)))
                            pImgFilter->release();   
                        else
                            {
                            *po_ImageFilter = pImgFilter;
                            FilterCreated = true;
                            }
                        }
                    }
                    if (po_PixelType != 0)
                        *po_PixelType = new HRPPixelTypeV24R8G8B8();
                    break;

                default:
                    HASSERT(0);
                    break;
                }
            break;


        case LTI_COLORSPACE_GRAYSCALEA:
            HASSERT(0);
            break;

        case LTI_COLORSPACE_GRAYSCALEA_PM:
            HASSERT(0);
            break;
            
        case LTI_COLORSPACE_RGBA:
            HASSERT(0);
            break;

        case LTI_COLORSPACE_RGBA_PM:
            HASSERT(0);
            break;

        default:
            HASSERT(0);
            break;
        }

    // Default filter required --> RGB 24 or Gray 8
    if (FilterRequiered && !FilterCreated)
        {
        // The LTIViewerImageFilter automatically adjust the pixel type to a R8G8B8 or Gray8 and
        // scaled the pixel values according to the dynamic range meta data in the image (if any).
        LTIViewerImageFilter* pImgFilter = LTIViewerImageFilter::create();
        if (LT_FAILURE(pImgFilter->initialize(pi_pImage, true, false)))
            pImgFilter->release();   
        else
            {
            if (pImgFilter->getColorSpace() == LTI_COLORSPACE_GRAYSCALE)
                {
                if (po_PixelType != 0)
                    *po_PixelType = new HRPPixelTypeV8Gray8();
                }
            else
                {
                if (po_PixelType != 0)
                    *po_PixelType = new HRPPixelTypeV24R8G8B8();
                }

            // Returns filter if requested.
            if (po_ImageFilter != 0)
                {
                *po_ImageFilter = pImgFilter;
                FilterCreated = true;
                }
            else
                pImgFilter->release();  
            }
        }

    return FilterCreated;
    }




//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
/*
void* MrSidFileImpl::ExtraMetaItem(const void* pData, int idx, LTIMetadataDataType type)
{
    void* pExtractedData = 0;

    switch (type)
    {
    case LTI_METADATA_DATATYPE_UINT8:
        {
            return (lt_uint32)((lt_uint8*)data)[idx];
        }
        break;

    case LTI_METADATA_DATATYPE_SINT8:
        {
            return (lt_int32)((lt_int8*)data)[idx];
        }
        break;

    case LTI_METADATA_DATATYPE_UINT16:
        {
            return (lt_uint32)((lt_uint16*)data)[idx];
        }
        break;

    case LTI_METADATA_DATATYPE_SINT16:
        {
            return (lt_int32)((lt_int16*)data)[idx];
        }
        break;

    case LTI_METADATA_DATATYPE_UINT32:
        {
            return (lt_uint32*)data)[idx];
        }
        break;
    case LTI_METADATA_DATATYPE_SINT32:
        {
            return (lt_int32*)data)[idx]
        }
        break;
    case LTI_METADATA_DATATYPE_UINT64:
        {
            return ((lt_uint64*)data)[idx];
        }
        break;
    case LTI_METADATA_DATATYPE_SINT64:
        {
            return ((lt_int64*)data)[idx];
        }
        break;
    case LTI_METADATA_DATATYPE_FLOAT32:
        {
            return ((float*)data)[idx];
        }
        break;
    case LTI_METADATA_DATATYPE_FLOAT64:
        {
            return ((double*)data)[idx];
        }
        break;
    case LTI_METADATA_DATATYPE_ASCII:
        {
            ((const char**)data)[idx];
        }
        break;
    }
}*/

bool HRFMrSIDFile::HasLookAheadByExtent(uint32_t pi_Page) const
    {
    return true;
    }


bool HRFMrSIDFile::CanPerformLookAhead(uint32_t pi_Page) const
    {
    return true;
    }


void HRFMrSIDFile::StopLookAhead(uint32_t pi_Page,
                                        uint32_t pi_ConsumerID)
    {
    // the LookAhead is not threaded, do nothing
    }

#endif // IPP_HAVE_MRSID_SUPPORT
