//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPngFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFPngFile
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRFPngFile.h>

#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCURLMemFile.h>
#include <Imagepp/all/h/HFCMemoryBinStream.h>
#include <Imagepp/all/h/HFCBuffer.h>

#include <Imagepp/all/h/HRPChannelOrgGray.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPHistogram.h>
#include <Imagepp/all/h/HRFPngLineEditor.h>
#include <Imagepp/all/h/HRFPngImageEditor.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI2R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HFCMemoryBinStream.h>
#include <Imagepp/all/h/HFCResourceLoader.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//**************** TO DO ******************
//#include "HRPPixelTypeI2SR8G8B8A8.h"
//#include "HRPPixelTypeV16Gray8Alpha8.h"
//#include "HRPPixelTypeV16Gray16.h"
//#include "HRPPixelTypeV32Gray16Alpha16.h"
//#include "HRPPixelTypeV48R16G16B16.h"
//#include "HRPPixelTypeV64R16G16B16A16.h"
//******************************************

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <png/png.h>

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

static void hmr_png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
    {
    png_size_t check;

    /* fread() returns 0 on error, so it is OK to store this in a png_size_t
     * instead of an int, which is what fread() actually returns.
     */
    HASSERT_X64(length < ULONG_MAX);
    check = (png_size_t) ((HFCBinStream*)png_ptr->io_ptr)->Read((void*)data, length);
    if (check != length)
        {
        png_error(png_ptr, "Read Error");
        }
    }

static void hmr_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
    {
    png_size_t check;

    HASSERT_X64(length < ULONG_MAX);
    check = (png_size_t) ((HFCBinStream*)png_ptr->io_ptr)->Write((void*)data, length);
    if (check != length)
        {
        png_error(png_ptr, "Write Error");
        }
    }

static void hmr_png_flush(png_structp png_ptr)
    {
    ((HFCBinStream*)png_ptr->io_ptr)->Flush();
    }

//-----------------------------------------------------------------------------
// HRFPngBlockCapabilities
//-----------------------------------------------------------------------------
class HRFPngBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFPngBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,        // AccessMode
                                  LONG_MAX,                     // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL)); // BlockAccess

        // Image Capability
        Add(new HRFImageCapability(HFC_READ_WRITE,  // AccessMode
                                   LONG_MAX,        // MaxSizeInBytes
                                   0,               // MinWidth
                                   LONG_MAX,        // MaxWidth
                                   0,               // MinHeight
                                   LONG_MAX));      // MaxHeight
        }
    };

//-----------------------------------------------------------------------------
// HRFPngCodecCapabilities
//-----------------------------------------------------------------------------
class HRFPngCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFPngCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Zlib (deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFPngBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFPngCapabilities
//-----------------------------------------------------------------------------
HRFPngCapabilities::HRFPngCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeI1R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI1R8G8B8::CLASS_ID,
                                   new HRFPngCodecCapabilities()));
    // PixelTypeI1R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI1R8G8B8A8::CLASS_ID,
                                   new HRFPngCodecCapabilities()));

    // This pixel type does not work properly...  -CedricB
//    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
//                                   HRPPixelTypeI2R8G8B8::CLASS_ID,
//                                   new HRFPngCodecCapabilities()));

    // PixelTypeI4R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI4R8G8B8::CLASS_ID,
                                   new HRFPngCodecCapabilities()));

    // PixelTypeI4R8G8B8A8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI4R8G8B8A8::CLASS_ID,
                                   new HRFPngCodecCapabilities()));

    // PixelTypeI1R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                   new HRFPngCodecCapabilities()));
    // PixelTypeV8Gray8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFPngCodecCapabilities()));
    // PixelTypeI8R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFPngCodecCapabilities()));
    // PixelTypeI8R8G8B8A8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8R8G8B8A8::CLASS_ID,
                                   new HRFPngCodecCapabilities()));
    // PixelTypeV24R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFPngCodecCapabilities()));
    // PixelTypeV32R8G8B8A8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV32R8G8B8A8::CLASS_ID,
                                   new HRFPngCodecCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Interlace capability
    Add(new HRFInterlaceCapability(HFC_READ_WRITE));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // Histogram capability
    Add(new HRFHistogramCapability(HFC_READ_CREATE));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeImageGamma(0)));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeBackground(0)));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeTimeModification));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeTitle));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeArtist));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeImageDescription));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeCopyright));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeDateTime));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeLegalDisclaimer));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeContentWarning));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeHostComputer));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeNotes));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeXResolution(0)));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeYResolution(0)));
    Add(new HRFTagCapability(HFC_READ_CREATE, new HRFAttributeResolutionUnit(0)));
    }

HFC_IMPLEMENT_SINGLETON(HRFPngCreator)

