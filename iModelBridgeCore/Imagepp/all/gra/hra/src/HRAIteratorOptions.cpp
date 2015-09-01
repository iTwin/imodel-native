//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAIteratorOptions.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRAIteratorOptions
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAIteratorOptions.h>
#include <Imagepp/all/h/HRARaster.h>
#include <Imagepp/all/h/HVEShape.h>


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRAIteratorOptions::HRAIteratorOptions()
    {
    m_ClipUsingEffectiveShape = true;

    m_MembersArePrivate = false;

    m_MaxResolutionStretchingFactor = 0;
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRAIteratorOptions::HRAIteratorOptions(const HRAIteratorOptions& pi_rOptions)
    : m_pPhysicalCoordSys(pi_rOptions.m_pPhysicalCoordSys),
      m_pRegionToProcess(pi_rOptions.m_pRegionToProcess)
    {
    m_ClipUsingEffectiveShape = pi_rOptions.m_ClipUsingEffectiveShape;

    m_MaxResolutionStretchingFactor = pi_rOptions.m_MaxResolutionStretchingFactor;

    m_MembersArePrivate = false;
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRAIteratorOptions::HRAIteratorOptions(const HFCPtr<HGF2DCoordSys>& pi_rpPhysicalCoordSys)
    : m_pPhysicalCoordSys(pi_rpPhysicalCoordSys)
    {
    m_ClipUsingEffectiveShape = true;

    m_MembersArePrivate = false;

    m_MaxResolutionStretchingFactor = 0;
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRAIteratorOptions::HRAIteratorOptions(const HFCPtr<HVEShape>&      pi_rpRegionToProcess,
                                       const HFCPtr<HGF2DCoordSys>& pi_rpPhysicalCoordSys,
                                       bool                        pi_ClipUsingEffectiveShape)
    : m_pPhysicalCoordSys(pi_rpPhysicalCoordSys),
      m_pRegionToProcess(pi_rpRegionToProcess)
    {
    m_ClipUsingEffectiveShape = pi_ClipUsingEffectiveShape;

    m_MembersArePrivate = false;

    m_MaxResolutionStretchingFactor = 0;
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRAIteratorOptions::HRAIteratorOptions(const HFCPtr<HVEShape>& pi_rpRegionToProcess,
                                       bool                   pi_ClipUsingEffectiveShape)
    : m_pRegionToProcess(pi_rpRegionToProcess)
    {
    m_ClipUsingEffectiveShape = pi_ClipUsingEffectiveShape;

    m_MembersArePrivate = false;

    m_MaxResolutionStretchingFactor = 0;
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRAIteratorOptions::~HRAIteratorOptions()
    {
    }


//-----------------------------------------------------------------------------
// Assignment
//-----------------------------------------------------------------------------
HRAIteratorOptions& HRAIteratorOptions::operator=(const HRAIteratorOptions& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_pPhysicalCoordSys             = pi_rObj.m_pPhysicalCoordSys;
        m_ClipUsingEffectiveShape       = pi_rObj.m_ClipUsingEffectiveShape;
        m_pRegionToProcess              = pi_rObj.m_pRegionToProcess;
        m_MaxResolutionStretchingFactor = pi_rObj.m_MaxResolutionStretchingFactor;

        m_MembersArePrivate = false;
        }

    return *this;
    }


//-----------------------------------------------------------------------------
// Make the options keep a personal copy of some attributes.
//-----------------------------------------------------------------------------
void HRAIteratorOptions::CreatePrivateCopies()
    {
    if (!m_MembersArePrivate)
        {
        // Only the shape needs to be copied
        if (m_pRegionToProcess != 0)
            m_pRegionToProcess = new HVEShape(*m_pRegionToProcess);

        m_MembersArePrivate = true;
        }
    }


//-----------------------------------------------------------------------------
// Clip using this raster's effective shape if necessary
//-----------------------------------------------------------------------------
void HRAIteratorOptions::ClipRegionToRaster(const HFCPtr<HRARaster>& pi_rpRaster)
    {
    CreatePrivateCopies();

    if (m_pRegionToProcess != 0)
        {
        // We already have a region. Intersect it if necessary
        if (m_ClipUsingEffectiveShape)
            m_pRegionToProcess->Intersect(*pi_rpRaster->GetEffectiveShape());
        }
    else
        {
        // Set a region
        m_pRegionToProcess = new HVEShape(*pi_rpRaster->GetEffectiveShape());
        }
    }


//-----------------------------------------------------------------------------
// Simply obtain the resulting (clipped) region, without
// modifying the internal region.
//-----------------------------------------------------------------------------
HFCPtr<HVEShape> HRAIteratorOptions::CalculateClippedRegion(const HFCPtr<HRARaster>& pi_rpRaster) const
    {
    HFCPtr<HVEShape> pClippedRegion;
    if (m_pRegionToProcess != 0)
        {
        pClippedRegion = new HVEShape(*m_pRegionToProcess);

        // We already have a region. Intersect it if necessary
        if (m_ClipUsingEffectiveShape)
            pClippedRegion->Intersect(*pi_rpRaster->GetEffectiveShape());
        }
    else
        {
        // Set a region
        pClippedRegion = new HVEShape(*pi_rpRaster->GetEffectiveShape());
        }

    return pClippedRegion;
    }

//-----------------------------------------------------------------------------
// GetRegionToProcess
//-----------------------------------------------------------------------------
const HFCPtr<HVEShape>& HRAIteratorOptions::GetRegionToProcess() const
    {
    return m_pRegionToProcess;
    }


//-----------------------------------------------------------------------------
// SetRegionToProcess
//-----------------------------------------------------------------------------
void HRAIteratorOptions::SetRegionToProcess(HFCPtr<HVEShape>& pi_rpRegionToProcess)
    {
    m_pRegionToProcess = pi_rpRegionToProcess;
    }


//-----------------------------------------------------------------------------
// GetPhysicalCoordSys
//-----------------------------------------------------------------------------
const HFCPtr<HGF2DCoordSys>& HRAIteratorOptions::GetPhysicalCoordSys() const
    {
    return m_pPhysicalCoordSys;
    }


//-----------------------------------------------------------------------------
// SetPhysicalCoordSys
//-----------------------------------------------------------------------------
void HRAIteratorOptions::SetPhysicalCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpPhysicalCoordSys)
    {
    m_pPhysicalCoordSys = pi_rpPhysicalCoordSys;
    }


//-----------------------------------------------------------------------------
// MustClipUsingEffectiveShape
//-----------------------------------------------------------------------------
bool HRAIteratorOptions::MustClipUsingEffectiveShape() const
    {
    return m_ClipUsingEffectiveShape;
    }


//-----------------------------------------------------------------------------
// ClipUsingEffectiveShape
//-----------------------------------------------------------------------------
void HRAIteratorOptions::ClipUsingEffectiveShape(bool pi_Clip)
    {
    m_ClipUsingEffectiveShape = pi_Clip;
    }


//-----------------------------------------------------------------------------
// MustClipUsingEffectiveShape
//-----------------------------------------------------------------------------
bool HRAIteratorOptions::IsShaped() const
    {
    return (m_pRegionToProcess != 0);
    }


//-----------------------------------------------------------------------------
// public
// MaxResolutionStretchingFactor
//-----------------------------------------------------------------------------
uint8_t HRAIteratorOptions::MaxResolutionStretchingFactor() const
    {
    return m_MaxResolutionStretchingFactor;
    }


//-----------------------------------------------------------------------------
// public
// SetMaxResolutionStretchingFactor
//-----------------------------------------------------------------------------
void HRAIteratorOptions::SetMaxResolutionStretchingFactor(uint8_t pi_Factor)
    {
    // Represents a percentage.
    HASSERT(pi_Factor <= 100);

    m_MaxResolutionStretchingFactor = pi_Factor;
    }
