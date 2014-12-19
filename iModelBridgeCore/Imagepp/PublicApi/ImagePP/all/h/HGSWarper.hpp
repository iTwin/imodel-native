//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSWarper.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> inline method for class HGSWarper
//:>-----------------------------------------------------------------------------

#include "HGSWarperImplementation.h"

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
inline HGSWarper::HGSWarper()
    : HGSGraphicTool()
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
inline HGSWarper::~HGSWarper()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetType
//-----------------------------------------------------------------------------
inline HCLASS_ID HGSWarper::GetType() const
    {
    return HGSWarperImplementation::CLASS_ID;
    }


//-----------------------------------------------------------------------------
// public
// WarpFrom
//-----------------------------------------------------------------------------
inline void HGSWarper::WarpFrom(const HGSSurface*           pi_pSrcSurface,
                                const HGF2DTransfoModel&    pi_rTransfoModel,
                                HRATransaction*             pi_pTransaction)
    {
    HPRECONDITION(GetImplementation() != 0);
    HPRECONDITION(pi_pSrcSurface != 0);
    HPRECONDITION(pi_pSrcSurface->GetImplementation() != 0);
    HPRECONDITION(pi_pSrcSurface->GetImplementation()->GetClassID() == GetImplementation()->GetSurfaceImplementation()->GetClassID());

    ((HGSWarperImplementation*)GetImplementation())->WarpFrom(pi_pSrcSurface->GetImplementation(),
                                                              pi_rTransfoModel,
                                                              pi_pTransaction);
    }

