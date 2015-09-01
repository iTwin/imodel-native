//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAReferenceToRasterIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRAReferenceToRasterIterator
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVEShape.h"
#include "HRARasterIterator.h"
#include "HRAReferenceToRaster.h"

BEGIN_IMAGEPP_NAMESPACE
class HRARaster;
class HRAReferenceToRaster;

class HRAReferenceToRasterIterator: public HRARasterIterator
    {
public:

    friend class HRAReferenceToRaster;

    // Primary methods

    HRAReferenceToRasterIterator(const HFCPtr<HRAReferenceToRaster>& pi_pReference,
                                 const HRAIteratorOptions&           pi_rOptions);

    HRAReferenceToRasterIterator(const HRAReferenceToRasterIterator& pi_rObj);

    HRAReferenceToRasterIterator&
    operator=(const HRAReferenceToRasterIterator& pi_rObj);

    virtual         ~HRAReferenceToRasterIterator();

    // Iterator operation

    virtual const HFCPtr<HRARaster>&
    Next();

    virtual const HFCPtr<HRARaster>&
    operator()();

    virtual void    Reset();

    // Debug function
#ifdef __HMR_PRINTSTATE
    virtual void    PrintState(ostream& po_rOutput) const;
#endif

protected:

private:

    // Internal iterator on source raster.
    HAutoPtr<HRARasterIterator> m_pSourceIterator;

    // Current raster (from source's iterator)
    HFCPtr<HRARaster>   m_pCurrentSourceRaster;

    // Current "ready to return" raster
    HFCPtr<HRARaster>   m_pRasterToReturn;

    // Note if the iterator is shaped or not
    bool               m_Shaped;

    HFCPtr<HGF2DCoordSys> m_pResolutionPhysicalCoordSys;
    };
END_IMAGEPP_NAMESPACE

