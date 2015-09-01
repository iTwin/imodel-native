//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDLayerInfoPDF.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDLayerInfo.h"

BEGIN_IMAGEPP_NAMESPACE

class HMDLayerInfoPDF : public HMDLayerInfo
    {
    HDECLARE_CLASS_ID(HMDLayerInfoId_PDF, HMDLayerInfo);

public :

    IMAGEPP_EXPORT HMDLayerInfoPDF(const WString& pi_rKeyName,
                    bool          pi_InitialVisibleState,
                    const WString& pi_rLayerName);
    IMAGEPP_EXPORT virtual ~HMDLayerInfoPDF();

    IMAGEPP_EXPORT const WString& GetLayerName() const;

protected :

    WString m_LayerName;

private :

    HMDLayerInfoPDF(const HMDLayerInfoPDF& pi_rObj);
    HMDLayerInfoPDF& operator=(const HMDLayerInfoPDF& pi_rObj);
    };

END_IMAGEPP_NAMESPACE