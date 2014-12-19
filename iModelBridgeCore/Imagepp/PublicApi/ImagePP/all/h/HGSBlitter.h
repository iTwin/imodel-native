//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSBlitter.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSBlitter
//-----------------------------------------------------------------------------
// General class for Stretchers.
//-----------------------------------------------------------------------------

#pragma once

#include "HGSGraphicTool.h"

class HGF2DTransfoModel;
class HRATransaction;

class HGSBlitter : public HGSGraphicTool
    {
    HDECLARE_CLASS_ID(1700, HGSGraphicTool)

public:

    // Primary methods
    HGSBlitter();
    virtual        ~HGSBlitter();


    // type
    virtual HCLASS_ID
    GetType() const;

    // stretch method
    void            BlitFrom(const HGSSurface*          pi_pSrcSurface,
                             HRATransaction*            pi_pTransaction = 0);
    void            BlitFrom(const HGSSurface*          pi_pSrcSurface,
                             const HGF2DTransfoModel&   pi_rTransfoModel,
                             HRATransaction*            pi_pTransaction = 0);

protected:

private:

    // disabled methods
    HGSBlitter(const HGSBlitter& pi_rObj);
    HGSBlitter&     operator=(const HGSBlitter& pi_rObj);
    };

#include "HGSBlitter.hpp"

