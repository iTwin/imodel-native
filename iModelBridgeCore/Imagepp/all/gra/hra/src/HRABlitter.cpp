//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRABlitter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//:>---------------------------------------------------------------------------------------
//:> Class HRABlitter
//:>---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRABlitter.h>

#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HRAEditor.h>
#include <Imagepp/all/h/HRPFilter.h>
#include <Imagepp/all/h/HGFScanlines.h>
#include <Imagepp/all/h/HRATransaction.h>

#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTranslation.h>


#include <Imagepp/all/h/HRASampler.h>
#include <Imagepp/all/h/HGStypes.h>
#include <Imagepp/all/h/HGSRegion.h>


//:>---------------------------------------------------------------------------------------
//:> public section
//:>---------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRABlitter::HRABlitter(HRASurface& pi_destSurface)
    : m_destSurface(pi_destSurface),
      m_ComposeRequired(false),
      m_ApplyGrid(false),
      m_samplingMode(HGSResampling::NEAREST_NEIGHBOUR),
      m_overviewsMode(false)
    {
    HPRECONDITION(m_destSurface.GetSurfaceDescriptor() != 0);
    HPRECONDITION(m_destSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));

    const HFCPtr<HGSMemoryBaseSurfaceDescriptor> rpSurfaceDescriptor =
        (const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)m_destSurface.GetSurfaceDescriptor();
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRABlitter::~HRABlitter()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRABlitter::SetAlphaBlend(bool alphaBlend)
    {
    m_ComposeRequired = alphaBlend;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRABlitter::SetGridMode(bool gridMode)
    {
    m_ApplyGrid = gridMode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRABlitter::SetSamplingMode(HGSResampling const& samplingMode)
    {
    m_samplingMode = samplingMode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRABlitter::SetOverviewsMode(bool overviews)
    {
    m_overviewsMode = overviews;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRABlitter::SetFilter(HFCPtr<HRPFilter> const& pFilter)
    {
    m_pFilter = pFilter;
    }

//-----------------------------------------------------------------------------
// public
// Blit
//-----------------------------------------------------------------------------
void HRABlitter::BlitFrom(const HRASurface&                 pi_srcSurface,
                          const HGF2DTransfoModel&          pi_rTransfoModel,
                          HRATransaction*                   pi_pTransaction)
    {
    HPRECONDITION(pi_srcSurface.GetSurfaceDescriptor() != 0);
    HPRECONDITION(pi_srcSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));
    HPRECONDITION(GetDestSurface().GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));
    HPRECONDITION(pi_rTransfoModel.IsStretchable());

    HFCPtr<HGSMemoryBaseSurfaceDescriptor> pSrcSurfaceDescriptor =
        (const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)pi_srcSurface.GetSurfaceDescriptor();
    HFCPtr<HGSMemoryBaseSurfaceDescriptor> pDstSurfaceDescriptor =
        (const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)GetDestSurface().GetSurfaceDescriptor();

    bool DstClipped=false;

    // check if we need to clip
    HFCPtr<HGSRegion> pDstClipRegion;
    HFCPtr<HGSRegion> pSrcClipRegion(pi_srcSurface.GetRegion());
    if (pSrcClipRegion != 0)
        {
        // the source is clipped, we need to clip the destination
        pDstClipRegion = GetDestSurface().GetRegion();
        if (pDstClipRegion == 0)
            {
            DstClipped = false;
            // the destination was not clipped
            HFCPtr<HGSRegion> pClipRegion = new HGSRegion(pSrcClipRegion,
                                                          pi_rTransfoModel);

            double aDstShape[4];
            aDstShape[0] = 0.0;
            aDstShape[1] = 0.0;
            aDstShape[2] = (double)pDstSurfaceDescriptor->GetWidth();
            aDstShape[3] = (double)pDstSurfaceDescriptor->GetHeight();

            pClipRegion->AddOperation(HGSRegion::INTERSECT,
                                      aDstShape,
                                      4);

            GetDestSurface().SetRegion(pClipRegion);
            }
        else
            {
            // Special case: If source and destionation are the same surface,
            // don't re-clip.
            if (pSrcSurfaceDescriptor != pDstSurfaceDescriptor)
                {
                DstClipped = true;

                pDstClipRegion->AddOperation(HGSRegion::INTERSECT,
                                             pSrcClipRegion);
                }
            else
                {
                DstClipped = false;
                }
            }
        }

    // check if we have a filter on the source
    const HRASurface* pSrcSurface;
    HAutoPtr<HRASurface> pFilteredSurface;

    HGF2DDisplacement   Translation;
    double             ScaleX;
    double             ScaleY;
    pi_rTransfoModel.GetStretchParams(&ScaleX, &ScaleY, &Translation);
    HAutoPtr<HGF2DStretch> pStretchModel(new HGF2DStretch(Translation, ScaleX, ScaleY));

    if (m_pFilter.GetPtr() != NULL)
        {
        pFilteredSurface = ApplyFilter(pi_srcSurface, m_pFilter);
        pSrcSurface = pFilteredSurface;

        // translate the surface
        const HRPPixelNeighbourhood& rNeighbourhood = m_pFilter->GetNeighbourhood();
        if (rNeighbourhood.GetXOrigin() != 0 || rNeighbourhood.GetYOrigin() != 0)
            {
            Translation.SetDeltaX(Translation.GetDeltaX() - (double)m_pFilter->GetNeighbourhood().GetXOrigin());
            Translation.SetDeltaY(Translation.GetDeltaY() - (double)m_pFilter->GetNeighbourhood().GetYOrigin());

            pStretchModel->SetTranslation(Translation);
            }
        }
    else
        {
        pSrcSurface = &pi_srcSurface;
        }


    if ((pDstSurfaceDescriptor->GetPixelType()->CountPixelRawDataBits() % 8) == 0 &&
        pDstSurfaceDescriptor->GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID))
        Optimized8BitsBlit(*pSrcSurface, *pStretchModel, pi_pTransaction);
    else
        NormalBlit(*pSrcSurface, *pStretchModel, pi_pTransaction);

    // check if we have shaped the destination
    if (pSrcClipRegion != 0)
        {
        if (DstClipped)
            pDstClipRegion->RemoveLastOperation();
        else
            {
            GetDestSurface().SetRegion(NULL);
            }
        }
    }


//:>---------------------------------------------------------------------------------------
//:> protected section
//:>---------------------------------------------------------------------------------------

//:>---------------------------------------------------------------------------------------
//:> private section
//:>---------------------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 Blit a 8 bits destination

 @param pi_srcSurface
 @param pi_rTransfoModel
-----------------------------------------------------------------------------*/
void HRABlitter::Optimized8BitsBlit(const HRASurface&               pi_srcSurface,
                                    const HGF2DStretch&             pi_rTransfoModel,
                                    HRATransaction*                 pi_pTransaction)
    {
    HPRECONDITION(pi_srcSurface.GetSurfaceDescriptor() != 0);
    HPRECONDITION(pi_srcSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));

    HPRECONDITION(GetDestSurface().GetSurfaceDescriptor() != 0);
    HPRECONDITION(GetDestSurface().GetSurfaceDescriptor()->IsCompatibleWith(HGSMemorySurfaceDescriptor::CLASS_ID));

    double PixelOffset(0.5);
    if (m_overviewsMode && HDOUBLE_EQUAL_EPSILON(fabs(pi_rTransfoModel.GetXScaling()), 2.0))
        {
        PixelOffset = 0.15;
        }

    const HFCPtr<HGSMemoryBaseSurfaceDescriptor> pSrcSurfaceDescriptor =
        (const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)pi_srcSurface.GetSurfaceDescriptor();


    const HFCPtr<HGSMemorySurfaceDescriptor> pDstSurfaceDescriptor =
        (const HFCPtr<HGSMemorySurfaceDescriptor>&)GetDestSurface().GetSurfaceDescriptor();
    HPRECONDITION(pDstSurfaceDescriptor->GetPacket() != 0);
    HPRECONDITION(pDstSurfaceDescriptor->GetPacket()->GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID));


    // the source sampler is read only
    HRASampler SrcSampler(m_samplingMode,
                          *pSrcSurfaceDescriptor,
                          HGF2DRectangle(0.0,
                                         0.0,
                                         fabs(pi_rTransfoModel.GetXScaling()),
                                         fabs(pi_rTransfoModel.GetYScaling())),
                          pi_rTransfoModel.GetXScaling(),
                          0.0);

    HFCPtr<HRPPixelType> pSourcePixelType(pSrcSurfaceDescriptor->GetPixelType());

    if (SrcSampler.GetOutputPixelType() != 0)
        {
        // Try to minimize pixel conversions by asking the sampler
        // to use the destination pixeltype directly.
        if (!m_ComposeRequired)
            SrcSampler.TryToUse(pDstSurfaceDescriptor->GetPixelType());

        pSourcePixelType = SrcSampler.GetOutputPixelType();
        }

    HFCPtr<HRPPixelConverter> pConverter;
    HArrayAutoPtr<Byte> pSrcRun;
    // this pointer will be set with the method use to convert the source data
    // to the destination data if necessary
    void (HRPPixelConverter::* pConvertMethod)(const void* pi_pSrcRawData,
                                               void*       pi_pDstRawData,
                                               size_t      pi_PixelCount) const = 0;

    if (m_ComposeRequired ||
        !pSourcePixelType->HasSamePixelInterpretation(*pDstSurfaceDescriptor->GetPixelType()))
        {
        pConverter = pSourcePixelType->GetConverterTo(pDstSurfaceDescriptor->GetPixelType());

        pSrcRun = CreateWorkingBuffer(*pSourcePixelType,
                                      pDstSurfaceDescriptor->GetWidth(),
                                      1);

        if (m_ComposeRequired)
            pConvertMethod = &HRPPixelConverter::Compose;   // use the Compose method
        else
            pConvertMethod = &HRPPixelConverter::Convert;   // use the Convert method
        }


    // use directly the scanline
    HRAEditor DstEditor(GetDestSurface(), m_ApplyGrid);

    HUINTX  StartPosX;
    HUINTX  StartPosY;
    size_t  PixelCount;

    double PosX;
    double PosY;


    Byte* pDstRun = (Byte*)DstEditor.GetFirstRun(&StartPosX,
                                                     &StartPosY,
                                                     &PixelCount,
                                                     pi_pTransaction);

    // if we have a transaction and the blit width is equal to the surface width,
    // record the entire bloc instead of line by line
    if (pi_pTransaction != 0 &&
        (DstEditor.GetScanlines() == 0 || DstEditor.GetScanlines()->IsRectangle() && PixelCount == pDstSurfaceDescriptor->GetWidth()))
        {
        uint32_t Height = (uint32_t)(DstEditor.GetScanlines() == 0 ? pDstSurfaceDescriptor->GetHeight() : DstEditor.GetScanlines()->GetScanlineCount());
        HUINTX OffsetX;
        HUINTX OffsetY;
        pDstSurfaceDescriptor->GetOffsets(&OffsetX, &OffsetY);
        pi_pTransaction->PushEntry(StartPosX + OffsetX,
                                   StartPosY + OffsetY,
                                   (uint32_t)PixelCount,
                                   Height,
                                   Height * pDstSurfaceDescriptor->GetBytesPerRow(),
                                   pDstRun);
        pi_pTransaction = 0;
        }

    if (!m_ApplyGrid || DstEditor.GetScanlines() == 0)
        {
        if (pConverter != 0)
            {
            HASSERT(pConvertMethod != 0);

            while (pDstRun != 0)
                {
                // convert the starting position
                pi_rTransfoModel.ConvertDirect((double)StartPosX + PixelOffset,
                                               (double)StartPosY + PixelOffset,
                                               &PosX,
                                               &PosY);

                BeAssertOnce(PosX >= 0.0);
                BeAssertOnce(PosY >= 0.0);
                HASSERT(PixelCount <= pDstSurfaceDescriptor->GetWidth());
                SrcSampler.GetPixels(PosX,
                                     PosY,
                                     PixelCount,
                                     pSrcRun);

                // call the convert method set before
                (pConverter->*pConvertMethod)(pSrcRun, pDstRun, PixelCount);

                pDstRun = (Byte*)DstEditor.GetNextRun(&StartPosX,
                                                       &StartPosY,
                                                       &PixelCount,
                                                       pi_pTransaction);
                }
            }
        else
            {
            while (pDstRun != 0)
                {
                // convert the starting position
                pi_rTransfoModel.ConvertDirect((double)StartPosX + PixelOffset,
                                               (double)StartPosY + PixelOffset,
                                               &PosX,
                                               &PosY);

                HASSERT(PosX >= 0.0);
                HASSERT(PosY >= 0.0);

                SrcSampler.GetPixels(PosX,
                                     PosY,
                                     PixelCount,
                                     pDstRun);

                pDstRun = (Byte*)DstEditor.GetNextRun(&StartPosX,
                                                       &StartPosY,
                                                       &PixelCount,
                                                       pi_pTransaction);
                }
            }
        }
    else
        {
        // In this case, we have scanlines that are using the loose
        // pixel selection method. We must make sure that the sampling
        // coordinates don't go outside the source data.

        void (HRPPixelConverter::* pConvertMethod)(const void* pi_pSrcRawData,
                                                   void*       pi_pDstRawData,
                                                   size_t      pi_PixelCount) const = 0;

        if (pConverter != 0)
            {
            if (m_ComposeRequired)
                pConvertMethod = &HRPPixelConverter::Compose;   // use the Compose method
            else
                pConvertMethod = &HRPPixelConverter::Convert;   // use the Convert method
            }

        const HGFScanLines* pScanlines = DstEditor.GetScanlines();

        double RealY;

        HUINTX   StartPosX;
        HUINTX   StartPosY;
        double  PosX;
        double  PosY;
        void* pDstRun;

        double ScaleX = fabs(pi_rTransfoModel.GetXScaling());
        double ScaleY = fabs(pi_rTransfoModel.GetYScaling());

        // we need to use GetRun onto the destination
        pDstRun = DstEditor.GetFirstRun(&StartPosX, &StartPosY, &PixelCount, pi_pTransaction);

        while (pDstRun != 0)
            {
            HASSERT(PixelCount <= pDstSurfaceDescriptor->GetWidth());

            RealY = pScanlines->AdjustYValueForCurrentRun((double)StartPosY + 0.5);

            // convert the starting position
            pi_rTransfoModel.ConvertDirect(pScanlines->AdjustXValueForCurrentRun((double)StartPosX + PixelOffset),
                                           RealY,
                                           &PosX,
                                           &PosY);

            // with loose pixel selection method, PosX and PosY can be smaller than 0.0
            PosX = MIN(MAX(PosX, 0.0), (double)(pSrcSurfaceDescriptor->GetWidth()) - HGLOBAL_EPSILON);
            PosY = MIN(MAX(PosY, 0.0), (double)(pSrcSurfaceDescriptor->GetHeight()) - HGLOBAL_EPSILON);

            if ((PosX + (double)PixelCount * ScaleX) >= (double)pSrcSurfaceDescriptor->GetWidth())
                {
                // change the scale to stay inside the source surface
                double NewScaleX = (((double)pSrcSurfaceDescriptor->GetWidth() - HGLOBAL_EPSILON) - (double)PosX) / (double)PixelCount;
                SrcSampler.SetScale(NewScaleX, ScaleY);
                }
            else
                SrcSampler.SetScale(ScaleX, ScaleY);



            if (pConverter)
                {
                SrcSampler.GetPixels(PosX,
                                     PosY,
                                     PixelCount,
                                     pSrcRun);

                // call the convert method setted before
                (pConverter->*pConvertMethod)(pSrcRun, pDstRun, PixelCount);
                }
            else
                {
                SrcSampler.GetPixels(PosX,
                                     PosY,
                                     PixelCount,
                                     pDstRun);
                }

            pDstRun = (Byte*)DstEditor.GetNextRun(&StartPosX,
                                                   &StartPosY,
                                                   &PixelCount,
                                                   pi_pTransaction);
            }
        }
    }


