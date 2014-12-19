//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAWarper.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRAWarper
//-----------------------------------------------------------------------------
// General class for Warper.
//-----------------------------------------------------------------------------
#pragma once

#include "HGSWarperImplementation.h"
#include "HGSMacros.h"
#include "HRPFilter.h"
#include "HRASurface.h"


class HRATransaction;

HGS_DECLARE_GRAPHICTOOL_DLL(_HDLLg, HRAWarper)

class HRAWarper : public HGSWarperImplementation
    {
    HDECLARE_CLASS_ID(1782, HGSWarperImplementation)

    HGS_DECLARE_GRAPHICCAPABILITIES()

public:

    // Primary methods
    HRAWarper(const HGSGraphicToolAttributes*   pi_pAttributes,
              HGSSurfaceImplementation*         pi_pSurfaceImplementation);

    virtual         ~HRAWarper();


    virtual void    WarpFrom(const HGSSurfaceImplementation*    pi_pSrcSurfaceImp,
                             const HGF2DTransfoModel&           pi_rTransfoModel,
                             HRATransaction*                    pi_pTransaction = 0);


protected:

private:

    // Members
    bool               m_ComposeRequired;
    bool               m_ApplyGrid;

    void                Optimized8BitsWarp(const HGSSurfaceImplementation*  pi_pSrcSurfaceImp,
                                           const HGF2DTransfoModel&         pi_rTransfoModel,
                                           HRATransaction*                  pi_pTransaction);

    void                NormalWarp(const HGSSurfaceImplementation*          pi_pSrcSurfaceImp,
                                   const HGF2DTransfoModel&                 pi_rTransfoModel,
                                   HRATransaction*                          pi_pTransaction);

    HRASurface*         ApplyFilter(const HRASurface*                       pi_pSurface,
                                    const HFCPtr<HRPFilter>&                pi_rpFilter) const;

    Byte*              CreateWorkingBuffer(const HRPPixelType&             pi_rPixelType,
                                            uint32_t                        pi_Width,
                                            uint32_t                        pi_Height) const;

    // disabled methods
    HRAWarper();
    HRAWarper(const HRAWarper& pi_rObj);
    HRAWarper&      operator=(const HRAWarper& pi_rObj);

    };
