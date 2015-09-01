//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFDtedFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFDtedFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFDtedFile.h>
#include <Imagepp/all/h/HRFDtedEditor.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include <Imagepp/all/h/HCPGeoTiffKeys.h>



//GDAL
#include <ImagePP-GdalLib/gdal_priv.h>
#include <ImagePP-GdalLib/cpl_string.h>


//-----------------------------------------------------------------------------
// HRFDtedBlockCapabilities
//-----------------------------------------------------------------------------
class HRFDtedBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFDtedBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add (new HRFLineCapability (HFC_READ_ONLY,
                                    ULONG_MAX,
                                    HRFBlockAccess::RANDOM));

        // Note : The value of MaxSizeInBytes should depends on the organization,
        // the number of bands and the pixel type of the image.

        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_ONLY,          // AccessMode
                                   LONG_MAX,               // MaxSizeInBytes
                                   2,                      // MinHeight
                                   8192,                   // MaxHeight
                                   1));                    // HeightIncrement
        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_ONLY,        // AccessMode
                                  LONG_MAX,             // MaxSizeInBytes
                                  1,                   // MinWidth
                                  8192,                 // MaxWidth
                                  1,                   // WidthIncrement
                                  1,                  // MinHeight
                                  8192,                  // MaxHeight
                                  1,                    // HeightIncrement
                                  false));              // Not Square

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
// HRFDtedCodecCapabilities
//-----------------------------------------------------------------------------
class HRFDtedCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFDtedCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_ONLY,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFDtedBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFDtedCapabilities
//-----------------------------------------------------------------------------
HRFDtedCapabilities::HRFDtedCapabilities()
    : HRFRasterFileCapabilities()
    {
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV16Int16::CLASS_ID,
                                   new HRFDtedCodecCapabilities()));

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

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(GeographicType);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);
    }

HFC_IMPLEMENT_SINGLETON(HRFDtedCreator)

//-----------------------------------------------------------------------------
// HRFDtedCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFDtedCreator::HRFDtedCreator()
    : HRFRasterFileCreator(HRFDtedFile::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    }

// Identification information
WString HRFDtedCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_DTED()); // DTED File Format
    }

// Identification information
WString HRFDtedCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

// Identification information
WString HRFDtedCreator::GetExtensions() const
    {
    return WString(L"*.dt0;*.dt1;*.dt2");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFDtedCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode,
                                             uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFDtedFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFDtedCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                   uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    //Will initialize GDal if not already initialize
    HRFGdalSupportedFile::Initialize();

    bool                       Result = false;
    HAutoPtr<HFCBinStream>      pFile;
    HArrayAutoPtr<char>        pLine(new char[4]);

    (const_cast<HRFDtedCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Open the IMG File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        pFile->Read(pLine, 3);
        //Check if the section User Header Label can be found
        if (strncmp(pLine, "UHL", 3) == 0)
            {
            pFile->SeekToPos(80);
            pFile->Read(pLine, 3);

            //Check if the section Data Set Identification Record can be found
            if (strncmp(pLine, "DSI", 3) == 0)
                {
                pFile->SeekToPos(728);
                pFile->Read(pLine, 3);

                //Check if the section Accuracy Record can be found
                if (strncmp(pLine, "ACC", 3) == 0)
                    {
                    Result = true;
                    }
                }
            }
        }

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFDtedCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFDtedCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }


//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of  File
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFDtedCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFDtedCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFDtedFile::HRFDtedFile(const HFCPtr<HFCURL>& pi_rURL,
                                HFCAccessMode   pi_AccessMode,
                                uint64_t       pi_Offset)

    : HRFGdalSupportedFile("DTED", pi_rURL, pi_AccessMode, pi_Offset)
    {
    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }

    // The ancestor store the access mode
    m_IsOpen        = false;
    m_GTModelType = TIFFGeo_Undefined;

    //Open the file. An exception should be thrown if the open failed.
    Open();

    HASSERT(m_IsOpen == true);
    // Create Page and Res Descriptors.
    CreateDescriptors();
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFDtedFile::~HRFDtedFile()
    {
    //All the work of closing the file and GDAL is done in the ancestor class.
    }

