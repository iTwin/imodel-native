//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFHMRFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFHMRFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <ImagePP/all/h/ImageppLib.h>

#include <Imagepp/all/h/HTIFFFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFHMRFile.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFHMRTileEditor.h>
#include <Imagepp/all/h/HRFTiffTileEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecLZW.h>

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPChannelOrgGray.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPixelPalette.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>


// this macro returns the number of bytes from a count of bits
#define BYTES_FROM_BITS(bits)  ((bits/8) + ((bits%8) ? 1:0))

// HMR tile size (fixe)
#define TILESIZE            256
#define HISTOGRAM_ENTRY     256




//-----------------------------------------------------------------------------
// HRFHMRBlockCapabilities
//-----------------------------------------------------------------------------
class HRFHMRBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFHMRBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_WRITE_CREATE, // AccessMode
                                  LONG_MAX,              // MaxSizeInBytes
                                  256,                   // MinWidth
                                  256,                   // MaxWidth
                                  0,                     // WidthIncrement
                                  256,                   // MinHeight
                                  256,                   // MaxHeight
                                  0));                   // HeightIncrement
        }
    };

//-----------------------------------------------------------------------------
// HRFHMRCodec1BitCapabilities
//-----------------------------------------------------------------------------
class HRFHMRCodec1BitCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFHMRCodec1BitCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFHMRBlockCapabilities()));
        // Codec CCITT
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRCCITT::CLASS_ID,
                                   new HRFHMRBlockCapabilities()));
        // Codec PackBits
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFHMRBlockCapabilities()));
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFHMRBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFHMRCodec8BitsCapabilities
//-----------------------------------------------------------------------------
class HRFHMRCodec8BitsCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFHMRCodec8BitsCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFHMRBlockCapabilities()));
        // Codec PackBits
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFHMRBlockCapabilities()));
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFHMRBlockCapabilities()));
        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFHMRBlockCapabilities()));

        }
    };

//-----------------------------------------------------------------------------
// HRFHMRCapabilities
//-----------------------------------------------------------------------------
HRFHMRCapabilities::HRFHMRCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeI1R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI1R8G8B8::CLASS_ID,
                                   new HRFHMRCodec1BitCapabilities()));
    // PixelTypeI8R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFHMRCodec8BitsCapabilities()));

    // PixelTypeI8Gray8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8Gray8::CLASS_ID,
                                   new HRFHMRCodec8BitsCapabilities()));


    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DIdentity::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DTranslation::CLASS_ID));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // MultiResolution Capability
    Add(new HRFMultiResolutionCapability(HFC_READ_WRITE_CREATE, // AccessMode,
                                         true,                  // SinglePixelType,
                                         true,                  // SingleStorageType,
                                         false,                 // ArbitaryXRatio,
                                         false));                // ArbitaryYRatio);

    // Embeding capability
    Add(new HRFEmbedingCapability(HFC_READ_WRITE_CREATE));

    // Shape capability
    Add(new HRFClipShapeCapability(HFC_READ_WRITE_CREATE, HRFCoordinateType::PHYSICAL));

    // Histogram capability
    Add(new HRFHistogramCapability(HFC_READ_WRITE_CREATE));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeDocumentName));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeImageDescription));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributePageName));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeDateTime));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeArtist));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeHostComputer));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeInkNames));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeYResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeCopyright));
    }

HFC_IMPLEMENT_SINGLETON(HRFHMRCreator)

//-----------------------------------------------------------------------------
// HRFHMRCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFHMRCreator::HRFHMRCreator()
    : HRFRasterFileCreator(HRFHMRFile::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    }

// Identification information
WString HRFHMRCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_HMR()); //HMR File Format
    }

// Identification information
WString HRFHMRCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

