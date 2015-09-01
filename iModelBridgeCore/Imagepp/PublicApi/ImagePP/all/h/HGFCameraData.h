//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFCameraData.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFCameraData
//-----------------------------------------------------------------------------
// Camera orientation parameters
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DPosition.h"
#include "HGF3DPoint.h"
#include "HGF2DTransfoModel.h"
#include "HGFMatrixOps.h"

BEGIN_IMAGEPP_NAMESPACE

class HGFCameraData
    {
public:
    HGFCameraData();
    HGFCameraData(double pi_Omega,
                  double pi_Phi,
                  double pi_Kappa,
                  double pi_FocalDistance,
                  const HGF2DPosition& pi_rPrincipalPoint,
                  const HGF3DPoint& pi_rPerspectiveCenter,
                  const HFCMatrix<3, 3>& pi_OrientationMatrix);
    HGFCameraData(const HGFCameraData& pi_rObj);
    ~HGFCameraData();

    HGFCameraData&  operator=(const HGFCameraData& pi_rObj);

    // Orientation parameters
    double          GetOmega() const;
    double          GetPhi() const;
    double          GetKappa() const;
    const HGF2DPosition&
    GetPrincipalPoint() const;
    double          GetFocalDistance() const;
    const HGF3DPoint&
    GetPerspectiveCenter() const;


    // Interior orientation
    HFCPtr<HGF2DTransfoModel>
    GetInteriorOrientation() const;

    // Orientation rotation matrix (3D)
    HFCMatrix<4, 4> Get3DRotationMatrix() const;

#ifdef HVERIFYCONTRACT
    void               ValidateInvariants() const
        {
        // The focal distance may not be 0 nor negative
        HASSERT(m_FocalDistance > 0.0);

        // The interior orientation must be valid (non-null determinant)
        HASSERT(CalculateDeterminant(m_OrientationMatrix) != 0.0);
        }
#endif

protected:

private:

    double m_Omega;
    double m_Phi;
    double m_Kappa;

    double m_FocalDistance;

    HGF2DPosition m_PrincipalPoint;
    HGF3DPoint    m_PerspectiveCenter;

    HFCMatrix<3, 3> m_OrientationMatrix;

    };

END_IMAGEPP_NAMESPACE
#include "HGFCameraData.hpp"

