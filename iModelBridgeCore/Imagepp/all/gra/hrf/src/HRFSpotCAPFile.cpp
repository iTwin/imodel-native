//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSpotCAPFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFSpotCAPFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFSpotCAPFile.h>
#include <Imagepp/all/h/HRFSpotCAPLineEditor.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRPChannelOrgGray.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFTWFPageFile.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DIdentity.h>

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#include <ImagePPInternal/ext/MatrixFromTiePts/MatrixFromTiePts.h>



//////////////////////////////////////////////////////////////////////////
#define IMAG_FILE   0
#define LEAD_FILE   1
#define NULL_FILE   2
#define TRAI_FILE   3
#define VOLD_FILE   4

#define FIRST_CORNER_PIX    0
#define FIRST_CORNER_LINE   1
#define FIRST_CORNER_LAT    3
#define FIRST_CORNER_LONG   4
#define SECOND_CORNER_PIX   6
#define SECOND_CORNER_LINE  7
#define SECOND_CORNER_LAT   9
#define SECOND_CORNER_LONG  10
#define THIRD_CORNER_PIX    12
#define THIRD_CORNER_LINE   13
#define THIRD_CORNER_LAT    15
#define THIRD_CORNER_LONG   16
#define FOURTH_CORNER_PIX   18
#define FOURTH_CORNER_LINE  19
#define FOURTH_CORNER_LAT   21
#define FOURTH_CORNER_LONG  22


//-----------------------------------------------------------------------------
// HRFSpotCAPBlockCapabilities
//-----------------------------------------------------------------------------
class HRFSpotCAPBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFSpotCAPBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add(new HRFLineCapability(HFC_READ_ONLY,
                                  LONG_MAX,
                                  HRFBlockAccess::RANDOM));
        }
    };

//-----------------------------------------------------------------------------
// HRFSpotCAPCodecIdentityCapabilities
//-----------------------------------------------------------------------------
class HRFSpotCAPCodecIdentityCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFSpotCAPCodecIdentityCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFSpotCAPBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFSpotCAPCapabilities
//-----------------------------------------------------------------------------
HRFSpotCAPCapabilities::HRFSpotCAPCapabilities()
    : HRFRasterFileCapabilities()
    {
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFSpotCAPCodecIdentityCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFSpotCAPCodecIdentityCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV32R8G8B8X8::CLASS_ID,
                                   new HRFSpotCAPCodecIdentityCapabilities()));

    // Histogram capability
    Add(new HRFHistogramCapability(HFC_READ_ONLY));

    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL));

    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DAffine::CLASS_ID     ));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DSimilitude::CLASS_ID ));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID    ));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID   ));

    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDocumentName));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeImageDescription));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributePageName));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDateTime));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeArtist));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeHostComputer));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeYResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeCopyright));
    }

HFC_IMPLEMENT_SINGLETON(HRFSpotCAPCreator)

//-----------------------------------------------------------------------------
// Creator
// This is the creator to instantiate Spot format
//-----------------------------------------------------------------------------
HRFSpotCAPCreator::HRFSpotCAPCreator()
    : HRFRasterFileCreator(HRFSpotCAPFile::CLASS_ID)
    {
    // Spot capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFSpotCAPCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_SpotCap()); // Spot CAP File Format
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFSpotCAPCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFSpotCAPCreator::GetExtensions() const
    {
    return WString(L"*.fil");
    }

//-----------------------------------------------------------------------------
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFSpotCAPCreator::Create(
    const HFCPtr<HFCURL>& pi_rpURL,
    HFCAccessMode         pi_AccessMode,
    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HFCPtr<HRFRasterFile> pFile = new HRFSpotCAPFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);
    return (pFile);
    }

