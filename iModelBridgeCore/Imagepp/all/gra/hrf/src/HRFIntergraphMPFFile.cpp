//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIntergraphMPFFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIntergraphMPFFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFIntergraphFile.h>
#include <Imagepp/all/h/HRFIntergraphMPFFile.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/HRPPixelPalette.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>

#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>

#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HRFIntergraphCitFile.h>
#include <Imagepp/all/h/HRFIntergraphTG4File.h>
#include <Imagepp/all/h/HRFIntergraphRleFile.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//-----------------------------------------------------------------------------
// HRFMPFBlockCapabilities
//-----------------------------------------------------------------------------

class HRFMPFBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFMPFBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add(new HRFLineCapability(HFC_READ_ONLY,                // AccessMode
                                  LONG_MAX,                     // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL)); // BlockAccess

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_ONLY,   // AccessMode
                                  LONG_MAX,        // MaxSizeInBytes
                                  32,              // MinWidth
                                  4096,            // MaxWidth
                                  32,              // WidthIncrement
                                  32,              // MinHeight
                                  4096,            // MaxHeight
                                  32));            // HeightIncrement
        }
    };

//-----------------------------------------------------------------------------
// HRFMPFCodecCapabilities
//-----------------------------------------------------------------------------
class HRFMPFCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFMPFCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec CCITT
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRCCITT::CLASS_ID,
                                   new HRFMPFBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRRLE1::CLASS_ID,
                                   new HRFMPFBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFMPFCapabilities
//-----------------------------------------------------------------------------
HRFMPFCapabilities::HRFMPFCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV1Gray1
    // Read/Write/Create capabilities
    // NOTE : Should be a bilevel PixelType
    // Read/Write/Create capabilities
    HFCPtr<HRFPixelTypeCapability> pPixelTypeV1Gray1;
    pPixelTypeV1Gray1 = new HRFPixelTypeCapability(HFC_READ_ONLY,
                                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                                   new HRFMPFCodecCapabilities());
    pPixelTypeV1Gray1->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
    Add((HFCPtr<HRFCapability>&)pPixelTypeV1Gray1);

    // Transfo Model
    // IRASB 6.0 does not support more than stretch
    // Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DProjective::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DAffine::CLASS_ID     ));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DSimilitude::CLASS_ID ));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DStretch::CLASS_ID    ));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DIdentity::CLASS_ID   ));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL ));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::UPPER_LEFT_VERTICAL   ));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::UPPER_RIGHT_VERTICAL  ));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL ));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::LOWER_LEFT_VERTICAL   ));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::LOWER_RIGHT_VERTICAL  ));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE));

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
    Add(new HRFTagCapability(HFC_READ_ONLY        , new HRFAttributeBackground(0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeYResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_ONLY        , new HRFAttributeDontSupportPersistentColor(true)));
    }

HFC_IMPLEMENT_SINGLETON(HRFIntergraphMPFCreator)

//-----------------------------------------------------------------------------
// Creator
// TIFF capabilities instance member definition
//-----------------------------------------------------------------------------

HRFIntergraphMPFCreator::HRFIntergraphMPFCreator()
    : HRFIntergraphFile::Creator(HRFIntergraphMPFFile::CLASS_ID)
    {
    // TIFF capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

WString HRFIntergraphMPFCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_MPF()); // Intergraph MPF File Format
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------

WString HRFIntergraphMPFCreator::GetExtensions() const
    {
    return WString(L"*.mpf");
    }

//-----------------------------------------------------------------------------
// allow to Open an image file
//-----------------------------------------------------------------------------

HFCPtr<HRFRasterFile> HRFIntergraphMPFCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                      HFCAccessMode         pi_AccessMode,
                                                      uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFIntergraphMPFFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
// At this level we cant know if the sub-format is supported, so redefine it
// On each descended sub-format.
//-----------------------------------------------------------------------------

