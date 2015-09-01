//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIntergraphC30File.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIntergraphC30File
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFIntergraphC30File.h>
#include <Imagepp/all/h/HRFIntergraphColorFile.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/HRPPixelPalette.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HCDCodecIJG.h>

#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <ImagePP/all/h/HFCException.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//-----------------------------------------------------------------------------
// HRFIntergraphC30BlockCapabilities
//-----------------------------------------------------------------------------
class HRFIntergraphC30BlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFIntergraphC30BlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add(new HRFLineCapability(HFC_READ_ONLY,                // HFC_READ_WRITE_CREATE,        // AccessMode
                                  LONG_MAX,                     // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL)); // BlockAccess

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_ONLY,          // AccessMode
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
// HRFIntergraphC30CodecCapabilities
//-----------------------------------------------------------------------------
class HRFIntergraphC30CodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFIntergraphC30CodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec JPEG
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFIntergraphC30BlockCapabilities()));
        }
    };



//-----------------------------------------------------------------------------
// HRFIntergraphC30Capabilities
//-----------------------------------------------------------------------------
HRFIntergraphC30Capabilities::HRFIntergraphC30Capabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV8Gray8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV8Gray8;
    pPixelTypeV8Gray8 = new HRFPixelTypeCapability(HFC_READ_ONLY,
                                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                                   new HRFIntergraphC30CodecCapabilities());
    pPixelTypeV8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV8Gray8);

    // Transfo Model
    // Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DProjective::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_RIGHT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_RIGHT_VERTICAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // MultiResolution Capability
    // NOTE : Some Intergraph file have very bad sub-resolution quality.
    //        So that we do is: we support multi resolution file (we can read file with
    //        multi-res), but do not take any account of the sub-resolution data.
    //        Which mean that we re-calculate the sub-resolution and store it in a cache file
    HFCPtr<HRFCapability> pMultiResolutionCapability = new HRFMultiResolutionCapability(
        HFC_READ_ONLY, // AccessMode,
        true,           // SinglePixelType,
        true,           // SingleBlockType,
        true,           // ArbitaryXRatio,
        true);          // ArbitaryYRatio);
    Add(pMultiResolutionCapability);

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeYResolution(0.0)));
    }

HFC_IMPLEMENT_SINGLETON(HRFIntergraphC30Creator)

//-----------------------------------------------------------------------------
// Creator
// TIFF capabilities instance member definition
//-----------------------------------------------------------------------------

HRFIntergraphC30Creator::HRFIntergraphC30Creator()
    : HRFIntergraphFile::Creator(HRFIntergraphC30File::CLASS_ID)
    {
    // TIFF capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

WString HRFIntergraphC30Creator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_C30()); //Intergraph C30
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

WString HRFIntergraphC30Creator::GetExtensions() const
    {
    return WString(L"*.c30");
    }

//-----------------------------------------------------------------------------
// allow to Open an image file
//-----------------------------------------------------------------------------

HFCPtr<HRFRasterFile> HRFIntergraphC30Creator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                      HFCAccessMode         pi_AccessMode,
                                                      uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFIntergraphC30File(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
// At this level we cant know if the sub-format is supported, so redefine it
// On each descended sub-format.
//-----------------------------------------------------------------------------

bool HRFIntergraphC30Creator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool   Result = false;
    HAutoPtr<HFCBinStream> pFile;
    unsigned short HeaderTypeCode;
    unsigned short DataTypeCode;
    unsigned short WordToFollow;
    uint32_t HeaderLen;

    (const_cast<HRFIntergraphC30Creator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Open the Cot30 File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        // Check if the file was a valid Intergraph Cot30...
        pFile->SeekToBegin();
        if (pFile->Read(&HeaderTypeCode, sizeof(unsigned short)) != sizeof(unsigned short))
            goto WRAPUP;

        if (HeaderTypeCode == 0x0908)
            {
            if (pFile->Read(&WordToFollow, sizeof(unsigned short)) != sizeof(unsigned short))
                goto WRAPUP;

            if (pFile->Read(&DataTypeCode, sizeof(unsigned short)) != sizeof(unsigned short))
                goto WRAPUP;

            if ((DataTypeCode == 30) && (pi_Offset || !IsMultiPage(*pFile, (WordToFollow + 2)/256)) )
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

                        if ((DataTypeCode == 30) && (pi_Offset || !IsMultiPage(*pFile, (WordToFollow + 2)/256)) )
                            Result = true;
                        }
                    }
                }
            }
        }

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFIntergraphC30Creator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFIntergraphC30Creator*>(this))->m_pSharingControl = 0;

    return Result;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of an Intergraph Cot30 file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFIntergraphC30Creator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFIntergraphC30Capabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFIntergraphC30File::HRFIntergraphC30File(const HFCPtr<HFCURL>& pi_rURL,
                                           HFCAccessMode         pi_AccessMode,
                                           uint64_t             pi_Offset)
    : HRFIntergraphColorFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    SetDatatypeCode(30);
    SetBitPerPixel (8);

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
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
HRFIntergraphC30File::HRFIntergraphC30File(const HFCPtr<HFCURL>& pi_rURL,
                                           HFCAccessMode         pi_AccessMode,
                                           uint64_t             pi_Offset,
                                           bool                 pi_DontOpenFile)
    : HRFIntergraphColorFile(pi_rURL, pi_AccessMode, pi_Offset, pi_DontOpenFile)
    {
    SetDatatypeCode(30);
    SetBitPerPixel (8);

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }
    }


//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors - Called in read mode...
//-----------------------------------------------------------------------------
void HRFIntergraphC30File::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);

    // Create the descriptor for each resolution of each page
    // for (UInt32 Page=0; Page < CalcNumberOfPage(); Page++)
        {
        // Initialise some members and read file header Read
        InitOpenedFile(0);

        // TranfoModel
        //HFCPtr<HGF2DTransfoModel> pTransfoModel = &GetTransfoModel();
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
                GetResolution(Resolution),        // XResolutionRatio,
                GetResolution(Resolution),        // YResolutionRatio,
                GetPixelType(),                     // PixelType,
                new HCDCodecIJG(),                  // Codec,
                BlockAccess,                            // RBlockAccess,
                BlockAccess,                            // WBlockAccess,
                GetScanlineOrientation(),           // ScanLineOrientation,
                HRFInterleaveType::PIXEL,               // InterleaveType
                false,                                  // IsInterlace,
                GetWidth (Resolution),            // Width,
                GetHeight(Resolution),            // Height,
                BlockWidth,                             // TileWidth,
                BlockHeight,                            // TileHeight,
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
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFIntergraphC30File::GetFileCurrentSize() const
    {
    return HRFIntergraphFile::GetFileCurrentSize();
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFIntergraphC30File::GetCapabilities () const
    {
    return HRFIntergraphC30Creator::GetInstance()->GetCapabilities();
    }
