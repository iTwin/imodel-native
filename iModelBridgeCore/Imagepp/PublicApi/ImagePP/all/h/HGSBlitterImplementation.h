//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSBlitterImplementation.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSBlitterImplementation
//-----------------------------------------------------------------------------
// General class for Blitters.
//-----------------------------------------------------------------------------
#pragma once

#include "HGSGraphicToolImplementation.h"

class HGSSurfaceImplementation;
class HGSGraphicToolAttributes;
class HGF2DTransfoModel;
class HRATransaction;

class HNOVTABLEINIT HGSBlitterImplementation : public HGSGraphicToolImplementation
    {
    HDECLARE_CLASS_ID(1705, HGSGraphicToolImplementation)

public:

    // Primary methods
    _HDLLg                 HGSBlitterImplementation(const HGSGraphicToolAttributes* pi_pAttributes,
                                                    HGSSurfaceImplementation*       pi_pSurfaceImplementation);

    _HDLLg virtual         ~HGSBlitterImplementation();


    virtual void    BlitFrom(const HGSSurfaceImplementation*    pi_pSrcSurfaceImp,
                             const HGF2DTransfoModel&           pi_rTransfoModel,
                             HRATransaction*                    pi_pTransaction = 0) = 0;

protected:


private:

    // disabled methods
    HGSBlitterImplementation(const HGSBlitterImplementation& pi_rObj);
    HGSBlitterImplementation&
    operator=(const HGSBlitterImplementation& pi_rObj);
    };

