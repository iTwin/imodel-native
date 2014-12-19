//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRADEMRaster.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HIMFilteredImage
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRADEMRaster.h>
#include <Imagepp/all/h/HFCGrid.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HGFMappedSurface.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGSRegion.h>
#include <Imagepp/all/h/HGSEditor.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HGSBlitter.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HRAStoredRaster.h>
#include <Imagepp/all/h/HRAReferenceToStoredRaster.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRPDEMFilter.h>
#include <Imagepp/all/h/HRADrawOptions.h>

HPM_REGISTER_CLASS(HRADEMRaster, HRAImageView)

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HRADEMRaster::HRADEMRaster()
    :   HRAImageView()
    {
    }

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HRADEMRaster::HRADEMRaster(const HFCPtr<HRAStoredRaster>& pi_pSource,
                           double                        pi_PixelSizeX,
                           double                        pi_PixelSizeY,
                           HFCPtr<HGF2DTransfoModel>      pi_pOrientationTransfo,
                           HFCPtr<HRPDEMFilter> const&    pi_Filter)
    :   HRAImageView(reinterpret_cast<HFCPtr<HRARaster>const&> (pi_pSource)),
        m_pFilter(new HRPDEMFilter(*pi_Filter)),        // Must keep our own copy because we "SetFor" for this specific source.
        m_pSourceStoredRaster(pi_pSource)
    {
    m_pFilter->SetFor(pi_pSource->GetPixelType(), pi_PixelSizeX, pi_PixelSizeY, pi_pOrientationTransfo);
    }

