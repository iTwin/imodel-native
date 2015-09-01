//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DExtent.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGF2DExtent
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DLocation.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HGF2DRectangle.h>

#include <Imagepp/all/h/HGF2DExtent.h>


/** -----------------------------------------------------------------------------
    Default constructor

    The extent coordinates are undefined after such construction.
    -----------------------------------------------------------------------------
*/
HGF2DExtent::HGF2DExtent ()
    : m_XDefined(false),
      m_YDefined(false),
      m_pCoordSys(new HGF2DCoordSys())
    {
    }

/** -----------------------------------------------------------------------------
    Constructor
    Constructed from the coordinates of an origin and a corner, wherever they are
    (both locations must use the same HGF2DCoordSys object).

    At all times, the XMin coordinate of the extent must be smaller or equal to
    XMax, and at all times YMin must be smaller or equal to YMax.

    @param pi_rOrigin IN Constant reference to a location object describing the
                          position of the origin (Xmin, Ymin pair) of extent.

    @param pi_rCorner IN Constant reference to a location object describing the
                         position of the corner (Xmax, Ymax pair) of extent.

    -----------------------------------------------------------------------------
*/
HGF2DExtent::HGF2DExtent(const HGF2DLocation& pi_rOrigin,
                         const HGF2DLocation& pi_rCorner)
    : m_XDefined(true),
      m_YDefined(true),
      m_pCoordSys(pi_rOrigin.GetCoordSys())
    {
    // The origin must contain lower or equal values to corner
    HPRECONDITION(pi_rOrigin.GetX() <= pi_rCorner.ExpressedIn(pi_rOrigin.GetCoordSys()).GetX());
    HPRECONDITION(pi_rOrigin.GetY() <= pi_rCorner.ExpressedIn(pi_rOrigin.GetCoordSys()).GetY());

    m_XMin = pi_rOrigin.GetX();
    m_YMin = pi_rOrigin.GetY();

    HGF2DLocation TempPoint(pi_rCorner, pi_rOrigin.GetCoordSys());

    m_XMax = TempPoint.GetX();
    m_YMax = TempPoint.GetY();
    }


//-----------------------------------------------------------------------------
// Serialization Constructor
//-----------------------------------------------------------------------------
HGF2DExtent::HGF2DExtent(string&                      pi_rSerializationString,
                         const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : m_XDefined(true),
      m_YDefined(true),
      m_pCoordSys(pi_rpCoordSys)
    {
    HPRECONDITION(pi_rSerializationString.size() == GetSerializationStrSize());

    m_XMin = *(double*)pi_rSerializationString.substr(0, sizeof(double)).c_str();
    m_XMax = *(double*)pi_rSerializationString.substr(sizeof(double), sizeof(double)).c_str();
    m_YMin = *(double*)pi_rSerializationString.substr(sizeof(double) * 2, sizeof(double)).c_str();
    m_YMax = *(double*)pi_rSerializationString.substr(sizeof(double) * 3, sizeof(double)).c_str();
    }

//-----------------------------------------------------------------------------
// Assignment operator.
//-----------------------------------------------------------------------------
HGF2DExtent& HGF2DExtent::operator=(const HGF2DExtent& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_XMin = pi_rObj.m_XMin;
        m_XMax = pi_rObj.m_XMax;
        m_YMin = pi_rObj.m_YMin;
        m_YMax = pi_rObj.m_YMax;

        m_XDefined = pi_rObj.m_XDefined;
        m_YDefined = pi_rObj.m_YDefined;

        m_pCoordSys = pi_rObj.m_pCoordSys;
        }

    return (*this);
    }

/** -----------------------------------------------------------------------------
    Equality evaluation operator.  Two extent objects are considered "equal" if
    they map to the same physical position in either of the extent coordinate
    system.  If they do not use the same HGF2DCoordSys object, internal conversion
    occurs in such a way that compared values are expressed in the same HGF2DCoordSys.

    @param pi_rObj IN Constant reference to the extent object to compare to.

    @return Boolean value reflecting equality or inequality between objects.

    @see operator!=()
    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::operator==(const HGF2DExtent& pi_rObj) const
    {
    HGF2DExtent TempExtent (pi_rObj);

    // Convert given extent to current coordinate system
    TempExtent.ChangeCoordSys(GetCoordSys());

    // Compare origin and corners
    return((m_XDefined == TempExtent.m_XDefined) &&
           (m_YDefined == TempExtent.m_YDefined) &&
           (!m_XDefined || ((TempExtent.m_XMin == m_XMin) && (TempExtent.m_XMax == m_XMax))) &&
           (!m_YDefined || ((TempExtent.m_YMin == m_YMin) && (TempExtent.m_YMax == m_YMax))));
    }

/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two extents with
    application of a tolerance (epsilon). This method applies the global fixed
    epsilon.
    The value of the epsilon is interpreted in the units of the dimension it applies to

    @param pi_rExtent IN Constant reference to the HGF2DExtent to compare with.

    @see operator==()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::IsEqualTo(const HGF2DExtent& pi_rObj) const
    {
    HGF2DExtent TempExtent (pi_rObj);

    // Convert given extent to current coordinate system
    TempExtent.ChangeCoordSys(GetCoordSys());

    // Compare origin and corners
    return((m_XDefined == TempExtent.m_XDefined) &&
           (m_YDefined == TempExtent.m_YDefined) &&
           (!m_XDefined || (HDOUBLE_EQUAL_EPSILON(TempExtent.m_XMin, m_XMin) && HDOUBLE_EQUAL_EPSILON(TempExtent.m_XMax, m_XMax))) &&
           (!m_YDefined || (HDOUBLE_EQUAL_EPSILON(TempExtent.m_YMin, m_YMin) && HDOUBLE_EQUAL_EPSILON(TempExtent.m_YMax, m_YMax))));
    }


/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two extents with
    application of a tolerance (epsilon). This method applies the given
    epsilon.
    The value of the epsilon is interpreted in the units of the dimension it applies to

    @param pi_rExtent IN Constant reference to the HGF2DExtent to compare with.

    @param pi_Epsilon IN Tolerance to apply to compare operation. This number
                         must be greater or equal to 0.0.

    @see operator==()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::IsEqualTo(const HGF2DExtent& pi_rObj, double pi_Epsilon) const
    {
    HGF2DExtent TempExtent (pi_rObj);

    // Convert given extent to current coordinate system
    TempExtent.ChangeCoordSys(GetCoordSys());

    // Compare origin and corners
    return((m_XDefined == TempExtent.m_XDefined) &&
           (m_YDefined == TempExtent.m_YDefined) &&
           (!m_XDefined || (HDOUBLE_EQUAL(TempExtent.m_XMin, m_XMin, pi_Epsilon) && HDOUBLE_EQUAL(TempExtent.m_XMax, m_XMax, pi_Epsilon))) &&
           (!m_YDefined || (HDOUBLE_EQUAL(TempExtent.m_YMin, m_YMin, pi_Epsilon) && HDOUBLE_EQUAL(TempExtent.m_YMax, m_YMax, pi_Epsilon))));
    }




/** -----------------------------------------------------------------------------
    Sets a new value for the Xmin coordinate of this extent.
    If the extent is originally undefined, then the opposite coordinate of
    the same dimension will also be set to the same coordinate. If a value as
    been provided for both dimensions, then the extent will no longer be
    undefined. If the extent is defined, then the rules (Xmin <= Xmax)
    and Ymin <= Ymax) must be respected.

    @param pi_rNewValue IN The new value for the coordinate, expressed
                        in the current coordinate system of the extent.

    @see SetYMin()
    @see SetXMax()
    @see SetYMax()
    @see GetXMin()
    -----------------------------------------------------------------------------
*/
void HGF2DExtent::SetXMin(double pi_XMin)
    {
    HPRECONDITION(!m_XDefined || m_XMax >= pi_XMin);

    // Set XMin
    m_XMin = pi_XMin;

    // Check if X is defined
    if (!m_XDefined)
        {
        m_XMax = m_XMin;
        m_XDefined = true;
        }
    }