//-----------------------------------------------------------------------------
// Private
// Create
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::Create()
    {
    // Open the file
    m_pFilFile = HFCBinStream::Instanciate(this->GetURL(), GetAccessMode(), 0, true);

    SharingControlCreate();

    m_IsOpen = true;

    return true;
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFSpotCAPCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                      uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    
    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    bool                       Result = false;
    HAutoPtr<HFCBinStream>      pFile;
    HAutoPtr<HFCBinStream>      pImagDirectoryFile;
    HAutoPtr<char>              Header;
    char*                       ValueStartPos;
    HFCPtr<HFCURL>              ImagFileUrl;


    (const_cast<HRFSpotCAPCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Open the Spot Scenes Records File : CD_DIR.FIL
    pFile = HFCBinStream::Instanciate(pi_rpURL, HFC_READ_ONLY | HFC_SHARE_READ_ONLY);
    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    //we allocate just enough to read the SCENEXX parameter
    Header = new char[15 + 1];
    Header[15] = '\0';
    pFile->SeekToBegin();
    if (pFile->Read(Header, 15) != 15)
        goto WRAPUP;

    BeStringUtilities::Strupr(Header);
    ValueStartPos = strstr((char*)Header, "SCENE");
    if (ValueStartPos != NULL)
        {
        // Unlock the sister file
        SisterFileLock.ReleaseKey();
        HASSERT(!(const_cast<HRFSpotCAPCreator*>(this))->m_pSharingControl->IsLocked());
        (const_cast<HRFSpotCAPCreator*>(this))->m_pSharingControl = 0;

        ValueStartPos += 5;
        HAutoPtr<char> SceneNumber;
        SceneNumber = new char[3];
        SceneNumber[2] = '\0';
        memcpy(SceneNumber, ValueStartPos, 2);

        if(atoi(SceneNumber) >= 0 && atoi(SceneNumber) <=99 )
            {
            // Get the Url for the "IMAG_XX.dat" file.

            // Find the file extension
            WString Path = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

            WString::size_type SlashPos = Path.find_last_of(L"\\/");

            if (SlashPos != WString::npos)
                Path = Path.substr(0, SlashPos);


            WString SceneNumberStr;
            BeStringUtilities::CurrentLocaleCharToWChar( SceneNumberStr,SceneNumber);

            // Compose url for .hdr, and .bnd files
            WString FileName = WString(HFCURLFile::s_SchemeName() + L"://")
                               + ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost()
                               + WString(L'\\' + Path + L'\\')
                               + WString(L"SCENE")
                               + SceneNumberStr
                               + WString(L'\\' + WString(L"IMAG_"))
                               + SceneNumberStr;


            // Create related files
            ImagFileUrl = new HFCURLFile(FileName + WString(L".dat"));


            // This creates the sister file for file sharing control if necessary.
            (const_cast<HRFSpotCAPCreator*>(this))->ImagSharingControlCreate(ImagFileUrl);
            HFCLockMonitor SisterFileLock(GetImagLockManager());

            pImagDirectoryFile = HFCBinStream::Instanciate(ImagFileUrl, HFC_READ_ONLY | HFC_SHARE_READ_ONLY);

            if (pImagDirectoryFile != 0 && pImagDirectoryFile->GetLastException() == 0)
                {
                char   CurrentField[5];
                CurrentField[4] = 0X0;
                pImagDirectoryFile->SeekToBegin();
                pImagDirectoryFile->Seek(268);
                if (pImagDirectoryFile->Read(CurrentField, sizeof(CurrentField)) != sizeof(CurrentField))
                    goto WRAPUP;

                if(strncmp(CurrentField, "BIL", 3) == 0)
                    {
                    //at least for this record
                    Result = true;
                    }
                }
            SisterFileLock.ReleaseKey();
            HASSERT(!(const_cast<HRFSpotCAPCreator*>(this))->m_pImagSharingControl->IsLocked());
            (const_cast<HRFSpotCAPCreator*>(this))->m_pImagSharingControl = 0;
            }
        }
    //we could be directly opening the IMAG_XX.DAT file
    else
        {
        char   CurrentField[5];
        CurrentField[4] = 0X0;
        pFile->SeekToBegin();
        pFile->Seek(268);
        if (pFile->Read(CurrentField, sizeof(CurrentField)) != sizeof(CurrentField))
            goto WRAPUP;

        if(strncmp(CurrentField, "BIL", 3) == 0)
            {
            //at least for this record
            Result = true;
            }

        // Unlock the sister file
        SisterFileLock.ReleaseKey();
        HASSERT(!(const_cast<HRFSpotCAPCreator*>(this))->m_pSharingControl->IsLocked());
        (const_cast<HRFSpotCAPCreator*>(this))->m_pSharingControl = 0;
        }

WRAPUP:

    return Result;
    }



//-----------------------------------------------------------------------------
// Get list of related files from a given URL.
//-----------------------------------------------------------------------------
bool HRFSpotCAPCreator::GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                        ListOfRelatedURLs& pio_rRelatedURLs) const
    {
    HASSERT (pio_rRelatedURLs.size() == 0);

    HFCPtr<HFCURL> pImagFilesURLS;
    HFCPtr<HFCURL> pLeadFilesURLS;
    HFCPtr<HFCURL> pNullFilesURLS;
    HFCPtr<HFCURL> pTraiFilesURLS;
    HFCPtr<HFCURL> pVoldFilesURLS;

    // Find the file extension
    WString Path = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

    WString::size_type SlashPos = Path.find_last_of(L"\\/");

    if (SlashPos != WString::npos)
        Path = Path.substr(0, SlashPos);

    for(int SceneNumber = 0; SceneNumber < 1; SceneNumber++)
        {
        stringstream Result;


        // Compose url for .hdr, and .bnd files
        WString FileName = WString(HFCURLFile::s_SchemeName() + L"://")
                           + ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost() + WString(L"\\")
                           + Path + WString(L"SCENE") + L"01" + L'\\';

        // Create related files
        pImagFilesURLS = new HFCURLFile(FileName + WString(L"IMAG_") + WString(L"01") + WString(L".DAT"));
        pLeadFilesURLS = new HFCURLFile(FileName + WString(L"LEAD_") + WString(L"01") + WString(L".DAT"));
        pNullFilesURLS = new HFCURLFile(FileName + WString(L"NULL_") + WString(L"01") + WString(L".DAT"));
        pTraiFilesURLS = new HFCURLFile(FileName + WString(L"TRAI_") + WString(L"01") + WString(L".DAT"));
        pVoldFilesURLS = new HFCURLFile(FileName + WString(L"VOLD_") + WString(L"01") + WString(L".DAT"));

        pio_rRelatedURLs.push_back(pImagFilesURLS);
        pio_rRelatedURLs.push_back(pLeadFilesURLS);
        pio_rRelatedURLs.push_back(pNullFilesURLS);
        pio_rRelatedURLs.push_back(pTraiFilesURLS);
        pio_rRelatedURLs.push_back(pVoldFilesURLS);

        }

    return true;

    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of BIL file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFSpotCAPCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFSpotCAPCapabilities();
    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFSpotCAPFile::HRFSpotCAPFile(const HFCPtr<HFCURL>& pi_rURL,
                               HFCAccessMode         pi_AccessMode,
                               uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen        = false;

    Initialize();

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
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFSpotCAPFile::HRFSpotCAPFile(const HFCPtr<HFCURL>& pi_rURL,
                               HFCAccessMode         pi_AccessMode,
                               uint64_t             pi_Offset,
                               bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen     = false;

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }

    Initialize();
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFSpotCAPFile::~HRFSpotCAPFile()
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        m_IsOpen = false;
    }

//-----------------------------------------------------------------------------
// Open
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::Open()
    {
    // Open the file


    if (!m_IsOpen)
        {
        m_pFilFile = HFCBinStream::Instanciate(GetURL(), m_Offset, HFC_READ_ONLY | HFC_SHARE_READ_ONLY, 0, true);

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Initialization of file struct.
        HFCLockMonitor SisterFileLock(GetLockManager());
        bool IsHeaderFileRead = false;

        m_IsFilHeader = IsFilHeader();

        if(m_IsFilHeader)
            {
            IsHeaderFileRead = ReadFilHeader();
            HASSERT(IsHeaderFileRead == true);
            }

        IsHeaderFileRead = ReadImagHeader();
        HASSERT(IsHeaderFileRead == true);

        //now that we can
        SetDataTiePoints();

        if(!ReadLeadHeader())
            {
            //do nothing. that file is not essential
            }

        if(!ReadVoldHeader())
            {
            //do nothing. that file is not essential
            }

        SisterFileLock.ReleaseKey();

        m_IsOpen = true;
        }

    return true;
    }

//-----------------------------------------------------------------------------
// ReadFilHeader
// Private
// Gets the header information from the ".fil" file. Returns false if there
// is no such file.
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::ReadFilHeader()
    {
    HPRECONDITION(m_IsFilHeader);
    HPRECONDITION(m_pFilFile != 0);

    HAutoPtr<char>              Header;
    uint32_t                    HeaderLength;
    char*                       ValueStartPos;

    m_pFilFile->SeekToEnd();

    HeaderLength = (uint32_t)m_pFilFile->GetCurrentPos();
    Header = new char[HeaderLength + 1];
    Header[HeaderLength] = '\0';
    m_pFilFile->SeekToBegin();
    m_pFilFile->Read(Header, HeaderLength);

    BeStringUtilities::Strupr(Header);
    ValueStartPos = strstr((char*)Header, "SCENE");
    while(ValueStartPos != NULL)
        {
        //seek the end of SCENE
        ValueStartPos += 5;
        HAutoPtr<char> SceneNumber;
        SceneNumber = new char[3];
        SceneNumber[2] = '\0';

        memcpy(SceneNumber, ValueStartPos, 2);
        if(atoi(SceneNumber) >= 0 && atoi(SceneNumber) <=99 )
            {
            WString SceneNumberStr;
            BeStringUtilities::CurrentLocaleCharToWChar( SceneNumberStr,SceneNumber);

            m_SceneNumbers.push_back(SceneNumberStr);
            }
        ValueStartPos = strstr(ValueStartPos, "SCENE");
        }

    return true;
    }

//-----------------------------------------------------------------------------
// ReadImagHeader
// Private
// Gets the header information from the "IMAG_XX.DAT"
//
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::ReadImagHeader()
    {
    bool                       Result = false;
    HFCPtr<HFCURL>              pImagFileURL;

    static const size_t         FIELD_BUFFER_SIZE = 8;
    char                       FieldBuffer[FIELD_BUFFER_SIZE+1];
    FieldBuffer[FIELD_BUFFER_SIZE] = '\0';

    static const size_t         WORD_BUFFER_SIZE = 4;
    Byte                       WordBuffer[WORD_BUFFER_SIZE];



    if(m_IsFilHeader)
        {
        // Find the file extension
        WString Path = ((HFCPtr<HFCURLFile>&)this->GetURL())->GetPath();
        WString Host = ((HFCPtr<HFCURLFile>&)this->GetURL())->GetHost();
        WString::size_type SlashPos = Path.find_last_of(L"\\/");

        if (SlashPos != WString::npos)
            Path = Path.substr(0, SlashPos);


        // Compose url for IMAG_XX.DAT file
        WString FileName = WString(HFCURLFile::s_SchemeName() + L"://")
                           + Host + WString(L"\\")
                           + Path + WString(L"\\") + WString(L"SCENE") + m_SceneNumbers.at(0) + L'\\';

        pImagFileURL = new HFCURLFile(FileName + WString(L"IMAG_") + m_SceneNumbers.at(0) + WString(L".DAT"));

        //open the IMAG_XX.DAT file
        m_pImagFile = HFCBinStream::Instanciate(pImagFileURL, HFC_READ_ONLY | HFC_SHARE_READ_ONLY, 0, true);

        // This creates the sister file for file sharing control if necessary.
        ImagSharingControlCreate(pImagFileURL);

        // Initialisation of file struct.
        HFCLockMonitor SisterFileLock(GetImagLockManager());

        Result = true;

        m_pImagFile->SeekToPos(8);//9
        m_pImagFile->Read(WordBuffer,4);
        m_ImgHeader.HeaderSize = WordBuffer[0]<<24 | (WordBuffer[1]&0xff)<<16 | (WordBuffer[2]&0xff)<<8 | (WordBuffer[3]&0xff);
        m_pImagFile->SeekToPos(180);//181
        m_pImagFile->Read(&FieldBuffer[2],6);
        m_ImgHeader.NbImageRecords = strtoul(&FieldBuffer[2], NULL, 10);
        m_pImagFile->SeekToPos(216);//217
        m_pImagFile->Read(&FieldBuffer[4],4);
        m_ImgHeader.NbBitsPerPixel = strtoul(&FieldBuffer[4], NULL, 10);
        m_pImagFile->SeekToPos(232);//233
        m_pImagFile->Read(&FieldBuffer[4],4);
        m_ImgHeader.NbBands = strtoul(&FieldBuffer[4], NULL, 10);
        m_pImagFile->SeekToPos(236);//237
        m_pImagFile->Read(FieldBuffer,8);
        m_ImgHeader.ImageHeight = strtoul(FieldBuffer, NULL, 10);
        m_pImagFile->SeekToPos(244);//245
        m_pImagFile->Read(&FieldBuffer[4],4);
        m_ImgHeader.NbLeftBorderPixels = strtoul(&FieldBuffer[4], NULL, 10);
        m_pImagFile->SeekToPos(248);//249
        m_pImagFile->Read(FieldBuffer,8);
        m_ImgHeader.ImageWidth = strtoul(FieldBuffer, NULL, 10);
        m_pImagFile->SeekToPos(256);//257
        m_pImagFile->Read(FieldBuffer,8);
        m_ImgHeader.NbRightBorderPixels = strtoul(FieldBuffer, NULL, 10);
        m_pImagFile->SeekToPos(276);//277
        //TODO : m_pImagFile->Read(&FieldBuffer[4],4);
        m_ImgHeader.NbBytesPrefixDataPerRecord = 32;
        m_pImagFile->SeekToPos(288);//289
        //TODO : m_pImagFile->Read(&FieldBuffer[4],4);
        m_ImgHeader.NbBytesSuffixDataPerRecord = 68;

        if(m_ImgHeader.NbBands >= 3)
            {
            m_ImgHeader.RedChannel = 1;
            m_ImgHeader.GreenChannel = 2;
            m_ImgHeader.BlueChannel = 3;
            }
        else
            {
            m_ImgHeader.RedChannel = 1;
            m_ImgHeader.GreenChannel = 0;
            m_ImgHeader.BlueChannel = 0;
            }

        // Unlock the sister file
        SisterFileLock.ReleaseKey();

        }
    else
        {
        pImagFileURL = GetURL();

        m_pImagFile = m_pFilFile;

        if(m_pImagFile != NULL)
            {

            Result = true;

            m_pImagFile->SeekToPos(8);//9
            m_pImagFile->Read(WordBuffer,4);
            m_ImgHeader.HeaderSize = WordBuffer[0]<<24 | (WordBuffer[1]&0xff)<<16 | (WordBuffer[2]&0xff)<<8 | (WordBuffer[3]&0xff);
            m_pImagFile->SeekToPos(180);//181
            m_pImagFile->Read(&FieldBuffer[2],6);
            m_ImgHeader.NbImageRecords = strtoul(&FieldBuffer[2], NULL, 10);
            m_pImagFile->SeekToPos(216);//217
            m_pImagFile->Read(&FieldBuffer[4],4);
            m_ImgHeader.NbBitsPerPixel = strtoul(&FieldBuffer[4], NULL, 10);
            m_pImagFile->SeekToPos(232);//233
            m_pImagFile->Read(&FieldBuffer[4],4);
            m_ImgHeader.NbBands = strtoul(&FieldBuffer[4], NULL, 10);
            m_pImagFile->SeekToPos(236);//237
            m_pImagFile->Read(FieldBuffer,8);
            m_ImgHeader.ImageHeight = strtoul(FieldBuffer, NULL, 10);
            m_pImagFile->SeekToPos(244);//245
            m_pImagFile->Read(&FieldBuffer[4],4);
            m_ImgHeader.NbLeftBorderPixels = strtoul(&FieldBuffer[4], NULL, 10);
            m_pImagFile->SeekToPos(248);//249
            m_pImagFile->Read(FieldBuffer,8);
            m_ImgHeader.ImageWidth = strtoul(FieldBuffer, NULL, 10);
            m_pImagFile->SeekToPos(256);//257
            m_pImagFile->Read(FieldBuffer,8);
            m_ImgHeader.NbRightBorderPixels = strtoul(FieldBuffer, NULL, 10);
            m_pImagFile->SeekToPos(276);//277
            //TODO : m_pImagFile->Read(&FieldBuffer[4],4);
            m_ImgHeader.NbBytesPrefixDataPerRecord = 32;
            m_pImagFile->SeekToPos(288);//289
            //TODO : m_pImagFile->Read(&FieldBuffer[4],4);
            m_ImgHeader.NbBytesSuffixDataPerRecord = 68;

            if(m_ImgHeader.NbBands >= 3)
                {
                m_ImgHeader.RedChannel = 1;
                m_ImgHeader.GreenChannel = 2;
                m_ImgHeader.BlueChannel = 3;
                }
            else
                {
                m_ImgHeader.RedChannel = 1;
                m_ImgHeader.GreenChannel = 0;
                m_ImgHeader.BlueChannel = 0;
                }
            }
        }




    return Result;
    }


//-----------------------------------------------------------------------------
// ReadLeadHeader
// Protected
// Gets the header information from the "LEAD_XX.DAT" file. Returns false if there
// is no such file.
//
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::ReadLeadHeader()
    {
    bool                       Result = false;
    HFCPtr<HFCURL>              pLeadFileURL;
    WString                     FileName;

    char                       FieldBuffer[12];

    // Find the file extension
    WString Path = ((HFCPtr<HFCURLFile>&)this->GetURL())->GetPath();
    WString Host = ((HFCPtr<HFCURLFile>&)this->GetURL())->GetHost();
    WString::size_type SlashPos = Path.find_last_of(L"\\/");

    if (SlashPos != WString::npos)
        Path = Path.substr(0, SlashPos);

    if(m_IsFilHeader)
        {

        // Compose url for LEAD_XX.DAT file
        FileName = WString(HFCURLFile::s_SchemeName() + L"://")
                   + Host + WString(L"\\")
                   + Path + WString(L"\\") + WString(L"SCENE") + m_SceneNumbers.at(0) + L'\\';
        }
    else
        {
        // Compose url for LEAD_XX.DAT file
        FileName = WString(HFCURLFile::s_SchemeName() + L"://")
                   + Host + WString(L"\\")
                   + Path + WString(L"\\");
        }

    pLeadFileURL = new HFCURLFile(FileName + WString(L"LEAD_") + WString(L"01") + WString(L".DAT"));

    //open the LEAD_XX.DAT file
    m_pLeadFile = HFCBinStream::Instanciate(pLeadFileURL, HFC_READ_ONLY | HFC_SHARE_READ_ONLY);

    // This creates the sister file for file sharing control if necessary.
    LeadSharingControlCreate(pLeadFileURL);

    // Initialisation of file struct.
    HFCLockMonitor SisterFileLock(GetLeadLockManager());

    if(m_pLeadFile != 0 && m_pLeadFile->GetLastException() == 0)
        {
        Result = true;

        string BufferBeforeConvert;
        char    CharBuffer[20];

        m_pLeadFile->SeekToPos(16);//17
        m_pLeadFile->Read(CharBuffer, 12);
        CharBuffer[12] = 0;
        m_LeadHeader.ImageFormatDescription = CharBuffer;
        CleanUpString(&m_LeadHeader.ImageFormatDescription);

        m_pLeadFile->SeekToPos(32);//33
        m_pLeadFile->Read(CharBuffer, 12);
        CharBuffer[12] = 0;
        m_LeadHeader.SoftwareUsed = CharBuffer;
        CleanUpString(&m_LeadHeader.SoftwareUsed);

        m_pLeadFile->SeekToPos(180);
        m_pLeadFile->Read(&FieldBuffer[2],6);
        m_LeadHeader.NbHeaderRecords = atol(&FieldBuffer[2]);
        m_pLeadFile->SeekToPos(186);
        m_pLeadFile->Read(&FieldBuffer[2],6);
        m_LeadHeader.HeaderRecLength = atol(&FieldBuffer[2]);
        m_pLeadFile->SeekToPos(192);
        m_pLeadFile->Read(&FieldBuffer[2],6);
        m_LeadHeader.NbAncillaryRecords = atol(&FieldBuffer[2]);
        m_pLeadFile->SeekToPos(198);
        m_pLeadFile->Read(&FieldBuffer[2],6);
        m_LeadHeader.AncillaryRecordLength = atol(&FieldBuffer[2]);
        m_pLeadFile->SeekToPos(204);
        m_pLeadFile->Read(&FieldBuffer[2],6);
        m_LeadHeader.NbAnnotationRecords = atol(&FieldBuffer[2]);
        m_pLeadFile->SeekToPos(210);
        m_pLeadFile->Read(&FieldBuffer[2],6);
        m_LeadHeader.AnnotationRecordLength = atol(&FieldBuffer[2]);
        //the tiepoints
        //1st corner (C1) : 1st pixel, 1st line in the raw scene.
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 148);//144
        m_pLeadFile->Read(CharBuffer, 8);
        CharBuffer[8]=0;
        BufferBeforeConvert = CharBuffer;
        m_pTiePoints[FIRST_CORNER_LAT]  = ConvertStringToRadian(&BufferBeforeConvert);
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 164);//160
        m_pLeadFile->Read(CharBuffer, 8);
        CharBuffer[8]=0;
        BufferBeforeConvert = CharBuffer;
        m_pTiePoints[FIRST_CORNER_LONG]  = ConvertStringToRadian(&BufferBeforeConvert);
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 190);
        m_pLeadFile->Read(CharBuffer, 6);
        m_pTiePoints[FIRST_CORNER_LINE]  = atof(CharBuffer);
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 206);
        m_pLeadFile->Read(CharBuffer, 6);
        m_pTiePoints[FIRST_CORNER_PIX] = atof(CharBuffer);

        //2nd corner (C2) : last pixel, 1st line in the raw scene.
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 212);
        m_pLeadFile->Read(CharBuffer, 8);
        CharBuffer[8]=0;
        BufferBeforeConvert = CharBuffer;
        m_pTiePoints[SECOND_CORNER_LAT] = ConvertStringToRadian(&BufferBeforeConvert);
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 228);
        m_pLeadFile->Read(CharBuffer, 8);
        CharBuffer[8]=0;
        BufferBeforeConvert = CharBuffer;
        m_pTiePoints[SECOND_CORNER_LONG] = ConvertStringToRadian(&BufferBeforeConvert);
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 254);
        m_pLeadFile->Read(CharBuffer, 6);
        m_pTiePoints[SECOND_CORNER_LINE]= atof(CharBuffer);
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 270);
        m_pLeadFile->Read(CharBuffer, 6);
        m_pTiePoints[SECOND_CORNER_PIX]= atof(CharBuffer);

        //3rd corner (C3) : 1st pixel, last line in the raw scene.
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 276);
        m_pLeadFile->Read(CharBuffer, 8);
        CharBuffer[8]=0;
        BufferBeforeConvert = CharBuffer;
        m_pTiePoints[THIRD_CORNER_LAT] = ConvertStringToRadian(&BufferBeforeConvert);
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 292);
        m_pLeadFile->Read(CharBuffer, 8);
        CharBuffer[8]=0;
        BufferBeforeConvert = CharBuffer;
        m_pTiePoints[THIRD_CORNER_LONG] = ConvertStringToRadian(&BufferBeforeConvert);
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 318);
        m_pLeadFile->Read(CharBuffer, 6);
        m_pTiePoints[THIRD_CORNER_LINE]= atof(CharBuffer);
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 334);
        m_pLeadFile->Read(CharBuffer, 6);
        m_pTiePoints[THIRD_CORNER_PIX]= atof(CharBuffer);

        //4th corner (C4) : last pixel, last line in the raw scene.
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 340);
        m_pLeadFile->Read(CharBuffer, 8);
        CharBuffer[8]=0;
        BufferBeforeConvert = CharBuffer;
        m_pTiePoints[FOURTH_CORNER_LAT] = ConvertStringToRadian(&BufferBeforeConvert);
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 356);
        m_pLeadFile->Read(CharBuffer, 8);
        CharBuffer[8]=0;
        BufferBeforeConvert = CharBuffer;
        m_pTiePoints[FOURTH_CORNER_LONG] = ConvertStringToRadian(&BufferBeforeConvert);
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 382);
        m_pLeadFile->Read(CharBuffer, 6);
        m_pTiePoints[FOURTH_CORNER_LINE]= atof(CharBuffer);
        m_pLeadFile->SeekToPos(m_LeadHeader.HeaderRecLength + 398);
        m_pLeadFile->Read(CharBuffer, 6);
        m_pTiePoints[FOURTH_CORNER_PIX]= atof(CharBuffer);


        //(the descriptor and the header) + (19th first ancilliary records)
        m_pLeadFile->SeekToPos((m_LeadHeader.HeaderRecLength *2 + m_LeadHeader.AncillaryRecordLength *19) - 1);
        m_LeadHeader.OffsetToHistoValuesPerRecord = 32;

        }

    SisterFileLock.ReleaseKey();

    return Result;
    }


