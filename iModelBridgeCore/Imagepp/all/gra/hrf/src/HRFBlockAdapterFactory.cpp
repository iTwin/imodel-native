//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFBlockAdapterFactory.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFBlockAdapterFactory
//-----------------------------------------------------------------------------
// This class describes the BlockAdapter implementation
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFBlockAdapterFactory.h>

HFC_IMPLEMENT_SINGLETON(HRFBlockAdapterFactory)

#include <Imagepp/all/h/HRFAdaptLineToImage.h>
#include <Imagepp/all/h/HRFAdaptLineToStrip.h>
#include <Imagepp/all/h/HRFAdaptLineToTile.h>
#include <Imagepp/all/h/HRFAdaptStripToLine.h>
#include <Imagepp/all/h/HRFAdaptStripToTile.h>
#include <Imagepp/all/h/HRFAdaptStripToImage.h>
#include <Imagepp/all/h/HRFAdaptStripToNStrip.h>
#include <Imagepp/all/h/HRFAdaptTileToStrip.h>
#include <Imagepp/all/h/HRFAdaptTileToImage.h>
#include <Imagepp/all/h/HRFAdaptTileToLine.h>
#include <Imagepp/all/h/HRFAdaptNTileToTile.h>
#include <Imagepp/all/h/HRFAdaptImageToLine.h>

HRF_REGISTER_BLOCKADAPTER(HRFAdaptLineToImageCreator)
HRF_REGISTER_BLOCKADAPTER(HRFAdaptLineToStripCreator)
HRF_REGISTER_BLOCKADAPTER(HRFAdaptLineToTileCreator)
HRF_REGISTER_BLOCKADAPTER(HRFAdaptStripToLineCreator)
HRF_REGISTER_BLOCKADAPTER(HRFAdaptStripToTileCreator)
HRF_REGISTER_BLOCKADAPTER(HRFAdaptStripToImageCreator)
HRF_REGISTER_BLOCKADAPTER(HRFAdaptStripToNStripCreator)
HRF_REGISTER_BLOCKADAPTER(HRFAdaptTileToStripCreator)
HRF_REGISTER_BLOCKADAPTER(HRFAdaptTileToImageCreator)
HRF_REGISTER_BLOCKADAPTER(HRFAdaptTileToLineCreator)
HRF_REGISTER_BLOCKADAPTER(HRFAdaptNTileToTileCreator)
HRF_REGISTER_BLOCKADAPTER(HRFAdaptImageToLineCreator)


//-----------------------------------------------------------------------------
// This is a helper class to instantiate an implementation object
// without knowing the different implementations.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRFBlockAdapterFactory::HRFBlockAdapterFactory()
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFBlockAdapterFactory::~HRFBlockAdapterFactory()
    {
    // Empty the registry
    while (m_Creators.size() > 0)
        {
        m_Creators.erase(m_Creators.begin());
        }
    }

