//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFDrawOptions.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>+--------------------------------------------------------------------------------------
// Class : HGFDrawOptions
//-----------------------------------------------------------------------------
#pragma once

#include "HPMAttributeSet.h"
#include "HGSTypes.h"
#include "HGFPreDrawOptions.h"


class HGFDrawOptions
    {
    HDECLARE_BASECLASS_ID(1758)

public:

    // Primary methods

    _HDLLg                             HGFDrawOptions();

    _HDLLg                             HGFDrawOptions(const HGFDrawOptions& pi_rOptions);
    _HDLLg                             HGFDrawOptions(const HGFDrawOptions* pi_pOptions);

    _HDLLg virtual                     ~HGFDrawOptions();


    // Operators

    HGFDrawOptions&         operator=(const HGFDrawOptions& pi_rObj);
    HGFDrawOptions&         operator=(const HGFDrawOptions* pi_pObj);


    // Filters

    bool                       ApplyAlphaBlend() const;
    void                        SetAlphaBlend(bool pi_ApplyDithering);

    const HGSResampling&        GetResamplingMode() const;
    void                        SetResamplingMode(const HGSResampling& pi_rResampling);

    const HPMAttributeSet&      GetAttributes() const;
    void                        SetAttributes(const HPMAttributeSet& pi_rpAttributes);

    void                        AbortDraw();
    void                        ResetAbortRequest();
    bool                       ShouldAbort() const;

    void                        SetTemporaryRenderMode(bool pi_TemporaryMode);
    bool                       TemporaryRenderMode() const;

    void                        SetOverviewMode(bool pi_mode);
    bool                       GetOverviewMode() const;

    void                        SetPreDrawOptions(HGFPreDrawOptions* pi_prPreDrawOptions);
    HGFPreDrawOptions*            GetPreDrawOptions();

    const HGFPreDrawOptions*    GetPreDrawOptions() const;

protected:

private:

    bool                       m_AbortRequest;

    HGSResampling               m_Resampling;

    bool                       m_ApplyVectorAware;

    bool                       m_ApplyAlphaBlend;

    bool                       m_InTemporaryRenderMode;

    HPMAttributeSet             m_Attributes;

    bool                       m_OverviewMode;

    HFCPtr<HGFPreDrawOptions>     m_pPreDrawOptions;
    };

#include "HGFDrawOptions.hpp"
