//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFLRDFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFLRDFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFLRDFile.h>
#include <Imagepp/all/h/HRFLRDLineEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>

#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DIdentity.h>

#include <Imagepp/all/h/HCDCodecLRDRLE.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>


//-----------------------------------------------------------------------------
// HRFLRDBlockCapabilities
//-----------------------------------------------------------------------------
class HRFLRDBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFLRDBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,        // AccessMode
                                  LONG_MAX,                     // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL)); // BlockAccess
        }
    };

//-----------------------------------------------------------------------------
// HRFLRDCodecCapabilities
//-----------------------------------------------------------------------------
class HRFLRDCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFLRDCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec HCDCodecLRDRLE
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLRDRLE::CLASS_ID,
                                   new HRFLRDBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFLRDCapabilities
//-----------------------------------------------------------------------------
HRFLRDCapabilities::HRFLRDCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV1Gray1
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE, HRPPixelTypeV1Gray1::CLASS_ID, new HRFLRDCodecCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_RIGHT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::LOWER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::LOWER_RIGHT_VERTICAL));

    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DProjective::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DIdentity::CLASS_ID));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeBackground(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDontSupportPersistentColor(true)));
    }

HFC_IMPLEMENT_SINGLETON(HRFLRDCreator)

//-----------------------------------------------------------------------------
// Creator
// This is the creator to instantiate LRD format
//-----------------------------------------------------------------------------

HRFLRDCreator::HRFLRDCreator()
    : HRFRasterFileCreator(HRFLRDFile::CLASS_ID)
    {
    // LRD capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// public
// Identification information
//-----------------------------------------------------------------------------

WString HRFLRDCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_LRD()); // LRD File Format
    }

//-----------------------------------------------------------------------------
// public
// Schemes information
//-----------------------------------------------------------------------------

WString HRFLRDCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// public
// Extensions information
//-----------------------------------------------------------------------------

WString HRFLRDCreator::GetExtensions() const
    {
    return WString(L"*.lrd");
    }

//-----------------------------------------------------------------------------
// public
// allow to Open an image file
//-----------------------------------------------------------------------------

