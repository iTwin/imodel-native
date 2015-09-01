//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DDisplacement.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Default Constructor
    This constructor create displacement object with no offsets, thus representing
    no displacement.

    @code
        HGF2DDisplacement        ANullDisplacement();
    @end

    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement::HGF2DDisplacement()
    :m_DeltaXDist(0),
     m_DeltaYDist(0)
    {
    }

/** -----------------------------------------------------------------------------
    Bearing constructor
    This constructor creates a displacement by specification of a bearing
    and a displacement distance.

    @param pi_rBearing IN Constant reference to a bearing indicating the direction
                          of the displacement.

    @param pi_Distance IN The distance of the displacement
                       in the given direction (pi_rBearing)

    @code
        HGFBearing          DispDirection(1.2);
        HGF2DDisplacement   Disp1(DispDirection, 10);
    @end


    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement::HGF2DDisplacement(const HGFBearing&  pi_rDirection,
                                            double             pi_Distance)
    {
    SetByBearing (pi_rDirection, pi_Distance);
    }


/** -----------------------------------------------------------------------------
    Destroyer
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement::~HGF2DDisplacement()
    {
    }

/** -----------------------------------------------------------------------------
    Offset constructor
    This constructor is used by specifying two offset distances.

    @param pi_rOffsetX IN Constant reference to the distance offset in the X dimension.

    @param pi_rOffsetY IN Constant reference to the distance offset in the Y dimension.

    @code
        HGF2DDisplacement      Disp(10, 12);
    @end
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement::HGF2DDisplacement(double  pi_rXOffset,
                                            double  pi_rYOffset)
    : m_DeltaXDist (pi_rXOffset),
      m_DeltaYDist (pi_rYOffset)
    {
    }


//-----------------------------------------------------------------------------
// The copy constructor.  It duplicates another displacement object.
//-----------------------------------------------------------------------------
inline HGF2DDisplacement::HGF2DDisplacement(const HGF2DDisplacement& pi_rObj)
    : m_DeltaXDist (pi_rObj.m_DeltaXDist),
      m_DeltaYDist (pi_rObj.m_DeltaYDist)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
// FOR PERFORMANCE REASON THERE ARE NO DESTRUCTOR DEFINED
//-----------------------------------------------------------------------------
// inline HGF2DDisplacement::~HGF2DDisplacement()
// {
// }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another displacement object.
//-----------------------------------------------------------------------------
inline HGF2DDisplacement& HGF2DDisplacement::operator=(const HGF2DDisplacement& pi_rObj)
    {
    // Check if it is the same object
    if (this != &pi_rObj)
        {
        m_DeltaXDist = pi_rObj.m_DeltaXDist;
        m_DeltaYDist = pi_rObj.m_DeltaYDist;
        }

    return (*this);
    }


/** -----------------------------------------------------------------------------
    Equality evaluation operator.  Two displacement objects are considered
    "equal" if they represent the same displacement, whatever the displacement
    units used. If they are not expressed in the same units, internal conversion
    of either one will occur.

    @return A boolean value that reflects the equality or inequality between operands

    @end

    @see operator!=()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DDisplacement::operator==(const HGF2DDisplacement& pi_rObj) const
    {
    return ((m_DeltaXDist == pi_rObj.m_DeltaXDist) &&
            (m_DeltaYDist == pi_rObj.m_DeltaYDist));
    }

/** -----------------------------------------------------------------------------
    Inequality evaluation operator.  Two displacement objects are considered
    "equal" if they represent the same displacement, whatever the displacement
    units used. If they are not expressed in the same units, internal conversion
    of either one will occur.

    @return A boolean value that reflects the equality or inequality between operands

    @code
    @end

    @see operator==()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DDisplacement::operator!=(const HGF2DDisplacement& pi_rObj) const
    {
    return ((m_DeltaXDist != pi_rObj.m_DeltaXDist) ||
            (m_DeltaYDist != pi_rObj.m_DeltaYDist));
    }



/** -----------------------------------------------------------------------------
    This method permits to extract the X offset displacement.

    @return The X offset of the displacement.

    @code

    @see SetDeltaX()
    @see GetDeltaY()
    -----------------------------------------------------------------------------
*/
inline double HGF2DDisplacement::GetDeltaX () const
    {
    return (m_DeltaXDist);
    }


/** -----------------------------------------------------------------------------
    This method permits to extract the Y offset displacement.

    @return The Y offset of the displacement.

    @see SetDeltaY()
    @see GetDeltaX()
    -----------------------------------------------------------------------------
*/
inline double HGF2DDisplacement::GetDeltaY () const
    {

    return (m_DeltaYDist);
    }

