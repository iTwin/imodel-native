//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDMetaDataContainerList.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDMetaDataContainer.h"

class HMDMetaDataContainerList : public HFCShareableObject<HMDMetaDataContainerList>
    {
    HDECLARE_BASECLASS_ID(7014);

public :

    _HDLLu HMDMetaDataContainerList();
    _HDLLu virtual ~HMDMetaDataContainerList();

    _HDLLu HMDMetaDataContainerList(const HMDMetaDataContainerList& pi_rObj);
    _HDLLu HMDMetaDataContainerList& operator=(const HMDMetaDataContainerList& pi_rObj);


    _HDLLu unsigned short                     GetNbContainers      () const;
    _HDLLu void                               GetMetaDataContainer (uint32_t                      pi_MDContainerInd,
                                                                    HFCPtr<HMDMetaDataContainer>& po_rpMDContainer) const;
    _HDLLu const HFCPtr<HMDMetaDataContainer> GetMetaDataContainer (HMDMetaDataContainer::Type    pi_ContainerType) const;
    _HDLLu void                               SetMetaDataContainer (const HFCPtr<HMDMetaDataContainer>& pi_rpMDContainer);
    _HDLLu void                               SetModificationStatus(bool pi_HasChanged);

private :

    _HDLLu void                               CopyMemberData(const HMDMetaDataContainerList& pi_rObj);

    typedef vector<HFCPtr<HMDMetaDataContainer> > ListOfMetaDataContainer;
    ListOfMetaDataContainer m_MetaDataContainers;
    };

