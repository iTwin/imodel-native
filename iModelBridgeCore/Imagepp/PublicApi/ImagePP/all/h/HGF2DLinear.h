//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLinear.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DLinear
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DVector.h"
#include "HGF2DDisplacement.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 
                           
    This abstract class encapsulates all attributes common to all vector
    linear elements. Linear elements are here defined as an element representing
    a continuous line in a 2D space.

    To each and every linear, whatever their specific type, there is only one
    start point, one end point, and a single path in the 2D space linking these
    extremity points one to the other.

    Since a linear is also a vector, then it possesses two arbitrary direction
    HGF2DVector::ALPHA and HGF2DVector::BETA used to define direction along
    its path. For all linear elements, the ALPHA direction is always toward
    the start point, and the BETA direction toward the end point. At the
    extremities, the direction toward the non-definition of the linear,
    the bearing and angular acceleration are those of the previous point.

    The origin of all linear elements is by default the start point of the
    linear. This arbitrary decision enables optimization of certain
    transformations such as move, scale etc... If a descending class deviates
    from this behavior, the appropriate methods must also be overridden.

    There are important types of linear, which may affect the operation of
    some methods. These are the irregular linear elements. These linear
    elements are characterized by some of their definition path
    crossing/flirting/being contiguous to their own path. Locations shared
    by many parts of the linear will result in strange behavior for relative
    position oriented operations (Shorten(), CalculateRelativePosition() etc...).
    As a general rule, irregular linear elements should be avoided since
    they can be annoying.

    The properties and behavior common to all linears is thouroughly explained
    in the "HMR Vector Handbook". Please refer to this document for the details
    of linear implementation.

    -----------------------------------------------------------------------------
