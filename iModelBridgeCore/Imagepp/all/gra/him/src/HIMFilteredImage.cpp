//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMFilteredImage.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HIMFilteredImage
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HIMFilteredImage.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HGF2DGrid.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HIMFilteredImage.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HGSBlitter.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HGSFilter.h>
#include <Imagepp/all/h/HRPTypeAdaptFilter.h>
#include <Imagepp/all/h/HFCGrid.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HRPComplexFilter.h>
#include <Imagepp/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HGSEditor.h>
#include <ImagePP/all/h/HRPFilter.h>

HPM_REGISTER_CLASS(HIMFilteredImage, HRAImageView)

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HIMFilteredImage::HIMFilteredImage()
    :   HRAImageView(HFCPtr<HRARaster>())
    {
    //LinkTo(GetSource());
    }

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HIMFilteredImage::HIMFilteredImage(const HFCPtr<HRARaster>& pi_pSource,
                                   const HFCPtr<HRPFilter>& pi_rpFilter,
                                   const HFCPtr<HRPPixelType> pi_pPixelType)
    :   HRAImageView(pi_pSource)
    {
    HPRECONDITION(pi_rpFilter != 0);

    m_pFilter = pi_rpFilter;
    m_pPixelType = pi_pPixelType;

    InitObject();
    //LinkTo(GetSource());
    }


