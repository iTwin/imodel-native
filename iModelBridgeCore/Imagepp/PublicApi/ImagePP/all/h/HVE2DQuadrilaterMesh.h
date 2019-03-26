//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DQuadrilaterMesh.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HVE2DMesh
//-----------------------------------------------------------------------------
// Description of a mesh
//-----------------------------------------------------------------------------

#pragma once

#include "HVE2DMesh.h"
#include "HVE2DQuadrilaterFacet.h"


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements a general quadrilater mesh. A quadrilater mesh is simply a
    mesh of which the facets are composed of four segments linked one to the other.
    The facets thus defines a closed area. As with any mesh, each facet may not
    overlap any other facet. In a mesh, a single position may belong to at most
    one facet.

    -----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
template<class ATTRIBUTE> class HVE2DQuadrilaterMesh : public HVE2DMesh<ATTRIBUTE>
    {

public:

    // Primary methods
    HVE2DQuadrilaterMesh(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
    virtual            ~HVE2DQuadrilaterMesh();


    bool             AddFacet(const HVE2DQuadrilaterFacet<ATTRIBUTE>& pi_rFacet);

protected:

private:
    // Desactivated
    HVE2DQuadrilaterMesh(const HVE2DQuadrilaterMesh&    pi_rObject);
    HVE2DQuadrilaterMesh&
    operator=(const HVE2DQuadrilaterMesh& pi_rObj);
    HVE2DQuadrilaterMesh();

    };
END_IMAGEPP_NAMESPACE

#include "HVE2DQuadrilaterMesh.hpp"

