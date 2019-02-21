/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/HVE2DTriangleFacet.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HVE2DTriangleFacet
//-----------------------------------------------------------------------------
// Description of a triangle facet to be placed in a triangle mesh
//-----------------------------------------------------------------------------

#ifndef __HVE2DTriangleFacet_H__
#define __HVE2DTriangleFacet_H__

#include "HVE2DPolygonOfSegments.h"
#include "HVE2DRectangle.h"


#include "HVE2DFacet.h"
// HPM_DECLARE_HEADER(HVE2DTriangleFacet)

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class implements a general triangle facet. A triangle facet is simply a
    facet of which the shape must be a triangle formed of exactly 3 segments.
    The facets thus defines a closed area.

    A triangle facet imposes a few restrictions upon the point defining the
    triangle. The points must not be co-linear. The triangle must enclose a
    non-null area.
    -----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
template<class ATTRIBUTE> class HVE2DTriangleFacet : public HVE2DFacet<ATTRIBUTE>
    {

public:

    // Primary methods
    HVE2DTriangleFacet(const HVE2DTriangleFacet<ATTRIBUTE>& pi_rObject);
    HVE2DTriangleFacet(const HVE2DPolygonOfSegments& pi_rPolygon);
    HVE2DTriangleFacet(const HVE2DPolygonOfSegments& pi_rPolygon,
                       const ATTRIBUTE& pi_rAttribute);
    virtual              ~HVE2DTriangleFacet();

    HVE2DTriangleFacet&              operator=(const HVE2DTriangleFacet<ATTRIBUTE>& pi_rObj);

    // Triangular Facet specific interface
    const HVE2DPolygonOfSegments&    GetTriangle() const;

    void                             SetTriangle(const HVE2DPolygonOfSegments& pi_rQuadrilater);

    const HGF2DPosition&             GetFirstSummit() const;
    const HGF2DPosition&             GetSecondSummit() const;
    const HGF2DPosition&             GetThirdSummit() const;

    virtual bool                     IsPointIn(const HGF2DLocation& i_rPoint) const;
    virtual bool                     IsPointIn(const HGF2DPosition& i_rPoint) const;



    // Overide from HVE2DFacet
    virtual HVE2DFacet<ATTRIBUTE>*   Clone() const;
    virtual HVE2DFacet<ATTRIBUTE>*   AllocateTransformed(const HFCMatrix<3, 3>& i_rTransformMatrix) const;

private:

    // Desactivated constructor
    HVE2DTriangleFacet();

    HGF2DPosition m_FirstPoint;
    HGF2DPosition m_SecondPoint;
    HGF2DPosition m_ThirdPoint;


    };
END_IMAGEPP_NAMESPACE

#include "HVE2DTriangleFacet.hpp"

#endif