/** -----------------------------------------------------------------------------
    Sets a new value for the Ymin coordinate of this extent.
    If the extent is originally undefined, then the opposite coordinate of
    the same dimension will also be set to the same coordinate. If a value as
    been provided for both dimensions, then the extent will no longer be
    undefined. If the extent is defined, then the rules (Xmin <= Xmax)
    and Ymin <= Ymax) must be respected.

    @param pi_rNewValue IN The new value for the coordinate, expressed
                        in the current coordinate system of the extent.

    @see SetYXin()
    @see SetXMax()
    @see SetYMax()
    @see GetYMin()
    -----------------------------------------------------------------------------
*/
void HGF2DExtent::SetYMin(double pi_YMin)
    {
    HPRECONDITION(!m_YDefined || m_YMax >= pi_YMin);

    // Set YMin
    m_YMin = pi_YMin;

    // Check if Y is defined
    if (!m_YDefined)
        {
        m_YMax = m_YMin;
        m_YDefined = true;
        }
    }


/** -----------------------------------------------------------------------------
    Sets a new value for the Xmax coordinate of this extent.
    If the extent is originally undefined, then the opposite coordinate of
    the same dimension will also be set to the same coordinate. If a value as
    been provided for both dimensions, then the extent will no longer be
    undefined. If the extent is defined, then the rules (Xmin <= Xmax)
    and Ymin <= Ymax) must be respected.

    @param pi_rNewValue IN The new value for the coordinate, expressed
                        in the current coordinate system of the extent.

    @see SetYMin()
    @see SetXMin()
    @see SetYMax()
    @see GetXMax()
    -----------------------------------------------------------------------------
*/
void HGF2DExtent::SetXMax(double pi_XMax)
    {
    HPRECONDITION(!m_XDefined || m_XMin <= pi_XMax);

    // Set XMax
    m_XMax = pi_XMax;

    // Check if X is defined
    if (!m_XDefined)
        {
        m_XMin = m_XMax;
        m_XDefined = true;
        }
    }

/** -----------------------------------------------------------------------------
    Sets a new value for the YMax coordinate of this extent.
    If the extent is originally undefined, then the opposite coordinate of
    the same dimension will also be set to the same coordinate. If a value as
    been provided for both dimensions, then the extent will no longer be
    undefined. If the extent is defined, then the rules (Xmin <= Xmax)
    and Ymin <= Ymax) must be respected.

    @param pi_rNewValue IN The new value for the coordinate, expressed
                        in the current coordinate system of the extent.

    @see SetYMin()
    @see SetXMax()
    @see SetXMin()
    @see GetYMax()
    -----------------------------------------------------------------------------
*/
void HGF2DExtent::SetYMax(double pi_YMax)
    {
    HPRECONDITION(!m_YDefined || m_YMin <= pi_YMax);

    // Set YMax
    m_YMax = pi_YMax;

    // Check if Y is defined
    if (!m_YDefined)
        {
        m_YMin = m_YMax;
        m_YDefined = true;
        }
    }




/** -----------------------------------------------------------------------------
    Sets a new position for the origin, which represents the values of the
    (Xmin, Ymim) coordinate. If the extent is originally undefined, then
    the corner will also be set to the same coordinate. If the extent is
    defined, then the origin being the Xmin, Ymin coordinates must have
    alues smaller than or equal for both dimensions than the Xmax, Ymax pair.

    @param pi_rOrigin IN Constant reference to a location object describing the
                         new position where the origin should be.

    @see SetCorner()
    @see GetOrigin()
    -----------------------------------------------------------------------------
*/
void HGF2DExtent::SetOrigin(const HGF2DLocation& pi_rOrigin)
    {
    // The given origin must be smaller that current corner (or not be defined)
    HPRECONDITION(!m_XDefined || pi_rOrigin.ExpressedIn(m_pCoordSys).GetX() <= m_XMax);
    HPRECONDITION(!m_YDefined || pi_rOrigin.ExpressedIn(m_pCoordSys).GetY() <= m_YMax);


    HGF2DLocation MyLoc(pi_rOrigin,m_pCoordSys);

    m_XMin = MyLoc.GetX();
    m_YMin = MyLoc.GetY();

    // Check if X is defined
    if (!m_XDefined)
        {
        m_XMax = m_XMin;
        m_XDefined = true;
        }

    // Check if Y is defined
    if (!m_YDefined)
        {
        m_YMax = m_YMin;
        m_YDefined = true;
        }
    }


