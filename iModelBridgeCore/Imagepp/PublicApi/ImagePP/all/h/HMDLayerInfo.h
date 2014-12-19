//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDLayerInfo.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDMetaData.h"

class HMDLayerInfo : public HMDMetaData
    {
    HDECLARE_CLASS_ID(7000, HMDMetaData);

    friend class HMDLayers;

public :
    _HDLLu HMDLayerInfo(const WString& pi_rKeyName,
                 bool    pi_InitialVisibleState);
    _HDLLu virtual ~HMDLayerInfo();

    _HDLLu HMDLayerInfo(const HMDLayerInfo& pi_rObj);
    _HDLLu HMDLayerInfo& operator=(const HMDLayerInfo& pi_rObj);

    _HDLLu bool          GetInitialVisibleState() const;

    _HDLLu const WString& GetKeyName() const;

protected:

    void           SetInitialVisibleState(bool pi_VisibleState) ;

    WString m_KeyName;
    bool   m_InitialVisibleState;

private :

    void CopyMemberData(const HMDLayerInfo& pi_rObj);

    };

