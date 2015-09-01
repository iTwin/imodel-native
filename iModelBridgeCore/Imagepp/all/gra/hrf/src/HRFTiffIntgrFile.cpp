//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFTiffIntgrFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFTiffIntgrFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <ImagePP/all/h/ImageppLib.h>

#include <Imagepp/all/h/HRFTiffIntgrFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFTiffTileEditor.h>
#include <Imagepp/all/h/HRFTiffStripEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HTIFFFile.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HCDCodecLZW.h>

#include <Imagepp/all/h/HRFGeoTiffFile.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HCDCodecIJG.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>



//-----------------------------------------------------------------------------
// HRFTiffCodec1BitCapabilities
//-----------------------------------------------------------------------------
class HRFTiffIntgrCodec1BitCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffIntgrCodec1BitCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        // Codec HMR CCITT
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecHMRCCITT::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        // Codec HMR PackBits
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFTiffCodecPaletteCapabilities
//-----------------------------------------------------------------------------
class HRFTiffIntgrCodecPaletteCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffIntgrCodecPaletteCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFTiffCodecTrueColorCapabilities
//-----------------------------------------------------------------------------
class HRFTiffIntgrCodecTrueColorCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffIntgrCodecTrueColorCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        // Codec IJG (Jpeg)
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFTiffCodecV32RGBAlphaCapabilities
//-----------------------------------------------------------------------------
class HRFTiffIntgrCodecV32RGBAlphaCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffIntgrCodecV32RGBAlphaCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        }
    };

//-----------------------------------------------------------------------------
// HRFTiffCodec16BitsPerChannelCapabilities
//-----------------------------------------------------------------------------
class HRFTiffIntgrCodec16BitsPerChannelCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffIntgrCodec16BitsPerChannelCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_WRITE,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFTiffCodecFloatCapabilities
//-----------------------------------------------------------------------------
class HRFTiffIntgrLosslessCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFTiffIntgrLosslessCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));

        // Codec LZW (Deflate)
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecLZW::CLASS_ID,
                                   new HRFTiffBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFTiffCapabilities
//-----------------------------------------------------------------------------
HRFTiffIntgrCapabilities::HRFTiffIntgrCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeI1R8G8B8
    // Read/Write capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeI1R8G8B8::CLASS_ID,
                                   new HRFTiffIntgrCodec1BitCapabilities()));
    // PixelTypeV1Gray1
    // Read/Write capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                   new HRFTiffIntgrCodec1BitCapabilities()));
    // PixelTypeV1GrayWhite1
    // Read/Write capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV1GrayWhite1::CLASS_ID,
                                   new HRFTiffIntgrCodec1BitCapabilities()));
    // PixelTypeV24R8G8B8
    // Read/Write capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFTiffIntgrCodecTrueColorCapabilities()));
    // PixelTypeV32R8G8B8A8
    // Read/Write capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV32R8G8B8A8::CLASS_ID,
                                   new HRFTiffIntgrCodecV32RGBAlphaCapabilities()));

    // PixelTypeV32R8G8B8A8
    // Read/Write capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV32R8G8B8X8::CLASS_ID,
                                   new HRFTiffIntgrCodecV32RGBAlphaCapabilities()));

    // PixelTypeV48R16G16B16
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV48R16G16B16::CLASS_ID,
                                   new HRFTiffIntgrCodec16BitsPerChannelCapabilities()));

    // PixelTypeV8Gray8
    // Read/Write capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFTiffIntgrCodecTrueColorCapabilities()));
    // PixelTypeV8GrayWhite8
    // Read/Write capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV8GrayWhite8::CLASS_ID,
                                   new HRFTiffIntgrCodecTrueColorCapabilities()));

    // PixelTypeV16Int16
    // Read/Write capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV16Int16::CLASS_ID,
                                   new HRFTiffIntgrLosslessCodecCapabilities()));

    // PixelTypeV32Float32
    // Read capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV32Float32::CLASS_ID,
                                   new HRFTiffIntgrLosslessCodecCapabilities()));

    // PixelTypeI4R8G8B8
    // Read/Write capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeI4R8G8B8::CLASS_ID,
                                   new HRFTiffIntgrCodecPaletteCapabilities()));
    // PixelTypeI8R8G8B8
    // Read/Write capability
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFTiffIntgrCodecPaletteCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE, HRFInterleaveType::PIXEL));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE));

    // Multi Resolution capability
    Add(new HRFMultiResolutionCapability(HFC_READ_WRITE));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE));

    // Embeding capability
    Add(new HRFEmbedingCapability(HFC_READ_WRITE));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeDocumentName));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeImageDescription));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeMake));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeModel));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributePageName));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeDateTime));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeArtist));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeHostComputer));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeInkNames));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeResolutionUnit(0)));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeYResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeCopyright));
    Add(new HRFTagCapability(HFC_READ_WRITE, new HRFAttributeImageSlo(4)));

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DProjective::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DIdentity::CLASS_ID));
    }

