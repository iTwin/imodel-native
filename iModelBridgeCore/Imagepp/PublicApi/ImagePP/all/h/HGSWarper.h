//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSWarper.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSWarper
//-----------------------------------------------------------------------------
// General class for Warper.
//-----------------------------------------------------------------------------

#pragma once

#include "HGSGraphicTool.h"
#include "HFCMatrix.h"

class HGF2DTransfoModel;
class HRATransaction;

class HGSWarper : public HGSGraphicTool
    {
    HDECLARE_CLASS_ID(1780, HGSGraphicTool)

public:

    // Primary methods
    HGSWarper();
    virtual        ~HGSWarper();

    virtual HCLASS_ID
    GetType() const;

    // stretch method
    void            WarpFrom(const HGSSurface*          pi_pSrcSurface,
                             const HGF2DTransfoModel&   pi_rTransfoModel,
                             HRATransaction*            pi_pTransaction = 0);

protected:

private:

    // disabled methods
    HGSWarper(const HGSWarper& pi_rObj);
    HGSWarper&      operator=(const HGSWarper& pi_rObj);
    };

#include "HGSWarper.hpp"

