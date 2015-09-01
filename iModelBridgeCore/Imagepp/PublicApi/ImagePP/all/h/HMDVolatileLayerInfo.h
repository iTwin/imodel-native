//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDVolatileLayerInfo.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDMetaData.h"

BEGIN_IMAGEPP_NAMESPACE
class HMDVolatileLayerInfo : public HMDMetaData
    {
    HDECLARE_CLASS_ID(HMDVolatileLayersId_Info, HMDMetaData);

public :
    IMAGEPP_EXPORT HMDVolatileLayerInfo(const HMDLayerInfo* pi_pLayerInfo);
    IMAGEPP_EXPORT virtual ~HMDVolatileLayerInfo();

    IMAGEPP_EXPORT HMDVolatileLayerInfo(const HMDVolatileLayerInfo& pi_rObj);
    IMAGEPP_EXPORT HMDVolatileLayerInfo& operator=(const HMDVolatileLayerInfo& pi_rObj);

    IMAGEPP_EXPORT const HMDLayerInfo* GetLayerInfo() const;
    IMAGEPP_EXPORT bool               GetVisibleState() const;
    IMAGEPP_EXPORT void                SetVisibleState(bool pi_NewVisibleState);

protected :

    const HMDLayerInfo* m_pLayerInfo;
    bool               m_VisibleState;

private :

    void CopyMemberData(const HMDVolatileLayerInfo& pi_rObj);
    };

END_IMAGEPP_NAMESPACE