bool HRFIntergraphMPFCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool   Result = false;
    HAutoPtr<HFCBinStream> pFile;
    unsigned short HeaderTypeCode;
    unsigned short DataTypeCode;
    unsigned short WordToFollow;

    (const_cast<HRFIntergraphMPFCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    // Check if the file was a valid Intergraph Cit...
    pFile->SeekToBegin();
    if (pFile->Read(&HeaderTypeCode, sizeof(unsigned short)) != sizeof(unsigned short))
        goto WRAPUP;

    if (HeaderTypeCode == 0x0908)
        {
        if (pFile->Read(&WordToFollow, sizeof(unsigned short)) != sizeof(unsigned short) ||
            pFile->Read(&DataTypeCode, sizeof(unsigned short)) != sizeof(unsigned short))
            goto WRAPUP;

        // If it's a tiled raster retreive the proper data type code.
        if (DataTypeCode == 65)
            {
            if ((((WordToFollow + 2) * 2) % 512) == 0)
                {
                uint32_t HeaderLen = ((WordToFollow + 2) /256) * 512;
                HeaderLen += 18;
                pFile->SeekToPos(HeaderLen);
                if (pFile->Read(&DataTypeCode, sizeof(unsigned short)) != sizeof(unsigned short))
                    goto WRAPUP;
                }
            }

        if (IsMultiPage(*pFile, (WordToFollow + 2)/256) && ((DataTypeCode == 24) || (DataTypeCode == 9)) )
            Result = true;
        }

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFIntergraphMPFCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFIntergraphMPFCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of an Intergraph Cit file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFIntergraphMPFCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFMPFCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFIntergraphMPFFile::HRFIntergraphMPFFile(const HFCPtr<HFCURL>& pi_rURL,
                                           HFCAccessMode         pi_AccessMode,
                                           uint64_t             pi_Offset)
    : HRFIntergraphMonochromeFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    SetDatatypeCode(0);
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
HRFIntergraphMPFFile::HRFIntergraphMPFFile(const HFCPtr<HFCURL>& pi_rURL,
                                           HFCAccessMode         pi_AccessMode,
                                           uint64_t             pi_Offset,
                                           bool                 pi_DontOpenFile)
    : HRFIntergraphMonochromeFile(pi_rURL, pi_AccessMode, pi_Offset, pi_DontOpenFile)
    {
    SetDatatypeCode(0);
    SetBitPerPixel (1);
    }


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRFIntergraphMPFFile::~HRFIntergraphMPFFile()
    {
    // Empty de stl vector of IntergraphBlockSupp
    while (m_IntergraphRasterFiles.size() > 0)
        {
        HFCPtr<HRFRasterFile>& pIntergraphMonochromeFile = m_IntergraphRasterFiles.front();
        pIntergraphMonochromeFile = 0;
        m_IntergraphRasterFiles.erase(m_IntergraphRasterFiles.begin());
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void HRFIntergraphMPFFile::Save()
    {
    IntergraphRasterFileVector::iterator fileIter = m_IntergraphRasterFiles.begin();
    IntergraphRasterFileVector::iterator fileIterEnd = m_IntergraphRasterFiles.end();

    while (fileIter != fileIterEnd)
        {
        (*fileIter)->Save();
        fileIter++;
        }

    HRFIntergraphFile::Save();
    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFIntergraphMPFFile::GetFileCurrentSize() const
    {
    return HRFIntergraphFile::GetFileCurrentSize();
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFIntergraphMPFFile::Open()
    {
    HPRECONDITION(!m_IsOpen);
    HPRECONDITION(!m_pIntergraphFile);

    if (HRFIntergraphFile::Open())
        {
        uint32_t PageCount = CalcNumberOfPage();

        HFCAccessMode CurrentAccessMode = GetAccessMode();

        if (CurrentAccessMode.m_HasReadAccess)
            CurrentAccessMode |= HFC_SHARE_READ_ONLY;

        if (CurrentAccessMode.m_HasWriteAccess)
            CurrentAccessMode |= HFC_SHARE_WRITE_ONLY;

        for (uint32_t PageIndex = 0; PageIndex < CalcNumberOfPage(); PageIndex++)
            {
            // For each counted page, instanciate their corresponding HRFIntergraphXXXFile
            // and add them to our internal vector m_IntergraphRasterFiles

            uint32_t PageOffset = GetpageOffset(PageIndex);
            HFCPtr<HRFRasterFile> pRaster;

            if (HRFIntergraphCitCreator::GetInstance()->IsKindOfFile(GetURL(), PageOffset) )
                {
                pRaster = HRFIntergraphCitCreator::GetInstance()->Create(GetURL(), CurrentAccessMode, PageOffset);
                }

            if (HRFIntergraphTG4Creator::GetInstance()->IsKindOfFile(GetURL(), PageOffset) )
                {
                pRaster = HRFIntergraphTG4Creator::GetInstance()->Create(GetURL(), CurrentAccessMode, PageOffset);
                }

            if (HRFIntergraphRleCreator::GetInstance()->IsKindOfFile(GetURL(), PageOffset) )
                {
                pRaster = HRFIntergraphRleCreator::GetInstance()->Create(GetURL(), CurrentAccessMode, PageOffset);
                }

            if (pRaster != 0)
                m_IntergraphRasterFiles.push_back(pRaster);
            }

        HPOSTCONDITION(m_IntergraphRasterFiles.size() == PageCount);
        }
    return m_IntergraphRasterFiles.size() > 0;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors - Called in read mode...
//-----------------------------------------------------------------------------
void HRFIntergraphMPFFile::CreateDescriptors()
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_IntergraphRasterFiles.size() > 0);
    HPRECONDITION(m_IntergraphRasterFiles.size() == CalcNumberOfPage());

    // Create the descriptor for each resolution of each page
    for (uint32_t Page=0; Page < CalcNumberOfPage(); Page++)
        {
        // Scan all the HRFIntergraphFile list, take their descriptor and
        // add them to the  m_ListOfPageDescriptor.
        HFCPtr<HRFPageDescriptor> pPage = m_IntergraphRasterFiles[Page]->GetPageDescriptor(0);
        m_ListOfPageDescriptor.push_back(pPage);
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFIntergraphMPFFile::GetCapabilities () const
    {
    return HRFIntergraphMPFCreator::GetInstance()->GetCapabilities();
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor: File manipulation
//-----------------------------------------------------------------------------

HRFResolutionEditor* HRFIntergraphMPFFile::CreateResolutionEditor(uint32_t       pi_PageIndex,
                                                                  unsigned short pi_Resolution,
                                                                  HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(GetPageDescriptor(pi_PageIndex) != 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_PageIndex)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_PageIndex)->GetResolutionDescriptor(pi_Resolution) != 0);

    return m_IntergraphRasterFiles[pi_PageIndex]->CreateResolutionEditor(0, pi_Resolution, pi_AccessMode);
    }
