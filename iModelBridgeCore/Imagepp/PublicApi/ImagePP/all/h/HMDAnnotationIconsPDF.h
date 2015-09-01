//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDAnnotationIconsPDF.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDLayers.h"
#include "HMDLayerInfoPDF.h"

BEGIN_IMAGEPP_NAMESPACE

class HMDAnnotationIconsPDF : public HMDMetaDataContainer
    {
    HDECLARE_CLASS_ID(HMDAnnotationId_IconsPDF, HMDMetaDataContainer);

public :
    IMAGEPP_EXPORT HMDAnnotationIconsPDF();
    IMAGEPP_EXPORT virtual     ~HMDAnnotationIconsPDF();

    IMAGEPP_EXPORT HMDAnnotationIconsPDF(const HMDAnnotationIconsPDF& pi_rObj);

    IMAGEPP_EXPORT void        SetRasterization(bool pi_RasterizeIcon);
    IMAGEPP_EXPORT bool       GetRasterization();

    IMAGEPP_EXPORT virtual HFCPtr<HMDMetaDataContainer> Clone() const;

private :
    HMDAnnotationIconsPDF& operator=(const HMDAnnotationIconsPDF& pi_rObj);

    bool m_RasterizeIcon;
    };

END_IMAGEPP_NAMESPACE