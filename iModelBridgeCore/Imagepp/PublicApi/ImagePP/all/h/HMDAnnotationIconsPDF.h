//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDAnnotationIconsPDF.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDLayers.h"
#include "HMDLayerInfoPDF.h"

class HMDAnnotationIconsPDF : public HMDMetaDataContainer
    {
    HDECLARE_CLASS_ID(7013, HMDMetaDataContainer);

public :
    _HDLLu HMDAnnotationIconsPDF();
    _HDLLu virtual     ~HMDAnnotationIconsPDF();

    _HDLLu HMDAnnotationIconsPDF(const HMDAnnotationIconsPDF& pi_rObj);

    _HDLLu void        SetRasterization(bool pi_RasterizeIcon);
    _HDLLu bool       GetRasterization();

    _HDLLu virtual HFCPtr<HMDMetaDataContainer> Clone() const;

private :
    HMDAnnotationIconsPDF& operator=(const HMDAnnotationIconsPDF& pi_rObj);

    bool m_RasterizeIcon;
    };

