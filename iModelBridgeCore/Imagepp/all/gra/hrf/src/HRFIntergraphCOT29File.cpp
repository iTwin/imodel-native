//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIntergraphCOT29File.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIntergraphCot29File
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HFCURLFile.h>#include <ImagePP/all/h/HRFIntergraphCOT29File.h>#include <ImagePP/all/h/HRFIntergraphColorFile.h>#include <ImagePP/all/h/HRFRasterFileCapabilities.h>
#include <ImagePP/all/h/HRPPixelPalette.h>#include <ImagePP/all/h/HRPPixelTypeI8R8G8B8.h>#include <ImagePP/all/h/HRPPixelTypeV8Gray8.h>#include <ImagePP/all/h/HCDCodecRLE8.h>
#include <ImagePP/all/h/HGF2DTransfoModel.h>#include <ImagePP/all/h/HGF2DIdentity.h>#include <ImagePP/all/h/HGF2DSimilitude.h>#include <ImagePP/all/h/HGF2DTranslation.h>#include <ImagePP/all/h/HGF2DProjective.h>#include <ImagePP/all/h/HGF2DAffine.h>#include <ImagePP/all/h/HGF2DStretch.h>#include <ImagePP/all/h/HRFUtility.h>
#include <ImagePP/all/h/ImagePPMessages.xliff.h>
//-----------------------------------------------------------------------------
// HRFCot29BlockCapabilities
//-----------------------------------------------------------------------------
class HRFCot29BlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFCot29BlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,        // AccessMode
                                  INT32_MAX,                     // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL)); // BlockAccess

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_WRITE_CREATE,  // AccessMode
                                  INT32_MAX,               // MaxSizeInBytes
                                  32,                     // MinWidth
                                  4096,                   // MaxWidth
                                  32,                     // WidthIncrement
                                  32,                     // MinHeight
                                  4096,                   // MaxHeight
                                  32));                   // HeightIncrement
        }
    };

//-----------------------------------------------------------------------------
// HRFCot29CodecCapabilities
//-----------------------------------------------------------------------------
class HRFCot29CodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFCot29CodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec RLE8
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecRLE8::CLASS_ID,
                                   new HRFCot29BlockCapabilities()));
        }
    };



//-----------------------------------------------------------------------------
// HRFCot29Capabilities
//-----------------------------------------------------------------------------
HRFCot29Capabilities::HRFCot29Capabilities()
    : HRFRasterFileCapabilities()
    {

    // PixelTypeI8R8G8B8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeI8R8G8B8;
    pPixelTypeI8R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                    HRPPixelTypeI8R8G8B8::CLASS_ID,
                                                    new HRFCot29CodecCapabilities());
    pPixelTypeI8R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeI8R8G8B8);

    // PixelTypeV8Gray8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV8Gray8;
    pPixelTypeV8Gray8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                                   new HRFCot29CodecCapabilities());
    pPixelTypeV8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV8Gray8);

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

HFC_IMPLEMENT_SINGLETON(HRFIntergraphCot29Creator)

//-----------------------------------------------------------------------------
// Creator
// TIFF capabilities instance member definition
//-----------------------------------------------------------------------------

HRFIntergraphCot29Creator::HRFIntergraphCot29Creator()
    : HRFIntergraphFile::Creator(HRFIntergraphCot29File::CLASS_ID)
    {
    // TIFF capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

Utf8String HRFIntergraphCot29Creator::GetLabel() const
    {
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_C29()); // Intergraph 29
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

Utf8String HRFIntergraphCot29Creator::GetExtensions() const
    {
    return Utf8String("*.c29;*.t29");
    }

//-----------------------------------------------------------------------------
// allow to Open an image file
//-----------------------------------------------------------------------------

HFCPtr<HRFRasterFile> HRFIntergraphCot29Creator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                        HFCAccessMode         pi_AccessMode,
                                                        uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFIntergraphCot29File(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
// At this level we cant know if the sub-format is supported, so redefine it
// On each descended sub-format.
//-----------------------------------------------------------------------------

bool HRFIntergraphCot29Creator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                              uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool   Result = false;
    HAutoPtr<HFCBinStream> pFile;
    uint16_t HeaderTypeCode;
    uint16_t DataTypeCode;
    uint16_t WordToFollow;
    uint32_t HeaderLen;

    // Open the Cot29 File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        // Check if the file was a valid Intergraph Cot29...
        pFile->SeekToBegin();
        if (pFile->Read(&HeaderTypeCode, sizeof(uint16_t)) != sizeof(uint16_t))
            goto WRAPUP;

        if (HeaderTypeCode == 0x0908)
            {
            if (pFile->Read(&WordToFollow, sizeof(uint16_t)) != sizeof(uint16_t))
                goto WRAPUP;

            if (pFile->Read(&DataTypeCode, sizeof(uint16_t)) != sizeof(uint16_t))
                goto WRAPUP;

            if ((DataTypeCode == 29) && (pi_Offset || !IsMultiPage(*pFile, (WordToFollow + 2)/256)) )
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
                        if (pFile->Read(&DataTypeCode, sizeof(uint16_t)) != sizeof(uint16_t))
                            goto WRAPUP;

                        if ((DataTypeCode == 29) && (pi_Offset || !IsMultiPage(*pFile, (WordToFollow + 2)/256)) )
                            Result = true;
                        }
                    }
                }
            }
        }

WRAPUP:

    return Result;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of an Intergraph Cot29 file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFIntergraphCot29Creator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFCot29Capabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFIntergraphCot29File::HRFIntergraphCot29File(const HFCPtr<HFCURL>& pi_rURL,
                                               HFCAccessMode         pi_AccessMode,
                                               uint64_t             pi_Offset)
    : HRFIntergraphColorFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    SetDatatypeCode(29);
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
HRFIntergraphCot29File::HRFIntergraphCot29File(const HFCPtr<HFCURL>& pi_rURL,
                                               HFCAccessMode         pi_AccessMode,
                                               uint64_t             pi_Offset,
                                               bool                 pi_DontOpenFile)
    : HRFIntergraphColorFile(pi_rURL, pi_AccessMode, pi_Offset, pi_DontOpenFile)
    {
    SetDatatypeCode(29);
    SetBitPerPixel (8);
    }


//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Return the current size of the file
//-----------------------------------------------------------------------------
uint64_t HRFIntergraphCot29File::GetFileCurrentSize() const
    {
    return HRFIntergraphFile::GetFileCurrentSize();
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors - Called in read mode...
//-----------------------------------------------------------------------------
void HRFIntergraphCot29File::CreateDescriptors()
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
        for (uint16_t Resolution=0; Resolution < CountSubResolution() + 1; Resolution++)
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
                new HCDCodecRLE8(),                 // Codec,
                BlockAccess,                            // RBlockAccess,
                BlockAccess,                            // WBlockAccess,
                GetScanlineOrientation(),           // ScanLineOrientation,
                HRFInterleaveType::PIXEL,               // InterleaveType
                false,                                  // IsInterlace,
                GetWidth(Resolution),             // Width,
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
            uint16_t UnitValue;
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
                                       &TagList);                   // TagList

        m_ListOfPageDescriptor.push_back(pPage);
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFIntergraphCot29File::GetCapabilities () const
    {
    return HRFIntergraphCot29Creator::GetInstance()->GetCapabilities();
    }
