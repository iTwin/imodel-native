//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDLayersPDF.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDLayers.h"
#include "HMDLayerInfoPDF.h"

BEGIN_IMAGEPP_NAMESPACE
class HMDLayersPDF : public HMDLayers
    {
    HDECLARE_CLASS_ID(HMDLayersId_PDF, HMDLayers);

public :
    IMAGEPP_EXPORT  HMDLayersPDF();
    IMAGEPP_EXPORT  virtual ~HMDLayersPDF();

    IMAGEPP_EXPORT  const HMDLayerInfoPDF* GetLayer(unsigned short pi_Index) const;

private :

    HMDLayersPDF(const HMDLayersPDF& pi_rObj);
    HMDLayersPDF& operator=(const HMDLayersPDF& pi_rObj);
    };

END_IMAGEPP_NAMESPACE