//-----------------------------------------------------------------------------
// ReadVoldHeader
// Protected
// Gets the header information from the "VOLD_XX.DAT" file. Returns false if there
// is no such file.
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::ReadVoldHeader()
    {
    bool                       Result = false;
    HFCPtr<HFCURL>              pVoldFileURL;
    WString                     FileName;

    Byte                       WordBuffer[4];


    // Find the file extension
    WString Path = ((HFCPtr<HFCURLFile>&)this->GetURL())->GetPath();
    WString Host = ((HFCPtr<HFCURLFile>&)this->GetURL())->GetHost();
    WString::size_type SlashPos = Path.find_last_of(L"\\/");

    if (SlashPos != WString::npos)
        Path = Path.substr(0, SlashPos);

    if(m_IsFilHeader)
        {

        // Compose url for LEAD_XX.DAT file
        FileName = WString(HFCURLFile::s_SchemeName() + L"://")
                   + Host + WString(L"\\")
                   + Path + WString(L"\\") + WString(L"SCENE") + m_SceneNumbers.at(0) + L'\\';
        }
    else
        {
        // Compose url for LEAD_XX.DAT file
        FileName = WString(HFCURLFile::s_SchemeName() + L"://")
                   + Host + WString(L"\\")
                   + Path + WString(L"\\");
        }

    pVoldFileURL = new HFCURLFile(FileName + WString(L"VOLD_") + WString(L"01") + WString(L".DAT"));


    //open the VOLD_XX.DAT file
    m_pVoldFile = HFCBinStream::Instanciate(pVoldFileURL, HFC_READ_ONLY | HFC_SHARE_READ_ONLY);

    // This creates the sister file for file sharing control if necessary.
    VoldSharingControlCreate(pVoldFileURL);

    // Initialisation of file struct.
    HFCLockMonitor SisterFileLock(GetVoldLockManager());

    if(m_pVoldFile != 0 && m_pVoldFile->GetLastException() == 0)
        {
        Result = true;

        m_pVoldFile->SeekToPos(8);//9
        m_pVoldFile->Read(WordBuffer,4);
        m_VoldHeader.RecordSize = WordBuffer[0]<<24 | (WordBuffer[1]&0xff)<<16 | (WordBuffer[2]&0xff)<<8 | (WordBuffer[3]&0xff);

        }

    SisterFileLock.ReleaseKey();

    return Result;
    }
