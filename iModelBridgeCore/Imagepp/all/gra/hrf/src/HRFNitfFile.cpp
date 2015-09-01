//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFNitfFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFNitfFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFNitfFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>
#include <Imagepp/all/h/HRPPixelTypeV96R32G32B32.h>

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




//GDAL
#include <ImagePP-GdalLib/gdal_priv.h>
#include <ImagePP-GdalLib/cpl_string.h>

//-----------------------------------------------------------------------------
// HRFNitfBlockCapabilities
//-----------------------------------------------------------------------------
class HRFNitfBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFNitfBlockCapabilities()
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
                                   1,                      // MinHeight
                                   8192,                   // MaxHeight
                                   1));                    // HeightIncrement
        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_ONLY,       // AccessMode
                                  LONG_MAX,            // MaxSizeInBytes
                                  1,                   // MinWidth
                                  8192,                // MaxWidth
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
// HRFNitfCodecCapabilities
//-----------------------------------------------------------------------------
class HRFNitfCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFNitfCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_ONLY,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFNitfBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFNitfCapabilities
//-----------------------------------------------------------------------------
HRFNitfCapabilities::HRFNitfCapabilities()
    : HRFRasterFileCapabilities()
    {
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFNitfCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV48R16G16B16::CLASS_ID,
                                   new HRFNitfCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFNitfCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV16Gray16::CLASS_ID,
                                   new HRFNitfCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV16Int16::CLASS_ID,
                                   new HRFNitfCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFNitfCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV24PhotoYCC::CLASS_ID,
                                   new HRFNitfCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV32Float32::CLASS_ID,
                                   new HRFNitfCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV96R32G32B32::CLASS_ID,
                                   new HRFNitfCodecCapabilities()));

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
    //Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeImageDescription));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributePageName));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDateTime));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeTitle));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(GeographicType);
    pGeocodingCapability->AddSupportedKey(GeogGeodeticDatum);
    pGeocodingCapability->AddSupportedKey(GeogSemiMinorAxis);
    pGeocodingCapability->AddSupportedKey(GeogSemiMajorAxis);
    pGeocodingCapability->AddSupportedKey(GeogPrimeMeridian);
    pGeocodingCapability->AddSupportedKey(GeogAngularUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);
    }

HFC_IMPLEMENT_SINGLETON(HRFNitfCreator)

//-----------------------------------------------------------------------------
// HRFNitfCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFNitfCreator::HRFNitfCreator()
    : HRFRasterFileCreator(HRFNitfFile::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    }

// Identification information
WString HRFNitfCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_NITF()); // NITF File Format
    }

// Identification information
WString HRFNitfCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

// Identification information
WString HRFNitfCreator::GetExtensions() const
    {
    return WString(L"*.ntf;*.nsf");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFNitfCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode,
                                             uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFNitfFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFNitfCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                   uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    //Will initialize GDal if not already initialize
    HRFGdalSupportedFile::Initialize();

    bool                       Result = false;
    HAutoPtr<HFCBinStream>      pFile;
    HArrayAutoPtr<char>        pLine(new char[5]);

    (const_cast<HRFNitfCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Open the IMG File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        pFile->Read(pLine, 4);
        //Check unique file identifier
        if (strncmp(pLine, "NITF", 4) == 0)
            {
            pFile->Read(pLine, 5);
            //Check if the file format version is supported
            if ((strncmp(pLine, "02.00", 5) == 0) ||
                (strncmp(pLine, "02.10", 5) == 0))
                {
                Result = true;
                }
            }
        else //NATO Secondary Imagery Format
            if (strncmp(pLine, "NSIF", 4) == 0)
                {
                pFile->Read(pLine, 5);

                if (strncmp(pLine, "01.00", 5) == 0)
                    {
                    Result = true;
                    }
                }
        }

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFNitfCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFNitfCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }


