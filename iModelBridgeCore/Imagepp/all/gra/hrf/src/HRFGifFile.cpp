//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGifFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFGifFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCMath.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLFile.h>

#include <Imagepp/all/h/HRFGifFile.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFGifLineEditor.h>
#include <Imagepp/all/h/HRFGifImageEditor.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HCDCodecHMRGIF.h>
#include <Imagepp/all/h/HCDPacket.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//-----------------------------------------------------------------------------
// HRFGifBlockCapabilities
//-----------------------------------------------------------------------------
class HRFGifBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFGifBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,        // AccessMode
                                  LONG_MAX,                     // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL)); // BlockAccess
        // Image Capability
        // ReadOnly, because some problem with the codec...
        Add(new HRFImageCapability(HFC_READ_ONLY,   // AccessMode
                                   LONG_MAX,        // MaxSizeInBytes
                                   1,               // MinWidth
                                   32000,           // MaxWidth
                                   1,               // MinHeight
                                   32000));         // MaxHeight
        }
    };

//-----------------------------------------------------------------------------
// HRFGifCodecCapabilities
//-----------------------------------------------------------------------------
class HRFGifCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFGifCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec HMRGIF (LZW)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRGIF::CLASS_ID,
                                   new HRFGifBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFGifCapabilities
//-----------------------------------------------------------------------------
HRFGifCapabilities::HRFGifCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeI8R8G8B8
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFGifCodecCapabilities()));
    // PixelTypeI8R8G8B8A8
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8R8G8B8A8::CLASS_ID,
                                   new HRFGifCodecCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Interlace capability
    Add(new HRFInterlaceCapability(HFC_READ_ONLY));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Multi Page capability
    Add(new HRFMultiPageCapability(HFC_READ_ONLY));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // Media type capability
    Add(new HRFAnimationCapability(HFC_READ_ONLY));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeGIFApplicationCode));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeNotes));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeBackground(0)));
    }

HFC_IMPLEMENT_SINGLETON(HRFGifCreator)


//-----------------------------------------------------------------------------
// Constructor
// Public (HRFGifCreator)
// This is the creator to instantiate Gif format
//-----------------------------------------------------------------------------
HRFGifCreator::HRFGifCreator()
    : HRFRasterFileCreator(HRFGifFile::CLASS_ID)
    {
    // Gif capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// GetLabel
// Public (HRFGifCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFGifCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_GIF()); // Gif File Format
    }

//-----------------------------------------------------------------------------
// GetSchemes
// Public (HRFGifCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFGifCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// GetExtensions
// Public (HRFGifCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFGifCreator::GetExtensions() const
    {
    return WString(L"*.gif");
    }

//-----------------------------------------------------------------------------
// Create
// Public (HRFGifCreator)
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFGifCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode   pi_AccessMode,
                                            uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFGifFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// IsKindOfFile
// Public (HRFGifCreator)
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFGifCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool                   Status = false;
    HAutoPtr<HFCBinStream>  pFile;

    GifHeader               gifHeader;
    GifImageDescriptor      gifImageDesc;
    GifGraphicControl       gifGraphicControl;
    GifPlainText            gifPlainText;
    GifApplication          gifApplication;
    GifComment              gifComment;

    (const_cast<HRFGifCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Boolean that indicate if the end of the file has been reach.
    bool   EndOfGifFile = false;

    // Open the Gif File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    // Count of the number of table lines displayed.
    unsigned short ImageCount;
    // Extension block identifier holder.
    Byte  Identifier;
    // Size of data sub-block holder.
    Byte  DataSize;

    if (HRFGifFile::ReadGifHeader(&gifHeader, pFile, 0))
        {
        // Verify if the signature and the version number is valide (GIF87a or GIF89a)
        if ((gifHeader.Signature[0] != 0x47 || gifHeader.Signature[1] != 0x49 || gifHeader.Signature[2] != 0x46) &&
            (gifHeader.Version[0] != 0x38 || (gifHeader.Version[1] != 0x37 && gifHeader.Version[1] != 0x39) ||
             gifHeader.Version[2] != 0x61))
            goto WRAPUP;

        // Clear the image counter.
        ImageCount = 0;
        while(!EndOfGifFile)
            {
            if (pFile->Read(&Identifier, 1) != 1)
                goto WRAPUP;

            switch (Identifier)
                {
                    // Trailer:
                case 0x3B:
                    EndOfGifFile = true;
                    break;

                    // Image Descriptor:
                case 0x2C:
                    if (!HRFGifFile::ReadGifImageDesc(&gifImageDesc, pFile, 0))
                        goto WRAPUP;

                    // Skip past the encoded image data.
                    Byte LZWMinimumCodeSize;
                    pFile->Read(&LZWMinimumCodeSize, sizeof(Byte));
                    while ((pFile->Read(&DataSize, 1) == 1) && (DataSize != 0))
                        pFile->SeekToPos(pFile->GetCurrentPos() + (long) DataSize);

                    break;

                    // Extension Block.
                case 0x21:
                    Byte Code;
                    if (pFile->Read(&Code, 1) != 1)
                        goto WRAPUP;

                    switch (Code)
                        {
                            // Plain Text Extension
                        case 0x01:
                            gifPlainText.PlainTextData = 0;
                            if (!HRFGifFile::ReadGifPlainText(&gifPlainText, pFile, 0))
                                goto WRAPUP;

                            delete [] gifPlainText.PlainTextData;

                            // Plain Text Extension is not yet supported.
                            goto WRAPUP;
                            break;

                            // Comment Extension
                        case 0xFE:
                            gifComment.CommentData = 0;
                            if (!HRFGifFile::ReadGifComment(&gifComment, pFile, 0))
                                goto WRAPUP;

                            // Allocated with malloc !!!
                            delete [] gifComment.CommentData;
                            break;

                            // Graphic Control Extension
                        case 0xF9:
                            if (!HRFGifFile::ReadGifGraphicControl(&gifGraphicControl, pFile, 0))
                                goto WRAPUP;
                            break;

                            // Application Extension.
                        case 0xFF:
                            gifApplication.ApplicationData = 0;
                            if (!HRFGifFile::ReadGifApplication(&gifApplication, pFile, 0))
                                goto WRAPUP;
                            delete [] gifApplication.ApplicationData;
                            break;
                        default:
                            goto WRAPUP;
                            break;
                        }
                    break;
                default:
                    goto WRAPUP;
                }
            }
        Status = true;
        }


    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFGifCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFGifCreator*>(this))->m_pSharingControl = 0;


WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public (HRFGifCreator)
// Create or get the singleton capabilities of PNG file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFGifCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFGifCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Destructor
// Public
// Destroy Gif file object
//-----------------------------------------------------------------------------
HRFGifFile::~HRFGifFile()
    {
    try
        {
        // Close the Gif file and initialize the Compre.exession Structure
        SaveGifFile(true);
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
// GetFileCurrentSize
// Return the current size of the file
//-----------------------------------------------------------------------------
uint64_t HRFGifFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pGifFile);
    }

//-----------------------------------------------------------------------------
// CreateResolutionEditor
// Public
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFGifFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    // If the image is interlace we need to used image editor.
    if (!GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->IsInterlace())
        pEditor = new HRFGifLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    else
        pEditor = new HRFGifImageEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// Save
// Public
// Saves the file
//-----------------------------------------------------------------------------
void HRFGifFile::Save()
    {
    //Keep last file position
    uint64_t CurrentPos = m_pGifFile->GetCurrentPos();

    SaveGifFile(false);

    //Set back position
    m_pGifFile->SeekToPos(CurrentPos);
    }

//-----------------------------------------------------------------------------
// AddPage
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFGifFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(pi_pPage != 0);

    // Animated GIF supported only in read-only
    if ((CountPages() > 0) &&
        (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess))
        {
        throw HRFAnimatedGifReadOnlyException(GetURL()->GetURL());
        }

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    // Compute Histo if we are in Creation + Transparency to be able to identify the background color.
    if(GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType()->GetClassID() == HRPPixelTypeI8R8G8B8A8::CLASS_ID)
        {
        m_pHistoCreationMode = new uint32_t[256];
        memset(m_pHistoCreationMode, 0, 256*sizeof(uint32_t));
        }

    return AssignStructTo(GetPageDescriptor(0));
    }