// Identification information
WString HRFHMRCreator::GetExtensions() const
    {
    return WString(L"*.hmr");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFHMRCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode         pi_AccessMode,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFHMRFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

// Opens the file and verifies if it is the right type
bool HRFHMRCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HAutoPtr<HTIFFFile>  pTiff;
    uint32_t    DirOffset;
    bool       bResult;

    HPRECONDITION(pi_rpURL != 0);

    // try to open the TIFF file, if it cannot be opened,
    // it is not a TIFF, so set the result to false
    HTIFFError* pErr;

    (const_cast<HRFHMRCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    pTiff = new HTIFFFile (pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
    if ((pTiff->IsValid(&pErr) || ((pErr != 0) && !pErr->IsFatal())) && (pTiff->IsTiff64() == false))
        {
        // set the result to true, it will be false if an error
        // occurs
        bResult = true;

        // To detect if it is a HMR file, verify if the private tag is present
        // if the tag is not there, set the result to false
        if (!pTiff->GetField (HMR_IMAGEINFORMATION, &DirOffset))
            bResult = false;
        else
            {
            // Declare the current pixel type and codec
            HFCPtr<HRPPixelType> CurrentPixelType;
            HFCPtr<HCDCodec>     CurrentCodec;
            try
                {
                // Create the current pixel type and codec
                CurrentPixelType = HRFTiffFile::CreatePixelTypeFromFile(pTiff);
                CurrentCodec     = HRFTiffFile::CreateCodecFromFile(pTiff);
                }

            catch (...)
                {
                bResult = false;
                }

            if (bResult)
                {
                // Create the codec list to be attach to the PixelType Capability.
                HFCPtr<HRFRasterFileCapabilities> pCurrentCodecCapability = new HRFRasterFileCapabilities();
                pCurrentCodecCapability->Add(new HRFCodecCapability(HFC_READ_ONLY,
                                                                    CurrentCodec->GetClassID(),
                                                                    new HRFHMRBlockCapabilities()));

                // Create the capability for the current pixel type and codec
                HFCPtr<HRFCapability> pPixelTypeCapability = new HRFPixelTypeCapability(HFC_READ_ONLY,
                                                                                        CurrentPixelType->GetClassID(),
                                                                                        pCurrentCodecCapability);

                // Check if we support these pixel type and codec
                bResult = ((HRFRasterFileCreator*)this)->GetCapabilities()->Supports(pPixelTypeCapability);
                }
            }
        }
    else
        bResult = false;

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFHMRCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFHMRCreator*>(this))->m_pSharingControl = 0;

    return bResult;
    }

// Create or get the singleton capabilities of HMR file.
const HFCPtr<HRFRasterFileCapabilities>& HRFHMRCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFHMRCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFHMRFile::HRFHMRFile(const HFCPtr<HFCURL>& pi_rURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset)
    : HRFTiffFile(pi_rURL, pi_AccessMode, pi_Offset, true)
    {
    InitPrivateTagDefault();
    m_HMRDirDirty = false;

    // if Open success and it is not a new file
    if (Open() && !GetAccessMode().m_HasCreateAccess)
        {
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file without open
//-----------------------------------------------------------------------------
HRFHMRFile::HRFHMRFile(const HFCPtr<HFCURL>& pi_rURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset,
                       bool                 pi_DontOpenFile)
    : HRFTiffFile(pi_rURL, pi_AccessMode, pi_Offset, pi_DontOpenFile)
    {
    InitPrivateTagDefault();
    m_HMRDirDirty = false;
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFHMRFile::~HRFHMRFile()
    {
    SaveHmrFile();
    // The tiff ancestor close the file
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFHMRFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_Page)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);
    HPRECONDITION(m_aPaddingByRes != 0);

    HFCPtr<HRFResolutionDescriptor> pRes(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution));
    HRFResolutionEditor* pResult = 0;

    // If there is padding, build a HMR tile editor otherwise, use the tiff editor
    if ((m_Version == HMR_VERSION_TILEPADDING) && (m_aPaddingByRes[pi_Resolution] > 0))
        pResult = new HRFHMRTileEditor(this, pi_Page, pi_Resolution, pi_AccessMode, m_aPaddingByRes[pi_Resolution]);
    else
        pResult = new HRFTiffTileEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    return pResult;
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFHMRFile::Save()
    {
    SaveHmrFile();
    HRFTiffFile::Save();
    }
//-----------------------------------------------------------------------------
// Private
// SaveHmrFile
//-----------------------------------------------------------------------------
void HRFHMRFile::SaveHmrFile()
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call after a exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        try
            {
            // Write Information
            if (GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess)
                {
                // Select the page
                HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

                SetImageInSubImage (GetIndexOfPage(0));

                if (pPageDescriptor->HasClipShape())
                    if (pPageDescriptor->ClipShapeHasChanged() || (GetAccessMode().m_HasCreateAccess))
                        SetClipShape(*pPageDescriptor->GetClipShape());

                if (pPageDescriptor->HasHistogram())
                    if (pPageDescriptor->HistogramHasChanged() || (GetAccessMode().m_HasCreateAccess))
                        {
                        HFCPtr<HRPHistogram> pHistogram = pPageDescriptor->GetHistogram();
                        HASSERT(pHistogram->GetEntryFrequenciesSize() <= HISTOGRAM_ENTRY);
                        m_pHistogram = new uint32_t[HISTOGRAM_ENTRY];
                        memset(m_pHistogram, 0, HISTOGRAM_ENTRY*sizeof(uint32_t));
                        pHistogram->GetEntryFrequencies(m_pHistogram);
                        m_HMRDirDirty = true;
                        }

                // Display each tag.
                HPMAttributeSet::HPMASiterator TagIterator;

                for (TagIterator  = pPageDescriptor->GetTags().begin(); 
                     TagIterator != pPageDescriptor->GetTags().end(); TagIterator++)
                    {
                    HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

                    if (pPageDescriptor->TagHasChanged(*pTag) || GetAccessMode().m_HasCreateAccess)
                        {
                        // DOCUMENTNAME Tag
                        if (pTag->GetID() == HRFAttributeDocumentName::ATTRIBUTE_ID)
                            GetFilePtr()->SetFieldA(DOCUMENTNAME, (AString(((HFCPtr<HRFAttributeDocumentName>&)pTag)->GetData().c_str()).c_str()));

                        // IMAGEDESCRIPTION Tag
                        if (pTag->GetID() == HRFAttributeImageDescription::ATTRIBUTE_ID)
                            GetFilePtr()->SetFieldA(IMAGEDESCRIPTION, AString(((HFCPtr<HRFAttributeImageDescription>&)pTag)->GetData().c_str()).c_str());

                        // PAGENAME Tag
                        if (pTag->GetID() == HRFAttributePageName::ATTRIBUTE_ID)
                            GetFilePtr()->SetFieldA(PAGENAME, AString(((HFCPtr<HRFAttributePageName>&)pTag)->GetData().c_str()).c_str());

                        // SOFTWARE Tag
                        if (pTag->GetID() == HRFAttributeSoftware::ATTRIBUTE_ID)
                            GetFilePtr()->SetFieldA(SOFTWARE, AString(((HFCPtr<HRFAttributeSoftware>&)pTag)->GetData().c_str()).c_str());

                        // DATETIME Tag
                        if (pTag->GetID() == HRFAttributeDateTime::ATTRIBUTE_ID)
                            GetFilePtr()->SetFieldA(DATETIME, AString(((HFCPtr<HRFAttributeDateTime>&)pTag)->GetData().c_str()).c_str());

                        // ARTIST Tag
                        if (pTag->GetID() == HRFAttributeArtist::ATTRIBUTE_ID)
                            GetFilePtr()->SetFieldA(ARTIST, AString(((HFCPtr<HRFAttributeArtist>&)pTag)->GetData().c_str()).c_str());

                        // HOSTCOMPUTER Tag
                        if (pTag->GetID() == HRFAttributeHostComputer::ATTRIBUTE_ID)
                            GetFilePtr()->SetFieldA(HOSTCOMPUTER, AString(((HFCPtr<HRFAttributeHostComputer>&)pTag)->GetData().c_str()).c_str());

                        // INKNAMES Tag
                        if (pTag->GetID() == HRFAttributeInkNames::ATTRIBUTE_ID)
                            GetFilePtr()->SetFieldA(INKNAMES, AString(((HFCPtr<HRFAttributeInkNames>&)pTag)->GetData().c_str()).c_str());

                        // RESOLUTIONUNIT Tag
                        if (pTag->GetID() == HRFAttributeResolutionUnit::ATTRIBUTE_ID)
                            GetFilePtr()->SetField(RESOLUTIONUNIT, ((HFCPtr<HRFAttributeResolutionUnit>&)pTag)->GetData());

                        // XRESOLUTION Tag
                        if (pTag->GetID() == HRFAttributeXResolution::ATTRIBUTE_ID)
                            {
                            RATIONAL XResolution;
                            XResolution.Value = ((HFCPtr<HRFAttributeXResolution>&)pTag)->GetData();
                            GetFilePtr()->SetField(XRESOLUTION, XResolution);
                            }
                        // YRESOLUTION Tag
                        if (pTag->GetID() == HRFAttributeYResolution::ATTRIBUTE_ID)
                            {
                            RATIONAL YResolution;
                            YResolution.Value = ((HFCPtr<HRFAttributeYResolution>&)pTag)->GetData();
                            GetFilePtr()->SetField(YRESOLUTION, YResolution);
                            }

                        // COPYRIGHT Tag
                        if (pTag->GetID() == HRFAttributeCopyright::ATTRIBUTE_ID)
                            GetFilePtr()->SetFieldA(COPYRIGHT, AString(((HFCPtr<HRFAttributeCopyright>&)pTag)->GetData().c_str()).c_str());
                        }

                    }

                // Update the TransfoModel
                if ((pPageDescriptor->HasTransfoModel()) && (pPageDescriptor->TransfoModelHasChanged()))
                    {
                    // We remove only when we save the model
                    HFCPtr<HGF2DTransfoModel> pModel = pPageDescriptor->GetTransfoModel()->Clone();
                    RemoveHMRInfoFromTransfoModel(pModel);

                    WriteTransfoModel(pModel);
                    }
                WritePrivateDirectory ();
                }
            }
        catch(...)
            {
            // Simply stop exceptions in the destructor
            // We want to known if a exception is throw.
            HASSERT(0);
            }
        }
    }


