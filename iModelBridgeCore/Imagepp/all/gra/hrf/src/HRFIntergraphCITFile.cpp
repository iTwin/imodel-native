//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIntergraphCITFile.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIntergraphCitFile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HFCURLFile.h>#include <ImagePP/all/h/HRFIntergraphFile.h>#include <ImagePP/all/h/HRFIntergraphCITFile.h>#include <ImagePP/all/h/HRFRasterFileCapabilities.h>
#include <ImagePP/all/h/HRPPixelPalette.h>#include <ImagePP/all/h/HRPPixelTypeV1Gray1.h>
#include <ImagePP/all/h/HCDCodecHMRCCITT.h>
#include <ImagePP/all/h/HGF2DTransfoModel.h>#include <ImagePP/all/h/HGF2DIdentity.h>#include <ImagePP/all/h/HGF2DSimilitude.h>#include <ImagePP/all/h/HGF2DTranslation.h>#include <ImagePP/all/h/HGF2DProjective.h>#include <ImagePP/all/h/HGF2DAffine.h>#include <ImagePP/all/h/HGF2DStretch.h>#include <ImagePP/all/h/HRFUtility.h>#include <ImagePP/all/h/HRFException.h>
#include <ImagePP/all/h/ImagePPMessages.xliff.h>
//-----------------------------------------------------------------------------
// HRFCitBlockCapabilities
//-----------------------------------------------------------------------------
class HRFCitBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFCitBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,        // AccessMode
                                  INT32_MAX,                     // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL)); // BlockAccess

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_WRITE,  // AccessMode
                                  INT32_MAX,        // MaxSizeInBytes
                                  32,              // MinWidth
                                  4096,            // MaxWidth
                                  32,              // WidthIncrement
                                  32,              // MinHeight
                                  4096,            // MaxHeight
                                  32));            // HeightIncrement
        }
    };

//-----------------------------------------------------------------------------
// HRFCitCodecCapabilities
//-----------------------------------------------------------------------------
class HRFCitCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFCitCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec CCITT
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRCCITT::CLASS_ID,
                                   new HRFCitBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFCitCapabilities
//-----------------------------------------------------------------------------
HRFCitCapabilities::HRFCitCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV1Gray1
    // Read/Write/Create capabilities
    // NOTE : Should be a bilevel PixelType
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV1Gray1;
    pPixelTypeV1Gray1 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                                   new HRFCitCodecCapabilities());
    pPixelTypeV1Gray1->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV1Gray1);

    // Transfo Model
    // IRASB 6.0 does not support more than stretch
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

    // re sizable capability
    Add(new HRFResizableCapability(HFC_READ_WRITE_CREATE));

    // MultiResolution Capability
    // NOTE : Some Intergraph file have very bad sub-resolution quality.
    //        So that we do is: we support multi resolution file (we can read file with
    //        multi-res), but do not take any account of the sub-resolution data.
    //        Which mean that we re-calculate the sub-resolution and store it in a cache file

    // HFCPtr<HRFCapability> pMultiResolutionCapability = new HRFMultiResolutionCapability(
    //                                                                HFC_READ_WRITE_CREATE, // AccessMode,
    //                                                                true,                  // SinglePixelType,
    //                                                                true,                  // SingleBlockType,
    //                                                                true,                  // ArbitaryXRatio,
    //                                                                true);                 // ArbitaryYRatio);
    // Add(pMultiResolutionCapability);

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeBackground(0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeYResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDontSupportPersistentColor(true)));
    }

HFC_IMPLEMENT_SINGLETON(HRFIntergraphCitCreator)

//-----------------------------------------------------------------------------
// Creator
// TIFF capabilities instance member definition
//-----------------------------------------------------------------------------

