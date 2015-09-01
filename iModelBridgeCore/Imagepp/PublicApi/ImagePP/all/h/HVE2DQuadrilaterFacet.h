//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DQuadrilaterFacet.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HVE2DFacet
//-----------------------------------------------------------------------------
// Description of a mesh
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DPolygonOfSegments.h"
#include "HVE2DRectangle.h"


#include "HVE2DFacet.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements a general quadrilater facet. A quadrilater facet is simply a
    facet of which the shape must be a quadrilater formed of exactly 4 segments.
    The facets thus defines a closed area.

    A quadrilater facet imposes a few restrictions upon the point defining the
    quadrilater. The points must not be all co-linear. The quadrilater must enclose a
    non-null area. It is valid however for up to 3 out of 4 points (summits) to be
    co-linear.

    -----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
template<class ATTRIBUTE> class HVE2DQuadrilaterFacet : public HVE2DFacet<ATTRIBUTE>
    {

public:

    // Primary methods
    HVE2DQuadrilaterFacet(const HVE2DQuadrilaterFacet<ATTRIBUTE>& pi_rObject);
    HVE2DQuadrilaterFacet(const HVE2DPolygonOfSegments& pi_rPolygon);
    HVE2DQuadrilaterFacet(const HVE2DPolygonOfSegments& pi_rPolygon,
                          const ATTRIBUTE& pi_rAttribute);
    HVE2DQuadrilaterFacet(const HVE2DRectangle& pi_rRectangle);
    HVE2DQuadrilaterFacet(const HVE2DRectangle& pi_rRectangle,
                          const ATTRIBUTE& pi_rAttribute);
    virtual            ~HVE2DQuadrilaterFacet();

    HVE2DQuadrilaterFacet&
    operator=(const HVE2DQuadrilaterFacet<ATTRIBUTE>& pi_rObj);

    // Quadrilater Facet specific interface
    const HVE2DPolygonOfSegments&
    GetQuadrilater() const;

    void               SetQuadrilater(const HVE2DPolygonOfSegments& pi_rQuadrilater);
    void               SetQuadrilater(const HVE2DRectangle& pi_rQuadrilater);


    // Overide from HVE2DFacet
    virtual HVE2DFacet<ATTRIBUTE>*
    Clone() const;

private:

    // Desactivated constructor
    HVE2DQuadrilaterFacet();

    };
END_IMAGEPP_NAMESPACE

#include "HVE2DQuadrilaterFacet.hpp"

