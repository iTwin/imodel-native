//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMColorBalancedImageIterator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HIMColorBalancedImageIterator.h>
#include <Imagepp/all/h/HIMColorBalancedImage.h>
#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HGF2DGrid.h>
#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HRPPixelBuffer.h>

#include <Imagepp/all/h/HFCGrid.h>

#define M_BALANCEDIMAGE  ( (HFCPtr<HIMColorBalancedImage>&) GetRaster() )

#define RGBDELTAS(TheDeltas)    ( (HIMColorBalancedImage::RGBDeltas*) TheDeltas )


/** ---------------------------------------------------------------------------
    Constructor
    ---------------------------------------------------------------------------
*/
HIMColorBalancedImageIterator::HIMColorBalancedImageIterator(
    const HFCPtr<HIMColorBalancedImage>& pi_pReference,
    const HRAIteratorOptions&            pi_rOptions)
    :    HRARasterIterator( (HFCPtr<HRARaster>&) pi_pReference, pi_rOptions)
    {
    InitObject();
    }


/** ---------------------------------------------------------------------------
    Copy constructor.
    ---------------------------------------------------------------------------
*/
HIMColorBalancedImageIterator::HIMColorBalancedImageIterator(
    const HIMColorBalancedImageIterator& pi_rObj)
    : HRARasterIterator(pi_rObj)
    {
    InitObject();
    }

/** ---------------------------------------------------------------------------
    The destructor.
    ---------------------------------------------------------------------------
*/
HIMColorBalancedImageIterator::~HIMColorBalancedImageIterator()
    {
    delete m_pSrcIterator;
    }


/** ---------------------------------------------------------------------------
    Assignment operator.  It duplicates another iterator.
    ---------------------------------------------------------------------------
*/
HIMColorBalancedImageIterator& HIMColorBalancedImageIterator::operator=(
    const HIMColorBalancedImageIterator& pi_rObj)
    {
    HASSERT(false);

    return (*this);
    }


/** ---------------------------------------------------------------------------
    Go to next raster object, and return it
    ---------------------------------------------------------------------------
*/
const HFCPtr<HRARaster>& HIMColorBalancedImageIterator::Next()
    {
    // if this is not the last raster, goto next valid raster
    if(m_pCurRaster != 0)
        {
        HFCPtr<HRARaster> pRaster;

        do
            {
            // go to next raster
            m_pSrcIterator->Next();

            // get the raster
            pRaster = ((*m_pSrcIterator)());

            if(pRaster == 0)
                m_pCurRaster = 0;
            else
                {
                m_pCurRaster = BalanceRaster(pRaster);
                }
            }
        while(pRaster != 0 && m_pCurRaster == 0);
        }

    // return the current raster
    return m_pCurRaster;
    }


/** ---------------------------------------------------------------------------
    Return current raster object
    ---------------------------------------------------------------------------
*/
const HFCPtr<HRARaster>& HIMColorBalancedImageIterator::operator()()
    {
    // return the current raster
    return m_pCurRaster;
    }


