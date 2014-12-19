//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDVolatileLayerInfo.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDMetaData.h"

class HMDVolatileLayerInfo : public HMDMetaData
    {
    HDECLARE_CLASS_ID(7006, HMDMetaData);

public :
    _HDLLu HMDVolatileLayerInfo(const HMDLayerInfo* pi_pLayerInfo);
    _HDLLu virtual ~HMDVolatileLayerInfo();

    _HDLLu HMDVolatileLayerInfo(const HMDVolatileLayerInfo& pi_rObj);
    _HDLLu HMDVolatileLayerInfo& operator=(const HMDVolatileLayerInfo& pi_rObj);

    _HDLLu const HMDLayerInfo* GetLayerInfo() const;
    _HDLLu bool               GetVisibleState() const;
    _HDLLu void                SetVisibleState(bool pi_NewVisibleState);

protected :

    const HMDLayerInfo* m_pLayerInfo;
    bool               m_VisibleState;

private :

    void CopyMemberData(const HMDVolatileLayerInfo& pi_rObj);
    };