//-------------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-------------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFSpotCAPFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

//-------------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-------------------------------------------------------------------------------
HRFResolutionEditor* HRFSpotCAPFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                            unsigned short pi_Resolution,
                                                            HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return new HRFSpotCAPLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    }

//-------------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-------------------------------------------------------------------------------
void HRFSpotCAPFile::Save()
    {
    HASSERT(!"HRFSpotCAPFile::Save():SpotCAP format is read only");
    }

//-------------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-------------------------------------------------------------------------------
bool HRFSpotCAPFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);
    HPRECONDITION(pi_pPage != 0);

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);


    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(0);



    return true;
    }

//-------------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-------------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFSpotCAPFile::GetCapabilities () const
    {
    return (HRFSpotCAPCreator::GetInstance()->GetCapabilities());
    }



//-------------------------------------------------------------------------------
// GetScanLineOrientationFromFile
// Private
// Find the scanline orientation from file
//-------------------------------------------------------------------------------
HRFScanlineOrientation HRFSpotCAPFile::GetScanLineOrientation() const
    {
    return HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);



    // Create Page and resolution Description/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor>         pResolution;
    HFCPtr<HRFPageDescriptor>               pPage;
    HPMAttributeSet                         TagList;
    HFCPtr<HPMGenericAttribute>             pTag;

    pResolution =  new HRFResolutionDescriptor(
        GetAccessMode(),                                // AccessMode,
        GetCapabilities(),                              // Capabilities,
        1.0,                                            // XResolutionRatio,
        1.0,                                            // YResolutionRatio,
        CreatePixelTypeFromFile(),                      // PixelType,
        new HCDCodecIdentity(),                         // Codecs,
        HRFBlockAccess::RANDOM,                         // RBlockAccess,
        HRFBlockAccess::RANDOM,                         // WBlockAccess,
        GetScanLineOrientation(),                       // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                       // InterleaveType
        false,                                          // IsInterlace,
        m_ImgHeader.ImageWidth,                         // Width image ,
        m_ImgHeader.ImageHeight,                        // Height image,
        m_ImgHeader.ImageWidth,                         // BlockWidth,
        1,                                              // BlockHeight,
        0,                                              // BlocksDataFlag
        HRFBlockType::LINE);

    //TODO
    //HFCPtr<HRPHistogram> pHistogram = GetHistogramFromFile();


    pTag = new HRFAttributeImageDescription(WString(m_LeadHeader.ImageFormatDescription.c_str(),false));
    TagList.Set(pTag);

    pTag = new HRFAttributeSoftware(WString(m_LeadHeader.SoftwareUsed.c_str(),false));
    TagList.Set(pTag);

    pTag = new HRFAttributeDateTime(WString(m_VoldHeader.DateOfCreation.c_str(),false));
    TagList.Set(pTag);

    pTag = new HRFAttributeCopyright(WString(m_VoldHeader.Copyright.c_str(),false));
    TagList.Set(pTag);



    pPage = new HRFPageDescriptor (GetAccessMode(),         // AccessMode
                                   GetCapabilities(),                                  // Capabilities,
                                   pResolution,                                        // ResolutionDescriptor,
                                   0,                                                  // RepresentativePalette,
                                   0,                                                  // Histogram,
                                   0,                                                  // Thumbnail,
                                   0,                                                  // ClipShape,
                                   0,/*CreateTransfoModel()*/                          // TransfoModel,
                                   0,                                                  // Filters
                                   &TagList);                                          // Attribute set


    m_ListOfPageDescriptor.push_back(pPage);
    }

