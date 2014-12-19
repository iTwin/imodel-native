//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDLayersPDF.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDLayers.h"
#include "HMDLayerInfoPDF.h"

class HMDLayersPDF : public HMDLayers
    {
    HDECLARE_CLASS_ID(7008, HMDLayers);

public :
    _HDLLu  HMDLayersPDF();
    _HDLLu  virtual ~HMDLayersPDF();

    _HDLLu  const HMDLayerInfoPDF* GetLayer(unsigned short pi_Index) const;

private :

    HMDLayersPDF(const HMDLayersPDF& pi_rObj);
    HMDLayersPDF& operator=(const HMDLayersPDF& pi_rObj);
    };

