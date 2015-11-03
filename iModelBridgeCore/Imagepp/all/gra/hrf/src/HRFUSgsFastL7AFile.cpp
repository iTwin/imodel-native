//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFUSgsFastL7AFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HTIFFFile.h>
#include <Imagepp/all/h/HRFUSgsFastL7AFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFUSgsFastL7ALineEditor.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>

#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DProjective.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/HCPGeoTiffKeys.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include "HRFGeoTiffTable.h"

#include <BeXml/BeXml.h>



//-----------------------------------------------------------------------------
// HRFUSgsFastL7ABlockCapabilities
//-----------------------------------------------------------------------------
class HRFUSgsFastL7ABlockCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFUSgsFastL7ABlockCapabilities ()
        : HRFRasterFileCapabilities()
        {
        // Block capability
        Add (new HRFLineCapability (HFC_READ_ONLY,
                                    LONG_MAX,
                                    HRFBlockAccess::SEQUENTIAL));
        }
    };

//-----------------------------------------------------------------------------
// HRFUSgsFastL7ACodecCapabilities
//-----------------------------------------------------------------------------
class HRFUSgsFastL7ACodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFUSgsFastL7ACodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_ONLY,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFUSgsFastL7ABlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFUSgsFastL7ACapabilities
//-----------------------------------------------------------------------------
HRFUSgsFastL7ACapabilities::HRFUSgsFastL7ACapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV8Gray8
    Add (new HRFPixelTypeCapability (HFC_READ_ONLY,
                                     HRPPixelTypeV8Gray8::CLASS_ID,
                                     new HRFUSgsFastL7ACodecCapabilities()));

    // PixelTypeV24R8G8B8
    Add (new HRFPixelTypeCapability (HFC_READ_ONLY,
                                     HRPPixelTypeV24R8G8B8::CLASS_ID,
                                     new HRFUSgsFastL7ACodecCapabilities()));

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

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);
    }

HFC_IMPLEMENT_SINGLETON(HRFUSgsFastL7ACreator);

