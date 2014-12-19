//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicToolImplementationFactory.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSGraphicToolImplementationFactory
//-----------------------------------------------------------------------------
// Pixel type definition.  It is composed of a set of channel descriptions
// with a palette definition.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"

class HGSGraphicToolAttributes;
class HGSGraphicToolImplementationCreator;
class HGSGraphicToolImplementation;
class HGSSurfaceImplementation;
class HGSGraphicToolCapabilities;
class HGSToolbox;

class HGSGraphicToolImplementationFactory
    {
    HDECLARE_BASECLASS_ID(1710)

    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HGSGraphicToolImplementationFactory)

public:

    // Primary methods
    HGSGraphicToolImplementationFactory();
    virtual         ~HGSGraphicToolImplementationFactory();

    // Registration
    void            Register(const HGSGraphicToolImplementationCreator* pi_pCreator);

    // Creation
    HGSGraphicToolImplementation*
    Create(HCLASS_ID                        pi_GraphicToolID,
           const HGSGraphicToolAttributes*  pi_pAttributes,
           HGSSurfaceImplementation*        pi_pSurfaceImplementation) const;

    const HGSGraphicToolImplementationCreator*
    FindCreator(HCLASS_ID                           pi_GraphicToolID,
                HCLASS_ID                           pi_SurfaceImplementationID,
                const HGSGraphicToolCapabilities&   pi_rCapabilities) const;


    bool           IsSurfaceCompatibleWithToolbox(HCLASS_ID           pi_SurfaceImplementationID,
                                                   const HGSToolbox&   pi_rToolbox) const;


protected:

private:

    // list of creators
    typedef list<const HGSGraphicToolImplementationCreator*, allocator<const HGSGraphicToolImplementationCreator*> >
    Creators;

    // Private members
    Creators        m_Creators;

    // disabled methods
    HGSGraphicToolImplementationFactory(const HGSGraphicToolImplementationFactory& pi_rObj);
    HGSGraphicToolImplementationFactory&
    operator=(const HGSGraphicToolImplementationFactory& pi_rObj);
    bool             operator==(const HGSGraphicToolImplementationFactory& pi_rObj);
    bool             operator!=(const HGSGraphicToolImplementationFactory& pi_rObj);
    };

#include "HGSGraphicToolImplementationFactory.hpp"
