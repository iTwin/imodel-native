//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImageppInternal.h>


#include <ImagePP/all/h/HMDLayers.h>
#include <ImagePP/all/h/HMDVolatileLayers.h>
#include <ImagePP/all/h/HMDVolatileLayerInfo.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDVolatileLayers::HMDVolatileLayers(const HFCPtr<HMDLayers>& pi_rpLayers)
    : HMDMetaDataContainer(HMDMetaDataContainer::HMD_LAYER_INFO)
    {
    m_NbVolatileLayers = pi_rpLayers->GetNbLayers();
    m_ppVolatileLayers = new HAutoPtr<HMDVolatileLayerInfo>[m_NbVolatileLayers];

    for (uint16_t LayerInd = 0; LayerInd < m_NbVolatileLayers; LayerInd++)
        {
        m_ppVolatileLayers[LayerInd] = new HMDVolatileLayerInfo(pi_rpLayers->GetLayer(LayerInd));
        }
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDVolatileLayers::~HMDVolatileLayers()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Default constructor
//-----------------------------------------------------------------------------
HMDVolatileLayers::HMDVolatileLayers(const HMDVolatileLayers& pi_rObj)
    : HMDMetaDataContainer(pi_rObj)
    {
    CopyMemberData(pi_rObj);
    }

//-----------------------------------------------------------------------------
//  Clone
//  This method dynamically allocates a copy of this
//-----------------------------------------------------------------------------
HFCPtr<HMDMetaDataContainer> HMDVolatileLayers::Clone() const
    {
    return HFCPtr<HMDMetaDataContainer>(new HMDVolatileLayers(*this));
    }

//-----------------------------------------------------------------------------
// Public
// Get the number of volatile layers
//-----------------------------------------------------------------------------
uint16_t HMDVolatileLayers::GetNbVolatileLayers() const
    {
    return m_NbVolatileLayers;
    }

//-----------------------------------------------------------------------------
// Public
// Get a volatile layer info
//-----------------------------------------------------------------------------
HMDVolatileLayerInfo* HMDVolatileLayers::GetVolatileLayerInfo(uint16_t pi_Index)
    {
    HPRECONDITION(pi_Index < m_NbVolatileLayers);

    return m_ppVolatileLayers[pi_Index].get();
    }

//-----------------------------------------------------------------------------
// Public
// Get the persistent layer info
//-----------------------------------------------------------------------------
const HMDLayerInfo* HMDVolatileLayers::GetLayerInfo(uint16_t pi_Index) const
    {
    HPRECONDITION(pi_Index < m_NbVolatileLayers);

    return m_ppVolatileLayers[pi_Index]->GetLayerInfo();
    }

//-----------------------------------------------------------------------------
// Public
// GetIndexFromKey
//-----------------------------------------------------------------------------
bool HMDVolatileLayers::GetIndexFromKey(const Utf8String& pi_rKey,
                                                uint16_t&       po_rIndex) const
    {
    for (po_rIndex = 0; po_rIndex < m_NbVolatileLayers; po_rIndex++)
        {
        if (m_ppVolatileLayers[po_rIndex]->GetLayerInfo()->GetKeyName() == pi_rKey)
            {
            return true;
            }
        }

    return false;
    }

//-----------------------------------------------------------------------------
// Public
// SameLayersOn
//-----------------------------------------------------------------------------
bool HMDVolatileLayers::SameLayersOn(const HMDVolatileLayers& pi_rObj) const
    {
    //The number of layers should be the same
    HPRECONDITION(GetNbVolatileLayers() == pi_rObj.GetNbVolatileLayers());

    bool Ret = true;

    if (this != &pi_rObj)
        {
        for (uint16_t LayerInd = 0; LayerInd < GetNbVolatileLayers(); LayerInd++)
            {
            if ((const_cast<HMDVolatileLayers*>(this))->GetVolatileLayerInfo(LayerInd)->GetVisibleState() !=
                (const_cast<HMDVolatileLayers*>(&pi_rObj))->GetVolatileLayerInfo(LayerInd)->GetVisibleState())
                {
                Ret = false;
                break;
                }
            }
        }

    return Ret;
    }

//-----------------------------------------------------------------------------
// Public
// SameLayersOn
//-----------------------------------------------------------------------------
void HMDVolatileLayers::ResetInitialVisibleState()
    {
    bool InitialVisibleState;

    for (uint16_t LayerInd = 0; LayerInd < GetNbVolatileLayers(); LayerInd++)
        {
        InitialVisibleState = m_ppVolatileLayers[LayerInd]->GetLayerInfo()
                              ->GetInitialVisibleState();

        m_ppVolatileLayers[LayerInd]->SetVisibleState(InitialVisibleState);
        }
    }

//-----------------------------------------------------------------------------
// Private
// CopyMemberData
//-----------------------------------------------------------------------------
void HMDVolatileLayers::CopyMemberData(const HMDVolatileLayers& pi_rObj)
    {
    m_NbVolatileLayers = pi_rObj.GetNbVolatileLayers();
    m_ppVolatileLayers = new HAutoPtr<HMDVolatileLayerInfo>[m_NbVolatileLayers];

    HMDVolatileLayers* pVolatileLayers = const_cast<HMDVolatileLayers*>(&pi_rObj);

    for (uint16_t LayerInd = 0; LayerInd < m_NbVolatileLayers; LayerInd++)
        {
        m_ppVolatileLayers[LayerInd] = new HMDVolatileLayerInfo(*(pVolatileLayers->
                                                                  GetVolatileLayerInfo(LayerInd)));
        }
    }