//-----------------------------------------------------------------------------
// Creator
// This is the creator to instantiate FastL7A format
//-----------------------------------------------------------------------------
HRFUSgsFastL7ACreator::HRFUSgsFastL7ACreator()
    : HRFRasterFileCreator(HRFUSgsFastL7AFile::CLASS_ID)
    {
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFUSgsFastL7ACreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_USGS_FastL7A());  // USGS FastL7A File Format
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFUSgsFastL7ACreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFUSgsFastL7ACreator::GetExtensions() const
    {
    return WString(L"*.dat;*.fst;*.usgs");
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// Allow to Open an image file
//-----------------------------------------------------------------------------

HRFUSgsFastL7AFile::HRFUSgsFastL7AFile(const HFCPtr<HFCURL>& pi_rURL,
                                              HFCAccessMode         pi_AccessMode,
                                              uint64_t             pi_Offset)

    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    m_IsOpen       = false;
    m_ImgRedBand   = 0;
    m_ImgGreenBand = 1;
    m_ImgBlueBand  = 2;

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
const HGF2DWorldIdentificator HRFUSgsFastL7AFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

//-----------------------------------------------------------------------------
// Public
// SetDefaultRatioToMeter
// Set the default ratio to meter specified by the user, if this ratio cannot
// be deduced from the file metadata.
//-----------------------------------------------------------------------------
void HRFUSgsFastL7AFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                       uint32_t pi_Page,
                                                       bool   pi_CheckSpecificUnitSpec,
                                                       bool   pi_InterpretUnitINTGR)
    {
    //The following key and its value is always added to the page descriptor :
    //GeogLinearUnits - TIFFGeo_Linear_Meter
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFUSgsFastL7AFile::HRFUSgsFastL7AFile(const HFCPtr<HFCURL>& pi_rURL,
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
HRFUSgsFastL7AFile::~HRFUSgsFastL7AFile()
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
HFCPtr<HRFRasterFile> HRFUSgsFastL7ACreator::Create(
    const HFCPtr<HFCURL>& pi_rpURL,
    HFCAccessMode         pi_AccessMode,
    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(pi_Offset >= 0);

    HFCPtr<HRFRasterFile> pFile = new HRFUSgsFastL7AFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Public
// IsKindOfFile
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFUSgsFastL7ACreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                          uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    bool   Result = false;

    HFCPtr<HFCURLFile>& rpURL((HFCPtr<HFCURLFile>&)pi_rpURL);

    (const_cast<HRFUSgsFastL7ACreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    if (BeStringUtilities::Wcsicmp(rpURL->GetExtension().c_str(), L"usgs") == 0)
        {
        WString USGSFileName;

        // Extract the standard file name from the main URL
        USGSFileName =  rpURL->GetHost();
        USGSFileName += L"\\";
        USGSFileName += rpURL->GetPath();

        // Open XML file
        BeXmlStatus xmlStatus;
        BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, USGSFileName.c_str ());

        //Validate pDom
        if (pXmlDom.IsNull() || (BEXML_Success != xmlStatus))
            goto WRAPUP;

        // Read data in XML file (strings)
        BeXmlNodeP pMainNode = pXmlDom->GetRootElement();
        if (NULL == pMainNode || BeStringUtilities::Stricmp (pMainNode->GetName(), "MultiChannelImageFileFormat") != 0)
            goto WRAPUP;

        // Validate VERSION node presence (for now only validate node presence)
        BeXmlNodeP pVersionNode = pMainNode->SelectSingleNode("VERSION");
        if(NULL == pVersionNode)
            goto WRAPUP;

        // Validate HEADER node presence
        WString HeaderFileStr;
        BeXmlNodeP pHeaderNode = pMainNode->SelectSingleNode("HEADER");
        if(NULL == pHeaderNode || BEXML_Success != pHeaderNode->GetContent(HeaderFileStr))
            goto WRAPUP;

        WString HeaderExtensionStr = HeaderFileStr.substr(HeaderFileStr.find(L".")+1);
        if (BeStringUtilities::Wcsicmp(HeaderExtensionStr.c_str(), L"fst") != 0)
            goto WRAPUP;

        // Validate CHANNELS node presence
        BeXmlNodeP pChannelNode = pMainNode->SelectSingleNode("CHANNELS");
        if (NULL == pChannelNode)
            goto WRAPUP;

        // Validate COUNT node presence
        BeXmlNodeP pCountNode = pChannelNode->SelectSingleNode ("COUNT");
        if (NULL == pCountNode)
            goto WRAPUP;

        // Validate channel count, must be 3 (type = UInt32)
        uint32_t ChannelCount = 0;
        if(BEXML_Success != pCountNode->GetContentUInt32Value (ChannelCount) || ChannelCount != 3)
            goto WRAPUP;

        // Validate RED node presence
        BeXmlNodeP pRedNode = pChannelNode->SelectSingleNode ("RED");
        if (NULL == pRedNode)
            goto WRAPUP;

        // Validate GREEN node presence
        BeXmlNodeP pGreenNode = pChannelNode->SelectSingleNode ("GREEN");
        if (NULL == pGreenNode)
            goto WRAPUP;

        // Validate BLUE node presence
        BeXmlNodeP pBlueNode = pChannelNode->SelectSingleNode ("BLUE");
        if (NULL == pBlueNode)
            goto WRAPUP;

        Result = true;
        }
    else
        {
        HAutoPtr<HFCBinStream>      pFile;
        char buffer[8+1];

        // Open the FastL7A Header File and place file pointer at the start of the file
        pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

        if (pFile == 0 || pFile->GetLastException() != 0)
            goto WRAPUP;

        if (pFile->Read(&buffer,8) != 8)
            goto WRAPUP;

        if (BeStringUtilities::Strnicmp(buffer, "REQ ID =",8) == 0)
            Result = true;
        }

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFUSgsFastL7ACreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFUSgsFastL7ACreator*>(this))->m_pSharingControl = 0;

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
bool HRFUSgsFastL7AFile::Open()
    {
    WString HeaderFileNameStr;
    WString RedFileNameStr;
    WString GreenFileNameStr;
    WString BlueFileNameStr;

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
            WString USGSFileName;

            // Extract the standard file name from the main URL
            USGSFileName =  rpURL->GetHost();
            USGSFileName += L"\\";
            USGSFileName += rpURL->GetPath();

            // Open USGS file
            BeXmlStatus xmlStatus;
            BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, USGSFileName.c_str());
            if (pXmlDom.IsNull() || (BEXML_Success != xmlStatus))
                throw HFCFileNotSupportedException(rpURL->GetURL());
    
            // Read data in USGS file (strings)
            BeXmlNodeP pHeaderNode       = pXmlDom->GetRootElement()->SelectSingleNode("HEADER");
            BeXmlNodeP pRedChannelNode   = pXmlDom->GetRootElement()->SelectSingleNode("CHANNELS/RED");
            BeXmlNodeP pGreenChannelNode = pXmlDom->GetRootElement()->SelectSingleNode("CHANNELS/GREEN");
            BeXmlNodeP pBlueChannelNode  = pXmlDom->GetRootElement()->SelectSingleNode("CHANNELS/BLUE");

            if(NULL == pHeaderNode || NULL == pRedChannelNode || NULL == pGreenChannelNode || NULL == pBlueChannelNode)
                throw HFCFileNotSupportedException(rpURL->GetURL());

            
            pHeaderNode->GetContent(HeaderFileNameStr);
            pRedChannelNode->GetContent(RedFileNameStr);
            pGreenChannelNode->GetContent(GreenFileNameStr);
            pBlueChannelNode->GetContent(BlueFileNameStr);

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

            BeStringUtilities::CurrentLocaleCharToWChar( RedFileNameStr, m_HeaderInfo.m_Bands[m_ImgRedBand].m_FileName);


            if (!(pRedFileURL = ComposeChannelURL(rpURL, RedFileNameStr)))
                throw HRFCannotOpenChildFileException(GetURL()->GetURL(),
                                            RedFileNameStr);

            // Open files read-only
            m_pRedFile = HFCBinStream::Instanciate(pRedFileURL, m_Offset, GetAccessMode(), 0, true);

            if (m_HeaderInfo.m_NumberOfBands > 2)
                {
                BeStringUtilities::CurrentLocaleCharToWChar( GreenFileNameStr, m_HeaderInfo.m_Bands[m_ImgGreenBand].m_FileName);
                BeStringUtilities::CurrentLocaleCharToWChar( BlueFileNameStr, m_HeaderInfo.m_Bands[m_ImgBlueBand].m_FileName);

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
            throw(HFCFileNotSupportedException(rpURL->GetURL()));

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
const HFCPtr<HRFRasterFileCapabilities>& HRFUSgsFastL7AFile::GetCapabilities () const
    {
    return (HRFUSgsFastL7ACreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Create or get the singleton capabilities of FastL7A file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFUSgsFastL7ACreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFUSgsFastL7ACapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFUSgsFastL7AFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                                unsigned short pi_Resolution,
                                                                HFCAccessMode  pi_AccessMode)
    {
    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFUSgsFastL7ALineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFUSgsFastL7AFile::Save()
    {
    HASSERT(!"HRFUSgsFastL7AFile::Save():USgsFastL7A format is read only");
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
// Create Raw File Descriptors
//-----------------------------------------------------------------------------
void HRFUSgsFastL7AFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);

    string                          pixelFormat;
    HFCPtr<HRPPixelType>            pPixelType;
    HFCPtr<HRFResolutionDescriptor> pResolution;
    HFCPtr<HRFPageDescriptor>       pPage;
    HFCPtr<HGF2DTransfoModel>       pTransfoModel;
    HFCPtr<HCPGeoTiffKeys>          pGeoTiffKeys;

    // Create pixel type
    if (m_HeaderInfo.m_NumberOfBands < 3)
        pPixelType = new HRPPixelTypeV8Gray8();
    else
        pPixelType = new HRPPixelTypeV24R8G8B8();

    // Create Page and resolution Description/Capabilities for this file.
    pResolution =  new HRFResolutionDescriptor(
        HFC_READ_ONLY,                                      // AccessMode,
        GetCapabilities(),                                  // Capabilities,
        1.0,                                                // XResolutionRatio,
        1.0,                                                // YResolutionRatio,
        pPixelType,                                         // PixelType
        new HCDCodecIdentity(),                             // Codec,
        HRFBlockAccess::SEQUENTIAL,                         // RBlockAccess,
        HRFBlockAccess::SEQUENTIAL,                         // WBlockAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,      // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                           // InterleaveType
        false,                                              // IsInterlace,
        m_HeaderInfo.m_AdminInfo.m_PixelPerLine,            // Width,
        m_HeaderInfo.m_AdminInfo.m_LineInOutput,            // Height,
        m_HeaderInfo.m_AdminInfo.m_PixelPerLine,            // BlockWidth,
        1,                                                  // BlockHeight,
        0,                                                  // BlocksDataFlag,
        HRFBlockType::LINE);                                 // BlockType

    // GeoRef Tag
    GetGeoKeyTag(pGeoTiffKeys);

    // TranfoModel
    pTransfoModel = CreateTransfoModelFromFastL7A();

    pPage = new HRFPageDescriptor (HFC_READ_ONLY,
                                   GetCapabilities(),    // Capabilities,
                                   pResolution,          // ResolutionDescriptor,
                                   0,                    // RepresentativePalette,
                                   0,                    // Histogram,
                                   0,                    // Thumbnail,
                                   0,                    // ClipShape,
                                   pTransfoModel,        // TransfoModel,
                                   0);                   // Filters


    pPage->InitFromRasterFileGeocoding(*RasterFileGeocoding::Create(pGeoTiffKeys));

    m_ListOfPageDescriptor.push_back(pPage);
    }

//-----------------------------------------------------------------------------
// Private
// GetFileHeader
// Read header file
//-----------------------------------------------------------------------------
bool HRFUSgsFastL7AFile::GetHeaderFile(HFCBinStream& pi_rHeaderFile)
    {
    bool Success = false;

    string FileNameEmpty = "";

    char TagName[100];
    char Value[100];
    char Blank[1000];
    int32_t i;

    memset(&m_HeaderInfo, 0, sizeof m_HeaderInfo);

    m_HeaderInfo.m_NumberOfBands = 0;

    // Read General information
    pi_rHeaderFile.Read(&TagName,8);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_ReqId, sizeof m_HeaderInfo.m_AdminInfo.m_ReqId-1);
    pi_rHeaderFile.Read(&TagName,6);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_Loc, sizeof m_HeaderInfo.m_AdminInfo.m_Loc-1);
    pi_rHeaderFile.Read(&TagName,19);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_AcquisitionDate, sizeof m_HeaderInfo.m_AdminInfo.m_AcquisitionDate-1);
    pi_rHeaderFile.Read(&Blank,2);

    // Read Satellite information
    for (i = 0; i < 3; i++)
        {
        pi_rHeaderFile.Read(&TagName,11);
        pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_Satellite, sizeof m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_Satellite-1);
        pi_rHeaderFile.Read(&TagName,9);
        pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_Sensor, sizeof m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_Sensor-1);
        pi_rHeaderFile.Read(&TagName,14);
        pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_SensorMode, sizeof m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_SensorMode-1);
        pi_rHeaderFile.Read(&TagName,13);
        pi_rHeaderFile.Read(&Value, 6);
        m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_LookAngle = (float)atof(Value);
        pi_rHeaderFile.Read(&Blank,24);
        pi_rHeaderFile.Read(&TagName,11);
        pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_Location, sizeof m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_Location-1);
        pi_rHeaderFile.Read(&TagName,19);
        pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_AcquisitionDate, sizeof m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_AcquisitionDate-1);
        pi_rHeaderFile.Read(&Blank,2);
        }

    // Validate first Satellite information
    if (BeStringUtilities::Stricmp(m_HeaderInfo.m_AdminInfo.m_Satellite[0].m_Satellite, "landsat7  ") != 0
        || BeStringUtilities::Stricmp(m_HeaderInfo.m_AdminInfo.m_Satellite[0].m_Sensor, "etm+      ") != 0
        || BeStringUtilities::Stricmp(m_HeaderInfo.m_AdminInfo.m_Satellite[0].m_SensorMode, "normal") != 0
        || m_HeaderInfo.m_AdminInfo.m_Satellite[0].m_LookAngle != 0.0)
        goto WRAPUP;


    // Read last Satellite information
    pi_rHeaderFile.Read(&TagName,11);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_Satellite, sizeof m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_Satellite-1);
    pi_rHeaderFile.Read(&TagName,9);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_Sensor, sizeof m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_Sensor-1);
    pi_rHeaderFile.Read(&TagName,14);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_SensorMode, sizeof m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_SensorMode-1);
    pi_rHeaderFile.Read(&TagName,13);
    pi_rHeaderFile.Read(&Value, 6);
    m_HeaderInfo.m_AdminInfo.m_Satellite[i].m_LookAngle = (float)atof(Value);
    pi_rHeaderFile.Read(&Blank,1);

    // Read and validate ProductType
    pi_rHeaderFile.Read(&TagName,14);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_ProductType, sizeof m_HeaderInfo.m_AdminInfo.m_ProductType-1);
    if (BeStringUtilities::Stricmp(m_HeaderInfo.m_AdminInfo.m_ProductType, "map oriented      ")    != 0
        && BeStringUtilities::Stricmp(m_HeaderInfo.m_AdminInfo.m_ProductType, "orbit oriented    ") != 0)
        goto WRAPUP;

    //Read and validate ProductSize
    pi_rHeaderFile.Read(&TagName,15);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_ProductSize, sizeof m_HeaderInfo.m_AdminInfo.m_ProductSize-1);
    if (BeStringUtilities::Stricmp(m_HeaderInfo.m_AdminInfo.m_ProductSize, "full scene")    != 0
        && BeStringUtilities::Stricmp(m_HeaderInfo.m_AdminInfo.m_ProductSize, "subscene  ") != 0
        && BeStringUtilities::Stricmp(m_HeaderInfo.m_AdminInfo.m_ProductSize, "multiscene") != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Blank,23);

    // Read and validate TypeOfProcessing
    pi_rHeaderFile.Read(&TagName,20);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_TypeOfProcessing, sizeof m_HeaderInfo.m_AdminInfo.m_TypeOfProcessing-1);
    if (BeStringUtilities::Stricmp(m_HeaderInfo.m_AdminInfo.m_TypeOfProcessing, "systematic ") != 0)
        goto WRAPUP;

    //Read and validate Resampling
    pi_rHeaderFile.Read(&TagName,13);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_Resampling, sizeof m_HeaderInfo.m_AdminInfo.m_Resampling-1);
    if (BeStringUtilities::Stricmp(m_HeaderInfo.m_AdminInfo.m_Resampling, "cc")    != 0
        && BeStringUtilities::Stricmp(m_HeaderInfo.m_AdminInfo.m_Resampling, "nn") != 0
        && BeStringUtilities::Stricmp(m_HeaderInfo.m_AdminInfo.m_Resampling, "mf") != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Blank,34);

    // Read Band General information
    pi_rHeaderFile.Read(&TagName,19);
    pi_rHeaderFile.Read(&Value,2);
    m_HeaderInfo.m_AdminInfo.m_VolumeNumber = atoi(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,2);
    m_HeaderInfo.m_AdminInfo.m_NumberOfVolume = atoi(Value);
    pi_rHeaderFile.Read(&TagName,18);
    pi_rHeaderFile.Read(&Value,5);
    m_HeaderInfo.m_AdminInfo.m_PixelPerLine = atoi(Value);
    pi_rHeaderFile.Read(&TagName,17);
    if (BeStringUtilities::Strnicmp(TagName, " lines per band =", 17) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Value,5);
    m_HeaderInfo.m_AdminInfo.m_LinePerPanBand = atoi(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,5);
    m_HeaderInfo.m_AdminInfo.m_LineInOutput = atoi(Value);
    pi_rHeaderFile.Read(&Blank,5);
    pi_rHeaderFile.Read(&TagName,14);
    if (BeStringUtilities::Strnicmp(TagName, "start line # =", 14) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Value,5);
    m_HeaderInfo.m_AdminInfo.m_StartLineNumber = atoi(Value);
    pi_rHeaderFile.Read(&TagName,18);
    pi_rHeaderFile.Read(&Value,2);
    m_HeaderInfo.m_AdminInfo.m_BlockingFactor = atoi(Value);
    pi_rHeaderFile.Read(&TagName,12);
    pi_rHeaderFile.Read(&Value,9);
    m_HeaderInfo.m_AdminInfo.m_RecSize = atoi(Value);
    pi_rHeaderFile.Read(&TagName,13);
    pi_rHeaderFile.Read(&Value,6);
    m_HeaderInfo.m_AdminInfo.m_PixelSize = (float)atof(Value);
    pi_rHeaderFile.Read(&Blank,1);

    // Read and validate OutputBitsPerPixel
    pi_rHeaderFile.Read(&TagName,23);
    pi_rHeaderFile.Read(&Value,2);
    m_HeaderInfo.m_AdminInfo.m_OutputBitsPerPixel = atoi(Value);
    if (m_HeaderInfo.m_AdminInfo.m_OutputBitsPerPixel != 8)
        goto WRAPUP;

    // Read and validate AcquiredBitsPerPixel
    pi_rHeaderFile.Read(&TagName,26);
    pi_rHeaderFile.Read(&Value,2);
    m_HeaderInfo.m_AdminInfo.m_AcquiredBitsPerPixel = atoi(Value);
    if (m_HeaderInfo.m_AdminInfo.m_AcquiredBitsPerPixel != 8)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Blank,27);

    pi_rHeaderFile.Read(&TagName,15);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_AdminInfo.m_BandsPresent, sizeof m_HeaderInfo.m_AdminInfo.m_BandsPresent-1);
    pi_rHeaderFile.Read(&Blank,33);

    // Create FileNameEmpty string
    for (i = 0; i < 29; i++)
        FileNameEmpty += " ";

    // Read Band FileName
    for (i = 0; i < 6; i+=2)
        {
        pi_rHeaderFile.Read(&TagName,10);
        if (BeStringUtilities::Strnicmp(TagName, "filename =", 10) != 0)
            goto WRAPUP;
        pi_rHeaderFile.Read(&m_HeaderInfo.m_Bands[i].m_FileName, sizeof m_HeaderInfo.m_Bands[i].m_FileName-1);
        if (strcmp(m_HeaderInfo.m_Bands[i].m_FileName, FileNameEmpty.c_str()) != 0)
            ++m_HeaderInfo.m_NumberOfBands;
        pi_rHeaderFile.Read(&TagName,10);
        pi_rHeaderFile.Read(&m_HeaderInfo.m_Bands[i+1].m_FileName, sizeof m_HeaderInfo.m_Bands[i+1].m_FileName-1);
        if (strcmp(m_HeaderInfo.m_Bands[i+1].m_FileName, FileNameEmpty.c_str()) != 0)
            ++m_HeaderInfo.m_NumberOfBands;
        pi_rHeaderFile.Read(&Blank,2);
        }

    pi_rHeaderFile.Read(&Blank,160);

    // Read and validate Revision
    pi_rHeaderFile.Read(&TagName,12);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_Revision, sizeof m_HeaderInfo.m_Revision-1);
    if (BeStringUtilities::Stricmp(m_HeaderInfo.m_Revision, "l7a") != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Blank,1);

    pi_rHeaderFile.Read(&TagName,50);
    if (BeStringUtilities::Strnicmp(TagName, "biases and gains in ascending band number order   ", 50)          != 0
        && BeStringUtilities::Strnicmp(TagName, "gains and biases gains in ascending band number order   ", 50) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Blank,30);

    // Read Bias and Gain of each Band
    for (i = 0; i < 8; i++)
        {
        pi_rHeaderFile.Read(&Value,24);
        m_HeaderInfo.m_Bands[i].m_Radiometric.m_Bias = atof(Value);
        pi_rHeaderFile.Read(&Blank,1);
        pi_rHeaderFile.Read(&Value,24);
        m_HeaderInfo.m_Bands[i].m_Radiometric.m_Gain = atof(Value);
        pi_rHeaderFile.Read(&Blank,31);
        }

    pi_rHeaderFile.Read(&Blank,816);

    // Read Geometric Data
    pi_rHeaderFile.Read(&TagName,14);
    if (BeStringUtilities::Strnicmp(TagName, "geometric data", 14) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&TagName,17);
    if (BeStringUtilities::Strnicmp(TagName, " map projection =", 17) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_ProjectionName, sizeof m_HeaderInfo.m_GeoInfo.m_ProjectionName-1);
    pi_rHeaderFile.Read(&TagName,12);
    if (BeStringUtilities::Strnicmp(TagName, " ellipsoid =", 12) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_Ellipsoid, sizeof m_HeaderInfo.m_GeoInfo.m_Ellipsoid-1);
    if (BeStringUtilities::Stricmp(m_HeaderInfo.m_GeoInfo.m_Ellipsoid, "wgs84             ") != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&TagName,8);
    if (BeStringUtilities::Strnicmp(TagName, " datum =", 8) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_Datum, sizeof m_HeaderInfo.m_GeoInfo.m_Datum-1);
    if (BeStringUtilities::Stricmp(m_HeaderInfo.m_GeoInfo.m_Datum, "wgs84 ") != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Blank,1);

    // Read USGS Projection Parameters
    pi_rHeaderFile.Read(&TagName,28);
    if (BeStringUtilities::Strnicmp(TagName, "usgs projection parameters =", 28) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p1 = atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p2 = atof(Value);
    pi_rHeaderFile.Read(&Blank,2);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p3 = atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p4 = atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p5 = atof(Value);
    pi_rHeaderFile.Read(&Blank,6);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p6 = atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p7 = atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p8 = atof(Value);
    pi_rHeaderFile.Read(&Blank,6);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p9 = atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p10 = atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p11 = atof(Value);
    pi_rHeaderFile.Read(&Blank,6);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p12 = atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p13 = atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p14 = atof(Value);
    pi_rHeaderFile.Read(&Blank,6);
    pi_rHeaderFile.Read(&Value,24);
    m_HeaderInfo.m_GeoInfo.m_ParamInfo.m_p15 = atof(Value);
    pi_rHeaderFile.Read(&Blank,1);

    pi_rHeaderFile.Read(&TagName,15);
    if (BeStringUtilities::Strnicmp(TagName, "usgs map zone =", 15) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_MapZone, sizeof m_HeaderInfo.m_GeoInfo.m_MapZone-1);
    pi_rHeaderFile.Read(&Blank,34);

    // Read Upper Left Corner information
    pi_rHeaderFile.Read(&TagName,4);
    if (BeStringUtilities::Strnicmp(TagName, "ul =", 4) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperLeft.m_Longitude, sizeof m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperLeft.m_Longitude-1);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperLeft.m_Latitude, sizeof m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperLeft.m_Latitude-1);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,13);
    m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperLeft.m_Easting = (float)atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,13);
    m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperLeft.m_Northing = (float)atof(Value);
    pi_rHeaderFile.Read(&Blank,21);

    // Read Upper Right Corner information
    pi_rHeaderFile.Read(&TagName,4);
    if (BeStringUtilities::Strnicmp(TagName, "ur =", 4) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperRight.m_Longitude, sizeof m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperRight.m_Longitude-1);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperRight.m_Latitude, sizeof m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperRight.m_Latitude-1);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,13);
    m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperRight.m_Easting = (float)atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,13);
    m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperRight.m_Northing = (float)atof(Value);
    pi_rHeaderFile.Read(&Blank,21);

    // Read Lower Right Corner information
    pi_rHeaderFile.Read(&TagName,4);
    if (BeStringUtilities::Strnicmp(TagName, "lr =", 4) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerRight.m_Longitude, sizeof m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerRight.m_Longitude-1);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerRight.m_Latitude, sizeof m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerRight.m_Latitude-1);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,13);
    m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerRight.m_Easting = (float)atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,13);
    m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerRight.m_Northing = (float)atof(Value);
    pi_rHeaderFile.Read(&Blank,21);

    // Read Lower Left Corner information
    pi_rHeaderFile.Read(&TagName,4);
    if (BeStringUtilities::Strnicmp(TagName, "ll =", 4) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerLeft.m_Longitude, sizeof m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerLeft.m_Longitude-1);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerLeft.m_Latitude, sizeof m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerLeft.m_Latitude-1);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,13);
    m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerLeft.m_Easting = (float)atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,13);
    m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerLeft.m_Northing = (float)atof(Value);
    pi_rHeaderFile.Read(&Blank,21);

    // Read Center information
    pi_rHeaderFile.Read(&TagName,8);
    if (BeStringUtilities::Strnicmp(TagName, "center =", 8) != 0)
        goto WRAPUP;
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_RefPosInfo.m_CoordSystem.m_Longitude, sizeof m_HeaderInfo.m_GeoInfo.m_RefPosInfo.m_CoordSystem.m_Longitude-1);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&m_HeaderInfo.m_GeoInfo.m_RefPosInfo.m_CoordSystem.m_Latitude, sizeof m_HeaderInfo.m_GeoInfo.m_RefPosInfo.m_CoordSystem.m_Latitude-1);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,13);
    m_HeaderInfo.m_GeoInfo.m_RefPosInfo.m_CoordSystem.m_Easting = (float)atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,13);
    m_HeaderInfo.m_GeoInfo.m_RefPosInfo.m_CoordSystem.m_Northing = (float)atof(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,5);
    m_HeaderInfo.m_GeoInfo.m_RefPosInfo.m_Pixel = atoi(Value);
    pi_rHeaderFile.Read(&Blank,1);
    pi_rHeaderFile.Read(&Value,5);
    m_HeaderInfo.m_GeoInfo.m_RefPosInfo.m_Line = atoi(Value);
    pi_rHeaderFile.Read(&Blank,5);

    pi_rHeaderFile.Read(&TagName,8);
    pi_rHeaderFile.Read(&Value,6);
    m_HeaderInfo.m_GeoInfo.m_Offset = atoi(Value);
    pi_rHeaderFile.Read(&TagName,20);
    pi_rHeaderFile.Read(&Value,6);
    m_HeaderInfo.m_GeoInfo.m_OrientationAngle = atof(Value);
    pi_rHeaderFile.Read(&Blank,40);

    // Read Sun information
    pi_rHeaderFile.Read(&TagName,21);
    pi_rHeaderFile.Read(&Value,4);
    m_HeaderInfo.m_GeoInfo.m_SunInfo.m_Elevation = (float)atof(Value);
    pi_rHeaderFile.Read(&TagName,20);
    pi_rHeaderFile.Read(&Value,6);
    m_HeaderInfo.m_GeoInfo.m_SunInfo.m_Azimuth = (float)atof(Value);

    Success = true;

