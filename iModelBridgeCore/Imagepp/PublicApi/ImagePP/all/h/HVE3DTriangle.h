//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE3DTriangle.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HVE3DTriangle
//-----------------------------------------------------------------------------
// Description of a mesh
//-----------------------------------------------------------------------------
#pragma once

#include "HGF3DPoint.h"
#include "HVE2DTriangleFacet.h"
#include "HVE3DPlane.h"
// HPM_DECLARE_HEADER(HVE3DTriangle)

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    -----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
class HVE3DTriangle : public HVE2DTriangleFacet<HVE3DPlane>
    {

public:

    // Primary methods
    HVE3DTriangle(const HVE3DTriangle& pi_rObject);
#if (0)
    HVE3DTriangle(const HGF3DPoint& pi_rFirstPoint,
                  const HGF3DPoint& pi_rSecondPoint,
                  const HGF3DPoint& pi_rThirdPoint,
                  const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys,
                  double i_Tolerance = HVE_USE_AUTO_TOLERANCE);
#else
    HVE3DTriangle(const HGF3DPoint& pi_rFirstPoint,
                  const HGF3DPoint& pi_rSecondPoint,
                  const HGF3DPoint& pi_rThirdPoint,
                  const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys);
#endif
    HVE3DTriangle(const HVE2DTriangleFacet<HVE3DPlane>& pi_rpFacet);
    virtual            ~HVE3DTriangle();

    HVE3DTriangle&
    operator=(const HVE3DTriangle& pi_rObj);

    // Plane specific interface

    // Elevation extraction
    double            GetElevationAt(const HGF2DPosition& pi_rPoint) const;
    const HVE3DPlane&  Get3DPlane() const;

#if (0)
    // From RawFacet
    virtual bool       IsPointIn(const HGF2DLocation& i_rPoint) const;
    virtual bool       IsPointIn(const HGF2DPosition& i_rPoint) const;
#endif

    // From Facet
    virtual HVE2DFacet<HVE3DPlane>*
    AllocateTransformed(const HFCMatrix<3, 3>& i_rTransformMatrix) const;


private:

    // Desactivate default constructor
    HVE3DTriangle();

//        HGF3DPoint m_FirstPoint;
//        HGF3DPoint m_SecondPoint;
//        HGF3DPoint m_ThirdPoint;

    };
END_IMAGEPP_NAMESPACE

#include "HVE3DTriangle.hpp"

