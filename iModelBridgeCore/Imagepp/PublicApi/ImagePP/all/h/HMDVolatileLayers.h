//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDVolatileLayers.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDMetaDataContainer.h"
#include <Imagepp/all/h/HMDVolatileLayerInfo.h>

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

    IMAGEPP_EXPORT virtual HFCPtr<HMDMetaDataContainer> Clone() const;

    IMAGEPP_EXPORT bool                                SameLayersOn        (const HMDVolatileLayers& pi_rObj) const;

    IMAGEPP_EXPORT unsigned short                      GetNbVolatileLayers () const;
    IMAGEPP_EXPORT HMDVolatileLayerInfo*                GetVolatileLayerInfo(unsigned short pi_Index);
    IMAGEPP_EXPORT const HMDLayerInfo*                  GetLayerInfo        (unsigned short pi_Index) const;
    IMAGEPP_EXPORT bool                                GetIndexFromKey     (const WString& pi_rKey,
                                                              unsigned short&       po_rIndex) const;

    IMAGEPP_EXPORT void                                 ResetInitialVisibleState();

protected:

    HArrayAutoPtr<HAutoPtr<HMDVolatileLayerInfo> > m_ppVolatileLayers;
    unsigned short                                 m_NbVolatileLayers;

private:
    HMDVolatileLayers& operator=(const HMDVolatileLayers& pi_rObj);
    void CopyMemberData(const HMDVolatileLayers& pi_rObj);
    };

END_IMAGEPP_NAMESPACE