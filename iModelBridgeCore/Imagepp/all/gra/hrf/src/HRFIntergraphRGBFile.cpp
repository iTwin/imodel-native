//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIntergraphRGBFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIntergraphRGBFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFIntergraphRGBFile.h>
#include <Imagepp/all/h/HRFIntergraphColorFile.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/HRPPixelPalette.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecRLE8.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//-----------------------------------------------------------------------------
// HRFRGBBlockCapabilities
//-----------------------------------------------------------------------------
class HRFRGBBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFRGBBlockCapabilities()
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
// HRFRGBCodecCapabilities
//-----------------------------------------------------------------------------
class HRFRGBCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFRGBCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec RGB RLE8
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecRLE8::CLASS_ID,
                                   new HRFRGBBlockCapabilities()));
                                   
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFRGBBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFRGBCapabilities
//-----------------------------------------------------------------------------
HRFRGBCapabilities::HRFRGBCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV24R8G8B8
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV24R8G8B8;
    pPixelTypeV24R8G8B8 = new HRFPixelTypeCapability( HFC_READ_WRITE_CREATE,
                                                      HRPPixelTypeV24R8G8B8::CLASS_ID,
                                                      new HRFRGBCodecCapabilities());
    pPixelTypeV24R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
    pPixelTypeV24R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV24R8G8B8);

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
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeBackground(0)));

    }

HFC_IMPLEMENT_SINGLETON(HRFIntergraphRGBCreator)

//-----------------------------------------------------------------------------
// Creator
// TIFF capabilities instance member definition
//-----------------------------------------------------------------------------

