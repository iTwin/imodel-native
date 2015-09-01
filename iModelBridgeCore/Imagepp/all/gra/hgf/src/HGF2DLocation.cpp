//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DLocation.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DLocation
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DLocation.h>



/** -----------------------------------------------------------------------------
    Equality and Inequality operators.  Two location objects are considered "equal"
    if they map to the same physical location in either coordinate system.
    If they do not use the same coordinate system, internal conversion occurs.

    @param pi_rObj IN Constant reference to location object to compare to.

    @return boolean value that reflects the equality or non-equality between
            operands.

    @see operator!=()
    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
bool HGF2DLocation::operator==(const HGF2DLocation& pi_rObj) const
    {
    // Declared doubles to receive results
    double     NewX;
    double     NewY;

    // Obtain new coordinates (expressed in the coordinate sys of self
    if (m_pCoordSys != pi_rObj.m_pCoordSys)
        {
        m_pCoordSys->ConvertFrom (pi_rObj.m_pCoordSys, pi_rObj.m_XValue, pi_rObj.m_YValue, &NewX, &NewY);

        // Compare coordinates
        return((NewX == m_XValue) && (NewY == m_YValue));
        }
    else
        {
        return((pi_rObj.m_XValue == m_XValue) && (pi_rObj.m_YValue == m_YValue));
        }
    }


/** -----------------------------------------------------------------------------
    Patch method required for older versions of MSVC. STL list required
    such an operator.

    to be destroyed
    -----------------------------------------------------------------------------
*/
bool HGF2DLocation::operator<(const HGF2DLocation& pi_rObj) const
    {
    // Declared doubles to receive results
    double     NewX;
    double     NewY;

    // Obtain new coordinates (expressed in the coordinate sys of self
    m_pCoordSys->ConvertFrom (pi_rObj.m_pCoordSys, pi_rObj.m_XValue, pi_rObj.m_YValue,
                              &NewX, &NewY);

    // Compare coordinates
    return((NewX < m_XValue) || (NewY < m_YValue));
    }



/** -----------------------------------------------------------------------------
    Performs addition between current location and a displacement. It returns
    the result HGF2DLocation object.

    @param pi_rOffset IN A constant reference to an HGF2DDisplacement object
                         containing the displacement vector.

    @return The new HGF2DLocation object resulting from addition

    @see operator+=()
    @see operator-()
    -----------------------------------------------------------------------------
*/
HGF2DLocation HGF2DLocation::operator+(const HGF2DDisplacement& pi_rOffset) const
    {
    return (HGF2DLocation(GetX() + pi_rOffset.GetDeltaX(),
                          GetY() + pi_rOffset.GetDeltaY(),
                          m_pCoordSys));
    }

//-----------------------------------------------------------------------------
// operator+ (friend function)
// Returns a new location object that represent the location moved
// by offset specified in left operand displacement.
//-----------------------------------------------------------------------------
HGF2DLocation ImagePP::operator+(const HGF2DDisplacement& pi_rOffset, const HGF2DLocation& pi_rLocation)
    {
    return (HGF2DLocation(pi_rLocation.GetX() + pi_rOffset.GetDeltaX(),
                          pi_rLocation.GetY() + pi_rOffset.GetDeltaY(),
                          pi_rLocation.m_pCoordSys));
    }


/** -----------------------------------------------------------------------------
    Performs subtraction between current location and a displacement. It returns
    the result HGF2DLocation object.

    @param pi_rOffset IN A constant reference to an HGF2DDisplacement object
                         containing the displacement vector.

    @return The new HGF2DLocation object resulting from substraction

    @see operator-=()
    @see operator+()
    -----------------------------------------------------------------------------
*/
HGF2DLocation HGF2DLocation::operator-(const HGF2DDisplacement& pi_rOffset) const
    {
    return (HGF2DLocation(GetX() - pi_rOffset.GetDeltaX(),
                          GetY() - pi_rOffset.GetDeltaY(),
                          m_pCoordSys));
    }


/** -----------------------------------------------------------------------------
    Performs subtraction between two locations. It returns the result
    HGF2DDisplacement object. If the two locations are not interpreted in the
    same coordinate system, then the displacement will be evaluated in the left
    operand coordinate system.

    @param pi_rObj IN A constant reference to an HGF2DLocation object containing
                      the other location

    @return The new HGF2DDisplacement object resulting from substraction.

    @see operator+()
    -----------------------------------------------------------------------------
*/
HGF2DDisplacement HGF2DLocation::operator-(const HGF2DLocation& pi_rObj) const
    {
    double NewX;
    double NewY;

    m_pCoordSys->ConvertFrom(pi_rObj.m_pCoordSys, pi_rObj.m_XValue, pi_rObj.m_YValue,
                             &NewX, &NewY);

    return HGF2DDisplacement(m_XValue - NewX, m_YValue - NewY);
    }


/** -----------------------------------------------------------------------------
    Adds the given displacement to the HGF2DLocation object.

    @param pi_rOffset IN A constant reference to an HGF2DDisplacement object
                        containing the displacement vector.

    @return Reference to self to be used as an l-value.

    @see operator+()
    @see operator-=()
    -----------------------------------------------------------------------------
*/
HGF2DLocation& HGF2DLocation::operator+=(const HGF2DDisplacement& pi_rOffset)
    {
    // Evaluate new position;
    m_XValue += pi_rOffset.GetDeltaX();
    m_YValue += pi_rOffset.GetDeltaY();

    return (*this);
    }

/** -----------------------------------------------------------------------------
    Subtracts the given displacement from the HGF2DLocation object.

    @param pi_rOffset IN A constant reference to an HGF2DDisplacement object
                        containing the displacement vector.

    @return Reference to self to be used as an l-value.

    @see operator-()
    @see operator+=()
    -----------------------------------------------------------------------------
*/
HGF2DLocation& HGF2DLocation::operator-=(const HGF2DDisplacement& pi_rOffset)
    {
    // Evaluate new position;
    m_XValue -= pi_rOffset.GetDeltaX();
    m_YValue -= pi_rOffset.GetDeltaY();

    return (*this);
    }



