//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDLayerInfo.h"

BEGIN_IMAGEPP_NAMESPACE

class HMDLayerInfoPDF : public HMDLayerInfo
    {
    HDECLARE_CLASS_ID(HMDLayerInfoId_PDF, HMDLayerInfo);

public :

    IMAGEPP_EXPORT HMDLayerInfoPDF(const Utf8String& pi_rKeyName,
                    bool          pi_InitialVisibleState,
                    const Utf8String& pi_rLayerName);
    IMAGEPP_EXPORT virtual ~HMDLayerInfoPDF();

    IMAGEPP_EXPORT const Utf8String& GetLayerName() const;

protected :

    Utf8String m_LayerName;

private :

    HMDLayerInfoPDF(const HMDLayerInfoPDF& pi_rObj);
    HMDLayerInfoPDF& operator=(const HMDLayerInfoPDF& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
