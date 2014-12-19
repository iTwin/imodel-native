//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSWarperImplementation.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSWarperImplementation
//-----------------------------------------------------------------------------
// General class for Warper.
//-----------------------------------------------------------------------------

#pragma once

#include "HGSGraphicToolImplementation.h"
#include "HFCMatrix.h"

class HGF2DTransfoModel;
class HRATransaction;

class HGSWarperImplementation : public HGSGraphicToolImplementation
    {
    HDECLARE_CLASS_ID(1781, HGSGraphicToolImplementation)

public:

    // Primary methods
    HGSWarperImplementation(const HGSGraphicToolAttributes* pi_pAttributes,
                            HGSSurfaceImplementation*       pi_pSurfaceImplementation);

    virtual         ~HGSWarperImplementation();



    virtual void    WarpFrom(const HGSSurfaceImplementation*    pi_pSrcSurfaceImp,
                             const HGF2DTransfoModel&           pi_rTransfoModel,
                             HRATransaction*                    pi_pTransaction = 0) = 0;

protected:


private:

    // disabled methods
    HGSWarperImplementation();
    HGSWarperImplementation(const HGSWarperImplementation& pi_rObj);
    HGSWarperImplementation&
    operator=(const HGSWarperImplementation& pi_rObj);
    };

