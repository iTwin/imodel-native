//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFBearing.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFBearing
//-----------------------------------------------------------------------------
// Description of an bearing.
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class encapsulates a bearing. A bearing expresses a absolute quantity
    as the direction in radians from the East direction counterclockwise.

    Trigonometric space
    The HGFBearing class implements a member function CalculateTrigoAngle()
    which returns a direction as defined in the trigonometric space. We define
    the trigonometric space here, as a space where angle base direction is
    EAST, and angles increase Counter-Clockwise, and finally these angles are
    always expressed in radians. Since the space is so precisely defined,
    the CalculateTrigoAngle() method simply returns a double. All other information
    is part of the trigonometric space.

    Substraction of bearings
    Many arithmetic operations are defined for the HGFBearing class objects,
    but most of them require as second operand a double interpreted as an angle. This kind of object
    blends well in such operations with the definition of the HGFBearing class.
    There is however an operation that takes as second operands an HGFBearing object.
    The operator- with HGFBearing as second operand can be quite useful,
    and easily permit to perform complicated angular operations. This method
    is a bit tricky to use however, and the operation should be precisely understood
    before using.
    This method calculates the angle formed between two bearings. Unfortunately,
    the angular space is always circular, and two possible angles define the
    angle formed between these two bearings

    The result of operator- is always the angle difference from first operand
    to second turning counter-clockwise.


    Example:
    @code
        HGFBearing      FirstBearing;
        HGFBearing      Bearing1(10);
        HGFBearing      SecondBearing (10);
        HGFBearing      OtherBearing = Bearing1;

        // Equality compare operations
        if (FirstBearing.IsEqualTo(SecondBearing, 0.000001))
          ...

        if (FirstBearing == SecondBearing)
          ...

        if (FirstBearing != SecondBearing)
          ...


        // Data accessors
        double MyAngle = MyBearing.GetAngle ();

        // Trigonometric angles
        double MyTrigoAngle = MyBearing.CalculateTrigoAngle ();

        // Sets
        MyBearing.SetAngle(34.6);

        // operations
        double    MyAngle = PI/4;
        HGFBearing  Sum = Bearing1 + MyAngle;

        HGFBearing  DiffOfBearings = Bearing1 - MyAngle;
        double      MyDiffAngle = DiffOfBearings - Bearing1;
        Bearing1 += MyAngle;
        Bearing1 -= MyAngle;

    @end
    -----------------------------------------------------------------------------
*/
class HGFBearing
    {
public:

    // Primary methods

    HGFBearing();
    HGFBearing(double            pi_Angle);
    HGFBearing(const HGFBearing& pi_rObj);
    virtual         ~HGFBearing();

    HGFBearing&     operator=(const HGFBearing& pi_rObj);

    // Compare operations
    bool           operator==(const HGFBearing& pi_rObj) const;
    bool           operator!=(const HGFBearing& pi_rObj) const;

    // Compare operations with epsilon
    bool           IsEqualTo(const HGFBearing& pi_rObj) const;
    bool           IsEqualTo(const HGFBearing& pi_rObj, double pi_Epsilon) const;
    bool           IsEqualToAutoEpsilon(const HGFBearing& pi_rObj) const;

    // Other compare operations
    bool           IsBearingWithinSweep(double pi_Sweep,
                                        const HGFBearing& pi_rBearing) const;

    // Value related methods
    double          GetAngle() const;
    void            SetAngle(double pi_Angle);

    double         CalculateTrigoAngle() const;


    // Arithmetic operations
    HGFBearing      operator+(double pi_Angle) const;
    HGFBearing      operator-(double pi_Angle) const;
    double          operator-(const HGFBearing& pi_rObj) const;
    HGFBearing&     operator+=(double pi_Angle);
    HGFBearing&     operator-=(double pi_Angle);

protected:

private:

    // private methods
    void            Copy(const HGFBearing& pi_rObj);

    // member attributes
    double        m_Angle;
    };

inline bool fIsBearingWithinSweep(HGFBearing sourceBearing, double sweep, HGFBearing compareBearing)
    {
    HGFBearing bearing1(sourceBearing);
    return bearing1.IsBearingWithinSweep(sweep, HGFBearing(compareBearing));
    }

END_IMAGEPP_NAMESPACE
#include "HGFBearing.hpp"

