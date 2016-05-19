//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DLiteExtent.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>



// The class declaration must be the last include file.
#include <Imagepp/all/h/HGF2DLiteExtent.h>

// VC8 support
// link warning LNK4221
// because all method are inline, no public symbol was found into the object file
void HGF2DLiteExtent::DummyMethod() const
    {
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
bool HGF2DLiteExtent::Contains(const HGF2DLiteExtent& pi_rExtent) const
    {
    HPRECONDITION(pi_rExtent.m_initializedXMin);
    HPRECONDITION(pi_rExtent.m_initializedYMin);
    HPRECONDITION(pi_rExtent.m_initializedXMax);
    HPRECONDITION(pi_rExtent.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    bool   Result;

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
bool HGF2DLiteExtent::InnerContains(const HGF2DLiteExtent& pi_rExtent) const
    {
    HPRECONDITION(pi_rExtent.m_initializedXMin);
    HPRECONDITION(pi_rExtent.m_initializedYMin);
    HPRECONDITION(pi_rExtent.m_initializedXMax);
    HPRECONDITION(pi_rExtent.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    bool   Result;

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
bool HGF2DLiteExtent::InnerContains(const HGF2DLiteExtent& pi_rExtent,
                                 double            pi_Epsilon) const
    {
    HPRECONDITION(pi_rExtent.m_initializedXMin);
    HPRECONDITION(pi_rExtent.m_initializedYMin);
    HPRECONDITION(pi_rExtent.m_initializedXMax);
    HPRECONDITION(pi_rExtent.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    bool   Result;

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
bool HGF2DLiteExtent::OuterContains(const HGF2DLiteExtent& pi_rExtent) const
    {
    HPRECONDITION(pi_rExtent.m_initializedXMin);
    HPRECONDITION(pi_rExtent.m_initializedYMin);
    HPRECONDITION(pi_rExtent.m_initializedXMax);
    HPRECONDITION(pi_rExtent.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    bool   Result;

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
bool HGF2DLiteExtent::OuterContains(const HGF2DLiteExtent& pi_rExtent,
                                 double            pi_Epsilon) const
    {
    HPRECONDITION(pi_rExtent.m_initializedXMin);
    HPRECONDITION(pi_rExtent.m_initializedYMin);
    HPRECONDITION(pi_rExtent.m_initializedXMax);
    HPRECONDITION(pi_rExtent.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    bool   Result;

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

    return (Result);
    }

/** -----------------------------------------------------------------------------
    Intersect method. When performing intersect of two extents, the result is
    the smallest extent that is enclosed by both operands.

    If either extents is undefined, then the result extent is also undefined.

    @param pi_rExtent IN Constant reference to extent object to intersect with.
    -----------------------------------------------------------------------------
*/
void HGF2DLiteExtent::Intersect(const HGF2DLiteExtent& pi_rExtent)
    {
    HPRECONDITION(pi_rExtent.m_initializedXMin);
    HPRECONDITION(pi_rExtent.m_initializedYMin);
    HPRECONDITION(pi_rExtent.m_initializedXMax);
    HPRECONDITION(pi_rExtent.m_initializedYMax);
    HPRECONDITION(m_initializedXMin);
    HPRECONDITION(m_initializedXMax);
    HPRECONDITION(m_initializedYMin);
    HPRECONDITION(m_initializedYMax);

    if (IsDefined() && pi_rExtent.IsDefined())
        {
        // Both extents are defined ...
        // Make copy of given extent
        HGF2DLiteExtent TempExtent (pi_rExtent);

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
            m_XMin = m_XMax;
            m_YMin = m_YMax;

            m_initializedXMin = false;
            m_initializedXMax = false;
            m_initializedYMin = false;
            m_initializedYMax = false;
            }
        }
    else
        {
        // One or both extents is not defined ... intersect is not defined
        m_XMin = m_XMax;
        m_YMin = m_YMax;
        m_initializedXMin = false;
        m_initializedXMax = false;
        m_initializedYMin = false;
        m_initializedYMax = false;
        }
    }
