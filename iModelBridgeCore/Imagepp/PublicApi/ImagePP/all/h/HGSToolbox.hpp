//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSToolbox.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSToolbox
//-----------------------------------------------------------------------------
// General class for surfaces.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// public
// Add
//-----------------------------------------------------------------------------
inline void HGSToolbox::Add(const HFCPtr<HGSGraphicTool>& pi_rpGraphicTool)
    {
    // set the surface used by the toolbox
    pi_rpGraphicTool->SetFor(m_pSurface);

    // add the tool in the list
    m_Tools.push_back(pi_rpGraphicTool);
    }


//-----------------------------------------------------------------------------
// public
// CountTools
//-----------------------------------------------------------------------------
inline uint32_t HGSToolbox::CountTools() const
    {
    return (uint32_t)m_Tools.size();
    }

//-----------------------------------------------------------------------------
// public
// GetTool
//-----------------------------------------------------------------------------
inline HFCPtr<HGSGraphicTool> HGSToolbox::GetTool(uint32_t pi_Index) const
    {
    return m_Tools[pi_Index];
    }

//-----------------------------------------------------------------------------
// public
// SetFor
//-----------------------------------------------------------------------------
inline void HGSToolbox::SetFor(const HFCPtr<HGSSurface>& pi_rpSurface)
    {
    // parse all the list of tools and call SetFor on each tool
    HGSToolbox::Tools::iterator Itr;
    for (Itr = m_Tools.begin(); Itr != m_Tools.end(); Itr++)
        (*Itr)->SetFor(pi_rpSurface);

    // keep a pointer on the surface in the case we add new tools
    m_pSurface = pi_rpSurface;
    }
