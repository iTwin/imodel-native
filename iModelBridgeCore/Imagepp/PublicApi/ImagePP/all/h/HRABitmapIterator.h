//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABitmapIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class: HRABitmapIterator
// ----------------------------------------------------------------------------

#pragma once

#include "HRARasterIterator.h"

BEGIN_IMAGEPP_NAMESPACE
class HRABitmapBase;

// ----------------------------------------------------------------------------
//  HRABitmapIterator
// ----------------------------------------------------------------------------
class HRABitmapIterator : public HRARasterIterator

    {
public:
    // Primary methods

    HRABitmapIterator   (const HFCPtr<HRABitmapBase>&  pi_pRaster,
                         const HRAIteratorOptions& pi_rOptions);

    HRABitmapIterator   (const HRABitmapIterator& pi_rObj);

    virtual         ~HRABitmapIterator(void);

    HRABitmapIterator&      operator=(const HRABitmapIterator& pi_rBitmap);

    // Inherited from HRARasterIterator

    virtual const HFCPtr<HRARaster>&
    Next();
    virtual const HFCPtr<HRARaster>&
    operator()();

    virtual void            Reset();

    // Debug function
    virtual void    PrintState(ostream& po_rOutput) const;

protected:

private:
    // Members

    // Index on the iteration in the raster.
    uint32_t            m_Index;

    // null raster to return when finished iterating
    HFCPtr<HRARaster>   m_pNullRaster;
    };


END_IMAGEPP_NAMESPACE