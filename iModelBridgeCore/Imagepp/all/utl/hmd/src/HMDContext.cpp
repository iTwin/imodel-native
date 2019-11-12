//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HMDContext.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDContext::HMDContext()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDContext::~HMDContext()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Add metadata container
//-----------------------------------------------------------------------------
void HMDContext::AddMetaDataContainer(HMDMetaDataContainer::Type    pi_Type,
                                      const HFCPtr<HMDMetaDataContainer>& pi_rContainer)
    {
    m_MetaDataContainerMap.insert(ContainerMap::value_type(pi_Type,
        pi_rContainer));
    }
//-----------------------------------------------------------------------------
// Public
// Get metadata container
//-----------------------------------------------------------------------------
HFCPtr<HMDMetaDataContainer> HMDContext::GetMetaDataContainer(HMDMetaDataContainer::Type pi_Type)
    {
    HFCPtr<HMDMetaDataContainer> pContainer;
    ContainerMap::iterator       ContainerIter = m_MetaDataContainerMap.find(pi_Type);

    if (ContainerIter != m_MetaDataContainerMap.end())
        {
        pContainer = (*ContainerIter).second;
        }

    return pContainer;
    }