HFCPtr<HRFRasterFile> HRFLRDCreator::Create( const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode,
                                             uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFLRDFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// public
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------

bool HRFLRDCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // This method read and verrify the presence of some ASCII character.
    // As for any valid LRD compliant raster file, we doesn't support unicode
    // or multi-byte char.  We MUST use here some function NOT unicode compliant.

    bool                   bResult = false;
    HAutoPtr<HFCBinStream>  pFile;

    (const_cast<HRFLRDCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the LRD File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        // Check if the file is a valid CRL
        pFile->SeekToBegin();

        char MagicString[5] = {0};

        if (pFile->Read(&MagicString, 5) == 5)
            bResult = !strncmp(MagicString, ".vec", 4);
        }
    SisterFileLock.ReleaseKey();

    HASSERT(!(const_cast<HRFLRDCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFLRDCreator*>(this))->m_pSharingControl = 0;

    return bResult;
    }

//-----------------------------------------------------------------------------
// public
// Create or get the singleton capabilities of LRD file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFLRDCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFLRDCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFLRDFile::HRFLRDFile(const HFCPtr<HFCURL>& pi_rURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset)
    :HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    // Initialise internal members...

    m_IsOpen             = false;
    m_HasHeaderFilled    = false;
    m_IsBigEndian        = false;

    m_BitPerPixel        = 0;
    m_pLRDFile           = 0;
    m_pLRDHeader         = 0;
    m_Width              = 0;
    m_Height             = 0;

    m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;

    if (GetAccessMode().m_HasCreateAccess)
        Create();
    else
        {
        if (Open())                 // if Open success and it is not a new
            CreateDescriptors();    // file Create Page and Res Descriptors.
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFLRDFile::HRFLRDFile(const HFCPtr<HFCURL>& pi_rURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset,
                       bool                 pi_DontOpenFile)
    :HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // Initialise internal members...
    m_IsOpen             = false;
    m_HasHeaderFilled    = false;
    m_IsBigEndian        = false;

    m_BitPerPixel        = 0;
    m_pLRDFile           = 0;
    m_pLRDHeader         = 0;
    m_Width              = 0;
    m_Height             = 0;

    m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRFLRDFile::~HRFLRDFile()
    {
    // Close the LRD file and free the header structure

    try
        {
        SaveLRDFile(true);

        }

    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------

HRFResolutionEditor* HRFLRDFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    // verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return new HRFLRDLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------

void HRFLRDFile::Save()
    {
    //Keep last file position
    uint64_t CurrentPos = m_pLRDFile->GetCurrentPos();

    SaveLRDFile(false);

    //Set back position
    m_pLRDFile->SeekToPos(CurrentPos);
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------

bool HRFLRDFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);
    m_HasHeaderFilled = CreateFileHeader(pi_pPage);

    return m_HasHeaderFilled;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFLRDFile::GetCapabilities () const
    {
    return (HRFLRDCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Protected
// This method open the file.
//-----------------------------------------------------------------------------

bool HRFLRDFile::Open()
    {
    HPRECONDITION(!m_IsOpen);
    HPRECONDITION(!m_pLRDFile);

    // Be sure the Intergraph raster file is NOT already open.
    m_pLRDFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

    m_IsOpen = true;

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    return m_IsOpen;
    }


//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFLRDFile::CreateDescriptors ()
    {
    HPRECONDITION (m_IsOpen);

    // Initialise some members and read file header Read
    InitOpenedFile();

    // Obtain Resolution Information
    // resolution dimension
    HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
        GetAccessMode(),                        // AccessMode,
        GetCapabilities(),                      // Capabilities,
        1.0,                                    // XResolutionRatio,
        1.0,                                    // YResolutionRatio,
        GetPixelType(),                         // PixelType,
        new HCDCodecLRDRLE(),                   // Codec
        HRFBlockAccess::SEQUENTIAL,             // RStorageAccess,
        HRFBlockAccess::SEQUENTIAL,             // WStorageAccess,
        GetScanlineOrientation(),               // ScanLineOrientation,
        HRFInterleaveType::PIXEL,               // InterleaveType
        false,                                  // IsInterlace
        m_Width,                                // Width,
        m_Height,                               // Height,
        m_Width,                                // BlockWidth,
        1,                                      // BlockHeight,
        0,                                      // BlocksDataFlag
        HRFBlockType::LINE);                    // BlockType

    // BACKGROUND Tag
    HPMAttributeSet TagList;
    HFCPtr<HPMGenericAttribute> pTag;
    pTag = new HRFAttributeBackground(0);
    TagList.Set(pTag);

    // Persistent color Tag
    pTag = new HRFAttributeDontSupportPersistentColor(true);
    TagList.Set(pTag);

    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor (GetAccessMode(),
                                   GetCapabilities(),           // Capabilities
                                   pResolution,                 // ResolutionDescriptor
                                   0,                           // RepresentativePalette
                                   0,                           // Histogram
                                   0,                           // Thumbnail
                                   0,                           // ClipShape
                                   GetTransfoModel(),           // TransfoModel
                                   0,                           // Filters
                                   &TagList);                   // Defined Tag

    m_ListOfPageDescriptor.push_back(pPage);
    }


//-----------------------------------------------------------------------------
// Private
// This method close the file.
//-----------------------------------------------------------------------------

void HRFLRDFile::SaveLRDFile(bool pi_CloseFile)
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        HASSERT(m_pLRDHeader != 0);
        HASSERT(m_pLRDFile != 0);


        // Should do something here ???
        // At least re-write the tag !!!

        if ( (GetAccessMode().m_HasWriteAccess) || (GetAccessMode().m_HasCreateAccess) )
            {
            // Update the Recordcount and the file size ???
            HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

            uint32_t FileSize = (uint32_t)m_pLRDFile->GetSize();
            uint32_t BufferSize = 512 - (FileSize % 512);

            if (BufferSize < 512)
                {
                unsigned short* pPaddingBuffer = new unsigned short[BufferSize / 2];
                memset(pPaddingBuffer, 0, BufferSize / 2);

                m_pLRDFile->Write(pPaddingBuffer, BufferSize);
                HASSERT(!(m_pLRDFile->GetSize() % 512));
                }
            else
                BufferSize = 0;

            int32_t Recordcount = (FileSize + BufferSize) / 512;
            bool WriteHeader = false;

            if (Recordcount != m_pLRDHeader->Recordcount)
                {
                m_pLRDHeader->Recordcount = (FileSize + BufferSize) / 512;
                WriteHeader = true;
                }

            // Update the TransfoModel
            if ((pPageDescriptor->HasTransfoModel()) && (pPageDescriptor->TransfoModelHasChanged()))
                {
                WriteTransfoModel(pPageDescriptor->GetTransfoModel());
                WriteHeader = true;
                }

            // Update the raster file header.
            if (WriteHeader)
                {
                m_pLRDFile->SeekToBegin();
                m_pLRDFile->Write(m_pLRDHeader, sizeof(LRDHeaderBlock)); // HRF_LRD_BLOCK_SIZE
                }

            m_pLRDFile->Flush();

            pPageDescriptor->Saved();
            }

        if (pi_CloseFile)
            {
            // Close and flush the BinStream
            m_pLRDFile = 0;

            delete m_pLRDHeader;
            m_pLRDHeader = 0;

            // Set the open flag to false
            m_IsOpen = false;
            }

        }
    }

//-----------------------------------------------------------------------------
// Private
// This method create the file.
//-----------------------------------------------------------------------------

bool HRFLRDFile::Create()
    {
    // Be sure the Intergraph raster file is NOT already open.
    HPRECONDITION(!m_IsOpen);
    HPRECONDITION(!m_pLRDFile);

    m_pLRDFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

    m_IsOpen = true;

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    return m_IsOpen;
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------

bool HRFLRDFile::ComputeRasterDimension()
    {
    HPRECONDITION(m_HasHeaderFilled);
    HPRECONDITION(((m_pLRDHeader->bStop - m_pLRDHeader->bStart) + 1) > 1);

    bool Status = false;

    m_Width  = (m_pLRDHeader->bStop - m_pLRDHeader->bStart) + 1;
    m_Height = (m_pLRDHeader->rStop - m_pLRDHeader->rStart) + 1;
    Status   = true;

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------

HFCPtr<HRPPixelType> HRFLRDFile::GetPixelType() const
    {
    HPRECONDITION(m_HasHeaderFilled);
    return m_pPixelType;
    }

//-----------------------------------------------------------------------------
// Publics
// Create a new header and write it to a file stream.
// On error return false.
//-----------------------------------------------------------------------------

bool HRFLRDFile::CreateFileHeader(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pLRDFile != 0);
    HPRECONDITION(!m_pLRDHeader);


    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pi_pPage->GetResolutionDescriptor(0);

    HFCPtr<HRPPixelType> pPixelType = pResolutionDescriptor->GetPixelType();

    // Create an header block
    if (CreateHeaderBlock(pResolutionDescriptor))
        {
        // Lock the sister file
        HFCLockMonitor SisterFileLock(GetLockManager());

        // Feed the header struct.
        strncpy(m_pLRDHeader->MagicWord, ".vec", 4);
        m_pLRDHeader->VecLevel    = 1;
        m_pLRDHeader->Recordcount = 1;

        AsciiDate (m_pLRDHeader->Date);   // Should look like : "22-Jun-04"
        AsciiTime (m_pLRDHeader->Time);   // Should look like : "15:22:59"
        memset(m_pLRDHeader->rFile, 0, 40);

        m_pLRDHeader->bStart      = 1;
        m_pLRDHeader->bStop       = (uint32_t)pResolutionDescriptor->GetWidth();
        m_pLRDHeader->BitsCount   = 0;
        m_pLRDHeader->rStart      = 1;
        m_pLRDHeader->rStop       = (uint32_t)pResolutionDescriptor->GetHeight();
        m_pLRDHeader->maxmis      = 0;
        m_pLRDHeader->minwid      = 0;
        m_pLRDHeader->eps         = 0;
        m_pLRDHeader->mintr0      = 0;
        m_pLRDHeader->maxtr0      = 0;
        m_pLRDHeader->fuzz        = 0;
        m_pLRDHeader->widfly      = 0;
        m_pLRDHeader->widhol      = 0;
        m_pLRDHeader->talhol      = 0;
        m_pLRDHeader->newtop      = 0;
        m_pLRDHeader->botmin      = 0;
        m_pLRDHeader->hmod        = 999;
        m_pLRDHeader->corsys      = 4;
        m_pLRDHeader->xpdens      = 72;
        m_pLRDHeader->ypdens      = 72;
        m_pLRDHeader->vsmin       = 1;
        m_pLRDHeader->vsmax       = (uint32_t)pResolutionDescriptor->GetHeight();
        m_pLRDHeader->vxmin       = 1;
        m_pLRDHeader->vxmax       = (uint32_t)pResolutionDescriptor->GetWidth();
        memset(m_pLRDHeader->TheExpense, 0, 208);
        m_pLRDHeader->valid       = 1;
        m_pLRDHeader->slo         = 4;

        m_pLRDHeader->Matrix[0]   = 1.0;
        m_pLRDHeader->Matrix[1]   = 0.0;
        m_pLRDHeader->Matrix[2]   = 0.0;
        m_pLRDHeader->Matrix[3]   = 0.0;

        m_pLRDHeader->Matrix[4]   = 0.0;
        m_pLRDHeader->Matrix[5]   = 1.0;
        m_pLRDHeader->Matrix[6]   = 0.0;
        m_pLRDHeader->Matrix[7]   = 0.0;

        m_pLRDHeader->Matrix[8]   = 0.0;
        m_pLRDHeader->Matrix[9]   = 0.0;
        m_pLRDHeader->Matrix[10]  = 1.0;
        m_pLRDHeader->Matrix[11]  = 0.0;

        m_pLRDHeader->Matrix[12]  = 0.0;
        m_pLRDHeader->Matrix[13]  = 0.0;
        m_pLRDHeader->Matrix[14]  = 0.0;
        m_pLRDHeader->Matrix[15]  = 1.0;

        // Write freshly created header physically into the file...
        m_pLRDFile->Write(m_pLRDHeader, sizeof(LRDHeaderBlock)); // HRF_LRD_BLOCK_SIZE

        // Unlock the sister file
        SisterFileLock.ReleaseKey();

        // Initialise some members and read file header Read
        m_HasHeaderFilled = true;
        InitOpenedFile();
        }
    return m_HasHeaderFilled;
    }

//-----------------------------------------------------------------------------
// Publics
// Create a new header and write it to a file stream.
// On error return false.
//-----------------------------------------------------------------------------

bool HRFLRDFile::CreateHeaderBlock(HRFResolutionDescriptor* pi_pResolutionDescriptor)
    {
    // This method write and modify some ASCII character.
    // As for any valid LRD raster file we doesn't support unicode
    // or multi-byte char.  We MUST use here some function NOT unicode compliant.

    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pLRDFile != 0);
    HPRECONDITION(m_pLRDHeader == 0);

    bool Result = true;

    // Use the most well recognized format.
    m_pLRDHeader = new LRDHeaderBlock;

    // Construct a standard header block information...
    memset(m_pLRDHeader, 0, sizeof(LRDHeaderBlock));

    return Result;
    }

//-----------------------------------------------------------------------------
// Private
// CreatePixelTypeFromFile
//-----------------------------------------------------------------------------

void HRFLRDFile::InitOpenedFile()
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pLRDFile != 0);

    if (!GetAccessMode().m_HasCreateAccess)
        {
        // Lock the sister file
        HFCLockMonitor SisterFileLock(GetLockManager());
        m_pLRDHeader = new LRDHeaderBlock;

        m_pLRDFile->SeekToBegin();

        // Read and fill the Type1BlockHeader
        m_pLRDFile->Read(m_pLRDHeader, sizeof(LRDHeaderBlock) ); // HRF_LRD_BLOCK_SIZE);
        m_HasHeaderFilled = true;

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();
        }

    if (m_HasHeaderFilled)
        {
        if (m_pLRDHeader->VecLevel == 1)
            m_IsBigEndian = false;
        else if (m_pLRDHeader->VecLevel == 0x01000000)
            m_IsBigEndian = true;
        else
            throw HFCCorruptedFileException(GetURL()->GetURL()); // InvalidFile or corrupted file, or..

        if (m_IsBigEndian)
            {
            // Swap some members.
            swaplong((char*) &m_pLRDHeader->Recordcount);
            swaplong((char*) &m_pLRDHeader->bStart     );
            swaplong((char*) &m_pLRDHeader->bStop      );
            swaplong((char*) &m_pLRDHeader->BitsCount  );
            swaplong((char*) &m_pLRDHeader->rStart     );
            swaplong((char*) &m_pLRDHeader->rStop      );
            swaplong((char*) &m_pLRDHeader->maxmis     );
            swaplong((char*) &m_pLRDHeader->minwid     );
            swaplong((char*) &m_pLRDHeader->eps        );
            swaplong((char*) &m_pLRDHeader->mintr0     );
            swaplong((char*) &m_pLRDHeader->maxtr0     );
            swaplong((char*) &m_pLRDHeader->fuzz       );
            swaplong((char*) &m_pLRDHeader->widfly     );
            swaplong((char*) &m_pLRDHeader->widhol     );
            swaplong((char*) &m_pLRDHeader->talhol     );
            swaplong((char*) &m_pLRDHeader->newtop     );
            swaplong((char*) &m_pLRDHeader->botmin     );
            swaplong((char*) &m_pLRDHeader->hmod       );
            swaplong((char*) &m_pLRDHeader->corsys     );
            swaplong((char*) &m_pLRDHeader->xpdens     );
            swaplong((char*) &m_pLRDHeader->ypdens     );
            swaplong((char*) &m_pLRDHeader->vsmin      );
            swaplong((char*) &m_pLRDHeader->vsmax      );
            swaplong((char*) &m_pLRDHeader->vxmin      );
            swaplong((char*) &m_pLRDHeader->vxmax      );

            swapdouble((char*) &m_pLRDHeader->Matrix[0]  );
            swapdouble((char*) &m_pLRDHeader->Matrix[1]  );
            swapdouble((char*) &m_pLRDHeader->Matrix[2]  );
            swapdouble((char*) &m_pLRDHeader->Matrix[3]  );
            swapdouble((char*) &m_pLRDHeader->Matrix[4]  );
            swapdouble((char*) &m_pLRDHeader->Matrix[5]  );
            swapdouble((char*) &m_pLRDHeader->Matrix[6]  );
            swapdouble((char*) &m_pLRDHeader->Matrix[7]  );
            swapdouble((char*) &m_pLRDHeader->Matrix[8]  );
            swapdouble((char*) &m_pLRDHeader->Matrix[9]  );
            swapdouble((char*) &m_pLRDHeader->Matrix[10] );
            swapdouble((char*) &m_pLRDHeader->Matrix[11] );
            swapdouble((char*) &m_pLRDHeader->Matrix[12] );
            swapdouble((char*) &m_pLRDHeader->Matrix[13] );
            swapdouble((char*) &m_pLRDHeader->Matrix[14] );
            swapdouble((char*) &m_pLRDHeader->Matrix[15] );
            }

        if (m_pLRDHeader->hmod != 999)
            {
            m_pLRDHeader->corsys = 1;
            m_pLRDHeader->xpdens = 1000;
            m_pLRDHeader->ypdens = 1000;
            m_pLRDHeader->vsmin  = m_pLRDHeader->rStart;
            m_pLRDHeader->vsmax  = m_pLRDHeader->rStop;
            m_pLRDHeader->vxmin  = m_pLRDHeader->bStart;
            m_pLRDHeader->vxmax  = m_pLRDHeader->bStop;
            }

        // Adjust pixel density (DPI)
        if (m_pLRDHeader->xpdens <= 0)
            m_pLRDHeader->xpdens = 200;

        if (m_pLRDHeader->ypdens <= 0)
            m_pLRDHeader->ypdens = 200;

        // Get some basic information contained in the header
        ComputeRasterDimension();

        // Set a pixelType...
        m_pPixelType = new HRPPixelTypeV1Gray1();

        // Create and initialise a new codec
        m_pCodec = new HCDCodecLRDRLE(m_Width, m_Height);
        m_pCodec->SetSubset(m_Width,1);

        m_BitPerPixel     = 1;
        m_HasHeaderFilled = true;
        }
    }