//-----------------------------------------------------------------------------
// This methods allow to find the best adapter type for the specified raster file.
//-----------------------------------------------------------------------------
bool HRFBlockAdapterFactory::FindBestAdapterTypeFor(
    HFCPtr<HRFRasterFile>  pi_rpForRasterFile,
    uint32_t               pi_AtPage,
    unsigned short        pi_AtResolution,
    HRFBlockType*          po_ToBlockType,
    uint32_t*                po_ToBlockWidth,
    uint32_t*                po_ToBlockHeight) const
    {
    bool Found = false;

    if (pi_rpForRasterFile->CountPages() > pi_AtPage &&
        !pi_rpForRasterFile->GetPageDescriptor(pi_AtPage)->IsUnlimitedResolution() &&    // don't support unlimited resolution raster
        pi_rpForRasterFile->GetPageDescriptor(pi_AtPage)->CountResolutions() > pi_AtResolution)
        {
        // Adapt From
        HFCPtr<HRFResolutionDescriptor>  ResolutionDescriptor = pi_rpForRasterFile->GetPageDescriptor(pi_AtPage)->GetResolutionDescriptor(pi_AtResolution);

        bool Is1Bit = (ResolutionDescriptor->GetBitsPerPixel() == 1);

        if (ResolutionDescriptor->GetSizeInBytes() <= 1024000) // SmallSize
            {
            // Small case
            *po_ToBlockType = HRFBlockType::IMAGE;
            *po_ToBlockWidth  = HRF_EQUAL_TO_RESOLUTION_WIDTH;
            *po_ToBlockHeight = HRF_EQUAL_TO_RESOLUTION_HEIGHT;
            }
        else if (ResolutionDescriptor->GetSizeInBytes() <= 3072000) // MediumSize
            {
            // Medium case
            if (Is1Bit)
                {
                *po_ToBlockType = HRFBlockType::STRIP;
                *po_ToBlockWidth  = HRF_EQUAL_TO_RESOLUTION_WIDTH;

                if (ResolutionDescriptor->GetBlockWidth() > 164000)
                    *po_ToBlockHeight = 512;
                else if (ResolutionDescriptor->GetBlockWidth() > 96000)
                    *po_ToBlockHeight = 1024;
                else
                    *po_ToBlockHeight = 2048;

                if ((*po_ToBlockType == ResolutionDescriptor->GetBlockType()) || (ResolutionDescriptor->GetBlockType() == HRFBlockType::TILE))
                    {
                    // Best match for n of src block
                    *po_ToBlockHeight = (*po_ToBlockHeight / ResolutionDescriptor->GetBlockHeight()) *
                                        ResolutionDescriptor->GetBlockHeight();

                    // Take src block height
                    if (*po_ToBlockHeight == 0)
                        *po_ToBlockHeight = ResolutionDescriptor->GetBlockHeight();
                    }
                }
            else
                {
                *po_ToBlockType = HRFBlockType::TILE;
                *po_ToBlockWidth  = 256;
                *po_ToBlockHeight = 256;

                if ((*po_ToBlockType == ResolutionDescriptor->GetBlockType()) || (ResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP))
                    {
                    // Best match for n of src block
                    *po_ToBlockHeight = (*po_ToBlockHeight / ResolutionDescriptor->GetBlockHeight()) *
                                        ResolutionDescriptor->GetBlockHeight();

                    if (ResolutionDescriptor->GetBlockType() == HRFBlockType::TILE)
                        // Best match for n of src block
                        *po_ToBlockWidth = (*po_ToBlockWidth / ResolutionDescriptor->GetBlockWidth()) *
                                           ResolutionDescriptor->GetBlockWidth();

                    // Take src block height
                    if (*po_ToBlockHeight == 0)
                        *po_ToBlockHeight = ResolutionDescriptor->GetBlockHeight();
                    }
                }
            }
        else // LargeSize
            {
            // Large case
            if (Is1Bit)
                {
                *po_ToBlockType = HRFBlockType::STRIP;
                *po_ToBlockWidth  = HRF_EQUAL_TO_RESOLUTION_WIDTH;

                if (ResolutionDescriptor->GetBlockWidth() > 164000)
                    *po_ToBlockHeight = 512;
                else if (ResolutionDescriptor->GetBlockWidth() > 96000)
                    *po_ToBlockHeight = 1024;
                else
                    *po_ToBlockHeight = 2048;

                if ((*po_ToBlockType == ResolutionDescriptor->GetBlockType()) || (ResolutionDescriptor->GetBlockType() == HRFBlockType::TILE))
                    {
                    // Best match for n of src block
                    *po_ToBlockHeight = (*po_ToBlockHeight / ResolutionDescriptor->GetBlockHeight()) *
                                        ResolutionDescriptor->GetBlockHeight();
                    // Take src block height
                    if (*po_ToBlockHeight == 0)
                        *po_ToBlockHeight = ResolutionDescriptor->GetBlockHeight();
                    }
                }
            else
                {
                *po_ToBlockType = HRFBlockType::TILE;
                *po_ToBlockWidth  = 256;
                *po_ToBlockHeight = 256;

                if ((*po_ToBlockType == ResolutionDescriptor->GetBlockType()) || (ResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP))
                    {
                    // Best match for n of src block
                    *po_ToBlockHeight = (*po_ToBlockHeight / ResolutionDescriptor->GetBlockHeight()) *
                                        ResolutionDescriptor->GetBlockHeight();

                    if (*po_ToBlockHeight == 0) // the src block height is bigger than 256
                        *po_ToBlockHeight = ResolutionDescriptor->GetBlockHeight();



                    if (ResolutionDescriptor->GetBlockType() == HRFBlockType::TILE)
                        // Best match for n of src block
                        *po_ToBlockWidth = (*po_ToBlockWidth / ResolutionDescriptor->GetBlockWidth()) *
                                           ResolutionDescriptor->GetBlockWidth();

                    if (*po_ToBlockWidth == 0)// the src block width is bigger than 256
                        *po_ToBlockWidth = ResolutionDescriptor->GetBlockWidth();
                    }
                }
            }

        // TR 151026: make sure it fits the limit of HRFRasterFileBlockAdapterBlockCapabilities.  See HRFRasterFileBlockAdapter::CreateDescriptors()
        if(*po_ToBlockType == HRFBlockType::TILE)
            {
            HRFRasterFileBlockAdapterBlockCapabilities BlockAdapterCapabilities;

            HFCPtr<HRFTileCapability> pTileCapability;
            pTileCapability = static_cast<HRFTileCapability*>(BlockAdapterCapabilities.GetCapabilityOfType(HRFTileCapability::CLASS_ID).GetPtr());
            HASSERT(pTileCapability != 0);

            *po_ToBlockWidth  = pTileCapability->ValidateWidth(*po_ToBlockWidth);
            *po_ToBlockHeight = pTileCapability->ValidateHeight(*po_ToBlockHeight);
            }
        else if(*po_ToBlockType == HRFBlockType::STRIP)
            {
            HRFRasterFileBlockAdapterBlockCapabilities BlockAdapterCapabilities;

            HFCPtr<HRFStripCapability> pStripCapability;
            pStripCapability = static_cast<HRFStripCapability*>(BlockAdapterCapabilities.GetCapabilityOfType(HRFStripCapability::CLASS_ID).GetPtr());
            HASSERT(pStripCapability !=0);

            *po_ToBlockHeight = pStripCapability->ValidateHeight(*po_ToBlockHeight);
            }

        Found = CanAdapt(pi_rpForRasterFile,
                         pi_AtPage,
                         pi_AtResolution,
                         *po_ToBlockType,
                         *po_ToBlockWidth,
                         *po_ToBlockHeight);
        }

    return Found;
    }