//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of  File
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFNitfCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFNitfCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFNitfFile::HRFNitfFile(const HFCPtr<HFCURL>& pi_rURL,
                                HFCAccessMode         pi_AccessMode,
                                uint64_t             pi_Offset)

    : HRFGdalSupportedFile("NITF", pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen        = false;

    m_GTModelType = TIFFGeo_Undefined;

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
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
HRFNitfFile::~HRFNitfFile()
    {
    //All the work of closing the file and GDAL is done in the ancestor class.
    }

//-----------------------------------------------------------------------------
// Protected
// Open
// This method open the file
//-----------------------------------------------------------------------------
bool HRFNitfFile::Open()
    {
    if (m_IsOpen)
        return m_IsOpen;
    
    if (HRFGdalSupportedFile::Open())
        {
        HASSERT(m_pPixelType != 0);

        const char* pNITF_ABPP_String = GetDataSet()->GetMetadataItem("NITF_ABPP");
        const char* pNITF_PVTYPE_String = GetDataSet()->GetMetadataItem("NITF_PVTYPE");
        const char* pNITF_IC_String = GetDataSet()->GetMetadataItem("NITF_IC");
            
        uint32_t NbSignificantBits = pNITF_ABPP_String != NULL ? atoi(pNITF_ABPP_String) : 0;
            
        //Post validation

        //Invalid type - The number of bits used by GDAL must be equal or greater than ABPP
        if (GetBitsPerPixelPerBand() < NbSignificantBits && !IsReadPixelReal())
            {
            m_pPixelType = 0;
            throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
            }
        //GDAL doesn't support pixel binary image.
        else if (pNITF_PVTYPE_String != NULL && strncmp(pNITF_PVTYPE_String, "B", 1) == 0)
            {
            m_pPixelType = 0;
            throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
            }
        //GDAL doesn't support bi-level, masked vector quantization and any jpeg compressions.
        else if (pNITF_IC_String != NULL &&
                (   (strncmp(pNITF_IC_String, "I1", 2) == 0) ||
//                    (strncmp(pNITF_IC_String, "C3", 2) == 0) ||       Not supported by Gdal 1.10
                    (strncmp(pNITF_IC_String, "M3", 2) == 0) ||
//                    (strncmp(pNITF_IC_String, "M4", 2) == 0) ||       Not supported by Gdal 1.10
                    (strncmp(pNITF_IC_String, "C5", 2) == 0) ||
                    (strncmp(pNITF_IC_String, "M5", 2) == 0)))
            {
            throw HRFCodecNotSupportedException(GetURL()->GetURL());
            }                
        }

    return m_IsOpen;
    }

//-----------------------------------------------------------------------------
// Protected
// HandleNoDisplayBands
//
//-----------------------------------------------------------------------------
void HRFNitfFile::HandleNoDisplayBands()
    {
    //If no displayable band has been found, the first three bands should be RGB bands (NITF 2.1)
    HASSERT(m_NbBands >= 0);

    if ((m_NbBands == 1) || (m_NbBands == 2))
        {
        m_GrayBandInd = 1;
        }
    else if (m_NbBands >= 3)
        {
        m_RedBandInd = 1;
        m_GreenBandInd = 2;
        m_BlueBandInd = 3;
        }
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFNitfFile::CreateDescriptors()
    {
    HPRECONDITION(GetDataSet() != 0);

    // Create Page and resolution Description/Capabilities for this file.
    HPMAttributeSet TagList;

    // Tag information
    const char* pNITF_IDATIM = GetDataSet()->GetMetadataItem("NITF_IDATIM");
    if(pNITF_IDATIM != NULL)
        TagList.Set(new HRFAttributeDateTime(WString(pNITF_IDATIM,false)));

//     NITF_IID2 not supported available in NITF2.0
//     const char* pNITF_IID2 = GetDataSet()->GetMetadataItem("NITF_IID2");
//     if(pNITF_IID2 != NULL)
//         TagList.Set(new HRFAttributeImageDescription(pNITF_IID2));

    const char* pNITF_OSTAID = GetDataSet()->GetMetadataItem("NITF_OSTAID");
    if(pNITF_OSTAID != NULL)
        TagList.Set(new HRFAttributeSoftware(WString(pNITF_OSTAID,false)));
    
    const char* pNITF_IID1 = GetDataSet()->GetMetadataItem("NITF_IID1");
    if(pNITF_IID1 != NULL)
        TagList.Set(new HRFAttributePageName(WString(pNITF_IID1,false)));

    const char* pNITF_FTITLE = GetDataSet()->GetMetadataItem("NITF_FTITLE");
    if(pNITF_FTITLE != NULL)
        TagList.Set(new HRFAttributeTitle(WString(pNITF_FTITLE,false)));
    
    HRFGdalSupportedFile::CreateDescriptorsWith(new HCDCodecIdentity(), TagList);
    }


//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFNitfFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                         unsigned short pi_Resolution,
                                                         HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFNitfEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }


//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFNitfFile::Save()
    {
    }


//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFNitfFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
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
                    XResolution = ((HRFAttributeXResolution>&)pTag)->GetData();
                // Y Resolution Tag
                else if (pTag->GetID() == HRF_ATTRIBUTEIDS_YResolution)
                    YResolution = ((HRFAttributeYResolution>&)pTag)->GetData();
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
const HFCPtr<HRFRasterFileCapabilities>& HRFNitfFile::GetCapabilities () const
    {
    return (HRFNitfCreator::GetInstance()->GetCapabilities());
    }


