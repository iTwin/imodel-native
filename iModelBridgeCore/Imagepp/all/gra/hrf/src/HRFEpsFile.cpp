//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFEpsFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFEpsFile.h>
#include <Imagepp/all/h/HRFEpsLineEditor.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>


static const string   s_EndOfLine("\r\n");
static const string   s_FileHeader("%!PS-Adobe-3.0 EPSF-3.0");
static const string   s_BoundingBoxStatement("%%BoundingBox: ");
static const string   s_PicstrStatement("picstr");
static const string   s_StringdefStatement("string def");
static const string   s_ScaleStatement("scale");
static const string   s_CurrentfileStatement("currentfile");
static const string   s_ReadhexstringStatement("readhexstring");
static const string   s_PopStatement("pop");
static const string   s_ImageStatement("image");
static const string   s_ColorimageStatement("colorimage");
static const string   s_FalseStatement("false");
static const string   s_EndCommentsStatement("%%EndComments");
static const string   s_DocumentDataStatement("%%DocumentData: Clean7Bit");
static const string   s_TitleStatement("%%Title: ");
static const string   s_EOFStatement("%%EOF");



/** -----------------------------------------------------------------------------
    HRFEpsFile block capabilities
    -----------------------------------------------------------------------------
*/
class HRFEpsBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFEpsBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add(new HRFLineCapability(HFC_WRITE_AND_CREATE,
                                  LONG_MAX,
                                  HRFBlockAccess::SEQUENTIAL));
        }
    };


/** -----------------------------------------------------------------------------
    HRFEpsFile codec capabilities
    -----------------------------------------------------------------------------
*/
class HRFEpsCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFEpsCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_WRITE_AND_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFEpsBlockCapabilities()));
        }
    };


/** -----------------------------------------------------------------------------
    HRFEpsFile capabilities constructor
    -----------------------------------------------------------------------------
*/
HRFEpsCapabilities::HRFEpsCapabilities()
    : HRFRasterFileCapabilities()
    {
    // HRPPixelTypeV1Gray1
    Add(new HRFPixelTypeCapability(HFC_WRITE_AND_CREATE,
                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                   new HRFEpsCodecCapabilities()));

    // HRPPixelTypeV8Gray8
    Add(new HRFPixelTypeCapability(HFC_WRITE_AND_CREATE,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFEpsCodecCapabilities()));

    // HRPPixelTypeV24R8G8B8
    Add(new HRFPixelTypeCapability(HFC_WRITE_AND_CREATE,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFEpsCodecCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_WRITE_AND_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_WRITE_AND_CREATE, HRFInterleaveType::PIXEL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_WRITE_AND_CREATE));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_WRITE_AND_CREATE));
    }


HFC_IMPLEMENT_SINGLETON(HRFEpsCreator)


/** -----------------------------------------------------------------------------
    Default constructor
    -----------------------------------------------------------------------------
*/
HRFEpsCreator::HRFEpsCreator()
    : HRFRasterFileCreator(HRFEpsFile::CLASS_ID)
    {
    // capabilities instance member initialization
    m_pCapabilities = 0;
    }


/** -----------------------------------------------------------------------------
    Retrieve a string identifying the EPS file format
    -----------------------------------------------------------------------------
*/
WString HRFEpsCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_EncapsulatedPostScript()); // Encapsulated PostScript
    }


/** -----------------------------------------------------------------------------
    Get possible URL schemes for the file format
    -----------------------------------------------------------------------------
*/
WString HRFEpsCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }


/** -----------------------------------------------------------------------------
    Retrieve the file format extension list
    -----------------------------------------------------------------------------
*/
WString HRFEpsCreator::GetExtensions() const
    {
    return WString(L"*.eps");
    }


/** -----------------------------------------------------------------------------
    Open an Eps image file
    -----------------------------------------------------------------------------
*/
HFCPtr<HRFRasterFile> HRFEpsCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode         pi_AccessMode,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFEpsFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


/** -----------------------------------------------------------------------------
    Opens the file and verifies if it is an Eps file
    -----------------------------------------------------------------------------
*/
bool HRFEpsCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    // Can't open anything
    return false;
    }


/** -----------------------------------------------------------------------------
    Obtain the capabilities of the Eps file format.
    -----------------------------------------------------------------------------
*/
const HFCPtr<HRFRasterFileCapabilities>& HRFEpsCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFEpsCapabilities();

    return m_pCapabilities;
    }


/** -----------------------------------------------------------------------------
    Constructor. Open an Eps file
    -----------------------------------------------------------------------------
*/
HRFEpsFile::HRFEpsFile(const HFCPtr<HFCURL>& pi_rURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    m_IsOpen = false;

    // if the access is in create mode we create a new empty file otherwise we open the existent file
    if (GetAccessMode().m_HasCreateAccess)
        {
        Create();
        }
    else
        {
        // We can only create new files.
        HASSERT(false);
        }
    }


/** -----------------------------------------------------------------------------
    Destructor
    -----------------------------------------------------------------------------
*/
HRFEpsFile::~HRFEpsFile()
    {
    try
        {
        SaveEpsFile(true);
        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }
    }


/** -----------------------------------------------------------------------------
    Retrieve the world identificator for the file
    -----------------------------------------------------------------------------
*/
const HGF2DWorldIdentificator HRFEpsFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }


/** -----------------------------------------------------------------------------
    Create a new resolution editor
    -----------------------------------------------------------------------------
*/
HRFResolutionEditor* HRFEpsFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFEpsLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

void HRFEpsFile::Save()
    {
    //Keep last file position
    uint64_t CurrentPos = m_pFile->GetCurrentPos();

    SaveEpsFile(false);

    m_pFile->Flush();

    //Set back position
    m_pFile->SeekToPos(CurrentPos);
    }

