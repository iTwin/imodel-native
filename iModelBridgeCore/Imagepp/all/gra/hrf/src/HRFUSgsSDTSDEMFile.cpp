//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFUSgsSDTSDEMFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFUSgsSDTSDEMFile
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

#include <Imagepp/all/h/HRFUSgsSDTSDEMEditor.h>
#include <Imagepp/all/h/HRFUSgsSDTSDEMFile.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//GDAL
#include <ImagePP-GdalLib/gdal_priv.h>
#include <ImagePP-GdalLib/cpl_string.h>





//-----------------------------------------------------------------------------
// HRFUSgsSDTSDEMBlockCapabilities
//-----------------------------------------------------------------------------
class HRFUSgsSDTSDEMBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFUSgsSDTSDEMBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add (new HRFLineCapability (HFC_READ_ONLY,
                                    ULONG_MAX,
                                    HRFBlockAccess::RANDOM));
        }
    };

//-----------------------------------------------------------------------------
// HRFUSgsSDTSDEMCodecCapabilities
//-----------------------------------------------------------------------------
class HRFUSgsSDTSDEMCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFUSgsSDTSDEMCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_ONLY,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFUSgsSDTSDEMBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFUSgsSDTSDEMCapabilities
//-----------------------------------------------------------------------------
HRFUSgsSDTSDEMCapabilities::HRFUSgsSDTSDEMCapabilities()
    : HRFRasterFileCapabilities()
    {
    Add(new HRFPixelTypeCapability( HFC_READ_ONLY,
                                    HRPPixelTypeV16Int16::CLASS_ID,
                                    new HRFUSgsSDTSDEMCodecCapabilities()));

    Add(new HRFPixelTypeCapability( HFC_READ_ONLY,
                                    HRPPixelTypeV32Float32::CLASS_ID,
                                    new HRFUSgsSDTSDEMCodecCapabilities()));

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
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeBackground(0)));
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

    // Tag capability

    }

HFC_IMPLEMENT_SINGLETON(HRFUSgsSDTSDEMCreator)

//-----------------------------------------------------------------------------
// HRFUSgsSDTSDEMCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFUSgsSDTSDEMCreator::HRFUSgsSDTSDEMCreator()
    : HRFRasterFileCreator(HRFUSgsSDTSDEMFile::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    }

// Identification information
WString HRFUSgsSDTSDEMCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_USGS_SDTS()); // SDTS DEM File Format
    }

// Identification information
WString HRFUSgsSDTSDEMCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