//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFHMRFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);

    // Validate the page and all resolutions with the HMR capabilities
    // Create other resolution if necessary

    // allways we have a transfo model with HMR
    if (!pi_pPage->HasTransfoModel())
        {
        HFCPtr<HGF2DTransfoModel> NullModel;
        HFCPtr<HGF2DTransfoModel> pModel = AddHMRInfoToTransfoModel(NullModel,
                                                                    (uint32_t)pi_pPage->GetResolutionDescriptor(0)->GetHeight());
        pi_pPage->SetTransfoModel(*pModel);
        }

    AllocMembers(pi_pPage->GetResolutionDescriptor(0)->GetPixelType(),
                 pi_pPage->GetResolutionDescriptor(0)->GetBlockWidth(),
                 pi_pPage->GetResolutionDescriptor(0)->GetBlockHeight());

    // Add the page descriptor to the list
    bool Ret = HRFTiffFile::AddPage(pi_pPage);

    // If padding is on top of the file, adjust height and Padding line
    //
    if (m_Version == HMR_VERSION_TILEPADDING)
        {
        uint32_t NewImgHeight = (uint32_t)ceil((double)pi_pPage->GetResolutionDescriptor(0)->GetHeight() /
                                           (double)pi_pPage->GetResolutionDescriptor(0)->GetBlockHeight()) *
                              pi_pPage->GetResolutionDescriptor(0)->GetBlockHeight();

        m_PaddingLines  = NewImgHeight - (uint32_t)pi_pPage->GetResolutionDescriptor(0)->GetHeight();

        m_aPaddingByRes = new uint32_t[pi_pPage->CountResolutions()];

        uint32_t Height;
        for (unsigned short Resolution=0; Resolution < pi_pPage->CountResolutions(); Resolution++)
            {
            Height = (uint32_t)ceil((double)pi_pPage->GetResolutionDescriptor(Resolution)->GetHeight() /
                                  (double)pi_pPage->GetResolutionDescriptor(Resolution)->GetBlockHeight()) *
                     pi_pPage->GetResolutionDescriptor(Resolution)->GetBlockHeight();

            GetFilePtr()->SetDirectory(Resolution);
            GetFilePtr()->SetField (IMAGELENGTH, Height);

            m_aPaddingByRes[Resolution] = NewImgHeight - (uint32_t)pi_pPage->GetResolutionDescriptor(Resolution)->GetHeight();
            NewImgHeight /= 2;
            NewImgHeight = ((NewImgHeight + (TILESIZE - 1)) >> 8) << 8;
            }
        }

    return Ret;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFHMRFile::GetCapabilities () const
    {
    return HRFHMRCreator::GetInstance()->GetCapabilities();
    }

