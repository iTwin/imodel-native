//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HMDMetaDataContainerList.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HMDMetaDataContainer.h"

BEGIN_IMAGEPP_NAMESPACE
class HMDMetaDataContainerList : public HFCShareableObject<HMDMetaDataContainerList>
    {
    HDECLARE_BASECLASS_ID(HMDMetaDataId_ContainerList);

public :

    IMAGEPP_EXPORT HMDMetaDataContainerList();
    IMAGEPP_EXPORT virtual ~HMDMetaDataContainerList();

    IMAGEPP_EXPORT HMDMetaDataContainerList(const HMDMetaDataContainerList& pi_rObj);
    IMAGEPP_EXPORT HMDMetaDataContainerList& operator=(const HMDMetaDataContainerList& pi_rObj);


    IMAGEPP_EXPORT unsigned short                     GetNbContainers      () const;
    IMAGEPP_EXPORT void                               GetMetaDataContainer (uint32_t                      pi_MDContainerInd,
                                                                    HFCPtr<HMDMetaDataContainer>& po_rpMDContainer) const;
    IMAGEPP_EXPORT const HFCPtr<HMDMetaDataContainer> GetMetaDataContainer (HMDMetaDataContainer::Type    pi_ContainerType) const;
    IMAGEPP_EXPORT void                               SetMetaDataContainer (const HFCPtr<HMDMetaDataContainer>& pi_rpMDContainer);
    IMAGEPP_EXPORT void                               SetModificationStatus(bool pi_HasChanged);

private :

    IMAGEPP_EXPORT void                               CopyMemberData(const HMDMetaDataContainerList& pi_rObj);

    typedef vector<HFCPtr<HMDMetaDataContainer> > ListOfMetaDataContainer;
    ListOfMetaDataContainer m_MetaDataContainers;
    };

END_IMAGEPP_NAMESPACE