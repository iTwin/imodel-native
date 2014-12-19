//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDLayersWMS.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDLayers.h"
#include "HMDLayerInfoWMS.h"

class HMDLayersWMS : public HMDLayers
    {
    HDECLARE_CLASS_ID(7009, HMDLayers);

public :
    _HDLLu HMDLayersWMS();
    _HDLLu virtual                ~HMDLayersWMS();

    _HDLLu const HMDLayerInfoWMS* GetLayer(unsigned short pi_Index) const;

private :

    HMDLayersWMS(const HMDLayersWMS& pi_rObj);
    HMDLayersWMS& operator=(const HMDLayersWMS& pi_rObj);
    };

