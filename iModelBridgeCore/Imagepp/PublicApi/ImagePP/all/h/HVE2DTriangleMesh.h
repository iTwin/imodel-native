//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DTriangleMesh.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE2DTriangleMesh
//-----------------------------------------------------------------------------
// Description of a mesh
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DMesh.h"
#include "HVE2DTriangleFacet.h"


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements a general triangle mesh. A triangle mesh is simply a
    mesh of which the facets are composed of three segments linked one to the other.
    The facets thus defines a closed area. As with any mesh, each facet may not
    overlap any other facet. In a mesh, a single position may belong to at most
    one facet.

    -----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
template<class ATTRIBUTE> class HVE2DTriangleMesh : public HVE2DMesh<ATTRIBUTE>
    {

public:

    // Primary methods
    HVE2DTriangleMesh(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    virtual            ~HVE2DTriangleMesh();

    bool              AddFacet(const HVE2DTriangleFacet<ATTRIBUTE>& pi_rFacet);
    bool              AddFacet(HVE2DTriangleFacet<ATTRIBUTE>& i_rpFacet);

    virtual const HVE2DFacet<ATTRIBUTE>*
    GetFacetAt(const HGF2DLocation& pi_rLocation) const;

    virtual const HVE2DFacet<ATTRIBUTE>*
    GetFacetAt(const HGF2DPosition& i_rPosition) const;


protected:


private:
    // Desactivated
    HVE2DTriangleMesh(const HVE2DTriangleMesh&   pi_rObject);
    HVE2DTriangleMesh&
    operator=(const HVE2DTriangleMesh& pi_rObj);
    HVE2DTriangleMesh();

    // Acceleration attributes
    mutable double m_x1;
    mutable double m_x2;
    mutable double m_x3;
    mutable double m_y1;
    mutable double m_y2;
    mutable double m_y3;
    mutable double m_x2Mx1;
    mutable double m_x3Mx2;
    mutable double m_x1Mx3;
    mutable double m_y2My1;
    mutable double m_y3My2;
    mutable double m_y1My3;

    };
END_IMAGEPP_NAMESPACE

#include "HVE2DTriangleMesh.hpp"

