//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFUSgsNDFFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HTIFFFile.h>
#include <Imagepp/all/h/HRFUSgsNDFFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFUSgsNDFLineEditor.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>

#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>

#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DProjective.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include "HRFGeoTiffTable.h"

#include <BeXml/BeXml.h>



//-----------------------------------------------------------------------------
// HRFUSgsNDFBlockCapabilities
//-----------------------------------------------------------------------------
class HRFUSgsNDFBlockCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFUSgsNDFBlockCapabilities ()
        : HRFRasterFileCapabilities()
        {
        // Block capability
        Add (new HRFLineCapability (HFC_READ_ONLY,
                                    LONG_MAX,
                                    HRFBlockAccess::SEQUENTIAL));
        }
    };

//-----------------------------------------------------------------------------
// HRFUSgsNDFCodecCapabilities
//-----------------------------------------------------------------------------
class HRFUSgsNDFCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFUSgsNDFCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_ONLY,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFUSgsNDFBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFUSgsNDFCapabilities
//-----------------------------------------------------------------------------
HRFUSgsNDFCapabilities::HRFUSgsNDFCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV8Gray8
    Add (new HRFPixelTypeCapability (HFC_READ_ONLY,
                                     HRPPixelTypeV8Gray8::CLASS_ID,
                                     new HRFUSgsNDFCodecCapabilities()));

    // PixelTypeV16Gray16
    Add (new HRFPixelTypeCapability (HFC_READ_ONLY,
                                     HRPPixelTypeV16Gray16::CLASS_ID,
                                     new HRFUSgsNDFCodecCapabilities()));

    // PixelTypeV24R8G8B8
    Add (new HRFPixelTypeCapability (HFC_READ_ONLY,
                                     HRPPixelTypeV24R8G8B8::CLASS_ID,
                                     new HRFUSgsNDFCodecCapabilities()));

    // PixelTypeV48R16G16B16
    Add (new HRFPixelTypeCapability (HFC_READ_ONLY,
                                     HRPPixelTypeV48R16G16B16::CLASS_ID,
                                     new HRFUSgsNDFCodecCapabilities()));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // Interleave Capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Still Image Capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DProjective::CLASS_ID));
    }

HFC_IMPLEMENT_SINGLETON(HRFUSgsNDFCreator);

//-----------------------------------------------------------------------------
// Creator
// This is the creator to instantiate NDF format
//-----------------------------------------------------------------------------
HRFUSgsNDFCreator::HRFUSgsNDFCreator()
    : HRFRasterFileCreator(HRFUSgsNDFFile::CLASS_ID)
    {
    m_pCapabilities = 0;
    m_RedBand       = 0;
    m_GreenBand     = 1;
    m_BlueBand      = 2;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFUSgsNDFCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_USGS_NDF());  // USGS NDF File Format
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFUSgsNDFCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFUSgsNDFCreator::GetExtensions() const
    {
    return WString(L"*.h1;*.h2;*.h3;*.usgs");
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// Allow to Open an image file
//-----------------------------------------------------------------------------

HRFUSgsNDFFile::HRFUSgsNDFFile(const HFCPtr<HFCURL>& pi_rURL,
                                      int32_t                pi_RedBand,
                                      int32_t                pi_GreenBand,
                                      int32_t                pi_BlueBand,
                                      HFCAccessMode         pi_AccessMode,
                                      uint64_t             pi_Offset)

    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    m_IsOpen        = false;
    m_ImgRedBand    = pi_RedBand;
    m_ImgGreenBand  = pi_GreenBand;
    m_ImgBlueBand   = pi_BlueBand;

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        throw HFCFileReadOnlyException(pi_rURL->GetURL());

    Open();
    CreateDescriptors();
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFUSgsNDFFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

//-----------------------------------------------------------------------------
// Public
// SetDefaultRatioToMeter
// Set the default ratio to meter specified by the user, if this ratio cannot
// be deduced from the file metadata.
//-----------------------------------------------------------------------------
void HRFUSgsNDFFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                   uint32_t pi_Page,
                                                   bool   pi_CheckSpecificUnitSpec,
                                                   bool   pi_InterpretUnitINTGR)
    {
    //The following tag and its value is always added to the page descriptor :
    //GeogLinearUnits - TIFFGeo_Linear_Meter
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFUSgsNDFFile::HRFUSgsNDFFile(const HFCPtr<HFCURL>& pi_rURL,
                                      HFCAccessMode         pi_AccessMode,
                                      uint64_t             pi_Offset,
                                      bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen = false;
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFUSgsNDFFile::~HRFUSgsNDFFile()
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        try
            {
            m_pRedFile = 0;
            m_pGreenFile = 0;
            m_pBlueFile = 0;

            delete[] m_Bands;
            m_IsOpen = false;
            }
        catch(...)
            {
            // Simply stop exceptions in the destructor
            // We want to known if a exception is throw.
            HASSERT(0);
            }
        }
    }

