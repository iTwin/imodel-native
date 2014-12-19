//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDVolatileLayers.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDMetaDataContainer.h"
#include <Imagepp/all/h/HMDVolatileLayerInfo.h>

class HMDLayerInfo;
class HMDLayers;
class HMDVolatileLayerInfo;

class HMDVolatileLayers : public HMDMetaDataContainer
    {
    HDECLARE_CLASS_ID(7007, HMDMetaDataContainer);

public :
    _HDLLu HMDVolatileLayers(const HFCPtr<HMDLayers>& pi_rpLayers);
    _HDLLu virtual ~HMDVolatileLayers();

    _HDLLu HMDVolatileLayers(const HMDVolatileLayers& pi_rObj);

    _HDLLu virtual HFCPtr<HMDMetaDataContainer> Clone() const;

    _HDLLu bool                                SameLayersOn        (const HMDVolatileLayers& pi_rObj) const;

    _HDLLu unsigned short                      GetNbVolatileLayers () const;
    _HDLLu HMDVolatileLayerInfo*                GetVolatileLayerInfo(unsigned short pi_Index);
    _HDLLu const HMDLayerInfo*                  GetLayerInfo        (unsigned short pi_Index) const;
    _HDLLu bool                                GetIndexFromKey     (const WString& pi_rKey,
                                                              unsigned short&       po_rIndex) const;

    _HDLLu void                                 ResetInitialVisibleState();

protected:

    HArrayAutoPtr<HAutoPtr<HMDVolatileLayerInfo> > m_ppVolatileLayers;
    unsigned short                                 m_NbVolatileLayers;

private:
    HMDVolatileLayers& operator=(const HMDVolatileLayers& pi_rObj);
    void CopyMemberData(const HMDVolatileLayers& pi_rObj);
    };

