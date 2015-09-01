//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMStripAdapterIterator.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIMStripAdapterIterator
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HRARasterIterator.h"

BEGIN_IMAGEPP_NAMESPACE
class HRARaster;
class HRABitmap;
class HIMStripAdapter;

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

    HFCPtr<HRABitmap>   m_pRasterToReturn;
    HFCPtr<HGF2DCoordSys>
    m_pResolutionPhysicalCoordSys;

    // private methods
    void                InitObject();
    void                ComputeStrip();

    HIMStripAdapterIterator(const HIMStripAdapterIterator& pi_rObj);
    HIMStripAdapterIterator&
    operator=(const HIMStripAdapterIterator& pi_rObj);
    };

END_IMAGEPP_NAMESPACE