//-----------------------------------------------------------------------------
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFUSgsNDFCreator::Create(
    const HFCPtr<HFCURL>& pi_rpURL,
    HFCAccessMode         pi_AccessMode,
    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(pi_Offset >= 0);

    HFCPtr<HRFRasterFile> pFile = new HRFUSgsNDFFile(pi_rpURL, m_RedBand, m_GreenBand, m_BlueBand, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Public
// IsKindOfFile
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFUSgsNDFCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                      uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    
    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    bool   Result = false;

    HFCPtr<HFCURLFile>& rpURL((HFCPtr<HFCURLFile>&)pi_rpURL);

    (const_cast<HRFUSgsNDFCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    if (BeStringUtilities::Wcsicmp(rpURL->GetExtension().c_str(), L"usgs") == 0)
        {
        uint32_t ChannelCount = 0;
        WString USGSFileName;
        WString ChannelCountStr;
        WString headerFileWStr;
        string HeaderExtensionStr;

        // Extract the standard file name from the main URL
        USGSFileName = rpURL->GetHost();
        USGSFileName += L"\\";
        USGSFileName += rpURL->GetPath();

        // Open XML file
        BeXmlStatus xmlStatus;
        BeXmlDomPtr pDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, USGSFileName.c_str ());

        //Validate pDom
        if (pDom.IsNull() || (BEXML_Success != xmlStatus))
            goto WRAPUP;

        // Validate main node presence
        BeXmlNodeP pMainNode = pDom->GetRootElement ();
        if (NULL == pMainNode || BeStringUtilities::Stricmp (pMainNode->GetName(), "MultiChannelImageFileFormat") != 0)
            goto WRAPUP;

        // Validate VERSION node presence (for now only validate node presence)
        BeXmlNodeP pVersionNode = pMainNode->SelectSingleNode ("VERSION");   
        if (!pVersionNode)
            goto WRAPUP;

        // Validate HEADER node presence
        BeXmlNodeP pHeaderNode = pMainNode->SelectSingleNode ("HEADER");
        if (!pHeaderNode)
            goto WRAPUP;

        pHeaderNode->GetContent (headerFileWStr);
        headerFileWStr = headerFileWStr.substr (headerFileWStr.find (L".") +1 );
        if (BeStringUtilities::Wcsicmp (headerFileWStr.c_str (), L"h1") != 0
         && BeStringUtilities::Wcsicmp (headerFileWStr.c_str(), L"h2") != 0
         && BeStringUtilities::Wcsicmp (headerFileWStr.c_str(), L"h3") != 0)
            goto WRAPUP;

        // Validate CHANNELS node presence
        BeXmlNodeP pChannelNode = pMainNode->SelectSingleNode ("CHANNELS");
        if (!pChannelNode)
            goto WRAPUP;

        // Validate COUNT node presence
        BeXmlNodeP pCountNode = pChannelNode->SelectSingleNode ("COUNT");
        if (!pCountNode)
            goto WRAPUP;

        // Validate channel count, must be 3 (type = UInt32)
        if(BEXML_Success != pCountNode->GetContentUInt32Value (ChannelCount) || ChannelCount != 3)
            goto WRAPUP;

        // Validate RED node presence
        BeXmlNodeP pRedNode = pChannelNode->SelectSingleNode ("RED");
        if (!pRedNode)
            goto WRAPUP;

        // Validate GREEN node presence
        BeXmlNodeP pGreenNode = pChannelNode->SelectSingleNode ("GREEN");
        if (!pGreenNode)
            goto WRAPUP;

        // Validate BLUE node presence
        BeXmlNodeP pBlueNode = pChannelNode->SelectSingleNode ("BLUE");
        if (!pBlueNode)
            goto WRAPUP;

        Result = true;
        }
    else
        {
        HAutoPtr<HFCBinStream>      pFile;
        char buffer[12+1];

        // Open the NDF Header File and place file pointer at the start of the file
        pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

        if (pFile == 0 || pFile->GetLastException() != 0)
            goto WRAPUP;

        if (pFile->Read(&buffer,12) != 12)
            goto WRAPUP;

        if (BeStringUtilities::Strnicmp(buffer, "NDF_REVISION",12) == 0)
            Result = true;
        }

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFUSgsNDFCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFUSgsNDFCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }

//-----------------------------------------------------------------------------
// Static
// ComposeChannelURL
// URL composition utility
//-----------------------------------------------------------------------------
static HFCPtr<HFCURL> ComposeChannelURL(const HFCPtr<HFCURLFile>& pi_rpFileURL,
                                        const WString             pi_ChannelStr)
    {
    HPRECONDITION(pi_rpFileURL != 0);
    HPRECONDITION(!pi_ChannelStr.empty());

    HFCPtr<HFCURL> pChURL = 0;
    WString NewChPathNameStr;

    // Compose URL string
    if ((pi_ChannelStr.find(L":\\\\") != WString::npos) || (pi_ChannelStr.find(L"://") != WString::npos))
        {
        // Complete URL nothing to do
        NewChPathNameStr = pi_ChannelStr;
        }
    else if (  (pi_ChannelStr.find(L"\\\\") != WString::npos) || (pi_ChannelStr.find(L"//") != WString::npos)
               || (pi_ChannelStr.find(L":\\") != WString::npos) || (pi_ChannelStr.find(L":/") != WString::npos) )
        {
        // Path is complete but we must add "file://" prefix
        NewChPathNameStr = WString(HFCURLFile::s_SchemeName() + L"://") + pi_ChannelStr;
        }
    else
        {
        // Path is relative to file location
        WString Path     = pi_rpFileURL->GetPath();
        WString FileName = pi_rpFileURL->GetFilename();

        WString::size_type FileNamePos = Path.rfind(FileName);

        if (FileNamePos != WString::npos)
            Path = Path.substr(0, FileNamePos);

        NewChPathNameStr = WString(HFCURLFile::s_SchemeName() + L"://")
                           + pi_rpFileURL->GetHost() + L"\\"
                           + Path
                           + pi_ChannelStr;
        }

    // Compose real URL
    try
        {
        pChURL = HFCURL::Instanciate(NewChPathNameStr);
        return pChURL;
        }
    catch(...)
        {
        return 0; // not a valid URL
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Open
// This method open the file
//-----------------------------------------------------------------------------
bool HRFUSgsNDFFile::Open()
    {
    WString HeaderFileNameStr;
    WString RedFileNameStr;
    WString GreenFileNameStr;
    WString BlueFileNameStr;
    WString USGSFileName;

    HFCPtr<HFCURL> pHeaderFileURL = 0;
    HFCPtr<HFCURL> pRedFileURL    = 0;
    HFCPtr<HFCURL> pGreenFileURL  = 0;
    HFCPtr<HFCURL> pBlueFileURL   = 0;

    bool IsUSGSFile = false;

    if (!m_IsOpen)
        {
        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Lock the sister file.
        HFCLockMonitor SisterFileLock (GetLockManager());

        HFCPtr<HFCURLFile>& rpURL((HFCPtr<HFCURLFile>&)GetURL());

        if (BeStringUtilities::Wcsicmp(rpURL->GetExtension().c_str(), L"usgs") == 0)
            {
            // Extract the standard file name from the main URL
            USGSFileName =  rpURL->GetHost();
            USGSFileName += L"\\";
            USGSFileName += rpURL->GetPath();

            // Open USGS file
            BeXmlStatus xmlStatus;
            BeXmlDomPtr pDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, USGSFileName.c_str ());

            //Validate pDom
            if (pDom.IsNull() || (BEXML_Success != xmlStatus))
                return false;

            // Read data in USGS file (strings)
            BeXmlNodeP pMainNode = pDom->GetRootElement ();
            if (NULL == pMainNode)
                return false;

            WString content;

            // Validate HEADER node presence
            BeXmlNodeP pHeaderNode = pMainNode->SelectSingleNode ("HEADER");
            pHeaderNode->GetContent (content);
            HeaderFileNameStr = content.c_str();

            // Validate RED node presence
            BeXmlNodeP pRedNode = pMainNode->SelectSingleNode ("CHANNELS/RED");
            pRedNode->GetContent (content);
            RedFileNameStr = content.c_str();

            // Validate GREEN node presence
            BeXmlNodeP pGreenNode = pMainNode->SelectSingleNode ("CHANNELS/GREEN");
            pGreenNode->GetContent (content);
            GreenFileNameStr = content.c_str();

            // Validate BLUE node presence
            BeXmlNodeP pBlueNode = pMainNode->SelectSingleNode ("CHANNELS/BLUE");
            pBlueNode->GetContent (content);
            BlueFileNameStr = content.c_str();

            if (!(pHeaderFileURL = ComposeChannelURL(rpURL, HeaderFileNameStr)))
                throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                            HeaderFileNameStr);
            IsUSGSFile = true;
            }
        else
            pHeaderFileURL = GetURL();

        HAutoPtr<HFCBinStream> pHeaderFile;
        pHeaderFile = HFCBinStream::Instanciate(pHeaderFileURL, m_Offset, GetAccessMode(), 0, true);

        // Read header file
        if(GetHeaderFile(*pHeaderFile))
            {
            if (IsUSGSFile)
                {
                // Get red band number
                if (!GetBandNumber(m_ImgRedBand, RedFileNameStr))
                    throw HRFUSGSBandNotFoundInHeaderFileException(GetURL()->GetURL(),
                                                    L"RED");
                // Get green band number
                if (!GetBandNumber(m_ImgGreenBand, GreenFileNameStr))
                    throw HRFUSGSBandNotFoundInHeaderFileException(GetURL()->GetURL(),
                                                    L"GREEN");
                // Get blue band number
                if (!GetBandNumber(m_ImgBlueBand, BlueFileNameStr))
                    throw HRFUSGSBandNotFoundInHeaderFileException(GetURL()->GetURL(),
                                                    L"BLUE");
                }

            BeStringUtilities::CurrentLocaleCharToWChar( RedFileNameStr,m_Bands[m_ImgRedBand].m_FileName.c_str());

            if (!(pRedFileURL = ComposeChannelURL(rpURL, RedFileNameStr)))
                throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                            RedFileNameStr);

            // Open files read-only
            m_pRedFile = HFCBinStream::Instanciate(pRedFileURL, m_Offset, GetAccessMode(), 0, true);

            if (m_HeaderInfo.m_NumberOfBands > 2)
                {
                BeStringUtilities::CurrentLocaleCharToWChar( GreenFileNameStr,m_Bands[m_ImgGreenBand].m_FileName.c_str());
                BeStringUtilities::CurrentLocaleCharToWChar( BlueFileNameStr,m_Bands[m_ImgBlueBand].m_FileName.c_str());

                if (!(pGreenFileURL = ComposeChannelURL(rpURL, GreenFileNameStr)))
                    throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                                GreenFileNameStr);
                if (!(pBlueFileURL = ComposeChannelURL(rpURL, BlueFileNameStr)))
                    throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                                BlueFileNameStr);

                // Open files read-only
                m_pGreenFile = HFCBinStream::Instanciate(pGreenFileURL, m_Offset, GetAccessMode(), 0, true);

                m_pBlueFile  = HFCBinStream::Instanciate(pBlueFileURL, m_Offset, GetAccessMode(), 0, true);
                }

            m_IsOpen = true;
            }
        else
            throw HFCFileNotSupportedException(rpURL->GetURL());

        // Unlock the sister file
        SisterFileLock.ReleaseKey();
        }

    return m_IsOpen;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFUSgsNDFFile::GetCapabilities () const
    {
    return (HRFUSgsNDFCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Create or get the singleton capabilities of NDF file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFUSgsNDFCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFUSgsNDFCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// SetImageBand
// Set the RGB Band of the image.
//-----------------------------------------------------------------------------
void HRFUSgsNDFCreator::SetImageBand (int32_t pi_RedBand,
                                      int32_t pi_GreenBand,
                                      int32_t pi_BlueBand)
    {
    m_RedBand   = pi_RedBand;
    m_GreenBand = pi_GreenBand;
    m_BlueBand  = pi_BlueBand;
    }

//-----------------------------------------------------------------------------
// Public
// GetRedBand
// Get the Red Band of the image.
//-----------------------------------------------------------------------------
int32_t HRFUSgsNDFCreator::GetRedBand ()
    {
    return m_RedBand;
    }

//-----------------------------------------------------------------------------
// Public
// GetGreenBand
// Get the Green Band of the image.
//-----------------------------------------------------------------------------
int32_t HRFUSgsNDFCreator::GetGreenBand ()
    {
    return m_GreenBand;
    }

//-----------------------------------------------------------------------------
// Public
// GetBlueBand
// Get the Blue Band of the image.
//-----------------------------------------------------------------------------
int32_t HRFUSgsNDFCreator::GetBlueBand ()
    {
    return m_BlueBand;
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFUSgsNDFFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                            unsigned short pi_Resolution,
                                                            HFCAccessMode  pi_AccessMode)
    {
    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFUSgsNDFLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFUSgsNDFFile::Save()
    {
    HASSERT(!"HRFUSgsNDFFile::Save():USgsNDF format is read only");
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
// Create Raw File Descriptors
//-----------------------------------------------------------------------------
void HRFUSgsNDFFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);

    string                          pixelFormat;
    HFCPtr<HRPPixelType>            pPixelType;
    HFCPtr<HRFResolutionDescriptor> pResolution;
    HFCPtr<HRFPageDescriptor>       pPage;
    HFCPtr<HGF2DTransfoModel>       pTransfoModel;

    // Create pixel type
    pixelFormat = m_HeaderInfo.m_FileInfo.m_PixelInfo.m_PixelFormat;

    if (m_HeaderInfo.m_NumberOfBands < 3)
        {
        if (pixelFormat.compare("BYTE") == 0)
            pPixelType = new HRPPixelTypeV8Gray8();
        else
            pPixelType = new HRPPixelTypeV16Gray16();
        }
    else
        {
        if (pixelFormat.compare("BYTE") == 0)
            pPixelType = new HRPPixelTypeV24R8G8B8();
        else
            pPixelType = new HRPPixelTypeV48R16G16B16();
        }

    // Create Page and resolution Description/Capabilities for this file.
    pResolution =  new HRFResolutionDescriptor(
        HFC_READ_ONLY,                                      // AccessMode,
        GetCapabilities(),                                  // Capabilities,
        1.0,                                                // XResolutionRatio,
        1.0,                                                // YResolutionRatio,
        pPixelType,                                         // PixelType
        new HCDCodecIdentity(),                             // Codecs,
        HRFBlockAccess::SEQUENTIAL,                         // RBlockAccess,
        HRFBlockAccess::SEQUENTIAL,                         // WBlockAccess,
        m_HeaderInfo.m_FileInfo.m_DataOrientation,          // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                           // InterleaveType
        false,                                              // IsInterlace,
        m_HeaderInfo.m_FileInfo.m_Width,                    // Width,
        m_HeaderInfo.m_FileInfo.m_Height,                   // Height,
        m_HeaderInfo.m_FileInfo.m_Width,                    // BlockWidth,
        1,                                                  // BlockHeight,
        0,                                                  // BlocksDataFlag,
        HRFBlockType::LINE);                                 // BlockType

    // GeoRef Tag
    HFCPtr<HCPGeoTiffKeys> pGeoTiffKeys;

    GetGeoKeyTag(pGeoTiffKeys);

    // TranfoModel
    pTransfoModel = CreateTransfoModelFromNDF();

    pPage = new HRFPageDescriptor (HFC_READ_ONLY,
                                   GetCapabilities(),    // Capabilities,
                                   pResolution,          // ResolutionDescriptor,
                                   0,                    // RepresentativePalette,
                                   0,                    // Histogram,
                                   0,                    // Thumbnail,
                                   0,                    // ClipShape,
                                   pTransfoModel,        // TransfoModel,
                                   0);                   // Filters

    pPage->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pGeoTiffKeys.GetPtr()));

    m_ListOfPageDescriptor.push_back(pPage);
    }