//-----------------------------------------------------------------------------
// AssignStructTo
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFGifFile::AssignStructTo(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    bool Result = false;

    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pi_pPage->GetResolutionDescriptor(0);

    // Gif Graphic Block to be add in the list.
    GifGraphicBlock     gifGraphicBlock;

    // Lock the sister file
    HFCLockMonitor SisterFileLock(GetLockManager());

    unsigned short BitsByPixel((unsigned short)pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits());

    // First Page
    if (CountPages() == 1)
        {
        m_pGlobalPalette = (HRPPixelPalette*) &pResolutionDescriptor->GetPixelType()->GetPalette();

        SetHeader((unsigned short)pResolutionDescriptor->GetWidth(),
                  (unsigned short)pResolutionDescriptor->GetHeight(),
                  BitsByPixel,
                  0,
                  BitsByPixel,
                  0,                                    // TO DO (AspectRatio)
                  m_pGlobalPalette);

        if (!WriteGifHeader(&m_GifHeader, m_pGifFile))
            goto WRAPUP;
        }

    SetImageDesc(0,                                     // LeftEdge
                 0,                                     // TopEdge
                 (unsigned short)pResolutionDescriptor->GetWidth(),
                 (unsigned short)pResolutionDescriptor->GetHeight(),
                 BitsByPixel,
                 pResolutionDescriptor->IsInterlace(),
                 BitsByPixel,
                 (HRPPixelPalette*) &pResolutionDescriptor->GetPixelType()->GetPalette(),
                 (GifImageDescriptor*) &gifGraphicBlock.ImageDescriptor);

    if(pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeI8R8G8B8A8::CLASS_ID)
        // || IsAnimated() (TO DO)
        {
        SetGraphicControl(10,                   // DelayTime (TO DO)
                          (Byte)m_AlphaColorIndex,
                          2,                    // DisposalMethode (TO DO)
                          0,                    // UserInput (TO DO)
                          (GifGraphicControl*) &gifGraphicBlock.GraphicControl);

        if (!WriteGifGraphicControl((GifGraphicControl*) &gifGraphicBlock.GraphicControl, m_pGifFile))
            goto WRAPUP;
        }

    if (!WriteGifImageDesc((GifImageDescriptor*) &gifGraphicBlock.ImageDescriptor, m_pGifFile))
        goto WRAPUP;


    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    m_ListGifGraphicBlock.push_back((const GifGraphicBlock&) gifGraphicBlock);

    // Adding the offset to the list
    m_ListPageDataOffset.push_back((int32_t)m_pGifFile->GetCurrentPos());
    m_ListPageDecompressMinCodeSize.push_back(BitsByPixel);

    m_ImageCount++;

    Result = true;

WRAPUP:
    return Result;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFGifFile::GetCapabilities () const
    {
    return (HRFGifCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Open
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFGifFile::Open()
    {
    // Open the file
    if (!m_IsOpen)
        {
        // Open the actual png file specified in the parameters.  The library
        // uses stdio HFCBinStream*, so open the file and satisfy the library
        m_pGifFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Lock the sister file
        HFCLockMonitor SisterFileLock(GetLockManager());

        // Initialisation of file struct.
        LookUpBlocks();

        // Unlock the sister file
        SisterFileLock.ReleaseKey();

        // Animated GIF supported only in read-only
        if ((m_ImageCount > 1) &&
            (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess))
            {
            throw HRFAnimatedGifReadOnlyException(GetURL()->GetURL());
            }

        m_IsOpen = true;
        }

    return true;
    }


//-----------------------------------------------------------------------------
// CreateDescriptors
// Protected
// Create Gif File Descriptors
//-----------------------------------------------------------------------------
void HRFGifFile::CreateDescriptors ()
    {
    for(uint32_t nbPages=0; nbPages < m_ImageCount; nbPages++)
        {
        // Obtain the width and height of the resolution.
        uint32_t Width  = m_ListGifGraphicBlock[nbPages].ImageDescriptor.ImageWidth;
        uint32_t Height = m_ListGifGraphicBlock[nbPages].ImageDescriptor.ImageHeight;

        // Create Page and Resolution Description/Capabilities for this file.
        HFCPtr<HRFResolutionDescriptor>     pResolution;
        HFCPtr<HRFPageDescriptor>           pPage;
        HRFBlockType                        BlockType;
        uint32_t                            BlockHeight;

        // Find blocks Data Flag
        if (m_ListGifGraphicBlock[nbPages].ImageDescriptor.PackedField & 0x40)
            {
            // Image is interlace.
            BlockType   = HRFBlockType::IMAGE;
            BlockHeight = Height;
            }
        else
            {
            // Image is interlace.
            BlockType   = HRFBlockType::LINE;
            BlockHeight = 1;
            }

        HFCPtr<HRPPixelType>  pPixelType = CreatePixelTypeFromFile(nbPages);

        // Throw that we have found a pixel type capability, other wise it
        // means that this pixel type is not supported by the pi_rpResolutionCapabilities.
        // Get the PixelType capability associete with pi_rpPixelType
        HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
        pPixelTypeCapability = new HRFPixelTypeCapability(GetAccessMode(),
                                                          pPixelType->GetClassID(),
                                                          new HRFRasterFileCapabilities());
        pPixelTypeCapability = static_cast<HRFPixelTypeCapability*>(GetCapabilities()->
                               GetCapabilityOfType(((HFCPtr<HRFCapability>)pPixelTypeCapability)).GetPtr());
        if (pPixelTypeCapability == 0)
            throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());


        // Create Resolution Descriptor
        pResolution =  new HRFResolutionDescriptor(
            GetAccessMode(),                                    // AccessMode,
            GetCapabilities(),                                  // Capabilities,
            1.0,                                                // XResolutionRatio,
            1.0,                                                // YResolutionRatio,
            pPixelType,                                         // PixelType,
            new HCDCodecHMRGIF(),                               // Codec,
            HRFBlockAccess::SEQUENTIAL,                         // RStorageAccess,
            HRFBlockAccess::SEQUENTIAL,                         // WStorageAccess,
            HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,      // ScanLineOrientation,
            HRFInterleaveType::PIXEL,                           // InterleaveType
            (m_ListGifGraphicBlock[nbPages].ImageDescriptor.PackedField & 0x40) != 0, // IsInterlace,
            Width,                                              // Width,
            Height,                                             // Height,
            Width,                                              // BlockWidth,
            BlockHeight,                                        // BlockHeight,
            0,                                                  // BlocksDataFlag
            BlockType);

        pPage = new HRFPageDescriptor (GetAccessMode(),
                                       GetCapabilities(),   // Capabilities,
                                       pResolution,         // ResolutionDescriptor,
                                       0,                   // RepresentativePalette,
                                       0,                   // Histogram,
                                       0,                   // Thumbnail,
                                       0,                   // ClipShape,
                                       0,                   // TransfoModel,
                                       0,                   // Filters
                                       &m_GeneralTagList);  // Defined Tag

        m_ListOfPageDescriptor.push_back(pPage);

        }
    }


//-----------------------------------------------------------------------------
// SaveGifFile
// Private
// This method save the file.
//-----------------------------------------------------------------------------
void HRFGifFile::SaveGifFile(bool pi_CloseFile)
    {
    HPRECONDITION(m_IsOpen && m_ListOfPageDescriptor.size() > 0);

    string Software         = "";
    string ApplicationCode  = "";
    string CommentData      = "";
    Byte Background       = 0;

    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

    // Display each tag.
    HPMAttributeSet::HPMASiterator TagIterator;

    if ((GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess) /*&& m_RasterIsDirty*/)
        {
        // Lock the sister file
        HFCLockMonitor SisterFileLock(GetLockManager());

        for (TagIterator  = pPageDescriptor->GetTags().begin();
             TagIterator != pPageDescriptor->GetTags().end(); TagIterator++)
            {
            HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

            if (pPageDescriptor->TagHasChanged(*pTag)) 
                {
                // Software tag
                if (pTag->GetID() == HRFAttributeSoftware::ATTRIBUTE_ID)
                    {
                    AString tempStrA;
                    BeStringUtilities::WCharToCurrentLocaleChar(tempStrA, ((HFCPtr<HRFAttributeSoftware>&)pTag)->GetData().c_str());
                    Software = tempStrA.c_str();
                    }
                // Application code tag
                if (pTag->GetID() == HRFAttributeGIFApplicationCode::ATTRIBUTE_ID)
                    ApplicationCode = ((HFCPtr<HRFAttributeGIFApplicationCode>&)pTag)->GetData();
                // Notes Tag
                if (pTag->GetID() == HRFAttributeNotes::ATTRIBUTE_ID)
                    {
                    AString tempStrA;
                    BeStringUtilities::WCharToCurrentLocaleChar(tempStrA, ((HFCPtr<HRFAttributeNotes>&)pTag)->GetData().c_str());
                    CommentData = tempStrA.c_str();
                    }
                // Background Tag
                if (pTag->GetID() == HRFAttributeBackground::ATTRIBUTE_ID)
                    Background = (Byte)((HFCPtr<HRFAttributeBackground>&)pTag)->GetData();
                }

            }

        // Write gif application to the file.
        if (Software != "")
            {
            GifApplication gifApp;

            gifApp.Introducer      = 0x21;
            gifApp.Label           = 0xff;
            gifApp.BlockSize       = 11;

            unsigned short IdenLength = (unsigned short)MIN(Software.size(), 8);

            for (unsigned short i = 0; i < IdenLength; i++)
                {
                gifApp.Identifier[i]  = Software.at(i);
                }

            if (IdenLength < 8)
                {
                memset(&(gifApp.Identifier[IdenLength]), 0, 8 - IdenLength);
                }

            if (ApplicationCode != "")
                {
                for (unsigned short i = 0; i<3; i++)
                    gifApp.AuthentCode[i]  = ApplicationCode.at(i);
                }
            else
                {
                for (unsigned short i = 0; i<3; i++)
                    gifApp.AuthentCode[i]  = 0;
                }
            gifApp.ApplicationData = NULL;
            gifApp.Terminator      = 0;

            if (!WriteGifApplication(&gifApp, 0, m_pGifFile))
                goto WRAPUP;
            }
        // Write gif comment to the file.
        if (CommentData != "")
            {
            GifComment gifComment;

            gifComment.Introducer      = 0x21;
            gifComment.Label           = 0xfe;
            gifComment.CommentData     = (Byte*) CommentData.c_str();
            gifComment.Terminator      = 0x00;

            if (!WriteGifComment(&gifComment, (uint32_t)CommentData.length(), m_pGifFile))
                goto WRAPUP;
            }

        if(m_RasterIsDirty)
            {
            Byte Code = 0x3b;
            if (m_pGifFile->Write(&Code, 1) != 1)
                goto WRAPUP;
            }

        // Write background color in file header.
        if (Background != 0)
            {
            m_pGifFile->SeekToPos(0x0b);
            if (m_pGifFile->Write(&Background, sizeof(Background)) != sizeof(Background))
                goto WRAPUP;
            }


        for(uint32_t nbPages=0; nbPages < m_ImageCount; nbPages++)
            {
            pPageDescriptor       = GetPageDescriptor(nbPages);
            HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(0);

            // Write the local color table.
            if(pPageDescriptor->GetResolutionDescriptor(0)->PaletteHasChanged())
                {
                m_pGlobalPalette = (HRPPixelPalette*) &pResolutionDescriptor->GetPixelType()->GetPalette();

                SetPalette((unsigned short)pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits(),
                           (unsigned short)pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits(),
                           (HRPPixelPalette*) &pResolutionDescriptor->GetPixelType()->GetPalette(),
                           m_ListGifGraphicBlock[nbPages].ImageDescriptor.LocalCT);

                // Get number of color table entries.
                unsigned short tableSize = (unsigned short) (1L << ((m_GifHeader.PackedField & 0x07) + 1));

                // Move the file pointer to the begining of the local color table.
                m_pGifFile->SeekToPos(m_ListPageDataOffset[nbPages] - 3*tableSize);

                // Write the local Color Table.
                if (!WritePalette(m_ListGifGraphicBlock[nbPages].ImageDescriptor.LocalCT, tableSize, m_pGifFile))
                    goto WRAPUP;

                // If we have a pixel type with alpha value we need to set the graphic control block.
                if(pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeI8R8G8B8A8::CLASS_ID)
                    {
                    // Offset to the beginnig of the last graphic control block.
                    uint32_t offset = m_ListPageDataOffset[nbPages] - 10 - 8 - 3*tableSize;

                    // Move the file pointer to the begining of the last graphic control block.
                    m_pGifFile->SeekToPos(offset);

                    SetGraphicControl(10,                   // DelayTime (TO DO)
                                      (Byte)m_AlphaColorIndex,
                                      2,                    // DisposalMethode (TO DO)
                                      0,                    // UserInput    (TO DO)
                                      (GifGraphicControl*) &m_ListGifGraphicBlock[nbPages].GraphicControl);

                    if (!WriteGifGraphicControl((GifGraphicControl*) &m_ListGifGraphicBlock[nbPages].GraphicControl, m_pGifFile))
                        goto WRAPUP;
                    }
                }
            }

        m_pGifFile->Flush();

        // Unlock the sister file
        SisterFileLock.ReleaseKey();
        }

    if (pi_CloseFile)
        {
        m_pGifFile = 0;
        m_IsOpen = false;
        }

WRAPUP:

    m_IsOpen = m_IsOpen;
    }

//-----------------------------------------------------------------------------
// Create
// Private
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFGifFile::Create()
    {
    // Open the file.
    m_pGifFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

    // Create the sister file for file sharing control if necessary.
    SharingControlCreate();

    m_IsOpen = true;

    m_ImageCount = 0;
    m_AlphaColorIndex = -1;

    m_HasComment     = false;
    m_HasApplication = false;
    m_HasPlainText   = false;

    m_pGlobalPalette =  NULL;

    m_ValidGraphicControl = false;

    m_ListPageDataOffset.erase (m_ListPageDataOffset.begin(),  m_ListPageDataOffset.end());
    m_ListGifGraphicBlock.erase(m_ListGifGraphicBlock.begin(), m_ListGifGraphicBlock.end());
    m_ListPageDecompressMinCodeSize.clear();

    return true;
    }

//-----------------------------------------------------------------------------
// CreatePixelTypeFromFile
// Private
// Find and create pixel type from file
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRFGifFile::CreatePixelTypeFromFile(uint32_t pi_PageIndex) const
    {
    HFCPtr<HRPPixelType>    pPixelType;
    uint32_t                 PaletteSize(0);
    const GifGraphicBlock* pGifGraphicBlock = &(m_ListGifGraphicBlock[pi_PageIndex]);
    const GifColorTable*    pGifColorTable = NULL;
    Byte                  TransparentIndex(0);
    bool                    TransparentIndexIsValid(false);

    // Determine which color table we should use.
    if(pGifGraphicBlock->ImageDescriptor.PackedField & 0x80)
        {
        // Use local table
        pGifColorTable = &pGifGraphicBlock->ImageDescriptor.LocalCT;
        PaletteSize = (uint32_t)pow(2.0, (pGifGraphicBlock->ImageDescriptor.PackedField & 0x07)+1);
        }
    else if(m_GifHeader.PackedField & 0x80)
        {
        // Use global color table
        pGifColorTable = &m_GifHeader.GlobalCT;
        PaletteSize = (uint32_t)pow(2.0, (m_GifHeader.PackedField & 0x07) + 1);
        }

    // Eval transparency index
    if(pGifGraphicBlock->GraphicControl.PackedField & 0x01)
        {
        TransparentIndexIsValid = true;
        TransparentIndex = pGifGraphicBlock->GraphicControl.ColorIndex;
        }

    // If we have a color table
    if(PaletteSize != 0)
        {
        uint32_t Index;

        if(TransparentIndexIsValid)
            {
            Byte ValueAlpha[4];

            // Create a palette with alpha
            pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB(8,8,8,8, HRPChannelType::UNUSED, HRPChannelType::VOID_CH, 0),8);
            //(pGifGraphicBlock->ImageDescriptor.PackedField & 0x07)+1);
            // Get the palette from the pixel type.
            HRPPixelPalette* pPalette = (HRPPixelPalette*)&(pPixelType->GetPalette());

            ValueAlpha[3] = 255; // Default is opaque
            // Copy the PNG palette into the pixel palette.
            for (Index = 0; Index < PaletteSize ; ++Index)
                {
                ValueAlpha[0] = (*pGifColorTable)[Index].Red;
                ValueAlpha[1] = (*pGifColorTable)[Index].Green;
                ValueAlpha[2] = (*pGifColorTable)[Index].Blue;

                // Add the entry to the pixel palette.
                pPalette->SetCompositeValue(Index, ValueAlpha);
                }
            for(; Index < 256; ++Index)
                {
                ValueAlpha[0] = 0;
                ValueAlpha[1] = 0;
                ValueAlpha[2] = 0;
                ValueAlpha[3] = 255;

                // Add the entry to the pixel palette.
                pPalette->SetCompositeValue(Index, ValueAlpha);
                }

            // Set transparent index
            ValueAlpha[0] = (*pGifColorTable)[TransparentIndex].Red;
            ValueAlpha[1] = (*pGifColorTable)[TransparentIndex].Green;
            ValueAlpha[2] = (*pGifColorTable)[TransparentIndex].Blue;
            ValueAlpha[3] = 0;

            pPalette->SetCompositeValue(TransparentIndex, ValueAlpha);
            }
        else
            {
            Byte Value[3];

            pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB(8,8,8,0, HRPChannelType::UNUSED,HRPChannelType::VOID_CH,0),8);
            //(pGifGraphicBlock->ImageDescriptor.PackedField & 0x07)+1);
            // Get the palette from the pixel type.
            HRPPixelPalette* pPalette = (HRPPixelPalette*)&(pPixelType->GetPalette());

            // Copy the PNG palette into the pixel palette.
            for (Index = 0; Index < PaletteSize ; ++Index)
                {
                Value[0] = (*pGifColorTable)[Index].Red;
                Value[1] = (*pGifColorTable)[Index].Green;
                Value[2] = (*pGifColorTable)[Index].Blue;

                // Add the entry to the pixel palette.
                pPalette->SetCompositeValue(Index, Value);
                }
            for(; Index < 256; ++Index)
                {
                Value[0] = 0;
                Value[1] = 0;
                Value[2] = 0;

                // Add the entry to the pixel palette.
                pPalette->SetCompositeValue(Index, Value);
                }
            }
        }
    else
        {
        // No local table and no global table use the default palette color with 8 bit par pixel.
        pPixelType = new HRPPixelTypeI8R8G8B8();
        }

    return pPixelType;
    }

