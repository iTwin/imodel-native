//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVEVSRelativeAttribute.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HVEVSRelativeAttribute
//-----------------------------------------------------------------------------
// spatial indexing attribute
//-----------------------------------------------------------------------------

#pragma once


#include "HVEShape.h"


BEGIN_IMAGEPP_NAMESPACE
class HVEVSRelativeAttribute : public HIDXAttribute
    {
public:

    HVEVSRelativeAttribute(const HFCPtr<HVEShape>& pi_rpShape = 0);

    virtual         ~HVEVSRelativeAttribute();


    void            ClearShape();
    void            SetShape(const HFCPtr<HVEShape>& pi_rpShape);
    HFCPtr<HVEShape>&
    GetShape() const;

private:

    // Disabled.
    HVEVSRelativeAttribute(const HVEVSRelativeAttribute& pi_rObj);
    HVEVSRelativeAttribute&
    operator=(const HVEVSRelativeAttribute& pi_rObj);

    mutable HFCPtr<HVEShape>
    m_pShape;
    };
END_IMAGEPP_NAMESPACE

#include "HVEVSRelativeAttribute.hpp"


