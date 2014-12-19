//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDContext.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDMetaDataContainer.h"

class HMDContext : public HFCShareableObject<HMDContext>
    {
    HDECLARE_BASECLASS_ID(7012)

public :

    _HDLLu HMDContext();
    _HDLLu virtual                             ~HMDContext();

    _HDLLu void                                AddMetaDataContainer(HMDMetaDataContainer::Type    pi_Type,
                                                                    const HFCPtr<HMDMetaDataContainer>& pi_rContainer);

    _HDLLu HFCPtr<HMDMetaDataContainer> GetMetaDataContainer(HMDMetaDataContainer::Type    pi_Type);

private :

    HMDContext(const HMDContext& pi_rObj);
    HMDContext&                         operator=(const HMDContext& pi_rObj);

    typedef map<HMDMetaDataContainer::Type,
            HFCPtr<HMDMetaDataContainer> > ContainerMap;

    ContainerMap m_MetaDataContainerMap;
    };