//-----------------------------------------------------------------------------
// LookUpBlocks
// Private
// Find all used full blocks
//-----------------------------------------------------------------------------
bool HRFGifFile::LookUpBlocks()
    {
    HPRECONDITION (SharingControlIsLocked());

    bool Status       = false;
    bool EndOfGifFile = false; // Boolean that indicate if the end of the file has been reach.
    unsigned short LineCount;          // Count of the number of table lines displayed.
    unsigned short BlockCount;         // Running count of the number of data blocks.
    Byte  Identifier;         // Extension block identifier holder.
    Byte  DataSize;           // Size of data sub-block holder.
    unsigned short GlobalTableSize;    // Number of entires in the global color table.

    m_ValidGraphicControl = false;

    // Read the GIF image file header information.
    if (!ReadGifHeader(&m_GifHeader, m_pGifFile, this))
        goto WRAPUP;

    if ((m_GifHeader.PackedField & 0x80) && m_GifHeader.ColorIndex != 0)
        {
        // Adding tag in the general tag list.
        HFCPtr<HPMGenericAttribute> pTag;
        // Background Tag
        pTag = new HRFAttributeBackground((uint32_t) m_GifHeader.ColorIndex);
        m_GeneralTagList.Set(pTag);
        }

    // Clear the image counter.
    m_ImageCount = 0;

    // Gif Graphic Block to be add in the list.
    GifGraphicBlock     gifGraphicBlock;

    while(!EndOfGifFile)
        {
        if (m_pGifFile->Read(&Identifier, 1) != 1)
            goto WRAPUP;

        switch (Identifier)
            {
                // Trailer:
            case 0x3B:
                EndOfGifFile = true;
                break;

                // Image Descriptor:
            case 0x2C:
                gifGraphicBlock.ImageDescriptor.ImageSeparator = Identifier;

                if (!ReadGifImageDesc(&gifGraphicBlock.ImageDescriptor, m_pGifFile, this))
                    goto WRAPUP;

                m_ImageCount++;
                // Read and display local color table, if present.
                if (gifGraphicBlock.ImageDescriptor.PackedField & 0x80)
                    {
                    LineCount = 0;  /* Count of the number of lines displayed */
                    GlobalTableSize = (unsigned short) (1L << ((gifGraphicBlock.ImageDescriptor.PackedField & 0x07) + 1));
                    }

                // Adding the offset to the list
                Byte DecompressMinCodeSize;
                if (m_pGifFile->Read(&DecompressMinCodeSize, 1) != 1)
                    goto WRAPUP;

                m_ListPageDataOffset.push_back((int32_t)m_pGifFile->GetCurrentPos());
                m_ListPageDecompressMinCodeSize.push_back((short)DecompressMinCodeSize);

                BlockCount = 0;
                while ((m_pGifFile->Read(&DataSize, 1) == 1) && (DataSize != 0))
                    {
                    BlockCount++;
                    m_pGifFile->SeekToPos(m_pGifFile->GetCurrentPos() + (long) DataSize);
                    }

                // Adding the GifGraphicControl if present for this block.
                if (m_ValidGraphicControl)
                    gifGraphicBlock.GraphicControl = m_GifGraphicControl;
                else
                    memset(&gifGraphicBlock.GraphicControl, 0, sizeof(GifGraphicControl));

                // Adding the GifGraphicBlock to the list
                m_ListGifGraphicBlock.push_back((const GifGraphicBlock&) gifGraphicBlock);

                // Reset the flag.
                m_ValidGraphicControl = false;

                break;

                // Extension Block.
            case 0x21:
                if(!LookUpExtensionBlocks())
                    goto WRAPUP;
                break;
            default:
                fprintf(stderr, "\n\nUnknown Block Separator Character: 0x%02x\n\n", Identifier);
                goto WRAPUP;
            }
        }


    Status = true;

WRAPUP:

    return Status;
    }