//-----------------------------------------------------------------------------
// This methods allow to if it is possible to adapt this raster file.
//-----------------------------------------------------------------------------
bool HRFBlockAdapterFactory::CanAdapt(
    HFCPtr<HRFRasterFile>  pi_rpFromRasterFile,
    uint32_t               pi_AtPage,
    unsigned short        pi_AtResolution,
    HRFBlockType           pi_ToBlockType,
    uint32_t               pi_ToWidth,
    uint32_t               pi_ToHeight) const
    {
    bool CanAdapt = false;

    if ((pi_rpFromRasterFile->CountPages() > pi_AtPage) &&
        (pi_rpFromRasterFile->GetPageDescriptor(pi_AtPage)->CountResolutions() > pi_AtResolution))
        {
        // Adapt From
        HFCPtr<HRFResolutionDescriptor>  FromResolutionDescriptor;
        FromResolutionDescriptor = pi_rpFromRasterFile->GetPageDescriptor(pi_AtPage)->GetResolutionDescriptor(pi_AtResolution);

        HRFBlockType   AdaptFrom  = FromResolutionDescriptor->GetBlockType();
        uint32_t       FromWidth  = FromResolutionDescriptor->GetBlockWidth();
        uint32_t       FromHeight = FromResolutionDescriptor->GetBlockHeight();

        // Search the if we can adapt the editor
        if (FindCreator(AdaptFrom, FromWidth, FromHeight, pi_ToBlockType, pi_ToWidth, pi_ToHeight))
            CanAdapt = true;
        }

    return CanAdapt;
    }

