//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDLayerInfoWMS.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HMDLayerInfo.h"

BEGIN_IMAGEPP_NAMESPACE
class HMDLayerInfoWMS : public HMDLayerInfo
    {
    HDECLARE_CLASS_ID(HMDLayerInfoId_WMS, HMDLayerInfo);

public :
    IMAGEPP_EXPORT  HMDLayerInfoWMS(const WString&  pi_rKeyName,
                            const WString&  pi_rLayerName,
                            const WString&  pi_rLayerTitle,
                            const WString&  pi_rLayerAbstract,
                            const double   pi_MinScaleHint,
                            const double   pi_MaxScaleHint,
                            const WString&  pi_rStyleName,
                            const WString&  pi_rStyleTitle,
                            const WString&  pi_rStyleAbstract,
                            bool           pi_Opaque);
    IMAGEPP_EXPORT virtual  ~HMDLayerInfoWMS();

    IMAGEPP_EXPORT const WString&  GetLayerName() const;
    IMAGEPP_EXPORT const WString&  GetLayerTitle() const;
    IMAGEPP_EXPORT const WString&  GetLayerAbstract() const;
    IMAGEPP_EXPORT const WString&  GetStyleName() const;
    IMAGEPP_EXPORT const WString&  GetStyleTitle() const;
    IMAGEPP_EXPORT const WString&  GetStyleAbstract() const;
    IMAGEPP_EXPORT double         GetMinScaleHint() const;
    IMAGEPP_EXPORT double         GetMaxScaleHint() const;
    IMAGEPP_EXPORT bool           IsOpaque() const;

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

END_IMAGEPP_NAMESPACE