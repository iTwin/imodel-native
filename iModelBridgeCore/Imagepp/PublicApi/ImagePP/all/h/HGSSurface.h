//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurface.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSSurface
//-----------------------------------------------------------------------------
// General class for surfaces.
//-----------------------------------------------------------------------------
#pragma once

#include "HGSSurfaceImplementation.h"
#include "HGSSurfaceCapabilities.h"
#include "HFCPtr.h"
#include "HGSSurfaceDescriptor.h"
#include "HGSSurfaceOption.h"


class HGSToolbox;

class HGSSurface : public HFCShareableObject<HGSSurface>
    {
    HDECLARE_BASECLASS_ID(1718)

public:

    // Primary methods
    _HDLLg                 HGSSurface(const HFCPtr<HGSSurfaceDescriptor>& pi_rpDescriptor,
                                      const HGSToolbox*                   pi_pToolbox = 0,
                                      const HGSSurfaceCapabilities*       pi_pCapabilities = 0);

    // DO NOT USE THIS CONSTRUCTOR, UNLESS IT'S ABSOLUTELY NECESSARY
    _HDLLg                HGSSurface(HGSSurfaceImplementation* pi_pImplementation);

    _HDLLg virtual         ~HGSSurface();

    // descriptor
    const HFCPtr<HGSSurfaceDescriptor>&
    GetSurfaceDescriptor() const;

    // Surface creation
    _HDLLg HGSSurface*     CreateCompatibleSurface(const HFCPtr<HGSSurfaceDescriptor>& pi_rpSurfaceDescriptor) const;

    // Options
    void            AddOption(const HFCPtr<HGSSurfaceOption>& pi_rpOption);
    void            RemoveOption(HCLASS_ID pi_OptionID);
    const HFCPtr<HGSSurfaceOption>
    GetOption(HCLASS_ID  pi_OptionID) const;
    void            SetOption(const HFCPtr<HGSSurfaceOption>& pi_rpOption);
    const HGSSurfaceOptions&
    GetOptions() const;


    // this method is provided only for HGSGraphicTool
    _HDLLg HGSSurfaceImplementation*
    GetImplementation() const;

    // Utilities
    _HDLLg void            RequestCompatibility(const HGSToolbox* pi_pToolbox = 0,
                                                const HGSSurfaceCapabilities* pi_pCapabilities = 0);

    // Update
    void            Update();

protected:

private:


    // private members
    mutable HAutoPtr<HGSSurfaceImplementation>
    m_pImplementation;

    mutable HAutoPtr<HGSSurfaceImplementation>
    m_pOriginalImplementation;


    // disabled methods
    HGSSurface();
    HGSSurface(const HGSSurface& pi_rObj);
    HGSSurface&     operator=(const HGSSurface& pi_rObj);
    };

#include "HGSSurface.hpp"