//-----------------------------------------------------------------------------
// Protected
// HandleNoDisplayBands
//
//-----------------------------------------------------------------------------
void HRFDtedFile::HandleNoDisplayBands()
    {
    //There should at least be one band in the file.
    HASSERT(m_NbBands >= 1);

    if (m_NbBands >= 1)
        {
        m_GrayBandInd = 1;
        }
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFDtedFile::CreateDescriptors()
    {
    HPRECONDITION(GetDataSet() != 0);

    // Create Page and resolution Description/Capabilities for this file.
    HPMAttributeSet                         TagList;

    HRFGdalSupportedFile::CreateDescriptorsWith(new HCDCodecIdentity(), TagList);

    IRasterBaseGcsPtr pBaseGCS;

    if (GetPageDescriptor(0)->GetGeocodingCP() == NULL)
        pBaseGCS = GCSServices->_CreateRasterBaseGcs();
    else
		pBaseGCS = GetPageDescriptor(0)->GetGeocodingCP()->Clone();

    //Elevation values in a DTED file are always in meters.
    if(pBaseGCS != NULL)
        pBaseGCS->SetVerticalUnits(1.0);

    GetPageDescriptor(0)->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pBaseGCS.get()));
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFDtedFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                         unsigned short pi_Resolution,
                                                         HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return new HRFDtedEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    }


//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFDtedFile::Save()
    {
    }


//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFDtedFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HASSERT(false); //TODO
    /*
        for (TagIterator  = pPageDescriptor->GetTags().begin();
            TagIterator != pPageDescriptor->GetTags().end(); TagIterator++)
        {
            HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

            if (pPageDescriptor->HasTag(pTag))
            {
                // X Resolution Tag
                if (pTag->GetID() == HRF_ATTRIBUTEIDS_XResolution)
                    XResolution = ((HFCPtr<HRFAttributeXResolution>&)pTag)->GetData();
                // Y Resolution Tag
                else if (pTag->GetID() == HRF_ATTRIBUTEIDS_YResolution)
                    YResolution = ((HFCPtr<HRFAttributeYResolution>&)pTag)->GetData();
                // Resolution Unit
                else if (pTag->GetID() == HRF_ATTRIBUTEIDS_ResolutionUnit)
                    Unit = ((HFCPtr<HRFAttributeResolutionUnit>&)pTag)->GetData();
            }
        }
        */
    return false;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFDtedFile::GetCapabilities () const
    {
    return (HRFDtedCreator::GetInstance()->GetCapabilities());
    }


//-----------------------------------------------------------------------------
// Protected
// DetectOptimalBlockAccess
//-----------------------------------------------------------------------------
void HRFDtedFile::DetectOptimalBlockAccess()
    {
    HPRECONDITION(m_NbBands > 0);

    //It is assumed that each band has the same optimal access type
    //Note : The width and height are inversed here to read the image as an SLO 0
    m_poDataset->GetRasterBand(1)->GetBlockSize(&m_BlockHeight, &m_BlockWidth);
    }

//-----------------------------------------------------------------------------
// Protected
// GetScanLineOrientation
//-----------------------------------------------------------------------------
HRFScanlineOrientation HRFDtedFile::GetScanLineOrientation()const
    {
    return HRFScanlineOrientation::UPPER_LEFT_VERTICAL;
    }

//-----------------------------------------------------------------------------
// Protected
// GetBandRole
// Get the type of information (elevation, color, temperature, etc...) of
// the requested band.
//-----------------------------------------------------------------------------
HRPChannelType::ChannelRole HRFDtedFile::GetBandRole(int32_t pi_RasterBand) const
    {
    return HRPChannelType::ELEVATION;
    }



//-----------------------------------------------------------------------------
// Public
// Set the default ratio to meter specified by the user, if this ratio cannot
// be deduced from the file metadata.
//-----------------------------------------------------------------------------
void HRFDtedFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                uint32_t pi_Page,
                                                bool   pi_CheckSpecificUnitSpec,
                                                bool   pi_InterpretUnitINTGR)
    {
    //The units is implicitly specified in the specification.
    }
