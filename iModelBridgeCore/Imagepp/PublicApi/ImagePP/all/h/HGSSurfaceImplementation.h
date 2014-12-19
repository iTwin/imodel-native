//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurfaceImplementation.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HGSSurfaceImplementation
//:>-----------------------------------------------------------------------------
#pragma once

#include "HGSSurfaceImplementationCreator.h"
#include "HFCMacros.h"
#include "HGSSurfaceDescriptor.h"
#include "HGSSurfaceCapabilities.h"
#include "HGSSurfaceOption.h"

class HRPPixelPalette;


class HGSSurfaceImplementation
    {
    HDECLARE_BASECLASS_ID(1722)

public:

    // Primary methods
    _HDLLg virtual                 ~HGSSurfaceImplementation();


    void                    AddOption(const HFCPtr<HGSSurfaceOption>&
                                      pi_rpOption);
    void                    RemoveOption(HCLASS_ID          pi_OptionID);
    const HFCPtr<HGSSurfaceOption>
    GetOption(HCLASS_ID             pi_OptionID) const;
    void                    SetOption(const HFCPtr<HGSSurfaceOption>&
                                      pi_rpOption);
    const HGSSurfaceOptions&
    GetOptions() const;

    // Implementations creation
    virtual const HGSSurfaceImplementationCreator*
    GetCreator() const = 0;


    // description of the surface
    const HFCPtr<HGSSurfaceDescriptor>&
    GetSurfaceDescriptor() const;


    // misceallous for surfaces having palette (like screen in Windows)
    virtual bool           HasPhysicalPalette() const;
    virtual void            SetPhysicalPalette(const HRPPixelPalette& pi_rpPalette);

protected:

    // Primary methos
    _HDLLg                     HGSSurfaceImplementation();
    _HDLLg                     HGSSurfaceImplementation(const HFCPtr<HGSSurfaceDescriptor>&    pi_rpDescriptor);

    void                    SetSurfaceDescriptor(const HFCPtr<HGSSurfaceDescriptor>& pi_rpDescriptor);


private:

    // private members
    HFCPtr<HGSSurfaceDescriptor>    m_pSurfaceDescriptor;

    HGSSurfaceOptions               m_Options;


    // disabled methods
    HGSSurfaceImplementation(const HGSSurfaceImplementation& pi_rObj);
    HGSSurfaceImplementation&
    operator=(const HGSSurfaceImplementation& pi_rObj);
    };

#include "HGSSurfaceImplementation.hpp"