HRADEMRaster::HRADEMRaster(const HFCPtr<HRAReferenceToStoredRaster>& pi_pSource,
                           double                        pi_PixelSizeX,
                           double                        pi_PixelSizeY,
                           HFCPtr<HGF2DTransfoModel>      pi_pOrientationTransfo,
                           HFCPtr<HRPDEMFilter> const&    pi_Filter)
    :   HRAImageView(reinterpret_cast<HFCPtr<HRARaster>const&> (pi_pSource)),
        m_pFilter(new HRPDEMFilter(*pi_Filter))     // Must keep our own copy because we "SetFor" for this specific source.
    {
    HPRECONDITION(pi_pSource->GetSource()->IsCompatibleWith(HRAStoredRaster::CLASS_ID));

    m_pFilter->SetFor(pi_pSource->GetSource()->GetPixelType(), pi_PixelSizeX, pi_PixelSizeY, pi_pOrientationTransfo);
    m_pSourceStoredRaster = reinterpret_cast<HFCPtr<HRAStoredRaster>const&>(pi_pSource->GetSource());
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRADEMRaster::HRADEMRaster(const HRADEMRaster& pi_rObject)
    :   HRAImageView(pi_rObject),
        m_pSourceStoredRaster(pi_rObject.m_pSourceStoredRaster),
        m_pFilter(new HRPDEMFilter(*pi_rObject.m_pFilter))
    {
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HRADEMRaster::~HRADEMRaster()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HRARaster* HRADEMRaster::Clone (HPMObjectStore* pi_pStore,
                                HPMPool*        pi_pLog) const
    {
    return new HRADEMRaster(*this);
    }
//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HPMPersistentObject* HRADEMRaster::Clone () const
    {
    return new HRADEMRaster(*this);
    }

//-----------------------------------------------------------------------------
// public
// ContainsPixelsWithChannel
//-----------------------------------------------------------------------------
bool HRADEMRaster::ContainsPixelsWithChannel(HRPChannelType::ChannelRole pi_Role,
                                              Byte                      pi_Id) const
    {
    return GetPixelType()->GetChannelOrg().GetChannelIndex(pi_Role, pi_Id) != HRPChannelType::FREE;
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRADEMRaster::GetPixelType() const
    {
    return m_pFilter->GetOutputPixelType();
    }

//-----------------------------------------------------------------------------
// PreDraw
//-----------------------------------------------------------------------------
void HRADEMRaster::PreDraw(HRADrawOptions* pio_pOptions)
    {
    GetSource()->PreDraw(pio_pOptions);
    }

//-----------------------------------------------------------------------------
// Draw
//-----------------------------------------------------------------------------
void HRADEMRaster::Draw(const HFCPtr<HGFMappedSurface>& pio_rpSurface,
                        const HGFDrawOptions* pi_pOptions) const
    {
    HPRECONDITION(pio_rpSurface != 0);

    bool DrawDone = false;

    HRADrawOptions Options(pi_pOptions);
    Options.SetTransaction(0);  // record only when we draw into pio_rpSurface

    HVEShape RegionToDraw(Options.GetShape() != 0 ? *Options.GetShape() : *GetEffectiveShape());
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
        HASSERT(*m_pFilter->GetOutputPixelType() == *GetPixelType());

        HRPPixelNeighbourhood Neighborhood = m_pFilter->GetNeighbourhood();

        // In order for our HandleBorderCases method to work, we must create a temporary surface that is not bigger than the source.
        HVEShape RegionToDrawInSurfaceCS(RegionToDraw);
        RegionToDrawInSurfaceCS.ChangeCoordSys(pio_rpSurface->GetSurfaceCoordSys());
        HGF2DExtent SurfaceExtent(RegionToDrawInSurfaceCS.GetExtent());
        HFCGrid     SurfaceGrid(SurfaceExtent.GetOrigin().GetX(), SurfaceExtent.GetOrigin().GetY(), SurfaceExtent.GetCorner().GetX(), SurfaceExtent.GetCorner().GetY());

        // Create temp surface with neighborhood.
        HFCPtr<HGSMemorySurfaceDescriptor> pTempSurfaceDesc(new HGSMemorySurfaceDescriptor(
                                                                (uint32_t)SurfaceGrid.GetWidth() + Neighborhood.GetWidth() - 1,
                                                                (uint32_t)SurfaceGrid.GetHeight() + Neighborhood.GetHeight() - 1,
                                                                GetSource()->GetPixelType(),
                                                                (((uint32_t)SurfaceGrid.GetWidth() + Neighborhood.GetWidth() - 1) * GetSource()->GetPixelType()->CountPixelRawDataBits() + 7) / 8));
        HASSERT(pTempSurfaceDesc != 0);

        HFCPtr<HGSSurface> pTempSurface(pio_rpSurface->CreateCompatibleSurface(pTempSurfaceDesc.GetPtr()));

        if (pTempSurface != 0)
            {
            try
                {
                // place the surface and scale it so it maps the same space as the original destination surface.
                HFCPtr<HGFMappedSurface> pMappedTempSurface(new HGFMappedSurface(pTempSurface->GetSurfaceDescriptor(),
                                                                                 0,
                                                                                 0,
                                                                                 pio_rpSurface->GetSurfaceCoordSys()));
                HASSERT(pMappedTempSurface != 0);

                pMappedTempSurface->Translate(HGF2DDisplacement((double)SurfaceGrid.GetXMin(), (double)SurfaceGrid.GetYMin()));

                if (!Neighborhood.IsUnity())
                    pMappedTempSurface->Translate(HGF2DDisplacement(- ((double)Neighborhood.GetXOrigin()), -((double)Neighborhood.GetYOrigin())));

                // Clear the temp. surface using our pixeltype default raw data.
                HFCPtr<HGSEditor> pEditor = new HGSEditor();
                pEditor->SetFor(pTempSurface);

                pEditor->Clear(GetSource()->GetPixelType()->GetDefaultRawData());
                pEditor = 0;

                HRADrawOptions  tempSurfaceOptions(Options);
                tempSurfaceOptions.SetAlphaBlend(false);  // Don't want alpha blend at this stage, we want to extract original pixels from the source.

                HVEShape TileShape(0.0, 0.0, pMappedTempSurface->GetSurfaceDescriptor()->GetWidth(), pMappedTempSurface->GetSurfaceDescriptor()->GetHeight(), pMappedTempSurface->GetSurfaceCoordSys());
                TileShape.Differentiate(*GetSource()->GetEffectiveShape());
                if (!TileShape.IsEmpty() && !Neighborhood.IsUnity())
                    {
                    // The temp. surface won't be filled completely. Since we do not know what has been filled
                    // we assume that top, bottom, left and right neighborhood are not filled.

                    // 1- We fill our temp surface without neighborhood.
                    HFCPtr<HVEShape> pFillShape(new HVEShape(Neighborhood.GetXOrigin(),
                                                             Neighborhood.GetYOrigin(),
                                                             Neighborhood.GetXOrigin() + (uint32_t)SurfaceGrid.GetWidth(),
                                                             Neighborhood.GetYOrigin() + (uint32_t)SurfaceGrid.GetHeight(),
                                                             pMappedTempSurface->GetSurfaceCoordSys()));
                    pFillShape->Intersect(*GetSource()->GetEffectiveShape());
                    tempSurfaceOptions.SetShape(pFillShape);
                    GetSource()->Draw(pMappedTempSurface, &tempSurfaceOptions);

                    // 2- From the filled surface we duplicate top, bottom, left and right to initialize the neighborhood pixels.
                    HandleBorderCases(pTempSurface, Neighborhood);

                    // 3- We redraw our temp surface but with neighborhood only so it fills what is available from the source.
                    HFCPtr<HVEShape> pSurfaceShape(new HVEShape(0.0, 0.0, pMappedTempSurface->GetSurfaceDescriptor()->GetWidth(), pMappedTempSurface->GetSurfaceDescriptor()->GetHeight(), pMappedTempSurface->GetSurfaceCoordSys()));
                    pSurfaceShape->Intersect(*GetSource()->GetEffectiveShape());
                    pSurfaceShape->Differentiate(*pFillShape);
                    tempSurfaceOptions.SetShape(pSurfaceShape);
                    GetSource()->Draw(pMappedTempSurface, &tempSurfaceOptions);
                    }
                else
                    {
                    // Fill the temp. surface with our source. Don't clip...
                    tempSurfaceOptions.SetShape(0);
                    GetSource()->Draw(pMappedTempSurface, &tempSurfaceOptions);
                    }

                HASSERT(pTempSurface->GetSurfaceDescriptor()->IsCompatibleWith(HGSMemorySurfaceDescriptor::CLASS_ID));


                // Apply filtering to source pixels.
                HFCPtr<HGSMemorySurfaceDescriptor> pFilteredSurfaceDesc = ApplyFilter((HGSMemorySurfaceDescriptor*)pTempSurface->GetSurfaceDescriptor().GetPtr(),
                                                                                      ComputeSourceToDestinationTransfo(pio_rpSurface->GetCoordSys(), Options.GetReplacingCoordSys()));

                // Draw temp. surface in destination. This is where the filter really gets applied.
                HFCPtr<HGSRegion> pOldClipRegion(static_cast<HGSRegion*>(pio_rpSurface->GetOption(HGSRegion::CLASS_ID).GetPtr()));
                pio_rpSurface->SetOption(new HGSRegion(new HVEShape(RegionToDraw), pio_rpSurface->GetSurfaceCoordSys()));

                HFCPtr<HGSBlitter> pBlitter(new HGSBlitter());
                if(Options.ApplyAlphaBlend())
                    pBlitter->AddAttribute(HGSColorConversionAttribute(HGSColorConversion::COMPOSE));

                if (Options.ApplyGridShape())
                    pBlitter->AddAttribute(HGSScanlinesAttribute(HGSScanlineMethod::GRID));

                HFCPtr<HGF2DTransfoModel> pTransfoModel(new HGF2DIdentity);

                HASSERT(pFilteredSurfaceDesc != 0);

                pBlitter->SetFor((HFCPtr<HGSSurface>&)pio_rpSurface);

                HFCPtr<HGSSurface> pFilteredSurface(pio_rpSurface->CreateCompatibleSurface(pFilteredSurfaceDesc.GetPtr()));

                // We're assuming
                // 1. Surfaces are compatible, since the super surface is the result of a CreateCompatibleSurface
                //    on the destination.
                // 2. The model is a stretch, since we only asked a scale between the two surfaces.

                if (pi_pOptions->IsCompatibleWith(HRADrawOptions::CLASS_ID))
                    pBlitter->BlitFrom(pFilteredSurface, *pTransfoModel, ((HRADrawOptions*)pi_pOptions)->GetTransaction());
                else
                    pBlitter->BlitFrom(pFilteredSurface, *pTransfoModel);

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
        HWARNING(0, L"HRADEMRaster::Draw out of memory. Filter will not be applied.");

        GetSource()->Draw(pio_rpSurface, pi_pOptions);
        }
    }

//-----------------------------------------------------------------------------
// Duplicate outer data when the needed region (the surface) has not been
// completely filled with source data.
//-----------------------------------------------------------------------------
void HRADEMRaster::HandleBorderCases(const HFCPtr<HGSSurface>&    pio_rpSurface,
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
//-----------------------------------------------------------------------------
// ApplyFilter
//-----------------------------------------------------------------------------
HFCPtr<HGSMemorySurfaceDescriptor> HRADEMRaster::ApplyFilter(HFCPtr<HGSMemorySurfaceDescriptor> pSourceSurfaceDesc, HFCPtr<HGF2DTransfoModel> pi_pSourceTransfo) const
    {
    // Create temp surface that will hold the filtered buffer.
    uint32_t DstWidth = pSourceSurfaceDesc->GetWidth() - (m_pFilter->GetNeighbourhood().GetWidth() - 1);
    uint32_t DstHeight = pSourceSurfaceDesc->GetHeight() - (m_pFilter->GetNeighbourhood().GetHeight() - 1);
    uint32_t DstBytesPerRow = (DstWidth * GetPixelType()->CountPixelRawDataBits() + 7) / 8;
    uint32_t DstBufferSize = DstBytesPerRow * DstHeight;

    // allocate a buffer for dst
    HFCPtr<HCDPacket> pPacket(new HCDPacket(new HCDCodecIdentity(), new Byte[DstBufferSize], DstBufferSize, DstBufferSize));
    pPacket->SetBufferOwnership(true);

    HFCPtr<HGSMemorySurfaceDescriptor> pDestSurfaceDesc(new HGSMemorySurfaceDescriptor(DstWidth, DstHeight, GetPixelType(), pPacket,
                                                        HGF_UPPER_LEFT_HORIZONTAL, DstBytesPerRow));
    // Compute scaling between source and destination
    double            ScaleFactorX;
    double            ScaleFactorY;
    HGF2DDisplacement  Displacement;
    pi_pSourceTransfo->GetStretchParams (&ScaleFactorX, &ScaleFactorY, &Displacement);

    m_pFilter->ProcessPixels(pDestSurfaceDesc, pSourceSurfaceDesc, ScaleFactorX, ScaleFactorY);

    return pDestSurfaceDesc;
    }

//-----------------------------------------------------------------------------
// ComputeSourceToDestinationTransfo
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>   HRADEMRaster::ComputeSourceToDestinationTransfo(const HFCPtr<HGF2DCoordSys>& pi_rPhysicalCoordSys,
                                                                            const HFCPtr<HGF2DCoordSys>  pi_pNewLogicalCoordSys) const
    {
    if (pi_pNewLogicalCoordSys != 0)
        {
        HFCPtr<HGF2DTransfoModel> pTransfo = pi_rPhysicalCoordSys->GetTransfoModelTo (pi_pNewLogicalCoordSys);
        return pTransfo->ComposeInverseWithDirectOf(*GetCoordSys()->GetTransfoModelTo(m_pSourceStoredRaster->GetPhysicalCoordSys()));
        }

    return pi_rPhysicalCoordSys->GetTransfoModelTo (m_pSourceStoredRaster->GetPhysicalCoordSys());

    }