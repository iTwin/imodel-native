//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDLayers.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HMDLayers
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HMDLayers.h>


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDLayers::HMDLayers()
    : HMDMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO)
    {
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDLayers::~HMDLayers()
    {
    for (LayerInfoVec::iterator Itr = m_layerInfoVec.begin(); Itr != m_layerInfoVec.end(); Itr++)
        {
        const HMDLayerInfo* pInfo = *Itr;
        if (pInfo!=NULL)
            delete (pInfo);
        *Itr = NULL;
        }
    }

//-----------------------------------------------------------------------------
//  Clone
//  This method dynamically allocates a copy of this
//-----------------------------------------------------------------------------
HFCPtr<HMDMetaDataContainer> HMDLayers::Clone() const
    {
    return HFCPtr<HMDMetaDataContainer>(new HMDLayers(*this));
    }

//-----------------------------------------------------------------------------
// Public
// Default constructor
//-----------------------------------------------------------------------------
HMDLayers::HMDLayers(const HMDLayers& pi_rObj)
    : HMDMetaDataContainer(pi_rObj)
    {
    CopyMemberData(pi_rObj);
    }

//-----------------------------------------------------------------------------
// Public
// Add a layer
//-----------------------------------------------------------------------------
void HMDLayers::AddLayer(const HMDLayerInfo* pi_pLayer)
    {
    m_layerInfoVec.push_back(pi_pLayer);
    }

//-----------------------------------------------------------------------------
// Public
// Get a layer
//-----------------------------------------------------------------------------
const HMDLayerInfo* HMDLayers::GetLayer(uint16_t pi_Index) const
    {
    HPRECONDITION(pi_Index < m_layerInfoVec.size());

    return m_layerInfoVec[pi_Index];
    }

//-----------------------------------------------------------------------------
// Public
// Get the number of layers
//-----------------------------------------------------------------------------
uint16_t HMDLayers::GetNbLayers() const
    {
    return static_cast<uint16_t>(m_layerInfoVec.size());
    }

//-----------------------------------------------------------------------------
// Public
// GetIndexFromKey
//-----------------------------------------------------------------------------
bool HMDLayers::GetIndexFromKey(const Utf8String& pi_rKey, uint16_t&  po_rIndex) const
    {
    po_rIndex = 0;
    for (LayerInfoVec::const_iterator Itr = m_layerInfoVec.begin(); Itr != m_layerInfoVec.end(); Itr++)
        {
        if ((*Itr)->GetKeyName() == pi_rKey)
            {
            //Key Found
            return true;
            }
        po_rIndex++;
        }

    return false;
    }

//-----------------------------------------------------------------------------
// Private
// CopyMemberData
//-----------------------------------------------------------------------------
void HMDLayers::CopyMemberData(const HMDLayers& pi_rObj)
    {
    uint16_t InputNbLayers = pi_rObj.GetNbLayers();
    for (uint16_t LayerInd = 0; LayerInd < InputNbLayers; LayerInd++)
        {
        m_layerInfoVec.push_back(new HMDLayerInfo(*(pi_rObj.GetLayer(LayerInd))));
        }
    }

//-----------------------------------------------------------------------------
// Public
// Merge the layers of the HMDLayers object passed in parameter with the layers
// currently in the object
//-----------------------------------------------------------------------------
void HMDLayers::Merge(const HFCPtr<HMDLayers>& pi_rLayers)
    {
    for (uint16_t LayerInd = 0; LayerInd < pi_rLayers->GetNbLayers(); LayerInd++)
        {
        uint16_t ThisLayerIndex;

        if (GetIndexFromKey(pi_rLayers->GetLayer(LayerInd)->GetKeyName(), ThisLayerIndex))
            {
            ((HMDLayerInfo*)(m_layerInfoVec[ThisLayerIndex]))->SetInitialVisibleState(pi_rLayers
                                                                                        ->GetLayer(LayerInd)
                                                                                        ->GetInitialVisibleState());
            }
        else
            {
            m_layerInfoVec.push_back(new HMDLayerInfo(*(pi_rLayers->GetLayer(LayerInd))));
            }
        }
    }
