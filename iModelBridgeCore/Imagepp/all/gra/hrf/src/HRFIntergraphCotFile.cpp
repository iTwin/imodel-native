//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIntergraphCotFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIntergraphCotFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFIntergraphCotFile.h>
#include <Imagepp/all/h/HRFIntergraphColorFile.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/HRPPixelPalette.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>

#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//-----------------------------------------------------------------------------
// HRFCotBlockCapabilities
//-----------------------------------------------------------------------------
class HRFCotBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFCotBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,        // AccessMode
                                  LONG_MAX,                     // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL)); // BlockAccess

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_WRITE_CREATE,  // AccessMode
                                  LONG_MAX,               // MaxSizeInBytes
                                  32,                     // MinWidth
                                  4096,                   // MaxWidth
                                  32,                     // WidthIncrement
                                  32,                     // MinHeight
                                  4096,                   // MaxHeight
                                  32));                   // HeightIncrement
        }
    };

//-----------------------------------------------------------------------------
// HRFCotCodecCapabilities
//-----------------------------------------------------------------------------
class HRFCotCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFCotCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFCotBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFCotCapabilities
//-----------------------------------------------------------------------------
HRFCotCapabilities::HRFCotCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV8Gray8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV8Gray8;
    pPixelTypeV8Gray8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                                   new HRFCotCodecCapabilities());
    pPixelTypeV8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV8Gray8);


    // PixelTypeI8R8G8B8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI8R8G8B8;
    pPixelTypeI8R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                    HRPPixelTypeI8R8G8B8::CLASS_ID,
                                                    new HRFCotCodecCapabilities());
    pPixelTypeI8R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI8R8G8B8);


    // Transfo Model
    // Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DProjective::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DIdentity::CLASS_ID));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::UPPER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::UPPER_RIGHT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::LOWER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::LOWER_RIGHT_VERTICAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // MultiResolution Capability
    // NOTE : Some Intergraph file have very bad sub-resolution quality.
    //        So that we do is: we support multi resolution file (we can read file with
    //        multi-res), but do not take any account of the sub-resolution data.
    //        Which mean that we re-calculate the sub-resolution and store it in a cache file
    HFCPtr<HRFCapability> pMultiResolutionCapability = new HRFMultiResolutionCapability(
        HFC_READ_WRITE_CREATE, // AccessMode,
        true,                  // SinglePixelType,
        true,                  // SingleBlockType,
        true,                  // ArbitaryXRatio,
        true);                 // ArbitaryYRatio);
    Add(pMultiResolutionCapability);

    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeYResolution(0.0)));
    }

HFC_IMPLEMENT_SINGLETON(HRFIntergraphCotCreator)

//-----------------------------------------------------------------------------
// Creator
// TIFF capabilities instance member definition
//-----------------------------------------------------------------------------

HRFIntergraphCotCreator::HRFIntergraphCotCreator()
    : HRFIntergraphFile::Creator(HRFIntergraphCotFile::CLASS_ID)
    {
    // TIFF capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

WString HRFIntergraphCotCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_COT()); // Intergraph Cot File Format
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

WString HRFIntergraphCotCreator::GetExtensions() const
    {
    return WString(L"*.cot");
    }

//-----------------------------------------------------------------------------
// allow to Open an image file
//-----------------------------------------------------------------------------

HFCPtr<HRFRasterFile> HRFIntergraphCotCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                      HFCAccessMode         pi_AccessMode,
                                                      uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFIntergraphCotFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
// At this level we cant know if the sub-format is supported, so redefine it
// On each descended sub-format.
//-----------------------------------------------------------------------------

