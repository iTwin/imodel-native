//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFCameraData.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "hstdcpp.h"
#include "HGFCameraData.h"

#include "HGF2DProjective.h"


/**----------------------------------------------------------------------------
 Default constructor.

 The default information is void of signification

-----------------------------------------------------------------------------*/
HGFCameraData::HGFCameraData()
    {
    }


/**----------------------------------------------------------------------------
 Constructor.

 This constructor creates a camera information service from the given meta data

-----------------------------------------------------------------------------*/
HGFCameraData::HGFCameraData(double pi_Omega,
                             double pi_Phi,
                             double pi_Kappa,
                             double pi_FocalDistance,
                             const HGF2DPosition& pi_rPrincipalPoint,
                             const HGF3DPoint& pi_rPerspectiveCenter,
                             const HFCMatrix<3, 3>& pi_rOrientationMatrix)
    : m_Omega(pi_Omega),
      m_Phi(pi_Phi),
      m_Kappa(pi_Kappa),
      m_FocalDistance(pi_FocalDistance),
      m_PrincipalPoint(pi_rPrincipalPoint),
      m_PerspectiveCenter(pi_rPerspectiveCenter),
      m_OrientationMatrix(pi_rOrientationMatrix)

    {
    HINVARIANTS;
    }



/**----------------------------------------------------------------------------
 Copy constructor for this class.

 @param pi_rObj Camera data to duplicate.
-----------------------------------------------------------------------------*/
HGFCameraData::HGFCameraData(const HGFCameraData& pi_rObj)
    : m_Omega(pi_rObj.m_Omega),
      m_Phi(pi_rObj.m_Phi),
      m_Kappa(pi_rObj.m_Kappa),
      m_FocalDistance(pi_rObj.m_FocalDistance),
      m_PrincipalPoint(pi_rObj.m_PrincipalPoint),
      m_PerspectiveCenter(pi_rObj.m_PerspectiveCenter),
      m_OrientationMatrix(pi_rObj.m_OrientationMatrix)
    {
    HINVARIANTS;
    }



/**----------------------------------------------------------------------------
 Destructor
-----------------------------------------------------------------------------*/
HGFCameraData::~HGFCameraData()
    {
    }

/**----------------------------------------------------------------------------
  Assignment operator

  It duplicates a plane
  @param pi_rObj The plane to duplicate

  @return a reference to self to be used as a l-value.
-----------------------------------------------------------------------------*/
HGFCameraData& HGFCameraData::operator=(const HGFCameraData& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        m_Omega = pi_rObj.m_Omega;
        m_Phi = pi_rObj.m_Phi;
        m_Kappa = pi_rObj.m_Kappa;
        m_FocalDistance = pi_rObj.m_FocalDistance;
        m_PrincipalPoint = pi_rObj.m_PrincipalPoint;
        m_PerspectiveCenter = pi_rObj.m_PerspectiveCenter;
        m_OrientationMatrix = pi_rObj.m_OrientationMatrix;
        }

    HINVARIANTS;

    return(*this);
    }


/**----------------------------------------------------------------------------
 This method returns the interior orientation transformation model

 @return interior orientation model
-----------------------------------------------------------------------------*/
HFCPtr<HGF2DTransfoModel> HGFCameraData::GetInteriorOrientation() const
    {
    HINVARIANTS;

    HFCPtr<HGF2DProjective> pProjective = new HGF2DProjective(m_OrientationMatrix);


    return(&*pProjective);
    }


/**----------------------------------------------------------------------------
 This method returns the 3D rotation matrix of the camera to map

 @return 3D rotation matrix
-----------------------------------------------------------------------------*/
HFCMatrix<4, 4> HGFCameraData::Get3DRotationMatrix() const
    {
    HINVARIANTS;

    HFCMatrix<4, 4> MyMatrix;

    double SK = sin(GetKappa());
    double CK = cos(GetKappa());
    double SP = sin(GetPhi());
    double CP = cos(GetPhi());
    double SW = sin(GetOmega());
    double CW = cos(GetOmega());

    MyMatrix[0][0] = CP*CK;
    MyMatrix[0][1] = CW*SK + SW*SP*CK;
    MyMatrix[0][2] = SW*SK - CW*SP*CK;
    MyMatrix[1][0] = - CP*SK;
    MyMatrix[1][1] = CW*CK - SW*SP*SK;
    MyMatrix[1][2] = SW*CK + CW*SP*SK;
    MyMatrix[2][0] = SP;
    MyMatrix[2][1] = -SW*CP;
    MyMatrix[2][2] = CW*CP;
    MyMatrix[3][3] = 1.0;


    return(MyMatrix);
    }