//-----------------------------------------------------------------------------
// swap long word
//-----------------------------------------------------------------------------

int HRFLRDFile::swaplong ( char buf[] )
    {
    char temp[4];

    temp[0] = buf[3];
    temp[1] = buf[2];
    temp[2] = buf[1];
    temp[3] = buf[0];

    buf[0] = temp[0];
    buf[1] = temp[1];
    buf[2] = temp[2];
    buf[3] = temp[3];

    return 0;
    }

//-----------------------------------------------------------------------------
// swap double word
//-----------------------------------------------------------------------------

int HRFLRDFile::swapdouble ( char buf[] )
    {
    char temp[8];

    temp[0] = buf[7];
    temp[1] = buf[6];
    temp[2] = buf[5];
    temp[3] = buf[4];
    temp[4] = buf[3];
    temp[5] = buf[2];
    temp[6] = buf[1];
    temp[7] = buf[0];

    buf[0] = temp[0];
    buf[1] = temp[1];
    buf[2] = temp[2];
    buf[3] = temp[3];
    buf[4] = temp[4];
    buf[5] = temp[5];
    buf[6] = temp[6];
    buf[7] = temp[7];

    return 0;
    }

//-----------------------------------------------------------------------------
// swap double word
//-----------------------------------------------------------------------------

