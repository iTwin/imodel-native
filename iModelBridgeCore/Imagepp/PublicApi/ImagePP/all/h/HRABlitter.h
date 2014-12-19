//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABlitter.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRABlitter
//-----------------------------------------------------------------------------
// General class for Blitters.
//-----------------------------------------------------------------------------
#pragma once

#include "HGSBlitterImplementation.h"
#include "HGSMacros.h"
#include "HRASurface.h"
#include "HGSFilter.h"

class HGSGraphicToolAttributes;
class HGSSurfaceImplementation;
class HRPPixelType;
class HGF2DStretch;
class HRATransaction;

HGS_DECLARE_GRAPHICTOOL_DLL(_HDLLg, HRABlitter)

class HRABlitter : public HGSBlitterImplementation
    {
    HDECLARE_CLASS_ID(1740, HGSBlitterImplementation)

    HGS_DECLARE_GRAPHICCAPABILITIES()

public:

    // Primary methods
    HRABlitter(const HGSGraphicToolAttributes*  pi_pAttributes,
               HGSSurfaceImplementation*        pi_pSurfaceImplementation);

    virtual         ~HRABlitter();

    virtual void    BlitFrom(const HGSSurfaceImplementation*    pi_pSrcSurfaceImp,
                             const HGF2DTransfoModel&           pi_rTransfoModel,
                             HRATransaction*                    pi_pTransaction = 0);


protected:

private:

    bool           m_ComposeRequired;
    bool           m_VerticalFlip;
    bool           m_ApplyGrid;
    bool           m_AveragingMode;


    void            Optimized8BitsBlit(const HGSSurfaceImplementation*  pi_pSrcSurfaceImp,
                                       const HGF2DStretch&              pi_rTransfoModel,
                                       HRATransaction*                  pi_pTransaction);
    void            NormalBlit        (const HGSSurfaceImplementation*  pi_pSrcSurfaceImp,
                                       const HGF2DStretch&              pi_rTransfoModel,
                                       HRATransaction*                  pi_pTransaction);
    HRASurface*     ApplyFilter(const HRASurface*                       pi_pSurface,
                                const HFCPtr<HRPFilter>&                pi_rpFilter) const;

    Byte*          CreateWorkingBuffer(const HRPPixelType&             pi_rPixelType,
                                        uint32_t                        pi_Width,
                                        uint32_t                        pi_Height) const;

    // disabled methods
    HRABlitter(const HRABlitter& pi_rObj);
    HRABlitter&  operator=(const HRABlitter& pi_rObj);
    };