bool HRFIntergraphCotCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool   Result = false;
    HAutoPtr<HFCBinStream> pFile;
    unsigned short HeaderTypeCode;
    unsigned short DataTypeCode;
    unsigned short WordToFollow;
    uint32_t HeaderLen;

    (const_cast<HRFIntergraphCotCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the COT File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        // Check if the file was a valid Intergraph COT...
        pFile->SeekToBegin();
        if (pFile->Read(&HeaderTypeCode, sizeof(unsigned short)) != sizeof(unsigned short))
            goto WRAPUP;

        if (HeaderTypeCode == 0x0908)
            {
            if (pFile->Read(&WordToFollow, sizeof(unsigned short)) != sizeof(unsigned short))
                goto WRAPUP;

            if (pFile->Read(&DataTypeCode, sizeof(unsigned short)) != sizeof(unsigned short))
                goto WRAPUP;

            if ((DataTypeCode == 2) && (pi_Offset || !IsMultiPage(*pFile, (WordToFollow + 2)/256)) )
                Result = true;
            else
                {
                if (DataTypeCode == 65)
                    {
                    if ((((WordToFollow + 2) * 2) % 512) == 0)
                        {
                        HeaderLen = ((WordToFollow + 2) /256) * 512;
                        HeaderLen += 18;
                        pFile->SeekToPos(HeaderLen);
                        if (pFile->Read(&DataTypeCode, sizeof(unsigned short)) != sizeof(unsigned short))
                            goto WRAPUP;

                        if ((DataTypeCode == 2) && (pi_Offset || !IsMultiPage(*pFile, (WordToFollow + 2)/256)) )
                            Result = true;
                        }
                    }
                }
            }
        }

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFIntergraphCotCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFIntergraphCotCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of an Intergraph Cot file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFIntergraphCotCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFCotCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFIntergraphCotFile::HRFIntergraphCotFile(const HFCPtr<HFCURL>& pi_rURL,
                                           HFCAccessMode         pi_AccessMode,
                                           uint64_t             pi_Offset)
    : HRFIntergraphColorFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    SetDatatypeCode(2);
    SetBitPerPixel (8);

    if (GetAccessMode().m_HasCreateAccess)
        {
        // Create a new file
        Create();
        }
    else
        {
        // Open an existent file
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
HRFIntergraphCotFile::HRFIntergraphCotFile(const HFCPtr<HFCURL>& pi_rURL,
                                           HFCAccessMode         pi_AccessMode,
                                           uint64_t             pi_Offset,
                                           bool                 pi_DontOpenFile)
    : HRFIntergraphColorFile(pi_rURL, pi_AccessMode, pi_Offset, pi_DontOpenFile)
    {
    SetDatatypeCode(2);
    SetBitPerPixel (8);
    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize - Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFIntergraphCotFile::GetFileCurrentSize() const
    {
    return HRFIntergraphFile::GetFileCurrentSize();
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors - Called in read mode...
//-----------------------------------------------------------------------------
void HRFIntergraphCotFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);

    // Create the descriptor for each resolution of each page
    // for (UInt32 Page=0; Page < CalcNumberOfPage(); Page++)
        {
        // Initialise some members and read file header Read
        InitOpenedFile(0);

        // TranfoModel
        // HFCPtr<HGF2DTransfoModel> pTransfoModel = &GetTransfoModel();
        GetTransfoModel();

        // Instantiation of Resolution descriptor
        HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
        for (unsigned short Resolution=0; Resolution < CountSubResolution() + 1; Resolution++)
            {
            uint32_t BlockWidth  = GetWidth(Resolution);
            uint32_t BlockHeight = 1;

            // BlockType
            HRFBlockAccess  BlockAccess;
            HRFBlockType    BlockType;

            if (HasTileAccess(Resolution))
                {
                BlockAccess    = HRFBlockAccess::RANDOM;
                BlockWidth     = GetTileWidth (Resolution);
                BlockHeight    = GetTileHeight(Resolution);
                BlockType      = HRFBlockType::TILE;
                }
            else
                {
                BlockAccess = HRFBlockAccess::SEQUENTIAL;
                BlockType   = HRFBlockType::LINE;
                }

            // Obtain Resolution Information
            // resolution dimension
            HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
                GetAccessMode(),                        // AccessMode,
                GetCapabilities(),                      // Capabilities,
                GetResolution(Resolution),              // XResolutionRatio,
                GetResolution(Resolution),              // YResolutionRatio,
                GetPixelType(),                         // PixelType,
                new HCDCodecIdentity(),                 // Codec
                BlockAccess,                            // RBlockAccess,
                BlockAccess,                            // WBlockAccess,
                GetScanlineOrientation(),               // ScanLineOrientation
                HRFInterleaveType::PIXEL,               // InterleaveType
                false,                                  // IsInterlace,
                GetWidth (Resolution),                  // Width,
                GetHeight(Resolution),                  // Height,
                BlockWidth,                             // BlockWidth,
                BlockHeight,                            // BlockHeight,
                0,                                      // BlockDataFlags
                BlockType);                             // Block type

            ListOfResolutionDescriptor.push_back(pResolution);
            }

        // Tag
        HPMAttributeSet TagList;
        HFCPtr<HPMGenericAttribute> pTag;

        if (m_IntergraphHeader.IBlock1.drs != 0)
            {
            unsigned short UnitValue;
            double Resolution;

            if (m_IntergraphHeader.IBlock1.drs < 0)
                {
                // Unit is dpi.
                UnitValue  = 2; // Inch
                Resolution = -m_IntergraphHeader.IBlock1.drs;
                }
            else
                {
                // Unit is micron (10e-6 meter).
                UnitValue = 3;

                // Convert from micron to cm (m_IntergraphHeader.IBlock1.drs represante the distance
                // between pixel in real world)

                Resolution = 10000 / m_IntergraphHeader.IBlock1.drs;
                }

            // RESOLUTIONUNIT Tag
            pTag = new HRFAttributeResolutionUnit(UnitValue);
            TagList.Set(pTag);

            // XRESOLUTION Tag
            pTag = new HRFAttributeXResolution(Resolution);
            TagList.Set(pTag);

            // YRESOLUTION Tag
            pTag = new HRFAttributeYResolution(Resolution);
            TagList.Set(pTag);
            }

        HFCPtr<HRFPageDescriptor> pPage;
        pPage = new HRFPageDescriptor (GetAccessMode(),
                                       GetCapabilities(),           // Capabilities,
                                       ListOfResolutionDescriptor,  // ResolutionDescriptor,
                                       0,                           // RepresentativePalette,
                                       0,                           // Histogram,
                                       0,                           // Thumbnail,
                                       0,                           // ClipShape,
                                       m_pTransfoModel,             // TransfoModel,
                                       0,                           // Filters
                                       &TagList);                   // Defined Tag

        m_ListOfPageDescriptor.push_back(pPage);
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFIntergraphCotFile::GetCapabilities () const
    {
    return HRFIntergraphCotCreator::GetInstance()->GetCapabilities();
    }