//-----------------------------------------------------------------------------
// Private
// GetFileHeader
// Read header file
//-----------------------------------------------------------------------------
bool HRFUSgsNDFFile::GetHeaderFile(HFCBinStream& pi_rHeaderFile)
    {
    string  keyword;
    string  valueField;
    string  delimiter = ",";
    char   buffer    = ' ';
    char   endOfLine = ';';
    char   separator = '=';
    int32_t  nBand    = 0;
    bool   success   = true;
    bool   endOfFile = false;

    WString::size_type underScorePos;

    while(!endOfFile && success)
        {
        keyword = "";
        valueField = "";

        // Read keyword
        pi_rHeaderFile.Read(&buffer,sizeof buffer);
        while(buffer != separator && buffer != endOfLine)
            {
            keyword += (char)toupper(buffer);
            pi_rHeaderFile.Read(&buffer,sizeof buffer);
            }

        if (keyword.compare("END_OF_HDR") == 0)
            endOfFile = true;
        else
            {
            // Read valueField
            pi_rHeaderFile.Read(&buffer,sizeof buffer);
            while(buffer != endOfLine)
                {
                valueField += (char)toupper(buffer);
                pi_rHeaderFile.Read(&buffer,sizeof buffer);
                }

            if (keyword.compare("BITS_PER_PIXEL") == 0)
                m_HeaderInfo.m_FileInfo.m_PixelInfo.m_BitsPerPixel = atoi(valueField.c_str());
            else if (keyword.compare("PIXELS_PER_LINE") == 0)
                m_HeaderInfo.m_FileInfo.m_Width = atoi(valueField.c_str());
            else if (keyword.compare("LINES_PER_DATA_FILE") == 0)
                m_HeaderInfo.m_FileInfo.m_Height = atoi(valueField.c_str());
            else if (keyword.compare("NUMBER_OF_DATA_FILES") == 0)
                m_HeaderInfo.m_FileInfo.m_NumberOfDataFile = atoi(valueField.c_str());
            else if (keyword.compare("START_LINE_NUMBER") == 0)
                m_HeaderInfo.m_FileInfo.m_StartLineNumber = atoi(valueField.c_str());
            else if (keyword.compare("START_DATA_FILE") == 0)
                m_HeaderInfo.m_FileInfo.m_startDataFile = atoi(valueField.c_str());
            else if (keyword.compare("LINES_PER_VOLUME") == 0)
                m_HeaderInfo.m_FileInfo.m_LinesPerVolume = atoi(valueField.c_str());
            else if (keyword.compare("BLOCKING_FACTOR") == 0)
                m_HeaderInfo.m_FileInfo.m_BlockingFactor = atoi(valueField.c_str());
            else if (keyword.compare("RECORD_SIZE") == 0)
                m_HeaderInfo.m_FileInfo.m_RecordSize = atoi(valueField.c_str());
            else if (keyword.compare("REFERENCE_OFFSET") == 0)
                {
                m_HeaderInfo.m_RefInfo.m_Offset.m_X = (float)atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                m_HeaderInfo.m_RefInfo.m_Offset.m_Y = (float)atof(valueField.substr(valueField.find(delimiter)+1).c_str());
                }
            else if (keyword.compare("ORIENTATION") == 0)
                m_HeaderInfo.m_Orientation = atof(valueField.c_str());
            else if (keyword.compare("USGS_PROJECTION_NUMBER") == 0)
                m_HeaderInfo.m_USGSInfo.m_ProjNumber = atoi(valueField.c_str());
            else if (keyword.compare("USGS_MAP_ZONE") == 0)
                m_HeaderInfo.m_USGSInfo.m_MapZone = valueField;
            else if (keyword.compare("USGS_PROJECTION_PARAMETERS") == 0)
                {
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p1 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p2 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p3 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p4 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p5 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p6 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p7 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p8 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p9 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p10 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p11 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p12 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p13 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p14 = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_USGSInfo.m_ProjParameters.m_p15 = atof(valueField.c_str());
                }
            else if (keyword.compare("EARTH_ELLIPSOID_SEMI-MAJOR_AXIS") == 0)
                m_HeaderInfo.m_EarthEllipsoidInfo.m_SemiMajorAxis = (float)atof(valueField.c_str());
            else if (keyword.compare("EARTH_ELLIPSOID_SEMI-MINOR_AXIS") == 0)
                m_HeaderInfo.m_EarthEllipsoidInfo.m_SemiMinorAxis = (float)atof(valueField.c_str());
            else if (keyword.compare("EARTH_ELLIPSOID_ORIGIN_OFFSET") == 0)
                {
                m_HeaderInfo.m_EarthEllipsoidInfo.m_OriginOffsetInfo.m_X = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_EarthEllipsoidInfo.m_OriginOffsetInfo.m_Y = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_EarthEllipsoidInfo.m_OriginOffsetInfo.m_Z = atof(valueField.c_str());
                }
            else if (keyword.compare("EARTH_ELLIPSOID_ROTATION_OFFSET") == 0)
                {
                m_HeaderInfo.m_EarthEllipsoidInfo.m_RotationOffsetInfo.m_X = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_EarthEllipsoidInfo.m_RotationOffsetInfo.m_Y = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_EarthEllipsoidInfo.m_RotationOffsetInfo.m_Z = atof(valueField.c_str());
                }
            else if (keyword.compare("PIXEL_SPACING") == 0)
                {
                m_HeaderInfo.m_FileInfo.m_PixelInfo.m_PixelSpacingInfo.m_Horizontal = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                m_HeaderInfo.m_FileInfo.m_PixelInfo.m_PixelSpacingInfo.m_Vertical = atof(valueField.substr(valueField.find(delimiter)+1).c_str());
                }
            else if (keyword.compare("PROCESSING_LEVEL") == 0)
                {
                int32_t level = atoi(valueField.c_str());
                if (level > 0 && level < 11)
                    m_HeaderInfo.m_ProcessingInfo.m_Level = level;
                else
                    success = false;
                }
            else if (keyword.compare("SUN_ELEVATION") == 0)
                m_HeaderInfo.m_SunInfo.m_Elevation = (float)atof(valueField.c_str());
            else if (keyword.compare("SUN_AZIMUTH") == 0)
                m_HeaderInfo.m_SunInfo.m_Azimuth = (float)atof(valueField.c_str());
            else if (keyword.compare("NUMBER_OF_BANDS_IN_VOLUME") == 0)
                {
                m_HeaderInfo.m_NumberOfBands = atoi(valueField.c_str());
                m_Bands = new NDFBand[m_HeaderInfo.m_NumberOfBands];
                }
            else if (keyword.compare("NDF_REVISION") == 0)
                m_HeaderInfo.m_NDFRevision = valueField;
            else if (keyword.compare("DATA_SET_TYPE") == 0)
                {
                if (valueField.compare("EDC_MSS")   == 0 ||
                    valueField.compare("EDC_TM")    == 0 ||
                    valueField.compare("EDC_ETM+")  == 0 ||
                    valueField.compare("NLAPS_DEM") == 0)
                    m_HeaderInfo.m_DataSetType = valueField;
                else
                    success = false;
                }
            else if (keyword.compare("PRODUCT_NUMBER") == 0)
                m_HeaderInfo.m_ProductNumber = valueField;
            else if (keyword.compare("PIXEL_FORMAT") == 0)
                {
                if (valueField.compare("BYTE")     == 0 ||
                    valueField.compare("2BYTEINT") == 0)
                    m_HeaderInfo.m_FileInfo.m_PixelInfo.m_PixelFormat = valueField;
                else
                    success = false;
                }
            else if (keyword.compare("PIXEL_ORDER") == 0)
                {
                if (valueField.compare("NOT_INVERTED") == 0)
                    m_HeaderInfo.m_FileInfo.m_PixelInfo.m_PixelOrder = valueField;
                else
                    success = false;
                }
            else if (keyword.compare("DATA_ORIENTATION") == 0)
                {
                if (valueField.compare("UPPER_LEFT/RIGHT") == 0)
                    m_HeaderInfo.m_FileInfo.m_DataOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
                else
                    success = false;

                }
            else if (keyword.compare("DATA_FILE_INTERLEAVING") == 0
                     && valueField.compare("BSQ")              == 0)
                m_HeaderInfo.m_FileInfo.m_Interleaving = valueField;
            else if (keyword.compare("TAPE_SPANNING_FLAG") == 0)
                m_HeaderInfo.m_FileInfo.m_TapeSpanningFlag = valueField;
            else if (keyword.compare("UPPER_LEFT_CORNER") == 0)
                {
                m_HeaderInfo.m_CornerInfo.m_UpperLeft.m_Longitude = valueField.substr(0,valueField.find(delimiter));
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_CornerInfo.m_UpperLeft.m_Latitude = valueField.substr(0,valueField.find(delimiter));
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_CornerInfo.m_UpperLeft.m_Easting = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_CornerInfo.m_UpperLeft.m_Northing = atof(valueField.c_str());
                }
            else if (keyword.compare("UPPER_RIGHT_CORNER") == 0)
                {
                m_HeaderInfo.m_CornerInfo.m_UpperRight.m_Longitude = valueField.substr(0,valueField.find(delimiter));
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_CornerInfo.m_UpperRight.m_Latitude = valueField.substr(0,valueField.find(delimiter));
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_CornerInfo.m_UpperRight.m_Easting = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_CornerInfo.m_UpperRight.m_Northing = atof(valueField.c_str());
                }
            else if (keyword.compare("LOWER_LEFT_CORNER") == 0)
                {
                m_HeaderInfo.m_CornerInfo.m_LowerLeft.m_Longitude = valueField.substr(0,valueField.find(delimiter));
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_CornerInfo.m_LowerLeft.m_Latitude = valueField.substr(0,valueField.find(delimiter));
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_CornerInfo.m_LowerLeft.m_Easting = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_CornerInfo.m_LowerLeft.m_Northing = atof(valueField.c_str());
                }
            else if (keyword.compare("LOWER_RIGHT_CORNER") == 0)
                {
                m_HeaderInfo.m_CornerInfo.m_LowerRight.m_Longitude = valueField.substr(0,valueField.find(delimiter));
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_CornerInfo.m_LowerRight.m_Latitude = valueField.substr(0,valueField.find(delimiter));
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_CornerInfo.m_LowerRight.m_Easting = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_CornerInfo.m_LowerRight.m_Northing = atof(valueField.c_str());
                }
            else if (keyword.compare("REFERENCE_POINT") == 0)
                {
                if (valueField.compare("SCENE_CENTER") == 0 ||
                    valueField.compare("NONE")        == 0)
                    m_HeaderInfo.m_RefInfo.m_Point = valueField;
                else
                    success = false;
                }
            else if (keyword.compare("REFERENCE_POSITION") == 0)
                {
                m_HeaderInfo.m_RefInfo.m_Position.m_CoordSystem.m_Longitude = valueField.substr(0,valueField.find(delimiter));
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_RefInfo.m_Position.m_CoordSystem.m_Latitude = valueField.substr(0,valueField.find(delimiter));
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_RefInfo.m_Position.m_CoordSystem.m_Easting = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_RefInfo.m_Position.m_CoordSystem.m_Northing = atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_RefInfo.m_Position.m_Pixel = atoi(valueField.substr(0,valueField.find(delimiter)).c_str());
                valueField = valueField.substr(valueField.find(delimiter)+1);
                m_HeaderInfo.m_RefInfo.m_Position.m_Line = atoi(valueField.c_str());
                }
            else if (keyword.compare("MAP_PROJECTION_NAME") == 0)
                m_HeaderInfo.m_MapProjName = valueField;
            else if (keyword.compare("HORIZONTAL_DATUM") == 0)
                {
                if (valueField.compare("NAD27")     == 0 ||
                    valueField.compare("NAD83")     == 0 ||
                    valueField.compare("WGS84")     == 0 ||
                    valueField.compare("ELLIPSOID") == 0)
                    m_HeaderInfo.m_HorizontalDatum = valueField;
                else
                    success = false;
                }
            else if (keyword.compare("WRS") == 0)
                m_HeaderInfo.m_WRS = valueField;
            else if (keyword.compare("ACQUISITION_DATE/TIME") == 0)
                m_HeaderInfo.m_AcquisitionDateTime = valueField;
            else if (keyword.compare("SATELLITE") == 0)
                m_HeaderInfo.m_SatelliteInfo.m_SatNumber = valueField;
            else if (keyword.compare("SATELLITE_INSTRUMENT") == 0)
                {
                if (valueField.compare("TM")   == 0 ||
                    valueField.compare("MSS")  == 0 ||
                    valueField.compare("ETM+") == 0)
                    m_HeaderInfo.m_SatelliteInfo.m_SatInstrument = valueField;
                else
                    success = false;
                }
            else if (keyword.compare("PRODUCT_SIZE") == 0)
                {
                if (valueField.compare("FULL_SCENE")  == 0 ||
                    valueField.compare("SUBSCENE")    == 0 ||
                    valueField.compare("MULTI_SCENE") == 0)
                    m_HeaderInfo.m_ProductSize = valueField;
                else
                    success = false;
                }
            else if (keyword.compare("PIXEL_SPACING_UNITS") == 0)
                {
                if (valueField.compare("METERS") == 0)
                    m_HeaderInfo.m_FileInfo.m_PixelInfo.m_PixelSpacingInfo.m_Units = valueField;
                else
                    success = false;
                }
            else if (keyword.compare("RESAMPLING") == 0)
                m_HeaderInfo.m_Resampling = valueField;
            else if (keyword.compare("PROCESSING_DATE/TIME") == 0)
                m_HeaderInfo.m_ProcessingInfo.m_DateTime = valueField;
            else if (keyword.compare("PROCESSING_SOFTWARE") == 0)
                m_HeaderInfo.m_ProcessingInfo.m_Software = valueField;
            else if (keyword.substr(0,4).compare("BAND") == 0)
                {
                underScorePos = keyword.find("_");
                nBand = atoi(keyword.substr(4,underScorePos).c_str());
                --nBand;
                keyword = keyword.substr(underScorePos+1);
                if (keyword.compare("NAME") == 0)
                    m_Bands[nBand].m_Name = valueField;
                else if (keyword.compare("FILENAME") == 0)
                    m_Bands[nBand].m_FileName = valueField;
                else if (keyword.compare("WAVELENGTHS") == 0)
                    {
                    m_Bands[nBand].m_WaveLengths.m_Min = (float)atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                    m_Bands[nBand].m_WaveLengths.m_Max = (float)atof(valueField.substr(valueField.find(delimiter)+1).c_str());
                    }
                else if (keyword.compare("RADIOMETRIC_GAINS/BIAS") == 0)
                    {
                    m_Bands[nBand].m_Radiometric.m_Gain = (float)atof(valueField.substr(0,valueField.find(delimiter)).c_str());
                    m_Bands[nBand].m_Radiometric.m_Bias = (float)atof(valueField.substr(valueField.find(delimiter)+1).c_str());
                    }
                }
            else
                success = false;

            // Read end of line
            pi_rHeaderFile.Read(&buffer,sizeof buffer);
            }
        }

    return success;
    }