HFC_IMPLEMENT_SINGLETON(HRFTiffIntgrCreator)

//-----------------------------------------------------------------------------
// Creator
// TIFF capabilities instance member definition
//-----------------------------------------------------------------------------
HRFTiffIntgrCreator::HRFTiffIntgrCreator()
    : HRFRasterFileCreator(HRFTiffIntgrFile::CLASS_ID)
    {
    // TIFF capabilities instance member initialization
    m_pCapabilities = 0;
    }

// Identification information
WString HRFTiffIntgrCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_IngrTiff()); // Ingr. Tiff
    }

// Identification information
WString HRFTiffIntgrCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

// Identification information
WString HRFTiffIntgrCreator::GetExtensions() const
    {
    return WString(L"*.tif;*.tiff");
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFTiffIntgrCreator::Create(
    const HFCPtr<HFCURL>& pi_rpURL,
    HFCAccessMode         pi_AccessMode,
    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFTiffIntgrFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


// Opens the file and verifies if it is the right type
bool HRFTiffIntgrCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                        uint64_t             pi_Offset) const
    {
    HTIFFFile*  pTiff;
    uint32_t    DirOffset;
    bool       bResult;

    HPRECONDITION(pi_rpURL != 0);

    // try to open the TIFF file, if it cannot be opened,
    // it is not a TIFF, so set the result to false
    HTIFFError* pErr;

    (const_cast<HRFTiffIntgrCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    pTiff = new HTIFFFile (pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
    if ((pTiff->IsValid(&pErr) || ((pErr != 0) && !pErr->IsFatal())) && (pTiff->IsTiff64() == false))
        {
        // the tiff was opened successfully, verify if it is
        // a real tiff, or a HMR file.
        bResult = true;

        // To detect if it is a HMR file, verify if the private tag is present
        // if so, set the result to false
        if ((pTiff->GetField (HMR_IMAGEINFORMATION, &DirOffset)) ||
            (pTiff->GetField (HMR2_IMAGEINFORMATION, &DirOffset)))
            bResult = false;
        else
            {
            // Check if this is not a Geo tiff
            double*    pMat4by4;
            unsigned short*    pTagRagBag;
            unsigned short*    pKeyDirectory;
            uint32_t    Count;
            uint32_t    KeyCount;

            //No a TIFF Intergraph if no TIFF Intergraph tags. are found or
            //the GeoTIFF tags. have priority over the TIFF Intergraph tags.
            if ((!pTiff->GetField(INTERGRAPH_MATRIX, &Count, &pMat4by4) &&
                 !pTiff->GetField(INTERGRAPH_RAGBAG, &Count, &pTagRagBag)) ||
                ((ImageppLib::GetHost().GetImageppLibAdmin()._IsIgnoreGeotiffIntergraphTags() == true) &&
                 (pTiff->GetField(GEOTRANSMATRIX, &Count, &pMat4by4) ||
                  pTiff->GetField(GEOTIEPOINTS, &Count, &pMat4by4)   ||
                  pTiff->GetField(GEOPIXELSCALE, &Count, &pMat4by4)  ||
                  pTiff->GetField(GEOKEYDIRECTORY, &KeyCount, &pKeyDirectory))))
                {
                bResult = false;
                }
            else if (pTiff->GetField(INTERGRAPH_MATRIX, &Count, &pMat4by4))
                {
                // If the given matrix is invalid..
                if (!IsValidDoubleArray  (pMat4by4, Count))
                    bResult = false;
                }
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
                                                                        new HRFTiffBlockCapabilities()));

                    // Create the capability for the current pixel type and codec
                    HFCPtr<HRFCapability> pPixelTypeCapability = new HRFPixelTypeCapability(HFC_READ_ONLY,
                                                                                            CurrentPixelType->GetClassID(),
                                                                                            pCurrentCodecCapability);

                    // Check if we support these pixel type and codec
                    bResult = ((HRFRasterFileCreator*)this)->GetCapabilities()->Supports(pPixelTypeCapability);
                    }
                }
            }

        }
    else
        bResult = false;

    // close the file
    delete pTiff;

    SisterFileLock.ReleaseKey();

    HASSERT(!(const_cast<HRFTiffIntgrCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFTiffIntgrCreator*>(this))->m_pSharingControl = 0;

    return bResult;
    }

