//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFHVVHSpatialAttribute.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFHVVHSpatialAttribute
//-----------------------------------------------------------------------------
// spatial indexing attribute
//-----------------------------------------------------------------------------

#pragma once


#include "HGF2DExtent.h"
#include "HGFSpatialCriteria.h"
#include "HIDXAttribute.h"

BEGIN_IMAGEPP_NAMESPACE

class HGFHVVHSpatialAttribute : public HIDXAttribute
    {
public:

    HGFHVVHSpatialAttribute(void* pi_pNode, const HGF2DExtent& pi_rExtent);

    virtual         ~HGFHVVHSpatialAttribute();

    void            SetNode(void* pi_pNode);
    void*           GetNode() const;
    void            SetExtent(const HGF2DExtent& pi_rExtent);
    HGF2DExtent&    GetExtent() const;

private:

    // Disabled.
    HGFHVVHSpatialAttribute(const HGFHVVHSpatialAttribute& pi_rObj);
    HGFHVVHSpatialAttribute&
    operator=(const HGFHVVHSpatialAttribute& pi_rObj);

    // The node where the object is stored. It is either a leaf node
    // (contains the object directly), or a non-leaf (object is in the
    // node's cut tree).
    void*           m_pNode;

    mutable HGF2DExtent
    m_Extent;
    };

END_IMAGEPP_NAMESPACE
#include "HGFHVVHSpatialAttribute.hpp"


