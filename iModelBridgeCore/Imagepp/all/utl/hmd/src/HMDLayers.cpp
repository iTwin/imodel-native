//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDLayers.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HMDLayers
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMDLayers.h>


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
const HMDLayerInfo* HMDLayers::GetLayer(unsigned short pi_Index) const
    {
    HPRECONDITION(pi_Index < m_layerInfoVec.size());

    return m_layerInfoVec[pi_Index];
    }

//-----------------------------------------------------------------------------
// Public
// Get the number of layers
//-----------------------------------------------------------------------------
unsigned short HMDLayers::GetNbLayers() const
    {
    return static_cast<unsigned short>(m_layerInfoVec.size());
    }

//-----------------------------------------------------------------------------
// Public
// GetIndexFromKey
//-----------------------------------------------------------------------------
bool HMDLayers::GetIndexFromKey(const WString& pi_rKey, unsigned short&  po_rIndex) const
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
    unsigned short InputNbLayers = pi_rObj.GetNbLayers();
    for (unsigned short LayerInd = 0; LayerInd < InputNbLayers; LayerInd++)
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
    for (unsigned short LayerInd = 0; LayerInd < pi_rLayers->GetNbLayers(); LayerInd++)
        {
        unsigned short ThisLayerIndex;

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