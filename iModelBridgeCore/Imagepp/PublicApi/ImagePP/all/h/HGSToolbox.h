//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSToolbox.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSToolbox
//-----------------------------------------------------------------------------
// Toolbox class
//-----------------------------------------------------------------------------
#pragma once

#include "HGSGraphicTool.h"
#include "HGSSurface.h"

class HGSToolbox
    {
    HDECLARE_BASECLASS_ID(1726)

public:

    // Primary methods
    _HDLLg                 HGSToolbox();
    _HDLLg                 HGSToolbox(const HFCPtr<HGSGraphicTool>& pi_rpGraphicTool);
    _HDLLg virtual         ~HGSToolbox();

    // Tool management
    uint32_t        CountTools() const;
    HFCPtr<HGSGraphicTool>
    GetTool(uint32_t pi_Index) const;
    void            Add(const HFCPtr<HGSGraphicTool>& pi_rpGraphicTool);

    // association of the toolbox to all the tools
    _HDLLg void            SetFor(const HFCPtr<HGSSurface>& pi_rpSurface);

protected:

private:

    typedef vector<HFCPtr<HGSGraphicTool>, allocator<HFCPtr<HGSGraphicTool> > >
    Tools;

    // List of graphic tools
    Tools           m_Tools;

    HFCPtr<HGSSurface>
    m_pSurface;

    // Private methods
    HGSToolbox(const HGSToolbox& pi_rObj);
    HGSToolbox&     operator=(const HGSToolbox& pi_rObj);
    bool             operator==(const HGSToolbox& pi_rObj) const;
    bool             operator!=(const HGSToolbox& pi_rObj);
    };

#include "HGSToolbox.hpp"

