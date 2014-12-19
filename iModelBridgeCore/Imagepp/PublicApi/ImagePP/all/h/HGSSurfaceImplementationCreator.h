//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurfaceImplementationCreator.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSSurfaceImplementationCreator
//-----------------------------------------------------------------------------
// General class for surfaces.
//-----------------------------------------------------------------------------
#pragma once

#include "HGSSurfaceDescriptor.h"
#include "HGSRegion.h"

class HGSSurfaceCapabilities;
class HGSSurfaceImplementation;

class HNOVTABLEINIT HGSSurfaceImplementationCreator
    {
    HDECLARE_BASECLASS_ID(1723)

public:

    // Primary methods
    _HDLLg virtual         ~HGSSurfaceImplementationCreator();

    // Capabilities
    virtual const HGSSurfaceCapabilities*
    GetCapabilities() const = 0;

    // Associated surface implementation
    virtual HCLASS_ID
    GetSurfaceImplementationID() const = 0;

    // Creation
    virtual HGSSurfaceImplementation*
    Create(const HFCPtr<HGSSurfaceDescriptor>&  pi_rpDescriptor) const = 0;

    uint32_t        GetPriority() const;
    _HDLLg void            SetPriority(uint32_t pi_Priority);

protected:

    // Primary methods
    _HDLLg                 HGSSurfaceImplementationCreator();

private:

    // private members
    uint32_t        m_Priority;

    // Disabled Methods
    HGSSurfaceImplementationCreator(
        const HGSSurfaceImplementationCreator& pi_rObj);
    HGSSurfaceImplementationCreator&
    operator=(  const HGSSurfaceImplementationCreator& pi_rObj);
    bool             operator==( const HGSSurfaceImplementationCreator& pi_rObj) const;
    bool             operator!=( const HGSSurfaceImplementationCreator& pi_rObj) const;
    };