HRFIntergraphRGBCreator::HRFIntergraphRGBCreator()
    : HRFIntergraphFile::Creator(HRFIntergraphRGBFile::CLASS_ID)
    {
    // TIFF capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

WString HRFIntergraphRGBCreator::GetLabel() const
    {

    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_RGB()); // Intergraph RGB File Format
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

WString HRFIntergraphRGBCreator::GetExtensions() const
    {
    return WString(L"*.rgb");
    }

//-----------------------------------------------------------------------------
// allow to Open an image file
//-----------------------------------------------------------------------------

HFCPtr<HRFRasterFile> HRFIntergraphRGBCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                      HFCAccessMode         pi_AccessMode,
                                                      uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFIntergraphRGBFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
// At this level we cant know if the sub-format is supported, so redefine it
// On each descended sub-format.
//-----------------------------------------------------------------------------

bool HRFIntergraphRGBCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool   Result = false;
    HAutoPtr<HFCBinStream> pFile;
    unsigned short HeaderTypeCode;
    unsigned short DataTypeCode;
    unsigned short WordToFollow;
    uint32_t HeaderLen;

    (const_cast<HRFIntergraphRGBCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the RGB File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        // Check if the file was a valid Intergraph RGB...
        pFile->SeekToBegin();
        if (pFile->Read(&HeaderTypeCode, sizeof(unsigned short)) != sizeof(unsigned short))
            goto WRAPUP;

        if (HeaderTypeCode == 0x0908)
            {
            if (pFile->Read(&WordToFollow, sizeof(unsigned short)) != sizeof(unsigned short))
                goto WRAPUP;

            if (pFile->Read(&DataTypeCode, sizeof(unsigned short)) != sizeof(unsigned short))
                goto WRAPUP;

            if ((DataTypeCode == 27 || DataTypeCode == 28) && (pi_Offset || !IsMultiPage(*pFile, (WordToFollow + 2)/256)) )
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

                        if ((DataTypeCode == 27 || DataTypeCode == 28) && !IsMultiPage(*pFile, (WordToFollow + 2)/256))
                            Result = true;
                        }
                    }
                }
            }
        }

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFIntergraphRGBCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFIntergraphRGBCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of an Intergraph RGB file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFIntergraphRGBCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFRGBCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFIntergraphRGBFile::GetFileCurrentSize() const
    {
    return HRFIntergraphFile::GetFileCurrentSize();
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFIntergraphRGBFile::HRFIntergraphRGBFile(const HFCPtr<HFCURL>& pi_rURL,
                                           HFCAccessMode         pi_AccessMode,
                                           uint64_t             pi_Offset)
    : HRFIntergraphColorFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // Data type code will be set by InitOpenedFile() for existing file and AddPage() during creation.
    //SetDatatypeCode(27 or 28);
    SetBitPerPixel (24);

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

        HASSERT(GetDatatypeCode() == 27 || GetDatatypeCode() == 28);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRFIntergraphRGBFile::AddPage (HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HPRECONDITION(pi_pPage->GetResolutionDescriptor(0)->GetCodec()->GetClassID() == HCDCodecIdentity::CLASS_ID || 
                  pi_pPage->GetResolutionDescriptor(0)->GetCodec()->GetClassID() == HCDCodecRLE8::CLASS_ID);

    if(pi_pPage->GetResolutionDescriptor(0)->GetCodec()->GetClassID() == HCDCodecRLE8::CLASS_ID)
        SetDatatypeCode(27);   
    else
        SetDatatypeCode(28);

    return T_Super::AddPage(pi_pPage);
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors - Called in read mode...
//-----------------------------------------------------------------------------
void HRFIntergraphRGBFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);

    // Create the descriptor for each resolution of each page
    // for (UInt32 Page=0; Page < CalcNumberOfPage(); Page++)
        {
        // Initialise some members and read file header Read
        InitOpenedFile(0);

        HASSERT(GetDatatypeCode() == 27 || GetDatatypeCode() == 28);

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
                BlockAccess  = HRFBlockAccess::RANDOM;
                BlockWidth   = GetTileWidth (Resolution);
                BlockHeight  = GetTileHeight(Resolution);
                BlockType    = HRFBlockType::TILE;
                }
            else
                {
                BlockAccess = HRFBlockAccess::SEQUENTIAL;
                BlockType   = HRFBlockType::LINE;
                }

            HFCPtr<HCDCodec> pCodec;
            if(GetDatatypeCode() == 27)
               pCodec = new HCDCodecRLE8();
            else 
                pCodec = new HCDCodecIdentity();

            // Obtain Resolution Information
            // resolution dimension
            HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
                GetAccessMode(),                        // AccessMode,
                GetCapabilities(),                      // Capabilities,
                GetResolution(Resolution),        // XResolutionRatio,
                GetResolution(Resolution),        // YResolutionRatio,
                GetPixelType(),                     // PixelType,
                pCodec,                             // Codec
                BlockAccess,                            // RBlockAccess,
                BlockAccess,                            // WBlockAccess,
                GetScanlineOrientation(),           // ScanLineOrientation,
                HRFInterleaveType::PIXEL,               // InterleaveType
                false,                                  // IsInterlace,
                GetWidth (Resolution),            // Width,
                GetHeight(Resolution),            // Height,
                BlockWidth,                             // BlockWidth,
                BlockHeight,                            // BlockHeight,
                0,                                      // BlockDataFlags
                BlockType);                             // Block type

            ListOfResolutionDescriptor.push_back(pResolution);
            }

        // Tags
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

        uint32_t       BgColor              = 0;
        const uint32_t*   ScrColorComponentPtr = (uint32_t*)&m_IntergraphHeader.IBlock2.use[0];
        Byte*         DstColorComponentPtr = (Byte*)&BgColor;

        // This ensure that we can cast those field to bytes
        HASSERT(ScrColorComponentPtr[0] <= UCHAR_MAX);
        HASSERT(ScrColorComponentPtr[1] <= UCHAR_MAX);
        HASSERT(ScrColorComponentPtr[2] <= UCHAR_MAX);


        // Add each components so that the 4 bytes of the integer are filled with Byte1=R, Byte2=G, Byte3=B and Byte4 =A.
        *DstColorComponentPtr++ = (Byte)*ScrColorComponentPtr++;  // Red
        *DstColorComponentPtr++ = (Byte)*ScrColorComponentPtr++;  // Green
        *DstColorComponentPtr++ = (Byte)*ScrColorComponentPtr++;  // Blue
        *DstColorComponentPtr   = 0;                                // Alpha

        // BACKGROUND Tag
        pTag = new HRFAttributeBackground(BgColor);
        TagList.Set(pTag);


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
                                       &TagList,                    // Tag
                                       0);                          // Duration

        m_ListOfPageDescriptor.push_back(pPage);
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFIntergraphRGBFile::GetCapabilities () const
    {
    return HRFIntergraphRGBCreator::GetInstance()->GetCapabilities();
    }
