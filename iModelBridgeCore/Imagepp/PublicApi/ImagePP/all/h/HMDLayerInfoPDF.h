//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDLayerInfoPDF.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDLayerInfo.h"

class HMDLayerInfoPDF : public HMDLayerInfo
    {
    HDECLARE_CLASS_ID(7001, HMDLayerInfo);

public :

    _HDLLu HMDLayerInfoPDF(const WString& pi_rKeyName,
                    bool          pi_InitialVisibleState,
                    const WString& pi_rLayerName);
    _HDLLu virtual ~HMDLayerInfoPDF();

    _HDLLu const WString& GetLayerName() const;

protected :

    WString m_LayerName;

private :

    HMDLayerInfoPDF(const HMDLayerInfoPDF& pi_rObj);
    HMDLayerInfoPDF& operator=(const HMDLayerInfoPDF& pi_rObj);
    };

