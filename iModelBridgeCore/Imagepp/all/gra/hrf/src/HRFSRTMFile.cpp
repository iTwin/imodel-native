//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSRTMFile.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFSRTMFile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HRFSRTMFile.h>
#include <Imagepp/all/h/HRFSRTMEditor.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>


#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

// Constant initialization
const uint32_t HRFSRTMFile::SRTM1_SIZE = 25934402;
const uint32_t HRFSRTMFile::SRTM3_SIZE = 2884802;
const uint32_t HRFSRTMFile::SRTM1_LINEWIDTH = 3601;
const uint32_t HRFSRTMFile::SRTM3_LINEWIDTH = 1201;
const uint32_t HRFSRTMFile::SRTM1_LINEBYTES = 7202;
const uint32_t HRFSRTMFile::SRTM3_LINEBYTES = 2402;
const int16_t  HRFSRTMFile::SRTM_NODATAVALUE = -32768;
const double   HRFSRTMFile::EARTH_RADIUS = 6378137;
const double   HRFSRTMFile::SRTM1_RES = PI * HRFSRTMFile::EARTH_RADIUS / (HRFSRTMFile::SRTM1_LINEWIDTH * 180);
const double   HRFSRTMFile::SRTM3_RES = PI * HRFSRTMFile::EARTH_RADIUS / (HRFSRTMFile::SRTM3_LINEWIDTH * 180);

//-----------------------------------------------------------------------------
// HRFSRTMBlockCapabilities
//-----------------------------------------------------------------------------
class HRFSRTMBlockCapabilities : public HRFRasterFileCapabilities
    {
    public:
        // Constructor
        HRFSRTMBlockCapabilities()
            : HRFRasterFileCapabilities()
            {
            //// Block Capability
            Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,
                HRFSRTMFile::SRTM1_LINEBYTES,
                HRFBlockAccess::RANDOM));

            Add(new HRFImageCapability(HFC_READ_ONLY,     // AccessMode
                INT32_MAX,                                 // MaxSizeInBytes
                HRFSRTMFile::SRTM3_LINEWIDTH,               // MinWidth
                HRFSRTMFile::SRTM1_LINEWIDTH,               // MaxWidth
                HRFSRTMFile::SRTM3_LINEWIDTH,               // MinHeight
                HRFSRTMFile::SRTM1_LINEWIDTH));             // MaxHeight
            }
    };

//-----------------------------------------------------------------------------
// HRFSRTMCodecIdentityCapabilities
//-----------------------------------------------------------------------------
class HRFSRTMCodecIdentityCapabilities : public  HRFRasterFileCapabilities
    {
    public:
        // Constructor
        HRFSRTMCodecIdentityCapabilities()
            : HRFRasterFileCapabilities()
            {
            // Codec
            Add(new HRFCodecCapability(HFC_READ_ONLY,
                HCDCodecIdentity::CLASS_ID,
                new HRFSRTMBlockCapabilities()));
            }
    };

//-----------------------------------------------------------------------------
// HRFSRTMCapabilities
//-----------------------------------------------------------------------------
HRFSRTMCapabilities::HRFSRTMCapabilities()
    : HRFRasterFileCapabilities()
    {
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
        HRPPixelTypeV16Int16::CLASS_ID,
        new HRFSRTMCodecIdentityCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability;

    pGeocodingCapability = new HRFGeocodingCapability(HFC_READ_ONLY);

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSTypeLong);
    pGeocodingCapability->AddSupportedKey(GTCitation);
    pGeocodingCapability->AddSupportedKey(Projection);
    pGeocodingCapability->AddSupportedKey(ProjCoordTrans);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnitSize);
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
    pGeocodingCapability->AddSupportedKey(ProjRectifiedGridAngle);
    pGeocodingCapability->AddSupportedKey(ProjStraightVertPoleLong);
    pGeocodingCapability->AddSupportedKey(VerticalCSType);
    pGeocodingCapability->AddSupportedKey(VerticalCitation);
    pGeocodingCapability->AddSupportedKey(VerticalDatum);
    pGeocodingCapability->AddSupportedKey(VerticalUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));

    }

HFC_IMPLEMENT_SINGLETON(HRFSRTMCreator)

//-----------------------------------------------------------------------------
// Constructor
// Public (HRFSRTMCreator)
// This is the creator to instantiate SRTM format
//-----------------------------------------------------------------------------
HRFSRTMCreator::HRFSRTMCreator()
: HRFRasterFileCreator(HRFSRTMFile::CLASS_ID)
    {
    // SRTM capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// GetLabel
// Public (HRFSRTMCreator)
// Identification information
//-----------------------------------------------------------------------------
Utf8String HRFSRTMCreator::GetLabel() const
    {
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_SRTM()); // SRTM File Format
    }

