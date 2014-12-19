//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicToolImplementation.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSGraphicToolImplementation
//-----------------------------------------------------------------------------
// General class for GraphicTools.
//-----------------------------------------------------------------------------
#pragma once

#include "HGSGraphicToolImplementationCreator.h"
#include "HFCMacros.h"

class HGSGraphicToolAttributes;
class HGSSurfaceImplementation;

class HNOVTABLEINIT HGSGraphicToolImplementation
    {
    HDECLARE_BASECLASS_ID(1708)

public:

    // Primary methods
    virtual             ~HGSGraphicToolImplementation();

    // Implementations creation
    virtual const HGSGraphicToolImplementationCreator*
    GetCreator() const = 0;

    HGSSurfaceImplementation*
    GetSurfaceImplementation() const;

    const HGSGraphicToolAttributes*
    GetAttributes() const;
    _HDLLg virtual void        SetAttributes(const HGSGraphicToolAttributes* pi_pAttributes);

protected:

    HGSGraphicToolImplementation(const HGSGraphicToolAttributes* pi_pAttributes,
                                 HGSSurfaceImplementation*       pi_pSurfaceImplementation);



private:

    // private members
    HGSSurfaceImplementation*       m_pSurfaceImplementation;
    const HGSGraphicToolAttributes* m_pAttributes;

    // disabled methods
    HGSGraphicToolImplementation(const HGSGraphicToolImplementation& pi_rObj);
    HGSGraphicToolImplementation&
    operator=(const HGSGraphicToolImplementation& pi_rObj);
    bool             operator==(const HGSGraphicToolImplementation& pi_rObj) const;
    bool             operator!=(const HGSGraphicToolImplementation& pi_rObj);
    };

#include "HGSGraphicToolImplementation.hpp"