/** ---------------------------------------------------------------------------
    Reset to start state
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImageIterator::Reset()
    {
    // delete the source iterator if there is already one
    if(m_pSrcIterator == 0)
        delete m_pSrcIterator;

    // init the object again
    InitObject();
    }



// ----------------------------------------------------------- Privates

/** ---------------------------------------------------------------------------
    Start the iterator on the first useful portion.
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImageIterator::InitObject()
    {
    // Create options for the source iterator. We need to clip
    // using our effective shape.
    HRAIteratorOptions SourceOptions(GetOptions());
    SourceOptions.ClipRegionToRaster(GetRaster());
    SourceOptions.ClipUsingEffectiveShape(false);

    // create an iterator on the filtered source
    m_pSrcIterator = M_BALANCEDIMAGE->GetSource()->CreateIterator(SourceOptions);

    if(m_pSrcIterator == 0)
        {
        m_pCurRaster = 0;
        }
    else
        {
        // get the raster
        do
            {
            HFCPtr<HRARaster> pSourceRaster((*m_pSrcIterator)());

            if (pSourceRaster != 0)
                {
                m_pCurRaster = BalanceRaster(pSourceRaster);

                // go to next raster
                if(m_pCurRaster == 0)
                    m_pSrcIterator->Next();
                }
            }
        while(((*m_pSrcIterator)()) != 0 && m_pCurRaster == 0);
        }
    }

/** ---------------------------------------------------------------------------
    Apply color balancing to one portion.
    ---------------------------------------------------------------------------
*/
HFCPtr<HRARaster> HIMColorBalancedImageIterator::BalanceRaster(
    const HFCPtr<HRARaster> pi_pRaster)
    {
    // The result
    HFCPtr<HRARaster> pRasterToReturn;

    HFCPtr<HRARaster> pSourceRaster(pi_pRaster);

    if (M_BALANCEDIMAGE->GetNumberOfNeighbors() > 0)
        {
        HVEShape OriginalShape(*pi_pRaster->GetEffectiveShape());
        if(GetOptions().IsShaped())
            OriginalShape.Intersect(*GetOptions().GetRegionToProcess());
        OriginalShape.ChangeCoordSys(pi_pRaster->GetCoordSys());

        HVEShape PhysicalShape(OriginalShape);

        // get the extent of the intersection
        HFCPtr<HGF2DCoordSys> pRefPhysicalCoordSys;
        if(pRefPhysicalCoordSys == 0)
            {
            // Decapsulate reference
            if (pSourceRaster->IsCompatibleWith(HRAReferenceToRaster::CLASS_ID))
                {
                pSourceRaster = ((HFCPtr<HRAReferenceToRaster>&)pSourceRaster)->GetSource();

                // Do the inverse job of the Ref2Raster: Take the reference
                // shape back into the source raster.
                PhysicalShape.ChangeCoordSys(pi_pRaster->GetCoordSys());
                PhysicalShape.SetCoordSys(pSourceRaster->GetCoordSys());
                }

            if (pSourceRaster->IsCompatibleWith(HRAStoredRaster::CLASS_ID))
                {
                pRefPhysicalCoordSys = ((HFCPtr<HRAStoredRaster>&)pSourceRaster)->GetPhysicalCoordSys();
                }
            else
                {
                HVEShape TmpShape(pi_pRaster->GetAveragePixelSize());
                TmpShape.ChangeCoordSys(pi_pRaster->GetCoordSys());
                HGF2DExtent TmpExtent(TmpShape.GetExtent());
                HGF2DStretch Stretch(HGF2DDisplacement(0, 0),
                                     TmpExtent.GetWidth(),
                                     TmpExtent.GetHeight());

                pRefPhysicalCoordSys = new HGF2DCoordSys(Stretch, pi_pRaster->GetCoordSys());
                }
            }

        PhysicalShape.ChangeCoordSys(pRefPhysicalCoordSys);

        HFCPtr<HRARaster> pSrcBitmapExample =
            new HRABitmap(1,
                          1,
                          pRefPhysicalCoordSys->GetTransfoModelTo(pSourceRaster->GetCoordSys()),
                          pi_pRaster->GetCoordSys(),
                          pi_pRaster->GetPixelType());
        HFCPtr<HRARaster> pDstBitmapExample = pSrcBitmapExample;
        HASSERT(pSrcBitmapExample != 0);
        HASSERT(pDstBitmapExample != 0);

        HGF2DExtent PhysicalExtent(PhysicalShape.GetExtent());

        if(PhysicalExtent.IsDefined())
            {
            HFCGrid Grid(PhysicalExtent.GetXMin(),
                         PhysicalExtent.GetYMin(),
                         PhysicalExtent.GetXMax(),
                         PhysicalExtent.GetYMax());

            // calculate the physical region to process in the source

            HASSERT(Grid.GetXMin() <= LONG_MAX);
            HASSERT(Grid.GetYMin() <= LONG_MAX);
            HASSERT(Grid.GetWidth() <= LONG_MAX);
            HASSERT(Grid.GetHeight() <= LONG_MAX);

            int32_t X = (int32_t)Grid.GetXMin();
            int32_t Y = (int32_t)Grid.GetYMin();
            int32_t Width = (int32_t)Grid.GetWidth();
            int32_t Height = (int32_t)Grid.GetHeight();
            int32_t SrcWidth = Width;
            int32_t SrcHeight = Height;
            int32_t SrcDisplacementX = 0;
            int32_t SrcDisplacementY = 0;

            HFCPtr<HRABitmap> pSrcBitmap;
            HFCPtr<HGSMemorySurfaceDescriptor> pSrcDescriptor;

            if (pSourceRaster->IsCompatibleWith(HRABitmap::CLASS_ID))
                {
                HASSERT(pSrcBitmap->GetSurfaceDescriptor()->IsCompatibleWith(HGSMemorySurfaceDescriptor::CLASS_ID));
                // We will use the source bitmap directly

                pSrcBitmap = (HFCPtr<HRABitmap>&) pSourceRaster;

                // Retrieve the source dimensions
                pSrcDescriptor = static_cast<HGSMemorySurfaceDescriptor*>(pSrcBitmap->GetSurfaceDescriptor().GetPtr());
                SrcWidth = pSrcDescriptor->GetWidth();
                SrcHeight = pSrcDescriptor->GetHeight();

                SrcDisplacementX = X;
                SrcDisplacementY = Y;
                SrcDisplacementX = MAX(SrcDisplacementX, 0);
                SrcDisplacementY = MAX(SrcDisplacementY, 0);

                // Don't let destination overflow the source bitmap
                Width = MIN(Width, SrcWidth - SrcDisplacementX);
                Height = MIN(Height, SrcHeight - SrcDisplacementY);
                }
            else
                {
                HASSERT(0);  // Not supported yet.
#if 0
                // create the source bitmap
                pSrcBitmap = (HRABitmap*)pSrcBitmapExample->Clone(0);
                HASSERT(pSrcBitmap != 0);
                pSrcBitmap->InitSize(Width, Height);
                pSrcBitmap->Move(HGF2DDisplacement(OriginalExtent.GetXMin() - SourceExtent.GetXMin(),
                                                   OriginalExtent.GetYMin() - SourceExtent.GetYMin()));

                // copy the source of the filtered image in the source bitmap
                pSrcBitmap->CopyFrom(M_BALANCEDIMAGE->GetSource());

                pSrcDescriptor = (HFCPtr<HGSMemorySurfaceDescriptor>&)pSrcBitmap->GetSurfaceDescriptor();
#endif
                }

            // create the destination bitmap
            HFCPtr<HRABitmap> pDstBitmap((HRABitmap*)pDstBitmapExample->Clone(0));
            HASSERT(pDstBitmap != 0);
            pDstBitmap->InitSize(Width, Height);

            // Compute bitmap displacement
            double NewX;
            double NewY;
            double OriginX;
            double OriginY;
            pDstBitmap->GetTransfoModel()->ConvertDirect(X,
                                                         Y,
                                                         &NewX,
                                                         &NewY);

            pDstBitmap->GetTransfoModel()->ConvertDirect(0.0,
                                                         0.0,
                                                         &OriginX,
                                                         &OriginY);
            pDstBitmap->Move(HGF2DDisplacement(NewX - OriginX, NewY - OriginY));

            // create the source pixel buffer
            HRPPixelBuffer SrcPixelBuffer(*pSrcBitmap->GetPixelType(),
                                          pSrcDescriptor->GetPacket()->GetBufferAddress(),
                                          SrcWidth,
                                          SrcHeight);

            HASSERT(pDstBitmap->GetSurfaceDescriptor()->IsCompatibleWith(HGSMemorySurfaceDescriptor::CLASS_ID));

            HFCPtr<HGSMemorySurfaceDescriptor> pDstDescriptor(static_cast<HGSMemorySurfaceDescriptor*>(pDstBitmap->GetSurfaceDescriptor().GetPtr()));

            // create the destination pixel buffer
            HRPPixelBuffer DstPixelBuffer(*pDstBitmap->GetPixelType(),
                                          pDstDescriptor->GetPacket()->GetBufferAddress(),
                                          Width,
                                          Height);

            // Balance the data
            if (M_BALANCEDIMAGE->m_ApplyPositional)
                {
                HVEShape TotalShape(*M_BALANCEDIMAGE->GetApplicationShape());
                TotalShape.ChangeCoordSys(pRefPhysicalCoordSys);
                HGF2DExtent TotalExtent(TotalShape.GetExtent());

                if (M_BALANCEDIMAGE->m_ColorMode == HIMColorBalancedImage::COLORMODE_RGB)
                    {
                    if (M_BALANCEDIMAGE->m_ApplyGlobal)
                        {
                        if (M_BALANCEDIMAGE->GetNumberOfNeighbors() == 4)
                            ApplyColorBalanceRGB4(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                        else
                            ApplyColorBalanceRGB(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                        }
                    else
                        {
                        if (M_BALANCEDIMAGE->GetNumberOfNeighbors() == 4)
                            ApplyColorBalanceRGB4PositionalOnly(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                        else
                            ApplyColorBalanceRGBPositionalOnly(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                        }
                    }
                else
                    {
                    // COLORMODE_GRAY

                    if (M_BALANCEDIMAGE->m_ApplyGlobal)
                        {
                        if (M_BALANCEDIMAGE->GetNumberOfNeighbors() == 4)
                            ApplyColorBalanceGray4(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                        else
                            ApplyColorBalanceGray(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                        }
                    else
                        {
                        if (M_BALANCEDIMAGE->GetNumberOfNeighbors() == 4)
                            ApplyColorBalanceGray4PositionalOnly(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                        else
                            ApplyColorBalanceGrayPositionalOnly(&SrcPixelBuffer, &DstPixelBuffer, X, Y, TotalExtent, SrcDisplacementX, SrcDisplacementY);
                        }
                    }
                }
            else
                {
                if (M_BALANCEDIMAGE->m_ColorMode == HIMColorBalancedImage::COLORMODE_RGB)
                    ApplyColorBalanceRGBGlobalOnly(&SrcPixelBuffer, &DstPixelBuffer, SrcDisplacementX, SrcDisplacementY);
                else
                    ApplyColorBalanceGrayGlobalOnly(&SrcPixelBuffer, &DstPixelBuffer, SrcDisplacementX, SrcDisplacementY);
                }

            pDstBitmap->SetShape(OriginalShape);

            // put again the raster in a shaped reference
            pRasterToReturn = (HFCPtr<HRARaster>&) pDstBitmap;
            }
        }
    else
        {
        // No neighbors. Don't bother with the filtering.
        pRasterToReturn = pi_pRaster;
        }

    return pRasterToReturn;
    }



////////////////////
// COLORMODE_RGB
////////////////////



/** ---------------------------------------------------------------------------
    Apply balance on a 24 bits image (global + positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImageIterator::ApplyColorBalanceRGB(HRPPixelBuffer* pi_pSourcePixels,
                                                         HRPPixelBuffer* pi_pDestPixels,
                                                         int32_t pi_OriginX,
                                                         int32_t pi_OriginY,
                                                         const HGF2DExtent& pi_rTotalExtent,
                                                         int32_t pi_DisplacementX,
                                                         int32_t pi_DisplacementY)
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   3 * (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                        pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Red;
    double Green;
    double Blue;
    unsigned short SourceRed;
    unsigned short SourceGreen;
    unsigned short SourceBlue;

    double TotalWeight;
    double InvertedTotalWeight;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double NormalizedWeightLeft;
    double NormalizedWeightRight;
    double NormalizedWeightTop;
    double NormalizedWeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            TotalWeight = 0.0;
            if (M_BALANCEDIMAGE->m_pLeftDeltas != 0)
                {
                TotalWeight += WeightLeft;
                }
            if (M_BALANCEDIMAGE->m_pRightDeltas != 0)
                {
                TotalWeight += WeightRight;
                }
            if (M_BALANCEDIMAGE->m_pTopDeltas != 0)
                {
                TotalWeight += WeightTop;
                }
            if (M_BALANCEDIMAGE->m_pBottomDeltas != 0)
                {
                TotalWeight += WeightBottom;
                }
            InvertedTotalWeight = 1.0 / TotalWeight;

            SourceRed = M_BALANCEDIMAGE->m_GlobalMap.Red[*pSrc++];
            Red = SourceRed;
            SourceGreen = M_BALANCEDIMAGE->m_GlobalMap.Green[*pSrc++];
            Green = SourceGreen;
            SourceBlue = M_BALANCEDIMAGE->m_GlobalMap.Blue[*pSrc++];
            Blue = SourceBlue;

            // Take account of each neighbor
            if (M_BALANCEDIMAGE->m_pLeftDeltas != 0)
                {
                NormalizedWeightLeft = WeightLeft * InvertedTotalWeight;

                Red   += NormalizedWeightLeft * RGBDELTAS(M_BALANCEDIMAGE->m_pLeftDeltas)->Red[SourceRed];
                Green += NormalizedWeightLeft * RGBDELTAS(M_BALANCEDIMAGE->m_pLeftDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightLeft * RGBDELTAS(M_BALANCEDIMAGE->m_pLeftDeltas)->Blue[SourceBlue];
                }
            if (M_BALANCEDIMAGE->m_pRightDeltas != 0)
                {
                NormalizedWeightRight = WeightRight * InvertedTotalWeight;

                Red   += NormalizedWeightRight * RGBDELTAS(M_BALANCEDIMAGE->m_pRightDeltas)->Red[SourceRed];
                Green += NormalizedWeightRight * RGBDELTAS(M_BALANCEDIMAGE->m_pRightDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightRight * RGBDELTAS(M_BALANCEDIMAGE->m_pRightDeltas)->Blue[SourceBlue];
                }
            if (M_BALANCEDIMAGE->m_pTopDeltas != 0)
                {
                NormalizedWeightTop = WeightTop * InvertedTotalWeight;

                Red   += NormalizedWeightTop * RGBDELTAS(M_BALANCEDIMAGE->m_pTopDeltas)->Red[SourceRed];
                Green += NormalizedWeightTop * RGBDELTAS(M_BALANCEDIMAGE->m_pTopDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightTop * RGBDELTAS(M_BALANCEDIMAGE->m_pTopDeltas)->Blue[SourceBlue];
                }
            if (M_BALANCEDIMAGE->m_pBottomDeltas != 0)
                {
                NormalizedWeightBottom = WeightBottom * InvertedTotalWeight;

                Red   += NormalizedWeightBottom * RGBDELTAS(M_BALANCEDIMAGE->m_pBottomDeltas)->Red[SourceRed];
                Green += NormalizedWeightBottom * RGBDELTAS(M_BALANCEDIMAGE->m_pBottomDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightBottom * RGBDELTAS(M_BALANCEDIMAGE->m_pBottomDeltas)->Blue[SourceBlue];
                }

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Red, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Green, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Blue, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + 3 * (pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth());

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 24 bits image that has 4 neighbors (global + positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImageIterator::ApplyColorBalanceRGB4(HRPPixelBuffer* pi_pSourcePixels,
                                                          HRPPixelBuffer* pi_pDestPixels,
                                                          int32_t pi_OriginX,
                                                          int32_t pi_OriginY,
                                                          const HGF2DExtent& pi_rTotalExtent,
                                                          int32_t pi_DisplacementX,
                                                          int32_t pi_DisplacementY)
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   3 * (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                        pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Red;
    double Green;
    double Blue;
    unsigned short SourceRed;
    unsigned short SourceGreen;
    unsigned short SourceBlue;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            SourceRed = M_BALANCEDIMAGE->m_GlobalMap.Red[*pSrc++];
            Red = SourceRed;
            SourceGreen = M_BALANCEDIMAGE->m_GlobalMap.Green[*pSrc++];
            Green = SourceGreen;
            SourceBlue = M_BALANCEDIMAGE->m_GlobalMap.Blue[*pSrc++];
            Blue = SourceBlue;

            // Instead of normalizing (dividing the weights by 2), we can extract the
            // division and apply it once to the total.
            Red += ( WeightLeft *   RGBDELTAS(M_BALANCEDIMAGE->m_pLeftDeltas)->Red[SourceRed] +
                     WeightRight *  RGBDELTAS(M_BALANCEDIMAGE->m_pRightDeltas)->Red[SourceRed] +
                     WeightTop *    RGBDELTAS(M_BALANCEDIMAGE->m_pTopDeltas)->Red[SourceRed] +
                     WeightBottom * RGBDELTAS(M_BALANCEDIMAGE->m_pBottomDeltas)->Red[SourceRed] ) * 0.5;
            Green += ( WeightLeft *   RGBDELTAS(M_BALANCEDIMAGE->m_pLeftDeltas)->Green[SourceGreen] +
                       WeightRight *  RGBDELTAS(M_BALANCEDIMAGE->m_pRightDeltas)->Green[SourceGreen] +
                       WeightTop *    RGBDELTAS(M_BALANCEDIMAGE->m_pTopDeltas)->Green[SourceGreen] +
                       WeightBottom * RGBDELTAS(M_BALANCEDIMAGE->m_pBottomDeltas)->Green[SourceGreen] ) * 0.5;
            Blue += ( WeightLeft *   RGBDELTAS(M_BALANCEDIMAGE->m_pLeftDeltas)->Blue[SourceBlue] +
                      WeightRight *  RGBDELTAS(M_BALANCEDIMAGE->m_pRightDeltas)->Blue[SourceBlue] +
                      WeightTop *    RGBDELTAS(M_BALANCEDIMAGE->m_pTopDeltas)->Blue[SourceBlue] +
                      WeightBottom * RGBDELTAS(M_BALANCEDIMAGE->m_pBottomDeltas)->Blue[SourceBlue] ) * 0.5;

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Red, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Green, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Blue, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + 3 * (pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth());

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 24 bits image (positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImageIterator::ApplyColorBalanceRGBPositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                                       HRPPixelBuffer* pi_pDestPixels,
                                                                       int32_t pi_OriginX,
                                                                       int32_t pi_OriginY,
                                                                       const HGF2DExtent& pi_rTotalExtent,
                                                                       int32_t pi_DisplacementX,
                                                                       int32_t pi_DisplacementY)
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   3 * (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                        pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Red;
    double Green;
    double Blue;
    unsigned short SourceRed;
    unsigned short SourceGreen;
    unsigned short SourceBlue;

    double TotalWeight;
    double InvertedTotalWeight;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double NormalizedWeightLeft;
    double NormalizedWeightRight;
    double NormalizedWeightTop;
    double NormalizedWeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            TotalWeight = 0.0;
            if (M_BALANCEDIMAGE->m_pLeftDeltas != 0)
                {
                TotalWeight += WeightLeft;
                }
            if (M_BALANCEDIMAGE->m_pRightDeltas != 0)
                {
                TotalWeight += WeightRight;
                }
            if (M_BALANCEDIMAGE->m_pTopDeltas != 0)
                {
                TotalWeight += WeightTop;
                }
            if (M_BALANCEDIMAGE->m_pBottomDeltas != 0)
                {
                TotalWeight += WeightBottom;
                }
            InvertedTotalWeight = 1.0 / TotalWeight;

            SourceRed = *pSrc++;
            Red = SourceRed;
            SourceGreen = *pSrc++;
            Green = SourceGreen;
            SourceBlue = *pSrc++;
            Blue = SourceBlue;

            // Take account of each neighbor
            if (M_BALANCEDIMAGE->m_pLeftDeltas != 0)
                {
                NormalizedWeightLeft = WeightLeft * InvertedTotalWeight;

                Red   += NormalizedWeightLeft * RGBDELTAS(M_BALANCEDIMAGE->m_pLeftDeltas)->Red[SourceRed];
                Green += NormalizedWeightLeft * RGBDELTAS(M_BALANCEDIMAGE->m_pLeftDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightLeft * RGBDELTAS(M_BALANCEDIMAGE->m_pLeftDeltas)->Blue[SourceBlue];
                }
            if (M_BALANCEDIMAGE->m_pRightDeltas != 0)
                {
                NormalizedWeightRight = WeightRight * InvertedTotalWeight;

                Red   += NormalizedWeightRight * RGBDELTAS(M_BALANCEDIMAGE->m_pRightDeltas)->Red[SourceRed];
                Green += NormalizedWeightRight * RGBDELTAS(M_BALANCEDIMAGE->m_pRightDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightRight * RGBDELTAS(M_BALANCEDIMAGE->m_pRightDeltas)->Blue[SourceBlue];
                }
            if (M_BALANCEDIMAGE->m_pTopDeltas != 0)
                {
                NormalizedWeightTop = WeightTop * InvertedTotalWeight;

                Red   += NormalizedWeightTop * RGBDELTAS(M_BALANCEDIMAGE->m_pTopDeltas)->Red[SourceRed];
                Green += NormalizedWeightTop * RGBDELTAS(M_BALANCEDIMAGE->m_pTopDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightTop * RGBDELTAS(M_BALANCEDIMAGE->m_pTopDeltas)->Blue[SourceBlue];
                }
            if (M_BALANCEDIMAGE->m_pBottomDeltas != 0)
                {
                NormalizedWeightBottom = WeightBottom * InvertedTotalWeight;

                Red   += NormalizedWeightBottom * RGBDELTAS(M_BALANCEDIMAGE->m_pBottomDeltas)->Red[SourceRed];
                Green += NormalizedWeightBottom * RGBDELTAS(M_BALANCEDIMAGE->m_pBottomDeltas)->Green[SourceGreen];
                Blue  += NormalizedWeightBottom * RGBDELTAS(M_BALANCEDIMAGE->m_pBottomDeltas)->Blue[SourceBlue];
                }

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Red, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Green, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Blue, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + 3 * (pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth());

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 24 bits image that has 4 neighbors (positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImageIterator::ApplyColorBalanceRGB4PositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                                        HRPPixelBuffer* pi_pDestPixels,
                                                                        int32_t pi_OriginX,
                                                                        int32_t pi_OriginY,
                                                                        const HGF2DExtent& pi_rTotalExtent,
                                                                        int32_t pi_DisplacementX,
                                                                        int32_t pi_DisplacementY)
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   3 * (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                        pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Red;
    double Green;
    double Blue;
    unsigned short SourceRed;
    unsigned short SourceGreen;
    unsigned short SourceBlue;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            SourceRed = *pSrc++;
            Red = SourceRed;
            SourceGreen = *pSrc++;
            Green = SourceGreen;
            SourceBlue = *pSrc++;
            Blue = SourceBlue;

            // Instead of normalizing (dividing the weights by 2), we can extract the
            // division and apply it once to the total.
            Red += ( WeightLeft *   RGBDELTAS(M_BALANCEDIMAGE->m_pLeftDeltas)->Red[SourceRed] +
                     WeightRight *  RGBDELTAS(M_BALANCEDIMAGE->m_pRightDeltas)->Red[SourceRed] +
                     WeightTop *    RGBDELTAS(M_BALANCEDIMAGE->m_pTopDeltas)->Red[SourceRed] +
                     WeightBottom * RGBDELTAS(M_BALANCEDIMAGE->m_pBottomDeltas)->Red[SourceRed] ) * 0.5;
            Green += ( WeightLeft *   RGBDELTAS(M_BALANCEDIMAGE->m_pLeftDeltas)->Green[SourceGreen] +
                       WeightRight *  RGBDELTAS(M_BALANCEDIMAGE->m_pRightDeltas)->Green[SourceGreen] +
                       WeightTop *    RGBDELTAS(M_BALANCEDIMAGE->m_pTopDeltas)->Green[SourceGreen] +
                       WeightBottom * RGBDELTAS(M_BALANCEDIMAGE->m_pBottomDeltas)->Green[SourceGreen] ) * 0.5;
            Blue += ( WeightLeft *   RGBDELTAS(M_BALANCEDIMAGE->m_pLeftDeltas)->Blue[SourceBlue] +
                      WeightRight *  RGBDELTAS(M_BALANCEDIMAGE->m_pRightDeltas)->Blue[SourceBlue] +
                      WeightTop *    RGBDELTAS(M_BALANCEDIMAGE->m_pTopDeltas)->Blue[SourceBlue] +
                      WeightBottom * RGBDELTAS(M_BALANCEDIMAGE->m_pBottomDeltas)->Blue[SourceBlue] ) * 0.5;

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Red, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Green, 0.0), 255.0);
            *pDst++ = (Byte) MIN( MAX(Blue, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + 3 * (pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth());

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 24 bits image (global)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImageIterator::ApplyColorBalanceRGBGlobalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                                   HRPPixelBuffer* pi_pDestPixels,
                                                                   int32_t pi_DisplacementX,
                                                                   int32_t pi_DisplacementY)
    {
    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   3 * (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                        pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    for (uint32_t Y = 0 ; Y < pi_pDestPixels->GetHeight() ; ++Y)
        {
        for (uint32_t X = 0 ; X < pi_pDestPixels->GetWidth() ; ++X)
            {
            *pDst++ = M_BALANCEDIMAGE->m_GlobalMap.Red[*pSrc++];
            *pDst++ = M_BALANCEDIMAGE->m_GlobalMap.Green[*pSrc++];
            *pDst++ = M_BALANCEDIMAGE->m_GlobalMap.Blue[*pSrc++];
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + 3 * (pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth());

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }



////////////////////
// COLORMODE_GRAY
////////////////////



/** ---------------------------------------------------------------------------
    Apply balance on a 8 bits grayscale image (global + positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImageIterator::ApplyColorBalanceGray(HRPPixelBuffer* pi_pSourcePixels,
                                                          HRPPixelBuffer* pi_pDestPixels,
                                                          int32_t pi_OriginX,
                                                          int32_t pi_OriginY,
                                                          const HGF2DExtent& pi_rTotalExtent,
                                                          int32_t pi_DisplacementX,
                                                          int32_t pi_DisplacementY)
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                    pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Gray;
    unsigned short SourceGray;

    double TotalWeight;
    double InvertedTotalWeight;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double NormalizedWeightLeft;
    double NormalizedWeightRight;
    double NormalizedWeightTop;
    double NormalizedWeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            TotalWeight = 0.0;
            if (M_BALANCEDIMAGE->m_pLeftDeltas != 0)
                {
                TotalWeight += WeightLeft;
                }
            if (M_BALANCEDIMAGE->m_pRightDeltas != 0)
                {
                TotalWeight += WeightRight;
                }
            if (M_BALANCEDIMAGE->m_pTopDeltas != 0)
                {
                TotalWeight += WeightTop;
                }
            if (M_BALANCEDIMAGE->m_pBottomDeltas != 0)
                {
                TotalWeight += WeightBottom;
                }
            InvertedTotalWeight = 1.0 / TotalWeight;

            SourceGray = M_BALANCEDIMAGE->m_GlobalMap.Gray[*pSrc++];
            Gray = SourceGray;

            // Take account of each neighbor
            if (M_BALANCEDIMAGE->m_pLeftDeltas != 0)
                {
                NormalizedWeightLeft = WeightLeft * InvertedTotalWeight;

                Gray += NormalizedWeightLeft * M_BALANCEDIMAGE->m_pLeftDeltas->Gray[SourceGray];
                }
            if (M_BALANCEDIMAGE->m_pRightDeltas != 0)
                {
                NormalizedWeightRight = WeightRight * InvertedTotalWeight;

                Gray += NormalizedWeightRight * M_BALANCEDIMAGE->m_pRightDeltas->Gray[SourceGray];
                }
            if (M_BALANCEDIMAGE->m_pTopDeltas != 0)
                {
                NormalizedWeightTop = WeightTop * InvertedTotalWeight;

                Gray += NormalizedWeightTop * M_BALANCEDIMAGE->m_pTopDeltas->Gray[SourceGray];
                }
            if (M_BALANCEDIMAGE->m_pBottomDeltas != 0)
                {
                NormalizedWeightBottom = WeightBottom * InvertedTotalWeight;

                Gray += NormalizedWeightBottom * M_BALANCEDIMAGE->m_pBottomDeltas->Gray[SourceGray];
                }

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Gray, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth();

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 8 bits grayscale image that has 4 neighbors (global + positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImageIterator::ApplyColorBalanceGray4(HRPPixelBuffer* pi_pSourcePixels,
                                                           HRPPixelBuffer* pi_pDestPixels,
                                                           int32_t pi_OriginX,
                                                           int32_t pi_OriginY,
                                                           const HGF2DExtent& pi_rTotalExtent,
                                                           int32_t pi_DisplacementX,
                                                           int32_t pi_DisplacementY)
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                    pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Gray;
    unsigned short SourceGray;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            SourceGray = M_BALANCEDIMAGE->m_GlobalMap.Gray[*pSrc++];
            Gray = SourceGray;

            // Instead of normalizing (dividing the weights by 2), we can extract the
            // division and apply it once to the total.
            Gray += ( WeightLeft * M_BALANCEDIMAGE->m_pLeftDeltas->Gray[SourceGray] +
                      WeightRight * M_BALANCEDIMAGE->m_pRightDeltas->Gray[SourceGray] +
                      WeightTop * M_BALANCEDIMAGE->m_pTopDeltas->Gray[SourceGray] +
                      WeightBottom * M_BALANCEDIMAGE->m_pBottomDeltas->Gray[SourceGray] ) * 0.5;

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Gray, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth();

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 8 bits grayscale image (positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImageIterator::ApplyColorBalanceGrayPositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                                        HRPPixelBuffer* pi_pDestPixels,
                                                                        int32_t pi_OriginX,
                                                                        int32_t pi_OriginY,
                                                                        const HGF2DExtent& pi_rTotalExtent,
                                                                        int32_t pi_DisplacementX,
                                                                        int32_t pi_DisplacementY)
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                    pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Gray;
    unsigned short SourceGray;

    double TotalWeight;
    double InvertedTotalWeight;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double NormalizedWeightLeft;
    double NormalizedWeightRight;
    double NormalizedWeightTop;
    double NormalizedWeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            TotalWeight = 0.0;
            if (M_BALANCEDIMAGE->m_pLeftDeltas != 0)
                {
                TotalWeight += WeightLeft;
                }
            if (M_BALANCEDIMAGE->m_pRightDeltas != 0)
                {
                TotalWeight += WeightRight;
                }
            if (M_BALANCEDIMAGE->m_pTopDeltas != 0)
                {
                TotalWeight += WeightTop;
                }
            if (M_BALANCEDIMAGE->m_pBottomDeltas != 0)
                {
                TotalWeight += WeightBottom;
                }
            InvertedTotalWeight = 1.0 / TotalWeight;

            SourceGray = *pSrc++;
            Gray = SourceGray;

            // Take account of each neighbor
            if (M_BALANCEDIMAGE->m_pLeftDeltas != 0)
                {
                NormalizedWeightLeft = WeightLeft * InvertedTotalWeight;

                Gray += NormalizedWeightLeft * M_BALANCEDIMAGE->m_pLeftDeltas->Gray[SourceGray];
                }
            if (M_BALANCEDIMAGE->m_pRightDeltas != 0)
                {
                NormalizedWeightRight = WeightRight * InvertedTotalWeight;

                Gray += NormalizedWeightRight * M_BALANCEDIMAGE->m_pRightDeltas->Gray[SourceGray];
                }
            if (M_BALANCEDIMAGE->m_pTopDeltas != 0)
                {
                NormalizedWeightTop = WeightTop * InvertedTotalWeight;

                Gray += NormalizedWeightTop * M_BALANCEDIMAGE->m_pTopDeltas->Gray[SourceGray];
                }
            if (M_BALANCEDIMAGE->m_pBottomDeltas != 0)
                {
                NormalizedWeightBottom = WeightBottom * InvertedTotalWeight;

                Gray += NormalizedWeightBottom * M_BALANCEDIMAGE->m_pBottomDeltas->Gray[SourceGray];
                }

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Gray, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth();

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 8 bits grayscale image that has 4 neighbors (positional)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImageIterator::ApplyColorBalanceGray4PositionalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                                         HRPPixelBuffer* pi_pDestPixels,
                                                                         int32_t pi_OriginX,
                                                                         int32_t pi_OriginY,
                                                                         const HGF2DExtent& pi_rTotalExtent,
                                                                         int32_t pi_DisplacementX,
                                                                         int32_t pi_DisplacementY)
    {
    HGF2DGrid TotalGrid(pi_rTotalExtent);

    // Optimization: Fetch values here. The calls are inlined, but
    // do way too much work to be executed in the loops.
    const int32_t TotalGridXMin = TotalGrid.GetXMin();
    const int32_t TotalGridXMax = TotalGrid.GetXMax();
    const int32_t TotalGridYMin = TotalGrid.GetYMin();
    const int32_t TotalGridYMax = TotalGrid.GetYMax();
    const double InvertedTotalGridWidth = 1.0 / (double) TotalGrid.GetWidth(); // Used as divider (double) in the loop

    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                    pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    double Gray;
    unsigned short SourceGray;

    // Work with increments for weights instead of
    // computing each time.
    const double XRatioIncrement = 1.0 / TotalGrid.GetWidth();
    const double YRatioIncrement = 1.0 / TotalGrid.GetHeight();

    // Declare variables used inside the loop
    double WeightLeft;
    double WeightRight;
    double WeightTop;
    double WeightBottom;
    double XRatio;

    double YRatio = (double) (pi_OriginY - TotalGridYMin) / TotalGrid.GetHeight();

    const int32_t XMax = pi_OriginX + pi_pDestPixels->GetWidth();
    const int32_t YMax = pi_OriginY + pi_pDestPixels->GetHeight();

    for (int32_t Y = pi_OriginY ; Y < YMax ; ++Y)
        {
        if (Y <= TotalGridYMin)
            YRatio = 0.0;
        else if (Y > TotalGridYMax)
            YRatio = 1.0;
        else
            YRatio += YRatioIncrement;
        HASSERT(YRatio >= 0.0 && YRatio <= 1.01);

        WeightTop    = 1.0 - YRatio;
        WeightBottom = YRatio;

        XRatio = (double) (pi_OriginX - TotalGridXMin) * InvertedTotalGridWidth;

        for (int32_t X = pi_OriginX ; X < XMax ; ++X)
            {
            if (X <= TotalGridXMin)
                XRatio = 0.0;
            else if (X > TotalGridXMax)
                XRatio = 1.0;
            else
                XRatio += XRatioIncrement;
            HASSERT(XRatio >= 0.0 && XRatio <= 1.01);

            WeightLeft  = 1.0 - XRatio;
            WeightRight = XRatio;

            SourceGray = *pSrc++;
            Gray = SourceGray;

            // Instead of normalizing (dividing the weights by 2), we can extract the
            // division and apply it once to the total.
            Gray += ( WeightLeft * M_BALANCEDIMAGE->m_pLeftDeltas->Gray[SourceGray] +
                      WeightRight * M_BALANCEDIMAGE->m_pRightDeltas->Gray[SourceGray] +
                      WeightTop * M_BALANCEDIMAGE->m_pTopDeltas->Gray[SourceGray] +
                      WeightBottom * M_BALANCEDIMAGE->m_pBottomDeltas->Gray[SourceGray] ) * 0.5;

            // Bound the results
            *pDst++ = (Byte) MIN( MAX(Gray, 0.0), 255.0);
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth();

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }


/** ---------------------------------------------------------------------------
    Apply balance on a 8 bits grayscale image (global)
    ---------------------------------------------------------------------------
*/
void HIMColorBalancedImageIterator::ApplyColorBalanceGrayGlobalOnly(HRPPixelBuffer* pi_pSourcePixels,
                                                                    HRPPixelBuffer* pi_pDestPixels,
                                                                    int32_t pi_DisplacementX,
                                                                    int32_t pi_DisplacementY)
    {
    Byte* pSrc = ((Byte*) pi_pSourcePixels->GetBufferPtr()) +
                   (pi_DisplacementY * (pi_pSourcePixels->GetWidth() + pi_pSourcePixels->GetPaddingBytes()) +
                    pi_DisplacementX);

    Byte* pDst = (Byte*) pi_pDestPixels->GetBufferPtr();

    for (uint32_t Y = 0 ; Y < pi_pDestPixels->GetHeight() ; ++Y)
        {
        for (uint32_t X = 0 ; X < pi_pDestPixels->GetWidth() ; ++X)
            {
            *pDst++ = M_BALANCEDIMAGE->m_GlobalMap.Gray[*pSrc++];
            }

        pSrc += pi_pSourcePixels->GetPaddingBytes() + pi_pSourcePixels->GetWidth() - pi_pDestPixels->GetWidth();

        pDst += pi_pDestPixels->GetPaddingBytes();
        }
    }