//-----------------------------------------------------------------------------
// GetSchemes
// Public (HRFSRTMCreator)
// Identification information
//-----------------------------------------------------------------------------
Utf8String HRFSRTMCreator::GetSchemes() const
    {
    return Utf8String(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// GetExtensions
// Public (HRFSRTMCreator)
// Identification information
//-----------------------------------------------------------------------------
Utf8String HRFSRTMCreator::GetExtensions() const
    {
    return Utf8String("*.hgt");
    }

//-----------------------------------------------------------------------------
// Create
// Public (HRFSRTMCreator)
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFSRTMCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode   pi_AccessMode,
                                            uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFSRTMFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// IsKindOfFile
// Public (HRFSRTMCreator)
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFSRTMCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                 uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool                   bResult = false;
    HAutoPtr<HFCBinStream>  pFile;

    // Open the SRTM File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    //An SRTM file will always have one of two specific sizes
    if ((pFile->GetSize() != HRFSRTMFile::SRTM1_SIZE) && (pFile->GetSize() != HRFSRTMFile::SRTM3_SIZE))
        goto WRAPUP;

    //Since the only criteria we have is the filesize, we check the file extension to eliminate the possibility of error.
    if (!((HFCPtr<HFCURLFile>&)pi_rpURL)->GetExtension().EqualsI("hgt"))
        goto WRAPUP;

    bResult = true;

WRAPUP:
    return bResult;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public (HRFSRTMCreator)
// Create or get the singleton capabilities of SRTM file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFSRTMCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities = new HRFSRTMCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------

HRFSRTMFile::HRFSRTMFile(const HFCPtr<HFCURL>& pi_rURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset)
                       : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen = false;

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }
    else
        {
        // Create Page and Res Descriptors.
        Open();
        CreateDescriptors();
        }
    }

//-----------------------------------------------------------------------------
// Destructor
// Public
// Destroy SRTM file object
//-----------------------------------------------------------------------------
HRFSRTMFile::~HRFSRTMFile()
    {}

//-----------------------------------------------------------------------------
// Open
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFSRTMFile::Open()
    {
    // Open the file
    if (!m_IsOpen)
        {
        m_pSRTMFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

        if (IsSRTM1())
            {
            m_Width = 3601;
            }
        else
            {
            m_Width = 1201;
            }

        m_IsOpen = true;
        }

    return true;
    }

