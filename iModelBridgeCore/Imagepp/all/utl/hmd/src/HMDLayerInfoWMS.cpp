//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hmd/src/HMDLayerInfoWMS.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HMDLayerInfoWMS
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HMDLayerInfoWMS.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDLayerInfoWMS::HMDLayerInfoWMS(const WString& pi_rKeyName,
                                 const WString& pi_rLayerName,
                                 const WString& pi_rLayerTitle,
                                 const WString& pi_rLayerAbstract,
                                 const double  pi_MinScaleHint,
                                 const double  pi_MaxScaleHint,
                                 const WString& pi_rStyleName,
                                 const WString& pi_rStyleTitle,
                                 const WString& pi_rStyleAbstract,
                                 bool          pi_Opaque)
    : HMDLayerInfo(pi_rKeyName, true),
      m_LayerName(pi_rLayerName),
      m_LayerTitle(pi_rLayerTitle),
      m_LayerAbstract(pi_rLayerAbstract),
      m_MinScaleHint(pi_MinScaleHint),
      m_MaxScaleHint(pi_MaxScaleHint),
      m_StyleName(pi_rStyleName),
      m_StyleTitle(pi_rStyleTitle),
      m_StyleAbstract(pi_rStyleAbstract),
      m_Opaque(pi_Opaque)
    {
    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HMDLayerInfoWMS::~HMDLayerInfoWMS()
    {
    }


//-----------------------------------------------------------------------------
// Public
// GetLayerName
//-----------------------------------------------------------------------------
const WString& HMDLayerInfoWMS::GetLayerName() const
    {
    return m_LayerName;
    }

//-----------------------------------------------------------------------------
// Public
// GetLayerTile
//-----------------------------------------------------------------------------
const WString& HMDLayerInfoWMS::GetLayerTitle() const
    {
    return m_LayerTitle;
    }

//-----------------------------------------------------------------------------
// Public
// GetLayerAbstract
//-----------------------------------------------------------------------------
const WString& HMDLayerInfoWMS::GetLayerAbstract() const
    {
    return m_LayerAbstract;
    }

//-----------------------------------------------------------------------------
// Public
// GetStyleName
//-----------------------------------------------------------------------------
const WString& HMDLayerInfoWMS::GetStyleName() const
    {
    return m_StyleName;
    }

//-----------------------------------------------------------------------------
// Public
// GetStyleTitle
//-----------------------------------------------------------------------------
const WString& HMDLayerInfoWMS::GetStyleTitle() const
    {
    return m_StyleTitle;
    }

//-----------------------------------------------------------------------------
// Public
// GetStyleAbstract
//-----------------------------------------------------------------------------
const WString& HMDLayerInfoWMS::GetStyleAbstract() const
    {
    return m_StyleAbstract;
    }

//-----------------------------------------------------------------------------
// Public
// GetMinScaleHint
//-----------------------------------------------------------------------------
double HMDLayerInfoWMS::GetMinScaleHint() const
    {
    return m_MinScaleHint;
    }

//-----------------------------------------------------------------------------
// Public
// GetMaxScaleHint
//-----------------------------------------------------------------------------
double HMDLayerInfoWMS::GetMaxScaleHint() const
    {
    return m_MaxScaleHint;
    }

//-----------------------------------------------------------------------------
// Public
// IsOpaque
//-----------------------------------------------------------------------------
bool HMDLayerInfoWMS::IsOpaque() const
    {
    return m_Opaque;
    }
