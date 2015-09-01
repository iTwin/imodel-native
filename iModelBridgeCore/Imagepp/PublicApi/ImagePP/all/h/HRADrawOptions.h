//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRADrawOptions.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRADrawOptions
//-----------------------------------------------------------------------------

#pragma once

#include "HPMAttributeSet.h"
#include "HGSTypes.h"
#include "HVEShape.h"
#include "HRPPixelType.h"
#include "HRPFilter.h"
#include "HRACopyFromOptions.h"

BEGIN_IMAGEPP_NAMESPACE
class HRATransaction;

class HRADrawOptions
    {
    HDECLARE_SEALEDCLASS_ID(HRADrawOptionsId)

public:

    // Primary methods

    IMAGEPP_EXPORT HRADrawOptions();
    IMAGEPP_EXPORT HRADrawOptions(const HRADrawOptions& pi_rOptions);
    IMAGEPP_EXPORT HRADrawOptions(const HRACopyFromLegacyOptions& pi_rCFOptions);

    IMAGEPP_EXPORT ~HRADrawOptions();


    // Operators

    HRADrawOptions&             operator=(const HRADrawOptions& pi_rObj);

    //TR 300554 - Temporary fix the problem for STM raster draping only.
    bool                       GetDataDimensionFix() const;
    void                        SetDataDimensionFix(bool pi_fix);

    HFCPtr<HVEShape>            GetShape() const;
    void                        SetShape(const HFCPtr<HVEShape>& pi_rpShape);

    void                        SetGridShape(bool pi_GridShape);
    bool                        ApplyGridShape() const;

    HFCPtr<HGF2DCoordSys>       GetReplacingCoordSys() const;
    void                        SetReplacingCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);

    HFCPtr<HRPPixelType>        GetReplacingPixelType() const;
    void                        SetReplacingPixelType(const HFCPtr<HRPPixelType>& pi_rpPixelType);

    void                        SetTransaction(const HFCPtr<HRATransaction>& pi_rpTransaction);
    HFCPtr<HRATransaction>      GetTransaction() const;

    bool                        ApplyAlphaBlend() const;
    void                        SetAlphaBlend(bool pi_ApplyDithering);

    const HGSResampling&        GetResamplingMode() const;
    void                        SetResamplingMode(const HGSResampling& pi_rResampling);

    const HPMAttributeSet&      GetAttributes() const;
    void                        SetAttributes(const HPMAttributeSet& pi_rpAttributes);

    void                        SetOverviewMode(bool pi_mode);
    bool                        GetOverviewMode() const;

private:

    HGSResampling               m_Resampling;
    bool                        m_ApplyAlphaBlend;
    HPMAttributeSet             m_Attributes;
    bool                        m_OverviewMode;
    HFCPtr<HVEShape>            m_pShape;
    bool                        m_ApplyGridShape;
    HFCPtr<HGF2DCoordSys>       m_pReplacingCoordSys;
    HFCPtr<HRPPixelType>        m_pReplacingPixelType;
    HFCPtr<HRATransaction>      m_pTransaction;
    bool                        m_DataDimensionFix;
    };

END_IMAGEPP_NAMESPACE
#include "HRADrawOptions.hpp"