//-----------------------------------------------------------------------------
// AddPage
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFSRTMFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);
    HPRECONDITION(pi_pPage != 0);

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(0);

    // Set the image file information.
    m_Width = (uint32_t) pResolutionDescriptor->GetWidth();

    return true;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFSRTMFile::GetCapabilities() const
    {
    return (HRFSRTMCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// CreateDescriptors
// Protected
// Create SRTM File Descriptors
//-----------------------------------------------------------------------------
void HRFSRTMFile::CreateDescriptors()
    {
    // Create Page and Resolution Description/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor>     pResolution;
    HFCPtr<HRFPageDescriptor>           pPage;

    HFCPtr<HRPPixelType>  pPixelType = new HRPPixelTypeV16Int16(HRPChannelType::USER, &HRFSRTMFile::SRTM_NODATAVALUE);;

    double scale;
    double offsetLatitude;
    double offsetLongitude;

    if (IsSRTM1())
        {
        scale = HRFSRTMFile::SRTM1_RES;
        }
    else
        {
        scale = HRFSRTMFile::SRTM3_RES;
        }

    ExtractLatLong(&offsetLatitude, &offsetLongitude);

    if (offsetLongitude < -179.9999999999)
        offsetLongitude = -179.9999999999;
    if (offsetLongitude > 179.9999999999)
        offsetLongitude = 179.9999999999;

    // Geocoding and Reference
    HFCPtr<HGF2DTransfoModel> pTransfoModel;

    GeoCoordinates::BaseGCSPtr pBaseGCS;


    WString WKTString = L"PROJCS[\"EPSG:900913\", \
                          GEOGCS[\"GCS_Sphere_WGS84\", \
                          DATUM[\"SphereWGS84\", \
                          SPHEROID[\"SphereWGS84\",6378137.0,0.0], \
                          TOWGS84[0, 0, 0, 0, 0, 0, 0] \
                          ], \
                          PRIMEM[\"Greenwich\",0.0], \
                          UNIT[\"Degree\",0.0174532925199433 ], \
                          ], \
                          PROJECTION[\"Mercator\"], \
                          PARAMETER[\"False_Easting\",0.0], \
                          PARAMETER[\"False_Northing\",0.0], \
                          PARAMETER[\"Central_Meridian\",0.0], \
                          PARAMETER[\"Standard_Parallel_1\",0.0], \
                          UNIT[\"Meter\", 1.0] \
                          ]";

    // Obtain the GCS
    pBaseGCS = GeoCoordinates::BaseGCS::CreateGCS();
    if (SUCCESS == pBaseGCS->InitFromWellKnownText(NULL, NULL, GeoCoordinates::BaseGCS::wktFlavorOGC, WKTString.c_str()))
        {
        GeoPoint geoPoint = {offsetLongitude, offsetLatitude + 1, 0.0};
        DPoint3d cartesianPoint;
        pBaseGCS->CartesianFromLatLong(cartesianPoint, geoPoint);
        pTransfoModel = new HGF2DStretch(HGF2DDisplacement(cartesianPoint.x, cartesianPoint.y), scale, scale / cos((offsetLatitude + 0.5)* PI / 180));
        }


    if (pTransfoModel == nullptr)
        pTransfoModel = new HGF2DStretch(HGF2DDisplacement(0.0, 0.0), scale, scale / cos((offsetLatitude + 0.5)* PI / 180));

    // Flip the Y Axe because the origin of ModelSpace is lower-left
    HFCPtr<HGF2DStretch> pFlipModel = new HGF2DStretch();
    pFlipModel->SetYScaling(-1.0);
    pTransfoModel = pFlipModel->ComposeInverseWithDirectOf(*pTransfoModel);

    //TODO : Set NoDataValue? The line below doesn't work, due to const...We eould have to const cast, not sure if this is appropriate...
    //pPixelType->GetChannelOrg().GetChannelPtr(0)->SetNoDataValue(-32768);

    //TODO : See if block type, block access, width and height can be different.
    // Create Resolution Descriptor
    pResolution = new HRFResolutionDescriptor(
        GetAccessMode(),               // AccessMode,
        GetCapabilities(),             // Capabilities,
        1.0,                           // XResolutionRatio,
        1.0,                           // YResolutionRatio,
        pPixelType,                    // PixelType,
        new HCDCodecIdentity(),        // Codec,
        HRFBlockAccess::RANDOM,        // RBlockAccess,
        HRFBlockAccess::RANDOM,        // WBlockAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
        HRFInterleaveType::PIXEL,      // InterleaveType
        0,                             // IsInterlace,
        m_Width,                       // Width,
        m_Width,                       // Height,
        m_Width,                       // BlockWidth,
        m_Width,                       // BlockHeight,
        0,                             // BlocksDataFlag
        HRFBlockType::IMAGE);          // BlockType


    pPage = new HRFPageDescriptor(GetAccessMode(),
                                  GetCapabilities(),   // Capabilities,
                                  pResolution,         // ResolutionDescriptor,
                                  0,                   // RepresentativePalette,
                                  0,                   // Histogram,
                                  0,                   // Thumbnail,
                                  0,                   // ClipShape,
                                  pTransfoModel,       // TransfoModel,
                                  0,                   // Filters
                                  0);                  // Defined Tag

	if (!pBaseGCS.IsNull() && pBaseGCS->IsValid())
        pPage->SetGeocoding(pBaseGCS.get());
								  
    m_ListOfPageDescriptor.push_back(pPage);
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFSRTMFile::GetWorldIdentificator() const
    {
    //return HGF2DWorld_GEOTIFFUNKNOWN;
	return HGF2DWorld_HMRWORLD;
    }

//-----------------------------------------------------------------------------
// CreateResolutionEditor
// Public
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFSRTMFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        uint16_t pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFSRTMImageEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    //pEditor = new HRFSRTMLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// Save
// Disabled
//-----------------------------------------------------------------------------
void HRFSRTMFile::Save()
    {}

//-----------------------------------------------------------------------------
// Private
// ExtractLatLong
// Outputs the latitude and longitude of the bottom-left corner of the SRTM file
//-----------------------------------------------------------------------------
void HRFSRTMFile::ExtractLatLong(double* latitude, double* longitude) const
    {
    const HFCPtr<HFCURL>& urlFile = m_pSRTMFile->GetURL();
    Utf8String fileName = ((HFCPtr<HFCURLFile>&) urlFile)->GetFilename();
    char latHemi = fileName[0];
    char lonHemi = fileName[3];

    HASSERT(fileName[1] >= '0' && (fileName[1] <= '9'));
    HASSERT(fileName[2] >= '0' && (fileName[2] <= '9'));
    HASSERT(fileName[4] >= '0' && (fileName[4] <= '9'));
    HASSERT(fileName[5] >= '0' && (fileName[5] <= '9'));
    HASSERT(fileName[6] >= '0' && (fileName[6] <= '9'));

    *latitude = (double) atof(fileName.substr(1, 2).c_str());
    *longitude = (double) atof(fileName.substr(4, 3).c_str());
    if (latHemi == 'S')
        {
        *latitude *= -1;
        }
    if (lonHemi == 'W')
        {
        *longitude *= -1;
        }
    }

//-----------------------------------------------------------------------------
// Private
// IsSRTM1
// Outputs true if the file is SRTM1, false otherwise (should be SRTM3)
//-----------------------------------------------------------------------------

bool HRFSRTMFile::IsSRTM1() const
    {
    return (m_pSRTMFile->GetSize() == HRFSRTMFile::SRTM1_SIZE);
    }