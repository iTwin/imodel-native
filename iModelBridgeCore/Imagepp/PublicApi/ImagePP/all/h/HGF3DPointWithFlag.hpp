//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF3DPointWithFlag.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#include "HNumeric.h"

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HGF3DPointWithFlag::HGF3DPointWithFlag()
    : HGF3DCoord<double>(0.0, 0.0, 0.0),
      m_UserFlag(0)
    {
    }

//-----------------------------------------------------------------------------
// Full Constructor
//-----------------------------------------------------------------------------
inline HGF3DPointWithFlag::HGF3DPointWithFlag(double pi_X,
                                              double pi_Y,
                                              double pi_Z,
                                              int    pi_UserFlag)
    : HGF3DCoord<double>(pi_X, pi_Y, pi_Z),
      m_UserFlag(pi_UserFlag)
    {
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HGF3DPointWithFlag::HGF3DPointWithFlag(const HGF3DPointWithFlag& pi_rObj)
    : HGF3DCoord<double>(pi_rObj), m_UserFlag(pi_rObj.m_UserFlag)
    {
    }

//-----------------------------------------------------------------------------
// Full Constructor from HGF3DCoord
//-----------------------------------------------------------------------------
inline HGF3DPointWithFlag::HGF3DPointWithFlag(const HGF3DCoord<double> pi_3DPoint,
                                              int    pi_UserFlag)
    : HGF3DCoord<double>(pi_3DPoint),
      m_UserFlag(pi_UserFlag)
    {
    }



//-----------------------------------------------------------------------------
// Full Constructor from HGF2DCoord
//-----------------------------------------------------------------------------
inline HGF3DPointWithFlag::HGF3DPointWithFlag(const HGF2DCoord<double> pi_2DPoint,
                                              double pi_Elevation,
                                              int    pi_UserFlag)
    : HGF3DCoord<double>(pi_2DPoint, pi_Elevation),
      m_UserFlag(pi_UserFlag)
    {
    }



//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
inline HGF3DPointWithFlag::~HGF3DPointWithFlag()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another position object.
//-----------------------------------------------------------------------------
inline HGF3DPointWithFlag& HGF3DPointWithFlag::operator=(const HGF3DPointWithFlag& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HGF3DCoord<double>::operator=(pi_rObj);
        m_UserFlag = pi_rObj.m_UserFlag;
        }
    return (*this);
    }


/** -----------------------------------------------------------------------------
    Returns the value of the user flag

    @return The user flag for this DTM point

    Example:
    @code
    HGF3DPointWithFlag      MyPoint (10.0, 10.0, 0.0, 2);

        int = MyPoint.GetUserFlag();
    @end
    -----------------------------------------------------------------------------
*/
inline int HGF3DPointWithFlag::GetUserFlag() const
    {
    // Return x value
    return (m_UserFlag);
    }


/** -----------------------------------------------------------------------------
    Sets a new value for the user flag

    @param pi_UserFlag New user flag to assign.

    Example:
    @code
    HGF3DPointWithFlag      MyPoint;

        MyPoint.SetUserFlag(12);
    @end

    @see GetUserFlag()
    -----------------------------------------------------------------------------
*/
inline void HGF3DPointWithFlag::SetUserFlag(int pi_UserFlag)
    {
    // Set value
    m_UserFlag = pi_UserFlag;
    }


/** -----------------------------------------------------------------------------
    Equality operator.  Two coordinate objects are considered "equal" if they
    have the same coordinates.

    @param pi_rObj Constant reference to coordinate object to compare to.

    @return A boolean value that reflects the equality or non-equality
            between operands.

    Example:
    @code
    HGF3DPointWithFlag<double> MyFirstPoint (0, 0);
        HGF3DPointWithFlag<double> OtherPoint (10.0, 10.0)
        if (ImageOrigin == OtherPoint)
        {
          ...
        }
    @end

    @see operator!=()
    @see operator<()
    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
inline bool HGF3DPointWithFlag::operator==(const HGF3DPointWithFlag& pi_rObj) const
    {
    return (HGF3DCoord<double>::operator==(pi_rObj) && (m_UserFlag == pi_rObj.m_UserFlag));
    }

/** -----------------------------------------------------------------------------
    Inequality operator.  Two coordinate objects are considered "equal" if they
    have the same coordinates.

    @param pi_rObj Constant reference to coordinate object to compare to.

    @return A boolean value that reflects the equality or non-equality
            between operands.

    Example:
    @code
    HGF3DPointWithFlag<double> MyFirstPoint (0, 0);
        HGF3DPointWithFlag<double> OtherPoint (10.0, 10.0)
        if (ImageOrigin != OtherPoint)
        {
          ...
        }
    @end

    @see operator==()
    @see operator<()
    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
inline bool HGF3DPointWithFlag::operator!=(const HGF3DPointWithFlag& pi_rObj) const
    {
    return (HGF3DCoord<double>::operator!=(pi_rObj) || (m_UserFlag != pi_rObj.m_UserFlag));
    }














