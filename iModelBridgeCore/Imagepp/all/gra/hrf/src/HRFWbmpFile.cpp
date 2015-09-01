//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFWbmpFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFWbmpFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFWbmpFile.h>
#include <Imagepp/all/h/HRFWbmpLineEditor.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>

#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//-----------------------------------------------------------------------------
// HRFBMPBlockCapabilities
//-----------------------------------------------------------------------------
class HRFWBMPBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFWBMPBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,
                                  ULONG_MAX,
                                  HRFBlockAccess::RANDOM));
        }
    };


//-----------------------------------------------------------------------------
// HRFWBMPCodecIdentityCapabilities
//-----------------------------------------------------------------------------
class HRFWBMPCodecIdentityCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFWBMPCodecIdentityCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFWBMPBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFWBMPCapabilities
//-----------------------------------------------------------------------------
HRFWBMPCapabilities::HRFWBMPCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV1Gray1
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV1Gray1;
    pPixelTypeV1Gray1 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                                   new HRFWBMPCodecIdentityCapabilities());
    Add((HFCPtr<HRFCapability>&)pPixelTypeV1Gray1);

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));
    }

HFC_IMPLEMENT_SINGLETON(HRFWbmpCreator)

//-----------------------------------------------------------------------------
// Constructor
// Public (HRFWbmpCreator)
// This is the creator to instantiate bmp format
//-----------------------------------------------------------------------------
HRFWbmpCreator::HRFWbmpCreator()
    : HRFRasterFileCreator(HRFWbmpFile::CLASS_ID)
    {
    // Bmp capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// GetLabel
// Public (HRFWbmpCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFWbmpCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_WirelessBitmap()); // WBMP File Format
    }

//-----------------------------------------------------------------------------
// GetSchemes
// Public (HRFWbmpCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFWbmpCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// GetExtensions
// Public (HRFWbmpCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFWbmpCreator::GetExtensions() const
    {
    return WString(L"*.wbmp");
    }

//-----------------------------------------------------------------------------
// Create
// Public (HRFWbmpCreator)
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFWbmpCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode   pi_AccessMode,
                                             uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFWbmpFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// IsKindOfFile
