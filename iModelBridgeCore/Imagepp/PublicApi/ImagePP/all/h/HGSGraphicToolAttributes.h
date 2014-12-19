//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicToolAttributes.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSGraphicToolAttributes
//-----------------------------------------------------------------------------
// General class for Graphics.
//-----------------------------------------------------------------------------

#pragma once

#include "HGSGraphicToolAttribute.h"
#include "HGSGraphicToolCapabilities.h"

class HGSGraphicToolAttributes
    {
    HDECLARE_BASECLASS_ID(1706)

public:

    // Primary methods
    HGSGraphicToolAttributes();
    HGSGraphicToolAttributes(const HGSGraphicToolAttribute& pi_rAttribute);
    HGSGraphicToolAttributes(const HGSGraphicToolAttributes& pi_rObj);
    virtual         ~HGSGraphicToolAttributes();

    // Attributes editing
    void            Add(const HGSGraphicToolAttribute& pi_rAttribute);

    // Tests the Attributes
    bool           Contains(const HGSGraphicToolAttribute& pi_rAttribute) const;
    bool           Contains(const HGSGraphicToolAttributes& pi_rAttributes) const;

    // chck GT to change
    const HGSGraphicToolAttribute*
    FindOfType(HCLASS_ID pi_AttributeID) const;

    uint32_t        GetCount() const;

    const HFCPtr<HGSGraphicToolAttribute>&
    GetAttribute(uint32_t pi_Index) const;

    HGSGraphicToolCapabilities*
    GetRequiredCapabilities() const;

protected:

private:

    // list of attributes
    typedef vector<HFCPtr<HGSGraphicToolAttribute>, allocator<HFCPtr<HGSGraphicToolAttribute> > >
    Attributes;

    // private members
    Attributes      m_Attributes;

    // disabled methods
    HGSGraphicToolAttributes&
    operator=(const HGSGraphicToolAttributes& pi_rObj);
    bool             operator==(const HGSGraphicToolAttributes& pi_rObj) const;
    bool             operator!=(const HGSGraphicToolAttributes& pi_rObj) const;
    };

#include "HGSGraphicToolAttributes.hpp"

