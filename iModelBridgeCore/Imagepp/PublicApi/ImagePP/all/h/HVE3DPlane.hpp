//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE3DPlane.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

 BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 This method returns true if the two planes are identical
 To be identical the equation parameters must be exactly the same
 The definition points must also be identical

 @return true if both plane and definition points are identical.
-----------------------------------------------------------------------------*/
inline bool HVE3DPlane::operator==(const HVE3DPlane& pi_rObj) const
    {
    return((m_FirstPoint == pi_rObj.m_FirstPoint) &&
           (m_SecondPoint == pi_rObj.m_SecondPoint) &&
           (m_ThirdPoint == pi_rObj.m_ThirdPoint));

    }

/**----------------------------------------------------------------------------
 This method returns true if the two planes are different. A plane is different
 if any of the definition points is different

 @return true if any definition points are different.
-----------------------------------------------------------------------------*/
inline bool HVE3DPlane::operator!=(const HVE3DPlane& pi_rObj) const
    {
    return((m_FirstPoint != pi_rObj.m_FirstPoint) ||
           (m_SecondPoint != pi_rObj.m_SecondPoint) ||
           (m_ThirdPoint != pi_rObj.m_ThirdPoint));
    }




/**----------------------------------------------------------------------------
 This method returns the first definition point

 @return The first definition point
-----------------------------------------------------------------------------*/
inline const HGF3DPoint& HVE3DPlane::GetFirstDefinitionPoint() const
    {
    return(m_FirstPoint);
    }


/**----------------------------------------------------------------------------
 This method returns the second definition point

 @return The second definition point
-----------------------------------------------------------------------------*/
inline const HGF3DPoint& HVE3DPlane::GetSecondDefinitionPoint() const
    {
    return(m_SecondPoint);
    }


/**----------------------------------------------------------------------------
 This method returns the third definition point

 @return The third definition point
-----------------------------------------------------------------------------*/
inline const HGF3DPoint& HVE3DPlane::GetThirdDefinitionPoint() const
    {
    return(m_ThirdPoint);
    }


/**----------------------------------------------------------------------------
 This method returns the elevation at specified position

 @param pi_rPoint The 2D position of the point to obtain elevation on the plane.

 @return The elevation.
-----------------------------------------------------------------------------*/
inline double HVE3DPlane::GetElevationAt(const HGF2DPosition& pi_rPoint) const
    {
    // The plane must be valid to operate
    HPRECONDITION(IsValid());

    // Z = (-Ax-By-D) / C
    return(-(m_A * pi_rPoint.GetX() + m_B * pi_rPoint.GetY() + m_D) / m_C);
    }

/**----------------------------------------------------------------------------
 This method indicates if the plane is valid.

 @return true if valid and false otherwise
-----------------------------------------------------------------------------*/
inline bool HVE3DPlane::IsValid() const
    {
    return(m_C != 0.0);
    }
END_IMAGEPP_NAMESPACE