/** -----------------------------------------------------------------------------
Get the file's current size.
-----------------------------------------------------------------------------
*/
uint64_t HRFEpsFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pFile);
    }


/** -----------------------------------------------------------------------------
    Add a new page to the file
    -----------------------------------------------------------------------------
*/
bool HRFEpsFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);
    HPRECONDITION(pi_pPage != 0);

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(0);

    WriteHeader(pResolutionDescriptor);

    return true;
    }


/** -----------------------------------------------------------------------------
    Retrieve the capabilities of the file
    -----------------------------------------------------------------------------
*/
const HFCPtr<HRFRasterFileCapabilities>& HRFEpsFile::GetCapabilities () const
    {
    return (HRFEpsCreator::GetInstance()->GetCapabilities());
    }


/** -----------------------------------------------------------------------------
    Create a new encapsulated postscript file
    -----------------------------------------------------------------------------
*/
bool HRFEpsFile::Create()
    {
    // Open the file.
    m_pFile = HFCBinStream::Instanciate(GetURL(), GetAccessMode(), 0, true);

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    m_IsOpen = true;

    return true;
    }


/** -----------------------------------------------------------------------------
    Saves the eps file and close it if needed
    -----------------------------------------------------------------------------
*/
void HRFEpsFile::SaveEpsFile(bool pi_CloseFile)
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        WriteFooter();

        if(pi_CloseFile)
            m_IsOpen = false;
        }
    }


/** -----------------------------------------------------------------------------
    Write the eps file header
    -----------------------------------------------------------------------------
*/
void HRFEpsFile::WriteHeader(HFCPtr<HRFResolutionDescriptor>& pi_rpResDescriptor)
    {
    HASSERT(m_pFile != 0);

    char Temp[256];

    uint32_t FileWidth  = (uint32_t)pi_rpResDescriptor->GetWidth();
    uint32_t FileHeight = (uint32_t)pi_rpResDescriptor->GetHeight();

    bool IsRGBFile = pi_rpResDescriptor->GetPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID);
    bool Is1Bit    = pi_rpResDescriptor->GetPixelType()->IsCompatibleWith(HRPPixelTypeV1Gray1::CLASS_ID);

    // Lock the sister file for the GetBmpInfoHeaderFromFile method
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Start line
    m_pFile->Write(s_FileHeader.c_str(), s_FileHeader.size());
    m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());

    // Boundingbox
    sprintf(Temp, "0 0 %ld %ld", FileWidth, FileHeight);
    m_pFile->Write(s_BoundingBoxStatement.c_str(), s_BoundingBoxStatement.size());
    m_pFile->Write(Temp, (uint32_t)strlen(Temp));
    m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());

    // Title
    m_pFile->Write(s_TitleStatement.c_str(), s_TitleStatement.size());
    WString::size_type Pos = 0;
    if ((GetURL()->GetSchemeSpecificPart().substr(Pos,2) == L"//") ||
        (GetURL()->GetSchemeSpecificPart().substr(Pos,2) == L"\\\\"))
        {
        Pos += 2;
        }
    AString urlA(GetURL()->GetSchemeSpecificPart().c_str() + Pos);
    m_pFile->Write(urlA.c_str(), urlA.size());
    m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());

    // Document data
    m_pFile->Write(s_DocumentDataStatement.c_str(), s_DocumentDataStatement.size());
    m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());

    // end comments
    m_pFile->Write(s_EndCommentsStatement.c_str(), s_EndCommentsStatement.size());
    m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());

    // variable definition
    // It must be big enough to hold one line of data.
    sprintf(Temp,
            "/%s %ld %s",
            s_PicstrStatement.c_str(),
            IsRGBFile ? FileWidth * 3 : FileWidth,
            s_StringdefStatement.c_str());
    m_pFile->Write(Temp, strlen(Temp));
    m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());

    // Scale to dimensions
    sprintf(Temp, "%ld %ld %s", FileWidth, FileHeight, s_ScaleStatement.c_str());
    m_pFile->Write(Temp, strlen(Temp));
    m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());

    // Image data information. Fixed Upper left horizontal
    sprintf(Temp,
            "%ld %ld %d [%ld 0 0 -%ld 0 %ld]",
            FileWidth,
            FileHeight,
            Is1Bit ? 1 : 8,
            FileWidth,
            FileHeight,
            FileHeight);
    m_pFile->Write(Temp, strlen(Temp));
    m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());

    // Procedure
    sprintf(Temp,
            "{%s %s %s %s}",
            s_CurrentfileStatement.c_str(),
            s_PicstrStatement.c_str(),
            s_ReadhexstringStatement.c_str(),
            s_PopStatement.c_str());
    m_pFile->Write(Temp, strlen(Temp));
    m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());

    // Image start
    if (IsRGBFile)
        {
        // Specify one data source, and "colorimage" marks the start
        sprintf(Temp, "%s 3", s_FalseStatement.c_str());
        m_pFile->Write(Temp, strlen(Temp));
        m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());
        m_pFile->Write(s_ColorimageStatement.c_str(), s_ColorimageStatement.size());
        m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());
        }
    else
        {
        // Grayscale (1 or 8 bits)

        // "image" marks the start
        m_pFile->Write(s_ImageStatement.c_str(), s_ImageStatement.size());
        m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());
        }

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();
    }


/** -----------------------------------------------------------------------------
    Write the eps file footer
    -----------------------------------------------------------------------------
*/
void HRFEpsFile::WriteFooter()
    {
    HASSERT(m_pFile != 0);

    m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());
    m_pFile->Write(s_EOFStatement.c_str(), s_EOFStatement.size());
    m_pFile->Write(s_EndOfLine.c_str(), s_EndOfLine.size());
    }