// Public (HRFWbmpCreator)
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFWbmpCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                   uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool                   bResult = false;
    uint64_t ExpectedSize = 0;
    size_t HeaderLength = 0;
    HAutoPtr<HFCBinStream>  pFile;

    (const_cast<HRFWbmpCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the BMP File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    WbmpFileHeader FileHeader;

    HeaderLength = HRFWbmpFile::GetFileHeaderFromFile(*pFile, FileHeader);

    //Empty file
    if (HeaderLength == 0)
        goto WRAPUP;

    if (FileHeader.m_TypeField != 0)
        goto WRAPUP;

    if (FileHeader.m_FixHeaderField != 0)
        goto WRAPUP;

    //Compute the expected size.
    ExpectedSize = (uint64_t)ceil(FileHeader.m_Width / 8.0) * FileHeader.m_Height + HeaderLength;

    if (pFile->GetSize() != ExpectedSize)
        goto WRAPUP;

    bResult = true;

WRAPUP:
    SisterFileLock.ReleaseKey();

    HASSERT(!(const_cast<HRFWbmpCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFWbmpCreator*>(this))->m_pSharingControl = 0;

    return bResult;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public (HRFWbmpCreator)
// Create or get the singleton capabilities of WBMP file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFWbmpCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFWBMPCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------

HRFWbmpFile::HRFWbmpFile(const HFCPtr<HFCURL>& pi_rURL,
                                HFCAccessMode         pi_AccessMode,
                                uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset),
      m_OffsetToFirstRowInByte(0)
    {
    // The ancestor store the access mode
    m_IsOpen             = false;

    if (GetAccessMode().m_HasCreateAccess)
        {
        Create();
        }
    else
        {
        // Create Page and Res Descriptors.
        Open();
        CreateDescriptors();
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFWbmpFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }


//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFWbmpFile::HRFWbmpFile(const HFCPtr<HFCURL>& pi_rURL,
                                HFCAccessMode         pi_AccessMode,
                                uint64_t             pi_Offset,
                                bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen     = false;
    }


//-----------------------------------------------------------------------------
// Protected
// GetFilePtr
// Get the Bmp file pointer.
//-----------------------------------------------------------------------------
HFCBinStream* HRFWbmpFile::GetFilePtr  ()
    {
    return (m_pWbmpFile);
    }

//-----------------------------------------------------------------------------
// Destructor
// Public
// Destroy Bmp file object
//-----------------------------------------------------------------------------
HRFWbmpFile::~HRFWbmpFile()
    {
    try
        {
        // Close the WBMP file
        SaveWbmpFile(true);
        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }
    }

//-----------------------------------------------------------------------------
// CreateResolutionEditor
// Public
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFWbmpFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                         unsigned short pi_Resolution,
                                                         HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFWbmpLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// AddPage
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFWbmpFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);
    HPRECONDITION(pi_pPage != 0);

    bool IsPageAdded = false;

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(0);

    if (pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV1Gray1::CLASS_ID)
        {
        m_WbmpFileHeader.m_TypeField = 0;
        m_WbmpFileHeader.m_FixHeaderField = 0;
        m_WbmpFileHeader.m_pExtFields = 0;

        HASSERT(pResolutionDescriptor->GetWidth() <= ULONG_MAX);
        HASSERT(pResolutionDescriptor->GetHeight() <= ULONG_MAX);

        m_WbmpFileHeader.m_Width = (uint32_t)pResolutionDescriptor->GetWidth();
        m_WbmpFileHeader.m_Height = (uint32_t)pResolutionDescriptor->GetHeight();

        // Finding padding bits per row.
        uint32_t LinePadBits                  = 8;
        uint32_t UsedBitsPerRow               = pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() *
                                             m_WbmpFileHeader.m_Width;

        m_PaddingBitsPerRow   = (unsigned short)(LinePadBits - (UsedBitsPerRow % LinePadBits));
        if (m_PaddingBitsPerRow == LinePadBits)
            {
            m_PaddingBitsPerRow = 0;
            }

        // Lock the sister file
        HFCLockMonitor SisterFileLock(GetLockManager());

        // Write the file header information.
        m_pWbmpFile->SeekToPos(0);
        SetFileHeaderToFile();

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        IsPageAdded = true;
        }

    return IsPageAdded;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFWbmpFile::GetCapabilities () const
    {
    return (HRFWbmpCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Open
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFWbmpFile::Open()
    {
    // Open the file
    if (!m_IsOpen)
        {
        // Open the actual wbmp file.
        m_pWbmpFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetPhysicalAccessMode(), 0, true);

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Lock the sister file
        HFCLockMonitor SisterFileLock(GetLockManager());

        // Initialisation of file struct.
        m_OffsetToFirstRowInByte = GetFileHeaderFromFile(*m_pWbmpFile, m_WbmpFileHeader);

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        m_IsOpen = true;
        }

    return m_IsOpen;
    }


//-----------------------------------------------------------------------------
// CreateDescriptors
// Protected
// Create Wbmp File Descriptors
//-----------------------------------------------------------------------------
void HRFWbmpFile::CreateDescriptors ()
    {
    // Create Page and Resolution Description/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor>     pResolution;
    HFCPtr<HRFPageDescriptor>           pPage;

    HFCPtr<HRPPixelType>  pPixelType = CreatePixelTypeFromFile();

    // Find Padding Bits Per Row
    uint32_t LinePadBits     = 8;
    uint32_t UsedBitsPerRow  = pPixelType->CountPixelRawDataBits() * m_WbmpFileHeader.m_Width;
    m_PaddingBitsPerRow   = (unsigned short)(LinePadBits - (UsedBitsPerRow % LinePadBits));
    if (m_PaddingBitsPerRow == LinePadBits)
        {
        m_PaddingBitsPerRow = 0;
        }

    HRFBlockAccess  BlockAccess = HRFBlockAccess::RANDOM;
    HRFBlockType    BlockType   = HRFBlockType::LINE;
    uint32_t        BlockHeight = 1;

    // Create Resolution Descriptor
    pResolution =  new HRFResolutionDescriptor(GetAccessMode(),               // AccessMode,
                                               GetCapabilities(),             // Capabilities,
                                               1.0,                           // XResolutionRatio,
                                               1.0,                           // YResolutionRatio,
                                               pPixelType,                    // PixelType,
                                               new HCDCodecIdentity(),        // Codec,
                                               BlockAccess,                   // RBlockAccess,
                                               BlockAccess,                   // WBlockAccess,
                                               HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
                                               HRFInterleaveType::PIXEL,      // InterleaveType,
                                               0,                             // IsInterlace,
                                               m_WbmpFileHeader.m_Width,      // Width,
                                               m_WbmpFileHeader.m_Height,     // Height,
                                               m_WbmpFileHeader.m_Width,      // BlockWidth,
                                               BlockHeight,                   // BlockHeight,
                                               0,                             // BlocksDataFlag,
                                               BlockType);                    // BlockType

    pPage = new HRFPageDescriptor (GetAccessMode(),         // AccessMode
                                   GetCapabilities(),       // Capabilities,
                                   pResolution,             // ResolutionDescriptor,
                                   0,                       // RepresentativePalette,
                                   0,                       // Histogram,
                                   0,                       // Thumbnail,
                                   0,                       // ClipShape,
                                   0,                       // TransfoModel,
                                   0,                       // Filters
                                   0);               // Defined Tag

    m_ListOfPageDescriptor.push_back(pPage);
    }


//-----------------------------------------------------------------------------
// SaveWbmpFile
// Private
// This method saves the file and close it if needed
//-----------------------------------------------------------------------------
void HRFWbmpFile::SaveWbmpFile(bool pi_CloseFile)
    {
    HPRECONDITION(GetPageDescriptor(0)->CountResolutions() == 1);
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.

    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        if ( (GetAccessMode().m_HasWriteAccess) || (GetAccessMode().m_HasCreateAccess) )
            {
            bool SaveHeader = false;

            HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

            // Lock the sister file
            HFCLockMonitor SisterFileLock(GetLockManager());

            // Write the file header information.
            m_pWbmpFile->SeekToPos(0);

            if (m_WbmpFileHeader.m_Width != pPageDescriptor->GetResolutionDescriptor(0)->GetWidth())
                {
                HASSERT(pPageDescriptor->GetResolutionDescriptor(0)->GetWidth() <= ULONG_MAX);

                m_WbmpFileHeader.m_Width = (uint32_t)pPageDescriptor->GetResolutionDescriptor(0)->GetWidth();
                SaveHeader = true;
                }

            if (m_WbmpFileHeader.m_Height != pPageDescriptor->GetResolutionDescriptor(0)->GetHeight())
                {
                HASSERT(pPageDescriptor->GetResolutionDescriptor(0)->GetHeight() <= ULONG_MAX);

                m_WbmpFileHeader.m_Height = (uint32_t)pPageDescriptor->GetResolutionDescriptor(0)->GetHeight();
                SaveHeader = true;
                }

            if (SaveHeader == true)
                {
                SetFileHeaderToFile();
                }

            pPageDescriptor->Saved();
            pPageDescriptor->GetResolutionDescriptor(0)->Saved();

            // Unlock the sister file.
            SisterFileLock.ReleaseKey();
            }

        if (pi_CloseFile)
            {
            m_IsOpen = false;
            m_pWbmpFile = 0;
            }
        else
            {
            m_pWbmpFile->Flush();
            }
        }
    }

//-----------------------------------------------------------------------------
// Save
// Private
// This method saves the file.
//-----------------------------------------------------------------------------
void HRFWbmpFile::Save()
    {
    //Keep last file position
    uint64_t CurrentPos = m_pWbmpFile->GetCurrentPos();

    SaveWbmpFile(false);

    //Set back position
    m_pWbmpFile->SeekToPos(CurrentPos);
    }

//-----------------------------------------------------------------------------
// Create
// Private
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFWbmpFile::Create()
    {
    // Open the file.
    m_pWbmpFile = HFCBinStream::Instanciate(GetURL(), GetAccessMode(), 0, true);

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    m_IsOpen = true;

    return true;
    }

//-----------------------------------------------------------------------------
// CreatePixelTypeFromFile
// Private
// Find and create pixel type from file
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRFWbmpFile::CreatePixelTypeFromFile() const
    {
    HFCPtr<HRPPixelType> pPixelType;

    if (m_WbmpFileHeader.m_TypeField == 0)
        {
        pPixelType = new HRPPixelTypeV1Gray1();
        }

    return (pPixelType);
    }


//-----------------------------------------------------------------------------
// GetFileHeaderFromFile
// Private
// Read file header from file
//-----------------------------------------------------------------------------
size_t HRFWbmpFile::GetFileHeaderFromFile(HFCBinStream& pi_rFile,
                                          WbmpFileHeader&         po_rHeader)
    {
    size_t NbBytes = 0;

    NbBytes += ReadNextMultiByteInteger(pi_rFile, po_rHeader.m_TypeField);

    NbBytes += pi_rFile.Read(&po_rHeader.m_FixHeaderField, sizeof po_rHeader.m_FixHeaderField);

    po_rHeader.m_pExtFields = 0; //Currently not used by any type.

    NbBytes += ReadNextMultiByteInteger(pi_rFile, po_rHeader.m_Width);
    NbBytes += ReadNextMultiByteInteger(pi_rFile, po_rHeader.m_Height);

    return NbBytes;
    }

//-----------------------------------------------------------------------------
// ReadNextMultiByteInteger
// Private
// Read the next multi byte integer from the file's header
//-----------------------------------------------------------------------------
size_t HRFWbmpFile::ReadNextMultiByteInteger(HFCBinStream& pi_rFile, uint32_t& po_rValRead)
    {
    Byte Byte;
    size_t TotalNbByteRead;
    size_t NbByteRead;

    TotalNbByteRead = 0;
    po_rValRead     = 0;

    do
        {
        NbByteRead = pi_rFile.Read(&Byte, sizeof(Byte));

        if (NbByteRead > 0)
            {
            TotalNbByteRead += NbByteRead;
            HASSERT(Byte != 0x80);
            po_rValRead <<= 7;
            po_rValRead |= (0x7F & Byte);
            }
        }
    while ((Byte & 0x80) && (NbByteRead > 0) && (TotalNbByteRead < 5));

    return TotalNbByteRead;
    }

//-----------------------------------------------------------------------------
// WriteNextMultiByteInteger
// Private
// Write the next multi byte integer to the file's header
//-----------------------------------------------------------------------------
size_t HRFWbmpFile::WriteNextMultiByteInteger(HFCBinStream& pi_rFile, uint32_t pi_valToWrite)
    {
    static const uint32_t s_SevenLSBmask = 0x00007F;
    Byte                aByte = 0;
    size_t              NbByteWritten = 0;
    Byte                ByteBuffer[5]; //There is a maximum of 5 byte needed to encode an int32.
    short NbBytes = 0;

    memset(ByteBuffer, 0, sizeof(Byte) * 5);

    if (pi_valToWrite == 0)
        {
        NbByteWritten += pi_rFile.Write(&aByte, sizeof(Byte));
        }
    else
        {
        do
            {
            aByte = (Byte)(pi_valToWrite & s_SevenLSBmask);
            pi_valToWrite >>= 7;
            //Set the msb to 1 to indicate it is not the last byte
            //for the integer currently read.
            if (NbBytes > 0)
                {
                aByte |= 0x80;
                }
            ByteBuffer[NbBytes] = aByte;
            NbBytes++;
            }
        while (pi_valToWrite != 0);


        for (int ByteInd = NbBytes - 1; ByteInd >= 0; ByteInd--)
            {
            NbByteWritten += pi_rFile.Write(&ByteBuffer[ByteInd], sizeof(Byte));
            }
        }

    return NbByteWritten;
    }


//-----------------------------------------------------------------------------
// SetFileHeader
// Private
// Write file header to file
//-----------------------------------------------------------------------------
void HRFWbmpFile::SetFileHeaderToFile()
    {
    HPRECONDITION(m_pWbmpFile != 0);

    // Lock the sister file for the SetFileHeadertoFile method
    HFCLockMonitor SisterFileLock(GetLockManager());

    m_OffsetToFirstRowInByte = 0;

    m_OffsetToFirstRowInByte += WriteNextMultiByteInteger(*m_pWbmpFile, m_WbmpFileHeader.m_TypeField);
    m_OffsetToFirstRowInByte += m_pWbmpFile->Write(&m_WbmpFileHeader.m_FixHeaderField, sizeof(m_WbmpFileHeader.m_FixHeaderField));
    HASSERT(m_WbmpFileHeader.m_pExtFields == 0);
    m_OffsetToFirstRowInByte += WriteNextMultiByteInteger(*m_pWbmpFile, m_WbmpFileHeader.m_Width);
    m_OffsetToFirstRowInByte += WriteNextMultiByteInteger(*m_pWbmpFile, m_WbmpFileHeader.m_Height);

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();
    }