// Identification information
WString HRFUSgsSDTSDEMCreator::GetExtensions() const
    {
    return WString(L"*catd.ddf");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFUSgsSDTSDEMCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                    HFCAccessMode         pi_AccessMode,
                                                    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFUSgsSDTSDEMFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFUSgsSDTSDEMCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                          uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    
    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    //Will initialize GDal if not already initialize
    HRFGdalSupportedFile::Initialize();

    bool                  Result = false;
    HAutoPtr<HFCBinStream> pFile;
    char                  HeaderBuffer[9];

    (const_cast<HRFUSgsSDTSDEMCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Open the IMG File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile->GetSize() >= 24)
        {
        pFile->Read(HeaderBuffer, 9);

        //This is the validation done by GDAL
        if (((HeaderBuffer[5] == '1') || (HeaderBuffer[5] == '2') || (HeaderBuffer[5] == '3')) &&
            (HeaderBuffer[6] == 'L') &&
            ((HeaderBuffer[8] == '1') || (HeaderBuffer[8] == ' ')))
            {
            Result = true;
            }
        }

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFUSgsSDTSDEMCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFUSgsSDTSDEMCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of  File
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFUSgsSDTSDEMCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFUSgsSDTSDEMCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFUSgsSDTSDEMFile::HRFUSgsSDTSDEMFile(const HFCPtr<HFCURL>& pi_rURL,
                                              HFCAccessMode         pi_AccessMode,
                                              uint64_t             pi_Offset)

    : HRFGdalSupportedFile("SDTS", pi_rURL, pi_AccessMode, pi_Offset)
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
HRFResolutionEditor* HRFUSgsSDTSDEMFile::CreateResolutionEditor(uint32_t      pi_Page,
                                                                       unsigned short pi_Resolution,
                                                                       HFCAccessMode pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFUSgsSDTSDEMEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFUSgsSDTSDEMFile::~HRFUSgsSDTSDEMFile()
    {
    //All the work of closing the file and GDAL is done in the ancestor class.
    }

//-----------------------------------------------------------------------------
// Protected
// BuildTransfoModel
//
//-----------------------------------------------------------------------------
RasterFileGeocodingPtr HRFUSgsSDTSDEMFile::ExtractGeocodingInformation()
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
void HRFUSgsSDTSDEMFile::HandleNoDisplayBands()
    {
    //This format is not multiband
    HASSERT(m_NbBands == 1);

    m_GrayBandInd = 1;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFUSgsSDTSDEMFile::CreateDescriptors()
    {
    HPRECONDITION(GetDataSet() != 0);
    HPRECONDITION(GetURL()->IsCompatibleWith(HFCURLFile::CLASS_ID));

    // Create Page and resolution Description/Capabilities for this file.
    HPMAttributeSet                         TagList;

    // Background Tag
    uint32_t BackgroundValue = (unsigned short)USGS_SDTS_BACKGROUND_VALUE;
    TagList.Set(new HRFAttributeBackground(BackgroundValue));

    HFCPtr<HFCURLFile> pURL(new HFCURLFile(GetURL()->GetURL()));

    WString FileName(pURL->GetFilename());

    size_t Pos = FileName.find(L"CATD");

    if (Pos == string::npos)
    {
        throw HFCInvalidFileNameException(FileName);
    }

    FileName   = FileName.replace(Pos, 4, L"DDOM");

    pURL->SetFileName(FileName);

    HAutoPtr<HFCBinStream> pDDOMFileStream(HFCBinStream::Instanciate(pURL.GetPtr(), HFC_READ_ONLY));

    if (pDDOMFileStream != NULL && pDDOMFileStream->GetLastException() == 0)
        {
        //This file is assumed to be small so that loading it all in memory isn't costly.
        HASSERT(pDDOMFileStream->GetSize() < 20000);

        HAutoPtr<char> pDDOMDataBuffer(new char[(size_t)pDDOMFileStream->GetSize() + 1]);

        pDDOMFileStream->Read(pDDOMDataBuffer.get(), (size_t)pDDOMFileStream->GetSize());

        pDDOMDataBuffer[pDDOMFileStream->GetSize()] = '\0';

        double MinElevation;
        double MaxElevation;
        char*  pCurrentDDOMInfoSearchLoc = pDDOMDataBuffer.get();

        if (GetElevationLimit(true, &pCurrentDDOMInfoSearchLoc, &MinElevation) == true)
            {
            if (GetElevationLimit(false, &pCurrentDDOMInfoSearchLoc, &MaxElevation))
                {
                vector<double>             ElevationLimit;

                // Set minimum and maximum value tags.
                ElevationLimit.push_back(MinElevation);
                TagList.Set(new HRFAttributeMinSampleValue(ElevationLimit));

                ElevationLimit.clear();
                ElevationLimit.push_back(MaxElevation);
                TagList.Set(new HRFAttributeMaxSampleValue(ElevationLimit));
                }
            }
        }

    HRFGdalSupportedFile::CreateDescriptorsWith(new HCDCodecIdentity(), TagList);
    }

//-----------------------------------------------------------------------------
// Protected
// GetElevationLimit
// Get the minimum or maximum elevation.
// The string describing the limit has the following form :
//
// ELEVATION§USGS/NMD§INTEGER§I§meters§MAX§1182§Maximum elevation of the DEM
//
// where the § character represents the unit separator character (ascii code 31).
//-----------------------------------------------------------------------------
bool HRFUSgsSDTSDEMFile::GetElevationLimit(bool    pi_SearchMin,
                                            char**  pio_ppDDOMInfo,
                                            double* po_pElevationLimit) const
    {
    HPRECONDITION(pio_ppDDOMInfo != 0 && *pio_ppDDOMInfo != 0);
    HPRECONDITION(po_pElevationLimit != 0);

    bool  LimitFound = false;
    char* pStrFound;

    if (pi_SearchMin == true)
        {
        pStrFound = strstr(*pio_ppDDOMInfo, "MIN");
        }
    else
        {
        pStrFound = strstr(*pio_ppDDOMInfo, "MAX");
        }

    if (pStrFound != 0)
        {
        HASSERT((pStrFound - 7 > *pio_ppDDOMInfo) &&
                ((BeStringUtilities::Strnicmp(pStrFound - 7, "METERS", 6) == 0) ||
                 //I FEET
                 (BeStringUtilities::Strnicmp(pStrFound - 7, "I\x1f\x46\x45\x45\x54", 6) == 0)));

        pStrFound += 4;

        char* pStrElev = pStrFound;

        *po_pElevationLimit = strtod(pStrElev, &pStrFound);

        //Conversion succeeded
        if (pStrFound > pStrElev)
            {
            *pio_ppDDOMInfo = pStrFound;
            LimitFound = true;
            }
        }

    return LimitFound;
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFUSgsSDTSDEMFile::Save()
    {
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFUSgsSDTSDEMFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
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
const HFCPtr<HRFRasterFileCapabilities>& HRFUSgsSDTSDEMFile::GetCapabilities () const
    {
    return (HRFUSgsSDTSDEMCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Protected
// GetScanLineOrientation
//-----------------------------------------------------------------------------
HRFScanlineOrientation HRFUSgsSDTSDEMFile::GetScanLineOrientation()const
    {
    return HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
    }

//-----------------------------------------------------------------------------
// Protected
// GetBandRole
// Get the type of information (elevation, color, temperature, etc...) of
// the requested band.
//-----------------------------------------------------------------------------
HRPChannelType::ChannelRole HRFUSgsSDTSDEMFile::GetBandRole(int32_t pi_RasterBand) const
    {
    HPRECONDITION(m_GrayBandInd == pi_RasterBand);

    return HRPChannelType::ELEVATION;
    }