/** -----------------------------------------------------------------------------
    Sets a new position for the corner, which represents the values of the
    (Xmax, Ymax) coordinate. If the extent is originally undefined, then the
    origin will also be set to the same coordinate, the extent no longer being
    empty, but the width and height being NULL. If the extent is defined, then
    the corner coordinates must have values greater or equal for both
    dimensions than the Xmin, Ymin pair.

    @param pi_rCorner IN Constant reference to a location object describing the
                         new position where the corner should be.

    @see GetCorner()
    @see SetOrigin()
    -----------------------------------------------------------------------------
*/
void HGF2DExtent::SetCorner(const HGF2DLocation& pi_rCorner)
    {
    // The given corner must be greater that current origin (or not be defined)
    HPRECONDITION(!m_XDefined || pi_rCorner.ExpressedIn(m_pCoordSys).GetX() >= m_XMin);
    HPRECONDITION(!m_YDefined || pi_rCorner.ExpressedIn(m_pCoordSys).GetY() >= m_YMin);

    HGF2DLocation MyLoc(pi_rCorner,m_pCoordSys);

    m_XMax = MyLoc.GetX();
    m_YMax = MyLoc.GetY();

    // Check if X is defined
    if (!m_XDefined)
        {
        m_XMin = m_XMax;
        m_XDefined = true;
        }

    // Check if Y is defined
    if (!m_YDefined)
        {
        m_YMin = m_YMax;
        m_YDefined = true;
        }
    }



/** -----------------------------------------------------------------------------
    This method changes the coordinate system object used by this extent, trying
    to do so without changing its physical position (its position in "world").
    To maintain the physical position, actual Xmin-Ymin and Xmax-Ymax values
    of this extent are converted, because they were expressed in the old coordinate
    system, before it changed. Since an extent must be oriented along the axes
    of the coordinate system it refers to, if there is a rotation between the
    previous and the new coordinate system, or for any other reason, the axes
    are not in the same directions for both systems, then the extent will be
    increased in order to contain all of the extent defined in the previous
    coordinate system.

    Reference to the coordinate system object is kept as a smart pointer
    to it, allowing utilization of a single coordinate system object among many
    extent, points and other objects using coordinates, without having to track
    which coordinate system is used or not.

    @param pi_rpCoordSys IN A constant reference to smart pointer to a new
                            coordinate system object used by this extent.

    @see GetCoordSys()
    @see SetCoordSys()
    -----------------------------------------------------------------------------
*/
void HGF2DExtent::ChangeCoordSys (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    if (IsDefined())
        {
        if (m_pCoordSys != pi_rpCoordSys)
            {
            HFCPtr<HGF2DTransfoModel> transfoModel = GetCoordSys()->GetTransfoModelTo(pi_rpCoordSys);

            // When changing the coord sys of an extent using a transformation that prevserves linearity, 
            // the four corner of the original extent stays the four corners of the transformed extent. 
            // This isn't true for non-linear transforms,
            // where the sides of the extent can curves and get out of the zone delimited by the four transformed
            // corners of the original extent. To solve this problem, a shape HGF2DShape is used. This class
            // supports non linear transforms.
            if (transfoModel->PreservesLinearity())
                {
                // Compute coordinates of four corners
                HGF2DLocation BottomLeft(m_XMin, m_YMin, m_pCoordSys);
                HGF2DLocation TopLeft(m_XMin, m_YMax, m_pCoordSys);
                HGF2DLocation TopRight(m_XMax, m_YMax, m_pCoordSys);
                HGF2DLocation BottomRight(m_XMax, m_YMin, m_pCoordSys);

                // Change coordinate sytem for these four points
                BottomLeft.ChangeCoordSys (pi_rpCoordSys);
                TopLeft.ChangeCoordSys (pi_rpCoordSys);
                TopRight.ChangeCoordSys (pi_rpCoordSys);
                BottomRight.ChangeCoordSys (pi_rpCoordSys);

                // Set coordinate system of extent
                SetCoordSys (pi_rpCoordSys);

                // Reset extent
                m_XDefined = false;
                m_YDefined = false;

                // Add location to empty extent
                Add(BottomLeft);
                Add(TopLeft);
                Add(TopRight);
                Add(BottomRight);
                }
            else
                {
                HGF2DRectangle currentExtent(GetXMin(), GetYMin(), GetXMax(), GetYMax());
                HFCPtr<HGF2DShape> resultShape = currentExtent.AllocTransformDirect(*transfoModel);
                HGF2DLiteExtent finalExtent = resultShape->GetExtent();

                // Set coordinate system of extent
                SetCoordSys (pi_rpCoordSys);

                // Reset extent
                m_XDefined = false;
                m_YDefined = false;

                HGF2DLocation BottomLeft(finalExtent.GetXMin(), finalExtent.GetYMin(), pi_rpCoordSys);
                HGF2DLocation TopLeft(finalExtent.GetXMin(), finalExtent.GetYMax(), pi_rpCoordSys);
                HGF2DLocation TopRight(finalExtent.GetXMax(), finalExtent.GetYMax(), pi_rpCoordSys);
                HGF2DLocation BottomRight(finalExtent.GetXMax(), finalExtent.GetYMin(), pi_rpCoordSys);

                // Add location to empty extent
                Add(BottomLeft);
                Add(TopLeft);
                Add(TopRight);
                Add(BottomRight);
                }
            }
        }
    else
        {
        // Set coordinate system of extent
        SetCoordSys (pi_rpCoordSys);

        // Reset extent
        m_XDefined = false;
        m_YDefined = false;
        }
    }





/** -----------------------------------------------------------------------------
    Displacement operation. This operation of addition with an HGF2DDisplacement
    permit to apply a translation in its own coordinate system to the extent,
    and thus result in a new extent object.

    @param pi_rOffset IN Constant reference to an HGF2DDisplacement object to
                         expressing the displacement.

    @return The new extent.

    @see operator-()
    @see operator+=()
    -----------------------------------------------------------------------------
*/
HGF2DExtent HGF2DExtent::operator+(const HGF2DDisplacement& pi_rOffset) const
    {
    return (HGF2DExtent(m_XMin +pi_rOffset.GetDeltaX(),
                        m_YMin + pi_rOffset.GetDeltaY(),
                        m_XMax + pi_rOffset.GetDeltaX(),
                        m_YMax + pi_rOffset.GetDeltaY(),
                        m_pCoordSys));
    }


