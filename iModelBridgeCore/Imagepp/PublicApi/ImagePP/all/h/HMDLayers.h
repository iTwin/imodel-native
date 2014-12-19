//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDLayers.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDMetaDataContainer.h"
#include "HMDLayerInfo.h"

class HMDLayers : public HMDMetaDataContainer
    {
    HDECLARE_CLASS_ID(7003, HMDMetaDataContainer);

public :
    _HDLLu HMDLayers();
    _HDLLu virtual ~HMDLayers();

    _HDLLu HMDLayers(const HMDLayers& pi_rObj);

    _HDLLu virtual HFCPtr<HMDMetaDataContainer> Clone() const;

    _HDLLu void                          AddLayer       (const HMDLayerInfo*      pi_pLayer);

    _HDLLu const HMDLayerInfo*           GetLayer       (unsigned short          pi_Index) const;
    _HDLLu unsigned short                GetNbLayers    () const;
    _HDLLu bool                          GetIndexFromKey(const WString&           pi_rKey,
                                                         unsigned short&                 po_rIndex) const;
    _HDLLu void                          Merge          (const HFCPtr<HMDLayers>& pi_rLayers);

protected:

    //Avoid the copy
    typedef vector<const HMDLayerInfo*> LayerInfoVec;

    LayerInfoVec m_layerInfoVec;

private :
    HMDLayers& operator=(const HMDLayers& pi_rObj);

    void CopyMemberData(const HMDLayers& pi_rObj);


    };