//---------------------------------------------------------- Protected

bool HRFHMRFile::Open(bool pi_CreateBigTifFormat)
    {
    bool Ret;

    // Call the parent
    if ((Ret = HRFTiffFile::Open(pi_CreateBigTifFormat)))
        {
        // if this is not a new file then check for hmr validation
        if (!GetAccessMode().m_HasCreateAccess)
            {
            // Valid HMR File
            // Check if the HMR Directory is present
            if (!GetFilePtr()->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, 0)))  // HMR file is a single page
                throw HFCFileNotSupportedException(GetURL()->GetURL());
            m_HMRDirDirty = false;
            ReadPrivateDirectory ();
            }
        else
            m_HMRDirDirty = true;
        }

    return Ret;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFHMRFile::CreateDescriptors()
    {
    HPRECONDITION (m_IsOpen);

    // Create the descriptor for each resolution of each page
    for (uint32_t Page=0; Page < CalcNumberOfPage(); Page++)
        {
        // Select the page
        SetImageInSubImage (GetIndexOfPage(Page));

        // Compression the GetCodecsList
        HFCPtr<HCDCodec> pCodec = CreateCodecFromFile(GetFilePtr(), Page);

        // Pixel Type
        HFCPtr<HRPPixelType> PixelType = CreatePixelTypeFromFile(GetFilePtr(), Page);

        if ((PixelType->GetClassID() != HRPPixelTypeI1R8G8B8::CLASS_ID) &&
            (PixelType->GetClassID() != HRPPixelTypeI8R8G8B8::CLASS_ID))
            throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());

        //TR #206935 - If a grayscale palette is detected, change the pixel type to grayscale palette.
        if (PixelType->GetClassID() == HRPPixelTypeI8R8G8B8::CLASS_ID)
            {
            HRPPixelPalette*    pPalette;
            Byte*              pTableEntry;
            uint32_t              MaxColor;
            bool                IsGrayscalePalette = true;

            pPalette = (HRPPixelPalette*)&(PixelType->GetPalette());
            MaxColor = pPalette->GetMaxEntries();

            for (uint32_t Index = 0; Index < MaxColor; Index++)
                {
                pTableEntry = (Byte*)pPalette->GetCompositeValue(Index);

                if ((pTableEntry[0] != Index) ||
                    (pTableEntry[1] != Index) ||
                    (pTableEntry[2] != Index))
                    {
                    IsGrayscalePalette = false;
                    break;
                    }
                }

            if (IsGrayscalePalette == true)
                {
                PixelType = new HRPPixelTypeI8Gray8();
                }
            }

        uint32_t Width;
        uint32_t Height;
        uint32_t BlockWidth=0;
        uint32_t BlockHeight=0;

        uint32_t HeightMainImage;
        uint32_t PhysicalImageSize;

        // Select the page and resolution and read the image height, all
        // other Resolution are computed from this value.
        SetImageInSubImage (GetIndexOfPage(Page));
        GetFilePtr()->GetField(IMAGELENGTH, &PhysicalImageSize);
        HeightMainImage = PhysicalImageSize - m_PaddingLines;

        // Keep the computed padding byte by resolution.
        m_aPaddingByRes = new uint32_t[CalcNumberOfSubResolution(GetIndexOfPage(Page))+1];

        // TranfoModel
        HFCPtr<HGF2DTransfoModel> pTransfoModel = CreateTransfoModelFromTiffMatrix();
        // Ajsute with HMR Model
        pTransfoModel = AddHMRInfoToTransfoModel(pTransfoModel, HeightMainImage);

        HFCPtr<HRPHistogram> pHistogram = new HRPHistogram(m_pHistogram, HISTOGRAM_ENTRY);

        // Instantiation of Resolution descriptor
        HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
        for (unsigned short Resolution=0; Resolution < CalcNumberOfSubResolution(GetIndexOfPage(Page))+1; Resolution++)
            {
            // Obtain Resolution Information

            // Select the page and resolution
            SetImageInSubImage (GetIndexOfPage(Page)+Resolution);

            // resolution dimension
            GetFilePtr()->GetField(IMAGEWIDTH, &Width);
//
// Bizzz, when we create the image, we use ceil, but if we use it here, we have one line a the
// beginning...
//            Height = (UInt32)ceil((double)HeightMainImage / (pow(2.0, (double)Resolution)));
            Height = (uint32_t)((double)HeightMainImage / (pow(2.0, (double)Resolution)));
            m_aPaddingByRes[Resolution] = PhysicalImageSize - Height;

            // Block dimension
            GetFilePtr()->GetField(TILEWIDTH, &BlockWidth);
            GetFilePtr()->GetField(TILELENGTH, &BlockHeight);

            HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
                GetAccessMode(),                               // AccessMode,
                GetCapabilities(),                             // Capabilities,
                GetResolutionRatio(Page, Resolution),          // XResolutionRatio,
                GetResolutionRatio(Page, Resolution),          // YResolutionRatio,
                PixelType,                                     // PixelType,
                pCodec,                                        // Codec,
                HRFBlockAccess::RANDOM,                        // RStorageAccess,
                HRFBlockAccess::RANDOM,                        // WStorageAccess,
                HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL, // ScanLineOrientation,
                HRFInterleaveType::PIXEL,                      // InterleaveType
                false,                                         // IsInterlace,
                Width,                                         // Width,
                Height,                                        // Height,
                BlockWidth,                                    // BlockWidth,
                BlockHeight,                                   // BlockHeight,
                0,                                             // BlocksDataFlag
                HRFBlockType::TILE);

            ListOfResolutionDescriptor.push_back(pResolution);

            PhysicalImageSize /= 2;
            PhysicalImageSize = ((PhysicalImageSize + (TILESIZE - 1)) >> 8) << 8;
            }

        // Pass only one time by here, because HMR has only
        // one page....

        // Alloc. tile buffer if the version is 1 (possibly pading line)
        AllocMembers(PixelType, BlockWidth, BlockHeight);

        // Tag information
        char*  pSystem;
        HPMAttributeSet TagList;
        HFCPtr<HPMGenericAttribute> pTag;
        SetImageInSubImage (GetIndexOfPage(Page));

        // DOCUMENTNAME Tag
        if (GetFilePtr()->GetField(DOCUMENTNAME, &pSystem))
            {
            pTag = new HRFAttributeDocumentName(WString(pSystem,false));
            TagList.Set(pTag);
            }

        // IMAGEDESCRIPTION Tag
        if (GetFilePtr()->GetField(IMAGEDESCRIPTION, &pSystem))
            {
            pTag = new HRFAttributeImageDescription(WString(pSystem,false));
            TagList.Set(pTag);
            }

        // PAGENAME Tag
        if (GetFilePtr()->GetField(PAGENAME, &pSystem))
            {
            pTag = new HRFAttributePageName(WString(pSystem,false));
            TagList.Set(pTag);
            }

        // SOFTWARE Tag
        if (GetFilePtr()->GetField(SOFTWARE, &pSystem))
            {
            pTag = new HRFAttributeSoftware(WString(pSystem,false));
            TagList.Set(pTag);
            }

        // DATETIME Tag
        if (GetFilePtr()->GetField(DATETIME, &pSystem))
            {
            pTag = new HRFAttributeDateTime(WString(pSystem,false));
            TagList.Set(pTag);
            }

        // ARTIST Tag
        if (GetFilePtr()->GetField(ARTIST, &pSystem))
            {
            pTag = new HRFAttributeArtist(WString(pSystem,false));
            TagList.Set(pTag);
            }

        // HOSTCOMPUTER Tag
        if (GetFilePtr()->GetField(HOSTCOMPUTER, &pSystem))
            {
            pTag = new HRFAttributeHostComputer(WString(pSystem,false));
            TagList.Set(pTag);
            }

        // INKNAMES Tag
        if (GetFilePtr()->GetField(INKNAMES, &pSystem))
            {
            pTag = new HRFAttributeInkNames(WString(pSystem,false));
            TagList.Set(pTag);
            }

        // RESOLUTIONUNIT Tag
        unsigned short UnitValue;
        if (GetFilePtr()->GetField(RESOLUTIONUNIT, &UnitValue))
            {
            pTag = new HRFAttributeResolutionUnit(UnitValue);
            TagList.Set(pTag);
            }
        // XRESOLUTION Tag
        RATIONAL XResolution;
        if (GetFilePtr()->GetField(XRESOLUTION, &XResolution))
            {
            pTag = new HRFAttributeXResolution(XResolution.Value);
            TagList.Set(pTag);
            }
        // YRESOLUTION Tag
        RATIONAL YResolution;
        if (GetFilePtr()->GetField(YRESOLUTION, &YResolution))
            {
            pTag = new HRFAttributeYResolution(YResolution.Value);
            TagList.Set(pTag);
            }
        // COPYRIGHT Tag
        if (GetFilePtr()->GetField(COPYRIGHT, &pSystem))
            {
            pTag = new HRFAttributeCopyright(WString(pSystem,false));
            TagList.Set(pTag);
            }

        HAutoPtr<HRFClipShape> pClipShape(GetClipShape());
        HFCPtr<HRFPageDescriptor> pPage;
        pPage = new HRFPageDescriptor (GetAccessMode(),
                                       GetCapabilities(),           // Capabilities,
                                       ListOfResolutionDescriptor,  // ResolutionDescriptor,
                                       0,                           // RepresentativePalette,
                                       pHistogram,                  // Histogram,
                                       0,                           // Thumbnail,
                                       pClipShape,                  // ClipShape,
                                       pTransfoModel,               // TransfoModel,
                                       0,                           // Filters
                                       &TagList);                   // Tag Information List

        m_ListOfPageDescriptor.push_back(pPage);
        }
    }