/** -----------------------------------------------------------------------------
    This method permits to set the X offset of the displacement

    pi_rNewDeltaX The new X offset to assign to displacement


    @see GetDeltaX()
    @see SetDeltaY()
    -----------------------------------------------------------------------------
*/
inline void HGF2DDisplacement::SetDeltaX (double pi_NewDeltaX)
    {
    m_DeltaXDist = pi_NewDeltaX;
    }

/** -----------------------------------------------------------------------------
    This method permits to set the Y offset of the displacement

    pi_rNewDeltaY The new Y offset to assign to displacement


    @see SetDeltaX()
    @see GetDeltaY()
    -----------------------------------------------------------------------------
*/
inline void HGF2DDisplacement::SetDeltaY (double pi_NewDeltaY)
    {
    m_DeltaYDist = pi_NewDeltaY;
    }


/** -----------------------------------------------------------------------------
    This method permits to set the X and Y offset of the displacement by
    specifying a bearing and a distance.

    @param pi_rDirection IN A constant reference to HGFBearing indicating the
                            direction of the displacement.

    @param pi_Distance IN The length of the displacement.

    @code
        HGF2DDisplacement   Vector2;
        ...
        Vector2.SetByBearing(HGFBearing(PI), 1.85);

    @end

    @see SetDeltaX()
    @see SetDeltaY()
    -----------------------------------------------------------------------------
*/
inline void HGF2DDisplacement::SetByBearing (const HGFBearing&  pi_rDirection,
                                             double  pi_Distance)
    {
    // Extract trigonometric angle value in radians
    double     TrigoAngle = pi_rDirection.GetAngle();

    // Compute and set offsets
    m_DeltaXDist = pi_Distance * cos(TrigoAngle);

    m_DeltaYDist = pi_Distance * sin(TrigoAngle);
    }