/** -----------------------------------------------------------------------------
    Displacement operation. This operation of subtraction with an HGF2DDisplacement
    permit to apply a translation in its own coordinate system to the extent,
    and thus result in a new extent object.

    @param pi_rOffset IN Constant reference to an HGF2DDisplacement object to
                         expressing the displacement.

    @return The new extent.

    @see operator+()
    @see operator-=()
    -----------------------------------------------------------------------------
*/
HGF2DExtent HGF2DExtent::operator-(const HGF2DDisplacement& pi_rOffset) const
    {
    return (HGF2DExtent(m_XMin - pi_rOffset.GetDeltaX(),
                        m_YMin - pi_rOffset.GetDeltaY(),
                        m_XMax - pi_rOffset.GetDeltaX(),
                        m_YMax - pi_rOffset.GetDeltaY(),
                        m_pCoordSys));
    }



/** -----------------------------------------------------------------------------
    Extent increase operator.  When adding an extent with a location, the extent
    resulting from the operation is the smallest extent that will enclose both
    the previous extent and the location point added. If the extent was
    previously empty, then the result extent will only contain the given point ,
    but will no longer be undefined.

    If the HGF2DCoordSys object used by each operand is not the same, the
    extent coordinate system is used and internal conversion occurs while
    calculating new coordinates.

    @param pi_rLocation IN Constant reference to a location object containing
                           the location to add.
    -----------------------------------------------------------------------------
*/
void HGF2DExtent::Add(const HGF2DLocation& pi_rLocation)
    {
    if (m_pCoordSys == pi_rLocation.GetCoordSys())
        {
        // Process X Coordinate
        if (!m_XDefined)
            {
            // X not defined yet ... set
            m_XMin = pi_rLocation.GetX();
            m_XMax = m_XMin;
            m_XDefined = true;
            }
        else
            {
            if (m_XMin > pi_rLocation.GetX())
                m_XMin = pi_rLocation.GetX();

            if (m_XMax < pi_rLocation.GetX())
                m_XMax = pi_rLocation.GetX();

            }

        // Process Y Coordinate
        if (!m_YDefined)
            {
            // Y not defined yet ... set
            m_YMin = pi_rLocation.GetY();
            m_YMax = m_YMin;
            m_YDefined = true;
            }
        else
            {
            if (m_YMin > pi_rLocation.GetY())
                m_YMin = pi_rLocation.GetY();

            if (m_YMax < pi_rLocation.GetY())
                m_YMax = pi_rLocation.GetY();

            }
        }
    else
        {
        Add(pi_rLocation.ExpressedIn(m_pCoordSys));
        }

    }


