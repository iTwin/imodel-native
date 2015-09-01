//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAWarper.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HRAWarper
//---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAWarper.h>

#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HGSMemoryBaseSurfaceDescriptor.h>
#include <Imagepp/all/h/HRAEditor.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRASampler.h>
#include <Imagepp/all/h/HRPFilter.h>
#include <Imagepp/all/h/HGSRegion.h>
#include <Imagepp/all/h/HGFScanlines.h>

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>

/**----------------------------------------------------------------------------
 Constructor for this class

 @param pi_pAttributes
 @param pi_pSurfaceImplementation
-----------------------------------------------------------------------------*/
HRAWarper::HRAWarper(HRASurface& pi_destSurface)
    : m_destSurface(pi_destSurface),
      m_ComposeRequired(false),
      m_ApplyGrid(false),
      m_samplingMode(HGSResampling::NEAREST_NEIGHBOUR)
    {
    HPRECONDITION(m_destSurface.GetSurfaceDescriptor() != 0);
    HPRECONDITION(m_destSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));

    HFCPtr<HGSMemoryBaseSurfaceDescriptor> rpDescriptor;
    rpDescriptor = (const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)m_destSurface.GetSurfaceDescriptor();
    }

/**----------------------------------------------------------------------------
 Destructor for this class
-----------------------------------------------------------------------------*/
HRAWarper::~HRAWarper()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAWarper::SetAlphaBlend(bool alphaBlend)
    {
    m_ComposeRequired = alphaBlend;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAWarper::SetGridMode(bool gridMode)
    {
    m_ApplyGrid = gridMode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAWarper::SetSamplingMode(HGSResampling const& samplingMode)
    {
    m_samplingMode = samplingMode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRAWarper::SetFilter(HFCPtr<HRPFilter> const& pFilter)
    {
    m_pFilter = pFilter;
    }

/**----------------------------------------------------------------------------
 WarpFrom

 @param pi_pSurface
 @param pi_rTransfoModel
-----------------------------------------------------------------------------*/
void HRAWarper::WarpFrom(const HRASurface&                  pi_srcSurface,
                         const HGF2DTransfoModel&           pi_rTransfoModel,
                         HRATransaction*                    pi_pTransaction)
    {
    HPRECONDITION(pi_srcSurface.GetSurfaceDescriptor() != 0);
    HPRECONDITION(pi_srcSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));
    HPRECONDITION(GetDestSurface().GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));

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
            DstClipped = true;

            pDstClipRegion->AddOperation(HGSRegion::INTERSECT,
                                         pSrcClipRegion);
            }
        }

    // check if we have a filter on the source
    const HRASurface* pSrcSurface;
    HAutoPtr<HRASurface> pFilteredSurface;
    if (m_pFilter != NULL)
        {
        HFCPtr<HRPFilter> pFilter(m_pFilter->Clone());
        pFilter->SetInputPixelType(pSrcSurfaceDescriptor->GetPixelType());
        pFilter->SetOutputPixelType(pDstSurfaceDescriptor->GetPixelType());
        pFilteredSurface = ApplyFilter(pi_srcSurface, pFilter);
        pSrcSurface = pFilteredSurface;
        }
    else
        pSrcSurface = &pi_srcSurface;


    if ((pDstSurfaceDescriptor->GetPixelType()->CountPixelRawDataBits() % 8) == 0 &&
        pDstSurfaceDescriptor->GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID))
        Optimized8BitsWarp(*pSrcSurface, pi_rTransfoModel, pi_pTransaction);
    else
        NormalWarp(*pSrcSurface, pi_rTransfoModel, pi_pTransaction);

    // check if we have shaped the destination
    if (pSrcClipRegion != 0)
        {
        if (DstClipped)
            pDstClipRegion->RemoveLastOperation();
        else
            GetDestSurface().SetRegion(NULL);
        }
    }




//:>---------------------------------------------------------------------------------------
//:> protected section
//:>---------------------------------------------------------------------------------------

//:>---------------------------------------------------------------------------------------
//:> private section
//:>---------------------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Warp a 8 bits destination

 This version write directly into the destination without using the editor.
 This is possible because we have a 8 bits destination without compression.

 @param pi_srcSurface
 @param pi_rTransfoModel
