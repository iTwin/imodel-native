//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFErdasImgFile.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFErdasImgFile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>
#ifdef IPP_HAVE_GDAL_SUPPORT


#include <ImagePP/all/h/ImageppLib.h>

#include <ImagePP/all/h/HRFErdasImgFile.h>
#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HFCBinStream.h>
#include <ImagePP/all/h/HRFUtility.h>
#include <ImagePP/all/h/HRFDoqEditor.h>
#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HRFException.h>

#include <ImagePP/all/h/HRPPixelTypeI8R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV8Gray8.h>
#include <ImagePP/all/h/HRPPixelTypeV16Gray16.h>
#include <ImagePP/all/h/HRPPixelTypeV16Int16.h>
#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV32Float32.h>
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HRPPixelTypeV48R16G16B16.h>
#include <ImagePP/all/h/HRPPixelTypeV64R16G16B16X16.h>
#include <ImagePP/all/h/HRPPixelTypeV96R32G32B32.h>

#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HRFRasterFileCapabilities.h>

#include <ImagePP/all/h/HGF2DAffine.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HGF2DProjective.h>
#include <ImagePP/all/h/HGF2DStretch.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HGF2DSimilitude.h>
#include <ImagePP/all/h/HGF2DTranslation.h>

#include <ImagePP/all/h/ImagePPMessages.xliff.h>


//GDAL
#include <ImagePP-GdalLib/gdal_priv.h>
#include <ImagePP-GdalLib/cpl_string.h>



//-----------------------------------------------------------------------------
// HRFErdasImgBlockCapabilities
//-----------------------------------------------------------------------------
class HRFErdasImgBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFErdasImgBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add (new HRFLineCapability (HFC_READ_WRITE,
                                    UINT32_MAX,
                                    HRFBlockAccess::RANDOM));

        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_WRITE,         // AccessMode
                                   INT32_MAX,               // MaxSizeInBytes
                                   1,                      // MinHeight
                                   8192,                   // MaxHeight
                                   1));                    // HeightIncrement

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_WRITE_CREATE,        // AccessMode
                                  INT32_MAX,             // MaxSizeInBytes
                                  2,                   // MinWidth
                                  8192,                 // MaxWidth
                                  1,                   // WidthIncrement
                                  2,                  // MinHeight
                                  8192,                  // MaxHeight
                                  1,                    // HeightIncrement
                                  false));              // Not Square

        // Image Capability
        Add(new HRFImageCapability(HFC_READ_ONLY,          // AccessMode
                                   INT32_MAX,               // MaxSizeInBytes
                                   0,                      // MinWidth
                                   INT32_MAX,               // MaxWidth
                                   0,                      // MinHeight
                                   INT32_MAX));             // MaxHeight
        }
    };

//-----------------------------------------------------------------------------
// HRFErdasImgCodecCapabilities
//-----------------------------------------------------------------------------
class HRFErdasImgCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFErdasImgCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_WRITE_CREATE,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFErdasImgBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFErdasImgCapabilities
//-----------------------------------------------------------------------------
HRFErdasImgCapabilities::HRFErdasImgCapabilities()
    : HRFRasterFileCapabilities()
    {
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFErdasImgCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV16Gray16::CLASS_ID,
                                   new HRFErdasImgCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV16Int16::CLASS_ID,
                                   new HRFErdasImgCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFErdasImgCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV48R16G16B16::CLASS_ID,
                                   new HRFErdasImgCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV96R32G32B32::CLASS_ID,
                                   new HRFErdasImgCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV32R8G8B8A8::CLASS_ID,
                                   new HRFErdasImgCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV32Float32::CLASS_ID,
                                   new HRFErdasImgCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV64R16G16B16X16::CLASS_ID,
                                   new HRFErdasImgCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFErdasImgCodecCapabilities()));

    //Supported by Erdas but not way to differentiate RGB from RGBA with GDAL
    /*
    Add(new HRFPixelTypeCapability( HFC_READ_WRITE_CREATE,
                                    HRPPixelTypeI8R8G8B8A8::CLASS_ID,
                                    new HRFErdasImgCodecCapabilities()));
    */

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DIdentity::CLASS_ID));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Still Image Capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // Histogram capability
    Add(new HRFHistogramCapability(HFC_READ_ONLY));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::LINE));

    // Maximum file size capability - Can likely be more than that, but this limit should be
    // ok for some time...
    Add(new HRFMaxFileSizeCapability(HFC_READ_WRITE_CREATE, (uint64_t)UINT64_MAX));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMinSampleValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeMaxSampleValue));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeVerticalUnitRatioToMeter));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_WRITE_CREATE));

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