//-----------------------------------------------------------------------------
// This factory methods allow to instantiate the adapter for the specified resolution.
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFBlockAdapterFactory::New(
    HFCPtr<HRFRasterFile> pi_rpForRasterFile,
    uint32_t              pi_AtPage,
    unsigned short       pi_AtResolution,
    HFCAccessMode         pi_WithAccessMode) const
    {
    const HRFBlockAdapterCreator* pCreator      = 0;
    HRFBlockAdapter*              pBlockAdapter = 0;

    // Instantiate the raster file
    pCreator = FindCreator(pi_rpForRasterFile, pi_AtPage, pi_AtResolution);

    if (pCreator)
        pBlockAdapter = pCreator->Create(pi_rpForRasterFile,
                                         pi_AtPage,
                                         pi_AtResolution,
                                         pi_WithAccessMode);

    HPOSTCONDITION(pBlockAdapter != 0);

    return pBlockAdapter;
    }

//-----------------------------------------------------------------------------
// Add the creators to the registry
//-----------------------------------------------------------------------------
void HRFBlockAdapterFactory::Register(const HRFBlockAdapterCreator* pi_pCreator)
    {
    // Register the Raster File Creators
    m_Creators.push_back((HRFBlockAdapterCreator*)pi_pCreator);
    }

//-----------------------------------------------------------------------------
// Search the appropriate creator
//-----------------------------------------------------------------------------
const HRFBlockAdapterCreator* HRFBlockAdapterFactory::FindCreator(
    HFCPtr<HRFRasterFile> pi_rpForRasterFile,
    uint32_t              pi_AtPage,
    unsigned short       pi_AtResolution) const
    {
    const HRFBlockAdapterCreator* pCreator = 0;

    if ((pi_rpForRasterFile->CountPages() > pi_AtPage) &&
        (pi_rpForRasterFile->GetPageDescriptor(pi_AtPage)->CountResolutions() > pi_AtResolution))
        {
        // Adapt From
        HFCPtr<HRFResolutionDescriptor>  FromResolutionDescriptor = ((HFCPtr<HRFRasterFileBlockAdapter>&)pi_rpForRasterFile)->GetExtendedFile()->
                                                                    GetPageDescriptor(pi_AtPage)->
                                                                    GetResolutionDescriptor(pi_AtResolution);

        HRFBlockType   AdaptFrom  = FromResolutionDescriptor->GetBlockType();
        uint32_t       FromWidth  = FromResolutionDescriptor->GetBlockWidth();
        uint32_t       FromHeight = FromResolutionDescriptor->GetBlockHeight();

        // Adapt To
        HFCPtr<HRFResolutionDescriptor>  ToResolutionDescriptor = pi_rpForRasterFile->
                                                                  GetPageDescriptor(pi_AtPage)->
                                                                  GetResolutionDescriptor(pi_AtResolution);

        HRFBlockType   AdaptTo  = ToResolutionDescriptor->GetBlockType();
        uint32_t       ToWidth  = ToResolutionDescriptor->GetBlockWidth();
        uint32_t       ToHeight = ToResolutionDescriptor->GetBlockHeight();


        // Search the best creator for that
        pCreator = FindCreator(AdaptFrom, FromWidth, FromHeight, AdaptTo, ToWidth, ToHeight);
        }

    return pCreator;
    }