/** -----------------------------------------------------------------------------
    Extent increase operator.  When adding an extent with another extent, the extent
    resulting from the operation is the smallest extent that will enclose both
    the previous extent and the extent added. If the extent was
    previously empty, then the result extent will only contain the given extent,
    but will no longer be undefined.

    If the HGF2DCoordSys object used by each operand is not the same, the
    extent coordinate system is used and internal conversion occurs while
    calculating new coordinates.

    @param pi_rExtent IN Constant reference to a extent object that is used
                         for source coordinates
    -----------------------------------------------------------------------------
*/
void HGF2DExtent::Add (const HGF2DExtent& pi_rExtent)
    {
    if (m_pCoordSys == pi_rExtent.m_pCoordSys)
        {
        // Combine extents
        // If X not defined yet but defined in given
        if ((!m_XDefined) && (pi_rExtent.m_XDefined))
            {
            // X not defined yet ... set
            m_XMin = pi_rExtent.m_XMin;
            m_XMax = pi_rExtent.m_XMax;
            m_XDefined = true;
            }
        else if ((m_XDefined) && (pi_rExtent.m_XDefined))
            {
            // X dims of both extents are defined
            if (m_XMin > pi_rExtent.m_XMin)
                m_XMin = pi_rExtent.m_XMin;

            if (m_XMax < pi_rExtent.m_XMax)
                m_XMax = pi_rExtent.m_XMax;
            }

        // If Y not defined yet but defined in given
        if ((!m_YDefined) && (pi_rExtent.m_YDefined))
            {
            // X not defined yet ... set
            m_YMin = pi_rExtent.m_YMin;
            m_YMax = pi_rExtent.m_YMax;
            m_YDefined = true;
            }
        else if ((m_YDefined) && (pi_rExtent.m_YDefined))
            {
            // X dims of both extents are defined
            if (m_YMin > pi_rExtent.m_YMin)
                m_YMin = pi_rExtent.m_YMin;

            if (m_YMax < pi_rExtent.m_YMax)
                m_YMax = pi_rExtent.m_YMax;
            }

        }
    else
        {
        // Make copy of given extent
        HGF2DExtent TempExtent (pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys (m_pCoordSys);

        // Combine extents
        // If X not defined yet but defined in given
        if ((!m_XDefined) && (TempExtent.m_XDefined))
            {
            // X not defined yet ... set
            m_XMin = TempExtent.m_XMin;
            m_XMax = TempExtent.m_XMax;
            m_XDefined = true;
            }
        else if ((m_XDefined) && (TempExtent.m_XDefined))
            {
            // X dims of both extents are defined
            if (m_XMin > TempExtent.m_XMin)
                m_XMin = TempExtent.m_XMin;

            if (m_XMax < TempExtent.m_XMax)
                m_XMax = TempExtent.m_XMax;
            }

        // If Y not defined yet but defined in given
        if ((!m_YDefined) && (TempExtent.m_YDefined))
            {
            // X not defined yet ... set
            m_YMin = TempExtent.m_YMin;
            m_YMax = TempExtent.m_YMax;
            m_YDefined = true;
            }
        else if ((m_YDefined) && (TempExtent.m_YDefined))
            {
            // X dims of both extents are defined
            if (m_YMin > TempExtent.m_YMin)
                m_YMin = TempExtent.m_YMin;

            if (m_YMax < TempExtent.m_YMax)
                m_YMax = TempExtent.m_YMax;
            }
        }

    }


/** -----------------------------------------------------------------------------
    Intersect method. When performing intersect of two extents, the result is
    the smallest extent that is enclosed by both operands, having different
    HGF2DCoordSys objects or not.

    If the HGF2DCoordSys object used by each operand is not the same, internal
    conversion occurs.
    If either extents is undefined, then the result extent is also undefined.

    @param pi_rExtent IN Constant reference to extent object to intersect with.
    -----------------------------------------------------------------------------
*/
void HGF2DExtent::Intersect(const HGF2DExtent& pi_rExtent)
    {
    if (IsDefined() && pi_rExtent.IsDefined())
        {
        // Both extents are defined ...
        // Make copy of given extent
        HGF2DExtent TempExtent (pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys(m_pCoordSys);

        if (m_XMin < TempExtent.m_XMin)
            m_XMin = TempExtent.m_XMin;

        if (m_XMax > TempExtent.m_XMax)
            m_XMax = TempExtent.m_XMax;

        if (m_YMin < TempExtent.m_YMin)
            m_YMin = TempExtent.m_YMin;

        if (m_YMax > TempExtent.m_YMax)
            m_YMax = TempExtent.m_YMax;

        // If after these operations, the extent is invalid, then there is no intersection
        // and result is not defined
        if ((m_XMin > m_XMax) || (m_YMin > m_YMax))
            {
            m_XDefined = false;
            m_YDefined = false;
            }
        }
    else
        {
        // One or both extents is not defined ... intersect is not defined
        m_XDefined = false;
        m_YDefined = false;
        }
    }

/** -----------------------------------------------------------------------------
    This method returns true if the two extent object share any part of each
    other (If their intersection is not empty).

    If the HGF2DCoordSys object used by each operand is not the same, the
    left extent coordinate system is used and internal conversion occurs while
    calculating new coordinates..
    If either extent is empty then the method returns false.

    @param pi_rExtent IN Constant reference to extent object to check overlap with.

    @see InnerOverlaps()
    @see OutterOverlaps()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::DoTheyOverlap(const HGF2DExtent& pi_rExtent) const
    {
    bool   Result;

    if (m_pCoordSys != pi_rExtent.m_pCoordSys)
        {
        // Make copy of given extent
        HGF2DExtent TempExtent (pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys (m_pCoordSys);

        Result = DoTheyOverlap(TempExtent);
        }
    else
        {
        if (IsDefined() && pi_rExtent.IsDefined())
            {
            // Both extents are defined ... may overlap
            // The second part is required for horizontal or horizontal objects (0 thickness)
            // NOTE that two null sized extents may not overlap
            Result = ((m_XMax > pi_rExtent.m_XMin) &&
                      (m_XMin < pi_rExtent.m_XMax) &&
                      (m_YMax > pi_rExtent.m_YMin) &&
                      (m_YMin < pi_rExtent.m_YMax)
                     );
            }
        else
            {
            // At least one is not defined ... cannot overlap
            Result = false;
            }
        }

    return (Result);
    }


/** -----------------------------------------------------------------------------
    This method returns true if the two extent object share any part of each
    other (If their intersection is not empty).The OutterOverlap methods increases
    the self extent by an epsilon (HGLOBAL_EPSILON or given) before performing
    operation.

    If the HGF2DCoordSys object used by each operand is not the same, the
    left extent coordinate system is used and internal conversion occurs while
    calculating new coordinates..
    If either extent is empty then the method returns false.

    @param pi_rExtent IN Constant reference to extent object to check overlap with.

    @see InnerOverlaps()
    @see DoTheyOverlap()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::OutterOverlaps(const HGF2DExtent& pi_rExtent) const
    {
    bool   Result;

    if (m_pCoordSys != pi_rExtent.m_pCoordSys)
        {
        // Make copy of given extent
        HGF2DExtent TempExtent (pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys (m_pCoordSys);

        Result = OutterOverlaps(TempExtent);
        }
    else
        {
        if (IsDefined() && pi_rExtent.IsDefined())
            {
            // Both extents are defined ... may overlap

            // The second part is required for horizontal or horizontal objects (0 thickness)
            // NOTE that two null sized extents may not overlap
            Result = (HDOUBLE_GREATER_OR_EQUAL_EPSILON(m_XMax, pi_rExtent.m_XMin) &&
                      HDOUBLE_SMALLER_OR_EQUAL_EPSILON(m_XMin, pi_rExtent.m_XMax) &&
                      HDOUBLE_GREATER_OR_EQUAL_EPSILON(m_YMax, pi_rExtent.m_YMin) &&
                      HDOUBLE_SMALLER_OR_EQUAL_EPSILON(m_YMin, pi_rExtent.m_YMax)
                     );
            }
        else
            {
            // At least one is not defined ... cannot overlap
            Result = false;
            }
        }

    return (Result);
    }


/** -----------------------------------------------------------------------------
    This method returns true if the two extent object share any part of each
    other (If their intersection is not empty).The OutterOverlap methods increases
    the self extent by an epsilon (given) before performing operation.

    If the HGF2DCoordSys object used by each operand is not the same, the
    left extent coordinate system is used and internal conversion occurs while
    calculating new coordinates..
    If either extent is empty then the method returns false.

    @param pi_rExtent IN Constant reference to extent object to check overlap with.

    @see InnerOverlaps()
    @see DoTheyOverlap()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::OutterOverlaps(const HGF2DExtent& pi_rExtent,
                                 double pi_Epsilon) const
    {
    bool   Result;

    if (m_pCoordSys != pi_rExtent.m_pCoordSys)
        {
        // Make copy of given extent
        HGF2DExtent TempExtent(pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys (m_pCoordSys);

        Result = OutterOverlaps(TempExtent, pi_Epsilon);
        }
    else
        {
        if (IsDefined() && pi_rExtent.IsDefined())
            {
            // Both extents are defined ... may overlap

            // The second part is required for horizontal or horizontal objects (0 thickness)
            // NOTE that two null sized extents may not overlap
            Result = (HDOUBLE_GREATER_OR_EQUAL(m_XMax, pi_rExtent.m_XMin, pi_Epsilon) &&
                      HDOUBLE_SMALLER_OR_EQUAL(m_XMin, pi_rExtent.m_XMax, pi_Epsilon) &&
                      HDOUBLE_GREATER_OR_EQUAL(m_YMax, pi_rExtent.m_YMin, pi_Epsilon) &&
                      HDOUBLE_SMALLER_OR_EQUAL(m_YMin, pi_rExtent.m_YMax, pi_Epsilon)
                     );
            }
        else
            {
            // At least one is not defined ... cannot overlap
            Result = false;
            }
        }

    return (Result);
    }


/** -----------------------------------------------------------------------------
    This method returns true if the two extent object share any part of each
    other (If their intersection is not empty).
    The InnerOverlaps methods removes on all side of the self extent an
    epsilon (HGLOBAL_EPSILON).

    If the HGF2DCoordSys object used by each operand is not the same, the
    left extent coordinate system is used and internal conversion occurs while
    calculating new coordinates..
    If either extent is empty then the method returns false.

    @param pi_rExtent IN Constant reference to extent object to check overlap with.

    @see OutterOverlaps()
    @see DoTheyOverlap()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::InnerOverlaps(const HGF2DExtent& pi_rExtent) const
    {
    bool   Result;

    if (m_pCoordSys != pi_rExtent.m_pCoordSys)
        {
        // Make copy of given extent
        HGF2DExtent TempExtent (pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys (m_pCoordSys);

        Result = InnerOverlaps(TempExtent);
        }
    else
        {
        if (IsDefined() && pi_rExtent.IsDefined())
            {
            // Both extents are defined ... may overlap

            Result = (HDOUBLE_GREATER_EPSILON(m_XMax, pi_rExtent.m_XMin) &&
                      HDOUBLE_SMALLER_EPSILON(m_XMin, pi_rExtent.m_XMax) &&
                      HDOUBLE_GREATER_EPSILON(m_YMax, pi_rExtent.m_YMin) &&
                      HDOUBLE_SMALLER_EPSILON(m_YMin, pi_rExtent.m_YMax)
                     );
            }
        else
            {
            // At least one is not defined ... cannot overlap
            Result = false;
            }
        }

    return (Result);
    }


/** -----------------------------------------------------------------------------
    This method returns true if the two extent object share any part of each
    other (If their intersection is not empty).
    The InnerOverlaps methods removes on all side of the self extent an
    epsilon (given).

    If the HGF2DCoordSys object used by each operand is not the same, the
    left extent coordinate system is used and internal conversion occurs while
    calculating new coordinates..
    If either extent is empty then the method returns false.

    @param pi_rExtent IN Constant reference to extent object to check overlap with.

    @see OutterOverlaps()
    @see DoTheyOverlap()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::InnerOverlaps(const HGF2DExtent& pi_rExtent,
                                 double pi_Epsilon) const
    {
    bool   Result;

    if (m_pCoordSys != pi_rExtent.m_pCoordSys)
        {
        // Make copy of given extent
        HGF2DExtent TempExtent(pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys (m_pCoordSys);

        Result = InnerOverlaps(TempExtent, pi_Epsilon);
        }
    else
        {
        if (IsDefined() && pi_rExtent.IsDefined())
            {
            // Both extents are defined ... may overlap

            // The second part is required for horizontal or horizontal objects (0 thickness)
            // NOTE that two null sized extents may not overlap
            Result = (HDOUBLE_GREATER(m_XMax, pi_rExtent.m_XMin, pi_Epsilon) &&
                      HDOUBLE_SMALLER(m_XMin, pi_rExtent.m_XMax, pi_Epsilon) &&
                      HDOUBLE_GREATER(m_YMax, pi_rExtent.m_YMin, pi_Epsilon) &&
                      HDOUBLE_SMALLER(m_YMin, pi_rExtent.m_YMax, pi_Epsilon)
                     );
            }
        else
            {
            // At least one is not defined ... cannot overlap
            Result = false;
            }
        }

    return (Result);
    }








/** -----------------------------------------------------------------------------
    Differentiate method. When performing differentiation of two extents, the
    result is the smallest extent that encloses the extent of the first
    operand, but not the extent of the differentiated one, having different
    HGF2DCoordSys objects or not.  Since we are dealing with extents, the
    differentiate rules are somehow bent. An extent must be square and aligned
    in the coordinate system. For this reason, the extent resulting from the operation
    will be the extent of the self extent after the given extent has been removed
    from the other.

    If the HGF2DCoordSys object used by each operand is not the same, the left
    extent coordinate system is used and internal conversion occurs while
    calculating new coordinates.
    If the first extents is undefined, then the result extent is also
    undefined. If the second extent is undefined, then the original extent is
    unchanged.

    @param pi_rExtent IN Constant reference to extent object to differentiate with.

    @see Union()
    @see Intersect()
    -----------------------------------------------------------------------------
*/
void HGF2DExtent::Differentiate(const HGF2DExtent& pi_rExtent)
    {
    if (IsDefined() && pi_rExtent.IsDefined())
        {
        // Both extents are defined ...
        // Make copy of given extent
        HGF2DExtent TempExtent (pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys (m_pCoordSys);

        if ((m_XMax <= TempExtent.m_XMax) && (m_XMin >= TempExtent.m_XMin) &&
            (m_YMax <= TempExtent.m_YMax) && (m_YMin >= TempExtent.m_YMin))
            {
            // Self is completely included in given ... undefined result
            m_XDefined = false;
            m_YDefined = false;
            }
        else
            {
            if ((m_YMax <= TempExtent.m_YMax) && (m_YMin >= TempExtent.m_YMin))
                {
                if ((m_XMax >= TempExtent.m_XMin) && (m_XMin <= TempExtent.m_XMin))
                    m_XMax=TempExtent.m_XMin;

                if ((m_XMin <= TempExtent.m_XMax) && (m_XMax >= TempExtent.m_XMax))
                    m_XMin=TempExtent.m_XMax;
                }

            if ((m_XMax <= TempExtent.m_XMax) && (m_XMin >= TempExtent.m_XMin))
                {
                if ((m_YMax >= TempExtent.m_YMin) && (m_YMin <= TempExtent.m_YMin))
                    m_YMax=TempExtent.m_YMin;

                if ((m_YMin <= TempExtent.m_YMax) && (m_YMax >= TempExtent.m_YMax))
                    m_YMin=TempExtent.m_YMax;
                }
            }

        // If after these operations, the extent is invalid, then there is no difference exist
        // and result is not defined
        if ((m_XMin > m_XMax) || (m_YMin > m_YMax))
            {
            m_XDefined = false;
            m_YDefined = false;
            }
        }
    }


/** -----------------------------------------------------------------------------
    This method checks if the given extent is completely inside self.

    If the HGF2DCoordSys object used by each operand is not the same, the left
    extent coordinate system is used and internal conversion occurs while
    calculating new coordinates.
    If the either extents are undefined, then false is returned.

    @param pi_rExtent IN Constant reference to extent object to check containement
                         with.

    @see InnerContains()
    @see OutterContains()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::Contains(const HGF2DExtent& pi_rExtent) const
    {
    bool   Result;

    if (m_pCoordSys != pi_rExtent.m_pCoordSys)
        {
        // Make copy of given extent
        HGF2DExtent TempExtent(pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys (m_pCoordSys);

        Result = Contains(TempExtent);
        }
    else
        {
        if (IsDefined() && pi_rExtent.IsDefined())
            {
            // Both extents are defined ... may be contained

            Result = ((m_XMax > pi_rExtent.m_XMax) &&
                      (m_XMin < pi_rExtent.m_XMin) &&
                      (m_YMax > pi_rExtent.m_YMax) &&
                      (m_YMin < pi_rExtent.m_YMin)
                     );
            }
        else
            {
            // At least one is not defined ... cannot be contained
            Result = false;
            }
        }

    return (Result);
    }



/** -----------------------------------------------------------------------------
    This method checks if the given extent is completely inside self within
    the default epsilon. The method does not consider that self contains given
    if the given is not at least an epsilon within the boundaries of self.

    If the HGF2DCoordSys object used by each operand is not the same, the left
    extent coordinate system is used and internal conversion occurs while
    calculating new coordinates.
    If the either extents are undefined, then false is returned.

    @param pi_rExtent IN Constant reference to extent object to check containement
                         with.

    @see Contains()
    @see OutterContains()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::InnerContains(const HGF2DExtent& pi_rExtent) const
    {
    bool   Result;

    if (m_pCoordSys != pi_rExtent.m_pCoordSys)
        {
        // Make copy of given extent
        HGF2DExtent TempExtent(pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys (m_pCoordSys);

        Result = InnerContains(TempExtent);
        }
    else
        {
        if (IsDefined() && pi_rExtent.IsDefined())
            {
            // Both extents are defined ... may be contained

            Result = (HDOUBLE_GREATER_EPSILON(m_XMax, pi_rExtent.m_XMax) &&
                      HDOUBLE_SMALLER_EPSILON(m_XMin, pi_rExtent.m_XMin) &&
                      HDOUBLE_GREATER_EPSILON(m_YMax, pi_rExtent.m_YMax) &&
                      HDOUBLE_SMALLER_EPSILON(m_YMin, pi_rExtent.m_YMin)
                     );
            }
        else
            {
            // At least one is not defined ... cannot be contained
            Result = false;
            }
        }

    return (Result);
    }


/** -----------------------------------------------------------------------------
    This method checks if the given extent is completely inside self within
    the given epsilon. The method does not consider that self contains given
    if the given is not at least an epsilon within the boundaries of self.

    If the HGF2DCoordSys object used by each operand is not the same, the left
    extent coordinate system is used and internal conversion occurs while
    calculating new coordinates.
    If the either extents are undefined, then false is returned.

    @param pi_rExtent IN Constant reference to extent object to check containement
                         with.

    @param pi_Epsilon IN The tolerance to apply. This tolerance is interpreted
                         in the X and Y dimensions of the extent coordinate system.

    @see Contains()
    @see OutterContains()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::InnerContains(const HGF2DExtent& pi_rExtent,
                                 double            pi_Epsilon) const
    {
    bool   Result;

    if (m_pCoordSys != pi_rExtent.m_pCoordSys)
        {
        // Make copy of given extent
        HGF2DExtent TempExtent(pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys (m_pCoordSys);

        Result = InnerContains(TempExtent, pi_Epsilon);
        }
    else
        {
        if (IsDefined() && pi_rExtent.IsDefined())
            {
            // Both extents are defined ... may be contained

            Result = (HDOUBLE_GREATER(m_XMax, pi_rExtent.m_XMax, pi_Epsilon) &&
                      HDOUBLE_SMALLER(m_XMin, pi_rExtent.m_XMin, pi_Epsilon) &&
                      HDOUBLE_GREATER(m_YMax, pi_rExtent.m_YMax, pi_Epsilon) &&
                      HDOUBLE_SMALLER(m_YMin, pi_rExtent.m_YMin, pi_Epsilon)
                     );
            }
        else
            {
            // At least one is not defined ... cannot be contained
            Result = false;
            }
        }

    return (Result);
    }


/** -----------------------------------------------------------------------------
    This method checks if the given extent is completely inside self within
    the default epsilon. The method considers that self contains given
    even if the given is not inside by an epsilon outside the boundaries of self

    If the HGF2DCoordSys object used by each operand is not the same, the left
    extent coordinate system is used and internal conversion occurs while
    calculating new coordinates.
    If the either extents are undefined, then false is returned.

    @param pi_rExtent IN Constant reference to extent object to check containement
                         with.

    @see Contains()
    @see InnerContains()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::OuterContains(const HGF2DExtent& pi_rExtent) const
    {
    bool   Result;

    if (m_pCoordSys != pi_rExtent.m_pCoordSys)
        {
        // Make copy of given extent
        HGF2DExtent TempExtent(pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys (m_pCoordSys);

        Result = OuterContains(TempExtent);
        }
    else
        {
        if (IsDefined() && pi_rExtent.IsDefined())
            {
            // Both extents are defined ... may be contained

            Result = (HDOUBLE_GREATER_OR_EQUAL_EPSILON(m_XMax, pi_rExtent.m_XMax) &&
                      HDOUBLE_SMALLER_OR_EQUAL_EPSILON(m_XMin, pi_rExtent.m_XMin) &&
                      HDOUBLE_GREATER_OR_EQUAL_EPSILON(m_YMax, pi_rExtent.m_YMax) &&
                      HDOUBLE_SMALLER_OR_EQUAL_EPSILON(m_YMin, pi_rExtent.m_YMin)
                     );
            }
        else
            {
            // At least one is not defined ... cannot be contained
            Result = false;
            }
        }

    return (Result);
    }


/** -----------------------------------------------------------------------------
    This method checks if the given extent is completely inside self within
    the given epsilon. The method considers that self contains given
    even if the given is not inside by an epsilon outside the boundaries of self

    If the HGF2DCoordSys object used by each operand is not the same, the left
    extent coordinate system is used and internal conversion occurs while
    calculating new coordinates.
    If the either extents are undefined, then false is returned.

    @param pi_rExtent IN Constant reference to extent object to check containement
                         with.

    @param pi_Epsilon IN The tolerance to apply. This tolerance is interpreted
                         in the X and Y dimensions of the extent coordinate system.

    @see Contains()
    @see InnerContains()
    -----------------------------------------------------------------------------
*/
bool HGF2DExtent::OuterContains(const HGF2DExtent& pi_rExtent,
                                 double            pi_Epsilon) const
    {
    bool   Result;

    if (m_pCoordSys != pi_rExtent.m_pCoordSys)
        {
        // Make copy of given extent
        HGF2DExtent TempExtent(pi_rExtent);

        // Change its coordinate system to present
        TempExtent.ChangeCoordSys (m_pCoordSys);

        Result = OuterContains(TempExtent, pi_Epsilon);
        }
    else
        {
        if (IsDefined() && pi_rExtent.IsDefined())
            {
            // Both extents are defined ... may be contained

            Result = (HDOUBLE_GREATER_OR_EQUAL(m_XMax, pi_rExtent.m_XMax, pi_Epsilon) &&
                      HDOUBLE_SMALLER_OR_EQUAL(m_XMin, pi_rExtent.m_XMin, pi_Epsilon) &&
                      HDOUBLE_GREATER_OR_EQUAL(m_YMax, pi_rExtent.m_YMax, pi_Epsilon) &&
                      HDOUBLE_SMALLER_OR_EQUAL(m_YMin, pi_rExtent.m_YMin, pi_Epsilon)
                     );
            }
        else
            {
            // At least one is not defined ... cannot be contained
            Result = false;
            }
        }

    return (Result);
    }



/** ---------------------------------------------------------------------------------------------------------
    The present method will compute an approximative extent based on the current extent and the provided
    coordinate system. The extent returned will not necessarily contain all of the original extent but
    instead attempt to preserve the original extent location (center point) and area. Some effort is
    also made to preserve the aspect ratio of the extent if possible, depending greatly on
    the relation between provided coordinate system and coordinate system of self.
    @end

    @param pi_rpCoordSys : A reference to smart pointer to coordinate system to obtain extent in
    @end

    ----------------------------------------------------------------------------------------------------------
*/
HGF2DExtent HGF2DExtent::CalculateApproxExtentIn(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    HGF2DExtent ResultExtent(pi_rpCoordSys);

    // Check if self is defined
    if (IsDefined())
        {
        // Obtain the corners of self as location
        HGF2DLocation LowerLeftCorner(GetXMin(), GetYMin(), GetCoordSys());
        HGF2DLocation UpperLeftCorner(GetXMin(), GetYMax(), GetCoordSys());
        HGF2DLocation UpperRightCorner(GetXMax(), GetYMax(), GetCoordSys());
        HGF2DLocation LowerRightCorner(GetXMax(), GetYMin(), GetCoordSys());

        // Also prepare a location for center of extent
        HGF2DLocation Center((GetXMax() + GetXMin()) / 2.0, (GetYMax() + GetYMin()) / 2.0, GetCoordSys());

        // Change the coordinates of corners to destination coordinate system
        LowerLeftCorner.ChangeCoordSys(pi_rpCoordSys);
        UpperLeftCorner.ChangeCoordSys(pi_rpCoordSys);
        UpperRightCorner.ChangeCoordSys(pi_rpCoordSys);
        LowerRightCorner.ChangeCoordSys(pi_rpCoordSys);

        Center.ChangeCoordSys(pi_rpCoordSys);

        // Obtain the size of distances in this coordinate system
        double XLower = (LowerLeftCorner - LowerRightCorner).CalculateLength();
        double XUpper = (UpperLeftCorner - UpperRightCorner).CalculateLength();
        double YLeft  = (LowerLeftCorner - UpperLeftCorner).CalculateLength();
        double YRight = (LowerRightCorner - UpperRightCorner).CalculateLength();

        // Calculate the average sizes
        double AverageX = (XLower + XUpper) / 4.0;
        double AverageY = (YLeft + YRight) / 4.0;

        // Set new extent
        ResultExtent = HGF2DExtent(Center.GetX() - AverageX,
                                   Center.GetY() - AverageY,
                                   Center.GetX() + AverageX,
                                   Center.GetY() + AverageY,
                                   pi_rpCoordSys);
        }

    return(ResultExtent);
    }



/** ---------------------------------------------------------------------------------------------------------

    ----------------------------------------------------------------------------------------------------------
*/
string HGF2DExtent::Serialize()
    {
    char   TempBuffer[100];
    double ExtentValue;
    string  SerializationStr;

    //X Min
    ExtentValue = GetXMin();

    memcpy(TempBuffer,
           (char*)&ExtentValue,
           sizeof(double));

    TempBuffer[sizeof(double)] = '\0';

    SerializationStr.insert(SerializationStr.size(), TempBuffer, sizeof(double));

    //X Max
    ExtentValue = GetXMax();

    memcpy(TempBuffer,
           (char*)&ExtentValue,
           sizeof(double));

    SerializationStr.insert(SerializationStr.size(), TempBuffer, sizeof(double));

    //Y Min
    ExtentValue = GetYMin();

    memcpy(TempBuffer,
           (char*)&ExtentValue,
           sizeof(double));

    SerializationStr.insert(SerializationStr.size(), TempBuffer, sizeof(double));

    //Y Max
    ExtentValue = GetYMax();

    memcpy(TempBuffer,
           (char*)&ExtentValue,
           sizeof(double));

    SerializationStr.insert(SerializationStr.size(), TempBuffer, sizeof(double));

    return SerializationStr;
    }

/** ---------------------------------------------------------------------------------------------------------

    ----------------------------------------------------------------------------------------------------------
*/
uint32_t HGF2DExtent::GetSerializationStrSize()
    {
    return sizeof(double) * 4;
    }