/**----------------------------------------------------------------------------
 NormalBlit

 @param pi_srcSurface
 @param pi_rTransfoModel
-----------------------------------------------------------------------------*/
void HRABlitter::NormalBlit(const HRASurface&               pi_srcSurface,
                            const HGF2DStretch&             pi_rTransfoModel,
                            HRATransaction*                 pi_pTransaction)
    {
    HPRECONDITION(pi_srcSurface.GetSurfaceDescriptor() != 0);
    HPRECONDITION(pi_srcSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));

    HPRECONDITION(GetDestSurface().GetSurfaceDescriptor() != 0);
    HPRECONDITION(GetDestSurface().GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));

    const HFCPtr<HGSMemoryBaseSurfaceDescriptor> pSrcSurfaceDescriptor =
        (const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)pi_srcSurface.GetSurfaceDescriptor();


    const HFCPtr<HGSMemoryBaseSurfaceDescriptor> pDstSurfaceDescriptor =
        (const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)GetDestSurface().GetSurfaceDescriptor();

    // the source sampler is read only
    HRASampler SrcSampler(m_samplingMode,
                          *pSrcSurfaceDescriptor,
                          HGF2DRectangle(0.0,
                                         0.0,
                                         fabs(pi_rTransfoModel.GetXScaling()),
                                         fabs(pi_rTransfoModel.GetYScaling())),
                          pi_rTransfoModel.GetXScaling(),
                          0.0);

    HFCPtr<HRPPixelType> pSourcePixelType(pSrcSurfaceDescriptor->GetPixelType());

    if (SrcSampler.GetOutputPixelType() != 0)
        {
        // Try to minimize pixel conversions by asking the sampler
        // to use the destination pixeltype directly.
        if (!m_ComposeRequired)
            SrcSampler.TryToUse(pDstSurfaceDescriptor->GetPixelType());

        pSourcePixelType = SrcSampler.GetOutputPixelType();
        }

    HFCPtr<HRPPixelConverter> pConverter;

    // check if we need a converter
    if (m_ComposeRequired || !pSourcePixelType->HasSamePixelInterpretation(*pDstSurfaceDescriptor->GetPixelType()))
        // the converter is neccessary, create it
        pConverter = pSourcePixelType->GetConverterTo(pDstSurfaceDescriptor->GetPixelType());

    // the source and destination have the same pixel type, we don't have converter
    HArrayAutoPtr<Byte> pSrcRun(CreateWorkingBuffer(*pSourcePixelType,
                                                     pDstSurfaceDescriptor->GetWidth(),
                                                     1));

    // create the destination editor
    HRAEditor DstEditor(GetDestSurface());

    size_t  PixelCount;

    double PosX;
    double PosY;
    double PixelOffset = 0.5;

    if (!m_ApplyGrid || DstEditor.GetScanlines() == 0)
        {
        // for optimization, check if we need to call GetRun() onto destination
        // We need the destination data to compose pixels
        if (m_ComposeRequired)
            {
            HUINTX   StartPosX;
            HUINTX   StartPosY;
            void* pDstRun;

            // we need to use GetRun onto the destination
            pDstRun = DstEditor.GetFirstRun(&StartPosX, &StartPosY, &PixelCount, pi_pTransaction);

            while (pDstRun != 0)
                {
                HASSERT(StartPosX >= 0);
                HASSERT(StartPosY >= 0);

                // convert the destination run into the source coordinates
                pi_rTransfoModel.ConvertDirect((double)StartPosX + PixelOffset,
                                               (double)StartPosY + PixelOffset,
                                               &PosX,
                                               &PosY);

                HASSERT(PosX >= 0.0);
                HASSERT(PosY >= 0.0);

                // get the source pixel
                SrcSampler.GetPixels(PosX,
                                     PosY,
                                     PixelCount,
                                     pSrcRun);

                pConverter->Compose(pSrcRun, pDstRun, PixelCount);
                DstEditor.SetRun(StartPosX, StartPosY, PixelCount, pDstRun);

                pDstRun = DstEditor.GetNextRun(&StartPosX, &StartPosY, &PixelCount, pi_pTransaction);
                }
            }
        else
            {
            HSINTX   StartPosX;
            HSINTX   StartPosY;
            const HGFScanLines* pScanlines;
            pScanlines = DstEditor.GetScanlines();

            if (pScanlines != 0)
                {
                // we have a shape, we work with scanlines
                if (const_cast<HGFScanLines*>(pScanlines)->GotoFirstRun())
                    {
                    HArrayAutoPtr<Byte> pDstRun;

                    // test if we have a converter
                    if (pConverter != 0)
                        {
                        // create buffer to convert the source data into the destination
                        pDstRun = CreateWorkingBuffer(*pDstSurfaceDescriptor->GetPixelType(),
                                                      pDstSurfaceDescriptor->GetWidth(),
                                                      1);
                        }

                    do
                        {
                        pScanlines->GetCurrentRun(&StartPosX, &StartPosY, &PixelCount);
                        HPOSTCONDITION(StartPosX >= 0);
                        HPOSTCONDITION(StartPosY >= 0);

                        // convert the destination run into the source coordinates
                        pi_rTransfoModel.ConvertDirect((double)StartPosX + PixelOffset,
                                                       (double)StartPosY + PixelOffset,
                                                       &PosX,
                                                       &PosY);

                        HPOSTCONDITION(PosX >= 0.0);
                        HPOSTCONDITION(PosY >= 0.0);

                        // get the source pixel
                        SrcSampler.GetPixels(PosX,
                                             PosY,
                                             PixelCount,
                                             pSrcRun);

                        if (pConverter != 0)
                            {
                            // convert the source pixel directly into the destination run
                            pConverter->Convert(pSrcRun, pDstRun, PixelCount);

                            DstEditor.SetRun(StartPosX, StartPosY, PixelCount, pDstRun, pi_pTransaction);
                            }
                        else
                            {
                            // the source and destination pixeltype are the same
                            DstEditor.SetRun(StartPosX, StartPosY, PixelCount, pSrcRun, pi_pTransaction);
                            }

                        }
                    while (const_cast<HGFScanLines*>(pScanlines)->GotoNextRun());
                    }
                }
            else
                {
                // no scanline, the destination has no shape
                PixelCount = pDstSurfaceDescriptor->GetWidth();
                HArrayAutoPtr<Byte> pDstRun;

                if (pConverter != 0)
                    {
                    // create buffer to convert the source data into the destination
                    pDstRun = CreateWorkingBuffer(*pDstSurfaceDescriptor->GetPixelType(),
                                                  pDstSurfaceDescriptor->GetWidth(),
                                                  1);
                    }

                for (uint32_t i = 0; i < pDstSurfaceDescriptor->GetHeight(); i++)
                    {
                    pi_rTransfoModel.ConvertDirect(PixelOffset,
                                                   (double)i + PixelOffset,
                                                   &PosX,
                                                   &PosY);

                    HPOSTCONDITION(PosX >= 0.0);
                    HPOSTCONDITION(PosY >= 0.0);

                    // get the source pixel
                    SrcSampler.GetPixels(PosX,
                                         PosY,
                                         PixelCount,
                                         pSrcRun);

                    if (pConverter != 0)
                        {
                        // convert the source data into the destination pixeltype
                        pConverter->Convert(pSrcRun, pDstRun, PixelCount);

                        // write the run into the destination
                        DstEditor.SetRun(0, i, PixelCount, pDstRun, pi_pTransaction);
                        }
                    else
                        {
                        // write the run into the destination
                        DstEditor.SetRun(0, i, PixelCount, pSrcRun, pi_pTransaction);
                        }
                    }
                }
            }
        }
    else
        {
        // In this case, we have scanlines that are using the loose
        // pixel selection method. We must make sure that the sampling
        // coordinates don't go outside the source data.

        void (HRPPixelConverter::* pConvertMethod)(const void* pi_pSrcRawData,
                                                   void*       pi_pDstRawData,
                                                   size_t      pi_PixelCount) const = 0;

        if (pConverter != 0)
            {
            if (m_ComposeRequired)
                pConvertMethod = &HRPPixelConverter::Compose;   // use the Compose method
            else
                pConvertMethod = &HRPPixelConverter::Convert;   // use the Convert method
            }

        const HGFScanLines* pScanlines = DstEditor.GetScanlines();

        double RealY;

        HUINTX   StartPosX;
        HUINTX   StartPosY;
        double  PosX;
        double  PosY;

        void* pDstRun;

        double ScaleX = fabs(pi_rTransfoModel.GetXScaling());
        double ScaleY = fabs(pi_rTransfoModel.GetYScaling());

        // we need to use GetRun onto the destination
        pDstRun = DstEditor.GetFirstRun(&StartPosX, &StartPosY, &PixelCount, pi_pTransaction);

        while (pDstRun != 0)
            {
            HASSERT(PixelCount <= pDstSurfaceDescriptor->GetWidth());

            RealY = pScanlines->AdjustYValueForCurrentRun((double)StartPosY + 0.5);

            // convert the starting position
            pi_rTransfoModel.ConvertDirect(pScanlines->AdjustXValueForCurrentRun((double)StartPosX + PixelOffset),
                                           RealY,
                                           &PosX,
                                           &PosY);
            HPOSTCONDITION(PosX >= 0.0);
            HPOSTCONDITION(PosY >= 0.0);

            if ((PosX + (double)PixelCount * ScaleX) >= (double)pSrcSurfaceDescriptor->GetWidth())
                {
                // change the scale to stay inside the source surface
                double NewScaleX = (((double)pSrcSurfaceDescriptor->GetWidth() - HGLOBAL_EPSILON) - (double)PosX) / (double)PixelCount;
                SrcSampler.SetScale(NewScaleX, ScaleY);
                }
            else
                SrcSampler.SetScale(ScaleX, ScaleY);


            SrcSampler.GetPixels(PosX,
                                 PosY,
                                 PixelCount,
                                 pSrcRun);

            if (pConverter)
                {
                // call the convert method setted before
                (pConverter->*pConvertMethod)(pSrcRun, pDstRun, PixelCount);

                DstEditor.SetRun(StartPosX, StartPosY, PixelCount, pDstRun);
                }
            else
                {
                DstEditor.SetRun(StartPosX, StartPosY, PixelCount, pSrcRun);
                }

            pDstRun = DstEditor.GetNextRun(&StartPosX, &StartPosY, &PixelCount, pi_pTransaction);
            }
        }
    }

