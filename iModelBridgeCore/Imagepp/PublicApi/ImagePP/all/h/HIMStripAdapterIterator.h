//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMStripAdapterIterator.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIMStripAdapterIterator
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HVEShape.h"
#include "HRARasterIterator.h"
#include "HIMStripAdapter.h"

class HRARaster;

class HIMStripAdapterIterator: public HRARasterIterator
    {
public:

    friend class HIMStripAdapter; // use only with new constructor...Pyramid

    // Primary methods

    HIMStripAdapterIterator(const HFCPtr<HIMStripAdapter>& pi_pReference,
                            const HRAIteratorOptions&      pi_rOptions);

    virtual         ~HIMStripAdapterIterator();

    // Iterator operation

    virtual const HFCPtr<HRARaster>&
    Next();

    virtual const HFCPtr<HRARaster>&
    operator()();

    virtual void    Reset();


protected:

private:

    // members
    double             m_XMin;
    double             m_YMin;
    uint32_t            m_StripWidth;
    uint32_t            m_StripHeight;
    uint32_t            m_CurStrip;
    uint32_t            m_NumberOfStrips;

    HFCPtr<HRARaster>   m_pRasterToReturn;
    HFCPtr<HGF2DCoordSys>
    m_pResolutionPhysicalCoordSys;

    // private methods
    void                InitObject();
    void                ComputeStrip();

    HIMStripAdapterIterator(const HIMStripAdapterIterator& pi_rObj);
    HIMStripAdapterIterator&
    operator=(const HIMStripAdapterIterator& pi_rObj);
    };