HFCPtr<HGF2DTransfoModel> HRFLRDFile::GetTransfoModel() const
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pLRDHeader != 0);

    HFCPtr<HGF2DTransfoModel> pTransfoModel = new HGF2DAffine();
    HRFScanlineOrientation  scanlineOrientation = GetScanlineOrientation();

    if (m_pLRDHeader->valid)
        {
        HFCMatrix<3, 3> MyMatrix;

        MyMatrix[0][0] = m_pLRDHeader->Matrix[0];
        MyMatrix[0][1] = m_pLRDHeader->Matrix[1];
        MyMatrix[0][2] = m_pLRDHeader->Matrix[3];
        MyMatrix[1][0] = m_pLRDHeader->Matrix[4];
        MyMatrix[1][1] = m_pLRDHeader->Matrix[5];
        MyMatrix[1][2] = m_pLRDHeader->Matrix[7];
        MyMatrix[2][0] = m_pLRDHeader->Matrix[12];
        MyMatrix[2][1] = m_pLRDHeader->Matrix[13];
        MyMatrix[2][2] = m_pLRDHeader->Matrix[15];

        // SLO 0, SLO 1, SLO 2, SLO 3
        if (scanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL ||
            scanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL ||
            scanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL ||
            scanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
            {
            ((HFCPtr<HGF2DAffine> &)pTransfoModel)->SetByMatrixParameters(MyMatrix[0][2],
                                                                          MyMatrix[1][1],
                                                                          -MyMatrix[1][0],
                                                                          MyMatrix[1][2],
                                                                          MyMatrix[0][1],
                                                                          MyMatrix[0][0]);
            }
        // SLO 4, SLO 5, SLO 6, SLO 7
        else if (scanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL ||
                 scanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL ||
                 scanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL ||
                 scanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
            {
            ((HFCPtr<HGF2DAffine> &)pTransfoModel)->SetByMatrixParameters(MyMatrix[0][2],
                                                                          MyMatrix[0][0],
                                                                          -MyMatrix[0][1],
                                                                          MyMatrix[1][2],
                                                                          MyMatrix[1][0],
                                                                          -MyMatrix[1][1]);
            }
        }
    return pTransfoModel;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateScanlineOrentationModel
//-----------------------------------------------------------------------------

const HRFScanlineOrientation HRFLRDFile::GetScanlineOrientation() const
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pLRDHeader != 0);

    HRFScanlineOrientation ScanlineOrientation;

    switch (m_pLRDHeader->slo)
        {

        case 0:            // Origin : Upper Left      Line Orentation : Vertical
            ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_VERTICAL;
            break;

        case 1:            // Origin : Upper Right     Line Orentation : Vertical
            ScanlineOrientation = HRFScanlineOrientation::UPPER_RIGHT_VERTICAL;
            break;

        case 2:            // Origin : Lower Left      Line Orentation : Vertical
            ScanlineOrientation = HRFScanlineOrientation::LOWER_LEFT_VERTICAL;
            break;

        case 3:            // Origin : Lower Right     Line Orentation : Vertical
            ScanlineOrientation = HRFScanlineOrientation::LOWER_RIGHT_VERTICAL;
            break;

        case 4:            // Origin : Upper Left      Line Orentation : Horizontal
            ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
            break;

        case 5:            // Origin : Upper Right     Line Orentation : Horizontal
            ScanlineOrientation = HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL;
            break;

        case 6:            // Origin : Lower Left      Line Orentation : Horizontal
            ScanlineOrientation = HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL;
            break;

        case 7:            // Origin : Lower Right     Line Orentation : Horizontal
            ScanlineOrientation = HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL;
            break;
        default :          // Invalid Scan Line Orientation. In release keep the  default : SLO 4
            ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
            HASSERT(false);
            break;
        }
    return ScanlineOrientation;
    }