HRFIntergraphCitCreator::HRFIntergraphCitCreator()
    : HRFIntergraphFile::Creator(HRFIntergraphCitFile::CLASS_ID)
    {
    // TIFF capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

Utf8String HRFIntergraphCitCreator::GetLabel() const
    {
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_CIT()); // Intergraph cit File Format
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

Utf8String HRFIntergraphCitCreator::GetExtensions() const
    {
    return Utf8String("*.cit");
    }

//-----------------------------------------------------------------------------
// allow to Open an image file
//-----------------------------------------------------------------------------

HFCPtr<HRFRasterFile> HRFIntergraphCitCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                      HFCAccessMode         pi_AccessMode,
                                                      uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFIntergraphCitFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
// At this level we cant know if the sub-format is supported, so redefine it
// On each descended sub-format.
//-----------------------------------------------------------------------------

bool HRFIntergraphCitCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool   Result = false;
    HAutoPtr<HFCBinStream> pFile;
    uint16_t HeaderTypeCode;
    uint16_t DataTypeCode;
    uint16_t WordToFollow;

    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        // Check if the file was a valid Intergraph Cit...
        pFile->SeekToBegin();
        if (pFile->Read(&HeaderTypeCode, sizeof(uint16_t)) != sizeof(uint16_t))
            goto WRAPUP;

        if (HeaderTypeCode == 0x0908)
            {
            if (pFile->Read(&WordToFollow, sizeof(uint16_t)) != sizeof(uint16_t))
                goto WRAPUP;

            if (pFile->Read(&DataTypeCode, sizeof(uint16_t)) != sizeof(uint16_t))
                goto WRAPUP;

            if (DataTypeCode == 24) // && (pi_Offset || !IsMultiPage(*pFile, (WordToFollow + 2)/256)))
                Result = true;
            }
        }

WRAPUP:

    return Result;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of an Intergraph Cit file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFIntergraphCitCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFCitCapabilities();

    return m_pCapabilities;
    }