//-----------------------------------------------------------------------------
// LookUpExtensionBlocks
// Private
// Identify, read, and display block information.
//-----------------------------------------------------------------------------
bool HRFGifFile::LookUpExtensionBlocks()
    {
    HPRECONDITION (SharingControlIsLocked());

    bool Status = false;

    GifGraphicControl   gifGraphicControl;
    GifPlainText        gifPlainText;
    GifApplication      gifApplication;
    GifComment          gifComment;

    Byte*  ptr;
    Byte  dataSize;
    Byte  Identifier = 0x21;
    Byte  Label;

    if (m_pGifFile->Read(&Label, 1) != 1)
        goto WRAPUP;

    switch (Label)
        {
            // Plain Text Extension
        case 0x01:
            gifPlainText.Introducer    = Identifier;
            gifPlainText.Label         = Label;
            gifPlainText.PlainTextData = 0;

            if (!ReadGifPlainText(&gifPlainText, m_pGifFile, this))
                goto WRAPUP;

            m_HasPlainText = true;
            delete [] gifPlainText.PlainTextData;
            break;

            // Comment Extension
        case 0xFE:
            {
            gifComment.Introducer  = Identifier;
            gifComment.Label       = Label;
            gifComment.CommentData = 0;

            if (!ReadGifComment(&gifComment, m_pGifFile, this))
                goto WRAPUP;

            m_HasComment = true;

            ostringstream Text;
            ptr = gifComment.CommentData;
            while ((dataSize = *ptr) != 0)
                {
                // increment the pointer to skip the data size
                ptr++;

                // add the current comment data in the stream
                Text << string((const char*)ptr, dataSize);

                // proceed to the next data
                ptr += dataSize;
                }

            // Adding tag Comment in the general tag list.
            // Notes Tag
            HFCPtr<HPMGenericAttribute> pTag = new HRFAttributeNotes(WString(Text.str().c_str(),false));
            m_GeneralTagList.Set(pTag);
            delete [] gifComment.CommentData;
            }
        break;

        // Graphic Control Extension
        case 0xF9:
            gifGraphicControl.Introducer = Identifier;
            gifGraphicControl.Label      = Label;

            if (!ReadGifGraphicControl(&gifGraphicControl, m_pGifFile, this))
                goto WRAPUP;

            m_GifGraphicControl = gifGraphicControl;
            m_ValidGraphicControl = true;

            break;

            // Application Extension.
        case 0xFF:
            {
            gifApplication.Introducer      = Identifier;
            gifApplication.Label           = Label;
            gifApplication.ApplicationData = 0;

            if (!ReadGifApplication(&gifApplication, m_pGifFile, this))
                goto WRAPUP;

            m_HasApplication = true;

            string soft("");
            string appCode("");

            soft += (const char) gifApplication.Identifier[0];
            soft += (const char) gifApplication.Identifier[1];
            soft += (const char) gifApplication.Identifier[2];
            soft += (const char) gifApplication.Identifier[3];
            soft += (const char) gifApplication.Identifier[4];
            soft += (const char) gifApplication.Identifier[5];
            soft += (const char) gifApplication.Identifier[6];
            soft += (const char) gifApplication.Identifier[7];

            appCode += (const char) gifApplication.AuthentCode[0];
            appCode += (const char) gifApplication.AuthentCode[1];
            appCode += (const char) gifApplication.AuthentCode[2];

            // Adding tag Comment in the general tag list.
            HFCPtr<HPMGenericAttribute> pTag;
            // Software tag
            pTag = new HRFAttributeSoftware(WString(soft.c_str(),false));
            m_GeneralTagList.Set(pTag);
            // Application code tag
            pTag = new HRFAttributeGIFApplicationCode(appCode);
            m_GeneralTagList.Set(pTag);

            delete [] gifApplication.ApplicationData;
            }
        break;
        default:
            printf("\n\nUnknown Extension Label: 0x%02x\n", Label);
            break;
        }

    Status = true;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
//  Read a GIF image file Header.
//
//  This function reads the Header, Logical Screen Descriptor, and
//  Global Color Table (if any) from a GIF image file.  The information
//  is stored in a GIFHEAD structure.
//
//-----------------------------------------------------------------------------
bool HRFGifFile::ReadGifHeader(GifHeader* pio_pGifHeader, HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster)
    {
    HPRECONDITION ((pi_pRaster == 0) || pi_pRaster->SharingControlIsLocked());

    bool Status = false;

    if ((pi_pGifFile->Read(&pio_pGifHeader->Signature,    sizeof(Byte) * 3)                  != (sizeof(Byte) * 3))                  ||
        (pi_pGifFile->Read(&pio_pGifHeader->Version,      sizeof(Byte) * 3)                  != (sizeof(Byte) * 3))                  ||
        (pi_pGifFile->Read(&pio_pGifHeader->ScreenWidth,  sizeof pio_pGifHeader->ScreenWidth)  != (sizeof pio_pGifHeader->ScreenWidth))  ||
        (pi_pGifFile->Read(&pio_pGifHeader->ScreenHeight, sizeof pio_pGifHeader->ScreenHeight) != (sizeof pio_pGifHeader->ScreenHeight)) ||
        (pi_pGifFile->Read(&pio_pGifHeader->PackedField,  sizeof pio_pGifHeader->PackedField)  != (sizeof pio_pGifHeader->PackedField))  ||
        (pi_pGifFile->Read(&pio_pGifHeader->ColorIndex,   sizeof pio_pGifHeader->ColorIndex)   != (sizeof pio_pGifHeader->ColorIndex))   ||
        (pi_pGifFile->Read(&pio_pGifHeader->AspectRatio,  sizeof pio_pGifHeader->AspectRatio)  != (sizeof pio_pGifHeader->AspectRatio)))
        goto WRAPUP;

    // Check if a Global Color Table is present.
    if (pio_pGifHeader->PackedField & 0x80)
        {
        // Read number of color table entries.
        unsigned short tableSize = (unsigned short) (1L << ((pio_pGifHeader->PackedField & 0x07) + 1));

        if (pi_pGifFile->Read(&pio_pGifHeader->GlobalCT, tableSize*3) != tableSize*3)
            goto WRAPUP;
        }

    Status = true;

WRAPUP:
    return Status;
    }


//-----------------------------------------------------------------------------
//  Read a GIF Local Image Descriptor.
//
//  This function reads the Local Image Descriptor, and Local Color
//  Table (if any) from a GIF image file.  The information is stored
//  in a GIFIMAGEDESC structure.
//
//  Note that the ImageSeparator field value in the GIFIMAGEDESC
//  structure is assigned by the function calling ReadGifImageDesc().
//
//-----------------------------------------------------------------------------
bool HRFGifFile::ReadGifImageDesc(GifImageDescriptor* pio_pGifImageDesc, HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster)
    {
    HPRECONDITION ((pi_pRaster == 0) || pi_pRaster->SharingControlIsLocked());

    bool Status = false;

    if ((pi_pGifFile->Read(&pio_pGifImageDesc->ImageLeft,   sizeof pio_pGifImageDesc->ImageLeft)   != (sizeof pio_pGifImageDesc->ImageLeft)) ||
        (pi_pGifFile->Read(&pio_pGifImageDesc->ImageTop,    sizeof pio_pGifImageDesc->ImageTop)    != (sizeof pio_pGifImageDesc->ImageTop)) ||
        (pi_pGifFile->Read(&pio_pGifImageDesc->ImageWidth,  sizeof pio_pGifImageDesc->ImageWidth)  != (sizeof pio_pGifImageDesc->ImageWidth)) ||
        (pi_pGifFile->Read(&pio_pGifImageDesc->ImageHeight, sizeof pio_pGifImageDesc->ImageHeight) != (sizeof pio_pGifImageDesc->ImageHeight)) ||
        (pi_pGifFile->Read(&pio_pGifImageDesc->PackedField, sizeof pio_pGifImageDesc->PackedField) != (sizeof pio_pGifImageDesc->PackedField)))
        goto WRAPUP;

    // Check if a Local Color Table is present.
    if (pio_pGifImageDesc->PackedField & 0x80)
        {
        // Read number of color table entries.
        unsigned short tableSize = (unsigned short) (1L << ((pio_pGifImageDesc->PackedField & 0x07) + 1));

        if (pi_pGifFile->Read(&pio_pGifImageDesc->LocalCT, tableSize*3) != tableSize*3)
            goto WRAPUP;
        }

    pio_pGifImageDesc->LineIndex = 0;

    Status = true;

WRAPUP:
    return Status;
    }


//-----------------------------------------------------------------------------
//  Read a GIF Graphic Control Extension block.
//
//  Note that the Introducer and Label field values in the GIFGRAPHICCONTROL
//  structure are assigned by the function calling ReadGifGraphicControl().
//
//-----------------------------------------------------------------------------
bool HRFGifFile::ReadGifGraphicControl(GifGraphicControl* pio_pGifGraphicControl, HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster)
    {
    HPRECONDITION ((pi_pRaster == 0) || pi_pRaster->SharingControlIsLocked());

    bool Status = true;

    if ((pi_pGifFile->Read(&pio_pGifGraphicControl->BlockSize,   sizeof pio_pGifGraphicControl->BlockSize)   != (sizeof pio_pGifGraphicControl->BlockSize)) ||
        (pi_pGifFile->Read(&pio_pGifGraphicControl->PackedField, sizeof pio_pGifGraphicControl->PackedField) != (sizeof pio_pGifGraphicControl->PackedField)) ||
        (pi_pGifFile->Read(&pio_pGifGraphicControl->DelayTime,   sizeof pio_pGifGraphicControl->DelayTime)   != (sizeof pio_pGifGraphicControl->DelayTime)) ||
        (pi_pGifFile->Read(&pio_pGifGraphicControl->ColorIndex,  sizeof pio_pGifGraphicControl->ColorIndex)  != (sizeof pio_pGifGraphicControl->ColorIndex)) ||
        (pi_pGifFile->Read(&pio_pGifGraphicControl->Terminator,  sizeof pio_pGifGraphicControl->Terminator)  != (sizeof pio_pGifGraphicControl->Terminator)))
        Status = false;

    return Status;
    }


//-----------------------------------------------------------------------------
//  Read a GIF Plain Text Extension block.
//
//  Note that the Introducer and Label field values in the GIFLPLAINTEXT
//  structure are assigned by the function calling ReadGifPlainText().
//
//-----------------------------------------------------------------------------
bool HRFGifFile::ReadGifPlainText(GifPlainText* pio_pGifPlainText, HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster)
    {
    HPRECONDITION ((pi_pRaster == 0) || pi_pRaster->SharingControlIsLocked());

    bool Status = false;

    if ((pi_pGifFile->Read(&pio_pGifPlainText->BlockSize,       sizeof pio_pGifPlainText->BlockSize)        != (sizeof pio_pGifPlainText->BlockSize)) ||
        (pi_pGifFile->Read(&pio_pGifPlainText->TextGridLeft,    sizeof pio_pGifPlainText->TextGridLeft)     != (sizeof pio_pGifPlainText->TextGridLeft)) ||
        (pi_pGifFile->Read(&pio_pGifPlainText->TextGridTop,     sizeof pio_pGifPlainText->TextGridTop)      != (sizeof pio_pGifPlainText->TextGridTop)) ||
        (pi_pGifFile->Read(&pio_pGifPlainText->TextGridWidth,   sizeof pio_pGifPlainText->TextGridWidth)    != (sizeof pio_pGifPlainText->TextGridWidth)) ||
        (pi_pGifFile->Read(&pio_pGifPlainText->TextGridHeight,  sizeof pio_pGifPlainText->TextGridHeight)   != (sizeof pio_pGifPlainText->TextGridHeight)) ||
        (pi_pGifFile->Read(&pio_pGifPlainText->CellWidth,       sizeof pio_pGifPlainText->CellWidth)        != (sizeof pio_pGifPlainText->CellWidth)) ||
        (pi_pGifFile->Read(&pio_pGifPlainText->CellHeight,      sizeof pio_pGifPlainText->CellHeight)       != (sizeof pio_pGifPlainText->CellHeight)) ||
        (pi_pGifFile->Read(&pio_pGifPlainText->TextFgColorIndex,sizeof pio_pGifPlainText->TextFgColorIndex) != (sizeof pio_pGifPlainText->TextFgColorIndex)) ||
        (pi_pGifFile->Read(&pio_pGifPlainText->TextBgColorIndex,sizeof pio_pGifPlainText->TextBgColorIndex) != (sizeof pio_pGifPlainText->TextBgColorIndex)))
        goto WRAPUP;

    // Read in the Plain Text data sub-blocks.
    if (!(pio_pGifPlainText->PlainTextData = ReadDataSubBlocks(pi_pGifFile, pi_pRaster)))
        Status = false;

    pio_pGifPlainText->Terminator       = 0;

    Status = true;

WRAPUP:
    return Status;
    }


//-----------------------------------------------------------------------------
//  Read a GIF Application Extension block.
//
//  Note that the Introducer and Label field values in the GIFAPPLICATION
//  structure are assigned by the function calling ReadGifApplication().
//
//-----------------------------------------------------------------------------
bool HRFGifFile::ReadGifApplication(GifApplication* pio_pGifApplication, HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster)
    {
    HPRECONDITION ((pi_pRaster == 0) || pi_pRaster->SharingControlIsLocked());

    bool Status = false;

    if ((pi_pGifFile->Read(&pio_pGifApplication->BlockSize,   sizeof(pio_pGifApplication->BlockSize)) != (sizeof(pio_pGifApplication->BlockSize))) ||
        (pi_pGifFile->Read(&pio_pGifApplication->Identifier,  sizeof(Byte) * 8) != (sizeof(Byte) * 8)) ||
        (pi_pGifFile->Read(&pio_pGifApplication->AuthentCode, sizeof(Byte) * 3) != (sizeof(Byte) * 3)))
        goto WRAPUP;

    // Read in the Application data sub-blocks.
    if (!(pio_pGifApplication->ApplicationData = ReadDataSubBlocks(pi_pGifFile, pi_pRaster)))
        goto WRAPUP;

    Status = true;

WRAPUP:
    return Status;
    }


//-----------------------------------------------------------------------------
//  Read a GIF Comment Extension block.
//
//  Note that the Introducer and Label field values in the GIFCOMMENT
//  structure are assigned by the function calling ReadGifComment().
//
//-----------------------------------------------------------------------------
bool HRFGifFile::ReadGifComment(GifComment* pio_pGifComment, HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster)
    {
    HPRECONDITION ((pi_pRaster == 0) || pi_pRaster->SharingControlIsLocked());

    bool Status = true;

    // Read in the Comment data sub-blocks.
    if (!(pio_pGifComment->CommentData = ReadDataSubBlocks(pi_pGifFile, pi_pRaster)))
        Status = false;

    pio_pGifComment->Terminator = 0;

    return Status;
    }


//-----------------------------------------------------------------------------
//  Read one or more GIF data sub-blocks and write the information
//  to a buffer.
//
//  A GIF "sub-block" is a single count byte followed by 1 to 255
//  additional data bytes.
//
//  Returns: A NULL pointer if a memory allocation error occured,
//           otherwise a valid pointer if no error occured.
//-----------------------------------------------------------------------------
Byte* HRFGifFile::ReadDataSubBlocks(HFCBinStream* pi_pGifFile, HRFRasterFile* pi_pRaster)
    {
    HPRECONDITION ((pi_pRaster == 0) || pi_pRaster->SharingControlIsLocked());

#define BUFFER_INCREMENT 4096

    Byte  blockDataSize(0);

    // Get the size of the first sub-block.
    if (pi_pGifFile->Read(&blockDataSize, 1) != 1)
        return 0;
    else
        {

        // Allocate initial data buffer with extra space
        HFCBuffer Buffer(BUFFER_INCREMENT);

        while(blockDataSize)
            {
            // Write current block size
            *(Buffer.PrepareForNewData(1)) = blockDataSize;
            Buffer.SetNewDataSize(1);   

            // Read current block
            if (pi_pGifFile->Read(Buffer.PrepareForNewData(blockDataSize), blockDataSize) != blockDataSize)
                return 0;
            Buffer.SetNewDataSize(blockDataSize);       // Set data size read

            // Get size of the next block
            if (pi_pGifFile->Read(&blockDataSize, 1) != 1)
                return 0;
            }

        // Need to return a buffer # 0 even if not data read, create a temp buffer.
        if (Buffer.GetDataSize() > 0)
        {
            *(Buffer.PrepareForNewData(1)) = 0;
            Buffer.SetNewDataSize(1);      
            return (Byte*)Buffer.GetDataAndRelease();
        }
        else
            return new Byte[4];
        }
    }

//-----------------------------------------------------------------------------
//  Write a GIF image file Header.
//
//  This function write the Header, Logical Screen Descriptor, and
//  Global Color Table (if any) from a GIF image file.  The information
//  is stored in a GIFHEAD structure.
//
//-----------------------------------------------------------------------------
bool HRFGifFile::WriteGifHeader(GifHeader* pi_pGifHeader, HFCBinStream* pio_pGifFile)
    {
    HPRECONDITION (SharingControlIsLocked());

    bool Status = false;

    // Number of entires in the Global Color Table.
    unsigned short tableSize;

    if ((pio_pGifFile->Write(&pi_pGifHeader->Signature,     sizeof(Byte) * 3)                  != (sizeof(Byte) * 3)) ||
        (pio_pGifFile->Write(&pi_pGifHeader->Version,       sizeof(Byte) * 3)                  != (sizeof(Byte) * 3)) ||
        (pio_pGifFile->Write(&pi_pGifHeader->ScreenWidth,   sizeof(pi_pGifHeader->ScreenWidth))  != (sizeof(pi_pGifHeader->ScreenWidth))) ||
        (pio_pGifFile->Write(&pi_pGifHeader->ScreenHeight,  sizeof(pi_pGifHeader->ScreenHeight)) != (sizeof(pi_pGifHeader->ScreenHeight))) ||
        (pio_pGifFile->Write(&pi_pGifHeader->PackedField,   sizeof(pi_pGifHeader->PackedField))  != (sizeof(pi_pGifHeader->PackedField))) ||
        (pio_pGifFile->Write(&pi_pGifHeader->ColorIndex,    sizeof(pi_pGifHeader->ColorIndex))   != (sizeof(pi_pGifHeader->ColorIndex))) ||
        (pio_pGifFile->Write(&pi_pGifHeader->AspectRatio,   sizeof(pi_pGifHeader->AspectRatio))  != (sizeof(pi_pGifHeader->AspectRatio))))
        goto WRAPUP;

    // Check if a Global Color Table is present.
    if (pi_pGifHeader->PackedField & 0x80)
        {
        // Write number of color table entries.
        tableSize = (unsigned short) (1L << ((pi_pGifHeader->PackedField & 0x07) + 1));

        // Write the Global Color Table.
        if (!WritePalette(pi_pGifHeader->GlobalCT, tableSize, pio_pGifFile))
            goto WRAPUP;
        }


    Status = true;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
//  Write a GIF palette.
//
//-----------------------------------------------------------------------------
bool HRFGifFile::WritePalette(GifColorTable pi_pColorTable, uint32_t pi_TableSize, HFCBinStream* pio_pGifFile)
    {
    HPRECONDITION (SharingControlIsLocked());

    for (unsigned short i = 0; i < pi_TableSize; i++)
        {
        if (pio_pGifFile->Write(&pi_pColorTable[i].Red,   sizeof pi_pColorTable[i].Red)   != sizeof(Byte) ||
            pio_pGifFile->Write(&pi_pColorTable[i].Green, sizeof pi_pColorTable[i].Green) != sizeof(Byte) ||
            pio_pGifFile->Write(&pi_pColorTable[i].Blue,  sizeof pi_pColorTable[i].Blue)  != sizeof(Byte))
            return false;
        }
    return true;
    }

//-----------------------------------------------------------------------------
//  Write a GIF Local Image Descriptor.
//
//  This function write the Local Image Descriptor, and Local Color
//  Table (if any) from a GIF image file.  The information is stored
//  in a GIFIMAGEDESC structure.
//
//-----------------------------------------------------------------------------
bool HRFGifFile::WriteGifImageDesc(GifImageDescriptor* pi_pGifImageDesc, HFCBinStream* pio_pGifFile)
    {
    HPRECONDITION (SharingControlIsLocked());

    bool Status = false;

    // Number of entries in the Local Color Table.
    unsigned short tableSize;

    if ((pio_pGifFile->Write(&pi_pGifImageDesc->ImageSeparator, sizeof pi_pGifImageDesc->ImageSeparator)    != (sizeof pi_pGifImageDesc->ImageSeparator)) ||
        (pio_pGifFile->Write(&pi_pGifImageDesc->ImageLeft,      sizeof pi_pGifImageDesc->ImageLeft)         != (sizeof pi_pGifImageDesc->ImageLeft)) ||
        (pio_pGifFile->Write(&pi_pGifImageDesc->ImageTop,       sizeof pi_pGifImageDesc->ImageTop)          != (sizeof pi_pGifImageDesc->ImageTop)) ||
        (pio_pGifFile->Write(&pi_pGifImageDesc->ImageWidth,     sizeof pi_pGifImageDesc->ImageWidth)        != (sizeof pi_pGifImageDesc->ImageWidth)) ||
        (pio_pGifFile->Write(&pi_pGifImageDesc->ImageHeight,    sizeof pi_pGifImageDesc->ImageHeight)       != (sizeof pi_pGifImageDesc->ImageHeight)) ||
        (pio_pGifFile->Write(&pi_pGifImageDesc->PackedField,    sizeof pi_pGifImageDesc->PackedField)       != (sizeof pi_pGifImageDesc->PackedField)))
        goto WRAPUP;

    // Check if a Local Color Table is present.
    if (pi_pGifImageDesc->PackedField & 0x80)
        {
        // Read number of color table entries.
        tableSize = (unsigned short) (1L << ((pi_pGifImageDesc->PackedField & 0x07) + 1));

        // Write the Global Color Table.
        if (!WritePalette(pi_pGifImageDesc->LocalCT, tableSize, pio_pGifFile))
            goto WRAPUP;
        }

    Status = true;

WRAPUP:

    return Status;
    }


//-----------------------------------------------------------------------------
//  Write a GIF Graphic Control Extension block.
//
//-----------------------------------------------------------------------------
bool HRFGifFile::WriteGifGraphicControl(GifGraphicControl* pi_pGifGraphicControl, HFCBinStream* pio_pGifFile)
    {
    HPRECONDITION (SharingControlIsLocked());

    bool Status = true;

    if ((pio_pGifFile->Write(&pi_pGifGraphicControl->Introducer,    sizeof pi_pGifGraphicControl->Introducer)     != (sizeof pi_pGifGraphicControl->Introducer)) ||
        (pio_pGifFile->Write(&pi_pGifGraphicControl->Label,         sizeof pi_pGifGraphicControl->Label)          != (sizeof pi_pGifGraphicControl->Label)) ||
        (pio_pGifFile->Write(&pi_pGifGraphicControl->BlockSize,     sizeof pi_pGifGraphicControl->BlockSize)      != (sizeof pi_pGifGraphicControl->BlockSize)) ||
        (pio_pGifFile->Write(&pi_pGifGraphicControl->PackedField,   sizeof pi_pGifGraphicControl->PackedField)    != (sizeof pi_pGifGraphicControl->PackedField)) ||
        (pio_pGifFile->Write(&pi_pGifGraphicControl->DelayTime,     sizeof pi_pGifGraphicControl->DelayTime)      != (sizeof pi_pGifGraphicControl->DelayTime)) ||
        (pio_pGifFile->Write(&pi_pGifGraphicControl->ColorIndex,    sizeof pi_pGifGraphicControl->ColorIndex)     != (sizeof pi_pGifGraphicControl->ColorIndex)) ||
        (pio_pGifFile->Write(&pi_pGifGraphicControl->Terminator,    sizeof pi_pGifGraphicControl->Terminator)     != (sizeof pi_pGifGraphicControl->Terminator)))
        {
        Status = false;
        }

    return Status;
    }


//-----------------------------------------------------------------------------
//  Write a GIF Plain Text Extension block.
//
//-----------------------------------------------------------------------------
bool HRFGifFile::WriteGifPlainText(GifPlainText* pi_pGifPlainText, uint32_t pi_SizeOfData, HFCBinStream* pio_pGifFile)
    {
    HPRECONDITION (SharingControlIsLocked());

    bool Status = false;

    if ((pio_pGifFile->Write(&pi_pGifPlainText->Introducer,         sizeof pi_pGifPlainText->Introducer)          != (sizeof pi_pGifPlainText->Introducer)) ||
        (pio_pGifFile->Write(&pi_pGifPlainText->Label,              sizeof pi_pGifPlainText->Label)               != (sizeof pi_pGifPlainText->Label)) ||
        (pio_pGifFile->Write(&pi_pGifPlainText->BlockSize,          sizeof pi_pGifPlainText->BlockSize)           != (sizeof pi_pGifPlainText->BlockSize)) ||
        (pio_pGifFile->Write(&pi_pGifPlainText->TextGridLeft,       sizeof pi_pGifPlainText->TextGridLeft)        != (sizeof pi_pGifPlainText->TextGridLeft)) ||
        (pio_pGifFile->Write(&pi_pGifPlainText->TextGridTop,        sizeof pi_pGifPlainText->TextGridTop)         != (sizeof pi_pGifPlainText->TextGridTop)) ||
        (pio_pGifFile->Write(&pi_pGifPlainText->TextGridWidth,      sizeof pi_pGifPlainText->TextGridWidth)       != (sizeof pi_pGifPlainText->TextGridWidth)) ||
        (pio_pGifFile->Write(&pi_pGifPlainText->TextGridHeight,     sizeof pi_pGifPlainText->TextGridHeight)      != (sizeof pi_pGifPlainText->TextGridHeight)) ||
        (pio_pGifFile->Write(&pi_pGifPlainText->CellWidth,          sizeof pi_pGifPlainText->CellWidth)           != (sizeof pi_pGifPlainText->CellWidth)) ||
        (pio_pGifFile->Write(&pi_pGifPlainText->CellHeight,         sizeof pi_pGifPlainText->CellHeight)          != (sizeof pi_pGifPlainText->CellHeight)) ||
        (pio_pGifFile->Write(&pi_pGifPlainText->TextFgColorIndex,   sizeof pi_pGifPlainText->TextFgColorIndex)    != (sizeof pi_pGifPlainText->TextFgColorIndex)) ||
        (pio_pGifFile->Write(&pi_pGifPlainText->TextBgColorIndex,   sizeof pi_pGifPlainText->TextBgColorIndex)    != (sizeof pi_pGifPlainText->TextBgColorIndex)))
        goto WRAPUP;

    // Read in the Plain Text data sub-blocks.
    if (!WriteDataSubBlocks(pi_SizeOfData, pi_pGifPlainText->PlainTextData, pio_pGifFile))
        goto WRAPUP;

    Status = true;

WRAPUP:
    return Status;
    }


//-----------------------------------------------------------------------------
//  Write a GIF Application Extension block.
//
//  Note that the Introducer and Label field values in the GIFAPPLICATION
//  structure are assigned by the function calling ReadGifApplication().
//
//-----------------------------------------------------------------------------
bool HRFGifFile::WriteGifApplication(GifApplication* pi_pGifApplication, uint32_t pi_SizeOfData, HFCBinStream* pio_pGifFile)
    {
    HPRECONDITION (SharingControlIsLocked());

    bool Status = false;

    if ((pio_pGifFile->Write(&pi_pGifApplication->Introducer,   sizeof pi_pGifApplication->Introducer)   != (sizeof pi_pGifApplication->Introducer)) ||
        (pio_pGifFile->Write(&pi_pGifApplication->Label,        sizeof pi_pGifApplication->Label)        != (sizeof pi_pGifApplication->Label)) ||
        (pio_pGifFile->Write(&pi_pGifApplication->BlockSize,    sizeof pi_pGifApplication->BlockSize)    != (sizeof pi_pGifApplication->BlockSize)) ||
        (pio_pGifFile->Write(&pi_pGifApplication->Identifier,   sizeof(Byte) * 8)                      != (sizeof(Byte) * 8)) ||
        (pio_pGifFile->Write(&pi_pGifApplication->AuthentCode,  sizeof(Byte) * 3)                      != (sizeof(Byte) * 3)))
        goto WRAPUP;

    // Write the Application data sub-blocks and the terminator.
    if (!WriteDataSubBlocks(pi_SizeOfData, pi_pGifApplication->ApplicationData, pio_pGifFile))
        goto WRAPUP;

    Status = true;

WRAPUP:

    return Status;
    }


//-----------------------------------------------------------------------------
//  Write a GIF Comment Extension block.
//
//-----------------------------------------------------------------------------
bool HRFGifFile::WriteGifComment(GifComment* pi_pGifComment, uint32_t pi_SizeOfData, HFCBinStream* pio_pGifFile)
    {
    HPRECONDITION (SharingControlIsLocked());

    bool Status = false;

    if ((pio_pGifFile->Write(&pi_pGifComment->Introducer,   sizeof pi_pGifComment->Introducer)   != (sizeof pi_pGifComment->Introducer)) ||
        (pio_pGifFile->Write(&pi_pGifComment->Label,        sizeof pi_pGifComment->Label)        != (sizeof pi_pGifComment->Label)))
        goto WRAPUP;


    // Read in the Comment data sub-blocks.
    if (!WriteDataSubBlocks(pi_SizeOfData, pi_pGifComment->CommentData, pio_pGifFile))
        goto WRAPUP;

    Status = true;

WRAPUP:

    return Status;
    }


//-----------------------------------------------------------------------------
//  Write one or more GIF data sub-blocks
//
//  A GIF "sub-block" is a single count byte followed by 1 to 255
//  additional data bytes.
//
//  Returns: A NULL pointer if error occured,
//           otherwise a valid pointer if no error occured.
//-----------------------------------------------------------------------------
bool HRFGifFile::WriteDataSubBlocks(uint32_t pi_BufferSize, Byte* pi_pBuffer, HFCBinStream* pio_pGifFile)
    {
    HPRECONDITION (SharingControlIsLocked());

    bool Status = false;

    uint32_t indexInBuffer           = 0;
    uint32_t numberOfBlock           = pi_BufferSize / 255;
    uint32_t numberOfByteInLastBlock = pi_BufferSize % 255;
    Byte CodecZero = 0x00;

    while(numberOfBlock != 0)
        {
        Byte Code = 255;
        if (pio_pGifFile->Write(&Code, 1) != 1 ||
            pio_pGifFile->Write((pi_pBuffer+indexInBuffer), 255) != 255)
            goto WRAPUP;

        indexInBuffer += 255;
        numberOfBlock--;
        }

    if (numberOfByteInLastBlock != 0)
        {
        if ((pio_pGifFile->Write(&numberOfByteInLastBlock, 1) != 1) ||
            (pio_pGifFile->Write((pi_pBuffer+indexInBuffer), numberOfByteInLastBlock) == 0))
            goto WRAPUP;
        indexInBuffer += numberOfByteInLastBlock;
        }

    // Terminator Block (0x00)
    if (pio_pGifFile->Write(&CodecZero, 1) != 1)
        goto WRAPUP;

    Status = true;

WRAPUP:
    return Status;
    }


//-----------------------------------------------------------------------------
//    Write the GIF signature, the screen description, and the optional
//    color map.
//
//-----------------------------------------------------------------------------
void HRFGifFile::SetHeader(unsigned short  pi_Width,
                           unsigned short  pi_Height,
                           unsigned short  pi_BitsColorResolution,
                           unsigned short  pi_BackgroundColor,
                           unsigned short  pi_BitsByPixel,
                           Byte           pi_AspectRatio,          // TO DO
                           HRPPixelPalette* pi_pPalette)
    {
    m_GifHeader.Signature[0] = 'G';
    m_GifHeader.Signature[1] = 'I';
    m_GifHeader.Signature[2] = 'F';
    m_GifHeader.Version[0]   = '8';
    m_GifHeader.Version[1]   = '9';
    m_GifHeader.Version[2]   = 'a';

    m_GifHeader.ScreenWidth  = pi_Width;
    m_GifHeader.ScreenHeight = pi_Height;

    m_GifHeader.PackedField  =  CONVERT_TO_BYTE((0x80 |
                                                ((pi_BitsColorResolution - 1) << 4) |
                                                ((pi_BitsByPixel - 1) & 0x0F)));

    m_GifHeader.ColorIndex   = (Byte)pi_BackgroundColor;
    m_GifHeader.AspectRatio  = pi_AspectRatio;

    SetPalette(pi_BitsColorResolution, pi_BitsByPixel, pi_pPalette, m_GifHeader.GlobalCT);
    }

//-----------------------------------------------------------------------------
//    Write the graphic control and optional local color map.
//
//-----------------------------------------------------------------------------
void HRFGifFile::SetGraphicControl(unsigned short    pi_DelayTime,              // TO DO
                                   Byte             pi_TransparentColorIndex,
                                   Byte             pi_DisposalMethode,        // TO DO
                                   Byte             pi_UserInput,              // TO DO
                                   GifGraphicControl* po_pGraphicControl)
    {
    po_pGraphicControl->Introducer = 0x21;
    po_pGraphicControl->Label      = 0xf9;
    po_pGraphicControl->BlockSize  = 0x04;

    if (pi_TransparentColorIndex == -1)
        po_pGraphicControl->PackedField = 0x00 |
                                          (pi_DisposalMethode << 2) |
                                          (pi_UserInput << 1);
    else
        po_pGraphicControl->PackedField = 0x00 |
                                          (pi_DisposalMethode << 2) |
                                          (pi_UserInput << 1) |
                                          (0x01);

    po_pGraphicControl->DelayTime  = pi_DelayTime;
    po_pGraphicControl->ColorIndex = pi_TransparentColorIndex;
    po_pGraphicControl->Terminator = 0;
    }

//-----------------------------------------------------------------------------
//    Write the image description and optional local color map.
//
//-----------------------------------------------------------------------------
void HRFGifFile::SetImageDesc(unsigned short     pi_LeftEdge,
                              unsigned short     pi_TopEdge,
                              unsigned short     pi_Width,
                              unsigned short     pi_Height,
                              unsigned short     pi_BitsColorResolution,
                              bool               pi_Interlaced,
                              unsigned short     pi_BitsByPixel,
                              HRPPixelPalette*    pi_pPalette,
                              GifImageDescriptor* po_pImageDescriptor)
    {
    unsigned short interlace = 0;
    if (pi_Interlaced)
        interlace = 1;

    // Low bit only.
    interlace &= 1;

    po_pImageDescriptor->ImageSeparator = ',';
    po_pImageDescriptor->ImageLeft      = pi_LeftEdge;
    po_pImageDescriptor->ImageTop       = pi_TopEdge;
    po_pImageDescriptor->ImageWidth     = pi_Width;
    po_pImageDescriptor->ImageHeight    = pi_Height;


    po_pImageDescriptor->PackedField =   CONVERT_TO_BYTE((0x80 |
                                                        (interlace << 6) |
                                                        ((pi_BitsByPixel - 1) & 0x0F)));

    SetPalette(pi_BitsColorResolution, pi_BitsByPixel, pi_pPalette, po_pImageDescriptor->LocalCT);
    // Set the line index to 0.
    po_pImageDescriptor->LineIndex = 0;
    }

//-----------------------------------------------------------------------------
//    Write the specified color map.
//
// Inputs:
//    (see above)
//
// Returns:
//    0 = OK, else error code
//-----------------------------------------------------------------------------
void HRFGifFile::SetPalette (unsigned short  pi_BitsColorResolution,
                             unsigned short  pi_BitsByPixel,
                             HRPPixelPalette* pi_pPalette,
                             GifColorTable    po_ColorTable)
    {
    uint32_t         MaxColors;
    uint32_t        MaxHistoEntry   = 0;
    bool           HasAlpha        = false;
    Byte*          pPaletteValue;

    // No tranparent color
    m_AlphaColorIndex = -1;

    MaxColors = 1 << pi_BitsByPixel;

    if((pi_pPalette->GetChannelOrg()).GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
        HasAlpha = true;

    for (uint32_t Index=0 ; Index < MaxColors ; Index++)
        {
		Byte defaultColor[4];
        if (Index < pi_pPalette->CountUsedEntries())
            {
            pPaletteValue = (Byte*)pi_pPalette->GetCompositeValue(Index);
            }
        else
            {
            defaultColor[0]   = (Byte)Index;
            defaultColor[1]   = (Byte)Index;
            defaultColor[2]   = (Byte)Index;
			defaultColor[3]   = 255;
			pPaletteValue = (Byte*)&defaultColor;
            }

        po_ColorTable[Index].Red   = pPaletteValue[0];
        po_ColorTable[Index].Green = pPaletteValue[1];
        po_ColorTable[Index].Blue  = pPaletteValue[2];

        if(HasAlpha && pPaletteValue[3] == 0)
            {
            if (m_pHistoCreationMode != 0)
                {
                if (m_pHistoCreationMode[Index] > MaxHistoEntry)
                    {
                    MaxHistoEntry = m_pHistoCreationMode[Index];
                    m_AlphaColorIndex = (short)Index;
                    }
                }
            else
                m_AlphaColorIndex = (short)Index;
            }
        }
    }

//-----------------------------------------------------------------------------
//    SetDirty
//-----------------------------------------------------------------------------
void HRFGifFile::SetDirty(bool pi_Dirty)
    {
    m_RasterIsDirty = pi_Dirty;
    }
//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------

HRFGifFile::HRFGifFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen     = false;
    m_ImageCount = 0;
    m_RasterIsDirty = false;

    if (GetAccessMode().m_HasCreateAccess)
        {
        Create();
        }
    else
        {
        // if Open success and it is not a new file
        Open();
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFGifFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }


//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFGifFile::HRFGifFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset,
                              bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen        = false;
    m_RasterIsDirty = false;
    }


//-----------------------------------------------------------------------------
// Protected
// GetFilePtr
// Get the Gif file pointer.
//-----------------------------------------------------------------------------
HFCBinStream* HRFGifFile::GetFilePtr  ()
    {
    return (m_pGifFile);
    }