//-----------------------------------------------------------------------------
// Search the appropriate creator
//-----------------------------------------------------------------------------
const HRFBlockAdapterCreator* HRFBlockAdapterFactory::FindCreator(HRFBlockType   pi_FromBlockType,
                                                                  uint32_t       pi_FromWidth,
                                                                  uint32_t       pi_FromHeight,
                                                                  HRFBlockType   pi_ToBlockType,
                                                                  uint32_t       pi_ToWidth,
                                                                  uint32_t       pi_ToHeight) const
    {
    HRFBlockAdapterCreator* pCreator = 0;

    // Adapt From
    HRFBlockAdapterCapabilities::Capability CapAdaptFrom;

    if (pi_FromBlockType == HRFBlockType::LINE)  CapAdaptFrom = HRFBlockAdapterCapabilities::FROM_LINE;
    else if (pi_FromBlockType == HRFBlockType::STRIP) CapAdaptFrom = HRFBlockAdapterCapabilities::FROM_STRIP;
    else if (pi_FromBlockType == HRFBlockType::TILE)  CapAdaptFrom = HRFBlockAdapterCapabilities::FROM_TILE;
    else  
        {
        HASSERT(pi_FromBlockType == HRFBlockType::IMAGE);
        CapAdaptFrom = HRFBlockAdapterCapabilities::FROM_IMAGE;
        }
    // Adapt To
    HRFBlockAdapterCapabilities::Capability CapAdaptTo;

    if (pi_ToBlockType == HRFBlockType::LINE)  CapAdaptTo = HRFBlockAdapterCapabilities::TO_LINE;
    else if (pi_ToBlockType == HRFBlockType::STRIP) CapAdaptTo = HRFBlockAdapterCapabilities::TO_STRIP;
    else if (pi_ToBlockType == HRFBlockType::TILE)  CapAdaptTo = HRFBlockAdapterCapabilities::TO_TILE;
    else  
        {
        HASSERT(pi_ToBlockType == HRFBlockType::IMAGE);
        CapAdaptTo = HRFBlockAdapterCapabilities::TO_IMAGE;
        }

    // Iterator is used to loop through the vector.
    Creators::const_iterator CreatorIterator;

    // Search the best creator with the capabilities
    CreatorIterator  = m_Creators.begin();
    while (CreatorIterator != m_Creators.end())
        {
        // Test if this is the good creator
        if ((((HRFBlockAdapterCreator*)(*CreatorIterator))->GetCapabilities()->Supports(CapAdaptFrom)) &&
            (((HRFBlockAdapterCreator*)(*CreatorIterator))->GetCapabilities()->Supports(CapAdaptTo)))
            {
            // Convert to the same block type
            // check if we have a multiple of source block in the destination block size
            if ((pi_FromBlockType == pi_ToBlockType) &&
                ((HRFBlockAdapterCreator*)(*CreatorIterator))->GetCapabilities()->Supports(HRFBlockAdapterCapabilities::TO_N_BLOCK)
                && ((pi_ToHeight % pi_FromHeight) == 0))
                {
                if (((pi_ToBlockType == HRFBlockType::STRIP) && (pi_ToHeight > pi_FromHeight)) ||
                    ((pi_ToBlockType == HRFBlockType::TILE)  && ((pi_ToWidth % pi_FromWidth) == 0)) &&
                    ((pi_ToWidth > pi_FromWidth) || (pi_ToHeight > pi_FromHeight)))
                    {
                    // Found the creator
                    pCreator = (*CreatorIterator);
                    // Stop the research
                    CreatorIterator = m_Creators.end();
                    }
                else
                    CreatorIterator++;
                }
            else
                {
                if ((pi_FromBlockType == HRFBlockType::STRIP && pi_ToBlockType == HRFBlockType::TILE)  ||
                    (pi_FromBlockType == HRFBlockType::TILE && pi_ToBlockType  == HRFBlockType::STRIP))
                    {
                    if ((pi_ToHeight % pi_FromHeight) == 0)
                        {
                        // Found the creator
                        pCreator = (*CreatorIterator);
                        // Stop the research
                        CreatorIterator = m_Creators.end();
                        }
                    else
                        CreatorIterator++;
                    }
                else
                    {
                    // Found the creator
                    pCreator = (*CreatorIterator);
                    // Stop the research
                    CreatorIterator = m_Creators.end();
                    }
                }
            }
        else
            CreatorIterator++;
        }

    return pCreator;
    }
