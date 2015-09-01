//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDMetaDataContainerList.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMDMetaDataContainerList.h>


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDMetaDataContainerList::HMDMetaDataContainerList()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDMetaDataContainerList::~HMDMetaDataContainerList()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Default constructor
//-----------------------------------------------------------------------------
HMDMetaDataContainerList::HMDMetaDataContainerList(const HMDMetaDataContainerList& pi_rObj)
    {
    CopyMemberData(pi_rObj);
    }

//-----------------------------------------------------------------------------
// Public
// Assignment operator
//-----------------------------------------------------------------------------
HMDMetaDataContainerList& HMDMetaDataContainerList::operator=(const HMDMetaDataContainerList& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        CopyMemberData(pi_rObj);
        }

    return *this;
    }

//-----------------------------------------------------------------------------
// Public
// Get the number of metadata containers
//-----------------------------------------------------------------------------
unsigned short HMDMetaDataContainerList::GetNbContainers() const
    {
    return (unsigned short)m_MetaDataContainers.size();
    }

//-----------------------------------------------------------------------------
// Public
// Get a metadata container at a given position in the metadata container list
//-----------------------------------------------------------------------------
void HMDMetaDataContainerList::GetMetaDataContainer(uint32_t                      pi_MDContainerInd,
                                                    HFCPtr<HMDMetaDataContainer>& po_rpMDContainer) const
    {
    HPRECONDITION(pi_MDContainerInd < m_MetaDataContainers.size());

    po_rpMDContainer = m_MetaDataContainers[pi_MDContainerInd];
    }

//-----------------------------------------------------------------------------
// Public
// SetMetaDataContainer
// Add the metadata container to the page descriptor
//-----------------------------------------------------------------------------
void HMDMetaDataContainerList::SetMetaDataContainer(const HFCPtr<HMDMetaDataContainer>& pi_rpMDContainer)
    {
    HMDMetaDataContainer::Type ContainerType = pi_rpMDContainer->GetType();

    ListOfMetaDataContainer::iterator ContainerIter(m_MetaDataContainers.begin());
    ListOfMetaDataContainer::iterator ContainerIterEnd(m_MetaDataContainers.end());

    while (ContainerIter != ContainerIterEnd)
        {
        if (ContainerType == (*ContainerIter)->GetType())
            {
            break;
            }
        ContainerIter++;
        }

    if (ContainerIter != ContainerIterEnd)
        {
        *ContainerIter = pi_rpMDContainer;
        }
    else
        {
        m_MetaDataContainers.push_back(pi_rpMDContainer);
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetMetadataContainer
// Get the metadata container of the specified type
//-----------------------------------------------------------------------------
const HFCPtr<HMDMetaDataContainer> HMDMetaDataContainerList::GetMetaDataContainer(HMDMetaDataContainer::Type pi_ContainerType) const
    {
    HFCPtr<HMDMetaDataContainer> pContainerFound;

    ListOfMetaDataContainer::const_iterator ContainerIter(m_MetaDataContainers.begin());
    ListOfMetaDataContainer::const_iterator ContainerIterEnd(m_MetaDataContainers.end());

    while (ContainerIter != ContainerIterEnd)
        {
        if (pi_ContainerType == (*ContainerIter)->GetType())
            {
            pContainerFound = *ContainerIter;
            break;
            }
        ContainerIter++;
        }

    return pContainerFound;
    }

//-----------------------------------------------------------------------------
// Public
// SetModificationStatus
// Set the modification status of all containers in the list.
//-----------------------------------------------------------------------------
void HMDMetaDataContainerList::SetModificationStatus(bool pi_HasChanged)
    {
    ListOfMetaDataContainer::const_iterator ContainerIter(m_MetaDataContainers.begin());
    ListOfMetaDataContainer::const_iterator ContainerIterEnd(m_MetaDataContainers.end());

    while (ContainerIter != ContainerIterEnd)
        {
        (*ContainerIter)->SetModificationStatus(pi_HasChanged);
        ContainerIter++;
        }
    }

//-----------------------------------------------------------------------------
// Private
// CopyMemberData
//-----------------------------------------------------------------------------
void HMDMetaDataContainerList::CopyMemberData(const HMDMetaDataContainerList& pi_rObj)
    {
    ListOfMetaDataContainer::const_iterator ContainerIter = pi_rObj.m_MetaDataContainers.begin();
    ListOfMetaDataContainer::const_iterator ContainerIterEnd = pi_rObj.m_MetaDataContainers.end();

    while (ContainerIter != ContainerIterEnd)
        {
        m_MetaDataContainers.push_back((*ContainerIter)->Clone());
        ContainerIter++;
        }
    }