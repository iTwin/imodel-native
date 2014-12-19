//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSBlitter.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSBlitter
//-----------------------------------------------------------------------------
// General class for Stretchers.
//-----------------------------------------------------------------------------

#include "HGSBlitterImplementation.h"
#include "HGF2DStretch.h"
#include "HGF2DDisplacement.h"
#include "HRATransaction.h"


//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
inline HGSBlitter::HGSBlitter()
    : HGSGraphicTool()
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
inline HGSBlitter::~HGSBlitter()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetType
//-----------------------------------------------------------------------------
inline HCLASS_ID HGSBlitter::GetType() const
    {
    return HGSBlitterImplementation::CLASS_ID;
    }


//-----------------------------------------------------------------------------
// public
// Stretch
//-----------------------------------------------------------------------------
inline void HGSBlitter::BlitFrom(const HGSSurface*  pi_pSrcSurface,
                                 HRATransaction*    pi_pTransaction)
    {
    HPRECONDITION(GetImplementation() != 0);
    HPRECONDITION(pi_pSrcSurface != 0);

    HFCPtr<HGSSurfaceDescriptor> pDstDescriptor = GetSurface()->GetSurfaceDescriptor();
    HFCPtr<HGSSurfaceDescriptor> pSrcDescriptor = pi_pSrcSurface->GetSurfaceDescriptor();

    BlitFrom(pi_pSrcSurface,
             HGF2DStretch(HGF2DDisplacement(),
                          pDstDescriptor->GetWidth() / pSrcDescriptor->GetWidth(),
                          pDstDescriptor->GetHeight() / pSrcDescriptor->GetHeight()),
             pi_pTransaction);
    }

//-----------------------------------------------------------------------------
// public
// Stretch
//-----------------------------------------------------------------------------
inline void HGSBlitter::BlitFrom(const HGSSurface*          pi_pSrcSurface,
                                 const HGF2DTransfoModel&   pi_rTransfoModel,
                                 HRATransaction*            pi_pTransaction)
    {
    HPRECONDITION(GetImplementation() != 0);
    HPRECONDITION(pi_pSrcSurface != 0);
    HPRECONDITION(pi_pSrcSurface->GetImplementation() != 0);
    HPRECONDITION(pi_pSrcSurface->GetImplementation()->GetClassID() == GetImplementation()->GetSurfaceImplementation()->GetClassID());

    ((HGSBlitterImplementation*)GetImplementation())->BlitFrom(pi_pSrcSurface->GetImplementation(),
                                                               pi_rTransfoModel,
                                                               pi_pTransaction);
    }