//-----------------------------------------------------------------------------
// Private
// CreateTransfoModelFromNDF
// Create a HGF2DTransfoModel using the infomation found in the NDF file Header.
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFUSgsNDFFile::CreateTransfoModelFromNDF()
    {
    HFCPtr<HGF2DTransfoModel> pTransfoModel = new HGF2DIdentity();
    double pixelCenter = 0.5;

    // Create Matrix
    double pMatrix[4][4];
    double pTiePoints[24];
    HFCMatrix<3, 3> theMatrix;

    pTiePoints[0]  = pixelCenter;
    pTiePoints[1]  = pixelCenter;
    pTiePoints[3]  = m_HeaderInfo.m_CornerInfo.m_UpperLeft.m_Easting;
    pTiePoints[4]  = m_HeaderInfo.m_CornerInfo.m_UpperLeft.m_Northing;

    pTiePoints[6]  = m_HeaderInfo.m_FileInfo.m_Width - pixelCenter;
    pTiePoints[7]  = pixelCenter;
    pTiePoints[9]  = m_HeaderInfo.m_CornerInfo.m_UpperRight.m_Easting;
    pTiePoints[10] = m_HeaderInfo.m_CornerInfo.m_UpperRight.m_Northing;

    pTiePoints[12] = pixelCenter;
    pTiePoints[13] = m_HeaderInfo.m_FileInfo.m_Height - pixelCenter;
    pTiePoints[15] = m_HeaderInfo.m_CornerInfo.m_LowerLeft.m_Easting;
    pTiePoints[16] = m_HeaderInfo.m_CornerInfo.m_LowerLeft.m_Northing;

    pTiePoints[18] = m_HeaderInfo.m_FileInfo.m_Width  - pixelCenter;
    pTiePoints[19] = m_HeaderInfo.m_FileInfo.m_Height - pixelCenter;
    pTiePoints[21] = m_HeaderInfo.m_CornerInfo.m_LowerRight.m_Easting;
    pTiePoints[22] = m_HeaderInfo.m_CornerInfo.m_LowerRight.m_Northing;

    GetTransfoMatrixFromScaleAndTiePts (pMatrix, 24, pTiePoints, 0, NULL);

    theMatrix[0][0] = pMatrix[0][0];
    theMatrix[0][1] = pMatrix[0][1];
    theMatrix[0][2] = pMatrix[0][3];
    theMatrix[1][0] = pMatrix[1][0];
    theMatrix[1][1] = pMatrix[1][1];
    theMatrix[1][2] = pMatrix[1][3];
    theMatrix[2][0] = pMatrix[3][0];
    theMatrix[2][1] = pMatrix[3][1];
    theMatrix[2][2] = pMatrix[3][3];

    pTransfoModel = new HGF2DProjective(theMatrix);

    // Get the simplest model possible.
    HFCPtr<HGF2DTransfoModel> pTempTransfoModel = pTransfoModel->CreateSimplifiedModel();

    if (pTempTransfoModel != 0)
        pTransfoModel = pTempTransfoModel;

    // Flip the Y Axe because the origin of ModelSpace is lower-left
    HFCPtr<HGF2DStretch> pFlipModel = new HGF2DStretch();

    pFlipModel->SetYScaling(-1.0);
    pTransfoModel = pFlipModel->ComposeInverseWithDirectOf(*pTransfoModel);

    return pTransfoModel;
    }

