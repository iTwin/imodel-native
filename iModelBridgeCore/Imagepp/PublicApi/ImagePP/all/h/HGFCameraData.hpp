//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFCameraData.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE

/**----------------------------------------------------------------------------
 This method returns Omega rotation is radians

 @return The Omega rotation angle in radians.
-----------------------------------------------------------------------------*/
inline double HGFCameraData::GetOmega() const
    {
    HINVARIANTS;

    return(m_Omega);
    }


/**----------------------------------------------------------------------------
 This method returns Phi rotation is radians

 @return The Phi rotation angle in radians.
-----------------------------------------------------------------------------*/
inline double HGFCameraData::GetPhi() const
    {
    HINVARIANTS;
    return(m_Phi);
    }

/**----------------------------------------------------------------------------
 This method returns Kappa rotation is radians

 @return The Kappa rotation angle in radians.
-----------------------------------------------------------------------------*/
inline double HGFCameraData::GetKappa() const
    {
    HINVARIANTS;
    return(m_Kappa);
    }


/**----------------------------------------------------------------------------
 This method returns principal point in meters

 @return The principal point. The coordinates are in meters.
-----------------------------------------------------------------------------*/
inline const HGF2DPosition& HGFCameraData::GetPrincipalPoint() const
    {
    HINVARIANTS;
    return(m_PrincipalPoint);
    }


/**----------------------------------------------------------------------------
 This method returns the focal distance in meters

 @return The focal distance in meter
-----------------------------------------------------------------------------*/
inline double HGFCameraData::GetFocalDistance() const
    {
    HINVARIANTS;
    return(m_FocalDistance);
    }


/**----------------------------------------------------------------------------
 This method returns perspective center point in meters

 @return The perspective center point. The coordinates are in meters.
-----------------------------------------------------------------------------*/
inline const HGF3DPoint& HGFCameraData::GetPerspectiveCenter() const
    {
    HINVARIANTS;
    return(m_PerspectiveCenter);
    }




END_IMAGEPP_NAMESPACE


