//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFUSgsDEMFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFUSgsDEMFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLFile.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>

#include <Imagepp/all/h/HRFUSgsDEMEditor.h>
#include <Imagepp/all/h/HRFUSgsDEMFile.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//GDAL
#include <ImagePP-GdalLib/gdal_priv.h>
#include <ImagePP-GdalLib/cpl_string.h>



//-----------------------------------------------------------------------------
// HRFUSgsDEMBlockCapabilities
//-----------------------------------------------------------------------------
class HRFUSgsDEMBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFUSgsDEMBlockCapabilities()
        : HRFRasterFileCapabilities()
        {

        // Image Capability
        Add(new HRFImageCapability(HFC_READ_ONLY,          // AccessMode
                                   LONG_MAX,               // MaxSizeInBytes
                                   0,                      // MinWidth
                                   LONG_MAX,               // MaxWidth
                                   0,                      // MinHeight
                                   LONG_MAX));             // MaxHeight

        }
    };

//-----------------------------------------------------------------------------
// HRFUSgsDEMCodecCapabilities
//-----------------------------------------------------------------------------
class HRFUSgsDEMCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFUSgsDEMCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_ONLY,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFUSgsDEMBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFUSgsDEMCapabilities
//-----------------------------------------------------------------------------
HRFUSgsDEMCapabilities::HRFUSgsDEMCapabilities()
    : HRFRasterFileCapabilities()
    {
    Add(new HRFPixelTypeCapability( HFC_READ_ONLY,
                                    HRPPixelTypeV16Int16::CLASS_ID,
                                    new HRFUSgsDEMCodecCapabilities()));

    Add(new HRFPixelTypeCapability( HFC_READ_ONLY,
                                    HRPPixelTypeV32Float32::CLASS_ID,
                                    new HRFUSgsDEMCodecCapabilities()));

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // Still Image Capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMinSampleValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMaxSampleValue));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(PCSCitation);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSTypeLong);
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

HFC_IMPLEMENT_SINGLETON(HRFUSgsDEMCreator)

//-----------------------------------------------------------------------------
// HRFUSgsDEMCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFUSgsDEMCreator::HRFUSgsDEMCreator()
    : HRFRasterFileCreator(HRFUSgsDEMFile::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    }

// Identification information
WString HRFUSgsDEMCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_USGS_ASCII()); // USGS DEM File Format
    }

// Identification information
WString HRFUSgsDEMCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

// Identification information
WString HRFUSgsDEMCreator::GetExtensions() const
    {
    return WString(L"*.dem");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFUSgsDEMCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                HFCAccessMode         pi_AccessMode,
                                                uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFUSgsDEMFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFUSgsDEMCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                      uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    //Will initialize GDal if not already initialize
    HRFGdalSupportedFile::Initialize();

    bool                       Result = true;
    HAutoPtr<HFCBinStream>      pFile;
    HArrayAutoPtr<char>        pLine(new char[6]);

    (const_cast<HRFUSgsDEMCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Open the IMG File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        pFile->SeekToPos(156);
        pFile->Read(pLine, 6);

        //Check that the code defining the ground planimetric
        //reference system is valid.
        if ((strncmp(pLine, "     0", 6) != 0) &&
            (strncmp(pLine, "     1", 6) != 0) &&
            (strncmp(pLine, "     2", 6) != 0) &&
            (strncmp(pLine, "     3", 6) != 0))
            {
            Result = false;
            }

        //Check that the code defining the elevation pattern is valid.
        if (Result == true)
            {
            pFile->SeekToPos(150);
            pFile->Read(pLine, 6);

            if ((strncmp(pLine, "     1", 6) != 0))
                {
                Result = false;
                }
            }
        }
    else
        {
        Result = false;
        }

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFUSgsDEMCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFUSgsDEMCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of  File
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFUSgsDEMCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFUSgsDEMCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFUSgsDEMFile::HRFUSgsDEMFile(const HFCPtr<HFCURL>& pi_rURL,
                                      HFCAccessMode         pi_AccessMode,
                                      uint64_t             pi_Offset)

    : HRFGdalSupportedFile("USGSDEM", pi_rURL, pi_AccessMode, pi_Offset)
    {
    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }

    // The ancestor store the access mode
    m_IsOpen      = false;
    m_GTModelType = TIFFGeo_Undefined;

    //Open the file. An exception should be thrown if the open failed.
    Open();

    HASSERT(m_IsOpen == true);
    // Create Page and Res Descriptors.
    CreateDescriptors();
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// Create the resolution editor
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFUSgsDEMFile::CreateResolutionEditor(uint32_t      pi_Page,
                                                                   unsigned short pi_Resolution,
                                                                   HFCAccessMode pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFUSgsDEMEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFUSgsDEMFile::~HRFUSgsDEMFile()
    {
    //All the work of closing the file and GDAL is done in the ancestor class.
    }

//-----------------------------------------------------------------------------
// Protected
// BuildTransfoModel
//
//-----------------------------------------------------------------------------
RasterFileGeocodingPtr HRFUSgsDEMFile::ExtractGeocodingInformation()
    {
    HPRECONDITION(m_NbBands == 1);

    RasterFileGeocodingPtr pGeocoding = HRFGdalSupportedFile::ExtractGeocodingInformation();
    if (pGeocoding->GetGeocodingCP() != NULL)
        {
        // Need to clone GCS to modify the vertical unit.
        GeoCoordinates::BaseGCSPtr pNewGcs = GeoCoordinates::BaseGCS::CreateGCS(*pGeocoding->GetGeocodingCP());
        AddVerticalUnitToGeocoding(*pNewGcs);

        pGeocoding = RasterFileGeocoding::Create(pNewGcs.get());
        }
    
    return pGeocoding;
    }

//-----------------------------------------------------------------------------
// Protected
// HandleNoDisplayBands
//
//-----------------------------------------------------------------------------
void HRFUSgsDEMFile::HandleNoDisplayBands()
    {
    //This format is not multiband
    HASSERT(m_NbBands == 1);

    m_GrayBandInd = 1;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFUSgsDEMFile::CreateDescriptors()
    {
    HPRECONDITION(GetDataSet() != 0);

    // Create Page and resolution Description/Capabilities for this file.
    HPMAttributeSet                         TagList;

    HRFGdalSupportedFile::CreateDescriptorsWith(new HCDCodecIdentity(), TagList);
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFUSgsDEMFile::Save()
    {
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFUSgsDEMFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    //Read only file format
    HASSERT(false);
    return false;
    }


//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFUSgsDEMFile::GetCapabilities () const
    {
    return (HRFUSgsDEMCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Protected
// GetScanLineOrientation
//-----------------------------------------------------------------------------
HRFScanlineOrientation HRFUSgsDEMFile::GetScanLineOrientation()const
    {
    return HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
    }

//-----------------------------------------------------------------------------
// Protected
// GetBandRole
// Get the type of information (elevation, color, temperature, etc...) of
// the requested band.
//-----------------------------------------------------------------------------
HRPChannelType::ChannelRole HRFUSgsDEMFile::GetBandRole(int32_t pi_RasterBand) const
    {
    HPRECONDITION(m_GrayBandInd == pi_RasterBand);

    return HRPChannelType::ELEVATION;
    }