//-----------------------------------------------------------------------------
// Private
// GetGeoKeyTag
// Get the geokey tag.
//-----------------------------------------------------------------------------
void HRFUSgsNDFFile::GetGeoKeyTag(HFCPtr<HCPGeoTiffKeys>& po_rpGeoTiffKeys)
    {
    HASSERT(po_rpGeoTiffKeys == 0);

    short Code;

    po_rpGeoTiffKeys = new HCPGeoTiffKeys();

    // GTModelType
    po_rpGeoTiffKeys->AddKey(GTModelType, (uint32_t)TIFFGeo_ModelTypeProjected);

    // GTRasterType
    po_rpGeoTiffKeys->AddKey(GTRasterType, (uint32_t)TIFFGeo_RasterPixelIsArea);

    // ProjectedCSType
    if (GetCoordSystem(Code) != TIFFGeo_Undefined)
        {
        HASSERT(Code >= 0);
        po_rpGeoTiffKeys->AddKey(ProjectedCSType, (uint32_t)Code);
        }

    // GeogLinearUnits
    po_rpGeoTiffKeys->AddKey(GeogLinearUnits, (uint32_t)TIFFGeo_Linear_Meter);
    }

//-----------------------------------------------------------------------------
// Private
// GetBandNumber
// Get the band number of band filename
//-----------------------------------------------------------------------------
bool HRFUSgsNDFFile::GetBandNumber(int32_t& pio_rBand,
                                    const WString& pi_rFileName)
    {
    bool Result = false;
    WString FileNameStr;

    for(int32_t i = 0; i < m_HeaderInfo.m_NumberOfBands && !Result; i++)
        {
        BeStringUtilities::CurrentLocaleCharToWChar( FileNameStr,m_Bands[i].m_FileName.c_str());

        if (BeStringUtilities::Wcsicmp(pi_rFileName.c_str(), FileNameStr.c_str()) == 0)
            {
            pio_rBand = i;
            Result = true;
            }
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// Private
// GetCoordSystem
// Get the coordonate system code
//-----------------------------------------------------------------------------
bool HRFUSgsNDFFile::GetCoordSystem(short& po_rCode)
    {
    bool Found = false;

    po_rCode = TIFFGeo_Undefined;

    HRFGeoTiffCoordSysTable*  pGeoTiffCoordSysTable = HRFGeoTiffCoordSysTable::GetInstance();
    pGeoTiffCoordSysTable->LockTable();

    string Datum    = m_HeaderInfo.m_HorizontalDatum;
    string ProjName = m_HeaderInfo.m_MapProjName;
    string Zone     = m_HeaderInfo.m_USGSInfo.m_MapZone;
    HRFGeoTiffCoordSysTable::HRFGeoTiffCoordSysRecord CoordSysRecord;

    // Search Coordonate System with original Datum (ex: NAD83)
    Found = pGeoTiffCoordSysTable->GetRecord(Datum,
                                             ProjName,
                                             Zone,
                                             &CoordSysRecord);

    if (!Found && Datum.compare("ELLIPSOID") != 0)
        {
        // Search Coordonate System with space between name and number Datum (ex: NAD 83)
        Datum = Datum.substr(0,3) + " " +Datum.substr(3);
        Found = pGeoTiffCoordSysTable->GetRecord(Datum,
                                                 ProjName,
                                                 Zone,
                                                 &CoordSysRecord);
        }

    if(Found)
        po_rCode = CoordSysRecord.CSCode;

    pGeoTiffCoordSysTable->ReleaseTable();

    return Found;
    }
