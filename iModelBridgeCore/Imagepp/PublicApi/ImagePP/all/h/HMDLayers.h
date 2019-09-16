//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDMetaDataContainer.h"
#include "HMDLayerInfo.h"

BEGIN_IMAGEPP_NAMESPACE
class HMDLayers : public HMDMetaDataContainer
    {
    HDECLARE_CLASS_ID(HMDLayersId_Base, HMDMetaDataContainer);

public :
    IMAGEPP_EXPORT HMDLayers();
    IMAGEPP_EXPORT virtual ~HMDLayers();

    IMAGEPP_EXPORT HMDLayers(const HMDLayers& pi_rObj);

    IMAGEPP_EXPORT HFCPtr<HMDMetaDataContainer> Clone() const override;

    IMAGEPP_EXPORT void                          AddLayer       (const HMDLayerInfo*      pi_pLayer);

    IMAGEPP_EXPORT const HMDLayerInfo*           GetLayer       (uint16_t          pi_Index) const;
    IMAGEPP_EXPORT uint16_t                GetNbLayers    () const;
    IMAGEPP_EXPORT bool                          GetIndexFromKey(const Utf8String&           pi_rKey,
                                                         uint16_t&                 po_rIndex) const;
    IMAGEPP_EXPORT void                          Merge          (const HFCPtr<HMDLayers>& pi_rLayers);

protected:

    //Avoid the copy
    typedef vector<const HMDLayerInfo*> LayerInfoVec;

    LayerInfoVec m_layerInfoVec;

private :
    HMDLayers& operator=(const HMDLayers& pi_rObj);

    void CopyMemberData(const HMDLayers& pi_rObj);


    };

END_IMAGEPP_NAMESPACE
