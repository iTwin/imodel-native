//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDLayerInfoWMS.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDLayerInfo.h"

class HMDLayerInfoWMS : public HMDLayerInfo
    {
    HDECLARE_CLASS_ID(7002, HMDLayerInfo);

public :
    _HDLLu  HMDLayerInfoWMS(const WString&  pi_rKeyName,
                            const WString&  pi_rLayerName,
                            const WString&  pi_rLayerTitle,
                            const WString&  pi_rLayerAbstract,
                            const double   pi_MinScaleHint,
                            const double   pi_MaxScaleHint,
                            const WString&  pi_rStyleName,
                            const WString&  pi_rStyleTitle,
                            const WString&  pi_rStyleAbstract,
                            bool           pi_Opaque);
    _HDLLu virtual  ~HMDLayerInfoWMS();

    _HDLLu const WString&  GetLayerName() const;
    _HDLLu const WString&  GetLayerTitle() const;
    _HDLLu const WString&  GetLayerAbstract() const;
    _HDLLu const WString&  GetStyleName() const;
    _HDLLu const WString&  GetStyleTitle() const;
    _HDLLu const WString&  GetStyleAbstract() const;
    _HDLLu double         GetMinScaleHint() const;
    _HDLLu double         GetMaxScaleHint() const;
    _HDLLu bool           IsOpaque() const;

protected:

private :

    // members
    WString     m_LayerName;
    WString     m_LayerTitle;
    WString     m_LayerAbstract;
    WString     m_StyleName;
    WString     m_StyleTitle;
    WString     m_StyleAbstract;
    double     m_MinScaleHint;
    double     m_MaxScaleHint;
    bool       m_Opaque;

    HMDLayerInfoWMS(const HMDLayerInfoWMS& pi_rObj);
    HMDLayerInfoWMS& operator=(const HMDLayerInfoWMS& pi_rObj);
    };

