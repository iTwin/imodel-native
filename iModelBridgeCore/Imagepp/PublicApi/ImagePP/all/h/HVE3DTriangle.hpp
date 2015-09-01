//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE3DTriangle.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------



BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 This method returns the elevation at specified position

 @param pi_rPoint The 2D position of the point to obtain elevation on the plane.

 @return The elevation.
-----------------------------------------------------------------------------*/
inline double HVE3DTriangle::GetElevationAt(const HGF2DPosition& pi_rPoint) const
    {
    // The point must be located inside (or on) the triangle
    HPRECONDITION(GetTriangle().IsPointIn(HGF2DLocation(pi_rPoint, GetTriangle().GetCoordSys())) ||
                  GetTriangle().IsPointOn(HGF2DLocation(pi_rPoint, GetTriangle().GetCoordSys())));

    // Obtain elevation from attribute
    return(GetAttribute().GetElevationAt(pi_rPoint));
    }




/**----------------------------------------------------------------------------
 This method returns a reference to the plane

 @return The 3D plane
-----------------------------------------------------------------------------*/
inline const HVE3DPlane&  HVE3DTriangle::Get3DPlane() const
    {
    return(GetAttribute());
    }
END_IMAGEPP_NAMESPACE