//-----------------------------------------------------------------------------
// Protected
// CreateTransfoModelFromTiffMatrix
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFHMRFile::CreateTransfoModelFromTiffMatrix() const
    {
    HFCPtr<HGF2DTransfoModel> pTransfoModel;
    double*                  pMat4by4;
    uint32_t                  Count;

    // Read the matrix from file
    if (GetFilePtr()->GetField(GEOTRANSMATRIX, &Count, &pMat4by4))
        {
        pTransfoModel = new HGF2DAffine();

        ((HFCPtr<HGF2DAffine>&)pTransfoModel)->SetByMatrixParameters(pMat4by4[3],
                                                                     pMat4by4[0],
                                                                     pMat4by4[1],
                                                                     pMat4by4[7],
                                                                     pMat4by4[4],
                                                                     pMat4by4[5]);
        }

    return pTransfoModel;
    }

//-----------------------------------------------------------------------------
// Protected
// GetResolutionRatio - Return resolution ratio for the specified page and resolution
//-----------------------------------------------------------------------------
double HRFHMRFile::GetResolutionRatio(uint32_t pi_Page,
                                       unsigned short pi_Resolution) const
    {
    return (1.0 / pow(2.0, (double)pi_Resolution));
    }


//---------------------------------------------------------- Privates

