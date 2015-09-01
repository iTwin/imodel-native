//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDLayersWMS.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDLayers.h"
#include "HMDLayerInfoWMS.h"

BEGIN_IMAGEPP_NAMESPACE
class HMDLayersWMS : public HMDLayers
    {
    HDECLARE_CLASS_ID(HMDLayersId_WMS, HMDLayers);

public :
    IMAGEPP_EXPORT HMDLayersWMS();
    IMAGEPP_EXPORT virtual                ~HMDLayersWMS();

    IMAGEPP_EXPORT const HMDLayerInfoWMS* GetLayer(unsigned short pi_Index) const;

private :

    HMDLayersWMS(const HMDLayersWMS& pi_rObj);
    HMDLayersWMS& operator=(const HMDLayersWMS& pi_rObj);
    };

END_IMAGEPP_NAMESPACE