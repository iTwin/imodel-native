//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDLayerInfo.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDMetaData.h"

BEGIN_IMAGEPP_NAMESPACE

class HMDLayerInfo : public HMDMetaData
    {
    HDECLARE_CLASS_ID(HMDLayerInfoId_Base, HMDMetaData);

    friend class HMDLayers;

public :
    IMAGEPP_EXPORT HMDLayerInfo(const Utf8String& pi_rKeyName,
                 bool    pi_InitialVisibleState);
    IMAGEPP_EXPORT virtual ~HMDLayerInfo();

    IMAGEPP_EXPORT HMDLayerInfo(const HMDLayerInfo& pi_rObj);
    IMAGEPP_EXPORT HMDLayerInfo& operator=(const HMDLayerInfo& pi_rObj);

    IMAGEPP_EXPORT bool          GetInitialVisibleState() const;

    IMAGEPP_EXPORT const Utf8String& GetKeyName() const;

protected:

    void           SetInitialVisibleState(bool pi_VisibleState) ;

    Utf8String m_KeyName;
    bool   m_InitialVisibleState;

private :

    void CopyMemberData(const HMDLayerInfo& pi_rObj);

    };

END_IMAGEPP_NAMESPACE