//-----------------------------------------------------------------------------
// private
// AddHMRInfoToTransfoModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFHMRFile::AddHMRInfoToTransfoModel(HFCPtr<HGF2DTransfoModel>& pi_rpTransfo,
                                                               uint32_t                   pi_ImageHeight)
    {
    HFCPtr<HGF2DTransfoModel> pTransfoModel;

    // If the file has a Matrix use it
    if (pi_rpTransfo != 0)
        {
        // Flip the Y Axe because the origin in a HMR file is at Bottom-Left
        HFCPtr<HGF2DStretch> pFlipModel = new HGF2DStretch();
        pFlipModel->SetYScaling(-1.0);

        pTransfoModel = pFlipModel->ComposeInverseWithDirectOf(*pi_rpTransfo);
        }
    else
        {
        // Make model between Physical and Logical CoordSys.
        // Flip the Y Axe because the origin in a HMR file is at Bottom-Left
        pTransfoModel = (HFCPtr<HGF2DTransfoModel>)new HGF2DStretch ();
        ((HFCPtr<HGF2DStretch>&)pTransfoModel)->SetXScaling(m_PixelSizeX);
        ((HFCPtr<HGF2DStretch>&)pTransfoModel)->SetYScaling(-m_PixelSizeY);
        ((HFCPtr<HGF2DStretch>&)pTransfoModel)->SetTranslation (
            HGF2DDisplacement (m_OriginX, m_OriginY + (pi_ImageHeight*m_PixelSizeY)));
        }

    // Apply possible scaling and translation factor
    double Factor(ImageppLib::GetHost().GetImageppLibAdmin()._GetHMRFileFactor());
    ((HFCPtr<HGF2DStretch>&)pTransfoModel)->SetXScaling (((HFCPtr<HGF2DStretch>&)pTransfoModel)->GetXScaling () * Factor);
    ((HFCPtr<HGF2DStretch>&)pTransfoModel)->SetYScaling (((HFCPtr<HGF2DStretch>&)pTransfoModel)->GetYScaling () * Factor);

    HGF2DDisplacement  displacement (((HFCPtr<HGF2DStretch>&)pTransfoModel)->GetTranslation ());

    displacement *= Factor;

    ((HFCPtr<HGF2DStretch>&)pTransfoModel)->SetTranslation (displacement);

    return pTransfoModel;
    }



//-----------------------------------------------------------------------------
// Private
// RemoveHMRInfoFromTransfoModel
//-----------------------------------------------------------------------------
void HRFHMRFile::RemoveHMRInfoFromTransfoModel(HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel)
    {
    HPRECONDITION(GetPageDescriptor(0) != 0);
    HPRECONDITION(GetPageDescriptor(0)->CountResolutions() > 0);

    // Remove the Flip in the Matrix
    HFCPtr<HGF2DStretch> pFlipModel = new HGF2DStretch();

    pFlipModel->SetYScaling(-1.0);

    HFCPtr<HGF2DTransfoModel> pModel = pFlipModel->ComposeInverseWithDirectOf(*pi_rpTransfoModel);

    // Set the PixelSize and Origin for the HMR Header
    HGF2DDisplacement  Displacement;
    pModel->GetStretchParams (&m_PixelSizeX, &m_PixelSizeY, &Displacement);

    // Can't have a pixelSize < 0
    m_PixelSizeX = fabs(m_PixelSizeX);
    m_PixelSizeY = fabs(m_PixelSizeX);

    m_OriginX = 0.0;
    // Remove the Height, origin -> Left-Bottom
    m_OriginY = (double)GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight();
    pi_rpTransfoModel->ConvertDirect(&m_OriginX, &m_OriginY);

    pi_rpTransfoModel = pModel;

    m_HMRDirDirty = true;
    }

//-----------------------------------------------------------------------------
// private
// InitPrivateTagDefault -
//-----------------------------------------------------------------------------
void HRFHMRFile::InitPrivateTagDefault ()
    {
    // Init. default value for the Private Tags
    //
    m_Version       = HMR_VERSION_TILEPADDING;

    memset (m_SystemCoord, ' ', HMR_LgStringSystemCoord);
    m_SystemCoord[HMR_LgStringSystemCoord-1] = '\0';
    m_OriginX       = 0.0;
    m_OriginY       = 0.0;
    m_PixelSizeX    = 1.0;
    m_PixelSizeY    = 1.0;
    memset (m_HistoDateTime, ' ', HMR_LgStringDateTime);
    m_HistoDateTime[HMR_LgStringDateTime-1] = '\0';
    m_pHistogram = new uint32_t[HISTOGRAM_ENTRY];
    for (int i=0; i<HISTOGRAM_ENTRY; i++)
        m_pHistogram[i] = 1L;
    m_PaddingLines  = 0L;

    m_HMRClipShapeLength = 0L;
    m_pHMRClipShape      = 0;
    m_HMRClipShapeInFile = false;
    m_IsBundleTagPresent    = false;        // to compability with Descartes


    m_HMRTransparentShapeLength = 0L;
    m_pHMRTransparentShape = 0;

    m_HMRUserDataLength = 0L;
    m_pHMRUserData = 0;
    }

