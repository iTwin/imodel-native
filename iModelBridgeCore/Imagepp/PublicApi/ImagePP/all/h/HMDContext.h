//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDContext.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDMetaDataContainer.h"

BEGIN_IMAGEPP_NAMESPACE

class HMDContext : public HFCShareableObject<HMDContext>
    {
    HDECLARE_BASECLASS_ID(HMDContextId_Base)

public :

    IMAGEPP_EXPORT HMDContext();
    IMAGEPP_EXPORT virtual                             ~HMDContext();

    IMAGEPP_EXPORT void                                AddMetaDataContainer(HMDMetaDataContainer::Type    pi_Type,
                                                                    const HFCPtr<HMDMetaDataContainer>& pi_rContainer);

    IMAGEPP_EXPORT HFCPtr<HMDMetaDataContainer> GetMetaDataContainer(HMDMetaDataContainer::Type    pi_Type);

private :

    HMDContext(const HMDContext& pi_rObj);
    HMDContext&                         operator=(const HMDContext& pi_rObj);

    typedef map<HMDMetaDataContainer::Type,
            HFCPtr<HMDMetaDataContainer> > ContainerMap;

    ContainerMap m_MetaDataContainerMap;
    };

END_IMAGEPP_NAMESPACE