/** -----------------------------------------------------------------------------
    Addition operation. This operation adds two displacement together. If the
    displacements are expressed in different units, then the result is expressed
    in the units of the left operand for operator+.

    @param pi_rObj Constant reference to a HGF2DDisplacement to add.

    @return A to HGF2DDisplacement resulting from addition.

    @code
        HGF2DDisplacement   Disp1(10, 12);
        HGF2DDisplacement   Disp2(13, 32);
        HGF2DDisplacement   SumOfDisps = Disp1 + Disp2;
    @end

    @see operator+=()
    @see operator-()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement HGF2DDisplacement::operator+(const HGF2DDisplacement& pi_rObj) const
    {
    return (HGF2DDisplacement(m_DeltaXDist + pi_rObj.m_DeltaXDist, m_DeltaYDist + pi_rObj.m_DeltaYDist));
    }

/** -----------------------------------------------------------------------------
    Substraction operation. It substracts two displacements together. If the
    displacements are expressed in different units, then the result is
    expressed in the units of the left operand.

    @param pi_rObj Constant reference to a HGF2DDisplacement to substract.

    @return An HGF2DDisplacement resulting from substraction.

    @code
        HGF2DDisplacement   Disp1(10, 12);
        HGF2DDisplacement   Disp2(13, 32);
        HGF2DDisplacement   DiffOfDisps = Disp2 - Disp1;
    @end

    @see operator-=()
    @see operator+()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement HGF2DDisplacement::operator-(const HGF2DDisplacement& pi_rObj) const
    {
    return (HGF2DDisplacement(m_DeltaXDist - pi_rObj.m_DeltaXDist, m_DeltaYDist - pi_rObj.m_DeltaYDist));
    }

/** -----------------------------------------------------------------------------
    The unary operator- which produces a displacement which is the reverse of self.

    @return An HGF2DDisplacement resulting from reversal.

    @code
        HGF2DDisplacement   Disp1(10, 12);
        HGF2DDisplacement   Disp2(13, 32);
        HGF2DDisplacement   OtherDisp = - Disp1;
    @end

    @see operator-=()
    @see operator+()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement HGF2DDisplacement::operator-() const
    {
    return (HGF2DDisplacement(-m_DeltaXDist, -m_DeltaYDist));
    }


/** -----------------------------------------------------------------------------
    Multiplication operation. The displacement is scaled by the multiplicand.
    The result is returned in a new displacement object.
    Scaling a displacement result in the multiplication of the length by
    the multiplicand given.

    @param pi_Multiplicator IN Double representing the multiplicand.

    @return An HGF2DDisplacement resulting from multiplication.

    @code
        HGF2DDisplacement   Disp1(10, 12);
        HGF2DDisplacement   ScaledDisp = Disp1 * 2;
    @end

    @see operator*=()
    @see operator/()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement HGF2DDisplacement::operator*(double pi_Multiplicator) const
    {
    return (HGF2DDisplacement(m_DeltaXDist * pi_Multiplicator, m_DeltaYDist * pi_Multiplicator));
    }

/** -----------------------------------------------------------------------------
    Division operation. The displacement expressed by the first operand is
    divided and produces a result displacement expressed in the same units
    as the first operand. The offset or length of the displacement is
    divided by these operations.

    @param pi_Divisor IN Double representing the divisor. This number must
                         be different from 0.0.

    @return An HGF2DDisplacement resulting from division.

    @code
        HGF2DDisplacement   Disp1(10, 12);
        HGF2DDisplacement   ScaledDisp = Disp1 / 2;
    @end

    @see operator*()
    @see operator/=()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement HGF2DDisplacement::operator/(double pi_Divisor) const
    {
    HPRECONDITION (pi_Divisor != 0.0);

    return (HGF2DDisplacement(m_DeltaXDist / pi_Divisor, m_DeltaYDist / pi_Divisor));
    }



/** -----------------------------------------------------------------------------
    Increment operator. This operation adds given displacement into self .

    @param pi_rObj Constant reference to a HGF2DDisplacement to add to self.

    @return A reference to self to be used as an l-value.

    @code
        HGF2DDisplacement   Disp1(10, 12);
        HGF2DDisplacement   Disp2(13, 32);
        Disp2 += Disp1;
    @end

    @see operator+()
    @see operator-=()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement& HGF2DDisplacement::operator+=(const HGF2DDisplacement& pi_rObj)
    {
    m_DeltaXDist += pi_rObj.m_DeltaXDist;
    m_DeltaYDist += pi_rObj.m_DeltaYDist;

    return (*this);
    }

/** -----------------------------------------------------------------------------
    Decrement operator. This operation substracts given displacement from self .

    @param pi_rObj Constant reference to a HGF2DDisplacement to subtract to self.

    @return A reference to self to be used as an l-value.

    @code
        HGF2DDisplacement   Disp1(10, 12);
        HGF2DDisplacement   Disp2(13, 32);
        Disp2 -= Disp1;
    @end

    @see operator-()
    @see operator+=()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement& HGF2DDisplacement::operator-=(const HGF2DDisplacement& pi_rObj)
    {
    m_DeltaXDist -= pi_rObj.m_DeltaXDist;
    m_DeltaYDist -= pi_rObj.m_DeltaYDist;

    return (*this);
    }



/** -----------------------------------------------------------------------------
    Scale operator. This operation scales the displacement

    @param pi_Multiplicator IN Double representing the multiplicand.

    @return A reference to self to be used as an l-value.

    @code
        HGF2DDisplacement   Disp1(10, 12);
        Disp1 *= 2;
    @end

    @see operator*()
    @see operator/=()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement& HGF2DDisplacement::operator*=(double pi_Multiplicator)
    {
    m_DeltaXDist *= pi_Multiplicator;
    m_DeltaYDist *= pi_Multiplicator;

    return (*this);
    }

/** -----------------------------------------------------------------------------
    Division operation. The displacement expressed by the first operand is
    divided. The offset or length of the displacement is divided by these operations.

    @param pi_Divisor IN Double representing the divisor.
                         This number must be different from 0.0.

    @return A reference to self to be used as an l-value.

    @code
        HGF2DDisplacement   Disp1(10, 12);
        Disp1 /= 2;
    @end

    @see operator*=()
    @see operator/()
    -----------------------------------------------------------------------------
*/
inline HGF2DDisplacement& HGF2DDisplacement::operator/=(double pi_Divisor)
    {
    HPRECONDITION (pi_Divisor != 0.0);

    m_DeltaXDist /= pi_Divisor;
    m_DeltaYDist /= pi_Divisor;

    return (*this);
    }

//-----------------------------------------------------------------------------
// ::operator*
// Friend function : Scales the displacement
//-----------------------------------------------------------------------------
inline HGF2DDisplacement operator*(double pi_Multiplicator, const HGF2DDisplacement& pi_rDisp)
    {
    return (HGF2DDisplacement(pi_rDisp.m_DeltaXDist * pi_Multiplicator, pi_rDisp.m_DeltaYDist * pi_Multiplicator));
    }
END_IMAGEPP_NAMESPACE