//-----------------------------------------------------------------------------
// private
// ReadPrivateDirectory - Read the HMR private Directory on the TIFF file.
//-----------------------------------------------------------------------------
bool HRFHMRFile::ReadPrivateDirectory ()
    {
    HPRECONDITION (GetFilePtr() != 0);

    bool   Ret = true;
    HTIFFFile::DirectoryID CurDir = GetFilePtr()->CurrentDirectory();
    double* pData;
    char*   pUserData;

    // Init. default value for the Private Tags
    //
    InitPrivateTagDefault ();

    m_Version = HMR_VERSION_TILEPADDING;

    // Check if the private tag is present
    //
    if (GetFilePtr()->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, 0)))
        {
        char*  pSystem;
        uint32_t* pHisto;

        m_HMRDirDirty   = false;

        // Read System Coord.
        if (GetFilePtr()->GetField(HMR_IMAGECOORDINATESYSTEM,  &pSystem))
            {
            if (strlen (pSystem) >= HMR_LgStringSystemCoord)
                memcpy (m_SystemCoord, pSystem, HMR_LgStringSystemCoord);
            else
                memcpy (m_SystemCoord, pSystem, strlen(pSystem));
            m_SystemCoord[HMR_LgStringSystemCoord-1] = '\0';
            }

        // Read Origin
        GetFilePtr()->GetField(HMR_XORIGIN, &m_OriginX);
        GetFilePtr()->GetField(HMR_YORIGIN, &m_OriginY);

        // Read PixleSize
        GetFilePtr()->GetField(HMR_XPIXELSIZE, &m_PixelSizeX);
        GetFilePtr()->GetField(HMR_YPIXELSIZE, &m_PixelSizeY);

        // DateTime Histogramme.
        memset (m_HistoDateTime, ' ', HMR_LgStringDateTime);
        m_HistoDateTime[HMR_LgStringDateTime-1] = '\0';
        if (GetFilePtr()->GetField(HMR_HISTOGRAMDATETIME,  &pSystem))
            memcpy (m_HistoDateTime, pSystem, strlen(pSystem));

        // Read Histogramme.
        uint32_t Count;
        if (GetFilePtr()->GetField(HMR_HISTOGRAM, &Count, &pHisto))
            {
            HASSERT(Count == HISTOGRAM_ENTRY);
            m_pHistogram = new uint32_t[HISTOGRAM_ENTRY];

            for (int i=0; i<HISTOGRAM_ENTRY; i++)
                m_pHistogram[i] = pHisto[i];
            }

        // Read File version
        GetFilePtr()->GetField(HMR_VERSION, &m_Version);

        // Read Padding lines.
        GetFilePtr()->GetField(HMR_PADDING, &m_PaddingLines);

        // Get logical shape
        GetFilePtr()->GetField(HMR_LOGICALSHAPE, &m_HMRClipShapeLength, &pData);
        if (m_HMRClipShapeLength > 0)
            {
            m_pHMRClipShape = new double[m_HMRClipShapeLength];
            memcpy(m_pHMRClipShape, pData, m_HMRClipShapeLength * sizeof(double));

            m_HMRClipShapeInFile = true;
            m_IsBundleTagPresent = true;        // to compability with Descartes
            }

        // Get transparent shape
        GetFilePtr()->GetField(HMR_TRANSPARENTSHAPE, (uint32_t*)&m_HMRTransparentShapeLength, &pData);
        if (m_HMRTransparentShapeLength > 0)
            {
            m_pHMRTransparentShape = new double[m_HMRTransparentShapeLength];
            memcpy(m_pHMRTransparentShape, pData, m_HMRTransparentShapeLength * sizeof(double));
            m_IsBundleTagPresent = true;        // to compability with Descartes
            }

        // Get User data
        if (GetFilePtr()->GetField(HMR_USERDATA, &pUserData))
            {
            m_HMRUserDataLength = (int32_t)strlen(pUserData)+1;
            m_pHMRUserData = new Byte[m_HMRUserDataLength];
            memcpy(m_pHMRUserData, pUserData, m_HMRUserDataLength * sizeof(char));
            m_IsBundleTagPresent = true;        // to compability with Descartes
            }
        else
            m_HMRUserDataLength = 0;

        // Reset Directory
        GetFilePtr()->SetDirectory(CurDir);
        }
    else
        {
        m_HMRDirDirty = true;
        Ret = false;
        }

    return (Ret);
    }