//-----------------------------------------------------------------------------
// class HRFIntergraphCitFile
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFIntergraphCitFile::HRFIntergraphCitFile(const HFCPtr<HFCURL>& pi_rURL,
                                           HFCAccessMode         pi_AccessMode,
                                           uint64_t             pi_Offset)
    : HRFIntergraphMonochromeFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    SetDatatypeCode(24);
    SetBitPerPixel (1);

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
HRFIntergraphCitFile::HRFIntergraphCitFile(const HFCPtr<HFCURL>& pi_rURL,
                                           HFCAccessMode         pi_AccessMode,
                                           uint64_t             pi_Offset,
                                           bool                 pi_DontOpenFile)
    : HRFIntergraphMonochromeFile(pi_rURL, pi_AccessMode, pi_Offset, pi_DontOpenFile)
    {
    SetDatatypeCode(24);
    SetBitPerPixel (1);
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors - Called in read mode...
//-----------------------------------------------------------------------------
void HRFIntergraphCitFile::CreateDescriptors()
    {
    HPRECONDITION(m_IsOpen);

    // Create the descriptor for each resolution of each page
    // for (UInt32 Page=0; Page < CalcNumberOfPage(); Page++)
        {
        // Initialise some members and read file header Read
        InitOpenedFile(0);

        // TranfoModel
        //HFCPtr<HGF2DTransfoModel> pTransfoModel = &GetTransfoModel();
        GetTransfoModel();

        bool         FirstResIsTile         = HasTileAccess(0);
        HFCAccessMode AccessMode             = GetAccessMode();
        int16_t InvalidResolutionCount = 0;

        // Scan all sub res to see if we have more than one Bloc type.
        for (uint16_t ResIndex=1; ResIndex <= CountSubResolution(); ResIndex++)
            {
            //HDEBUGCODE( bool SubResIsTile = HasTileAccess(ResIndex); );

            if ((HasTileAccess(ResIndex) != FirstResIsTile) && (AccessMode.m_HasCreateAccess || AccessMode.m_HasWriteAccess))
                {
                Utf8String CurrentFileName(GetURL()->GetURL());

                throw HRFSubResAccessDifferReadOnlyException(CurrentFileName);
                }

            // Do not support invalid sub res.  Any res smaller than 32 x 32 should be consider as invalid.
            if ( (GetWidth(ResIndex)  < 32) || (GetHeight(ResIndex)  < 32))
                {
                InvalidResolutionCount++;
                }
            }

        int16_t ValidResolutionCount = CountSubResolution() + 1;
        HASSERT(InvalidResolutionCount < ValidResolutionCount);

        ValidResolutionCount -= InvalidResolutionCount;
        HASSERT(ValidResolutionCount >= 1);

        // Instantiation of Resolution descriptor
        HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
        for (uint16_t Resolution=0; Resolution < ValidResolutionCount; Resolution++)
            {
            HASSERT_DATA(Resolution > 0 ? GetWidth(Resolution) >= 32 : true);
            HASSERT_DATA(Resolution > 0 ? GetHeight(Resolution) >= 32 : true);

            uint32_t BlockWidth  = GetWidth(Resolution);
            uint32_t BlockHeight = 1;

            // BlockAccess
            HRFBlockAccess  BlockAccess;
            HRFBlockType    BlockType;
            if (HasTileAccess(Resolution))
                {
                BlockAccess    = HRFBlockAccess::RANDOM;
                BlockType      = HRFBlockType::TILE;
                BlockWidth     = GetTileWidth (Resolution);
                BlockHeight    = GetTileHeight(Resolution);
                }
            else
                {
                BlockAccess = HRFBlockAccess::SEQUENTIAL;
                BlockType   = HRFBlockType::LINE;
                }

            HASSERT_DATA(Resolution > 0 ? BlockWidth >= 32 : true);

            // Obtain Resolution Information
            // resolution dimension
            HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
                GetAccessMode(),                        // AccessMode,
                GetCapabilities(),                      // Capabilities,
                GetResolution(Resolution),              // ResolutionRatio,
                GetResolution(Resolution),              // ResolutionRatio,
                GetPixelType(),                         // PixelType,
                new HCDCodecHMRCCITT(),                 // Codec,
                BlockAccess,                            // RBlockAccess,
                BlockAccess,                            // WBlockAccess,
                GetScanlineOrientation(),               // ScanLineOrientation,
                HRFInterleaveType::PIXEL,               // InterleaveType
                false,                                  // IsInterlace,
                GetWidth (Resolution),                  // Width,
                GetHeight(Resolution),                  // Height,
                BlockWidth,                             // BlockWidth,
                BlockHeight,                            // BlockHeight,
                0,                                      // BlockDataFlags
                BlockType);                             // BlockType


            ListOfResolutionDescriptor.push_back(pResolution);
            }

        // Tag
        HPMAttributeSet TagList;
        HFCPtr<HPMGenericAttribute> pTag;

        pTag = new HRFAttributeBackground(0);
        TagList.Set(pTag);

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

        // Persistent color Tag
        pTag = new HRFAttributeDontSupportPersistentColor(true);
        TagList.Set(pTag);

        bool Resizable = ListOfResolutionDescriptor.size() == 1 && (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess);

        HFCPtr<HRFPageDescriptor> pPage;
        pPage = new HRFPageDescriptor (GetAccessMode(),
                                       GetCapabilities(),               // Capabilities,
                                       ListOfResolutionDescriptor,      // ResolutionDescriptor,
                                       0,                               // RepresentativePalette,
                                       0,                               // Histogram,
                                       0,                               // Thumbnail,
                                       0,                               // ClipShape,
                                       m_pTransfoModel,                 // TransfoModel,
                                       0,                               // Filters
                                       &TagList,                        // Defined Tag
                                       0,                               // Duration
                                       Resizable);                      // resizable


        m_ListOfPageDescriptor.push_back(pPage);
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFIntergraphCitFile::GetFileCurrentSize() const
    {
    return HRFIntergraphFile::GetFileCurrentSize();
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFIntergraphCitFile::GetCapabilities () const
    {
    return HRFIntergraphCitCreator::GetInstance()->GetCapabilities();
    }