*/
class HNOVTABLEINIT HGF2DLinear : public HGF2DVector
    {

    HDECLARE_CLASS_ID(HGF2DLinearId_Base, HGF2DVector)

public:

    enum EndPointProcessing
        {
        INCLUDE_END_POINT,
        EXCLUDE_END_POINT
        };

    // Primary methods
    HGF2DLinear ();
    HGF2DLinear (const HGF2DPosition& pi_rStartPoint,
               const HGF2DPosition& pi_rEndPoint);
    HGF2DLinear (const HGF2DLinear&  pi_rObject);
    virtual            ~HGF2DLinear();

    HGF2DLinear&       operator=(const HGF2DLinear& pi_rObj);

    const HGF2DPosition&   GetStartPoint() const;
    const HGF2DPosition&   GetEndPoint() const;

    // Linear classification
    /** -----------------------------------------------------------------------------
        This method determines of the vector is a basic linear (an object
        of type HHGBasicLinear)

        @return true if the vector is a basic linear, and false otherwise.

        Example:
        @code
        @end

        @see IsComplex()
        @see HGF2DBasicLinear
        @see HGFComplexLinear
        -----------------------------------------------------------------------------
    */
    virtual bool      IsABasicLinear() const = 0;

    /** -----------------------------------------------------------------------------
        This method determines of the vector is a complex
        linear (an object of type HGFComplexLinear).

        @return true if the vector is a complex linear, and false otherwise.

        Example:
        @code
        @end

        @see IsABasicLinear()
        @see HGF2DBasicLinear
        @see HGFComplexLinear
        -----------------------------------------------------------------------------
    */
    virtual bool      IsComplex() const = 0;

    // Geometric information
    /** -----------------------------------------------------------------------------
        This method returns the linear length of the HGF2DLinear object. 

        @return The length of the object.

        Example:
        @code
        HGF2DLinear    MyFirst();
        HGF2DLinear    MySecond();
        ...
        double TotalLength = MyFirst.CalculateLength() + MySecond.CalculateLength();
        @end

        @see HGF2DSegment
        -----------------------------------------------------------------------------
    */
    virtual double    CalculateLength() const = 0;

    /** -----------------------------------------------------------------------------
        This method calculates and returns the point specified by the given
        parameter. This parameter is the relative position of desired point
        based on the linear length starting at the first
        point (Relative position = 0.0) to last point (relative position 1.0).
        Therefore to obtain the mid point on linear, the relative position 0.5
        must be given. The relative position cannot be outside the
        range 0.0 to 1.0. This method will work properly with irregular linears.

        The point returned is interpreted in the same coordinate system as the linear.

        @param pi_rRelativePosition    The relative position from first point
                                    normalised to linear length. This value must
                                    be between or equal to 0.0 to 1.0.

        @return The relative point on the linear.

        Example:
        @code
        HGF2DPosition        MyFirstPoint(10, 10);
        HGF2DPosition        MySecondPoint(15, 16)
        // A segment is a kind of linear
        HGF2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
        ...
        HGF2DPosition      MyGivenPoint(12, 13);
        HGF2DLocaton MyPoint = MySeg1.CalculateRelativePoint(0.254);
        @end

        @see HGF2DCoordSys
        @see HGF2DSegment
        -----------------------------------------------------------------------------
    */
    virtual HGF2DPosition    CalculateRelativePoint(double pi_RelativePos) const = 0;


    /** -----------------------------------------------------------------------------
        This method calculates and returns the relative position of the given
        point which must be located on the linear (see IsPointOn()). The
        relative position of the point is based on the linear length
        starting at the start point (relative position = 0.0) to end
        point (relative position 1.0). Therefore if the mid point of linear is
        given, the relative position 0.5 will be returned. The relative position
        returned cannot be outside the range 0.0 to 1.0This method may not work
        as desired in the case of irregular linears. The relative position
        returned of a point shared by many parts of the linear will be the
        position closest to the start point. This method may not be called
        for null length (CalculateLength()) linears.

        @param pi_rPointOnLinear A constant reference to an HGF2DPosition to
                                 obtain relative position of. This point must
                                 be located ON the linear..

        @return The relative position on the linear.

        Example:
        @code
        HGF2DPosition        MyFirstPoint(10, 10);
        HGF2DPosition        MySecondPoint(15, 16);
        // A segment is a kind of linear
        HGF2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
        ...
        HGF2DPosition      MyGivenPoint(12, 13);
        double ThePosition = MySeg1.CalculateRelativePosition(MyGivenPoint);
        @end

        @see HGF2DSegment
        @see IsPointOn()
        @see CalculateLength()
        -----------------------------------------------------------------------------
    */
    virtual double    CalculateRelativePosition(
        const HGF2DPosition& pi_rPointOnLinear) const = 0;


    /** -----------------------------------------------------------------------------
        This method calculates and returns the area captured by a ray
        originating from the given point and ending at extremity points.
        This method provides a way of calculating the area enclosed by
        shapes built from any type of generic linears of unknown types.

        @param pi_rPoint    A constant reference to an HGF2DPosition representing the ray origin

        @return The area of the ray.

        Example:
        @code
        HGF2DPosition        MyFirstPoint(10, 10);
        HGF2DPosition        MySecondPoint(15, 16);
        // A segment is a kind of linear
        HGF2DSegment         MySeg1(MyFirstPoint, MySecondPoint);
        ...
        HGF2DPosition        MyGivenPoint(12, 13);
        double               RayArea = MySeg1.CalculateRayArea(MyGivenPoint);
        @end

        @see HGF2DCoordSys
        -----------------------------------------------------------------------------
    */
    virtual double    CalculateRayArea(const HGF2DPosition& pi_rPoint) const = 0;

    // Manipulation
    /** -----------------------------------------------------------------------------
        The Shorten(), ShortenFrom() and ShortenTo() methods permit to modify the
        linear by specification of a new start point, a new end point or
        both either by giving the point itself, or giving a relative position
        for this or these new points. The Shorten method performs shortening
        of linear by both extremities, ShortenFrom shortens from the start
        extremity, and ShortenTo from the end extremity. If the new extremity
        is specified by a location, then this location must be located ON the
        linear (IsPointOn()). If a relative position is provided, then this
        relative position must be between 0.0 and 1.0, or equal to either one.
        In the case of the Shorten() methods, the relative position of the new start
        extremity must be smaller than the end extremity. These methods may not be
        used with a NULL length linear. These methods may be used with irregular
        linears, but when specifying a location, if this location is shared by
        many parts of the linear, the corresponding location will be the
        one closest to the start point.

        @param pi_rNewStart Constant reference to the HGF2DPosition which must
                            be located on linear that becomes the new start
                            point of linear.

        @param pi_rNewEnd Constant reference to the HGF2DPosition which must
                           be located on linear that becomes the new end
                           point of linear.

        @param pi_StartRelativePosition Relative position (between 0.0
                                        and 1.0 incusively) of the new
                                        start point desired.

        @param pi_EndRelativePosition Relative position (between 0.0
                                      and 1.0 inclusively) of the new
                                      end point desired.


        Example:
        @code
        HGF2DPosition        MyFirstPoint(10, 10);
        HGF2DPosition        MySecondPoint(15, 16);
        // A segment is a kind of linear
        HGF2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
        ...
        HGF2DPosition      MyThirdPoint(12, 13);
        HGF2Dlocation        MyFourthPoint(12, -3);
        HGF2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);

        MySeg1.ShortenTo(0.5);
        MySeg1.ShortenFrom(0.2);
        MySeg2.Shorten(0.3, 0.8);
        MySeg2.ShortenFrom(HGF2DPosition(12.0, 1.0));

        // Stupid but valid
        MySeg1.Shorten(0.0, 1.0);
        MySeg1.Shorten(0.5, 0.5);
        // MySeg1 is now NULL length and may not be shortened anymore
        MySeg2.ShortenFrom(MySeg2.GetStartPoint());
        @end

        @see ShortenTo()
        @see ShortenFrom()
        -----------------------------------------------------------------------------
    */
    virtual void       Shorten(double pi_StartRelativePos,
                               double pi_EndRelativePos) = 0;
    virtual void       Shorten(const HGF2DPosition& pi_rNewStartPoint,
                               const HGF2DPosition& pi_rNewEndPoint) = 0;

    /** -----------------------------------------------------------------------------
        The ShortenTo() methods permit to modify the linear by specification
        of a new end point.
        The new end location must be located ON the linear (IsPointOn()).
        This method may not be used with a NULL length linear.
        These methods may be used with irregular linears, but when
        specifying a location, if this location is shared by
        many parts of the linear, the corresponding location will be the
        one closest to the start point.

        @param pi_rNewEnd Constant reference to the HGF2DPosition which must
                           be located on linear that becomes the new end
                           point of linear.


        @see Shorten for examples

        @see Shorten
        @see IsPointOn()
        -----------------------------------------------------------------------------
    */
    virtual void       ShortenTo(const HGF2DPosition& pi_rNewEndPoint) = 0;

    /** -----------------------------------------------------------------------------
        The ShortenTo() methods permit to modify the linear by specification
        of a new end point by a relative position. This relative position
        must be between 0.0 and 1.0, or equal to either one.
        These methods may not be used with a NULL length linear.
        These methods may be used with irregular linears.

        @param pi_EndRelativePosition Relative position (between 0.0
                                      and 1.0 inclusively) of the new
                                      end point desired.


        @see Shorten for examples

        @see Shorten()
        -----------------------------------------------------------------------------
    */
    virtual void       ShortenTo(double pi_EndRelativePosition) = 0;

    /** -----------------------------------------------------------------------------
        The ShortenFrom() permits to modify the linear by specification of a
        new start point.
        The new start location must be located ON the linear (IsPointOn()).
        This methods may not be used with a NULL length linear.
        This method may be used with irregular linears, but when specifying
        a location shared by many parts of the linear, the corresponding
        location will be the one closest to the start point.

        @param pi_rNewStart Constant reference to the HGF2DPosition which must
                            be located on linear that becomes the new start
                            point of linear.


        @see Shorten for examples

        @see Shorten
        @see IsPointOn()
        -----------------------------------------------------------------------------
    */
    virtual void       ShortenFrom(const HGF2DPosition& pi_rNewStartPoint) = 0;

    /** -----------------------------------------------------------------------------
        The ShortenFrom() method permits to modify the linear by
        specification of a new start point as a relative position.
        The relative position provided, must be between 0.0 and 1.0,
        or equal to either one.
        This method may not be used with a NULL length linear.
        This methods may be used with irregular linears.

        @param pi_StartRelativePosition Relative position (between 0.0
                                        and 1.0 incusively) of the new
                                        start point desired.

        @see Shorten for examples

        @see Shorten()
        -----------------------------------------------------------------------------
    */
    virtual void       ShortenFrom(double pi_StartRelativePosition) = 0;

    virtual void       Reverse();

    /** -----------------------------------------------------------------------------
        This method adjusts the start of a linear to some very
        close location. The maximum distance tolerable for adjustement
        is one time the tolerance of the linear of distance. This method is
        only provided for adjustements of variations of start point
        as a result of mathematical errors.

        @param pi_rNewStartPoint Constant reference to the new start point.
                                 This point must not be located farther from
                                 original start point than the current linear tolerance
                                 of distance.

        Example:
        @code
        @end

        @see AdjustEndPointTo()
        @see GetTolerance()
        @see GetStartPoint()
        -----------------------------------------------------------------------------
    */
    virtual void       AdjustStartPointTo(const HGF2DPosition& pi_rPoint) = 0;

    /** -----------------------------------------------------------------------------
        This method adjusts the end of a linear to some very
        close location. The maximum distance tolerable for adjustement
        is one time the tolerance of the linear of distance. This method is
        only provided for adjustements of variations of end point
        as a result of mathematical errors.

        @param pi_rNewEndPoint Constant reference to the new end point.
                                 This point must not be located farther from
                                 original start point than the current linear tolerance
                                 of distance.

        Example:
        @code
        @end

        @see AdjustStartPointTo()
        @see GetTolerance()
        @see GetEndPoint()
        -----------------------------------------------------------------------------
    */
    virtual void       AdjustEndPointTo(const HGF2DPosition& pi_rPoint) = 0;

    virtual void       Drop(HGF2DPositionCollection* po_pPoint,
                            double                   pi_rTolerance,
                            EndPointProcessing pi_EndPointProcessing = INCLUDE_END_POINT) const = 0;



    // Determination of linear properties
    virtual bool      LinksTo(const HGF2DLinear& pi_rLinear) const;
    virtual bool      ConnectsTo(const HGF2DVector& pi_rVector) const;

    /** -----------------------------------------------------------------------------
        This method indicates if a linear path crosses itself.

        @return true if the linear crosses itself

        Example:
        @code
        @end

        @see Crosses()
        -----------------------------------------------------------------------------
    */
    virtual bool      AutoCrosses() const = 0;
//        virtual int        AutoIntersect(HGF2DPositionCollection* po_pPoints) const = 0;

    // From HGF2DVector
    virtual HGF2DVectorTypeId
    GetMainVectorType() const;
    virtual bool      IsAtAnExtremity(const HGF2DPosition& pi_rLocation,
                                       double pi_Tolerance = HGF_USE_INTERNAL_EPSILON) const;

    virtual void       Move(const HGF2DDisplacement& pi_rDisplacement);
    virtual void       Scale(double pi_ScaleFactor,
                             const HGF2DPosition& pi_rScaleOrigin);


    virtual void       PrintState(ostream& po_rOutput) const;

protected:
    // Those are protected for performance reason only
    HGF2DPosition           m_StartPoint;
    HGF2DPosition           m_EndPoint;

    };

END_IMAGEPP_NAMESPACE
#include "HGF2DLinear.hpp"
