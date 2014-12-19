//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurfaceImplementationFactory.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSSurfaceImplementationFactory
//-----------------------------------------------------------------------------
// Pixel type definition.  It is composed of a set of channel descriptions
// with a palette definition.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HGSSurfaceImplementationCreator.h"
#include "HGSToolbox.h"

class HGSSurfaceImplementationFactory
    {
    HDECLARE_BASECLASS_ID(1724)

    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HGSSurfaceImplementationFactory)

public:

    // Primary methods
    HGSSurfaceImplementationFactory();
    virtual         ~HGSSurfaceImplementationFactory();

    // Registration
    _HDLLg void            Register(const HGSSurfaceImplementationCreator* pi_pCreator);

    // Creation
    HGSSurfaceImplementation*
    Create( const HFCPtr<HGSSurfaceDescriptor>& pi_rpDescriptor,
            const HGSToolbox*                   pi_pToolbox,
            const HGSSurfaceCapabilities*       pi_pCapabilities ) const;

protected:

private:

    // list of creators
    typedef list<const HGSSurfaceImplementationCreator*, allocator<const HGSSurfaceImplementationCreator*> >
    Creators;

    // Private members
    Creators        m_Creators;

    // disabled methods
    HGSSurfaceImplementationFactory(const HGSSurfaceImplementationFactory& pi_rObj);
    HGSSurfaceImplementationFactory&
    operator=(const HGSSurfaceImplementationFactory& pi_rObj);
    };