//-----------------------------------------------------------------------------
// CreatePixelTypeFromFile
// Private
// Find and create pixel type from file
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRFSpotCAPFile::CreatePixelTypeFromFile() const
    {
    HFCPtr<HRPPixelType> pPixelType = 0;


    if ((1 <= m_ImgHeader.NbBands) )
        {
        if (8 == m_ImgHeader.NbBitsPerPixel)
            pPixelType = new HRPPixelTypeV8Gray8();
        }
    else
        {
        if (3 == m_ImgHeader.NbBands)
            {
            if (8 == m_ImgHeader.NbBitsPerPixel)
                pPixelType = new HRPPixelTypeV24R8G8B8();
            }
        else if(4 == m_ImgHeader.NbBands)
            {
            if (8 == m_ImgHeader.NbBitsPerPixel)
                pPixelType = new HRPPixelTypeV32R8G8B8X8();
            }
        }


    if (pPixelType == 0)
        throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());

    return (pPixelType);
    }


//-----------------------------------------------------------------------------
// IsFliHeader
// Determines if we're trying to open the .fil header
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::IsFilHeader()
    {

    bool Result = false;


    if(m_pFilFile != 0)
        {
        char   CurrentField[6];
        CurrentField[5] = 0X0;
        m_pFilFile->SeekToBegin();
        m_pFilFile->Read((char*)CurrentField, sizeof CurrentField);
        if(strncmp(CurrentField, "SCENE", 5) == 0)
            {
            Result = true;
            }
        }

    return Result;

    }




