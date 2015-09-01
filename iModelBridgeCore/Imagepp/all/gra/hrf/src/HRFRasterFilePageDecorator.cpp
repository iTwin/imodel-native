//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRasterFilePageDecorator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFRasterFilePageDecorator
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCAccessMode.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HGFResolutionDescriptor.h>

#include <Imagepp/all/h/HRFCapability.h>
#include <Imagepp/all/h/HRFResBoosterEditor.h>
#include <Imagepp/all/h/HRFCacheRandomBlockEditor.h>
#include <Imagepp/all/h/HRFRasterFilePageDecorator.h>
#include <Imagepp/all/h/HRFCacheSequentialBlockEditor.h>
#include <Imagepp/all/h/HRFCombinedRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFSLOModelComposer.h>
#include <Imagepp/all/h/HGF2DStretch.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFRasterFilePageDecorator::HRFRasterFilePageDecorator(HFCPtr<HRFRasterFile>&    pi_rpOriginalFile,
                                                       const HRFPageFileCreator* pi_pCreator)
    : HRFRasterFileExtender(pi_rpOriginalFile)
    {
    HPRECONDITION(pi_rpOriginalFile != 0);
    HPRECONDITION(pi_pCreator != 0);

    // The ancestor store the access mode
    m_IsOpen          = false;

    // Keep the page file
    m_pPageFile = pi_pCreator->CreateFor(pi_rpOriginalFile);

    // Create the capabilities
    m_CombinedRasterFileCapabilities = new HRFCombinedRasterFileCapabilities(m_pOriginalFile->GetCapabilities(),
                                                                             m_pPageFile->GetCapabilities());
    if (GetAccessMode().m_HasCreateAccess)
        {
        m_IsOpen = true;

        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    else
        {
        Open();
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFRasterFilePageDecorator::HRFRasterFilePageDecorator(
    HFCPtr<HRFRasterFile>&    pi_rpOriginalFile,
    HFCPtr<HRFPageFile>&      pi_rpPageFile)
    : HRFRasterFileExtender(pi_rpOriginalFile)
    {
    HPRECONDITION(pi_rpOriginalFile != 0);
    HPRECONDITION(pi_rpPageFile != 0);

    // The ancestor store the access mode
    m_IsOpen          = false;

    // Keep the page file
    m_pPageFile     = pi_rpPageFile;

    // Create the capabilities
    m_CombinedRasterFileCapabilities = new HRFCombinedRasterFileCapabilities(m_pOriginalFile->GetCapabilities(),
                                                                             m_pPageFile->GetCapabilities());

    if (GetAccessMode().m_HasCreateAccess)
        {
        m_IsOpen = true;

        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    else
        {
        Open();
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFRasterFilePageDecorator::~HRFRasterFilePageDecorator()
    {
    Close();
    }


//-----------------------------------------------------------------------------
// Public
// Save
// Save the decorator and the extended.
//-----------------------------------------------------------------------------
void HRFRasterFilePageDecorator::Save()
    {
    Save(false);
    HRFRasterFileExtender::Save();
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Save the page decorator
//-----------------------------------------------------------------------------
void HRFRasterFilePageDecorator::Save(bool pi_IsToBeClosed)
    {
    bool OriginalFileWriteAccess = (m_pOriginalFile->GetAccessMode().m_HasWriteAccess ||
                                     m_pOriginalFile->GetAccessMode().m_HasCreateAccess);
    bool PageFileWriteAccess = (m_pPageFile->GetAccessMode().m_HasWriteAccess ||
                                 m_pPageFile->GetAccessMode().m_HasCreateAccess);

    if (OriginalFileWriteAccess || PageFileWriteAccess)
        {
        // Update Descriptors and raster data
        for (uint32_t Page=0; Page < m_pOriginalFile->CountPages(); Page++)
            {
            // Obtain the Page descriptor
            HFCPtr<HRFPageDescriptor>   pPageDescriptor           = GetPageDescriptor(Page);

            // Update the page Descriptors

            // Update the TransfoModel
            if ((pPageDescriptor->HasTransfoModel()) && (pPageDescriptor->TransfoModelHasChanged()))
                {
                HFCPtr<HGF2DTransfoModel> pModel;

                // only first page is supported
                if (Page == 0)
                    {
                    // Remove the SLO conversion for the pagefileDecorator
                    if (m_pSLOConverterModel != 0)
                        {
                        m_pSLOConverterModel->Reverse();

                        pModel = pPageDescriptor->GetTransfoModel()->Clone();
                        HASSERT(pModel != 0);
                        pModel = m_pSLOConverterModel->ComposeInverseWithDirectOf(*pModel);
                        }
                    else
                        pModel = pPageDescriptor->GetTransfoModel();

                    // simplifie model
                    HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pModel->CreateSimplifiedModel();
                    if (pSimplifiedModel != 0)
                        pModel = pSimplifiedModel;

                    HFCPtr<HRFCapability> pTransfoCapabilityPageFile = new HRFTransfoModelCapability(HFC_WRITE_ONLY, pModel->GetClassID());
                    // Update to Cache file
                    if (Page == 0 && m_pPageFile->GetCapabilities()->Supports(pTransfoCapabilityPageFile))
                        {
                        if (PageFileWriteAccess)
                            {
                            m_pPageFile->GetPageDescriptor(Page)->SetTransfoModel(*pModel);
                            }
                        }
                    }
                else if (OriginalFileWriteAccess)   // Update to Source file
                    {
                    pModel = pPageDescriptor->GetTransfoModel();

                    // simplifie model
                    HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pModel->CreateSimplifiedModel();
                    if (pSimplifiedModel != 0)
                        pModel = pSimplifiedModel;

                    HFCPtr<HRFCapability> pTransfoCapabilityOriginal = new HRFTransfoModelCapability(HFC_WRITE_ONLY, pModel->GetClassID());
                    if (m_pOriginalFile->GetCapabilities()->Supports(pTransfoCapabilityOriginal))
                        m_pOriginalFile->GetPageDescriptor(Page)->SetTransfoModel(*pModel);
                    }
                }
            // Update the Filter
            if ((pPageDescriptor->HasFilter()) && (pPageDescriptor->FiltersHasChanged()))
                {
                HFCPtr<HRFCapability> pFilterCapability = new HRFFilterCapability(HFC_WRITE_ONLY, pPageDescriptor->GetFilter().GetClassID());

                // Update to Cache file
                if (m_pPageFile->GetCapabilities()->Supports(pFilterCapability))
                    {
                    if (PageFileWriteAccess)
                        m_pPageFile->GetPageDescriptor(Page)->SetFilter(pPageDescriptor->GetFilter());
                    }

                // Update to Source file
                else if (m_pOriginalFile->GetCapabilities()->Supports(pFilterCapability) &&
                         OriginalFileWriteAccess)
                    m_pOriginalFile->GetPageDescriptor(Page)->SetFilter(pPageDescriptor->GetFilter());

                }

            // Update the ClipShape
            if ((pPageDescriptor->HasClipShape()) && (pPageDescriptor->ClipShapeHasChanged()))
                {
                HFCPtr<HRFCapability> pShapeCapability = new HRFClipShapeCapability(HFC_WRITE_ONLY, pPageDescriptor->GetClipShape()->GetCoordinateType());

                // Update to Cache file
                if (m_pPageFile->GetCapabilities()->Supports(pShapeCapability))
                    {
                    if (PageFileWriteAccess)
                        m_pPageFile->GetPageDescriptor(Page)->SetClipShape(*pPageDescriptor->GetClipShape());
                    }

                // Update to Source file
                else if (m_pOriginalFile->GetCapabilities()->Supports(pShapeCapability) &&
                         OriginalFileWriteAccess)
                    m_pOriginalFile->GetPageDescriptor(Page)->SetClipShape(*pPageDescriptor->GetClipShape());

                }

            // Update the Histogram
            if ((pPageDescriptor->HasHistogram()) && (pPageDescriptor->HistogramHasChanged()))
                {
                HFCPtr<HRFCapability> pHistogramCapability = new HRFHistogramCapability(HFC_WRITE_ONLY);

                // Update to Cache file
                if (m_pPageFile->GetCapabilities()->Supports(pHistogramCapability))
                    {
                    if (PageFileWriteAccess)
                        m_pPageFile->GetPageDescriptor(Page)->SetHistogram(*pPageDescriptor->GetHistogram());
                    }

                // Update to Source file
                else if (m_pOriginalFile->GetCapabilities()->Supports(pHistogramCapability) &&
                         OriginalFileWriteAccess)
                    m_pOriginalFile->GetPageDescriptor(Page)->SetHistogram(*pPageDescriptor->GetHistogram());
                }

            // Update the Thumbnail
            if ((pPageDescriptor->HasThumbnail()) && (pPageDescriptor->ThumbnailHasChanged()))
                {
                HFCPtr<HRFCapability> pThumbnailCapability = new HRFThumbnailCapability(HFC_WRITE_ONLY);

                // Update to Cache file
                if (m_pPageFile->GetCapabilities()->Supports(pThumbnailCapability))
                    {
                    if (PageFileWriteAccess)
                        m_pPageFile->GetPageDescriptor(Page)->SetThumbnail(*pPageDescriptor->GetThumbnail());
                    }

                // Update to Source file
                else if (m_pOriginalFile->GetCapabilities()->Supports(pThumbnailCapability) &&
                         OriginalFileWriteAccess)
                    m_pOriginalFile->GetPageDescriptor(Page)->SetThumbnail(*pPageDescriptor->GetThumbnail());

                }

            // Update the RepresentativePalette
            if ((pPageDescriptor->HasRepresentativePalette()) && (pPageDescriptor->RepresentativePaletteHasChanged()))
                {
                HFCPtr<HRFCapability> pRepresentativePaletteCapability = new HRFRepresentativePaletteCapability(HFC_WRITE_ONLY);

                // Update to Cache file
                if (m_pPageFile->GetCapabilities()->Supports(pRepresentativePaletteCapability))
                    {
                    if (PageFileWriteAccess)
                        m_pPageFile->GetPageDescriptor(Page)->SetRepresentativePalette(pPageDescriptor->GetRepresentativePalette());
                    }

                // Update to Source file
                else if (m_pOriginalFile->GetCapabilities()->Supports(pRepresentativePaletteCapability) &&
                         OriginalFileWriteAccess)
                    m_pOriginalFile->GetPageDescriptor(Page)->SetRepresentativePalette(pPageDescriptor->GetRepresentativePalette());

                }
            }

        if (pi_IsToBeClosed == false)
            {
            m_pPageFile->WriteToDisk();
            }
        }
    }


//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFRasterFilePageDecorator::CreateResolutionEditor(
    uint32_t        pi_Page,
    unsigned short pi_Resolution,
    HFCAccessMode   pi_AccessMode)
    {
    // Create the original editor
    return m_pOriginalFile->CreateResolutionEditor(pi_Page, pi_Resolution, pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFRasterFilePageDecorator::CreateUnlimitedResolutionEditor(uint32_t      pi_Page,
        double       pi_Resolution,
        HFCAccessMode pi_AccessMode)
    {
    // Create the original editor
    return m_pOriginalFile->CreateUnlimitedResolutionEditor(pi_Page, pi_Resolution, pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFRasterFilePageDecorator::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);

    // if the RasterFile is multipage, the PageFileCreator must be support the multipage
    bool OriginalFileMultiPageSupported;
    OriginalFileMultiPageSupported = (GetOriginalFile()->GetCapabilities()->
                                      GetCapabilityOfType(HRFMultiPageCapability::CLASS_ID,
                                                          GetOriginalFile()->GetAccessMode()) != 0);

    bool PageFileMultiPageSupported;
    PageFileMultiPageSupported = (GetExtendedFile()->GetCapabilities()->
                                  GetCapabilityOfType(HRFMultiPageCapability::CLASS_ID,
                                                      GetOriginalFile()->GetAccessMode()) != 0);

    if (OriginalFileMultiPageSupported && !PageFileMultiPageSupported)
        throw HRFMultiPageNotSupportedException(GetURL()->GetURL());

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    // ??????????????????????????????????????? NOT SUPPORTED NOW
    return true;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFRasterFilePageDecorator::GetCapabilities () const
    {
    return m_CombinedRasterFileCapabilities;
    }

//-----------------------------------------------------------------------------
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFRasterFilePageDecorator::Open()
    {
    // Open the file
    if (!m_IsOpen)
        {
        m_IsOpen = true;
        }

    return true;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFRasterFilePageDecorator::CreateDescriptors ()
    {
    HPRECONDITION (m_IsOpen);

    // Unify descriptors from original file and Booster file

    // HChk NEEDS_WORK TR 114705 : Page file decorator does not support multi-page yet.
    //  for (UInt32 Page=0; Page < m_pOriginalFile->CountPages(); Page++)
    for (uint32_t Page=0; Page < m_pOriginalFile->CountPages(); Page++)
        {
        HFCPtr<HRFPageDescriptor> pOriginalPageDescriptor = m_pOriginalFile->GetPageDescriptor(Page);

        // Add all resolutions descriptor from original file to the Decorator
        HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
        for (unsigned short Resolution=0; Resolution < pOriginalPageDescriptor->CountResolutions(); Resolution++)
            {
            // Create a copy of this resolution descriptor for the ResBooster
            HFCPtr<HRFResolutionDescriptor> pResolution = new HRFResolutionDescriptor(
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetAccessMode(),          // Access Mode
                GetCapabilities(),                                                                      // Capabilities,
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetResolutionXRatio(),    // ResolutionRatio,
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetResolutionYRatio(),    // ResolutionRatio,
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetPixelType(),           // PixelType,
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetCodec(),               // CodecsList,
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetReaderBlockAccess(),   // StorageAccess,
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetWriterBlockAccess(),   // StorageAccess,
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetScanlineOrientation(), // ScanLineOrientation,
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetInterleaveType(),      // InterleaveType
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->IsInterlace(),            // IsInterlace,
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetWidth(),               // Width,
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetHeight(),              // Height,
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetBlockWidth(),          // BlockWidth,
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetBlockHeight(),         // BlockHeight,
                0,                                                                                      // BlocksDataFlag
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetBlockType(),         // Storage Type
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetNumberOfPass(),          // Storage Type
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetPaddingBits(),          // Storage Type
                pOriginalPageDescriptor->GetResolutionDescriptor(Resolution)->GetDownSamplingMethod());          // Storage Type

            ListOfResolutionDescriptor.push_back(pResolution);
            }

        HFCPtr<HRFPageDescriptor> pPageFileDescriptor;

        // get the page from the page file if it exist
        if (Page < m_pPageFile->CountPages())
            {
            HPRECONDITION(Page == 0); // only the first page is supported
            pPageFileDescriptor = m_pPageFile->GetPageDescriptor(Page);
            }

        if (pPageFileDescriptor != 0 && pPageFileDescriptor->HasTransfoModel())
            {
            // we need to have a resolution
            HASSERT(pOriginalPageDescriptor->CountResolutions() > 0);

            HRFScanlineOrientation PageOrientation = pPageFileDescriptor->GetTransfoModelOrientation();
            HRFScanlineOrientation RasterOrientation = pOriginalPageDescriptor->GetResolutionDescriptor(0)->GetScanlineOrientation();

            // support only SLO4, SLO6
            if (RasterOrientation != HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL &&
                RasterOrientation != HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
                throw HRFSloNotSupportedException(m_pOriginalFile->GetURL()->GetURL());

            if (PageOrientation != HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL &&
                PageOrientation != HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
                throw HRFSisterFileSloNotSupportedException(m_pPageFile->GetURL()->GetURL());

            HFCPtr<HGF2DTransfoModel> pModel;

            if (RasterOrientation != PageOrientation)
                {
                m_pSLOConverterModel = new HGF2DStretch();
                ((HFCPtr<HGF2DStretch>&)m_pSLOConverterModel)->SetYScaling(-1.0);
                uint64_t Height = m_pOriginalFile->GetPageDescriptor(Page)->GetResolutionDescriptor(0)->GetHeight();

                CHECK_HUINT64_TO_HDOUBLE_CONV(Height);

                ((HFCPtr<HGF2DStretch>&)m_pSLOConverterModel)->SetTranslation(HGF2DDisplacement(0.0, (double)Height));
                pModel = m_pSLOConverterModel->ComposeInverseWithDirectOf(*pPageFileDescriptor->GetTransfoModel());
                }
            else
                pModel = pPageFileDescriptor->GetTransfoModel();

            // create page with the new model
            HFCPtr<HRFClipShape> pClipShape = 0;
            HRPFilter*           pFilter = 0;
            HRPPixelPalette*     pPixelPalette = 0;
            HFCPtr<HRPHistogram> pHistogram = 0;
            HFCPtr<HRFThumbnail> pThumbnail = 0;

            if (pPageFileDescriptor->HasClipShape())
                pClipShape = pPageFileDescriptor->GetClipShape();

            if (pPageFileDescriptor->HasFilter())
                pFilter = (HRPFilter*)&(pPageFileDescriptor->GetFilter());

            if (pPageFileDescriptor->HasRepresentativePalette())
                pPixelPalette = (HRPPixelPalette*)&(pPageFileDescriptor->GetRepresentativePalette());

            if (pPageFileDescriptor->HasHistogram())
                pHistogram = pPageFileDescriptor->GetHistogram();

            if (pPageFileDescriptor->HasThumbnail())
                pThumbnail = pPageFileDescriptor->GetThumbnail();

            pPageFileDescriptor = new HRFPageDescriptor(m_pPageFile->GetAccessMode(),
                                                        m_pPageFile->GetCapabilities(),
                                                        pPixelPalette,
                                                        pHistogram,
                                                        pThumbnail,
                                                        pClipShape,
                                                        pModel,
                                                        &PageOrientation,
                                                        pFilter,
                                                        (HPMAttributeSet*)&(pPageFileDescriptor->GetTags()),
                                                        pPageFileDescriptor->GetDuration());
            }
        else
            {
            // add page from the original file
            HFCPtr<HRFClipShape> pClipShape = 0;
            HRPFilter*           pFilter = 0;
            HRPPixelPalette*     pPixelPalette = 0;
            HFCPtr<HRPHistogram> pHistogram = 0;
            HFCPtr<HRFThumbnail> pThumbnail = 0;

            if (pOriginalPageDescriptor->HasClipShape())
                pClipShape = pOriginalPageDescriptor->GetClipShape();

            if (pOriginalPageDescriptor->HasFilter())
                pFilter = (HRPFilter*)&(pOriginalPageDescriptor->GetFilter());

            if (pOriginalPageDescriptor->HasRepresentativePalette())
                pPixelPalette = (HRPPixelPalette*)&(pOriginalPageDescriptor->GetRepresentativePalette());

            if (pOriginalPageDescriptor->HasHistogram())
                pHistogram = pOriginalPageDescriptor->GetHistogram();

            if (pOriginalPageDescriptor->HasThumbnail())
                pThumbnail = pOriginalPageDescriptor->GetThumbnail();

            HRFScanlineOrientation ScanOri;
            HRFScanlineOrientation* pScanOri = 0;
            if (pOriginalPageDescriptor->HasTransfoModel())
                {
                ScanOri = pOriginalPageDescriptor->GetTransfoModelOrientation();
                pScanOri = &ScanOri;
                }

            pPageFileDescriptor = new HRFPageDescriptor(pOriginalPageDescriptor->GetAccessMode(),
                                                        pOriginalPageDescriptor->GetCapabilities(),
                                                        pPixelPalette,
                                                        pHistogram,
                                                        pThumbnail,
                                                        pClipShape,
                                                        (pOriginalPageDescriptor->HasTransfoModel() ?
                                                         pOriginalPageDescriptor->GetTransfoModel() : 0),
                                                        pScanOri,
                                                        pFilter,
                                                        (HPMAttributeSet*)&(pOriginalPageDescriptor->GetTags()),
                                                        pOriginalPageDescriptor->GetDuration());

            }

        // Combine the Page descriptors information from original file and Page File
        HFCPtr<HRFPageDescriptor> pPage;
        pPage = new HRFPageDescriptor(pPageFileDescriptor->GetAccessMode(),
                                      GetCapabilities(),
                                      pPageFileDescriptor,
                                      pOriginalPageDescriptor,
                                      ListOfResolutionDescriptor,
                                      pOriginalPageDescriptor->IsUnlimitedResolution());

        m_ListOfPageDescriptor.push_back(pPage);
        }
    }


//-----------------------------------------------------------------------------
// Private
// This method close the file.
//-----------------------------------------------------------------------------
void HRFRasterFilePageDecorator::Close()
    {
    // close the ResBooster file
    if (m_IsOpen)
        {
        m_IsOpen = false;
        Save(true);
        }
    }


//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFRasterFilePageDecorator::GetWorldIdentificator () const
    {
    HPRECONDITION(CountPages() == 1);

    return m_pPageFile->GetWorldIdentificator();
    }

//-----------------------------------------------------------------------------
// Public
// GetPageWorldIdentificator
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFRasterFilePageDecorator::GetPageWorldIdentificator (uint32_t pi_Page) const
    {
    if (pi_Page == 0)
        return m_pPageFile->GetWorldIdentificator();
    else
        return m_pOriginalFile->GetWorldIdentificator();
    }

//-----------------------------------------------------------------------------
// public
// GetPageFile
// Allow to obtain the page file
//-----------------------------------------------------------------------------
HFCPtr<HRFPageFile> HRFRasterFilePageDecorator::GetPageFile()
    {
    return m_pPageFile;
    }