-----------------------------------------------------------------------------*/
void HRAWarper::Optimized8BitsWarp(HRASurface const&                pi_srcSurface,
                                   const HGF2DTransfoModel&         pi_rTransfoModel,
                                   HRATransaction*                  pi_pTransaction)
    {
    HPRECONDITION(pi_srcSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));
    HPRECONDITION(GetDestSurface().GetSurfaceDescriptor()->IsCompatibleWith(HGSMemorySurfaceDescriptor::CLASS_ID));

    const HFCPtr<HGSMemoryBaseSurfaceDescriptor> pSrcSurfaceDescriptor =
        (const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)pi_srcSurface.GetSurfaceDescriptor();
    const HFCPtr<HGSMemorySurfaceDescriptor> pDstSurfaceDescriptor =
        (const HFCPtr<HGSMemorySurfaceDescriptor>&)GetDestSurface().GetSurfaceDescriptor();
    // the destination must be not compressed
    HPRECONDITION(pDstSurfaceDescriptor->GetPacket() != 0);
    HPRECONDITION(pDstSurfaceDescriptor->GetPacket()->GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID));

    // calculate the size of the sampling into the source
    HFCPtr<HGF2DCoordSys> pDstCoordSys = new HGF2DCoordSys();
    HFCPtr<HGF2DCoordSys> pSrcCoordSys = new HGF2DCoordSys(pi_rTransfoModel, pDstCoordSys);
    HGF2DExtent DstExtent(0.0, 0.0, 1.0, 1.0, pDstCoordSys);
    HGF2DExtent SrcExtent(DstExtent.CalculateApproxExtentIn(pSrcCoordSys));

    // Create source sampler
    HRASampler SrcSampler(m_samplingMode,
                          *pSrcSurfaceDescriptor,
                          HGF2DRectangle(0.0,
                                         0.0,
                                         SrcExtent.GetWidth(),
                                         SrcExtent.GetHeight()),
                          0.0,
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

    // check if we need a converter
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
        // we need a converter, create converter and a temporary buffer
        pConverter = pSourcePixelType->GetConverterTo(
                         pDstSurfaceDescriptor->GetPixelType());

        pSrcRun = CreateWorkingBuffer(*pSourcePixelType,
                                      pDstSurfaceDescriptor->GetWidth(),
                                      1);

        if (m_ComposeRequired)
            pConvertMethod = &HRPPixelConverter::Compose;   // use the Compose method
        else
            pConvertMethod = &HRPPixelConverter::Convert;   // use the Convert method
        }

    // create the destination editor
    // because we are in N8 with no compression, we write directly into the the destination pointer

    HRAEditor DstEditor(GetDestSurface());

    HUINTX  StartPosX;
    HUINTX  StartPosY;
    size_t  PixelCount;
    double YMax = DstEditor.GetScanlines() != 0 ? DstEditor.GetScanlines()->GetYMax() - HGLOBAL_EPSILON : pDstSurfaceDescriptor->GetHeight();

    uint32_t MaxPixelsInOneDestLine = GetDestSurface().GetSurfaceDescriptor()->GetWidth();
    HArrayAutoPtr<double> pXPositions(new double[MaxPixelsInOneDestLine]);
    HArrayAutoPtr<double> pYPositions(new double[MaxPixelsInOneDestLine]);

    Byte* pDstRun = (Byte*)DstEditor.GetFirstRun(&StartPosX,
                                                     &StartPosY,
                                                     &PixelCount,
                                                     pi_pTransaction);

    if (!m_ApplyGrid || DstEditor.GetScanlines() == 0)
        {
        if (pConverter != 0)
            {
            while (pDstRun != 0)
                {
                // convert the destination run into the source coordinates
                pi_rTransfoModel.ConvertDirect(MIN((double)StartPosY + 0.5, YMax),
                                               (double)StartPosX + 0.5,
                                               (uint32_t)PixelCount,
                                               1.0,
                                               pXPositions,
                                               pYPositions);

                for (size_t i = 0 ; i < PixelCount ; ++i)
                    {
                    if (pXPositions[i] < 0.0)
                        pXPositions[i] = 0.0;
                    if (pXPositions[i] >= pSrcSurfaceDescriptor->GetWidth())
                        pXPositions[i] = pSrcSurfaceDescriptor->GetWidth() - HGLOBAL_EPSILON;
                    if (pYPositions[i] < 0.0)
                        pYPositions[i] = 0.0;
                    if (pYPositions[i] >= pSrcSurfaceDescriptor->GetHeight())
                        pYPositions[i] = pSrcSurfaceDescriptor->GetHeight() - HGLOBAL_EPSILON;
                    }

                // get the source pixel
                SrcSampler.GetPixels(pXPositions,
                                     pYPositions,
                                     PixelCount,
                                     pSrcRun);

                // convert the source pixel directly into the destination run
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
                // convert the destination run into the source coordinates
                pi_rTransfoModel.ConvertDirect(MIN((double)StartPosY + 0.5, YMax),
                                               (double)StartPosX + 0.5,
                                               PixelCount,
                                               1.0,
                                               pXPositions,
                                               pYPositions);

                for (size_t i = 0 ; i < PixelCount ; ++i)
                    {
                    if (pXPositions[i] < 0.0)
                        pXPositions[i] = 0.0;
                    if (pXPositions[i] >= pSrcSurfaceDescriptor->GetWidth())
                        pXPositions[i] = pSrcSurfaceDescriptor->GetWidth() - HGLOBAL_EPSILON;
                    if (pYPositions[i] < 0.0)
                        pYPositions[i] = 0.0;
                    if (pYPositions[i] >= pSrcSurfaceDescriptor->GetHeight())
                        pYPositions[i] = pSrcSurfaceDescriptor->GetHeight() - HGLOBAL_EPSILON;
                    }

                // get the source pixel directly into the destination
                SrcSampler.GetPixels(pXPositions,
                                     pYPositions,
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

        const HGFScanLines* pScanlines = DstEditor.GetScanlines();
        double RealY;

        while (pDstRun != 0)
            {
            HASSERT(PixelCount > 0);

            RealY = pScanlines->AdjustYValueForCurrentRun((double)StartPosY + 0.5);

            // Compute first position
            pi_rTransfoModel.ConvertDirect(pScanlines->AdjustXValueForCurrentRun((double)StartPosX + 0.5),
                                           RealY,
                                           &pXPositions[0],
                                           &pYPositions[0]);
            if (PixelCount > 2)
                {
                // Compute intermediate positions
                pi_rTransfoModel.ConvertDirect(RealY,
                                               (double)StartPosX + 1.5,
                                               PixelCount-2,
                                               1.0,
                                               &pXPositions[1],
                                               &pYPositions[1]);
                }
            // Compute last position
            pi_rTransfoModel.ConvertDirect(pScanlines->AdjustXValueForCurrentRun((double)(StartPosX+PixelCount-1) + 0.5),
                                           RealY,
                                           &pXPositions[PixelCount-1],
                                           &pYPositions[PixelCount-1]);

            for (size_t i = 0 ; i < PixelCount ; ++i)
                {
                if (pXPositions[i] < 0.0)
                    pXPositions[i] = 0.0;
                if (pXPositions[i] >= pSrcSurfaceDescriptor->GetWidth())
                    pXPositions[i] = pSrcSurfaceDescriptor->GetWidth() - HGLOBAL_EPSILON;
                if (pYPositions[i] < 0.0)
                    pYPositions[i] = 0.0;
                if (pYPositions[i] >= pSrcSurfaceDescriptor->GetHeight())
                    pYPositions[i] = pSrcSurfaceDescriptor->GetHeight() - HGLOBAL_EPSILON;
                }

            if (pConverter)
                {
                SrcSampler.GetPixels(pXPositions,
                                     pYPositions,
                                     PixelCount,
                                     pSrcRun);

                // call the convert method setted before
                (pConverter->*pConvertMethod)(pSrcRun, pDstRun, PixelCount);
                }
            else
                {
                SrcSampler.GetPixels(pXPositions,
                                     pYPositions,
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
 NormalWarp

 This version is use when the destination pixel type is not a N8 or the
 destination has compression.

 @param pi_srcSurface
 @param pi_rTransfoModel
-----------------------------------------------------------------------------*/
void HRAWarper::NormalWarp(HRASurface const&                pi_srcSurface,
                           const HGF2DTransfoModel&         pi_rTransfoModel,
                           HRATransaction*                  pi_pTransaction)
    {
    HPRECONDITION(pi_srcSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));
    HPRECONDITION(GetDestSurface().GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));

    const HFCPtr<HGSMemoryBaseSurfaceDescriptor> pSrcSurfaceDescriptor =
        (const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)pi_srcSurface.GetSurfaceDescriptor();
    const HFCPtr<HGSMemoryBaseSurfaceDescriptor> pDstSurfaceDescriptor =
        (const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)GetDestSurface().GetSurfaceDescriptor();

    // calculate the size of the sampling into the source
    HFCPtr<HGF2DCoordSys> pDstCoordSys = new HGF2DCoordSys();
    HFCPtr<HGF2DCoordSys> pSrcCoordSys = new HGF2DCoordSys(pi_rTransfoModel, pDstCoordSys);
    HGF2DExtent DstExtent(0.0, 0.0, 1.0, 1.0, pDstCoordSys);
    HGF2DExtent SrcExtent(DstExtent.CalculateApproxExtentIn(pSrcCoordSys));

    // Create source sampler
    HRASampler SrcSampler(m_samplingMode,
                          *pSrcSurfaceDescriptor,
                          HGF2DRectangle(0.0,
                                         0.0,
                                         SrcExtent.GetWidth(),
                                         SrcExtent.GetHeight()),
                          0.0,
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

    // create the destination editor
    HRAEditor DstEditor(GetDestSurface());

    // the source and destination have the same pixel type, we don't have converter
    HArrayAutoPtr<Byte> pSrcRun(CreateWorkingBuffer(*pSourcePixelType,
                                                     pDstSurfaceDescriptor->GetWidth(),
                                                     1));

    uint32_t MaxPixelsInOneDestLine = GetDestSurface().GetSurfaceDescriptor()->GetWidth();
    HArrayAutoPtr<double> pXPositions(new double[MaxPixelsInOneDestLine]);
    HArrayAutoPtr<double> pYPositions(new double[MaxPixelsInOneDestLine]);

    double YMax = DstEditor.GetScanlines() != 0 ? (double)((uint32_t)DstEditor.GetScanlines()->GetYMax()) - 0.5 - HGLOBAL_EPSILON :
                   (double)pDstSurfaceDescriptor->GetHeight() - 0.5 - HGLOBAL_EPSILON;


    double SrcXMax = (double)pSrcSurfaceDescriptor->GetWidth() - HGLOBAL_EPSILON;
    double SrcYMax = (double)pSrcSurfaceDescriptor->GetHeight() - HGLOBAL_EPSILON;

    size_t  PixelCount;

    if (!m_ApplyGrid || DstEditor.GetScanlines() == 0)
        {
        if (m_ComposeRequired)
            {
            HUINTX  StartPosX;
            HUINTX  StartPosY;

            void* pDstRun;
            pDstRun = DstEditor.GetFirstRun(&StartPosX, &StartPosY, &PixelCount, pi_pTransaction);
            while (pDstRun != 0)
                {
                // convert the destination run into the source coordinates
                pi_rTransfoModel.ConvertDirect(MIN((double)StartPosY + 0.5, YMax),
                                               (double)StartPosX + 0.5,
                                               PixelCount,
                                               1.0,
                                               pXPositions,
                                               pYPositions);

                for (size_t i = 0 ; i < PixelCount ; ++i)
                    {
                    pXPositions[i] = MIN(MAX(pXPositions[i], 0.0), SrcXMax);
                    pYPositions[i] = MIN(MAX(pYPositions[i], 0.0), SrcYMax);
                    }

                // get the source pixel
                SrcSampler.GetPixels(pXPositions,
                                     pYPositions,
                                     PixelCount,
                                     pSrcRun);

                pConverter->Compose(pSrcRun, pDstRun, PixelCount);
                DstEditor.SetRun(StartPosX, StartPosY, PixelCount, pDstRun);

                pDstRun = DstEditor.GetNextRun(&StartPosX, &StartPosY, &PixelCount, pi_pTransaction);
                }
            }
        else
            {
            HSINTX  StartPosX;
            HSINTX  StartPosY;
            const HGFScanLines* pScanlines;
            pScanlines = DstEditor.GetScanlines();

            // test if we are shaped
            if (pScanlines != 0)
                {
                if (const_cast<HGFScanLines*>(pScanlines)->GotoFirstRun())
                    {
                    double YMax = pScanlines->GetYMax() - HGLOBAL_EPSILON;
                    HArrayAutoPtr<Byte> pDstRun;

                    // test if we have a oonverter
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
                        pi_rTransfoModel.ConvertDirect(MIN((double)StartPosY + 0.5, YMax),
                                                       (double)StartPosX + 0.5,
                                                       PixelCount,
                                                       1.0,
                                                       pXPositions,
                                                       pYPositions);

                        for (size_t i = 0 ; i < PixelCount ; ++i)
                            {
                            pXPositions[i] = MIN(MAX(pXPositions[i], 0.0), SrcXMax);
                            pYPositions[i] = MIN(MAX(pYPositions[i], 0.0), SrcYMax);
                            }

                        // get the source pixel
                        SrcSampler.GetPixels(pXPositions,
                                             pYPositions,
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
                    pi_rTransfoModel.ConvertDirect((double)i + 0.5,
                                                   0.0,
                                                   PixelCount,
                                                   1.0,
                                                   pXPositions,
                                                   pYPositions);

                    for (size_t j = 0 ; j < PixelCount ; ++j)
                        {
                        pXPositions[j] = MIN(MAX(pXPositions[j], 0.0), SrcXMax);
                        pYPositions[j] = MIN(MAX(pYPositions[j], 0.0), SrcYMax);
                        }

                    // get the source pixel
                    SrcSampler.GetPixels(pXPositions,
                                         pYPositions,
                                         PixelCount,
                                         pSrcRun);

                    if (pConverter != 0)
                        {
                        // convert the source pixel directly into the destination run
                        pConverter->Convert(pSrcRun, pDstRun, PixelCount);
                        DstEditor.SetRun(0, i, PixelCount, pDstRun, pi_pTransaction);
                        }
                    else
                        {
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

        uint32_t MaxPixelsInOneDestLine = GetDestSurface().GetSurfaceDescriptor()->GetWidth();
        HArrayAutoPtr<double> pXPositions(new double[MaxPixelsInOneDestLine]);
        HArrayAutoPtr<double> pYPositions(new double[MaxPixelsInOneDestLine]);
        double RealY;

        HUINTX StartPosX;
        HUINTX StartPosY;
        void* pDstRun;

        // we need to use GetRun onto the destination
        pDstRun = DstEditor.GetFirstRun(&StartPosX, &StartPosY, &PixelCount, pi_pTransaction);

        while (pDstRun != 0)
            {
            HASSERT(PixelCount > 0);

            RealY = pScanlines->AdjustYValueForCurrentRun((double)StartPosY + 0.5);

            // Compute first position
            pi_rTransfoModel.ConvertDirect(pScanlines->AdjustXValueForCurrentRun((double)StartPosX + 0.5),
                                           RealY,
                                           &pXPositions[0],
                                           &pYPositions[0]);
            if (PixelCount > 2)
                {
                // Compute intermediate positions
                pi_rTransfoModel.ConvertDirect(RealY,
                                               (double)StartPosX + 1.5,
                                               PixelCount-2,
                                               1.0,
                                               &pXPositions[1],
                                               &pYPositions[1]);
                }
            // Compute last position
            pi_rTransfoModel.ConvertDirect(pScanlines->AdjustXValueForCurrentRun((double)(StartPosX+PixelCount-1) + 0.5),
                                           RealY,
                                           &pXPositions[PixelCount-1],
                                           &pYPositions[PixelCount-1]);

            HASSERT(PixelCount <= pDstSurfaceDescriptor->GetWidth());

            SrcSampler.GetPixels(pXPositions,
                                 pYPositions,
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

 @param pi_pSurface
 @param pi_rTransfoModel
-----------------------------------------------------------------------------*/
HRASurface* HRAWarper::ApplyFilter(const HRASurface&         pi_srcSurface,
                                   const HFCPtr<HRPFilter>&  pi_rpFilter) const
    {
    HPRECONDITION(pi_rpFilter != 0);
    HPRECONDITION(pi_srcSurface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemorySurfaceDescriptor::CLASS_ID));
    HPRECONDITION(GetDestSurface().GetSurfaceDescriptor()->IsCompatibleWith(HGSMemorySurfaceDescriptor::CLASS_ID));

    HFCPtr<HGSMemorySurfaceDescriptor> pSurfaceDescriptor(
        (const HFCPtr<HGSMemorySurfaceDescriptor>&)pi_srcSurface.GetSurfaceDescriptor());

    const HRPPixelNeighbourhood& rFilterNeighbourhood = pi_rpFilter->GetNeighbourhood();

    uint32_t DstWidth = pSurfaceDescriptor->GetWidth() - (rFilterNeighbourhood.GetWidth() - 1);
    uint32_t DstHeight = pSurfaceDescriptor->GetHeight() - (rFilterNeighbourhood.GetHeight() - 1);
    HArrayAutoPtr<Byte> pRawData;

    uint32_t BytesPerRow = (DstWidth * pSurfaceDescriptor->GetPixelType()->CountPixelRawDataBits() + 7) / 8;
    uint32_t BufferSize = BytesPerRow * DstHeight;

    // allocate a buffer for dst
    pRawData = new Byte[BufferSize];

    // create the source pixel buffer
    HPRECONDITION(pi_rpFilter->GetInputPixelType()->HasSamePixelInterpretation(*pSurfaceDescriptor->GetPixelType()));

    HRPPixelBuffer SrcPixelBuffer(rFilterNeighbourhood,
                                  *pi_rpFilter->GetInputPixelType(),
                                  pSurfaceDescriptor->GetPacket()->GetBufferAddress(),
                                  pSurfaceDescriptor->GetWidth() - (rFilterNeighbourhood.GetWidth() - 1),
                                  pSurfaceDescriptor->GetHeight() - (rFilterNeighbourhood.GetHeight() - 1),
                                  0,
                                  true);

    // create the destination pixel buffer

    HDEBUGCODE(HFCPtr<HGSMemorySurfaceDescriptor> )
    HDEBUGCODE(            pDstSurfaceDescriptor((const HFCPtr<HGSMemorySurfaceDescriptor>&)GetDestSurface().GetSurfaceDescriptor());)
    HDEBUGCODE(HPRECONDITION(pDstSurfaceDescriptor->GetPixelType()->HasSamePixelInterpretation(*pi_rpFilter->GetOutputPixelType()));)

    HRPPixelBuffer DstPixelBuffer(*pi_rpFilter->GetOutputPixelType(),
                                  pRawData,
                                  DstWidth,
                                  DstHeight);


    pi_rpFilter->Convert(&SrcPixelBuffer,
                         &DstPixelBuffer);

    // now, create the filtered surface
    HFCPtr<HCDPacket> pPacket(new HCDPacket(new HCDCodecIdentity(),
                                            pRawData.release(),
                                            BufferSize,
                                            BufferSize));
    pPacket->SetBufferOwnership(true);

    HFCPtr<HGSSurfaceDescriptor> pFilteredSurfaceDescriptor(new HGSMemorySurfaceDescriptor(DstWidth,
                                                                  DstHeight,
                                                                  pi_rpFilter->GetOutputPixelType(),
                                                                  pPacket,
                                                                  pSurfaceDescriptor->GetSLO(),
                                                                  BytesPerRow));


    return new HRASurface(pFilteredSurfaceDescriptor);
    }


/**----------------------------------------------------------------------------
 CreateWorkingBuffer

 @param pi_rPixelType
 @param pi_Size
-----------------------------------------------------------------------------*/
Byte* HRAWarper::CreateWorkingBuffer(const HRPPixelType&  pi_rPixelType,
                                      uint32_t             pi_Width,
                                      uint32_t             pi_Height) const
    {
    size_t BytesPerLine;

    // In 1 bit RLE, allocate worst case
    if (pi_rPixelType.IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) ||
        pi_rPixelType.IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
        BytesPerLine = (pi_Width * 2 + 1) * sizeof(unsigned short);
    else
        BytesPerLine = ((pi_Width * pi_rPixelType.CountPixelRawDataBits()) + 7) / 8;

    return new Byte[BytesPerLine * pi_Height];
    }
