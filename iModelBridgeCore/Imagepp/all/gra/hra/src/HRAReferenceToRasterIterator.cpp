//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAReferenceToRasterIterator.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRAReferenceToRasterIterator
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAReferenceToRasterIterator.h>
#include <Imagepp/all/h/HRARaster.h>
#include <Imagepp/all/h/HRPPixelType.h>


//-----------------------------------------------------------------------------
// The constructor.  It takes a pointer to the raster to scan completely.
//-----------------------------------------------------------------------------
HRAReferenceToRasterIterator::HRAReferenceToRasterIterator(
    const HFCPtr<HRAReferenceToRaster>& pi_pReference,
    const HRAIteratorOptions&           pi_rOptions)
    : HRARasterIterator( (HFCPtr<HRARaster>&) pi_pReference, pi_rOptions)
    {
    if (pi_rOptions.GetPhysicalCoordSys() != 0)
        m_pResolutionPhysicalCoordSys = new HGF2DCoordSys(*pi_pReference->GetSource()->GetCoordSys()->GetTransfoModelTo(pi_pReference->GetCoordSys()), pi_rOptions.GetPhysicalCoordSys());

    // Calculate region to work on, and take that region in the source's system
    HFCPtr<HVEShape> pRegion(pi_rOptions.CalculateClippedRegion((HFCPtr<HRARaster>&) pi_pReference));
    pRegion->ChangeCoordSys(pi_pReference->GetCoordSys());
    pRegion->SetCoordSys(pi_pReference->GetSource()->GetCoordSys());

    // Create cource options. Since our effective shape was taken into account,
    // the source raster's effective was already accounted too...
    HRAIteratorOptions SourceOptions(pRegion, m_pResolutionPhysicalCoordSys, false);

    // Create source iterator
    m_pSourceIterator = pi_pReference->GetSource()->CreateIterator(SourceOptions);

    // Go to first raster to be returned
    m_pCurrentSourceRaster = (*m_pSourceIterator)();
    }