//-----------------------------------------------------------------------------
// private
// WritePrivateDirectory -
//-----------------------------------------------------------------------------
void HRFHMRFile::WritePrivateDirectory ()
    {
    HPRECONDITION (GetFilePtr() != 0);

    // HMR Directory is changed ?
    if (m_HMRDirDirty)
        {
        uint32_t PreviousDirectory = GetFilePtr()->CurrentDirectory();

        // If description doesn't set, sets a default
        char* pDesc;
        if (!GetFilePtr()->GetField(IMAGEDESCRIPTION, &pDesc))
            {
            // Set page 0
            SetImageInSubImage (GetIndexOfPage(0));

            char Description[HMR_LgStringDescriptor+1];
            memset (Description, ' ', HMR_LgStringDescriptor);
            Description[HMR_LgStringDescriptor-1] = '\0';
            GetFilePtr()->SetFieldA(IMAGEDESCRIPTION, Description);
            }

        // If directory not present, Add it.
        if (!GetFilePtr()->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::HMR, 0)))
            GetFilePtr()->AddHMRDirectory(HMR_IMAGEINFORMATION);


        // Tag HMR
        GetFilePtr()->SetField(HMR_VERSION,                m_Version);
        GetFilePtr()->SetField(HMR_PADDING,                m_PaddingLines);
        GetFilePtr()->SetFieldA(HMR_IMAGECOORDINATESYSTEM,  m_SystemCoord);
        GetFilePtr()->SetField(HMR_XORIGIN,                m_OriginX);
        GetFilePtr()->SetField(HMR_YORIGIN,                m_OriginY);

        // PixelSize can't be negatif.
        GetFilePtr()->SetField(HMR_XPIXELSIZE,             fabs(m_PixelSizeX));
        GetFilePtr()->SetField(HMR_YPIXELSIZE,             fabs(m_PixelSizeY));
        GetFilePtr()->SetField(HMR_HISTOGRAM, HISTOGRAM_ENTRY, m_pHistogram);

        // DateTime
        GetSystemDateTime (m_HistoDateTime);
        GetFilePtr()->SetFieldA(HMR_HISTOGRAMDATETIME,  m_HistoDateTime);


        // logical, transparent and user data tag
        // Write these bundle of tags when present in file
        if (m_IsBundleTagPresent)
            {
            // Default empty value
            double defDouble[3];
            char   defUserData[10] = "none";
            defDouble[0] = 0.0;
            defDouble[1] = 0.0;
            defDouble[2] = 0.0;

            // Set logical shape
            if ((m_HMRClipShapeLength > 0) || m_HMRClipShapeInFile)
                GetFilePtr()->SetField(HMR_LOGICALSHAPE,
                                       m_HMRClipShapeLength, m_pHMRClipShape);
            else
                GetFilePtr()->SetField(HMR_LOGICALSHAPE,
                                       3, defDouble);

            // Set transparent shape
            if (m_HMRTransparentShapeLength > 0)
                GetFilePtr()->SetField(HMR_TRANSPARENTSHAPE,
                                       m_HMRTransparentShapeLength, m_pHMRTransparentShape);
            else
                GetFilePtr()->SetField(HMR_TRANSPARENTSHAPE,
                                       3, defDouble);

            // Set User data
            if (m_HMRUserDataLength > 0)
                GetFilePtr()->SetFieldA(HMR_USERDATA, (char*)m_pHMRUserData.get());
            else
                GetFilePtr()->SetFieldA(HMR_USERDATA, defUserData);
            }

        // Reset Directory
        GetFilePtr()->SetDirectory(PreviousDirectory);

        m_HMRDirDirty = false;
        }
    }


HRFClipShape* HRFHMRFile::GetClipShape ()
    {
    HRFClipShape* pClipShape = 0;


    // Create the shape from the file if available
    if (m_HMRClipShapeLength > 3)
        {
        pClipShape =  ImportShapeFromArrayOfDouble(m_pHMRClipShape, m_HMRClipShapeLength);
        }

    return (pClipShape);
    }

//-----------------------------------------------------------------------------
// private
// SetClipShape
//-----------------------------------------------------------------------------
void HRFHMRFile::SetClipShape (const HRFClipShape& pi_rShape)
    {
    size_t  NbPoints;

    m_HMRDirDirty        = true;
    m_IsBundleTagPresent = true;

    m_pHMRClipShape = ExportClipShapeToArrayOfDouble(pi_rShape, &NbPoints);
    m_HMRClipShapeLength = (uint32_t)NbPoints;
    }

//-----------------------------------------------------------------------------
// private
// AllocMembers
//-----------------------------------------------------------------------------
void HRFHMRFile::AllocMembers (const HFCPtr<HRPPixelType>& pi_rPixelType,
                               uint32_t pi_BlockWidth, uint32_t pi_BlockHeight)
    {
    // Alloc. tile buffer if the version is 1 (possibly pading line)
    if (m_Version == HMR_VERSION_TILEPADDING)
        {
        m_BytesByPixel = BYTES_FROM_BITS(pi_rPixelType->CountPixelRawDataBits());

        if (pi_rPixelType->CountPixelRawDataBits() < 8)
            {
            // Diviser use if the pixel lenght is < 8
            // Presently, support only 1 or 4 bits by pixel
            m_Pixel1_4Bits = 8 / pi_rPixelType->CountPixelRawDataBits();
            }
        else
            m_Pixel1_4Bits = 1;

        m_pTileBuffer  = new Byte[(pi_BlockWidth * pi_BlockHeight * m_BytesByPixel) /
                                   m_Pixel1_4Bits];
        }
    else
        m_pTileBuffer = 0;
    }

//-----------------------------------------------------------------------------
// Public
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFHMRFile::GetFileCurrentSize() const
    {
    return HRFTiffFile::GetFileCurrentSize();
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFHMRFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_HMRWORLD;
    }

//-----------------------------------------------------------------------------
// Public
// Set the default ratio to meter specified by the user, if this ratio cannot
// be deduced from the file metadata.
//-----------------------------------------------------------------------------
void HRFHMRFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                               uint32_t pi_Page,
                                               bool   pi_CheckSpecificUnitSpec,
                                               bool   pi_InterpretUnitINTGR)
    {
    //The units is implicitly specified in the specification.
    }
