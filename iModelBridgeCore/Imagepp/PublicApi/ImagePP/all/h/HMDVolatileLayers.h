//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDVolatileLayers.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDMetaDataContainer.h"
#include <ImagePP/all/h/HMDVolatileLayerInfo.h>

BEGIN_IMAGEPP_NAMESPACE

class HMDLayerInfo;
class HMDLayers;
class HMDVolatileLayerInfo;

class HMDVolatileLayers : public HMDMetaDataContainer
    {
    HDECLARE_CLASS_ID(HMDVolatileLayersId_Base, HMDMetaDataContainer);

public :
    IMAGEPP_EXPORT HMDVolatileLayers(const HFCPtr<HMDLayers>& pi_rpLayers);
    IMAGEPP_EXPORT virtual ~HMDVolatileLayers();

    IMAGEPP_EXPORT HMDVolatileLayers(const HMDVolatileLayers& pi_rObj);

    IMAGEPP_EXPORT HFCPtr<HMDMetaDataContainer> Clone() const override;

    IMAGEPP_EXPORT bool                                SameLayersOn        (const HMDVolatileLayers& pi_rObj) const;

    IMAGEPP_EXPORT uint16_t                      GetNbVolatileLayers () const;
    IMAGEPP_EXPORT HMDVolatileLayerInfo*                GetVolatileLayerInfo(uint16_t pi_Index);
    IMAGEPP_EXPORT const HMDLayerInfo*                  GetLayerInfo        (uint16_t pi_Index) const;
    IMAGEPP_EXPORT bool                                GetIndexFromKey     (const Utf8String& pi_rKey,
                                                              uint16_t&       po_rIndex) const;

    IMAGEPP_EXPORT void                                 ResetInitialVisibleState();

protected:

    HArrayAutoPtr<HAutoPtr<HMDVolatileLayerInfo> > m_ppVolatileLayers;
    uint16_t                                 m_NbVolatileLayers;

private:
    HMDVolatileLayers& operator=(const HMDVolatileLayers& pi_rObj);
    void CopyMemberData(const HMDVolatileLayers& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
