//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicTool.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSGraphicTool
//-----------------------------------------------------------------------------
// General class for GraphicTools.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HFCPtr.h"
#include "HGSGraphicToolImplementation.h"
#include "HGSSurface.h"
#include "HGSGraphicToolAttributes.h"

class HGSGraphicToolAttribute;

class HNOVTABLEINIT HGSGraphicTool : public HFCShareableObject<HGSGraphicTool>
    {
    HDECLARE_BASECLASS_ID(1707)

public:

    // Primary methods
    _HDLLg virtual        ~HGSGraphicTool();

    // type
    virtual HCLASS_ID
    GetType() const = 0;

    // attributes
    const HGSGraphicToolAttributes&
    GetAttributes() const;
    void            AddAttribute(const HGSGraphicToolAttribute& pi_rAttribute);

    // set the tool for a surface
    _HDLLg void            SetFor(const HFCPtr<HGSSurface>& pi_rpSurface);
    HFCPtr<HGSSurface>
    GetSurface() const;

protected:

    _HDLLg                 HGSGraphicTool();

    _HDLLg                 HGSGraphicTool(const HGSGraphicToolAttributes& pi_rAttributes);

    HGSGraphicToolImplementation*
    GetImplementation() const;

private:

    // private members
    HGSGraphicToolAttributes
    m_Attributes;

    HFCPtr<HGSSurface>
    m_pSurface;

    HAutoPtr<HGSGraphicToolImplementation>
    m_pImplementation;

    // disabled methods
    HGSGraphicTool(const HGSGraphicTool& pi_rObj);
    HGSGraphicTool&
    operator=(const HGSGraphicTool& pi_rObj);
    bool             operator==(const HGSGraphicTool& pi_rObj) const;
    bool             operator!=(const HGSGraphicTool& pi_rObj);
    };

#include "HGSGraphicTool.hpp"