//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HIMFilteredImage::HIMFilteredImage(const HIMFilteredImage& pi_rFilteredImage)
    :   HRAImageView(pi_rFilteredImage)
    {
    m_pFilter = pi_rFilteredImage.m_pFilter->Clone();
    HPOSTCONDITION(m_pFilter != 0);

    if (pi_rFilteredImage.m_pPixelType != 0)
        {
        m_pPixelType = (HRPPixelType*)pi_rFilteredImage.m_pPixelType->Clone();
        HPOSTCONDITION(m_pPixelType != 0);
        }
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HIMFilteredImage::~HIMFilteredImage()
    {
    DeepDelete();
    //UnlinkFrom(GetSource());
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HRARaster* HIMFilteredImage::Clone (HPMObjectStore* pi_pStore,
                                    HPMPool*        pi_pLog) const
    {
    return new HIMFilteredImage(*this);
    }
//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HPMPersistentObject* HIMFilteredImage::Clone () const
    {
    return new HIMFilteredImage(*this);
    }

//-----------------------------------------------------------------------------
// public
// ContainsPixelsWithChannel
//-----------------------------------------------------------------------------
bool HIMFilteredImage::ContainsPixelsWithChannel(
    HRPChannelType::ChannelRole pi_Role,
    Byte                      pi_Id) const
    {
    return (GetPixelType()->GetChannelOrg().GetChannelIndex(pi_Role, pi_Id) != HRPChannelType::FREE);
    }


//-----------------------------------------------------------------------------
// public
// GetPixelSizeRange
//-----------------------------------------------------------------------------
void HIMFilteredImage::GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const
    {
    // Get the average pixel size and return it for both
    po_rMinimum = GetAveragePixelSize();
    po_rMaximum = po_rMinimum;
    }

//-----------------------------------------------------------------------------
// public
// GetAveragePixelSize
//-----------------------------------------------------------------------------
HGF2DExtent HIMFilteredImage::GetAveragePixelSize() const
    {
    HGF2DExtent ResultExtent(GetCoordSys());

    // Extract from source the minimum pixel size (since the filtered uses this highest resolution)
    HGF2DExtent DumExtent(GetCoordSys());
    GetSource()->GetPixelSizeRange(ResultExtent, DumExtent);

    return(ResultExtent);
    }


//-----------------------------------------------------------------------------
// public
// GetFilter
//-----------------------------------------------------------------------------
HFCPtr<HRPFilter> HIMFilteredImage::GetFilter() const
    {
    return m_pFilter;
    }

//-----------------------------------------------------------------------------
// Notification for content changed
//-----------------------------------------------------------------------------
/*bool HIMFilteredImage::NotifyContentChanged(HMGMessage& pi_rMessage)
{
    // create a new shape
    HVEShape Shape((((HRAContentChangedMsg&)pi_rMessage).GetShape()).GetExtent());

    // create a new msg with that shape
    HRAContentChangedMsg Msg(Shape);

    // call the parent method with the new msg
    HRAImageView::NotifyContentChanged(Msg);

    // notify the raster that the content has changed
    Propagate(HRAContentChangedMsg(Msg));

    // do not propagate anymore the msg with the old shape
    return false;

    return true;
}
*/

//-----------------------------------------------------------------------------
// public
// InitObject
//-----------------------------------------------------------------------------
void HIMFilteredImage::InitObject()
    {
    }

//-----------------------------------------------------------------------------
// public
// DeepDelete
//-----------------------------------------------------------------------------
void HIMFilteredImage::DeepDelete()
    {
    m_pFilter = 0;
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------

HFCPtr<HRPPixelType> HIMFilteredImage::GetPixelType() const
    {
    HFCPtr<HRPPixelType> pPixelType = m_pPixelType;

    if(pPixelType == 0)
        {
        HFCPtr<HRPPixelType> pFilterPixelType;

        // No pixeltype specified. Try to obtain filter pixeltype
        if (m_pFilter->IsCompatibleWith(HRPTypedFilter::CLASS_ID))
            {
            pFilterPixelType = ((HFCPtr<HRPTypedFilter>&)m_pFilter)->GetFilterPixelType();
            }
        else if (m_pFilter->IsCompatibleWith(HRPTypeAdaptFilter::CLASS_ID))
            {
            pFilterPixelType = ((HFCPtr<HRPTypeAdaptFilter>&)m_pFilter)->
                               GetPreferredFilterFor(GetSource()->GetPixelType())->GetFilterPixelType();
            }
        else if (m_pFilter->IsCompatibleWith(HRPComplexFilter::CLASS_ID))
            {
            const HRPComplexFilter::ListFilters FilterList = ((HFCPtr<HRPComplexFilter>&)m_pFilter)->GetList();
            HASSERT(!FilterList.empty());
            HASSERT(FilterList.back()->IsCompatibleWith(HRPTypedFilter::CLASS_ID));

            if (!FilterList.empty() && FilterList.back()->IsCompatibleWith(HRPTypedFilter::CLASS_ID))
                {
                pFilterPixelType =  ((HRPTypedFilter*)FilterList.back())->GetFilterPixelType();
                }
            }
        else
            {
            HASSERT (false);
            }

        // Use source pixeltype by default
        pPixelType = GetSource()->GetPixelType();

        if (pFilterPixelType != 0)
            {
            // the returned pixeltype will be the highest pixeltype between the source and filter
            if (pFilterPixelType->CountPixelRawDataBits() > pPixelType->CountPixelRawDataBits() ||
                pFilterPixelType->CountValueBits() > pPixelType->CountValueBits())
                {
                pPixelType = pFilterPixelType;
                }

            // Should be the highest + not lost the alpha channel if possible.
            // Special case to solve this problem temporary if the source is 48 or 64 we select 32Alpha,
            // the default one presently in the filter.
            //
            // ** The solution will be create a new HRPAlphaReplacer and Compose for 16bits +
            // ** HIMTranslucentImageCreator::CreateTranslucentRaster should instantiate the good one base
            // ** on the pixeltype 24 or 48 or ...
            else if (pFilterPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
                {
                pPixelType = pFilterPixelType;
                }
            }
        }

    return pPixelType;
    }


//-----------------------------------------------------------------------------
// public
// SetFilter
//-----------------------------------------------------------------------------
void HIMFilteredImage::SetFilter(const HFCPtr<HRPFilter>& pi_rpFilter)
    {
    HPRECONDITION(pi_rpFilter != 0);

    m_pFilter = pi_rpFilter;
    }


//-----------------------------------------------------------------------------
// public
// SetPixelType
//-----------------------------------------------------------------------------
void HIMFilteredImage::SetPixelType(const HFCPtr<HRPPixelType> pi_pPixelType)
    {
    m_pPixelType = pi_pPixelType;
    }

//-----------------------------------------------------------------------------
// PreDraw
//-----------------------------------------------------------------------------
void HIMFilteredImage::PreDraw(HRADrawOptions* pio_pOptions)
    {
    GetSource()->PreDraw(pio_pOptions);
    }

//-----------------------------------------------------------------------------
// Draw
//-----------------------------------------------------------------------------
void HIMFilteredImage::Draw(const HFCPtr<HGFMappedSurface>& pio_rpSurface,
                            const HGFDrawOptions* pi_pOptions) const
    {
    HPRECONDITION(pio_rpSurface != 0);

    bool DrawDone = false;

    HRADrawOptions Options(pi_pOptions);
    Options.SetTransaction(0);  // record only when we draw into pio_rpSurface

    HVEShape RegionToDraw(Options.GetShape() != 0 ? *Options.GetShape() : HRARaster::GetShape());
    RegionToDraw.ChangeCoordSys(GetCoordSys());
    if (Options.GetReplacingCoordSys() != 0)
        RegionToDraw.SetCoordSys(Options.GetReplacingCoordSys());

    HFCPtr<HGSRegion> pClipRegion(static_cast<HGSRegion*>(pio_rpSurface->GetOption(HGSRegion::CLASS_ID).GetPtr()));
    if (pClipRegion != 0)
        {
        // Intersect it with the destination
        HFCPtr<HVEShape> pSurfaceShape(pClipRegion->GetShape());
        RegionToDraw.Intersect(*pSurfaceShape);
        }
    else
        {
        // Create a rectangular clip region to stay
        // inside the destination surface.
        HVEShape DestSurfaceShape(0.0, 0.0, pio_rpSurface->GetSurfaceDescriptor()->GetWidth(), pio_rpSurface->GetSurfaceDescriptor()->GetHeight(), pio_rpSurface->GetSurfaceCoordSys());
        RegionToDraw.Intersect(DestSurfaceShape);
        }

    if (!RegionToDraw.IsEmpty())
        {
        const HRPPixelNeighbourhood& Neighborhood = m_pFilter->GetNeighbourhood();

        // In order for our HandleBorderCases method to work, we must create a temporary surface that is not bigger than the source.
        HVEShape RegionToDrawInSurfaceCS(RegionToDraw);
        RegionToDrawInSurfaceCS.ChangeCoordSys(pio_rpSurface->GetSurfaceCoordSys());
        HGF2DExtent SurfaceExtent(RegionToDrawInSurfaceCS.GetExtent());
        HFCGrid     SurfaceGrid(SurfaceExtent.GetOrigin().GetX(), SurfaceExtent.GetOrigin().GetY(), SurfaceExtent.GetCorner().GetX(), SurfaceExtent.GetCorner().GetY());

        // Create temp surface with neighborhood.
        HFCPtr<HGSSurfaceDescriptor> pRenderedDescriptor(new HGSMemorySurfaceDescriptor((uint32_t)SurfaceGrid.GetWidth() + Neighborhood.GetWidth() - 1,
                                                                                        (uint32_t)SurfaceGrid.GetHeight() + Neighborhood.GetHeight() - 1,
                                                                                        GetPixelType(),
                                                                                        (((uint32_t)SurfaceGrid.GetWidth() + Neighborhood.GetWidth() - 1) * GetPixelType()->CountPixelRawDataBits() + 7) / 8));
        HASSERT(pRenderedDescriptor != 0);

        HFCPtr<HGSSurface> pRenderedSurface(pio_rpSurface->CreateCompatibleSurface(pRenderedDescriptor));

        if (pRenderedSurface != 0)
            {
            try
                {
                // place the surface and scale it so it maps the same space as the original destination surface.
                HFCPtr<HGFMappedSurface> pMappedRenderedSurface(new HGFMappedSurface(pRenderedSurface->GetSurfaceDescriptor(),
                                                                                     0,
                                                                                     0,
                                                                                     pio_rpSurface->GetSurfaceCoordSys()));
                HASSERT(pMappedRenderedSurface != 0);

                pMappedRenderedSurface->Translate(HGF2DDisplacement((double)SurfaceGrid.GetXMin(), (double)SurfaceGrid.GetYMin()));

                if (!Neighborhood.IsUnity())
                    pMappedRenderedSurface->Translate(HGF2DDisplacement(- ((double)Neighborhood.GetXOrigin()),
                                                                        - ((double)Neighborhood.GetYOrigin())));

                // Clear the temp. surface using our pixeltype default raw data.
                HFCPtr<HGSEditor> pEditor = new HGSEditor();
                pEditor->SetFor(pRenderedSurface);

                pEditor->Clear(GetPixelType()->GetDefaultRawData());
                pEditor = 0;

                // Compose the filters together. We don't compose filters that use a neighborhood.
                HFCPtr<HRARaster> pSource(GetSource());
                HFCPtr<HRPFilter> pFilter(GetFilter());
                if (pFilter->GetNeighbourhood().IsUnity())
                    {
                    while (pSource->IsCompatibleWith(HIMFilteredImage::CLASS_ID) &&
                           ((HFCPtr<HIMFilteredImage>&)pSource)->GetFilter()->GetNeighbourhood().IsUnity())
                        {
                        pFilter = ((HFCPtr<HIMFilteredImage>&)pSource)->GetFilter()->ComposeWith(pFilter);
                        pSource = ((HFCPtr<HIMFilteredImage>&)pSource)->GetSource();
                        }
                    }

                HRADrawOptions  tempSurfaceOptions(Options);
                tempSurfaceOptions.SetAlphaBlend(false);  // Don't want alpha blend at this stage, we want to extract original pixels from the source.

                HVEShape TileShape(0.0, 0.0, pMappedRenderedSurface->GetSurfaceDescriptor()->GetWidth(), pMappedRenderedSurface->GetSurfaceDescriptor()->GetHeight(), pMappedRenderedSurface->GetSurfaceCoordSys());
                TileShape.Differentiate(*pSource->GetEffectiveShape());
                if (!TileShape.IsEmpty() && !Neighborhood.IsUnity())
                    {
                    // The temp. surface won't be filled completely. Since we do not know what has been filled
                    // we assume that top, bottom, left and right neighborhood are not filled.

                    // 1- We fill our temp surface without neighborhood.
                    HFCPtr<HVEShape> pFillShape(new HVEShape(Neighborhood.GetXOrigin(),
                                                             Neighborhood.GetYOrigin(),
                                                             Neighborhood.GetXOrigin() + (uint32_t)SurfaceGrid.GetWidth(),
                                                             Neighborhood.GetYOrigin() + (uint32_t)SurfaceGrid.GetHeight(),
                                                             pMappedRenderedSurface->GetSurfaceCoordSys()));
                    pFillShape->Intersect(*pSource->GetEffectiveShape());
                    tempSurfaceOptions.SetShape(pFillShape);
                    pSource->Draw(pMappedRenderedSurface, &tempSurfaceOptions);

                    // 2- From the filled surface we duplicate top, bottom, left and right to initialize the neighborhood pixels.
                    HandleBorderCases(pRenderedSurface, Neighborhood);

                    // 3- We redraw our temp surface but with neighborhood only so it fills what is available from the source.
                    HFCPtr<HVEShape> pSurfaceShape(new HVEShape(0.0, 0.0, pMappedRenderedSurface->GetSurfaceDescriptor()->GetWidth(), pMappedRenderedSurface->GetSurfaceDescriptor()->GetHeight(), pMappedRenderedSurface->GetSurfaceCoordSys()));
                    pSurfaceShape->Intersect(*pSource->GetEffectiveShape());
                    pSurfaceShape->Differentiate(*pFillShape);
                    tempSurfaceOptions.SetShape(pSurfaceShape);
                    pSource->Draw(pMappedRenderedSurface, &tempSurfaceOptions);
                    }
                else
                    {
                    // Fill the temp. surface with our source. Don't clip...
                    tempSurfaceOptions.SetShape(0);
                    pSource->Draw(pMappedRenderedSurface, &tempSurfaceOptions);
                    }

                // Draw temp. surface in destination. This is where the filter really gets applied.
                HFCPtr<HGSRegion> pOldClipRegion(static_cast<HGSRegion*>(pio_rpSurface->GetOption(HGSRegion::CLASS_ID).GetPtr()));
                pio_rpSurface->SetOption(new HGSRegion(new HVEShape(RegionToDraw), pio_rpSurface->GetSurfaceCoordSys()));

                HFCPtr<HGSBlitter> pBlitter(new HGSBlitter());
                if(Options.ApplyAlphaBlend())
                    pBlitter->AddAttribute(HGSColorConversionAttribute(HGSColorConversion::COMPOSE));

                if (Options.ApplyGridShape())
                    pBlitter->AddAttribute(HGSScanlinesAttribute(HGSScanlineMethod::GRID));

                pMappedRenderedSurface->SetOption(new HGSFilter(pFilter));

                HFCPtr<HGF2DTransfoModel> pTransfoModel(new HGF2DTranslation(HGF2DDisplacement(((double)(-SurfaceGrid.GetXMin() + Neighborhood.GetXOrigin())),
                                                                                               ((double)(-SurfaceGrid.GetYMin() + Neighborhood.GetYOrigin())))));

                pBlitter->SetFor((HFCPtr<HGSSurface>&)pio_rpSurface);

                // We're assuming
                // 1. Surfaces are compatible, since the super surface is the result of a CreateCompatibleSurface
                //    on the destination.
                // 2. The model is a stretch, since we only asked a scale between the two surfaces.

                if (pi_pOptions->IsCompatibleWith(HRADrawOptions::CLASS_ID))
                    pBlitter->BlitFrom(pMappedRenderedSurface, *pTransfoModel, ((HRADrawOptions*)pi_pOptions)->GetTransaction());
                else
                    pBlitter->BlitFrom(pMappedRenderedSurface, *pTransfoModel);

                if (pOldClipRegion == 0)
                    pio_rpSurface->RemoveOption(HGSRegion::CLASS_ID);
                else
                    pio_rpSurface->SetOption((const HFCPtr<HGSSurfaceOption>&)pOldClipRegion);

                DrawDone = true;
                }
            catch(HFCException Exception)
                {
                // We will fall through in the standard case
                if (Exception.GetID() != HFC_OUT_OF_MEMORY_EXCEPTION)
                    {
                    throw Exception;
                    }
                }
            }
        }

    // Standard draw code, one image at a time, bottom up
    if (!DrawDone)
        {
        HWARNING(0, L"HIMFilteredImage::Draw out of memory. Filter will not be applied.");

        GetSource()->Draw(pio_rpSurface, pi_pOptions);
        }
    }

//-----------------------------------------------------------------------------
// Duplicate outer data when the needed region (the surface) has not been
// completely filled with source data.
//-----------------------------------------------------------------------------
void HIMFilteredImage::HandleBorderCases(const HFCPtr<HGSSurface>&    pio_rpSurface,
                                         const HRPPixelNeighbourhood& pi_rNeighborhood) const
    {
    HFCPtr<HGSEditor> pEditor = new HGSEditor();
    pEditor->SetFor(pio_rpSurface);

    uint32_t SurfaceWidth = pio_rpSurface->GetSurfaceDescriptor()->GetWidth();
    uint32_t SurfaceHeight = pio_rpSurface->GetSurfaceDescriptor()->GetHeight();
    uint32_t SurfaceWidthWithoutNeighborhood = SurfaceWidth - (pi_rNeighborhood.GetWidth() - 1);
    uint32_t SurfaceHeightWithoutNeighborhood = SurfaceHeight - (pi_rNeighborhood.GetHeight() - 1);

    uint32_t NeighborhoodLeft = pi_rNeighborhood.GetXOrigin();
    uint32_t NeighborhoodRight = pi_rNeighborhood.GetWidth() - pi_rNeighborhood.GetXOrigin() - 1;
    uint32_t NeighborhoodBottom = pi_rNeighborhood.GetHeight() - pi_rNeighborhood.GetYOrigin() - 1;

    if (pi_rNeighborhood.GetHeight() > 1)
        {
        void* pSrcLine = 0;

        // Fill top
        for (uint32_t Line = 0 ; Line < pi_rNeighborhood.GetYOrigin() ; ++Line)
            {
            if(pSrcLine == 0)
                pSrcLine = pEditor->GetRun(pi_rNeighborhood.GetXOrigin(), pi_rNeighborhood.GetYOrigin(), SurfaceWidthWithoutNeighborhood);

            pEditor->SetRun(pi_rNeighborhood.GetXOrigin(), Line, SurfaceWidthWithoutNeighborhood, pSrcLine);
            }

        pSrcLine = 0;

        // Fill bottom
        for (uint32_t Line = SurfaceHeight - NeighborhoodBottom ; Line < SurfaceHeight ; ++Line)
            {
            if(pSrcLine == 0)
                pSrcLine = pEditor->GetRun(pi_rNeighborhood.GetXOrigin(), pi_rNeighborhood.GetYOrigin() + SurfaceHeightWithoutNeighborhood-1, SurfaceWidthWithoutNeighborhood);

            pEditor->SetRun(pi_rNeighborhood.GetXOrigin(), Line, SurfaceWidthWithoutNeighborhood, pSrcLine);
            }
        }

    if (pi_rNeighborhood.GetWidth() > 1)
        {
        for (uint32_t Line = 0 ; Line < SurfaceHeight ; ++Line)
            {
            // Fill left side
            if(NeighborhoodLeft > 0)
                {
                void* pLeftPixelValue = pEditor->GetPixel(pi_rNeighborhood.GetXOrigin(), Line);
                pEditor->ClearRun(0, Line, NeighborhoodLeft, pLeftPixelValue);
                }

            // Fill right side
            if(NeighborhoodRight > 0)
                {
                void* pRightPixelValue = pEditor->GetPixel(pi_rNeighborhood.GetXOrigin() + SurfaceWidthWithoutNeighborhood-1, Line);
                pEditor->ClearRun(pi_rNeighborhood.GetXOrigin() + SurfaceWidthWithoutNeighborhood,Line, NeighborhoodRight, pRightPixelValue);
                }
            }
        }
    }