//-----------------------------------------------------------------------------
// WriteTransfoModel
//-----------------------------------------------------------------------------
void HRFLRDFile::WriteTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel)
    {
    // Get the simplest model possible.
    HFCPtr<HGF2DTransfoModel> pTransfoModel(pi_rpTransfoModel);

    HFCPtr<HGF2DTransfoModel> pTempTransfoModel = pi_rpTransfoModel->CreateSimplifiedModel();
    if (pTempTransfoModel != 0)
        pTransfoModel = pTempTransfoModel;

    // Set the model using matix if the user had ask so or
    // if the file already have a matrix.
    if (pTransfoModel->CanBeRepresentedByAMatrix())
        {
        HFCMatrix<3, 3> TheMatrix;
        TheMatrix = pTransfoModel->GetMatrix();

        HRFScanlineOrientation  scanlineOrientation = GetScanlineOrientation();

        // SLO 0, SLO 1, SLO 2, SLO 3
        if (scanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL ||
            scanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL ||
            scanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL ||
            scanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
            {
            // a-b-c-d
            m_pLRDHeader->Matrix[0]  = TheMatrix[0][0];
            m_pLRDHeader->Matrix[1]  = TheMatrix[0][1];
            m_pLRDHeader->Matrix[2]  = 0.0;
            m_pLRDHeader->Matrix[3]  = TheMatrix[0][2];

            // e-f-g-h
            m_pLRDHeader->Matrix[4]  = -TheMatrix[1][0];
            m_pLRDHeader->Matrix[5]  = TheMatrix[1][1];
            m_pLRDHeader->Matrix[6]  = 0.0;
            m_pLRDHeader->Matrix[7]  = TheMatrix[1][2];

            // i-j-k-l
            m_pLRDHeader->Matrix[8]  = 0.0;
            m_pLRDHeader->Matrix[9]  = 0.0;
            m_pLRDHeader->Matrix[10] = 1.0;
            m_pLRDHeader->Matrix[11] = 0.0;

            // m-n-o-p
            m_pLRDHeader->Matrix[12] = TheMatrix[2][0];
            m_pLRDHeader->Matrix[13] = TheMatrix[2][1];
            m_pLRDHeader->Matrix[14] = 0.0;
            m_pLRDHeader->Matrix[15] = TheMatrix[2][2];

            }
        else // SLO 4, SLO 5, SLO 6, SLO 7
            {
            // a-b-c-d
            m_pLRDHeader->Matrix[0]  = TheMatrix[0][0];
            m_pLRDHeader->Matrix[1]  = -TheMatrix[0][1];
            m_pLRDHeader->Matrix[2]  = 0.0;
            m_pLRDHeader->Matrix[3]  = TheMatrix[0][2];

            // e-f-g-h
            m_pLRDHeader->Matrix[4]  = TheMatrix[1][0];
            m_pLRDHeader->Matrix[5]  = -TheMatrix[1][1];
            m_pLRDHeader->Matrix[6]  = 0.0;
            m_pLRDHeader->Matrix[7]  = TheMatrix[1][2];

            // i-j-k-l
            m_pLRDHeader->Matrix[8]  = 0.0;
            m_pLRDHeader->Matrix[9]  = 0.0;
            m_pLRDHeader->Matrix[10] = 1.0;
            m_pLRDHeader->Matrix[11] = 0.0;

            // m-n-o-p
            m_pLRDHeader->Matrix[12] = TheMatrix[2][0];
            m_pLRDHeader->Matrix[13] = TheMatrix[2][1];
            m_pLRDHeader->Matrix[14] = 0.0;
            m_pLRDHeader->Matrix[15] = TheMatrix[2][2];
            }
        }
    }


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRFLRDFile::AsciiDate (char adate[])
    {
    time_t t;
    char* s;

    uint64_t tt = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    tt = (uint64_t)(tt / 1000.0);   // Convert in second

    t = (time_t)tt;
    s=ctime(&t);

    adate[0]=s[8];
    adate[1]=s[9];
    adate[2]='-';
    adate[3]=s[4];
    adate[4]=s[5];
    adate[5]=s[6];
    adate[6]='-';
    adate[7]=s[22];
    adate[8]=s[23];
    adate[9]=0;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRFLRDFile::AsciiTime (char atime[])
    {
    time_t t;
    char* s;
    long i;

    uint64_t tt = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    tt = (uint64_t)(tt / 1000.0);   // Convert in second

    t = (time_t)tt;
    t=time(0L);
    s=ctime(&t);

    for (i=0; i<=7; i++)
        atime[i] = s[11+i];
    atime[8]=0;
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFLRDFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_INTERGRAPHWORLD;
    }

//-----------------------------------------------------------------------------
// Public
// GetIntergraphFilePtr
// File information
//-----------------------------------------------------------------------------
const HFCBinStream* HRFLRDFile::GetLRDFilePtr() const
    {
    return m_pLRDFile;
    }

//-----------------------------------------------------------------------------
// Public
// GetIntergraphFilePtr
// File information
//-----------------------------------------------------------------------------
const HFCPtr<HCDCodecLRDRLE>& HRFLRDFile::GetLRDCodecPtr() const
    {
    return m_pCodec;
    }

//-----------------------------------------------------------------------------
// Public
// SetDefaultRatioToMeter
// Set the default ratio to meter specified by the user, if this ratio cannot
// be deduced from the file metadata.
//-----------------------------------------------------------------------------
void HRFLRDFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                               uint32_t pi_Page,
                                               bool   pi_CheckSpecificUnitSpec,
                                               bool   pi_InterpretUnitINTGR)
    {
    //The unit used by an Intergraph file format is always UoR.
    }