// Create or get the singleton capabilities of tiff file.
const HFCPtr<HRFRasterFileCapabilities>& HRFTiffIntgrCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFTiffIntgrCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
inline void s_ConvertFromDoubleToUShortBuffer(double    pi_In,
                                              unsigned short*   po_pOut)
    {
    HPRECONDITION(po_pOut != 0);

    unsigned short* pIn  = (unsigned short*)&pi_In;
    unsigned short* pOut = po_pOut;

    *pOut++ = *pIn++;
    *pOut++ = *pIn++;
    *pOut++ = *pIn++;
    *pOut++ = *pIn++;
    }

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
inline void s_ConvertFromUShortBufferToDouble(unsigned short*   pi_pIn,
                                              double*   po_pOut)
    {
    HPRECONDITION(pi_pIn != 0);
    HPRECONDITION(po_pOut != 0);

    unsigned short* pIn  = pi_pIn;
    unsigned short* pOut = (unsigned short*)po_pOut;

    *pOut++ = *pIn++;
    *pOut++ = *pIn++;
    *pOut++ = *pIn++;
    *pOut++ = *pIn++;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFTiffIntgrFile::HRFTiffIntgrFile(const HFCPtr<HFCURL>& pi_rURL,
                                          HFCAccessMode         pi_AccessMode,
                                          uint64_t             pi_Offset)
    : HRFTiffFile(pi_rURL, pi_AccessMode, pi_Offset, true)
    {
    // if Open success and it is not a new file
    if (Open() && !GetAccessMode().m_HasCreateAccess)
        {
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFTiffIntgrFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_INTERGRAPHWORLD;
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
inline HRFTiffIntgrFile::HRFTiffIntgrFile(const HFCPtr<HFCURL>& pi_rURL,
                                          HFCAccessMode         pi_AccessMode,
                                          uint64_t             pi_Offset,
                                          bool                 pi_DontOpenFile)
    : HRFTiffFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFTiffIntgrFile::~HRFTiffIntgrFile()
    {
    try
        {
        SaveTiffIntgrFile();
        // The tiff ancestor close the file
        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }
    }


//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFTiffIntgrFile::CreateDescriptors()
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

        // the main 1:1 width
        uint32_t MainWidth = 0;
        // MainWidth to calc the resolution ratio
        GetFilePtr()->GetField(IMAGEWIDTH, &MainWidth);


        // Instantiation of Resolution descriptor
        HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
        for (unsigned short Resolution=0; Resolution < CalcNumberOfSubResolution(GetIndexOfPage(Page))+1; Resolution++)
            {
            // Obtain Resolution Information

            // Select the page and resolution
            SetImageInSubImage (GetIndexOfPage(Page)+Resolution);

            // resolution dimension
            uint32_t Width;
            uint32_t Height;
            GetFilePtr()->GetField(IMAGEWIDTH, &Width);
            GetFilePtr()->GetField(IMAGELENGTH, &Height);

            uint32_t BlockWidth (0);
            uint32_t BlockHeight (0);
            HRFBlockType BlockType;

            if (GetFilePtr()->IsTiled())
                {
                // Tile dimension
                GetFilePtr()->GetField(TILEWIDTH, &BlockWidth);
                GetFilePtr()->GetField(TILELENGTH, &BlockHeight);
                BlockType = HRFBlockType::TILE;

                }
            else
                {
                // Strip dimension
                BlockWidth = Width;
                GetFilePtr()->GetField(ROWSPERSTRIP, &BlockHeight);
                BlockType = HRFBlockType::STRIP;
                }

            double Ratio = HRFResolutionDescriptor::RoundResolutionRatio(MainWidth, Width);

            HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
                GetAccessMode(),                               // AccessMode,
                GetCapabilities(),                             // Capabilities,
                Ratio,                                         // XResolutionRatio,
                Ratio,                                         // YResolutionRatio,
                PixelType,                                     // PixelType,
                pCodec,                                        // Codec,
                HRFBlockAccess::RANDOM,                        // RBlockAccess,
                HRFBlockAccess::RANDOM,                        // WBlockAccess,
                HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL, // ScanLineOrientation,
                HRFInterleaveType::PIXEL,                      // InterleaveType
                false,                                         // IsInterlace,
                Width,                                         // Width,
                Height,                                        // Height,
                BlockWidth,                                    // BlockWidth,
                BlockHeight,                                   // BlockHeight,
                0,                                             // BlocksDataFlag
                BlockType);                                       // BlockType

            ListOfResolutionDescriptor.push_back(pResolution);
            }

        // Tag information
        char*  pSystem;
        HPMAttributeSet TagList;
        HFCPtr<HPMGenericAttribute> pTag;
        SetImageInSubImage (GetIndexOfPage(Page));

        CHECK_HUINT64_TO_HDOUBLE_CONV(ListOfResolutionDescriptor[0]->GetWidth())
        CHECK_HUINT64_TO_HDOUBLE_CONV(ListOfResolutionDescriptor[0]->GetHeight())

        HFCPtr<HGF2DTransfoModel> pTransfoModel;

        pTransfoModel = CreateTransfoModelFromTiffMatrix((double)ListOfResolutionDescriptor[0]->GetWidth(),
                                                         (double)ListOfResolutionDescriptor[0]->GetHeight());

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

        // MAKE Tag
        if (GetFilePtr()->GetField(MAKE, &pSystem))
            {
            pTag = new HRFAttributeMake(WString(pSystem,false));
            TagList.Set(pTag);
            }

        // MODEL Tag
        if (GetFilePtr()->GetField(MODEL, &pSystem))
            {
            pTag = new HRFAttributeModel(WString(pSystem,false));
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

        // SLO
        pTag = new HRFAttributeImageSlo((unsigned short)GetScanLineOrientation().m_ScanlineOrientation);
        TagList.Set(pTag);

        HFCPtr<HRFPageDescriptor> pPage;
        pPage = new HRFPageDescriptor (GetAccessMode(),
                                       GetCapabilities(),           // Capabilities,
                                       ListOfResolutionDescriptor,  // ResolutionDescriptor,
                                       0,                           // RepresentativePalette,
                                       0,                           // Histogram,
                                       0,                           // Thumbnail,
                                       0,                           // ClipShape,
                                       pTransfoModel,                // TransfoModel,
                                       0,                           // Filters
                                       &TagList);                   // Defined Tag

        m_ListOfPageDescriptor.push_back(pPage);
        }
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFTiffIntgrFile::Save()
    {
    SaveTiffIntgrFile();
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFTiffIntgrFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validate the file access
    HPRECONDITION(GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess);

    // Validation with the capabilities if it's possible to add a page
    HPRECONDITION(pi_pPage != 0);

    HPRECONDITION(pi_pPage->CountResolutions() > 0);

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    // add the main resolution for the specified page
    if ((CountPages()-1) == 0)
        AddResolutionToFile (0, 0);
    else
        AddResolutionToFile(CountPages() - 1, 0);

    // Add subresolution
    for (unsigned short Resolution=1; Resolution < pi_pPage->CountResolutions(); Resolution++)
        AddResolutionToFile(CountPages() - 1, Resolution);

    // allways we have a transfo model with Intgr Tiff file
    if (!pi_pPage->HasTransfoModel())
        {
        HFCPtr<HGF2DTransfoModel> NullModel;
        NullModel = new HGF2DIdentity();
        pi_pPage->SetTransfoModel(*NullModel);
        }

    return true;
    }


//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFTiffIntgrFile::GetCapabilities () const
    {
    return HRFTiffIntgrCreator::GetInstance()->GetCapabilities();
    }


//---------------------------------------------------------- Protected

//-----------------------------------------------------------------------------
// Protected
// CreateTransfoModelFromTiffMatrix
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HRFTiffIntgrFile::CreateTransfoModelFromTiffMatrix(double pi_Width,
                                                                             double pi_Height) const
    {
    HPRECONDITION(sizeof(double) % sizeof(unsigned short) == 0 && sizeof(double) / sizeof(unsigned short) == 4);
    HPRECONDITION(!HDOUBLE_EQUAL_EPSILON(pi_Width, 0.0) && !HDOUBLE_EQUAL_EPSILON(pi_Height, 0.0));

    HFCMatrix<3, 3>             Matrix;
    HFCPtr<HGF2DTransfoModel>   pTransfoModel;
    unsigned short*                    pTagRagBag;
    uint32_t                    Count;
    double*                    pMat4by4;


    if (GetFilePtr()->GetField(INTERGRAPH_RAGBAG, &Count, &pTagRagBag) && Count >= 17 * 4)
        {
        double aMat4by4[16];
        Byte  i;

        // this code is adapted from ImageManager.
        // I don't know why but the first DOUBLE is not use
        unsigned short* pTagRagBagPtr = pTagRagBag + 4;
        for (i = 0; i < 16; i++, pTagRagBagPtr += 4)
            s_ConvertFromUShortBufferToDouble(pTagRagBagPtr, &aMat4by4[i]);

        // TR 155710
        Matrix[0][0] = (HDOUBLE_EQUAL_EPSILON(aMat4by4[0], 0.0) ? 0.0 : aMat4by4[0]);
        Matrix[0][1] = (HDOUBLE_EQUAL_EPSILON(aMat4by4[1], 0.0) ? 0.0 : aMat4by4[1]);
        Matrix[0][2] = (HDOUBLE_EQUAL_EPSILON(aMat4by4[3], 0.0) ? 0.0 : aMat4by4[3]);
        Matrix[1][0] = (HDOUBLE_EQUAL_EPSILON(aMat4by4[4], 0.0) ? 0.0 : aMat4by4[4]);
        Matrix[1][1] = (HDOUBLE_EQUAL_EPSILON(aMat4by4[5], 0.0) ? 0.0 : aMat4by4[5]);
        Matrix[1][2] = (HDOUBLE_EQUAL_EPSILON(aMat4by4[7], 0.0) ? 0.0 : aMat4by4[7]);
        Matrix[2][0] = (HDOUBLE_EQUAL_EPSILON(aMat4by4[12], 0.0) ? 0.0 : aMat4by4[12]);
        Matrix[2][1] = (HDOUBLE_EQUAL_EPSILON(aMat4by4[13], 0.0) ? 0.0 : aMat4by4[13]);
        Matrix[2][2] = (HDOUBLE_EQUAL_EPSILON(aMat4by4[15], 0.0) ? 0.0 : aMat4by4[15]);

        if (!IsValidMatrix(Matrix))
            {
            Matrix[0][0] = 1.0;
            Matrix[0][1] = 0.0;
            Matrix[0][2] = 0.0;
            Matrix[1][0] = 0.0;
            Matrix[1][1] = 1.0;
            Matrix[1][2] = 0.0;
            Matrix[2][0] = 0.0;
            Matrix[2][1] = 0.0;
            Matrix[2][2] = 1.0;
            }
        pTransfoModel = new HGF2DProjective(Matrix);

        double CurrentRotation = ((HFCPtr<HGF2DProjective> &)(pTransfoModel))->GetRotation();

        if (!HDOUBLE_EQUAL_EPSILON(CurrentRotation, 0.0))
            {
            // Add twice the rotation inverted.
            CurrentRotation =  (CurrentRotation * -2);

            ((HFCPtr<HGF2DProjective> &)(pTransfoModel))->AddRotation (CurrentRotation, Matrix[0][2], Matrix[1][2]);
            }
        }
    // Read the matrix from file
    else if (GetFilePtr()->GetField(INTERGRAPH_MATRIX, &Count, &pMat4by4) && Count >= 16)
        {
        Matrix[0][0] = pMat4by4[0];
        Matrix[0][1] = pMat4by4[1];
        Matrix[0][2] = pMat4by4[3];
        Matrix[1][0] = pMat4by4[4];
        Matrix[1][1] = pMat4by4[5];
        Matrix[1][2] = pMat4by4[7];
        Matrix[2][0] = pMat4by4[12];
        Matrix[2][1] = pMat4by4[13];
        Matrix[2][2] = pMat4by4[15];

        pTransfoModel = new HGF2DProjective(Matrix);
        }

    if (pTransfoModel != 0)
        {
        unsigned short Orientation;
        if (!GetFilePtr()->GetField(ORIENTATION, &Orientation))
            Orientation = 1;    // by default, UPPER LEFT HORIZONTAL (SLO4)

        // this convert is made by IRasB
        unsigned short SLO;
        switch(Orientation)
            {
            case 1:
                SLO = 4;
                break;
            case 2:
                SLO = 5;
                break;
            case 3:
                SLO = 7;
                break;
            case 4:
                SLO = 6;
                break;
            case 5:
                SLO = 0;
                break;
            case 6:
                SLO = 1;
                break;
            case 7:
                SLO = 3;
                break;
            case 8:
                SLO = 2;
                break;
            default:
                SLO = 4;
            };

        switch(SLO)
            {
            case 1: // UPPER_RIGHT_VERTICAL
            case 2: // LOWER_LEFT_VERTICAL
                ((HFCPtr<HGF2DProjective>&)pTransfoModel)->AddVerticalFlip(Matrix[1][2]);
                break;
            case 0: // UPPER_LEFT_VERTICAL
            case 3: // LOWER_RIGHT_VERTICAL
                ((HFCPtr<HGF2DProjective>&)pTransfoModel)->AddHorizontalFlip(Matrix[0][2]);
                break;

            case 4: // UPPER_LEFT_HORIZONTAL
            case 5: // UPPER_RIGHT_HORIZONTAL
            case 6: // LOWER_LEFT_HORIZONTAL
            case 7: // LOWER_RIGHT_HORIZONTAL
                ((HFCPtr<HGF2DProjective>&)pTransfoModel)->AddVerticalFlip(Matrix[1][2]);
                break;
            }

        HFCPtr<HGF2DTransfoModel> pModel =  pTransfoModel->CreateSimplifiedModel();
        if (pModel != 0)
            pTransfoModel = pModel;
        }

    return pTransfoModel;
    }

//-----------------------------------------------------------------------------
// Protected
// WriteTransfoModel
// Used to call different fonction depending of the file type.
//-----------------------------------------------------------------------------
bool HRFTiffIntgrFile::WriteTransfoModel(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel)
    {
    return WriteTransfoModelToTiffMatrix(pi_rpTransfoModel);
    }

//-----------------------------------------------------------------------------
// Protected
// WriteTransfoModelToTiffMatrix
//-----------------------------------------------------------------------------
bool HRFTiffIntgrFile::WriteTransfoModelToTiffMatrix(const HFCPtr<HGF2DTransfoModel>& pi_rpTransfoModel)
    {
    // Validate the file access
    HPRECONDITION(GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess);
    HPRECONDITION(pi_rpTransfoModel != 0);
    HPRECONDITION(sizeof(double) % sizeof(unsigned short) == 0 && sizeof(double) / sizeof(unsigned short) == 4);

    bool Ret = false;

    // Check if the transformation can be represented by a matrix.
    if (pi_rpTransfoModel->CanBeRepresentedByAMatrix())
        {
        unsigned short Orientation;
        unsigned short*  pTagRagBag;
        double*  pMat4by4;
        uint32_t  Count;
        double   aMat[16];

        if (!GetFilePtr()->GetField(ORIENTATION, &Orientation))
            Orientation = 4;    // by default, UPPER LEFT HORIZONTAL (SLO4)

        // this convert is made by IRasB
        unsigned short SLO;
        switch(Orientation)
            {
            case 1:
                SLO = 4;
                break;
            case 2:
                SLO = 5;
                break;
            case 3:
                SLO = 7;
                break;
            case 4:
                SLO = 6;
                break;
            case 5:
                SLO = 0;
                break;
            case 6:
                SLO = 1;
                break;
            case 7:
                SLO = 3;
                break;
            case 8:
                SLO = 2;
                break;
            default:
                SLO = 4;
            };

        HFCMatrix<3,3> Matrix(pi_rpTransfoModel->GetMatrix());
        HFCPtr<HGF2DTransfoModel> pTransfoModel = new HGF2DProjective(Matrix);

        switch(SLO)
            {
            case 0: // UPPER_LEFT_VERTICAL
            case 3: // LOWER_RIGHT_VERTICAL
                ((HFCPtr<HGF2DProjective>&)pTransfoModel)->AddHorizontalFlip(Matrix[0][2]);
                break;

            case 1: // UPPER_RIGHT_VERTICAL
            case 2: // LOWER_LEFT_VERTICAL
            case 4: // UPPER_LEFT_HORIZONTAL
            case 5: // UPPER_RIGHT_HORIZONTAL
            case 6: // LOWER_LEFT_HORIZONTAL
            case 7: // LOWER_RIGHT_HORIZONTAL
                ((HFCPtr<HGF2DProjective>&)pTransfoModel)->AddVerticalFlip(Matrix[1][2]);
                break;
            }

        // Before updating a "RagBag" Intergraph matrix, reinvert the rotation if any.
        if (GetFilePtr()->GetField(INTERGRAPH_RAGBAG, &Count, &pTagRagBag) && Count >= 17 * 4)
            {
            if (pTransfoModel->IsCompatibleWith(HGF2DProjective::CLASS_ID))
                {
                double CurrentRotation = ((HFCPtr<HGF2DProjective> &)(pTransfoModel))->GetRotation();
                HFCMatrix<3,3> TempMatrix(pTransfoModel->GetMatrix());

                if (!HDOUBLE_EQUAL_EPSILON(CurrentRotation, 0.0))
                    {
                    // Add twice the rotation inverted.
                    CurrentRotation = (CurrentRotation * -2);

                    ((HFCPtr<HGF2DProjective> &)(pTransfoModel))->AddRotation (CurrentRotation, TempMatrix[0][2], TempMatrix[1][2]);
                    }
                }
            else if (pTransfoModel->IsCompatibleWith(HGF2DAffine::CLASS_ID))
                {
                double CurrentRotation = ((HFCPtr<HGF2DAffine> &)(pTransfoModel))->GetRotation();
                HFCMatrix<3,3> TempMatrix(pTransfoModel->GetMatrix());

                if (!HDOUBLE_EQUAL_EPSILON(CurrentRotation, 0.0))
                    {
                    // Add twice the rotation inverted.
                    CurrentRotation = (CurrentRotation * -2);

                    ((HFCPtr<HGF2DAffine> &)(pTransfoModel))->AddRotation (CurrentRotation, TempMatrix[0][2], TempMatrix[1][2]);
                    }
                }
            }

        HFCPtr<HGF2DTransfoModel> pModel =  pTransfoModel->CreateSimplifiedModel();
        if (pModel != 0)
            pTransfoModel = pModel;

        if (pTransfoModel->CanBeRepresentedByAMatrix())
            {
            // Get the values to put into the matrix.
            HFCMatrix<3, 3> TheMatrix = pTransfoModel->GetMatrix();

            // Don't update the file if we don't change de Matrix.
            //
            // if the file don't have a Matrix, or
            // the new Matrix is different
            if (GetFilePtr()->GetField(INTERGRAPH_MATRIX, &Count, &pMat4by4) && Count >= 16)
                {
                // Fill the affine matrix.
                aMat[0]  = TheMatrix[0][0];
                aMat[1]  = TheMatrix[0][1];
                aMat[2]  = pMat4by4[2];
                aMat[3]  = TheMatrix[0][2];
                aMat[4]  = TheMatrix[1][0];
                aMat[5]  = TheMatrix[1][1];
                aMat[6]  = pMat4by4[6];
                aMat[7]  = TheMatrix[1][2];
                aMat[8]  = pMat4by4[8];
                aMat[9]  = pMat4by4[9];
                aMat[10] = pMat4by4[10];
                aMat[11] = pMat4by4[11];
                aMat[12] = TheMatrix[2][0];
                aMat[13] = TheMatrix[2][1];
                aMat[14] = pMat4by4[14];
                aMat[15] = TheMatrix[2][2];

                if (memcmp((double*)aMat, pMat4by4, 16 * sizeof(double)) != 0)
                    {
                    // Set the Matrix in the file
                    Ret = GetFilePtr()->SetField(INTERGRAPH_MATRIX, 16, (double*)aMat);
                    }
                else
                    {
                    Ret = true;
                    }
                }
            // 17 * 4 => 4 ushort by double, first double not use, 16 double for the matrix
            else if (GetFilePtr()->GetField(INTERGRAPH_RAGBAG, &Count, &pTagRagBag) && Count >= 17 * 4)
                {
                // Fill the affine matrix.
                aMat[0]  = TheMatrix[0][0];
                aMat[1]  = TheMatrix[0][1];
                s_ConvertFromUShortBufferToDouble(&pTagRagBag[3 * 4], &aMat[2]);  // skip the first double, 4 short by double ((1 + 3) * 4)
                aMat[3]  = TheMatrix[0][2];
                aMat[4]  = TheMatrix[1][0];
                aMat[5]  = TheMatrix[1][1];
                s_ConvertFromUShortBufferToDouble(&pTagRagBag[7 * 4], &aMat[6]);  // skip the first double, 4 short by double ((1 + 3) * 4)
                aMat[7]  = TheMatrix[1][2];
                s_ConvertFromUShortBufferToDouble(&pTagRagBag[10 * 4], &aMat[9]); // skip the first double, 4 short by double ((1 + 3) * 4)
                s_ConvertFromUShortBufferToDouble(&pTagRagBag[11 * 4], &aMat[10]);// skip the first double, 4 short by double ((1 + 3) * 4)
                s_ConvertFromUShortBufferToDouble(&pTagRagBag[12 * 4], &aMat[11]);// skip the first double, 4 short by double ((1 + 3) * 4)
                aMat[12] = TheMatrix[2][0];
                aMat[13] = TheMatrix[2][1];
                s_ConvertFromUShortBufferToDouble(&pTagRagBag[15 * 4], &aMat[14]);// skip the first double, 4 short by double ((1 + 3) * 4)
                aMat[15] = TheMatrix[2][2];

                if (memcmp((double*)aMat, pTagRagBag + 4, 16 * sizeof(double)) != 0)
                    {
                    Byte  i;

                    // this code is adapted from ImageManager.
                    // I don't know why but the first DOUBLE is not use
                    unsigned short* pTagRagBagPtr = pTagRagBag + 4;
                    for (i = 0; i < 16; i++, pTagRagBagPtr += 4)
                        s_ConvertFromDoubleToUShortBuffer(aMat[i], pTagRagBagPtr);

                    // Set the Matrix in the file
                    Ret = GetFilePtr()->SetField(INTERGRAPH_RAGBAG, Count, pTagRagBag);
                    }
                else
                    {
                    Ret = true;
                    }
                }
            }
        }

    return Ret;
    }

//-----------------------------------------------------------------------------
// Private
// SaveTiffIntgrFile
//-----------------------------------------------------------------------------
void HRFTiffIntgrFile::SaveTiffIntgrFile()
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {

        // For resampling method we assume that we have only one page per
        // iTiff file so we keep only a unidimentionnal array (for multires)
        HASSERT(CountPages() <= 1);

        // Write Information
        if (GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess)
            {

            // iTiff do not support multi-page yet, if we want to support it
            // we will need to write one private directory of each page.
            HASSERT(CountPages()<=1);

            // Update the modification to the file
            for (uint32_t Page=0; Page<CountPages(); Page++)
                {
                // Select the page
                SetImageInSubImage (GetIndexOfPage(Page));

                HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(Page);

                // Update the TransfoModel
                if ((pPageDescriptor->HasTransfoModel()) && (pPageDescriptor->TransfoModelHasChanged()))
                    {
                    WriteTransfoModel(pPageDescriptor->GetTransfoModel());
                    pPageDescriptor->Saved();
                    }
                }
            }

        }
    }