WRAPUP:
    return Success;
    }

//-----------------------------------------------------------------------------
// Private
// CreateTransfoModelFromFastL7A
// Create a HGF2DTransfoModel using the infomation found in the FastL7A file Header.
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFUSgsFastL7AFile::CreateTransfoModelFromFastL7A()
    {
    HFCPtr<HGF2DTransfoModel> pTransfoModel = new HGF2DIdentity();
    double pixelCenter = 0.5;

    // Create Matrix
    double pMatrix[4][4];
    double pTiePoints[24];
    HFCMatrix<3, 3> theMatrix;

    pTiePoints[0]  = pixelCenter;
    pTiePoints[1]  = pixelCenter;
    pTiePoints[3]  = m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperLeft.m_Easting;
    pTiePoints[4]  = m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperLeft.m_Northing;

    pTiePoints[6]  = m_HeaderInfo.m_AdminInfo.m_PixelPerLine - pixelCenter;
    pTiePoints[7]  = pixelCenter;
    pTiePoints[9]  = m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperRight.m_Easting;
    pTiePoints[10] = m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_UpperRight.m_Northing;

    pTiePoints[12] = pixelCenter;
    pTiePoints[13] = m_HeaderInfo.m_AdminInfo.m_LineInOutput - pixelCenter;
    pTiePoints[15] = m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerLeft.m_Easting;
    pTiePoints[16] = m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerLeft.m_Northing;

    pTiePoints[18] = m_HeaderInfo.m_AdminInfo.m_PixelPerLine  - pixelCenter;
    pTiePoints[19] = m_HeaderInfo.m_AdminInfo.m_LineInOutput - pixelCenter;
    pTiePoints[21] = m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerRight.m_Easting;
    pTiePoints[22] = m_HeaderInfo.m_GeoInfo.m_CornerInfo.m_LowerRight.m_Northing;

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
void HRFUSgsFastL7AFile::GetGeoKeyTag(HFCPtr<HCPGeoTiffKeys>& po_rpGeoTiffKeys)
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
bool HRFUSgsFastL7AFile::GetBandNumber(int32_t& pio_rBand,
                                        const WString& pi_rFileName)
    {
    bool Result = false;
    string FileName;
    WString FileNameStr;

    for(int32_t i = 0; i < m_HeaderInfo.m_NumberOfBands && !Result; i++)
        {
        FileName = m_HeaderInfo.m_Bands[i].m_FileName;
        RTrim(FileName);
        BeStringUtilities::CurrentLocaleCharToWChar( FileNameStr,FileName.c_str());

        if (BeStringUtilities::Wcsicmp(pi_rFileName.c_str(),FileNameStr.c_str()) == 0)
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
bool HRFUSgsFastL7AFile::GetCoordSystem(short& po_rCode)
    {
    bool Found = false;

    po_rCode = TIFFGeo_Undefined;

    HRFGeoTiffCoordSysTable*  pGeoTiffCoordSysTable = HRFGeoTiffCoordSysTable::GetInstance();
    pGeoTiffCoordSysTable->LockTable();

    HRFGeoTiffCoordSysTable::HRFGeoTiffCoordSysRecord CoordSysRecord;

    string Datum    = m_HeaderInfo.m_GeoInfo.m_Datum;
    string ProjName = m_HeaderInfo.m_GeoInfo.m_ProjectionName;
    string Zone     = m_HeaderInfo.m_GeoInfo.m_MapZone;

    // Eliminate space at the end of string
    RTrim(Datum);
    RTrim(ProjName);
    RTrim(Zone);

    // Search coordonate system code
    if (pGeoTiffCoordSysTable->GetRecord(Datum.substr(0,3) + " " + Datum.substr(3),
                                         ProjName,
                                         Zone,
                                         &CoordSysRecord)
        || pGeoTiffCoordSysTable->GetRecord(Datum,
                                            ProjName,
                                            Zone,
                                            &CoordSysRecord))
        {
        po_rCode = CoordSysRecord.CSCode;
        Found = true;
        }

    pGeoTiffCoordSysTable->ReleaseTable();

    return Found;
    }

//-----------------------------------------------------------------------------
// Private
// RTrim
// Eliminate space at the end of string
//-----------------------------------------------------------------------------
void HRFUSgsFastL7AFile::RTrim(string& pio_rString)
    {
    size_t Pos;

    // Remove all space from the end.
    Pos = pio_rString.find_last_not_of(' ');
    if (Pos != string::npos)
        pio_rString.erase(Pos+1);
    }