//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HRAReferenceToRasterIterator::HRAReferenceToRasterIterator(
    const HRAReferenceToRasterIterator& pi_rObj)
    : HRARasterIterator(pi_rObj)
    {
    m_pResolutionPhysicalCoordSys = pi_rObj.m_pResolutionPhysicalCoordSys;

    // Calculate region to work on, and take that region in the source's system
    HFCPtr<HVEShape> pRegion(GetOptions().CalculateClippedRegion(GetRaster()));
    pRegion->ChangeCoordSys(GetRaster()->GetCoordSys());
    pRegion->SetCoordSys(((HFCPtr<HRAReferenceToRaster>&)GetRaster())->GetSource()->GetCoordSys());

    // Create cource options. Since our effective shape was taken into account,
    // the source raster's effective was already accounted too...
    HRAIteratorOptions SourceOptions(pRegion, m_pResolutionPhysicalCoordSys, false);

    // Create source iterator
    m_pSourceIterator = ((HFCPtr<HRAReferenceToRaster>&)GetRaster())->GetSource()->CreateIterator(SourceOptions);

    // Source's iterator is constructed. Now copy it from other object.
    // We have to do it in two passes, because we don't know the type of
    // the iterator. We have to ask the source to create it for us.
    *m_pSourceIterator = *pi_rObj.m_pSourceIterator;

    m_pCurrentSourceRaster = pi_rObj.m_pCurrentSourceRaster;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HRAReferenceToRasterIterator::~HRAReferenceToRasterIterator()
    {
    }


//-----------------------------------------------------------------------------
// Assignment operator.  It duplicates another iterator.
//-----------------------------------------------------------------------------
HRAReferenceToRasterIterator& HRAReferenceToRasterIterator::operator=(
    const HRAReferenceToRasterIterator& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        // Copy the HRARasterIterator portion
        HRARasterIterator::operator=(pi_rObj);

        m_pResolutionPhysicalCoordSys = pi_rObj.m_pResolutionPhysicalCoordSys;

        // Delete current raster to return
        m_pRasterToReturn = 0;

        // Calculate region to work on, and take that region in the source's system
        HFCPtr<HVEShape> pRegion(GetOptions().CalculateClippedRegion(GetRaster()));
        pRegion->ChangeCoordSys(GetRaster()->GetCoordSys());
        pRegion->SetCoordSys(((HFCPtr<HRAReferenceToRaster>&)GetRaster())->GetSource()->GetCoordSys());

        // Create cource options. Since our effective shape was taken into account,
        // the source raster's effective was already accounted too...
        HRAIteratorOptions SourceOptions(pRegion, m_pResolutionPhysicalCoordSys, false);

        // Create source iterator
        m_pSourceIterator = ((HFCPtr<HRAReferenceToRaster>&)GetRaster())->GetSource()->CreateIterator(SourceOptions);

        // Source's iterator is constructed. Now copy it from other object.
        // We have to do it in two passes, because we don't know the type of
        // the iterator. We have to ask the source to create it for us.
        *m_pSourceIterator = *pi_rObj.m_pSourceIterator;

        m_pCurrentSourceRaster = pi_rObj.m_pCurrentSourceRaster;
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// Go to next raster object, and return it
//-----------------------------------------------------------------------------
const HFCPtr<HRARaster>& HRAReferenceToRasterIterator::Next()
    {
    // Find the next raster object whose working shape is not null...
    m_pCurrentSourceRaster = m_pSourceIterator->Next();

    m_pRasterToReturn = 0;

    return operator()();
    }


//-----------------------------------------------------------------------------
// Return current raster object
//-----------------------------------------------------------------------------
const HFCPtr<HRARaster>& HRAReferenceToRasterIterator::operator()()
    {
    if (m_pRasterToReturn == 0)
        {
        // Make a reference to the current source raster object only if
        // it is not null!
        if (m_pCurrentSourceRaster != 0)
            {
            HFCPtr<HGF2DCoordSys> pCSToApply;

            if (m_pCurrentSourceRaster->GetCoordSys() == ((HFCPtr<HRAReferenceToRaster>&)GetRaster())->GetSource()->GetCoordSys())
                {
                // Simple case: object to return is as expected.
                pCSToApply = GetRaster()->GetCoordSys();
                }
            else
                {
                // Object to return doesn't use the CS of our source. We must create
                // a new CS that applies our transformation correctly based on
                // that object's CS.

                // Extract the transformation we're applying to our source (T1)
                HFCPtr<HGF2DTransfoModel> pRefTransfo(GetRaster()->GetCoordSys()->GetTransfoModelTo(((HFCPtr<HRAReferenceToRaster>&)GetRaster())->GetSource()->GetCoordSys()));

                // Calculate the transformation between our source's CS and the CS
                // the object to return uses. (T2)
                HFCPtr<HGF2DTransfoModel> pCurrentToSource(m_pCurrentSourceRaster->GetCoordSys()->GetTransfoModelTo(((HFCPtr<HRAReferenceToRaster>&)GetRaster())->GetSource()->GetCoordSys()));

                // Create the CS by composing everything correctly :)
                // T2 o T1 o -T2
                HFCPtr<HGF2DTransfoModel> pToApply(pCurrentToSource->ComposeInverseWithDirectOf(*pRefTransfo)->ComposeInverseWithInverseOf(*pCurrentToSource));
                pCSToApply = new HGF2DCoordSys(*pToApply, m_pCurrentSourceRaster->GetCoordSys());
                }

            if (m_pCurrentSourceRaster->GetClassID() == HRAReferenceToRaster::CLASS_ID)
                {
                // Try composing references together...
                m_pRasterToReturn = ((HFCPtr<HRAReferenceToRaster>&)m_pCurrentSourceRaster)->TryToCompose(pCSToApply);
                }
            else
                {
                // Composition wasn't possible. Create a new layer.
                m_pRasterToReturn = new HRAReferenceToRaster(m_pCurrentSourceRaster,
                                                             pCSToApply,
                                                             true);
                }
            }
        }

    // Return copy of internal raster pointer
    return m_pRasterToReturn;
    }


//-----------------------------------------------------------------------------
// Reset to start state
//-----------------------------------------------------------------------------
void HRAReferenceToRasterIterator::Reset()
    {
    // Reset internal raster iterator
    m_pSourceIterator->Reset();

    // Take pointer to first useful raster
    m_pCurrentSourceRaster = (*m_pSourceIterator)();

    m_pRasterToReturn = 0;
    }



//-----------------------------------------------------------------------------
// Print the object's state
//-----------------------------------------------------------------------------
#ifdef __HMR_PRINTSTATE
void HRAReferenceToRasterIterator::PrintState(ostream& po_rOutput) const
    {
    // Call ancestor
    HRARasterIterator::PrintState(po_rOutput);

    po_rOutput
            << "HRAReferenceToRasterIterator"
            << endl;
    }
#endif


// ----------------------------------------------------------- Privates