//-----------------------------------------------------------------------------
// Protected
// CalculateFactorModelToMeter
//
//-----------------------------------------------------------------------------
const HFCPtr<HRPHistogram>   HRFSpotCAPFile::GetHistogramFromFile()const
    {

    HFCPtr<HRPHistogram> pHistogram = 0;


    if(m_pLeadFile != 0)
        {
        uint32_t** pEntryFrequencies;
        pEntryFrequencies = new uint32_t*[3];
        //reset the file
        m_pLeadFile->SeekToBegin();
        for (int ChannelIndex = 0; ChannelIndex < 3; ChannelIndex++)
            {
            pEntryFrequencies   [ChannelIndex]  = new uint32_t[256];
            memcpy(pEntryFrequencies[ChannelIndex],
                   m_pLeadFile
                   + ((m_LeadHeader.HeaderRecLength * 2
                       + m_LeadHeader.AncillaryRecordLength * (19 + ChannelIndex)))
                   + m_LeadHeader.OffsetToHistoValuesPerRecord
                   - 1,
                   256 * sizeof(uint32_t));
            }
        pHistogram = new HRPHistogram(pEntryFrequencies, 256, 3);

        for (int ChannelIndex = 0; ChannelIndex < 3; ChannelIndex++)
            delete pEntryFrequencies[ChannelIndex];
        delete pEntryFrequencies;

        }

    return pHistogram;

    }

//-----------------------------------------------------------------------------
// Private
// Initialize
// Initialize members
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::Initialize()
    {
    m_pTiePoints = new double[24];

    m_pFilFile      = NULL;
    m_pLeadFile     = NULL;
    m_pImagFile     = NULL;
    m_pVoldFile     = NULL;

    m_GeoRefInfo.m_A00 = 0.0;
    m_GeoRefInfo.m_A01 = 0.0;
    m_GeoRefInfo.m_A10 = 0.0;
    m_GeoRefInfo.m_A11 = 0.0;
    m_GeoRefInfo.m_Tx = 0.0;
    m_GeoRefInfo.m_Ty = 0.0;

    m_FilHeader.SceneDirectoryName = 0;
    m_FilHeader.SatelliteNumber = 0;
    m_FilHeader.Year = 0;
    m_FilHeader.Month = 0;
    m_FilHeader.Day = 0;
    m_FilHeader.ShiftAlongTrack = 0;
    m_ImgHeader.BlueChannel = 0;
    m_ImgHeader.GreenChannel = 0;;
    m_ImgHeader.HeaderSize = 0;;
    m_ImgHeader.ImageHeight = 0;;
    m_ImgHeader.ImageWidth = 0;;
    m_ImgHeader.NbBands = 0;;
    m_ImgHeader.NbBitsPerPixel = 0;;
    m_ImgHeader.NbBytesPrefixDataPerRecord = 0;;
    m_ImgHeader.NbBytesSuffixDataPerRecord = 0;
    m_ImgHeader.NbImageRecords = 0;
    m_ImgHeader.NbLeftBorderPixels = 0;
    m_ImgHeader.NbRightBorderPixels = 0;
    m_ImgHeader.RedChannel = 0;
    m_LeadHeader.AncillaryRecordLength = 0;
    m_LeadHeader.AnnotationRecordLength = 0;
    m_LeadHeader.HeaderRecLength = 0;
    m_LeadHeader.HeaderSize = 0;
    m_LeadHeader.ImageFormatDescription = "";
    m_LeadHeader.NbAncillaryRecords = 0;
    m_LeadHeader.NbAnnotationRecords = 0;
    m_LeadHeader.NbHeaderRecords = 0;
    m_LeadHeader.OffsetToHistoValuesPerRecord = 0;
    m_LeadHeader.SoftwareUsed = "";
    m_VoldHeader.RecordSize = 0;
    }




//-----------------------------------------------------------------------------
// Private
// CleanUpString
//
// Remove SPACE/TAB/ENTER from the begin and end of the file.
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::CleanUpString(string* pio_pString) const
    {
    HPRECONDITION(pio_pString != 0);

    size_t FirstValidPos = 0;
    size_t LastValidPos = 0;
    size_t StrLen = pio_pString->size();

    if (StrLen > 0)
        {

        // Remove the SPACE/TAB at the begin of the string.

        while (FirstValidPos < StrLen && !IsValidChar((*pio_pString)[FirstValidPos]))
            FirstValidPos++;

        // Remove at the end of the string.
        LastValidPos = StrLen;

        while(LastValidPos > 0 && !IsValidChar((*pio_pString)[LastValidPos]))
            LastValidPos--;

        *pio_pString = pio_pString->substr(FirstValidPos, LastValidPos+1);

        }

    }



//-----------------------------------------------------------------------------
// Private
// IsValidChar
//
// Check if the specified character is valid.  Valid character exclude SPACE/
// TAB/ENTER.
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::IsValidChar(const char pi_Char) const
    {
    bool IsValid = true;

    switch (pi_Char)
        {
        case ' ':
        case '\n':
        case '\t':
        case '\r':
        case '-':
        case '+':
            IsValid = false;
            break;
        }

    if(pi_Char < 31)
        IsValid = false;

    return IsValid;
    }