HFC_IMPLEMENT_SINGLETON(HRFErdasImgCreator)

//-----------------------------------------------------------------------------
// HRFErdasImgCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFErdasImgCreator::HRFErdasImgCreator()
    : HRFRasterFileCreator(HRFErdasImgFile::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    }

// Identification information
Utf8String HRFErdasImgCreator::GetLabel() const
    {
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_ErdasImg()); // "Erdas IMG"
    }

// Identification information
Utf8String HRFErdasImgCreator::GetSchemes() const
    {
    return Utf8String(HFCURLFile::s_SchemeName());
    }

// Identification information
Utf8String HRFErdasImgCreator::GetExtensions() const
    {
    return Utf8String("*.img");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFErdasImgCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                 HFCAccessMode         pi_AccessMode,
                                                 uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFErdasImgFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------

bool HRFErdasImgCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                       uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    //Will initialize GDal if not already initialize
    HRFGdalSupportedFile::Initialize();

    bool  Result = false;

    GDALDriver* pHFADriver = GetGDALDriverManager()->GetDriverByName("HFA");
    if(pHFADriver != NULL && pHFADriver->pfnIdentify != NULL) 
        {
        // TFS#86887: GDAL_FILENAME_IS_UTF8
        Utf8String filenameUtf8 = static_cast<HFCURLFile*>(pi_rpURL.GetPtr())->GetAbsoluteFileName();


        GDALOpenInfo oOpenInfo(filenameUtf8.c_str(), GA_ReadOnly);
        Result = (0 != pHFADriver->pfnIdentify(&oOpenInfo));
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of  File
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFErdasImgCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFErdasImgCapabilities();

    return m_pCapabilities;
    }



//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFErdasImgFile::HRFErdasImgFile(const HFCPtr<HFCURL>& pi_rURL,
                                        HFCAccessMode         pi_AccessMode,
                                        uint64_t             pi_Offset)

    : HRFGdalSupportedFile("HFA", pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen        = false;

    m_GTModelType =     TIFFGeo_Undefined;

    if (GetAccessMode().m_HasCreateAccess)
        {
        Create();
        }
    else
        {
        //Open the file. An exception should be thrown if the open failed.
        Open();

        HASSERT(m_IsOpen == true);
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFErdasImgFile::~HRFErdasImgFile()
    {

    //All the work of closing the file and GDAL is done in the ancestor class.
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFErdasImgFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                             uint16_t pi_Resolution,
                                                             HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFErdasImgEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFErdasImgFile::GetCapabilities () const
    {
    return (HRFErdasImgCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Protected
// ExtractGeocodingInformation
//
//-----------------------------------------------------------------------------
GeoCoordinates::BaseGCSPtr HRFErdasImgFile::ExtractGeocodingInformation(double* po_pVerticalUnitsToMeterRatio)
    {
    GeoCoordinates::BaseGCSPtr pGeocodingInfo;
    char pLocalCS[] = "LOCAL_CS[\"Unknown\"";

    CHECK_ERR(string sProj = m_poDataset->GetProjectionRef();)

    //TR 205873 and TR 230951
    if ((BeStringUtilities::Strnicmp(sProj.c_str(), pLocalCS, sizeof(pLocalCS)) != 0))
        {
        pGeocodingInfo = HRFGdalSupportedFile::ExtractGeocodingInformation(po_pVerticalUnitsToMeterRatio);
        }

    if ((GetNbBands() == 1) && (m_GrayBandInd != -1))
        {
        const char* pUnitName = GetRasterBand(m_GrayBandInd)->GetUnitType();

        HASSERT(pUnitName != 0);

        if (strcmp(pUnitName, "") != 0)
            {
            //Since there is actually no mecanism that ensure that the unit type returned
            //is related the elevation measurement, check that at least the pixel type detected
            //is a elevation related pixel type. Note that it will probably be an error to add a
            //the Geotiff key VerticalUnits if the data found in the ErdasIMG aren't
            //elevation measurements.
            HASSERT(m_pPixelType->IsCompatibleWith(HRPPixelTypeV16Int16::CLASS_ID) ||
                    m_pPixelType->IsCompatibleWith(HRPPixelTypeV32Float32::CLASS_ID));

            if (m_pUnitToNameToEPSGCodeMap == 0)
                {
                CreateUnitNameToEPSGCodeMap();
                }

            UnitNameToEPSGCodeMap::iterator UnitToNameIter = m_pUnitToNameToEPSGCodeMap->find(pUnitName);

            if ((nullptr != po_pVerticalUnitsToMeterRatio) && (UnitToNameIter != m_pUnitToNameToEPSGCodeMap->end()))
                {

                 if (UnitToNameIter->second == 9001)   // Meter
                     *po_pVerticalUnitsToMeterRatio = 1.0;
                 else if (UnitToNameIter->second == 9002)  // International foot
                     *po_pVerticalUnitsToMeterRatio = 0.3048;
                 else if (UnitToNameIter->second == 9003)  // US Survey foot
                     *po_pVerticalUnitsToMeterRatio = 12.0/39.37;

                }
            }
        }

    return pGeocodingInfo;
    }

//-----------------------------------------------------------------------------
// Protected
// BuildTransfoModel
//
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFErdasImgFile::BuildTransfoModel()
    {
    //If the image's physical world is unknown
    if (m_GTModelType == TIFFGeo_UserDefined)
        {
        if (m_GeoRefInfo.m_A11 < 0)
            {
            m_GeoRefInfo.m_A11 *= -1;
            }

        m_GeoRefInfo.m_Ty *= -1;
        }

    return HRFGdalSupportedFile::BuildTransfoModel();
    }

//-----------------------------------------------------------------------------
// Protected
// DetectPixelType
//
//-----------------------------------------------------------------------------
void HRFErdasImgFile::DetectPixelType()
    {
    //Discard RRD files
    if(GetNbBands() == 0)
        throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());

    m_IsBandSpecValid = false;

    ChannelToBandIndexMapping specifiedBand;
    bool IsBandSpecDefined = ImageppLib::GetHost().GetImageppLibAdmin()._GetChannelToBandIndexMapping(specifiedBand);


    if (IsBandSpecDefined)
        {
        uint16_t NbBands = (uint16_t)GetNbBands();

        m_IsBandSpecValid = ((specifiedBand.GetIndex(ChannelToBandIndexMapping::RED)   <= NbBands) &&
                             (specifiedBand.GetIndex(ChannelToBandIndexMapping::GREEN) <= NbBands) &&
                             (specifiedBand.GetIndex(ChannelToBandIndexMapping::BLUE)  <= NbBands) &&
                             (specifiedBand.GetIndex(ChannelToBandIndexMapping::ALPHA) <= NbBands) &&
                             (GetRasterBand(specifiedBand.GetIndex(ChannelToBandIndexMapping::RED))->GetRasterDataType() == GDT_Byte));


        if (m_IsBandSpecValid == true)
            {
            m_RedBandInd   = (signed char)specifiedBand.GetIndex(ChannelToBandIndexMapping::RED);
            m_GreenBandInd = (signed char)specifiedBand.GetIndex(ChannelToBandIndexMapping::GREEN);
            m_BlueBandInd  = (signed char)specifiedBand.GetIndex(ChannelToBandIndexMapping::BLUE);

            HASSERT((m_RedBandInd > 0) && (m_GreenBandInd > 0) && (m_BlueBandInd > 0));

            if (!specifiedBand.IsAlphaChannelDefined())
                {
                m_NbBands = 3;
                m_pPixelType = new HRPPixelTypeV24R8G8B8();
                }
            else
                {
                m_NbBands = 4;
                m_AlphaBandInd  = (signed char)specifiedBand.GetIndex(ChannelToBandIndexMapping::ALPHA);
                m_pPixelType = new HRPPixelTypeV32R8G8B8A8();
                }

            m_DisplayRep          = RGB;
            m_BitsPerPixelPerBand = 8;
            m_Signed              = false;
            }
        }

    if (!m_IsBandSpecValid)
        HRFGdalSupportedFile::DetectPixelType();
    }

//-----------------------------------------------------------------------------
// Protected
// AreDataNeedToBeScaled
//
//-----------------------------------------------------------------------------
bool HRFErdasImgFile::AreDataNeedToBeScaled()
    {
    list<signed char>    BandIndToVerify;
    bool           IsABandNeedToBeScaled = false;
    GDALRasterBand* pGDALRasterBand;
    int32_t          Err;

    //If the pixel type are signed
    if (IsReadPixelSigned() == true)
        {
        IsABandNeedToBeScaled = false;
        }
    else
        {
        //Just check the bands related to the pixel type detected since it is
        //possible for multiband images to include bands related to different
        //pixel type (i.e. : a grayscale band and RGB bands).

        /*Removed because single band unsigned data might be used to represent DEM.
        if (GetDispRep() == MONO)
        {
            if (m_GrayBandInd != -1)     BandIndToVerify.push_back(m_GrayBandInd);
        }
        else*/
        if (GetDispRep() == RGB)
            {
            if (m_RedBandInd != -1)      BandIndToVerify.push_back(m_RedBandInd);
            if (m_GreenBandInd != -1)    BandIndToVerify.push_back(m_GreenBandInd);
            if (m_BlueBandInd != -1)     BandIndToVerify.push_back(m_BlueBandInd);
            if (m_AlphaBandInd != -1)    BandIndToVerify.push_back(m_AlphaBandInd);
            if (m_ExtendedBandInd != -1) BandIndToVerify.push_back(m_ExtendedBandInd);
            }

        list<signed char>::const_iterator BandIndIter    = BandIndToVerify.begin();
        list<signed char>::const_iterator BandIndIterEnd = BandIndToVerify.end();

        while (BandIndIter != BandIndIterEnd)
            {
            pGDALRasterBand = GetRasterBand(*BandIndIter);

            if ((pGDALRasterBand->GetMetadataItem("STATISTICS_MINIMUM") != NULL) &&
                (pGDALRasterBand->GetMinimum(&Err) != GetMinimumPossibleValue((uint16_t)pGDALRasterBand->
                                                                              GetRasterDataType())))
                {
                IsABandNeedToBeScaled = true;
                break;
                }

            if ((pGDALRasterBand->GetMetadataItem("STATISTICS_MAXIMUM") != NULL) &&
                (pGDALRasterBand->GetMaximum(&Err) != GetMaximumPossibleValue((uint16_t)pGDALRasterBand->
                                                                              GetRasterDataType())))
                {
                IsABandNeedToBeScaled = true;
                break;
                }

            BandIndIter++;
            }
        }

    return IsABandNeedToBeScaled || HRFGdalSupportedFile::AreDataNeedToBeScaled();
    }

//-----------------------------------------------------------------------------
// Protected
// HandleNoDisplayBands
//
//-----------------------------------------------------------------------------
void HRFErdasImgFile::HandleNoDisplayBands()
    {
    bool HasColorTable = true;

    for(int32_t i=1; (i <= m_NbBands) && (HasColorTable == true); i++)
        {
        if (GetDataSet()->GetRasterBand(i)->GetColorInterpretation() == GCI_PaletteIndex ||
            GetDataSet()->GetRasterBand(i)->GetColorTable() == NULL)
            {
            HasColorTable = false;
            }
        }

    switch (m_NbBands)
        {
        case 1 :
        case 2 :
            {
            if (HasColorTable == true)
                {
                m_PaletteBandInd = 1;
                }
            else
                {
                m_GrayBandInd = 1;
                }
            break;
            }
        case 3 :
            {
            m_RedBandInd = 1;
            m_GreenBandInd = 2;
            m_BlueBandInd = 3;
            break;
            }
        case 4 :
            {
            m_RedBandInd = 1;
            m_GreenBandInd = 2;
            m_BlueBandInd = 3;
            //(Multiband TODO) For now, consider the fourth band an alpha band for any pixel value for an 8 bit band,
            if (GetDataSet()->GetRasterBand(4)->GetRasterDataType() == GDT_Byte)
                {
                m_AlphaBandInd = 4;
                }
            else
                {
                m_ExtendedBandInd = 4;
                }
            break;
            }
        }
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFErdasImgFile::CreateDescriptors()
    {
    HPRECONDITION(GetDataSet() != 0);

    // Create Page and resolution Description/Capabilities for this file.
    HPMAttributeSet       TagList;
    HFCPtr<HRPHistogram>  pHistogram;

    GetHistogramFromImgHeader(pHistogram);
    
    HRFGdalSupportedFile::CreateDescriptorsWith(new HCDCodecIdentity(), TagList, pHistogram);
    }


//-----------------------------------------------------------------------------
// Private
// Create
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFErdasImgFile::Create()
    {
    m_IsOpen = true;

    return m_IsOpen;
    }

//-----------------------------------------------------------------------------
// Protected
// GetGeoRefInfo
//-----------------------------------------------------------------------------
void HRFErdasImgFile::GetHistogramFromImgHeader(HFCPtr<HRPHistogram>& po_rHistogram)
    {
    HFCPtr<HRPHistogram> pHistogram;

    if (m_BitsPerPixelPerBand <= 16)
        {
        uint16_t      BandInd=0;
        const char*         pHistogramInImg;
        char                FreqSeparator[] = "|";
        int32_t               NbFrequencyVals = (int32_t)pow(2.0, m_BitsPerPixelPerBand);
        uint16_t      NbHistoCh = (uint16_t)MIN(3, m_NbBands);

        pHistogram = new HRPHistogram(NbFrequencyVals, NbHistoCh);

        ChannelToBandIndexMapping specifiedBand;
        if (m_IsBandSpecValid)
            ImageppLib::GetHost().GetImageppLibAdmin()._GetChannelToBandIndexMapping(specifiedBand);

        for (uint16_t ChannelIndex = 0; ChannelIndex < NbHistoCh; ChannelIndex++)
            {
            if (m_IsBandSpecValid == true)
                {
                switch(ChannelIndex)
                    {
                    case ChannelToBandIndexMapping::RED:
                        BandInd = specifiedBand.GetIndex(ChannelToBandIndexMapping::RED);
                        break;
                    case ChannelToBandIndexMapping::GREEN:
                        BandInd = specifiedBand.GetIndex(ChannelToBandIndexMapping::GREEN);
                        break;
                    case ChannelToBandIndexMapping::BLUE:
                        BandInd = specifiedBand.GetIndex(ChannelToBandIndexMapping::BLUE);
                        break;
                    case ChannelToBandIndexMapping::ALPHA:
                        BandInd = specifiedBand.GetIndex(ChannelToBandIndexMapping::ALPHA);
                        break;
                    }
                }
            else
                {
                BandInd = ChannelIndex + 1;
                }

            pHistogramInImg = m_poDataset->GetRasterBand(BandInd)->GetMetadataItem("STATISTICS_HISTOBINVALUES");

            if (pHistogramInImg != 0)
                {
                const char*          pFrequency;
                int64_t              Frequency;
                int32_t                  FreqInd = 0;
                HArrayAutoPtr<char> pHistogramInImgDup(new char[strlen(pHistogramInImg) + 1]);

                strcpy(pHistogramInImgDup.get(), pHistogramInImg);

                char*         next_token=NULL;
                pFrequency = BeStringUtilities::Strtok(pHistogramInImgDup.get(), FreqSeparator,&next_token);

                //Set the frequency for each values until there is no frequency. If there
                //are less specified frequencies then the number of possible pixel value, assume
                //that the remaining frequencies are equal to 0.
                while (pFrequency != 0)
                    {
                    if (FreqInd >= NbFrequencyVals)
                        {
                        HASSERT(!"There are more frequencies specified than possible different values");
                        break;
                        }

                    Frequency = strtoll(pFrequency,0, 10);

                    //The histogram currently accepts only 32 bit frequency values
                    HASSERT(Frequency <= UINT32_MAX);

                    pHistogram->SetEntryCount(FreqInd, (uint32_t)Frequency, ChannelIndex);

                    FreqInd++;

                    pFrequency = BeStringUtilities::Strtok(NULL, FreqSeparator,&next_token);
                    }
                }
            else
                {
                pHistogram = 0;
                break;
                }
            }
        }

    po_rHistogram = pHistogram;
    }

//-----------------------------------------------------------------------------
// Protected
// SetCreationOptions
//-----------------------------------------------------------------------------
void HRFErdasImgFile::SetCreationOptions(HFCPtr<HRFPageDescriptor>& pi_rpPageDesc,
                                         char** &                    pio_rppCreationOptions) const
    {
    HFCPtr<HRFResolutionDescriptor> pResolutionDesc(pi_rpPageDesc->GetResolutionDescriptor(0));

    HASSERT((pResolutionDesc != 0) && (pResolutionDesc->GetBlockType() == HRFBlockType::TILE));

    //Only squared tile can be specified for the ERDAS IMG file format.
    if (pResolutionDesc->GetBlockWidth() == pResolutionDesc->GetBlockHeight())
        {
        char tempBuffer[256];
        sprintf(tempBuffer, "%i", pResolutionDesc->GetBlockHeight());
        pio_rppCreationOptions = CSLSetNameValue(pio_rppCreationOptions, "BLOCKSIZE", tempBuffer);
        }
    }

//-----------------------------------------------------------------------------
// private
// CreateUnitNameToEPSGCodeMap
// The mapping has been done using the document on the Erdas units
// (CVS : imagepp-serv.bentley.com\docs\Technical\Raster Files\Erdas IMG\UnitsDefinitionObtainedFromErdas.dat)
// and the CSMap's unit table (\msj\mstn\geocoord\CSMap\Source\CSdataU.c).
//-----------------------------------------------------------------------------
HAutoPtr<HRFErdasImgFile::UnitNameToEPSGCodeMap> HRFErdasImgFile::m_pUnitToNameToEPSGCodeMap;

void HRFErdasImgFile::CreateUnitNameToEPSGCodeMap()
    {
    HPRECONDITION(m_pUnitToNameToEPSGCodeMap == 0);

    m_pUnitToNameToEPSGCodeMap = new UnitNameToEPSGCodeMap;

    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("meters", (uint16_t)9001));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("meter", (uint16_t)9001));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("m", (uint16_t)9001));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("centimeters", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("centimeter", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("cm", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("millimeters", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("millimeter", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("mm", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("kilometers", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("kilometer", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("km", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("nanometers", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("nanometer", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("nm", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("micron", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("microns", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("micrometers", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("micrometer", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("other", (uint16_t)9001));
    /*
    ** following items are U.S. Survey foot.
    */
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("us_survey_feet", (uint16_t)9003));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("us_survey_foot", (uint16_t)9003));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("feet",(uint16_t) 9003));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("foot", (uint16_t)9003));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("ft", (uint16_t)9003));
    /*
    ** following items are related to Standard foot (0.3048).
    */
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("international_feet", (uint16_t)9002));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("international_foot", (uint16_t)9002));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("inches", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("inch", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("in", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("points", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("point", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("pt", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("yards", (uint16_t)9096));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("yard", (uint16_t)9096));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("yd", (uint16_t)9096));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("miles", (uint16_t)9093));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("mile", (uint16_t)9093));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("mi", (uint16_t)9093));
    /*
    ** variants
    */
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("modified_american_feet", (uint16_t)9004));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("modified_american_foot", (uint16_t)9004));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("clarke_feet",(uint16_t) 9005));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("clarke_foot", (uint16_t)9005));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("indian_feet", (uint16_t)9080));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("indian_foot", (uint16_t)9080));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("links", (uint16_t)9039));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("link", (uint16_t)9039));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("benoit_links", (uint16_t)9063));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("benoit_link", (uint16_t)9063));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("clarke_link", (uint16_t)9039));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("sears_links", (uint16_t)9043));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("sears_link", (uint16_t)9043));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("benoit_chains", (uint16_t)9062));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("benoit_chain", (uint16_t)9062));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("sears_foot", (uint16_t)9041));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("benoit_chain_1895_b", (uint16_t)9062));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("sears_chain", (uint16_t)9042));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("sears_chains", (uint16_t)9042));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("sears_yards", (uint16_t)9040));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("sears_yard", (uint16_t)9040));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("sears_yd", (uint16_t)9040));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("indian_yards", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("indian_yard", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("indian_yd", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("fathoms", (uint16_t)0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("fathom",(uint16_t) 0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("international_nautical_miles", (uint16_t)9030));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("international_nautical_mile", (uint16_t)9030));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("nautical_mile_international", (uint16_t)9030));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("device_pixels",(uint16_t) 0));
    m_pUnitToNameToEPSGCodeMap->insert(UnitNameToEPSGCodeMap::value_type("gold_coast_foot",(uint16_t) 9094));
    }
#endif
