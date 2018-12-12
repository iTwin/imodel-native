//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRARasterIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRARasterIterator
//-----------------------------------------------------------------------------
// Raster iterator : it scans raster data for a specific region in a raster
// object, in order to get physical raster objects.  These objects can
// themselves be scanned using raster editors.  Abstract class.
//-----------------------------------------------------------------------------

#pragma once

#include "HVEShape.h"
#include "HRARaster.h"

BEGIN_IMAGEPP_NAMESPACE
class HNOVTABLEINIT HRARasterIterator
    {
public:

    // Primary methods

    HRARasterIterator(const HFCPtr<HRARaster>&  pi_pRaster,
                      const HRAIteratorOptions& pi_rOptions);

    HRARasterIterator(const HRARasterIterator&  pi_rObj);

    HRARasterIterator&       operator=(const HRARasterIterator& pi_rObj);

    virtual                  ~HRARasterIterator();

    // Iterator operation

    virtual const HFCPtr<HRARaster>&
    Next() = 0;
    virtual const HFCPtr<HRARaster>&
    operator()() = 0;
    virtual void            Reset() = 0;

    // Other methods

    const HFCPtr<HRARaster>&
    GetRaster() const;
    const HRAIteratorOptions&
    GetOptions() const;

    // Debug function
    virtual void    PrintState(ostream& po_rOutput) const;

protected:


private:

    // iterator options
    HRAIteratorOptions      m_Options;

    // Pointer to iterated raster
    HFCPtr<HRARaster>       m_pRaster;
    };
END_IMAGEPP_NAMESPACE

#include "HRARasterIterator.hpp"