//-----------------------------------------------------------------------------
// Private
// ConvertStringToRadian
//-----------------------------------------------------------------------------
double HRFSpotCAPFile::ConvertStringToRadian(string* pio_pString) const
    {
    char OrientationLetter = pio_pString->substr(0,1).c_str()[0];

    double AngleRadian = 0;
    double Mult;
    uint32_t AngleToAdd;


    switch(OrientationLetter)
        {
        case 'N':
        case 'E':
            Mult = 1.0;
            break;
        case 'S':
        case 'W':
            Mult = -1.0;
            break;
        default:
            Mult = 0;
            break;
        }
    if(Mult !=  0)
        {
        AngleRadian = (double)strtoul(pio_pString->substr(1,3).c_str(), NULL, 10);
        AngleToAdd  =  strtoul(pio_pString->substr(4,2).c_str(), NULL, 10);
        if(AngleToAdd >= 0 && AngleToAdd <= 60)
            {
            AngleRadian += ((double)AngleToAdd)/60;
            AngleToAdd  =  strtoul(pio_pString->substr(6,2).c_str(), NULL, 10);
            if(AngleToAdd >= 0 && AngleToAdd <= 60)
                {
                AngleRadian += ((double)AngleToAdd)/60;
                //conversion from degrees to radian
                //AngleRadian *= PI/180;
                //depending of the orientation
                AngleRadian *= Mult; //the sign
                }
            else
                {
                AngleRadian = 0;
                }

            }
        else
            {
            AngleRadian = 0;
            }


        }

    return AngleRadian;

    }

//-----------------------------------------------------------------------------
// Private
// SetDataTiePoints
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::SetDataTiePoints()
    {
    //set the Tie point data we can
    //the  rest will be set while reading the lead file header
    m_pTiePoints[2] = 0;
    //the z coordinate of the CRS tie points is equal to zero, so we can set it now
    m_pTiePoints[5] = 0;
    m_pTiePoints[8] = 0;
    //the z coordinate of the CRS tie points is equal to zero, so we can set it now
    m_pTiePoints[11] = 0;
    m_pTiePoints[14] = 0;
    //the z coordinate of the CRS tie points is equal to zero, so we can set it now
    m_pTiePoints[17] = 0;
    m_pTiePoints[20] = 0;
    //the z coordinate of the CRS tie points is equal to zero, so we can set it now
    m_pTiePoints[23] = 0;
    }
//-----------------------------------------------------------------------------
// Private
// CreateTransfoModelFromGeoTiff
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFSpotCAPFile::CreateTransfoModel()const
    {
    HASSERT(0); // Not used for now as the transfo model is not good


    HFCPtr<HGF2DTransfoModel> pTransfoModel = new HGF2DIdentity();

    unsigned short NbTiePoints  = 24;
    unsigned short NbPixelScale = 0;


    double pMatrix[4][4];

    GetTransfoMatrixFromScaleAndTiePts (pMatrix, NbTiePoints, m_pTiePoints, NbPixelScale, NULL);
    HFCMatrix<3, 3> TheMatrix;

    TheMatrix[0][0] = pMatrix[0][0];
    TheMatrix[0][1] = pMatrix[0][1];
    TheMatrix[0][2] = pMatrix[0][3];
    TheMatrix[1][0] = pMatrix[1][0];
    TheMatrix[1][1] = pMatrix[1][1];
    TheMatrix[1][2] = pMatrix[1][3];
    TheMatrix[2][0] = pMatrix[3][0];
    TheMatrix[2][1] = pMatrix[3][1];
    TheMatrix[2][2] = pMatrix[3][3];

    pTransfoModel = new HGF2DProjective(TheMatrix);

    // Get the simplest model possible.
    HFCPtr<HGF2DTransfoModel> pTempTransfoModel = pTransfoModel->CreateSimplifiedModel();

    if (pTempTransfoModel != 0)
        pTransfoModel = pTempTransfoModel;

    HGF2DDisplacement Depl(-0.5, -0.5);
    HGF2DTranslation TranslateModel(Depl);
    pTransfoModel = TranslateModel.ComposeInverseWithDirectOf(*pTransfoModel);


    return pTransfoModel;
    }

//-----------------------------------------------------------------------------
// Protected
// GetNbBands
// Get the number of bands
//-----------------------------------------------------------------------------

uint32_t HRFSpotCAPFile::GetNbBands() const
    {
    return m_ImgHeader.NbBands;
    }

//-----------------------------------------------------------------------------
// Protected
// GetImageWidth
// Get the number samples per line
//-----------------------------------------------------------------------------

uint32_t HRFSpotCAPFile::GetImageWidth() const
    {
    return m_ImgHeader.ImageWidth;
    }

//-----------------------------------------------------------------------------
// Protected
// GetImageHeight
// Get the number of rows
//-----------------------------------------------------------------------------

uint32_t HRFSpotCAPFile::GetImageHeight() const
    {
    return m_ImgHeader.ImageHeight;
    }

//-----------------------------------------------------------------------------
// Protected
// GetHeaderSize
// Get size of the IMAG_XX.DAT file header
//-----------------------------------------------------------------------------

int32_t HRFSpotCAPFile::GetHeaderSize() const
    {
    return m_ImgHeader.HeaderSize;
    }

//-----------------------------------------------------------------------------
// Protected
// GetImageHeight
// Get the number of rows
//-----------------------------------------------------------------------------

uint32_t HRFSpotCAPFile::GetNbRightBorderPixels() const
    {
    return m_ImgHeader.NbRightBorderPixels;
    }

//-----------------------------------------------------------------------------
// Protected
// GetNbBytesPrefixDataPerRecord
// Get
//-----------------------------------------------------------------------------

uint32_t HRFSpotCAPFile::GetNbBytesPrefixDataPerRecord() const
    {
    return m_ImgHeader.NbBytesPrefixDataPerRecord;
    }

//-----------------------------------------------------------------------------
// Protected
// GetNbBytesSuffixDataPerRecord
// Get
//-----------------------------------------------------------------------------

uint32_t HRFSpotCAPFile::GetNbBytesSuffixDataPerRecord() const
    {
    return m_ImgHeader.NbBytesSuffixDataPerRecord;
    }

//-----------------------------------------------------------------------------
// Protected
// GetTotalBytesPerRow
// Get
//-----------------------------------------------------------------------------

