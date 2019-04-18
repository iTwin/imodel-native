//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDLayerInfo.h"

BEGIN_IMAGEPP_NAMESPACE
class HMDLayerInfoWMS : public HMDLayerInfo
    {
    HDECLARE_CLASS_ID(HMDLayerInfoId_WMS, HMDLayerInfo);

public :
    IMAGEPP_EXPORT  HMDLayerInfoWMS(const Utf8String&  pi_rKeyName,
                            const Utf8String&  pi_rLayerName,
                            const Utf8String&  pi_rLayerTitle,
                            const Utf8String&  pi_rLayerAbstract,
                            const double   pi_MinScaleHint,
                            const double   pi_MaxScaleHint,
                            const Utf8String&  pi_rStyleName,
                            const Utf8String&  pi_rStyleTitle,
                            const Utf8String&  pi_rStyleAbstract,
                            bool           pi_Opaque);
    IMAGEPP_EXPORT virtual  ~HMDLayerInfoWMS();

    IMAGEPP_EXPORT const Utf8String&  GetLayerName() const;
    IMAGEPP_EXPORT const Utf8String&  GetLayerTitle() const;
    IMAGEPP_EXPORT const Utf8String&  GetLayerAbstract() const;
    IMAGEPP_EXPORT const Utf8String&  GetStyleName() const;
    IMAGEPP_EXPORT const Utf8String&  GetStyleTitle() const;
    IMAGEPP_EXPORT const Utf8String&  GetStyleAbstract() const;
    IMAGEPP_EXPORT double         GetMinScaleHint() const;
    IMAGEPP_EXPORT double         GetMaxScaleHint() const;
    IMAGEPP_EXPORT bool           IsOpaque() const;

protected:

private :

    // members
    Utf8String     m_LayerName;
    Utf8String     m_LayerTitle;
    Utf8String     m_LayerAbstract;
    Utf8String     m_StyleName;
    Utf8String     m_StyleTitle;
    Utf8String     m_StyleAbstract;
    double     m_MinScaleHint;
    double     m_MaxScaleHint;
    bool       m_Opaque;

    HMDLayerInfoWMS(const HMDLayerInfoWMS& pi_rObj);
    HMDLayerInfoWMS& operator=(const HMDLayerInfoWMS& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
