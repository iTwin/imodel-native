//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDLayers.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    IMAGEPP_EXPORT virtual HFCPtr<HMDMetaDataContainer> Clone() const;

    IMAGEPP_EXPORT void                          AddLayer       (const HMDLayerInfo*      pi_pLayer);

    IMAGEPP_EXPORT const HMDLayerInfo*           GetLayer       (unsigned short          pi_Index) const;
    IMAGEPP_EXPORT unsigned short                GetNbLayers    () const;
    IMAGEPP_EXPORT bool                          GetIndexFromKey(const WString&           pi_rKey,
                                                         unsigned short&                 po_rIndex) const;
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