uint32_t HRFSpotCAPFile::GetTotalBytesPerRow() const
    {
    return  (m_ImgHeader.ImageWidth * m_ImgHeader.NbBitsPerPixel/8) +
            m_ImgHeader.NbBytesPrefixDataPerRecord +
            m_ImgHeader.NbBytesSuffixDataPerRecord +
            m_ImgHeader.NbLeftBorderPixels +
            m_ImgHeader.NbRightBorderPixels;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRFSpotCAPFile::GetRedChannel() const
    {
    return m_ImgHeader.RedChannel;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRFSpotCAPFile::GetGreenChannel() const
    {
    return m_ImgHeader.GreenChannel;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRFSpotCAPFile::GetBlueChannel() const
    {
    return m_ImgHeader.BlueChannel;
    }

//-----------------------------------------------------------------------------
// Public
// Returns a pointer on the LockManager object
//-----------------------------------------------------------------------------
HFCBinStreamLockManager* HRFSpotCAPFile::GetVoldLockManager()
    {
    HPRECONDITION (m_pVoldSharingControl != 0);
    return m_pVoldSharingControl->GetLockManager();
    }


//-----------------------------------------------------------------------------
// Public
// Creates an instance of the HRFSharingControl class.
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::VoldSharingControlCreate(HFCURL* pi_pVoldUrl)
    {
    HPRECONDITION (pi_pVoldUrl != 0);

    if (m_pVoldSharingControl == 0)
        m_pVoldSharingControl = new HRFSisterFileSharing(pi_pVoldUrl, GetAccessMode());
    }

//-----------------------------------------------------------------------------
// Public
// Returns true if the logical counter is desynchronized with the physical one.
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::VoldSharingControlNeedSynchronization()
    {
    HPRECONDITION (m_pVoldSharingControl != 0);

    return m_pVoldSharingControl->NeedSynchronization();
    }

//-----------------------------------------------------------------------------
// Public
// Synchronizes the logical and physical counters.
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::VoldSharingControlSynchronize()
    {
    HPRECONDITION (m_pVoldSharingControl != 0);

    m_pVoldSharingControl->Synchronize();
    }

//-----------------------------------------------------------------------------
// Public
// Increment the physical and logical counters.
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::VoldSharingControlIncrementCount()
    {
    HPRECONDITION (m_pVoldSharingControl != 0);

    m_pVoldSharingControl->IncrementCurrentModifCount();
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HRFSharingControl instance.
//-----------------------------------------------------------------------------
HRFSharingControl* HRFSpotCAPFile::GetVoldSharingControl()
    {
    HPRECONDITION (m_pVoldSharingControl != 0);

    return (m_pVoldSharingControl);
    }

//-----------------------------------------------------------------------------
// Public
// Return true if the sharing control file has been locked.
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::VoldSharingControlIsLocked()
    {
    HPRECONDITION (m_pVoldSharingControl != 0);

    return m_pVoldSharingControl->IsLocked();
    }

//-----------------------------------------------------------------------------
// Public
// Returns a pointer on the LockManager object
//-----------------------------------------------------------------------------
HFCBinStreamLockManager* HRFSpotCAPFile::GetImagLockManager()
    {
    HPRECONDITION (m_pImagSharingControl != 0);
    return m_pImagSharingControl->GetLockManager();
    }


//-----------------------------------------------------------------------------
// Public
// Creates an instance of the HRFSharingControl class.
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::ImagSharingControlCreate(HFCURL* pi_pImagUrl)
    {
    HPRECONDITION (pi_pImagUrl != 0);

    if (m_pImagSharingControl == 0)
        m_pImagSharingControl = new HRFSisterFileSharing(pi_pImagUrl, GetAccessMode());
    }

//-----------------------------------------------------------------------------
// Public
// Returns true if the logical counter is desynchronized with the physical one.
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::ImagSharingControlNeedSynchronization()
    {
    HPRECONDITION (m_pImagSharingControl != 0);

    return m_pImagSharingControl->NeedSynchronization();
    }

//-----------------------------------------------------------------------------
// Public
// Synchronizes the logical and physical counters.
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::ImagSharingControlSynchronize()
    {
    HPRECONDITION (m_pImagSharingControl != 0);

    m_pImagSharingControl->Synchronize();
    }

//-----------------------------------------------------------------------------
// Public
// Increment the physical and logical counters.
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::ImagSharingControlIncrementCount()
    {
    HPRECONDITION (m_pImagSharingControl != 0);

    m_pImagSharingControl->IncrementCurrentModifCount();
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HRFSharingControl instance.
//-----------------------------------------------------------------------------
HRFSharingControl* HRFSpotCAPFile::GetImagSharingControl()
    {
    HPRECONDITION (m_pImagSharingControl != 0);

    return (m_pImagSharingControl);
    }

//-----------------------------------------------------------------------------
// Public
// Return true if the sharing control file has been locked.
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::ImagSharingControlIsLocked()
    {
    HPRECONDITION (m_pImagSharingControl != 0);

    return m_pImagSharingControl->IsLocked();
    }

//-----------------------------------------------------------------------------
// Public
// Returns a pointer on the LockManager object
//-----------------------------------------------------------------------------
HFCBinStreamLockManager* HRFSpotCAPFile::GetLeadLockManager()
    {
    HPRECONDITION (m_pLeadSharingControl != 0);
    return m_pLeadSharingControl->GetLockManager();
    }


//-----------------------------------------------------------------------------
// Public
// Creates an instance of the HRFSharingControl class.
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::LeadSharingControlCreate(HFCURL* pi_pLeadUrl)
    {
    HPRECONDITION (pi_pLeadUrl != 0);

    if (m_pLeadSharingControl == 0)
        m_pLeadSharingControl = new HRFSisterFileSharing(pi_pLeadUrl, GetAccessMode());
    }

//-----------------------------------------------------------------------------
// Public
// Returns true if the logical counter is desynchronized with the physical one.
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::LeadSharingControlNeedSynchronization()
    {
    HPRECONDITION (m_pLeadSharingControl != 0);

    return m_pLeadSharingControl->NeedSynchronization();
    }

//-----------------------------------------------------------------------------
// Public
// Synchronizes the logical and physical counters.
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::LeadSharingControlSynchronize()
    {
    HPRECONDITION (m_pLeadSharingControl != 0);

    m_pLeadSharingControl->Synchronize();
    }

//-----------------------------------------------------------------------------
// Public
// Increment the physical and logical counters.
//-----------------------------------------------------------------------------
void HRFSpotCAPFile::LeadSharingControlIncrementCount()
    {
    HPRECONDITION (m_pLeadSharingControl != 0);

    m_pLeadSharingControl->IncrementCurrentModifCount();
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HRFSharingControl instance.
//-----------------------------------------------------------------------------
HRFSharingControl* HRFSpotCAPFile::GetLeadSharingControl()
    {
    HPRECONDITION (m_pLeadSharingControl != 0);

    return (m_pLeadSharingControl);
    }

//-----------------------------------------------------------------------------
// Public
// Return true if the sharing control file has been locked.
//-----------------------------------------------------------------------------
bool HRFSpotCAPFile::LeadSharingControlIsLocked()
    {
    HPRECONDITION (m_pLeadSharingControl != 0);

    return m_pLeadSharingControl->IsLocked();
    }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// The rest of this file is the implentation of the sharing control for the
// Creator struct.
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Public
// Creates an instance of the HRFSharingControl class.
//-----------------------------------------------------------------------------
void HRFSpotCAPCreator::ImagSharingControlCreate(const HFCPtr<HFCURL>& pi_pURL)
    {
    HPRECONDITION (pi_pURL != 0);

    if (m_pImagSharingControl == 0)
        m_pImagSharingControl = new HRFSisterFileSharing(pi_pURL, HFC_READ_ONLY);
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HRFSharingControl instance.
//-----------------------------------------------------------------------------
HRFSharingControl* HRFSpotCAPCreator::GetImagSharingControl() const
    {
    HPRECONDITION (m_pImagSharingControl != 0);

    return (m_pImagSharingControl);
    }

//-----------------------------------------------------------------------------
// Public
// Return a pointer on the HFCBinStreamLockManager instance.
//-----------------------------------------------------------------------------
HFCBinStreamLockManager* HRFSpotCAPCreator::GetImagLockManager() const
    {
    HPRECONDITION (m_pImagSharingControl != 0);

    return m_pImagSharingControl->GetLockManager();
    }
