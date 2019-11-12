//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HMDLayerInfoWMS
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HMDLayerInfoWMS.h>

//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HMDLayerInfoWMS::HMDLayerInfoWMS(const Utf8String& pi_rKeyName,
                                 const Utf8String& pi_rLayerName,
                                 const Utf8String& pi_rLayerTitle,
                                 const Utf8String& pi_rLayerAbstract,
                                 const double  pi_MinScaleHint,
                                 const double  pi_MaxScaleHint,
                                 const Utf8String& pi_rStyleName,
                                 const Utf8String& pi_rStyleTitle,
                                 const Utf8String& pi_rStyleAbstract,
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
const Utf8String& HMDLayerInfoWMS::GetLayerName() const
    {
    return m_LayerName;
    }

//-----------------------------------------------------------------------------
// Public
// GetLayerTile
//-----------------------------------------------------------------------------
const Utf8String& HMDLayerInfoWMS::GetLayerTitle() const
    {
    return m_LayerTitle;
    }

//-----------------------------------------------------------------------------
// Public
// GetLayerAbstract
//-----------------------------------------------------------------------------
const Utf8String& HMDLayerInfoWMS::GetLayerAbstract() const
    {
    return m_LayerAbstract;
    }

//-----------------------------------------------------------------------------
// Public
// GetStyleName
//-----------------------------------------------------------------------------
const Utf8String& HMDLayerInfoWMS::GetStyleName() const
    {
    return m_StyleName;
    }

//-----------------------------------------------------------------------------
// Public
// GetStyleTitle
//-----------------------------------------------------------------------------
const Utf8String& HMDLayerInfoWMS::GetStyleTitle() const
    {
    return m_StyleTitle;
    }

//-----------------------------------------------------------------------------
// Public
// GetStyleAbstract
//-----------------------------------------------------------------------------
const Utf8String& HMDLayerInfoWMS::GetStyleAbstract() const
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