//-----------------------------------------------------------------------------
// Constructor
// Public (HRFPngCreator)
// This is the creator to instantiate Png format
//-----------------------------------------------------------------------------
HRFPngCreator::HRFPngCreator()
    : HRFRasterFileCreator(HRFPngFile::CLASS_ID)
    {
    // Png capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// GetLabel
// Public (HRFPngCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFPngCreator::GetLabel() const
    {
    HFCResourceLoader* stringLoader = HFCResourceLoader::GetInstance();
    return stringLoader->GetString(IDS_FILEFORMAT_PNG); // PNG File Format
    }

//-----------------------------------------------------------------------------
// GetSchemes
// Public (HRFPngCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFPngCreator::GetSchemes() const
    {
    return HFCURLFile::s_SchemeName() + L";" + HFCURLMemFile::s_SchemeName();
    }

//-----------------------------------------------------------------------------
// GetExtensions
// Public (HRFPngCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFPngCreator::GetExtensions() const
    {
    return WString(L"*.png");
    }

//-----------------------------------------------------------------------------
// Create
// Public (HRFPngCreator)
// Allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFPngCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode   pi_AccessMode,
                                            uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFPngFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// IsKindOfFile
// Public (HRFPngCreator)
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFPngCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HAutoPtr<HFCBinStream>  pFile;

    bool                    Result = false;
    unsigned char           Buffer[8];

    // disable std io FILE*                   pPngFile       = 0;
    png_structp             pPngFileStruct = 0;
    png_infop               pPngInfo       = 0;
    png_infop               pPngEndInfo    = 0;


    (const_cast<HRFPngCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the PNG File & place file pointer at the start of the file
    // Read the first 8 bytes
    // Check if the file is a PNG or not
    pFile = HFCBinStream::Instanciate(CreateCombinedURLAndOffset(pi_rpURL, pi_Offset), HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastExceptionID() == NO_EXCEPTION)
        {
        if ((pFile->Read((void*)&Buffer, 1 * 8) == 8) && (png_check_sig(Buffer, 8)))
            {
            // Get the file read struct, info struct and End Struct.
            if ((pPngFileStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) &&
                (pPngInfo = png_create_info_struct(pPngFileStruct)) &&
                (pPngEndInfo = png_create_info_struct(pPngFileStruct)))
                {
                // Set png error handeling
                int JmpRes = setjmp(pPngFileStruct->jmpbuf);

                if (JmpRes == 0)
                    {
                    // Setup the input control to the standard C stream.

                    // disable the std io function png_init_io(pPngFileStruct, pPngFile);
                    png_set_read_fn( pPngFileStruct,                // png struct
                                     pFile,                         // IO stream
                                     hmr_png_read_data);            // IO Read function

                    // Tell png_lib that we have already read the first 8 bytes of the file.
                    png_set_sig_bytes(pPngFileStruct, 8);

                    // Read the file information.
                    png_read_info(pPngFileStruct, pPngInfo);

                    // Every file format that reach that point are supported
                    // at least for the brut raster data.     -CedricB
                    Result = true;
                    }
                }

            // Free the memory
            png_destroy_read_struct(&pPngFileStruct, &pPngInfo, &pPngEndInfo);
            }
        }

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFPngCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFPngCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public (HRFPngCreator)
// Create or get the singleton capabilities of PNG file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFPngCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFPngCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFPngFile::HRFPngFile(const HFCPtr<HFCURL>& pi_rURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen     = false;

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
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFPngFile::HRFPngFile(const HFCPtr<HFCURL>& pi_rURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset,
                       bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen     = false;
    }

//-----------------------------------------------------------------------------
// Destructor
// Public
// Destructor of PNG file
//-----------------------------------------------------------------------------
HRFPngFile::~HRFPngFile()
    {
    try
        {
        // Close the PNG file
        SavePngFile(true);
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
HRFResolutionEditor* HRFPngFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    if (!GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->IsInterlace())
        pEditor = new HRFPngLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    else
        pEditor = new HRFPngImageEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// Save
// Public
// Saves the file
//-----------------------------------------------------------------------------
void HRFPngFile::Save()
    {
    //Keep last file position
    uint64_t CurrentPos = m_pPngFile->GetCurrentPos();

    SavePngFile(false);

    //Set back position
    m_pPngFile->SeekToPos(CurrentPos);
    }

//-----------------------------------------------------------------------------
// AddPage
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFPngFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(pi_pPage != 0);
    HPRECONDITION(CountPages() == 0);

    // Add the page descriptor to the list
    return HRFRasterFile::AddPage(pi_pPage);
    }

//-----------------------------------------------------------------------------
// AssignStructTo
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFPngFile::AssignStructTo(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pi_pPage->GetResolutionDescriptor(0);

    // Set the image file information.
    m_pPngInfo->height = (uint32_t)pResolutionDescriptor->GetHeight();
    m_pPngInfo->width  = (uint32_t)pResolutionDescriptor->GetWidth();

    // Set the image color space.
    SetPixelTypeToPage(pResolutionDescriptor);

    // Display each tag.
    HPMAttributeSet::HPMASiterator TagIterator;

    unsigned short Unit = 1; // Unkown unit
    double XResolution = 0;
    double YResolution = 0;

    for (TagIterator  = pi_pPage->GetTags().begin(); 
         TagIterator != pi_pPage->GetTags().end(); TagIterator++)
        {
        HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);
        
        // Image Gamma Tag
        if (pTag->GetID() == HRFAttributeImageGamma::ATTRIBUTE_ID)
            png_set_gAMA(m_pPngFileStruct, m_pPngInfo, ((HFCPtr<HRFAttributeImageGamma>&)pTag)->GetData());
        // Background Tag
        else if (pTag->GetID() == HRFAttributeBackground::ATTRIBUTE_ID)
            {
            m_pPngInfo->valid  |= PNG_INFO_bKGD;
            switch (m_pPngInfo->color_type)
                {
                    // We have a monochrome image.
                case PNG_COLOR_TYPE_GRAY:
                    m_pPngInfo->background.gray = (png_uint_16)((uint32_t)((HFCPtr<HRFAttributeBackground>&)pTag)->GetData());
                    break;
                    // We have a monochrome image with alpha.
                case PNG_COLOR_TYPE_GRAY_ALPHA:
                    m_pPngInfo->background.gray = (png_uint_16)((uint32_t)((HFCPtr<HRFAttributeBackground>&)pTag)->GetData());
                    break;

                    // We have an image with a palette.
                case PNG_COLOR_TYPE_PALETTE:
                    m_pPngInfo->background.index = (Byte) ((HFCPtr<HRFAttributeBackground>&)pTag)->GetData();
                    break;
                    /***************** TO DO ******************
                    // We have a color image (RGB).
                    case PNG_COLOR_TYPE_RGB:
                        break;

                    // We have a color image with an alpha channel (RGB-Alpha)
                    case PNG_COLOR_TYPE_RGB_ALPHA:
                        break;
                    *********************************************/
                }
            }
        // Time Last Modification
        else if (pTag->GetID() == HRFAttributeTimeModification::ATTRIBUTE_ID)
            {
            m_pPngInfo->valid  |= PNG_INFO_tIME;

            sscanf ((char*)((HFCPtr<HRFAttributeTimeModification>&)pTag)->GetData().c_str(),
                    "%04hu:%02hhu:%02hhu %02hhu:%02hhu:%02hhu",
                    &m_pPngInfo->mod_time.year,
                    &m_pPngInfo->mod_time.month,
                    &m_pPngInfo->mod_time.day,
                    &m_pPngInfo->mod_time.hour,
                    &m_pPngInfo->mod_time.minute,
                    &m_pPngInfo->mod_time.second);
            }
        // Title Tag
        else if (pTag->GetID() == HRFAttributeTitle::ATTRIBUTE_ID)
            {
            png_text* pText = new png_text[1];
            pText->key  = "Title";
            pText->text = (char*)((HFCPtr<HRFAttributeTitle>&)pTag)->GetData().c_str();
            pText->compression = PNG_TEXT_COMPRESSION_NONE;
            png_set_text(m_pPngFileStruct, m_pPngInfo, pText, 1);
            delete[] pText;
            }
        // Artist Tag
        else if (pTag->GetID() == HRFAttributeArtist::ATTRIBUTE_ID)
            {
            png_text* pText = new png_text[1];
            pText->key  = "Author";
            pText->text = (char*)((HFCPtr<HRFAttributeArtist>&)pTag)->GetData().c_str();
            pText->compression = PNG_TEXT_COMPRESSION_NONE;
            png_set_text(m_pPngFileStruct, m_pPngInfo, pText, 1);
            delete[] pText;
            }
        // Image Description Tag
        else if (pTag->GetID() == HRFAttributeImageDescription::ATTRIBUTE_ID)
            {
            png_text* pText = new png_text[1];
            pText->key  = "Description";
            pText->text = (char*)((HFCPtr<HRFAttributeImageDescription>&)pTag)->GetData().c_str();
            pText->compression = PNG_TEXT_COMPRESSION_NONE;
            png_set_text(m_pPngFileStruct, m_pPngInfo, pText, 1);
            delete[] pText;
            }
        // Copyright Tag (Copyright: Copyright notice)
        else if (pTag->GetID() == HRFAttributeCopyright::ATTRIBUTE_ID)
            {
            png_text* pText = new png_text[1];
            pText->key  = "Copyright";
            pText->text = (char*)((HFCPtr<HRFAttributeCopyright>&)pTag)->GetData().c_str();
            pText->compression = PNG_TEXT_COMPRESSION_NONE;
            png_set_text(m_pPngFileStruct, m_pPngInfo, pText, 1);
            delete[] pText;
            }
        // Date Time Tag (Creation Time: Time of original image creation)
        else if (pTag->GetID() == HRFAttributeDateTime::ATTRIBUTE_ID)
            {
            png_text* pText = new png_text[1];
            pText->key  = "Creation Time";
            pText->text = (char*)((HFCPtr<HRFAttributeDateTime>&)pTag)->GetData().c_str();
            pText->compression = PNG_TEXT_COMPRESSION_NONE;
            png_set_text(m_pPngFileStruct, m_pPngInfo, pText, 1);
            delete[] pText;
            }
        // Software Tag (Software:  Software used to create the image)
        else if (pTag->GetID() == HRFAttributeSoftware::ATTRIBUTE_ID)
            {
            png_text* pText = new png_text[1];
            pText->key  = "Software";
            pText->text = (char*)((HFCPtr<HRFAttributeSoftware>&)pTag)->GetData().c_str();
            pText->compression = PNG_TEXT_COMPRESSION_NONE;
            png_set_text(m_pPngFileStruct, m_pPngInfo, pText, 1);
            delete[] pText;
            }
        // Legal Disclaimer Tag (Disclaimer:  Legal disclaimer)
        else if (pTag->GetID() == HRFAttributeLegalDisclaimer::ATTRIBUTE_ID)
            {
            png_text* pText = new png_text[1];
            pText->key  = "Disclaimer";
            pText->text = (char*)((HFCPtr<HRFAttributeLegalDisclaimer>&)pTag)->GetData().c_str();
            pText->compression = PNG_TEXT_COMPRESSION_NONE;
            png_set_text(m_pPngFileStruct, m_pPngInfo, pText, 1);
            delete[] pText;
            }
        // Content Warning Tag (Warning: Warning of nature of content)
        else if (pTag->GetID() == HRFAttributeContentWarning::ATTRIBUTE_ID)
            {
            png_text* pText = new png_text[1];
            pText->key  = "Warning";
            pText->text = (char*)((HFCPtr<HRFAttributeContentWarning>&)pTag)->GetData().c_str();
            pText->compression = PNG_TEXT_COMPRESSION_NONE;
            png_set_text(m_pPngFileStruct, m_pPngInfo, pText, 1);
            delete[] pText;
            }
        // Host Computer Tag (Source:  Device used to create the image)
        else if (pTag->GetID() == HRFAttributeHostComputer::ATTRIBUTE_ID)
            {
            png_text* pText = new png_text[1];
            pText->key  = "Source";
            pText->text = (char*)((HFCPtr<HRFAttributeHostComputer>&)pTag)->GetData().c_str();
            pText->compression = PNG_TEXT_COMPRESSION_NONE;
            png_set_text(m_pPngFileStruct, m_pPngInfo, pText, 1);
            delete[] pText;
            }
        // Notes Tag (Comment: Miscellaneous comment; conversion from GIF comment)
        else if (pTag->GetID() == HRFAttributeNotes::ATTRIBUTE_ID)
            {
            png_text* pText = new png_text[1];
            pText->key  = "Comment";
            pText->text = (char*)((HFCPtr<HRFAttributeNotes>&)pTag)->GetData().c_str();
            pText->compression = PNG_TEXT_COMPRESSION_NONE;
            png_set_text(m_pPngFileStruct, m_pPngInfo, pText, 1);
            delete[] pText;
            }
        // X Resolution Tag (pHYs Chunk)
        else if (pTag->GetID() == HRFAttributeXResolution::ATTRIBUTE_ID)
            {
            XResolution = ((HFCPtr<HRFAttributeXResolution>&)pTag)->GetData();
            }
        // Y Resolution Tag (pHYs Chunk)
        else if (pTag->GetID() == HRFAttributeYResolution::ATTRIBUTE_ID)
            {
            YResolution = ((HFCPtr<HRFAttributeYResolution>&)pTag)->GetData();
            }
        // Resolution Unit Tag (pHYs Chunk)
        else if (pTag->GetID() == HRFAttributeResolutionUnit::ATTRIBUTE_ID)
            {
            Unit = ((HFCPtr<HRFAttributeResolutionUnit>&)pTag)->GetData();
            }
        }

    // Set the resolution (in meter)
    if ((XResolution != 0) || (YResolution != 0))
        {
        m_pPngInfo->valid |= PNG_INFO_pHYs;

        // Unit is centimeter
        if (Unit == 3)
            {
            m_pPngInfo->phys_unit_type = 1; // Unit is meter

            m_pPngInfo->x_pixels_per_unit = (uint32_t)(XResolution * 100);
            m_pPngInfo->y_pixels_per_unit = (uint32_t)(YResolution * 100);
            }
        // Unit is inch
        else if (Unit == 2)
            {
            m_pPngInfo->phys_unit_type = 1; // Unit is meter

            m_pPngInfo->x_pixels_per_unit = (uint32_t)((XResolution / 2.54) * 100);
            m_pPngInfo->y_pixels_per_unit = (uint32_t)((YResolution / 2.54) * 100);
            }
        else
            {
            m_pPngInfo->phys_unit_type = 0; // Unkown unit

            m_pPngInfo->x_pixels_per_unit = (uint32_t) XResolution;
            m_pPngInfo->y_pixels_per_unit = (uint32_t) YResolution;
            }
        }

    // Set Histogram Info.
    if (pi_pPage->HasHistogram())
        {
        uint32_t* hist = new uint32_t[pi_pPage->GetHistogram()->GetEntryFrequenciesSize()];
        pi_pPage->GetHistogram()->GetEntryFrequencies(hist);
        png_set_hIST(m_pPngFileStruct, m_pPngInfo, (unsigned short*) hist);
        delete[] hist;
        }

    // Set the interlace handling if need.
    if (pResolutionDescriptor->IsInterlace())
        {
        m_pPngInfo->interlace_type = 1;
        png_set_interlace_handling(m_pPngFileStruct);
        }

    // Lock the sister file
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Write the file header information.
    png_write_info(m_pPngFileStruct, m_pPngInfo);

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFPngFile::GetCapabilities () const
    {
    return (HRFPngCreator::GetInstance()->GetCapabilities());
    }


//-----------------------------------------------------------------------------
// Protected
// Open
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFPngFile::Open()
    {
    // Open the file
    if (!m_IsOpen)
        {
        // Intialise the member
        m_InterlaceFileReaded = false;

        m_pPngFileStruct = 0;
        m_pPngInfo       = 0;
        m_pPngEndInfo    = 0;

        // Open the actual png file specified in the parameters.  The library
        // uses stdio FILE*, so open the file and satisfy the library
        m_pPngFile = HFCBinStream::Instanciate(CreateCombinedURLAndOffset(GetURL(), m_Offset), GetAccessMode());

        ThrowFileExceptionIfError(m_pPngFile, GetURL()->GetURL());

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Get the file read struct && the file read struct && the file end info struct &&
        if((m_pPngFileStruct  = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) &&
           (m_pPngInfo        = png_create_info_struct(m_pPngFileStruct))               &&
           (m_pPngEndInfo     = png_create_info_struct(m_pPngFileStruct)))
            {
            // Set png error handeling
            int JmpRes = setjmp(m_pPngFileStruct->jmpbuf);

            if(JmpRes == 0)
                {
                // Setup the input control to the standard C stream.
                // disable the std io function png_init_io(m_pPngFileStruct, m_pPngFile);
                png_set_read_fn( m_pPngFileStruct,              // png struct
                                 m_pPngFile,                    // IO stream
                                 hmr_png_read_data);            // IO Read function

                // Lock the sister file.
                HFCLockMonitor SisterFileLock (GetLockManager());

                // Read the file information.
                png_read_info(m_pPngFileStruct, m_pPngInfo);
                // *** BoB: OK we read pre-image data info but we do not check if info is present after image data!!!
                //          And it can be the case with some text tags...! To do! ***

                // Unlock the sister file
                SisterFileLock.ReleaseKey();

                // This call has been  added to provide an initialization for the PNG structure
                // when exporting the same png file without closing it between exports.
                // Original case: Exporting 16 bits per pixel png to all supported iTiff crashed.
                CreatePixelTypeFromFile();


                m_IsOpen = true;
                }
            else
                {
                png_destroy_read_struct(&m_pPngFileStruct, &m_pPngInfo, &m_pPngEndInfo);

                throw HFCFileException(HFC_READ_FAULT_EXCEPTION, GetURL()->GetURL());
                }
            }
        else
            {
            png_destroy_read_struct(&m_pPngFileStruct, &m_pPngInfo, &m_pPngEndInfo);

            if (m_pPngFileStruct == 0)
                {
                //         throw HFCFileException(HFC_CANNOT_CREATE_PNG_COMP_INFO_EXCEPTION, GetURL()->GetURL());
                }
            else
                {
                throw HFCException(HFC_MEMORY_EXCEPTION);
                }
            }
        }
    return true;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFPngFile::CreateDescriptors ()
    {
    // Obtain the width and height of the resolution.
    uint32_t Width  = m_pPngInfo->width;
    uint32_t Height = m_pPngInfo->height;

    // Create Page and Resolution Description/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor>     pResolution;
    HFCPtr<HRFPageDescriptor>           pPage;

    HFCPtr<HCDCodec> pZlibCodec(new HCDCodecZlib());
    
    // Find blocks Data Flag
    HRFBlockType blockType = HRFBlockType::LINE;
    uint32_t BlockHeight = 1;                               // IMAGE if very small #TR 202714
    if (m_pPngInfo->interlace_type == PNG_INTERLACE_ADAM7 || Height == 1)
        {
        blockType = HRFBlockType::IMAGE;
        BlockHeight = Height;
        }

    // Tag information
    HPMAttributeSet TagList;
    HFCPtr<HPMGenericAttribute> pTag;

    // Image Gamma Tag
    double gamma = 0.0;
    if (png_get_gAMA(m_pPngFileStruct, m_pPngInfo, &gamma))
        {
        pTag = new HRFAttributeImageGamma(gamma);
        TagList.Set(pTag);
        }

    // Resolution Tag
    if (m_pPngInfo->valid & PNG_INFO_pHYs)
        {
        unsigned short Unit = 1;      // Unkown
        double XResolution = m_pPngInfo->x_pixels_per_unit;
        double YResolution = m_pPngInfo->y_pixels_per_unit;

        if(m_pPngInfo->phys_unit_type == 1)
            {
            // Unit is centimeter
            Unit = 3;

            // Convert for meter to centimeter
            XResolution = XResolution * 0.01;
            YResolution = YResolution * 0.01;
            }

        // XRESOLUTION Tag
        pTag = new HRFAttributeXResolution(XResolution);
        TagList.Set(pTag);

        // YRESOLUTION Tag
        pTag = new HRFAttributeYResolution(YResolution);
        TagList.Set(pTag);

        // Resolution Unit
        pTag = new HRFAttributeResolutionUnit(Unit);
        TagList.Set(pTag);
        }

    // Background Tag
    png_color_16p pBackground;
    if (png_get_bKGD(m_pPngFileStruct, m_pPngInfo, &pBackground))
        {
        switch (m_pPngInfo->color_type)
            {
                // We have a monochrome image.
            case PNG_COLOR_TYPE_GRAY:
                pTag = new HRFAttributeBackground((uint32_t)pBackground->gray);
                TagList.Set(pTag);
                break;
                // We have a monochrome image with alpha.
            case PNG_COLOR_TYPE_GRAY_ALPHA:
                pTag = new HRFAttributeBackground((uint32_t)pBackground->gray);
                TagList.Set(pTag);
                break;

                // We have an image with a palette.
            case PNG_COLOR_TYPE_PALETTE:
                pTag = new HRFAttributeBackground((uint32_t)pBackground->index);
                TagList.Set(pTag);
                break;

                /************************* TO DO **************************
                // We have a color image (RGB).
                case PNG_COLOR_TYPE_RGB:
                    break;

                // We have a color image with an alpha channel (RGB-Alpha)
                case PNG_COLOR_TYPE_RGB_ALPHA:
                    break;
                ***********************************************************/
            }
        }

    // Time Last Modification Tag
    png_timep mod_time;
    if (png_get_tIME(m_pPngFileStruct, m_pPngInfo, &mod_time))
        {
        WPrintfString timeLastModification(L"%4d:%02d:%02d %02d:%02d:%02d", 
                                           mod_time->year,
                                           mod_time->month,
                                           mod_time->day,
                                           mod_time->hour,
                                           mod_time->minute,
                                           mod_time->second);

        pTag = new HRFAttributeTimeModification(timeLastModification);
        TagList.Set(pTag);
        }

    // Text Tag
    png_text* pText;
    int32_t NbText = 0;
    png_get_text(m_pPngFileStruct, m_pPngInfo, &pText, &NbText);

    for(int32_t i=0; i<NbText; i++)
        {
        // Title Tag
        if (!strcmp(pText[i].key, "Title"))
            {
            pTag = new HRFAttributeTitle(WString(pText[i].text,false));
            TagList.Set(pTag);
            }
        // Artist Tag (Author:  Name of image's creator)
        else if (!strcmp(pText[i].key, "Author"))
            {
            pTag = new HRFAttributeArtist(WString(pText[i].text,false));
            TagList.Set(pTag);
            }
        // Image Description Tag (Description: Description of image (possibly long))
        else if (!strcmp(pText[i].key, "Description"))
            {
            pTag = new HRFAttributeImageDescription(WString(pText[i].text,false));
            TagList.Set(pTag);
            }
        // Copyright Tag (Copyright: Copyright notice)
        else if (!strcmp(pText[i].key, "Copyright"))
            {
            pTag = new HRFAttributeCopyright(WString(pText[i].text,false));
            TagList.Set(pTag);
            }
        // Date Time Tag (Creation Time: Time of original image creation)
        else if (!strcmp(pText[i].key, "Creation Time"))
            {
            pTag = new HRFAttributeDateTime(WString(pText[i].text,false));
            TagList.Set(pTag);
            }
        // Software Tag (Software:  Software used to create the image)
        else if (!strcmp(pText[i].key, "Software"))
            {
            pTag = new HRFAttributeSoftware(WString(pText[i].text,false));
            TagList.Set(pTag);
            }
        // Disclaimer Tag (Disclaimer:  Legal disclaimer)
        else if (!strcmp(pText[i].key, "Disclaimer"))
            {
            pTag = new HRFAttributeLegalDisclaimer(WString(pText[i].text,false));
            TagList.Set(pTag);
            }
        // Warning Tag (Warning: Warning of nature of content)
        else if (!strcmp(pText[i].key, "Warning"))
            {
            pTag = new HRFAttributeContentWarning(WString(pText[i].text,false));
            TagList.Set(pTag);
            }
        // Host Computer Tag (Source:  Device used to create the image)
        else if (!strcmp(pText[i].key, "Source"))
            {
            pTag = new HRFAttributeHostComputer(WString(pText[i].text,false));
            TagList.Set(pTag);
            }
        // Notes Tag (Comment: Miscellaneous comment; conversion from GIF comment)
        else if (!strcmp(pText[i].key, "Comment"))
            {
            pTag = new HRFAttributeNotes(WString(pText[i].text,false));
            TagList.Set(pTag);
            }
        }

    // Get Histogram Info
    if (m_pPngInfo->num_palette > 0)
        {
        unsigned short* pHistogram;
        if (png_get_hIST(m_pPngFileStruct, m_pPngInfo, &pHistogram))
            {
            HArrayAutoPtr<uint32_t> pTmpHisto;
            pTmpHisto = new uint32_t[m_pPngInfo->num_palette];
            for (unsigned short i = 0; i < m_pPngInfo->num_palette; i++)
                pTmpHisto[i] = (uint32_t)pHistogram[i];

            m_pHistogram = new HRPHistogram(pTmpHisto, m_pPngInfo->num_palette);
            }
        }

    // Create Resolution Descriptor
    pResolution =  new HRFResolutionDescriptor(
        GetAccessMode(),                                // AccessMode,
        GetCapabilities(),                              // Capabilities,
        1.0,                                            // XResolutionRatio,
        1.0,                                            // YResolutionRatio,
        CreatePixelTypeFromFile(),                      // PixelType,
        pZlibCodec,                                     // Codec,
        HRFBlockAccess::SEQUENTIAL,                     // RStorageAccess,
        HRFBlockAccess::SEQUENTIAL,                     // WStorageAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                       // InterleaveType
        m_pPngInfo->interlace_type == PNG_INTERLACE_ADAM7, // IsInterlace,
        Width,                                          // Width,
        Height,                                         // Height,
        Width,                                          // BlockWidth,
        BlockHeight,                                    // BlockHeight,
        0,                                              // BlocksDataFlag
        blockType);

    pPage = new HRFPageDescriptor (GetAccessMode(),
                                   GetCapabilities(),   // Capabilities,
                                   pResolution,         // ResolutionDescriptor,
                                   0,                   // RepresentativePalette,
                                   m_pHistogram,        // Histogram,
                                   0,                   // Thumbnail,
                                   0,                   // ClipShape,
                                   0,                   // TransfoModel,
                                   0,                   // Filters
                                   &TagList);           // Defined Tag

    m_ListOfPageDescriptor.push_back(pPage);
    }


//-----------------------------------------------------------------------------
// Private
// SavePngFile
// This method saves the file and close it if needed.
//-----------------------------------------------------------------------------
void HRFPngFile::SavePngFile(bool pi_CloseFile)
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {

        if (GetAccessMode().m_HasCreateAccess)
            {
            HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

            // Lock the sister file.
            HFCLockMonitor SisterFileLock (GetLockManager());

            // Free all memory allocated by the read or write process.
            // If the number of writed rows is not equal to the number of rows
            // we can not save the PngInfo structure.
            if (m_pPngFileStruct->num_rows == m_pPngFileStruct->height)
                {
                // Write the rest of the file.
                png_write_end(m_pPngFileStruct, NULL);
                }

            m_pPngFile->Flush();

            // Unlock the sister file.
            SisterFileLock.ReleaseKey();
            }

        if(pi_CloseFile)
            {

            if (m_pPngFileStruct->write_data_fn)
                png_destroy_write_struct(&m_pPngFileStruct, &m_pPngInfo);
            else
                {
                HASSERT(m_pPngEndInfo != 0);
                png_destroy_read_struct(&m_pPngFileStruct, &m_pPngInfo, &m_pPngEndInfo);
                }

            // Close the file.
            // disable std io fclose(m_pPngFile);
            m_pPngFile = 0;
            m_IsOpen = false;
            }


        }
    }

//-----------------------------------------------------------------------------
// Private
// Create
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFPngFile::Create()
    {
    // Initialise the member.
    m_InterlaceFileReaded = false;

    m_pPngFileStruct = 0;
    m_pPngInfo       = 0;
    m_pPngEndInfo    = 0;

    // Open the file.
    m_pPngFile = HFCBinStream::Instanciate(CreateCombinedURLAndOffset(GetURL(), m_Offset), GetAccessMode());

    ThrowFileExceptionIfError(m_pPngFile, GetURL()->GetURL());

    // Create the sister file for file sharing control
    SharingControlCreate();

    // Get the file write struct && the file read struct &&
    if((m_pPngFileStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)) &&
       (m_pPngInfo       = png_create_info_struct(m_pPngFileStruct)))
        {
        // Set png error handeling
        int JmpRes = setjmp(m_pPngFileStruct->jmpbuf);

        if (JmpRes == 0)
            {
            // Setup the input control to the standard C stream.
            // disable the std io function png_init_io(m_pPngFileStruct, m_pPngFile);
            png_set_write_fn( m_pPngFileStruct,     // png struct
                              m_pPngFile,           // IO stream
                              hmr_png_write_data,   // IO Write function
                              hmr_png_flush);       // IO flush function

            m_IsOpen = true;
            }
        else
            {
            png_destroy_write_struct(&m_pPngFileStruct, &m_pPngInfo);

            throw HFCFileException(HFC_WRITE_FAULT_EXCEPTION, GetURL()->GetURL());
            }
        }
    else
        {
        png_destroy_write_struct(&m_pPngFileStruct, &m_pPngInfo);
        // disabled io std fclose(m_pPngFile);

        if (m_pPngFileStruct == 0)
            {
            //       throw HFCFileException(HFC_CANNOT_CREATE_PNG_COMP_INFO_EXCEPTION, GetURL()->GetURL());
            }
        else
            {
            throw HFCException(HFC_MEMORY_EXCEPTION);
            }
        }

    return true;
    }


//-----------------------------------------------------------------------------
// Private
// CreatePixelTypeFromFile
// Create pixel type for the current image
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRFPngFile::CreatePixelTypeFromFile() const
    {

    HFCPtr<HRPPixelType> pPixelType;

    // -------------------------------------------------------------
    // Build the pixel type based on the color space.
    //
    // The PNG file can have the following color space (color type):
    //      PNG_COLOR_TYPE_PALETTE      RGB palette
    //      PNG_COLOR_TYPE_GRAY         monochrome
    //      PNG_COLOR_TYPE_GRAY_ALPHA   monochrome/alpha
    //      PNG_COLOR_TYPE_RGB          red/green/blue
    //      PNG_COLOR_TYPE_RGB_ALPHA    red/green/blue/alpha
    // -------------------------------------------------------------

    //-------------------------------------------------------------------------
    // Update - 30/05/2001 - CedricB
    //
    // Now all the pixel type of png file are supported. But not exactly the
    // same way that they are written in the file.
    //
    // GrayScale :
    //      1 bit               -> V1Gray1
    //      2, 4, 8 and 16 bits -> V8Gray8
    //          * For 16 bits depth : the information is cut to 8 bits depth
    //
    // GrayScale with alpha :
    //      8 and 16 bits depth are translate to V32R8G8B8A8
    //
    // RGB :
    //      8 and 16 bits depth are set to a V24R8G8B8
    //          * For 16 bits depth : the information is cut to 8 bits depth
    //
    // RGBA :
    //      8 and 16 bits depth are set to a V32R8G8B8A8
    //          * For 16 bits depth : tre information is cut to 8 bits depth
    //
    // Palette without transparency :
    //      1 bit   ->  I1R8G8B8
    //      2 bits  ->  V24R8G8B8 (this is because we use the method png_set_expand()
    //      4 bits  ->  I4R8G8B8
    //      8 bits  ->  I8R8G8B8
    //
    // Palette with transparency :
    //      1 bit   ->  I1R8G8B8A8
    //      2 bits  ->  V32R8G8B8A8 (this is because we use the method png_set_expand()
    //      4 bits  ->  I4R8G8B8A8
    //      8 bits  ->  I8R8G8B8A8
    //-------------------------------------------------------------------------

    switch (m_pPngInfo->color_type)
        {
            // We have a monochrome image.
        case PNG_COLOR_TYPE_GRAY:
            switch (m_pPngInfo->pixel_depth)
                {
                case 1:
                    pPixelType = new HRPPixelTypeV1Gray1();
                    break;
                case 2:
                case 4:
                    pPixelType = new HRPPixelTypeV8Gray8();
                    png_set_expand(m_pPngFileStruct);
                    png_set_strip_alpha(m_pPngFileStruct);
                    break;
                case 8:
                    pPixelType = new HRPPixelTypeV8Gray8();
                    break;
                case 16:
                    pPixelType = new HRPPixelTypeV8Gray8();
                    png_set_strip_16(m_pPngFileStruct);
                    break;
                }
            break;

            // We have a color image (RGB).
        case PNG_COLOR_TYPE_RGB:
            pPixelType = new HRPPixelTypeV24R8G8B8();
            if (m_pPngInfo->bit_depth == 0x10)
                png_set_strip_16(m_pPngFileStruct);
            break;

            // We have a color image with an alpha channel (RGB-Alpha)
        case PNG_COLOR_TYPE_RGB_ALPHA:
            pPixelType = new HRPPixelTypeV32R8G8B8A8();
            if (m_pPngInfo->bit_depth == 0x10)
                png_set_strip_16(m_pPngFileStruct);
            break;

            // We have an image with a palette.
        case PNG_COLOR_TYPE_PALETTE:
            {
            int32_t  NombreEntre;
            unsigned char* pTrans;
            if (png_get_tRNS(m_pPngFileStruct, m_pPngInfo, &pTrans, &NombreEntre, NULL))
                {
                switch (m_pPngInfo->pixel_depth)
                    {
                    case 1 :
                        pPixelType = new HRPPixelTypeI1R8G8B8A8();
                        break;
                        // Since the pixel type I2R8G8B8A8 does not exit, we use the
                        // png_set_expand method. It convert the data to V32R8G8B8A8
                    case 2 :
                        pPixelType = new HRPPixelTypeV32R8G8B8A8();
                        png_set_expand(m_pPngFileStruct);
                        break;
                    case 4 :
                        pPixelType = new HRPPixelTypeI4R8G8B8A8();
                        break;
                    case 8 :
                        pPixelType = new HRPPixelTypeI8R8G8B8A8();
                        break;
                    }

                // Set the palette.
                if (m_pPngInfo->pixel_depth != 2)
                    {
                    int32_t             Index;
                    HRPPixelPalette*    pPalette;
                    Byte               Value[4];

                    // Get the palette from the pixel type.
                    pPalette = (HRPPixelPalette*)&(pPixelType->GetPalette());

                    // Copy the PNG palette into the pixel palette.
                    for (Index = 0 ; Index < m_pPngInfo->num_palette ; ++Index)
                        {
                        Value[0] = m_pPngInfo->palette[Index].red;
                        Value[1] = m_pPngInfo->palette[Index].green;
                        Value[2] = m_pPngInfo->palette[Index].blue;
                        if (Index < NombreEntre)
                            Value[3] = m_pPngInfo->trans[Index];
                        else
                            Value[3] = 255;
                        // Add the entry to the pixel palette.
                        pPalette->SetCompositeValue(Index, Value);
                        }
                    }
                }
            else
                {
                switch(m_pPngInfo->pixel_depth)
                    {
                    case 1 :
                        pPixelType = new HRPPixelTypeI1R8G8B8();
                        break;
                        // Since the pixel type I2R8G8B8A8 does not work well, we use
                        // the png_set_expand method. It convert the data to V24R8G8B8
                    case 2 :
                        pPixelType = new HRPPixelTypeV24R8G8B8();
                        png_set_expand(m_pPngFileStruct);
                        break;
                    case 4 :
                        pPixelType = new HRPPixelTypeI4R8G8B8();
                        break;
                    case 8 :
                        pPixelType = new HRPPixelTypeI8R8G8B8();
                        break;
                    }

                if (m_pPngInfo->pixel_depth != 2)
                    {
                    // Set the palette.
                    int32_t             Index;
                    HRPPixelPalette*    pPalette;
                    Byte               Value[3];

                    // Get the palette from the pixel type.
                    pPalette = (HRPPixelPalette*)&(pPixelType->GetPalette());

                    // Copy the PNG palette into the pixel palette.
                    for (Index = 0 ; Index < m_pPngInfo->num_palette ; ++Index)
                        {
                        Value[0] = m_pPngInfo->palette[Index].red;
                        Value[1] = m_pPngInfo->palette[Index].green;
                        Value[2] = m_pPngInfo->palette[Index].blue;

                        // Add the entry to the pixel palette.
                        pPalette->SetCompositeValue(Index, Value);
                        }
                    }
                }
            break;
            }
        case PNG_COLOR_TYPE_GRAY_ALPHA :
            pPixelType = new HRPPixelTypeV32R8G8B8A8();
            if (m_pPngInfo->bit_depth == 16)
                png_set_strip_16(m_pPngFileStruct);
            png_set_gray_to_rgb(m_pPngFileStruct);
            break;
        }
    return (pPixelType);
    }

//-----------------------------------------------------------------------------
// Private
// SetPixelTypeToPage
// Find the pixel type and set it to the page
//-----------------------------------------------------------------------------
void HRFPngFile::SetPixelTypeToPage(HFCPtr<HRFResolutionDescriptor> pi_pResolutionDescriptor)
    {
    HPRECONDITION(pi_pResolutionDescriptor != 0);

    if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
        HRPPixelTypeV24R8G8B8::CLASS_ID)
        {
        m_pPngInfo->bit_depth      = 8;
        m_pPngInfo->sig_bit.red    = 8;
        m_pPngInfo->sig_bit.green  = 8;
        m_pPngInfo->sig_bit.blue   = 8;
        m_pPngInfo->color_type     = PNG_COLOR_TYPE_RGB;

        }
    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
             HRPPixelTypeV32R8G8B8A8::CLASS_ID)
        {
        m_pPngInfo->bit_depth      = 8;
        m_pPngInfo->sig_bit.red    = 8;
        m_pPngInfo->sig_bit.green  = 8;
        m_pPngInfo->sig_bit.blue   = 8;
        m_pPngInfo->sig_bit.alpha  = 8;
        m_pPngInfo->color_type     = PNG_COLOR_TYPE_RGB_ALPHA;

        }
    // Palette Pixel Type
    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
             HRPPixelTypeI1R8G8B8::CLASS_ID)
        {
        Byte*                 pPaletteValue;
        const HRPPixelPalette& rPalette = pi_pResolutionDescriptor->GetPixelType()->GetPalette();

        m_pPngInfo->valid        |= PNG_INFO_PLTE;
        m_pPngInfo->bit_depth     = 1;
        m_pPngInfo->num_palette   = 2;
        m_pPngInfo->color_type    = PNG_COLOR_TYPE_PALETTE;

        // Memory Leaks
        //  The function png_set_PLTE realloc the field palette, we must use a temporary variable now.
        png_color aPalette[2];

        for (uint32_t Index=0 ; Index<2 ; Index++)
            {
            // Check if we have an entry for the palette entry, if not
            // we initialise to 0.
            if (Index < rPalette.CountUsedEntries())
                {
                pPaletteValue = (Byte*)rPalette.GetCompositeValue(Index);
                aPalette[Index].red   = pPaletteValue[0];
                aPalette[Index].green = pPaletteValue[1];
                aPalette[Index].blue  = pPaletteValue[2];
                }
            else
                {
                aPalette[Index].red   = 0;
                aPalette[Index].green = 0;
                aPalette[Index].blue  = 0;
                }
            }

        png_set_PLTE(m_pPngFileStruct,
                     m_pPngInfo,
                     aPalette,
                     m_pPngInfo->num_palette);
        }
    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
             HRPPixelTypeI2R8G8B8::CLASS_ID)
        {
        Byte*                 pPaletteValue;
        const HRPPixelPalette& rPalette = pi_pResolutionDescriptor->GetPixelType()->GetPalette();

        m_pPngInfo->bit_depth     = 2;
        m_pPngInfo->num_palette   = 4;
        m_pPngInfo->color_type    = PNG_COLOR_TYPE_PALETTE;

        // Memory Leaks
        //  The function png_set_PLTE realloc the field palette, we must use a temporary variable now.
        png_color aPalette[4];

        for (uint32_t Index=0 ; Index<4 ; Index++)
            {
            // Check if we have an entry for the palette entry, if not
            // we initialise to 0.
            if (Index < rPalette.CountUsedEntries())
                {
                pPaletteValue = (Byte*)rPalette.GetCompositeValue(Index);
                aPalette[Index].red   = pPaletteValue[0];
                aPalette[Index].green = pPaletteValue[1];
                aPalette[Index].blue  = pPaletteValue[2];

                }
            else
                {
                aPalette[Index].red   = 0;
                aPalette[Index].green = 0;
                aPalette[Index].blue  = 0;
                }
            }

        png_set_PLTE(m_pPngFileStruct,
                     m_pPngInfo,
                     aPalette,
                     m_pPngInfo->num_palette);
        }
    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
             HRPPixelTypeI4R8G8B8::CLASS_ID)
        {
        Byte*                 pPaletteValue;
        const HRPPixelPalette& rPalette = pi_pResolutionDescriptor->GetPixelType()->GetPalette();

        m_pPngInfo->bit_depth     = 4;
        m_pPngInfo->num_palette   = 16;
        m_pPngInfo->color_type    = PNG_COLOR_TYPE_PALETTE;

        // Memory Leaks
        //  The function png_set_PLTE realloc the field palette, we must use a temporary variable now.
        png_color aPalette[16];

        for (uint32_t Index=0 ; Index<16 ; Index++)
            {
            // Check if we have an entry for the palette entry, if not
            // we initialise to 0.
            if (Index < rPalette.CountUsedEntries())
                {
                pPaletteValue = (Byte*)rPalette.GetCompositeValue(Index);
                aPalette[Index].red   = pPaletteValue[0];
                aPalette[Index].green = pPaletteValue[1];
                aPalette[Index].blue  = pPaletteValue[2];
                }
            else
                {
                aPalette[Index].red   = 0;
                aPalette[Index].green = 0;
                aPalette[Index].blue  = 0;
                }
            }
        png_set_PLTE(m_pPngFileStruct,
                     m_pPngInfo,
                     aPalette,
                     m_pPngInfo->num_palette);
        }

    else if(pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
            HRPPixelTypeI8R8G8B8::CLASS_ID)
        {
        Byte*                 pPaletteValue;
        const HRPPixelPalette& rPalette = pi_pResolutionDescriptor->GetPixelType()->GetPalette();

        m_pPngInfo->bit_depth     = 8;
        m_pPngInfo->num_palette   = 256;
        m_pPngInfo->color_type    = PNG_COLOR_TYPE_PALETTE;

        // Memory Leaks
        //  The function png_set_PLTE realloc the field palette, we must use a temporary variable now.
        png_color aPalette[256];

        for (uint32_t Index=0 ; Index<256 ; Index++)
            {
            // Check if we have an entry for the palette entry, if not
            // we initialise to 0.
            if (Index < rPalette.CountUsedEntries())
                {
                pPaletteValue = (Byte*)rPalette.GetCompositeValue(Index);
                aPalette[Index].red   = pPaletteValue[0];
                aPalette[Index].green = pPaletteValue[1];
                aPalette[Index].blue  = pPaletteValue[2];
                }
            else
                {
                aPalette[Index].red   = 0;
                aPalette[Index].green = 0;
                aPalette[Index].blue  = 0;
                }
            }
        png_set_PLTE(m_pPngFileStruct,
                     m_pPngInfo,
                     aPalette,
                     m_pPngInfo->num_palette);
        }

    // Palette Pixel Type (with Alpha).
    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
             HRPPixelTypeI1R8G8B8A8::CLASS_ID)
        {
        Byte*                 pPaletteValue;
        const HRPPixelPalette& rPalette = pi_pResolutionDescriptor->GetPixelType()->GetPalette();

        m_pPngInfo->bit_depth     = 1;
        m_pPngInfo->num_palette   = 2;
        m_pPngInfo->num_trans     = 2;
        m_pPngInfo->color_type    = PNG_COLOR_TYPE_PALETTE;

        // Memory Leaks
        //  The function png_set_PLTE realloc the field palette, we must use a temporary variable now.
        png_color aPalette[2];
        Byte    aTrans[2];

        for (uint32_t Index=0 ; Index<2 ; Index++)
            {
            // Check if we have an entry for the palette entry, if not
            // we initialise to 0.
            if (Index < rPalette.CountUsedEntries())
                {
                pPaletteValue = (Byte*)rPalette.GetCompositeValue(Index);
                aPalette[Index].red   = pPaletteValue[0];
                aPalette[Index].green = pPaletteValue[1];
                aPalette[Index].blue  = pPaletteValue[2];
                aTrans[Index]         = pPaletteValue[3];
                }
            else
                {
                aPalette[Index].red   = 0;
                aPalette[Index].green = 0;
                aPalette[Index].blue  = 0;
                aTrans[Index]         = 0;
                }
            }
        png_set_PLTE(m_pPngFileStruct,
                     m_pPngInfo,
                     aPalette,
                     m_pPngInfo->num_palette);


        png_set_tRNS(m_pPngFileStruct,
                     m_pPngInfo,
                     aTrans,
                     m_pPngInfo->num_trans,
                     NULL);
        }
    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
             HRPPixelTypeI4R8G8B8A8::CLASS_ID)
        {
        Byte*  pPaletteValue;
        const HRPPixelPalette& rPalette = pi_pResolutionDescriptor->GetPixelType()->GetPalette();

        m_pPngInfo->bit_depth     = 4;
        m_pPngInfo->num_palette   = 16;
        m_pPngInfo->num_trans     = 16;
        m_pPngInfo->color_type    = PNG_COLOR_TYPE_PALETTE;

        // Memory Leaks
        //  The function png_set_PLTE realloc the field palette, we must use a temporary variable now.
        png_color aPalette[16];
        Byte    aTrans[16];

        for (uint32_t Index=0 ; Index<16 ; Index++)
            {
            // Check if we have an entry for the palette entry, if not
            // we initialise to 0.
            if (Index < rPalette.CountUsedEntries())
                {
                pPaletteValue = (Byte*)rPalette.GetCompositeValue(Index);
                aPalette[Index].red   = pPaletteValue[0];
                aPalette[Index].green = pPaletteValue[1];
                aPalette[Index].blue  = pPaletteValue[2];
                aTrans[Index]         = pPaletteValue[3];
                }
            else
                {
                aPalette[Index].red   = 0;
                aPalette[Index].green = 0;
                aPalette[Index].blue  = 0;
                aTrans[Index]         = 0;
                }
            }
        png_set_PLTE(m_pPngFileStruct,
                     m_pPngInfo,
                     aPalette,
                     m_pPngInfo->num_palette);


        png_set_tRNS(m_pPngFileStruct,
                     m_pPngInfo,
                     aTrans,
                     m_pPngInfo->num_trans,
                     NULL);
        }

    else if(pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
            HRPPixelTypeI8R8G8B8A8::CLASS_ID)
        {
        Byte*  pPaletteValue;
        const HRPPixelPalette& rPalette = pi_pResolutionDescriptor->GetPixelType()->GetPalette();

        m_pPngInfo->bit_depth     = 8;
        m_pPngInfo->num_palette   = 256;
        m_pPngInfo->num_trans     = 256;
        m_pPngInfo->color_type    = PNG_COLOR_TYPE_PALETTE;

        // Memory Leaks
        //  The function png_set_PLTE realloc the field palette, we must use a temporary variable now.
        png_color aPalette[256];
        Byte    aTrans[256];

        for (uint32_t Index=0 ; Index<256 ; Index++)
            {
            // Check if we have an entry for the palette entry, if not
            // we initialise to 0.
            if (Index < rPalette.CountUsedEntries())
                {
                pPaletteValue = (Byte*)rPalette.GetCompositeValue(Index);
                aPalette[Index].red   = pPaletteValue[0];
                aPalette[Index].green = pPaletteValue[1];
                aPalette[Index].blue  = pPaletteValue[2];
                aTrans[Index]         = pPaletteValue[3];
                }
            else
                {
                aPalette[Index].red   = 0;
                aPalette[Index].green = 0;
                aPalette[Index].blue  = 0;
                aTrans[Index]         = 0;
                }
            }
        png_set_PLTE(m_pPngFileStruct,
                     m_pPngInfo,
                     aPalette,
                     m_pPngInfo->num_palette);


        png_set_tRNS(m_pPngFileStruct,
                     m_pPngInfo,
                     aTrans,
                     m_pPngInfo->num_trans,
                     NULL);
        }
    else if(pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
            HRPPixelTypeV8Gray8::CLASS_ID)
        {
        m_pPngInfo->bit_depth     = 8;
        m_pPngInfo->color_type    = PNG_COLOR_TYPE_GRAY;
        }
    else if(pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
            HRPPixelTypeV1Gray1::CLASS_ID)
        {
        m_pPngInfo->bit_depth     = 1;
        m_pPngInfo->color_type    = PNG_COLOR_TYPE_GRAY;
        }

