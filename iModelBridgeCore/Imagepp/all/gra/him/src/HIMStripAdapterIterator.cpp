//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/him/src/HIMStripAdapterIterator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HIMStripAdapterIterator
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HIMStripAdapterIterator.h>

#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <Imagepp/all/h/HFCGrid.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HIMStripProgressIndicator.h>
#include <Imagepp/all/h/HRACopyFromOptions.h>
#include <Imagepp/all/h/HRABitmap.h>
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HIMStripAdapter.h>


//-----------------------------------------------------------------------------
// Shaped constructor.  It takes a pointer to the raster to scan, and the region
// to scan.
//-----------------------------------------------------------------------------
HIMStripAdapterIterator::HIMStripAdapterIterator(
    const HFCPtr<HIMStripAdapter>& pi_pReference,
    const HRAIteratorOptions&      pi_rOptions)
    :    HRARasterIterator( (HFCPtr<HRARaster>&) pi_pReference, pi_rOptions)
    {
    if (pi_rOptions.GetPhysicalCoordSys() == 0)
        // Use the CoordSys from the BitmapExemple
        m_pResolutionPhysicalCoordSys = ((HFCPtr<HIMStripAdapter>&)GetRaster())->GetInputBitmapExample()->GetCoordSys();
    else
        m_pResolutionPhysicalCoordSys = pi_rOptions.GetPhysicalCoordSys();

    InitObject();
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HIMStripAdapterIterator::~HIMStripAdapterIterator()
    {
    }

//-----------------------------------------------------------------------------
// Go to next raster object, and return it
//-----------------------------------------------------------------------------
const HFCPtr<HRARaster>& HIMStripAdapterIterator::Next()
    {
    m_CurStrip++;

    // We force to stop the iteration
    if (!HIMStripProgressIndicator::GetInstance()->ContinueIteration())
        m_CurStrip = m_NumberOfStrips;

    ComputeStrip();

    // return the current raster
    return operator()();
    }


//-----------------------------------------------------------------------------
// Return current raster object
//-----------------------------------------------------------------------------
const HFCPtr<HRARaster>& HIMStripAdapterIterator::operator()()
    {
    // return the current raster
    return (HFCPtr<HRARaster>&)m_pRasterToReturn;
    }


//-----------------------------------------------------------------------------
// Reset to start state
//-----------------------------------------------------------------------------
void HIMStripAdapterIterator::Reset()
    {
    // delete the source iterator if there is already one
    m_pRasterToReturn = 0;

    // init the object again
    InitObject();
    }



// ----------------------------------------------------------- Privates

//-----------------------------------------------------------------------------
// private
// InitObject
//-----------------------------------------------------------------------------
void HIMStripAdapterIterator::InitObject()
    {
    // Get the strip adapter object
    HFCPtr<HIMStripAdapter> pStripAdapter = (HFCPtr<HIMStripAdapter>&)GetRaster();

    // Adapt the resolution ccordsys for the strip with the quality factor
    // and best coordsys of the strip
    double MinScaleX;
    double MinScaleY;

    HGF2DExtent TmpExtentMin;
    HGF2DExtent TmpExtentMax;
    pStripAdapter->GetSource()->GetPixelSizeRange(TmpExtentMin, TmpExtentMax);

    // The pixel sizes must be defined
    HASSERT(TmpExtentMin.IsDefined());

    // Must in shape before apply ChangeCoordSys
    TmpExtentMin = TmpExtentMin.CalculateApproxExtentIn(m_pResolutionPhysicalCoordSys);

    MinScaleX = TmpExtentMin.GetWidth();
    MinScaleY = TmpExtentMin.GetHeight();

    MinScaleX /= pStripAdapter->GetQualityFactor();
    MinScaleY /= pStripAdapter->GetQualityFactor();

    HGF2DStretch Stretch(HGF2DDisplacement(0.0, 0.0), MinScaleX, MinScaleY);

    m_pResolutionPhysicalCoordSys = new HGF2DCoordSys(Stretch, m_pResolutionPhysicalCoordSys);

    // compute the mins
    HFCPtr<HVEShape> pShape(new HVEShape(*pStripAdapter->GetEffectiveShape()));
    pShape->ChangeCoordSys(m_pResolutionPhysicalCoordSys);
    if(GetOptions().IsShaped())
        pShape->Intersect(*GetOptions().GetRegionToProcess());

    // The region to effectively process must not be empty
    // Otherwise the strip would be difficult to create!
    HASSERT(!pShape->IsEmpty());

    HGF2DExtent Extent(pShape->GetExtent());
    m_XMin = Extent.GetXMin();
    m_YMin = Extent.GetYMin();

    // compute the width of a strip
    HFCGrid Grid(Extent.GetXMin(), Extent.GetYMin(), Extent.GetXMax(), Extent.GetYMax());

    HASSERT(Grid.GetWidth() <= ULONG_MAX);
    HASSERT(Grid.GetHeight() <= ULONG_MAX);

    m_StripWidth = (uint32_t)Grid.GetWidth();

    HFCPtr<HRPPixelType> pPixelType;
    pPixelType = pStripAdapter->GetInputBitmapExample()->GetPixelType();
    size_t BytesPerLine = (pPixelType->CountPixelRawDataBits() * m_StripWidth + 7) / 8;

    // If fixed dimension has been propose to the HIMStripAdapter, use it!
    if (pStripAdapter->m_StripWidth && pStripAdapter->m_StripHeigth)
        {
        // If a specific shape has been given, use it!
        m_StripHeight = pStripAdapter->m_StripHeigth;

        // Be sure the given strip height fit into the pStripAdapter allocated memory.
        HASSERT( m_StripWidth  <= m_StripWidth);
        HASSERT( m_StripHeight <= MIN(pStripAdapter->GetMaxSizeInBytes() / BytesPerLine, (uint32_t)Grid.GetHeight()));
        }
    else
        {
        // Compute the height of a strip if none have been specified at
        // the HIMStripAdapter construction.
        m_StripHeight = (uint32_t)MAX(MIN(pStripAdapter->GetMaxSizeInBytes() / BytesPerLine, (uint32_t)Grid.GetHeight()), 1.0);
        }

    // compute the number of strips
    m_NumberOfStrips = ((uint32_t)Grid.GetHeight() + m_StripHeight - 1) / m_StripHeight;

    // set the current strip position
    m_CurStrip = 0;

    // compute the first strip
    ComputeStrip();

    HIMStripProgressIndicator::GetInstance()->Restart(m_NumberOfStrips);
    }

//-----------------------------------------------------------------------------
// private
// ComputeStrip
//-----------------------------------------------------------------------------
void HIMStripAdapterIterator::ComputeStrip()
    {
    if(m_CurStrip < m_NumberOfStrips)
        {
        // get the strip adapter object
        HFCPtr<HIMStripAdapter> pStripAdapter = (HFCPtr<HIMStripAdapter>&)GetRaster();

        // compute the extent of a strip
        HGF2DExtent Extent( m_XMin,
                            m_YMin + (m_CurStrip * m_StripHeight),
                            m_XMin + m_StripWidth,
                            m_YMin + (m_CurStrip * m_StripHeight) + m_StripHeight,
                            m_pResolutionPhysicalCoordSys);

        // create the current strip from the example
        HFCPtr<HRABitmap> pBitmap(static_cast<HRABitmap*>(pStripAdapter->GetInputBitmapExample()->Clone(0).GetPtr()));
        HASSERT(pBitmap != 0);

        pBitmap->SetTransfoModel(HGF2DIdentity(), m_pResolutionPhysicalCoordSys);
        pBitmap->InitSize((uint32_t)floor(Extent.GetWidth()  + 0.5),
                          (uint32_t)floor(Extent.GetHeight() + 0.5));
        pBitmap->Move(HGF2DDisplacement(Extent.GetXMin(), Extent.GetYMin()));
        pBitmap->SetShape(Extent);

        // clear with the default background color of the bitmap
        pBitmap->Clear();

        // copy the raster in the srtip
        if(pStripAdapter->GetSource()->HasLookAhead())
            pStripAdapter->SetLookAhead(Extent, false);

        HRACopyFromOptions Options(true);
        pBitmap->CopyFrom(*pStripAdapter->GetSource(), Options);

        if (((HFCPtr<HIMStripAdapter>&)GetRaster())->m_ApplyClipping)
            {
            HVEShape Clip(*pStripAdapter->GetSource()->GetEffectiveShape());
            Clip.Intersect(*pBitmap->GetEffectiveShape());

            pBitmap->SetShape(Clip);
            }

        // Set the shape of the raster over the strip
        m_pRasterToReturn = pBitmap;
        }
    else
        {
        m_pRasterToReturn = 0;
        }
    }
