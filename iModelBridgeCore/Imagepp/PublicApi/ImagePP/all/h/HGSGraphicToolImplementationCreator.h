//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicToolImplementationCreator.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSGraphicToolImplementationCreator
//-----------------------------------------------------------------------------
// General class for GraphicTools.
//-----------------------------------------------------------------------------
#pragma once

class HGSGraphicToolAttributes;
class HGSGraphicToolCapabilities;
class HGSGraphicToolImplementation;
class HGSSurfaceImplementation;

class HNOVTABLEINIT HGSGraphicToolImplementationCreator
    {
    HDECLARE_BASECLASS_ID(1709)

public:

    // Primary methods
    _HDLLg virtual         ~HGSGraphicToolImplementationCreator();

    // Capabilities
    virtual const HGSGraphicToolCapabilities*
    GetCapabilities() const = 0;

    // Tool type
    virtual HCLASS_ID
    GetGraphicToolID() const = 0;

    // Associated surface implementation
    virtual HCLASS_ID
    GetSurfaceImplementationID() const = 0;

    // Creation
    virtual HGSGraphicToolImplementation*
    Create(const HGSGraphicToolAttributes*  pi_pAttributes,
           HGSSurfaceImplementation*        pi_pSurfaceImplementation) const = 0;

protected:

    // Primary methods
    _HDLLg                 HGSGraphicToolImplementationCreator();

private:

    // Disabled Methods
    HGSGraphicToolImplementationCreator(
        const HGSGraphicToolImplementationCreator& pi_rObj);
    HGSGraphicToolImplementationCreator&
    operator=(  const HGSGraphicToolImplementationCreator& pi_rObj);
    bool             operator==( const HGSGraphicToolImplementationCreator& pi_rObj) const;
    bool             operator!=( const HGSGraphicToolImplementationCreator& pi_rObj) const;
    };