//*************************** TO DO ************************************
//    else if(pResolutionDescriptor->GetPixelType()->GetClassID() ==
//            HRPPixelTypeV16Gray8Alpha8)
//    else if(pResolutionDescriptor->GetPixelType()->GetClassID() ==
//            HRPPixelTypeV16Gray16)
//    else if(pResolutionDescriptor->GetPixelType()->GetClassID() ==
//            HRPPixelTypeV32Gray16Alpha16)
//    else if(pResolutionDescriptor->GetPixelType()->GetClassID() ==
//            HRPPixelTypeV48R16G16B16)
//    else if(pResolutionDescriptor->GetPixelType()->GetClassID() ==
//            HRPPixelTypeV64R16G16B16A16)
//**********************************************************************

    }


//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFPngFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

//-----------------------------------------------------------------------------
// Public
// GetMemoryFilePtr
// Return the file pointer if the PNG was in memory, 0 otherwise
//-----------------------------------------------------------------------------
const HFCMemoryBinStream* HRFPngFile::GetMemoryFilePtr() const
    {
    HPOSTCONDITION(m_pPngFile != 0);

    if (m_pPngFile->IsCompatibleWith(HFCMemoryBinStream::CLASS_ID))
        return (HFCMemoryBinStream*)m_pPngFile.get();
    else
        return 0;
    }

//-----------------------------------------------------------------------------
// protected
// GetFilePtr   - Get the PNG file pointer.
//-----------------------------------------------------------------------------
HFCBinStream* HRFPngFile::GetFilePtr  ()
    {
    return (m_pPngFile);
    }