//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSFilter.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HGSFilter
//:>-----------------------------------------------------------------------------
//:> General class for HGSFilter.
//:>-----------------------------------------------------------------------------

#pragma once

#include "HGSSurfaceOption.h"
#include "HRPFilter.h"


class HGSFilter : public HGSSurfaceOption
    {
    HDECLARE_CLASS_ID(1715, HGSSurfaceOption)

public:

    // Primary methods
    HGSFilter(const HFCPtr<HRPFilter>&      pi_rpFilter);
    HGSFilter(const HGSFilter&              pi_rObj);
    virtual        ~HGSFilter();

    virtual HGSSurfaceOption*
    Clone() const;

    const HFCPtr<HRPFilter>&
    GetFilter() const;
    void            SetFilter(const HFCPtr<HRPFilter>&      pi_rpFilter);

protected:

private:

    // members
    HFCPtr<HRPFilter>   m_pFilter;

    // disabled methods
    HGSRegion&      operator=(const HGSRegion& pi_rObj);
    };


#include "HGSFilter.hpp"