/**----------------------------------------------------------------------------
 ApplyFilterOnSource

 @param pi_srcSurface
 @param pi_rTransfoModel
-----------------------------------------------------------------------------*/
HRASurface* HRABlitter::ApplyFilter(const HRASurface&         pi_srcSurface,
                                    const HFCPtr<HRPFilter>&  pi_rpFilter) const
    {
    HPRECONDITION(pi_rpFilter != 0);
    HPRECONDITION(pi_srcSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemorySurfaceDescriptor::CLASS_ID));

    HAutoPtr<HRPFilter> pFilter(pi_rpFilter->Clone());

    HFCPtr<HGSMemorySurfaceDescriptor> pSurfaceDescriptor(
        (const HFCPtr<HGSMemorySurfaceDescriptor>&)pi_srcSurface.GetSurfaceDescriptor());

    const HRPPixelNeighbourhood& rFilterNeighbourhood = pFilter->GetNeighbourhood();

    uint32_t DstWidth = pSurfaceDescriptor->GetWidth() - (rFilterNeighbourhood.GetWidth() - 1);
    uint32_t DstHeight = pSurfaceDescriptor->GetHeight() - (rFilterNeighbourhood.GetHeight() - 1);
    HArrayAutoPtr<Byte> pRawData;

    uint32_t BytesPerRow = (DstWidth * pSurfaceDescriptor->GetPixelType()->CountPixelRawDataBits() + 7) / 8;
    uint32_t BufferSize = BytesPerRow * DstHeight;

    // allocate a buffer for dst
    pRawData = new Byte[BufferSize];

    // create the source pixel buffer
    HRPPixelBuffer SrcPixelBuffer(rFilterNeighbourhood,
                                  *pSurfaceDescriptor->GetPixelType(),
                                  pSurfaceDescriptor->GetPacket()->GetBufferAddress(),
                                  pSurfaceDescriptor->GetWidth() - (rFilterNeighbourhood.GetWidth() - 1),
                                  pSurfaceDescriptor->GetHeight() - (rFilterNeighbourhood.GetHeight() - 1),
                                  0,
                                  true);

    // create the destination pixel buffer with the same pixel type as the surface
    HRPPixelBuffer DstPixelBuffer(*pSurfaceDescriptor->GetPixelType(),
                                  pRawData,
                                  DstWidth,
                                  DstHeight);


    pFilter->SetInputPixelType(pSurfaceDescriptor->GetPixelType());
    pFilter->SetOutputPixelType(pSurfaceDescriptor->GetPixelType());
    pFilter->Convert(&SrcPixelBuffer,
                     &DstPixelBuffer);

    // now, create the filtered surface
    HFCPtr<HCDPacket> pPacket(new HCDPacket(new HCDCodecIdentity(),
                                            pRawData.release(),
                                            BufferSize,
                                            BufferSize));
    pPacket->SetBufferOwnership(true);

    HFCPtr<HGSMemorySurfaceDescriptor> pFilteredSurfaceDescriptor(new HGSMemorySurfaceDescriptor(DstWidth,
                                                                  DstHeight,
                                                                  pSurfaceDescriptor->GetPixelType(),
                                                                  pPacket,
                                                                  pSurfaceDescriptor->GetSLO(),
                                                                  BytesPerRow));


    return new HRASurface(pFilteredSurfaceDescriptor.GetPtr());
    }


/**----------------------------------------------------------------------------
 CreateWorkingBuffer

 @param pi_rPixelType
 @param pi_Size
-----------------------------------------------------------------------------*/
Byte* HRABlitter::CreateWorkingBuffer(const HRPPixelType&  pi_rPixelType,
                                       uint32_t             pi_Width,
                                       uint32_t             pi_Height) const
    {
    size_t BytesPerLine;

    // In 1 bit RLE, allocate worst case
    if (pi_rPixelType.IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) ||
        pi_rPixelType.IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
        BytesPerLine = (pi_Width * 2 + 2) * sizeof(unsigned short);
    else
        BytesPerLine = ((pi_Width * pi_rPixelType.CountPixelRawDataBits()) + 7) / 8;

    return new Byte[BytesPerLine * pi_Height];
    }

