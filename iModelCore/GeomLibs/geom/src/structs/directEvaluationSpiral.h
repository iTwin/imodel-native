/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// Apply powers of scale factor to 3 derivatives
void ScaleDerivatives (double scale1, DVec2dP d1UV, DVec2dP d2UV, DVec2dP d3UV)
    {
    double scale2 = scale1 * scale1;
    if (d1UV)
        d1UV->Scale (scale1);
    if (d2UV)
        d2UV->Scale (scale2);
    if (d3UV)
        d3UV->Scale (scale2 * scale1);
    }
// Working with a function
//   s = p(x)
// where derivatives of of p with respect to x are known.
// Return derivatives of x with respect to s.
bool InvertDerivatives(double d1pdx, double d2pdx, double d3pdx, double &d1xds, double &d2xds, double &d3xds)
    {
    auto a = DoubleOps::ValidatedDivide (1.0, d1pdx, 0.0);
    if (!a.IsValid())
        {
        d1xds = d2xds = d3xds = 0.0;
        return false;
        }
    d1xds = a;
    double a3 = a * a * a;
    d2xds = - d2pdx * a3;
    double a5 = a3 * a * a;
    d3xds = - (d3pdx * d1pdx - 3.0 * d2pdx * d2pdx) * a5;
    return true;
    }
// convert 2 derivatives of y =f(x) to curvature.
double univariateCurvature (double dydx, double d2ydxdx)
    {
    return d2ydxdx / sqrt (1.0 + dydx * dydx);
    }
double parametricCurvature (double dx, double dy, double ddx, double ddy)
    {
    double q = dx * dx + dy * dy;
    return (dx * ddy - ddx * dy) / sqrt (q * q * q);
    }
double parametricCurvatureDerivative (double dx, double dy, double ddx, double ddy, double dddx, double dddy)
    {
    double q = dx * dx + dy * dy;
    double f = dx * ddy - ddx * dy;
    double df = dx * dddy - dddx * dy;
    double gg = q * q * q;
    double g = sqrt (q * q * q);
    double dg = 1.5 * sqrt (q) * 2.0 * (dx *ddx + dy * ddy);

    return (df * g - f * dg) / gg;
    }


double DSpiral2dDirectEvaluation::DistanceToLocalAngle  (double distance) const
    {
    double effectiveLength = m_nominalLength == 0.0 ? mLength : m_nominalLength;
    return FractionToLocalAngle (distance / effectiveLength);
    }
// Return the curvature at specified distance from start ...
double DSpiral2dDirectEvaluation::DistanceToCurvature   (double distance) const
    {
    double effectiveLength = m_nominalLength == 0.0 ? mLength : m_nominalLength;
    return FractionToCurvature (distance / effectiveLength);
    }
// Return the derivative of curvature wrt arc length at specified distance from start ...
double DSpiral2dDirectEvaluation::DistanceToCurvatureDerivative (double distance) const
    {
    double curvature, dCurvatureDFraction;
    double effectiveLength = m_nominalLength == 0.0 ? mLength : m_nominalLength;
    double fraction = distance / effectiveLength;
    double dCurvatureDDistance = 0.0;
    if (FractionToDCurvatureDFraction (fraction, curvature, dCurvatureDFraction))
        {
        double velocity = FractionToVelocity (fraction);
        DoubleOps::SafeDivide (dCurvatureDDistance, dCurvatureDFraction, velocity, 0.0);
        }
    return dCurvatureDDistance;
    }

double DSpiral2dDirectEvaluation::FractionToCurvature (double fraction) const
    {
    DPoint2d uv;
    DVec2d uvD1, uvD2;
    double curvature = 0.0;
    if (EvaluateAtFraction (fraction, uv, &uvD1, &uvD2, nullptr))
        {
        curvature = parametricCurvature (uvD1.x, uvD1.y, uvD2.x, uvD2.y);
        }
    return curvature;
    }    

double DSpiral2dDirectEvaluation::FractionToVelocity(double fraction) const
    {
    DPoint2d uv;
    DVec2d uvD1;
    double velocity = 0.0;
    if (EvaluateAtFraction (fraction, uv, &uvD1, nullptr, nullptr))
        {
        velocity = uvD1.Magnitude ();
        }
    return velocity;
    }    

double DSpiral2dDirectEvaluation::FractionToLocalAngle(double fraction) const
    {
    DPoint2d uv;
    DVec2d uvD1;
    double radians = 0.0;
    if (EvaluateAtFraction (fraction, uv, &uvD1, nullptr, nullptr))
        {
        radians = atan2 (uvD1.y, uvD1.x);
        }
    return radians;
    }    

bool DSpiral2dDirectEvaluation::FractionToDCurvatureDFraction (double fraction, double &curvature, double &dCurvatureDFraction) const
    {
    DPoint2d uv;
    DVec2d uvD1, uvD2, uvD3;
    if (EvaluateAtFraction (fraction, uv, &uvD1, &uvD2, &uvD3))
        {
        curvature = parametricCurvature (uvD1.x, uvD1.y, uvD2.x, uvD2.y);
        dCurvatureDFraction = parametricCurvatureDerivative (uvD1.x, uvD1.y, uvD2.x, uvD2.y, uvD3.x, uvD3.y);
        return true;
        }
    curvature = 0.0;
    dCurvatureDFraction = 0.0;
    return false;
    }  

// rotate each vector by radians ...
void DSpiral2dDirectEvaluation::ApplyCCWRotation
(
double radians,
DPoint2dR xy,          //!< [out] coordinates on spiral
DVec2dP d1XY,   //!< [out] first derivative wrt distance
DVec2dP d2XY,   //!< [out] second derivative wrt distance
DVec2dP d3XY    //!< [out] third derivative wrt distance
)
    {
    if (radians != 0.0)
        {
        xy.RotateCCW (xy, radians);
        if (d1XY)
            d1XY->RotateCCW (radians);
        if (d2XY)
            d2XY->RotateCCW (radians);
        if (d3XY)
            d3XY->RotateCCW (radians);
        }
    }

// Specialize spiral for NEWSOUTHWALES ....
DSpiral2dWesternAustralian::DSpiral2dWesternAustralian (double nominalLength) : DSpiral2dDirectEvaluation (nominalLength) {}
// STATIC method
bool DSpiral2dWesternAustralian::EvaluateAtDistanceInStandardOrientation
    (
    double s, //!< [in] distance for evaluation
    double length,  //!< [in] nominal length.   ASSUMED NONZERO
    double curvature1, //!< [in] exit curvature.  ASSUMED NONZERO
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
    )
    {
    double factorX = curvature1 * curvature1 / (40.0 * length * length);
    double s2 = s * s;
    double s3 = s2 * s;
    double s4 = s2 * s2;

    double factorY = curvature1 / (6.0 * length);
    xy.Init (s * (1.0 - s4 *  factorX), s3 * factorY);

    if (d1XY)
        d1XY->Init (1.0 - 5.0 * s4 * factorX, 3.0 * s2 * factorY);

    if (d2XY)
        d2XY->Init (-20.0 * s3 * factorX, 6.0 * s * factorY);

    if (d3XY)
        d3XY->Init (-60.0 * s2 * factorX, 6.0 * factorY);
    return true;
    }
bool DSpiral2dWesternAustralian::EvaluateAtFraction
    (
    double fraction, //!< [in] fraction for evaluation
    DPoint2dR xy,    //!< [out] coordinates on spiral
    DVec2dP d1XY,    //!< [out] first derivative wrt distance
    DVec2dP d2XY,    //!< [out] second derivative wrt distance
    DVec2dP d3XY     //!< [out] third derivative wrt distance
    ) const
    {
    if (mCurvature0 == 0.0)
        {
        static bool s_applyRotation = true;
        double L = m_nominalLength;
        double nominalDistance = fraction * L;

        bool stat = EvaluateAtDistanceInStandardOrientation (nominalDistance, L, mCurvature1, xy, d1XY, d2XY, d3XY);
        ScaleDerivatives (L, d1XY, d2XY, d3XY);
        if (stat && s_applyRotation)
            DSpiral2dDirectEvaluation::ApplyCCWRotation (mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }
    return false;
    }
DSpiral2dBaseP DSpiral2dWesternAustralian::Clone () const
    {
    double nominalLength = m_nominalLength;
    if (nominalLength == 0.0 && mLength != 0.0)
        nominalLength = mLength;
    DSpiral2dWesternAustralian *pClone = new DSpiral2dWesternAustralian (nominalLength);
    pClone->CopyBaseParameters (this);
    return pClone;
    }

// TWO terms from each of the sign and cosine series ..
// Used by both Arema and Chinese
bool DSpiral2dBase::EvaluateTwoTermClothoidSeriesAtDistanceInStandardOrientation
(
    double s, //!< [in] distance for evaluation
    double length,  //!< [in] nominal length.   ASSUMED NONZERO
    double curvature1, //!< [in] exit curvature.  ASSUMED NONZERO
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
)
    {
    double factorX = curvature1 * curvature1 / (40.0 * length * length);
    double s2 = s * s;
    double s3 = s2 * s;
    double s4 = s2 * s2;
    double s5 = s * s4;
    double s6 = s * s5;
    double s7 = s * s6;

    double factorY = curvature1 / (6.0 * length);
    double factorY1 = curvature1 * curvature1 * curvature1 / (336.0 * length * length * length);

    xy.Init(s * (1.0 - s4 * factorX), s3 * factorY - s7 * factorY1);

    if (d1XY)
        d1XY->Init(1.0 - 5.0 * s4 * factorX, 3.0 * s2 * factorY - 7 * s6 * factorY1);

    if (d2XY)
        d2XY->Init(-20.0 * s3 * factorX, 6.0 * s * factorY - 42 * s5 *factorY1);

    if (d3XY)
        d3XY->Init(-60.0 * s2 * factorX, 6.0 * factorY - 210 * s4 * factorY1);
    return true;
    }

// Specialize spiral for Chinese cubic ....
DSpiral2dChinese::DSpiral2dChinese(double nominalLength) : DSpiral2dDirectEvaluation(nominalLength)
    {
    }
//! Evaluate at distance a spiral in standard orientation -- zero curvature at origin.
//! This is implemented as a call to EvaluateTwoTermClothoidSeriesAtDistanceInStandardOrientation.
bool EvaluateAtDistanceInStandardOrientation
(
    double s,           //!< [in] distance for evaluation
    double length,      //!< [in] strictly nonzero length along spiral.
    double curvature1,  //!< [in] strictly nonzero exit curvature
    DPoint2dR xy,      //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
)
    {
    return DSpiral2dBase::EvaluateTwoTermClothoidSeriesAtDistanceInStandardOrientation(s, length, curvature1, xy, d1XY, d2XY, d3XY);
    }

bool DSpiral2dChinese::EvaluateAtFraction
    (
    double fraction, //!< [in] fraction for evaluation
    DPoint2dR xy,    //!< [out] coordinates on spiral
    DVec2dP d1XY,    //!< [out] first derivative wrt distance
    DVec2dP d2XY,    //!< [out] second derivative wrt distance
    DVec2dP d3XY     //!< [out] third derivative wrt distance
    ) const
    {
    if (mCurvature0 == 0.0)
        {
        static bool s_applyRotation = true;
        double L = m_nominalLength;
        double nominalDistance = fraction * L;

        bool stat = DSpiral2dBase::EvaluateTwoTermClothoidSeriesAtDistanceInStandardOrientation(nominalDistance, L, mCurvature1, xy, d1XY, d2XY, d3XY);
        ScaleDerivatives (L, d1XY, d2XY, d3XY);
        if (stat && s_applyRotation)
            DSpiral2dDirectEvaluation::ApplyCCWRotation (mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }
    return false;
    }
DSpiral2dBaseP DSpiral2dChinese::Clone() const
    {
    DSpiral2dChinese *pClone = new DSpiral2dChinese(m_nominalLength);
    pClone->CopyBaseParameters(this);
    return pClone;
    }
// Specialize spiral for Arema  ....
// April 2019
// * Exact copy of corresponding methods in Chinese -
// * Move EvaluateTwoTermSeriesAtDistanceInStandardOrientation to standalone static, use it from both.
// * The modest remaining code duplication is potentially important so either side can diverge because
//   ** change in Chinese specs
//   ** maybe Arema users will want (ouch) classic 3-digit feet/inches constants revived.
// Maybe one will want different use of nominal length and fraction
DSpiral2dArema::DSpiral2dArema(double nominalLength) : DSpiral2dDirectEvaluation(nominalLength)
    {
    }

bool DSpiral2dArema::EvaluateAtFraction
(
    double fraction, //!< [in] fraction for evaluation
    DPoint2dR xy,    //!< [out] coordinates on spiral
    DVec2dP d1XY,    //!< [out] first derivative wrt distance
    DVec2dP d2XY,    //!< [out] second derivative wrt distance
    DVec2dP d3XY     //!< [out] third derivative wrt distance
) const
    {
    if (mCurvature0 == 0.0)
        {
        static bool s_applyRotation = true;
        double L = m_nominalLength;
        double nominalDistance = fraction * L;

        bool stat = DSpiral2dBase::EvaluateTwoTermClothoidSeriesAtDistanceInStandardOrientation(nominalDistance, L, mCurvature1, xy, d1XY, d2XY, d3XY);
        ScaleDerivatives(L, d1XY, d2XY, d3XY);
        if (stat && s_applyRotation)
            DSpiral2dDirectEvaluation::ApplyCCWRotation(mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }
    return false;
    }
DSpiral2dBaseP DSpiral2dArema::Clone() const
    {
    DSpiral2dArema *pClone = new DSpiral2dArema(m_nominalLength);
    pClone->CopyBaseParameters(this);
    return pClone;
    }
// Specialize spiral for AUSTRALIAN ....

double DEGRAD(double a)
    {
    return Angle::DegreesToRadians (a);
    }
static double aecAlg_computeAustralianPhi( double R, double xc )
{
    double phi, expr1, expr2, expr3;

    expr1 = ( 2. / sqrt( 3. ));
    expr2 = ( - ( 3. / 4. ) * sqrt( 3. ) * xc / R );
    if (expr2 < -1.0)
        expr2 = -1.0;
    if (expr2 > 1.0)
        expr2 = 1.0;
    expr3 = DEGRAD( 240. );

    phi = asin( expr1 * cos( acos( expr2 ) / 3. + expr3 ));

    return( phi );
}

double aecAlg_computeAustralianLengthFromXcR( double R, double xc )
{
    double l, m, phi;

    phi = aecAlg_computeAustralianPhi( R, xc );

    m = tan( phi ) / ( 3. * xc * xc );

    l = xc  + (     9. /   10. ) * pow( m, 2 ) * pow( xc,  5 )
            - (     9. /    8. ) * pow( m, 4 ) * pow( xc,  9)
            + (   729. /  208. ) * pow( m, 6 ) * pow( xc, 13 )
            - ( 32805. / 2176. ) * pow( m, 8 ) * pow( xc, 17 );

    return( l );
}

double aecAlg_computeAustralianXcFromRL_fast( double L, double R )
{
    int idx = 0;
    double xc, l, m, phi;
    static double tolerance = 1.0e-5;

    xc = .7 * L;

    for( idx = 0; idx < 100; ++idx )
    {
        phi = aecAlg_computeAustralianPhi( R, xc );
        double xc2 = xc * xc;
        m = tan( phi ) / ( 3.0 * xc2);
        double m2x4 = m * m * xc2 * xc2;
        double correction = xc * m2x4 * (
                 (     9. /   10. ) + m2x4 * (
               - (     9. /    8. ) + m2x4 * (
               + (   729. /  208. ) + m2x4 *
               - ( 32805. / 2176. ))));
        l = xc + correction;
        xc = ( L / l ) * xc;

        if( fabs( L - l ) < tolerance )
            break;
    }

    return( xc );
}

double aecAlg_computeAustralianXcFromRL( double L, double R )
{
    int idx = 0;
    double xc, l, m, phi;

    xc = .7 * L;

    for( idx = 0; idx < 100; ++idx )
    {
        phi = aecAlg_computeAustralianPhi( R, xc );

        m = tan( phi ) / ( 3. * xc * xc );

        l = xc + (     9. /   10. ) * pow( m, 2 ) * pow( xc,  5 )
               - (     9. /    8. ) * pow( m, 4 ) * pow( xc,  9 )
               + (   729. /  208. ) * pow( m, 6 ) * pow( xc, 13 )
               - ( 32805. / 2176. ) * pow( m, 8 ) * pow( xc, 17 );

        xc = ( L / l ) * xc;

        if( fabs( L - l ) < 0.00001 )
            break;
    }

    return( xc );
}

DSpiral2dAustralianRailCorp::DSpiral2dAustralianRailCorp (double nominalLength) : DSpiral2dDirectEvaluation (nominalLength) {}
// STATIC method ...
bool DSpiral2dAustralianRailCorp::EvaluateAtDistanceInStandardOrientation
    (
    double s,           //!< [in] distance for evaluation
    double length,      //! [in] strictly nonzero length along spiral.
    double curvature1,  //! [in] strictly nonzero exit curvature
    DPoint2dR xy,      //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
    )
    {
    double radius1 = 1.0 / curvature1;
//    double xc = aecAlg_computeAustralianXcFromRL (length, radius1);
    double xc = aecAlg_computeAustralianXcFromRL_fast (length, radius1);
    // double xcA = aecAlg_computeAustralianXcFromRL_fast (length1, radius1);
    double phi = aecAlg_computeAustralianPhi( radius1, xc );

    double a1 = 0.9000;
    double a2 = 5.1750;
    double a3 = 43.1948;
    double a4 = 426.0564;
    double m = tan( phi ) / ( 3.0 * xc * xc );
    double m2s4 =  m * m * s * s * s * s;
    double x = s * (1.0 - m2s4 * (a1 - m2s4 * (a2 - m2s4 * (a3 - m2s4 * a4))));
    double y = m * x * x * x;
    xy.Init (x,y);
    if (d1XY)
        {
        double dxds = 1.0 - m2s4 * (5.0 * a1 - m2s4 * (9.0 * a2 - m2s4 * (13.0 * a3 - m2s4 * 17.0 * a4)));
        double dyds = 3.0 * m * x * x * dxds;  // chain rule!!
        d1XY->Init (dxds, dyds);
        if (d2XY)
            {
            double m2s3 = m * m * s * s * s;
            double d2xds2 = -m2s3 * (20.0 * a1 - m2s4 * (72.0 * a2 - m2s4 * (156.0 * a3 - m2s4 * 272.0 * a4 )));
            double d2yds2 = 3.0 * m * (2.0 * x * dxds * dxds + x * x * d2xds2);
            d2XY->Init (d2xds2, d2yds2);
            if (d3XY)
                {
                double m2s2 = m * m * s * s;
                double d3xds3 = -m2s2 * (60.0 * a1 - m2s4 * (504.0 * a2 - m2s4 * (1716.0 * a3 - m2s4 * 4080.0 * a4 )));
                double d3yds3 = 3.0 * m * (
                                  (2.0 * dxds * dxds * dxds + 4.0 * x * dxds * d2xds2)
                                + (2.0 * x * dxds * d2xds2 + x * x * d3xds3)
                                    );
                d3XY->Init (d3xds3, d3yds3);
                }
            }
        }
        
    return true;
    }
bool DSpiral2dAustralianRailCorp::EvaluateAtFraction
    (
    double fraction, //!< [in] fraction for evaluation
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
    ) const
    {
    if (mCurvature0 == 0.0)
        {
        static bool s_applyRotation = true;
        double L = m_nominalLength;
        double nominalDistance = fraction * L;
        bool stat = EvaluateAtDistanceInStandardOrientation (nominalDistance, L, mCurvature1, xy, d1XY, d2XY, d3XY);
        if (d1XY)
            d1XY->Scale (L);
        if (d2XY)
            d2XY->Scale (L * L);
        if (d3XY)
            d3XY->Scale (L * L * L);
        if (stat && s_applyRotation)
            DSpiral2dDirectEvaluation::ApplyCCWRotation (mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }       
    return false;
    }
DSpiral2dBaseP DSpiral2dAustralianRailCorp::Clone () const
    {
    DSpiral2dAustralianRailCorp *pClone = new DSpiral2dAustralianRailCorp (m_nominalLength);
    pClone->CopyBaseParameters (this);
    return pClone;
    }


// Specialize spiral for MXCubic ....
DSpiral2dMXCubicAlongArc::DSpiral2dMXCubicAlongArc (double nominalLength) : DSpiral2dDirectEvaluation (nominalLength) {}
// STATIC method
bool DSpiral2dMXCubicAlongArc::EvaluateAtFractionInStandardOrientation
    (
    double fraction, //!< [in] fraction for evaluation
    double length,  //!< [in] nominal length.   ASSUMED NONZERO
    double curvature1, //!< [in] exit curvature.  ASSUMED NONZERO
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt fraction
    DVec2dP d2XY,   //!< [out] second derivative wrt fraction
    DVec2dP d3XY   //!< [out] third derivative wrt fraction
    )
    {
    double s = fraction * length;
    double alpha = -curvature1 * curvature1;
    double axisLength = ClothoidCosineApproximation::Evaluate40R2L2Map (alpha, 1.0, length, length);
    double dx1, dx2, dx3;   // derivatives of x wrt s.
    double x = ClothoidCosineApproximation::Evaluate40R2L2Map (alpha, 1.0, length, s, dx1, dx2, dx3);

    double x2 = x * x;
    double x3 = x2 * x;

    double factorY = curvature1 / (6.0 * axisLength);   // axisLength here is unique feature of MX?
    xy.Init (x, x3 * factorY);

    if (d1XY || d2XY || d3XY)
        {
        // derivatives of y wrt x (feed to derivatives wrt via chain rule)
        double dy1 = 3.0 * factorY * x2;
        double dy2 = 6.0 * factorY * x;
        double dy3 = 6.0 * factorY;

        double LL = length * length;
        double LLL = LL * length;

        if (d1XY)
            {
            d1XY->Init (dx1, dy1 * dx1);
            d1XY->Scale (length);
            }

        if (d2XY)
            {
            d2XY->Init (dx2, dy2 * dx1 * dx1 + dy1 * dx2);
            d2XY->Scale (LL);
            }

        if (d3XY)
            {
            d3XY->Init (dx3,  dy3 * dx1 * dx1 * dx1 + 3.0 * dy2 * dx1 * dx2 + dy1 * dx3);
            d3XY->Scale (LLL);
            }
        }
    return true;
    }
bool DSpiral2dMXCubicAlongArc::EvaluateAtFraction
    (
    double fraction, //!< [in] distance for evaluation
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
    ) const
    {
    if (mCurvature0 == 0.0)
        {
        bool stat = EvaluateAtFractionInStandardOrientation (fraction, mLength, mCurvature1, xy, d1XY, d2XY, d3XY);
        if (stat)
            DSpiral2dDirectEvaluation::ApplyCCWRotation (mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }       
    return false;
    }
DSpiral2dBaseP DSpiral2dMXCubicAlongArc::Clone () const
    {
    DSpiral2dMXCubicAlongArc *pClone = new DSpiral2dMXCubicAlongArc (m_nominalLength);
    pClone->CopyBaseParameters (this);
    return pClone;
    }

// Specialize spiral for Italian ....
DSpiral2dItalian::DSpiral2dItalian (double nominalLength) : DSpiral2dDirectEvaluation (nominalLength) {}
DSpiral2dBaseP DSpiral2dItalian::Clone () const
    {
    double nominalLength = m_nominalLength;
    if (nominalLength == 0.0 && mLength != 0.0)
        nominalLength = mLength;
    DSpiral2dItalian *pClone = new DSpiral2dItalian (nominalLength);
    pClone->CopyBaseParameters (this);
    return pClone;
    }


bool DSpiral2dItalian::EvaluateAtFraction
    (
    double fraction, //!< [in] fraction for evaluation
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt fraction
    DVec2dP d2XY,   //!< [out] second derivative wrt fraction
    DVec2dP d3XY   //!< [out] third derivative wrt fraction
    ) const
    {
    if (mCurvature0 == 0.0)
        {
        static bool s_applyRotation = true;
        double Ls = m_nominalLength;
        double R1 = 1.0 / mCurvature1;
        double Lx = ClothoidCosineApproximation::EvaluateItalianCzechR2L2Map(-1.0, R1, Ls, Ls);

        bool stat = DSpiral2dCzech::EvaluateAtFractionOfNominalLengthInStandardOrientation(
            fraction,
            Lx, Ls,
            R1, xy,
            d1XY, d2XY, d3XY);
        ScaleDerivatives (Ls, d1XY, d2XY, d3XY);
        if (stat && s_applyRotation)
            DSpiral2dDirectEvaluation::ApplyCCWRotation (mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }
    return false;
    }




//********************************************************************************************
// ClothoidCosineApproximation -- support for approximating the cosine series in a clothoid.
size_t ClothoidCosineApproximation::s_evaluationCount = 0;
//! 
//! <ul>
//! <li>Instantiate the clothoid cosine approximation function with caller-supplied coefficient of u^5 term.
//! <li>This is a low level constructor -- the caller is responsible for incorporating typical R, L and sign into gamma.
//! </ul>
ClothoidCosineApproximation::ClothoidCosineApproximation (double targetValue, double gamma) : m_targetValue (targetValue), m_gamma (gamma) {}

//! Evaluate {f(u) = u + gamma * u^5 - targetValue}
bool ClothoidCosineApproximation::EvaluateRToRD (double u, double &f, double &dfdu)
    {
    double u2 = u * u;
    double u4 = u2 * u2;
    f = u * (1.0 + m_gamma * u4) - m_targetValue;
    dfdu = 1.0 + 5.0 * m_gamma * u4;
    s_evaluationCount++;
    return true;
    }

//! @returns Given target value f, return u so {u(1 + alpha * u^4/ (40 R*R*L*L) = f}
//! <ul>
//! <li>In use case with {alpha = positive one}, f is distance along spiral and the return value is (approximate) distance along axis.
//! <li>In use case with (alpha = negative one}, f is distance along axis and the return value is (approximate) distance along spiral.
//! </ul>
ValidatedDouble ClothoidCosineApproximation::Invert40R2L2Map (double alpha, double R, double L, double f, double &coff)
    {
    s_evaluationCount = 0;
    coff = alpha / (40.0 * R * R * L * L);
    ClothoidCosineApproximation callback (f, coff);
    NewtonIterationsRRToRR newton (1.0e-12);
    // use the target f as the initial guess
    bool stat = newton.RunNewton (f, callback);
    return ValidatedDouble (f, stat);
    }

//! @returns {f(u) = u * (1 + alpha * u^4/ (40 R R L L)}
//! <ul>
//! <li>In use case with {alpha = positive one}, return value is (approximate) distance along spiral and u is distance along axis.
//! <li>In use case with (alpha = negative one}, u is distance along spiral and the return value is (approximate) distance along axis.
//! </ul>
double ClothoidCosineApproximation::Evaluate40R2L2Map (double alpha, double R, double L, double u)
    {
    double u2 = u * u;
    return u * (1.0 + alpha * u2 * u2 / (40.0 * R * R * L * L));
    }

//! @returns {f(u) = u * (1 + alpha * u^4/ (10 (4 R R - L L)  L L)}
//! <ul>
//! <li>In use case with {alpha = positive one}, return value is (approximate) distance along spiral and u is distance along axis.
//! <li>In use case with (alpha = negative one}, u is distance along spiral and the return value is (approximate) distance along axis.
//! <li> Calling with {alpha=1} and both R and u equal to the nominal tangent length along x axis produces
//!     a very good approximation for the spiral length.
//! </ul>
double ClothoidCosineApproximation::EvaluateItalianCzechR2L2Map (double alpha, double R, double L, double u)
    {
    double u2 = u * u;
    double L2 = L * L;
    double Q = 4.0 * R * R - L2;
    return u * (1.0 + alpha * u2 * u2 / (10.0 * Q * L2));
    }

//! @returns Given target value f, return u so {u(1 + alpha * u^4/ (10 (4 R R - L L)  L L) = f}
//! <ul>
//! <li>In use case with {alpha = positive one}, f is distance along spiral and the return value is (approximate) distance along axis.
//! <li>In use case with (alpha = negative one}, f is distance along axis and the return value is (approximate) distance along spiral.
//! </ul>
ValidatedDouble ClothoidCosineApproximation::InvertItalianCzechR2L2Map (double alpha, double R, double Lx, double f, double &coff)
    {
    s_evaluationCount = 0;
    double L2 = Lx * Lx;
    double Q = 4.0 * R * R - L2;
    coff = 1.0 / ( 10.0 * Q * L2);
    ClothoidCosineApproximation callback (f,alpha * coff);
    NewtonIterationsRRToRR newton (1.0e-12);
    // use the target f as the initial guess
    bool stat = newton.RunNewton (f, callback);
    return ValidatedDouble (f, stat);
    }

//! @returns {f(u) = u * (1 + gamma * u^4}, along with 3 derivatives as return parameters.
double ClothoidCosineApproximation::EvaluateU4Map(double gamma, double u, double &dfdu, double &d2fdu2, double &d3fdu3)
    {
    double u2 = u * u;
    double u3 = u2 * u;
    double u4 = u2 * u2;
    dfdu = 1.0 + 5.0 * gamma * u4;
    d2fdu2 = 20.0 * gamma * u3;
    d3fdu3 = 60.0 * gamma * u2;
    return u * (1.0 + gamma * u4);
    }

//! return the czech/italian gamma factor {2R/ sqrt(4 R R - L L)}
double ClothoidCosineApproximation::CzechGamma (double R, double L)
    {
    return 2.0 * R / sqrt (4.0 * R * R - L * L);
    }

//! @returns {f(u) = u * (1 + alpha * u^4/ (40 R R L L)}, along with 3 derivatives as return parameters.
//! <ul>
//! <li>In use case with {alpha = positive one}, return value is (approximate) distance along spiral and u is distance along axis.
//! <li>In use case with (alpha = negative one}, u is distance along spiral and the return value is (approximate) distance along axis.
//! </ul>
double ClothoidCosineApproximation::Evaluate40R2L2Map (double alpha, double R, double L, double u, double &dfdu, double &d2fdu2, double &d3fdu3)
    {
    double u2 = u * u;
    double u3 = u2 * u;
    double u4 = u2 * u2;
    double gamma = alpha / (40.0 * R * R * L * L);
    dfdu = 1.0 + 5.0 * gamma * u4;
    d2fdu2 = 20.0 * gamma * u3;
    d3fdu3 = 60.0 * gamma * u2;
    return u * (1.0 + gamma * u4);
    }

// Specialize spiral for DIRECTHALFCOSINE ....


DSpiral2dDirectEvaluation::DSpiral2dDirectEvaluation (double nominalLength) : m_nominalLength (nominalLength) {}
DSpiral2dDirectHalfCosine::DSpiral2dDirectHalfCosine (double projectedLength) : DSpiral2dDirectEvaluation (projectedLength) {}
// STATIC method ...
bool DSpiral2dDirectHalfCosine::EvaluateAtFractionInStandardOrientation
    (
    double u,           //!< [in] fraction along x axis
    double length,      //! [in] strictly nonzero length along spiral.
    double radius1,  //! [in] strictly nonzero exit radius
    DPoint2dR xy,      //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt fractional position
    DVec2dP d2XY,   //!< [out] second derivative wrt fractional position
    DVec2dP d3XY   //!< [out] third derivative wrt fractional position
    )
    {
    double c = length * length / radius1;
    double pi = Angle::Pi ();
    double c1 = 1.0 / (2.0 * pi * pi);
    double c2 = 0.25;
    double upi = u * pi;
    double y = c * (c2 * u * u - c1 * (1.0 - cos (upi)));
    double dydu = c * (2.0 * c2 * u - c1 * pi * sin (upi));
    double d2ydu2 = c * (2.0 * c2 - c1 * pi * pi * cos (upi));
    double d3ydu3 = c * c1 * pi * pi * pi * sin (upi);

    xy.Init (u * length,y);
    if (d1XY)
        d1XY->Init (length, dydu);
    if (d2XY)
        d2XY->Init (0.0, d2ydu2);
    if (d3XY)
        d3XY->Init (0.0, d3ydu3);
        
    return true;
    }

bool DSpiral2dDirectHalfCosine::EvaluateAtFraction
    (
    double fraction, //!< [in] distance for evaluation
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
    ) const
    {
    if (mCurvature0 == 0.0)
        {
        bool stat = EvaluateAtFractionInStandardOrientation (fraction, mLength, 1.0 / mCurvature1, xy, d1XY, d2XY, d3XY);
        if (stat)
            DSpiral2dDirectEvaluation::ApplyCCWRotation (mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }       
    return false;
    }

DSpiral2dBaseP DSpiral2dDirectHalfCosine::Clone () const
    {
    DSpiral2dDirectHalfCosine *pClone = new DSpiral2dDirectHalfCosine (m_nominalLength);
    pClone->CopyBaseParameters (this);
    return pClone;
    }

// Specialize spiral for JAPANEECUBIC


DSpiral2dJapaneseCubic::DSpiral2dJapaneseCubic(double projectedLength) : DSpiral2dDirectEvaluation(projectedLength) {}
// STATIC method ...
bool DSpiral2dJapaneseCubic::EvaluateAtFractionInStandardOrientation
(
    double u,           //!< [in] fraction along x axis
    double length,      //! [in] strictly nonzero length along AXIS
    double radius1,  //! [in] strictly nonzero exit radius
    DPoint2dR xy,      //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt fractional position
    DVec2dP d2XY,   //!< [out] second derivative wrt fractional position
    DVec2dP d3XY   //!< [out] third derivative wrt fractional position
)
    {
    double m = 1.0 / (6.0 * length * radius1);

    double x = u * length;
    xy.Init(x, m * x * x * x);
    if (d1XY)
        d1XY->Init(length, 3.0 * m * x * x * length);
    if (d2XY)
        d2XY->Init(0.0, 6.0 * m * x * length * length);
    if (d3XY)
        d3XY->Init(0.0, 6.0 * m * length * length * length);

    return true;
    }

bool DSpiral2dJapaneseCubic::EvaluateAtFraction
(
    double fraction, //!< [in] distance for evaluation
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
) const
    {
    if (mCurvature0 == 0.0)
        {
        bool stat = EvaluateAtFractionInStandardOrientation(fraction, mLength, 1.0 / mCurvature1, xy, d1XY, d2XY, d3XY);
        if (stat)
            DSpiral2dDirectEvaluation::ApplyCCWRotation(mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }
    return false;
    }

DSpiral2dBaseP DSpiral2dJapaneseCubic::Clone() const
    {
    DSpiral2dJapaneseCubic *pClone = new DSpiral2dJapaneseCubic(m_nominalLength);
    pClone->CopyBaseParameters(this);
    return pClone;
    }

// Specialize spiral for CZECH ....
// NOTE WARNING BOMB the nominal length input is along x axis.
DSpiral2dCzech::DSpiral2dCzech(double xAxisLength) : DSpiral2dDirectEvaluation(xAxisLength) {}
DSpiral2dBaseP DSpiral2dCzech::Clone() const
{
    double nominalLength = m_nominalLength;
    if (nominalLength == 0.0 && mLength != 0.0)
        nominalLength = mLength;
    DSpiral2dCzech *pClone = new DSpiral2dCzech(nominalLength);
    pClone->CopyBaseParameters(this);
    return pClone;
}
//! return true if valid evaluation.
bool DSpiral2dCzech::EvaluateAtFraction
(
    double fraction, //!< [in] fraction for evaluation
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt fraction
    DVec2dP d2XY,   //!< [out] second derivative wrt fraction
    DVec2dP d3XY    //!< [out] third derivative wrt fraction
) const
    {
    if (mCurvature0 == 0.0)
        {
        static bool s_applyRotation = true;
        double Lx = m_nominalLength;
        double R = 1.0 / mCurvature1;
        double Ls = ClothoidCosineApproximation::EvaluateItalianCzechR2L2Map(1.0, R, Lx, Lx);
        bool stat = EvaluateAtFractionOfNominalLengthInStandardOrientation(fraction, Lx, Ls, R, xy, d1XY, d2XY, d3XY);
        ScaleDerivatives(Ls, d1XY, d2XY, d3XY);
        if (stat && s_applyRotation)
            DSpiral2dDirectEvaluation::ApplyCCWRotation(mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }
    return false;
    }
static void check(double a, double b)
    {
    }
//! Evaluate at distance a spiral in standard orientation -- zero curvature at origin.
// STATIC
bool DSpiral2dCzech::EvaluateAtFractionOfNominalLengthInStandardOrientation
(
    double fraction,           //!< [in] fraction
    double Lx,      //! [in] strictly nonzero nominal (x axis) length
    double Ls,      //! [in] strictly nonzero nominal length
    double R,  //! [in] strictly nonzero exit radius
    DPoint2dR xy,      //!< [out] coordinates on spiral
    DVec2dP d1XYds,   //!< [out] first derivative wrt distance
    DVec2dP d2XYds,   //!< [out] second derivative wrt distance
    DVec2dP d3XYds   //!< [out] third derivative wrt distance
)
    {
    // approximation of full spiral length . . . .( this is fixed computation -- no iteration)
    double s = fraction * Ls;
    double gamma = ClothoidCosineApproximation::CzechGamma(R, Lx);
    // x coordinate is inversion of   s = x(1+coffX*x^4)
    double coffX;
    double x = ClothoidCosineApproximation::InvertItalianCzechR2L2Map(1.0, R, Lx, s, coffX);
    double d1sdx, d2sdx, d3sdx;
    double s1 = ClothoidCosineApproximation::EvaluateU4Map (coffX, x, d1sdx, d2sdx, d3sdx);
    check (s1, s);
    double xx = x * x;
    double xxx = xx * x;
    double coffY = gamma / (6.0 * R * Lx);
    if (R < 0.0)
        coffY = - coffY;
    double y = coffY * xxx ;
    // Lx is a constant.
    xy.x = x;
    xy.y = y;
    if (d1XYds) {
        double d1xds, d2xds, d3xds;
        InvertDerivatives (d1sdx, d2sdx, d3sdx, d1xds, d2xds, d3xds);
        double d1ydx = 3.0 * coffY * xx;
        double d2ydx = 6.0 * coffY * x;
        double d3ydx = 6.0 * coffY;
        InvertDerivatives(d1sdx, d2sdx, d3sdx, d1xds, d2xds, d3xds);
        double d1xds2 = d1xds * d1xds;
        double d1yds = d1ydx * d1xds;
        // apply dds to d1yds.   Right side is product rule, and chain rule when dds applies to a dNydx
        double d2yds = d2ydx * d1xds2 + d1ydx * d2xds;
        double d3yds = d3ydx * d1xds2 * d1xds
                     + d2ydx * 2.0 * d1xds * d2xds
                     + d2ydx * d1xds * d2xds
                     + d1ydx * d3xds;
        d1XYds->Init(d1xds, d1yds);
        if (d2XYds)
            d2XYds->Init (d2xds, d2yds);
        if (d3XYds)
            d3XYds->Init (d3xds, d3yds);
        }
    return true;
    }
//////////////////////////////////////// POLISH_CUBIC_PARABOLA
// NOTE Nominal length is along the arc.
// * curve is y = m*x^3, with simple m
// * The distinctive part of POLISH is a high order series approximation for s = f(x).

double PolishDistanceApproximation::SeriesX4Coefficient(double radius1, double length1)
    {
    return 1.0 / (4.0 * radius1 * radius1 * length1 * length1);
    }

double PolishDistanceApproximation::XToYCubicCoefficient(double radius1, double length1)
    {
    return 1.0 / (6.0 * radius1 * length1);
    }
double PolishDistanceApproximation::SeriesX4Term (double x)
    {
    //return pow(partialLengthAlongTangent, 4) / (4.0 * pow(radius, 2) * pow(lengthAlongArc, 2));
    double x2 = x * x;
    return x2 * x2 * m_x4TermCoefficient;
    }

PolishDistanceApproximation::PolishDistanceApproximation(double radius1, double length1)
    {
    m_radius1 = radius1;
    m_length1 = length1;
    m_x4TermCoefficient = SeriesX4Coefficient (m_radius1, m_length1);
    m_cubicYCoefficient = XToYCubicCoefficient (m_radius1, m_length1);
    m_targetDistance = 0.0;
    }

double PolishDistanceApproximation::XToApproximateDistance(double radius1, double length1, double x)
    {               // C31 * ( 1 + 1 / 10 * E31 - 1 / 72 * E31^2 + 1 / 208 * E31^3 - 5 / 2176 * E31^4 )
    double a4 = SeriesX4Coefficient(radius1, length1);
    double Ax4 = a4 * x * x * x * x;
    return x * (1.0 + Ax4 * (1.0 / 10.0 + Ax4 * (- 1.0 / 72.0  + Ax4 * (1.0 / 208.0  - 5.0 * Ax4 / 2176.0))));
    }
//  evaluate polynomial with coefficients for powers 1,5,9,13,17, with (a * x^4)  as the horner term
// (hence a appears at powers 1,2,3,4 on terms with x power 5,9,13,17)
double EvaluateHornerSeries14(double x, double a, double b1, double b5, double b9, double b13, double b17,
double *f1, // 1st derivative
double *f2, // 2nd derivative
double *f3  // 3rd derivative
)
    {
    double ax2 = a * x * x;
    double ax3 = ax2 * x;
    double ax4 = ax3 * x;
    double f = x * (b1 + ax4 * (b5 + ax4 * (b9 + ax4 * (b13 + ax4 * b17))));
    // derivative multipliers 1,5,9,13,17
    if (f1 != nullptr)
        *f1 = b1 + ax4 * (5.0 * b5 + ax4 * ( 9.0 * b9 + ax4 * (13.0 * b13 + ax4 * 17.0 * b17)));
    // derivative multipliers 0,4,8,12,16
    if (f2 != nullptr)
        *f2 = ax3 * (20.0 * b5 + ax4 * (72.0 * b9 + ax4 * (156.0 * b13 + ax4 * 272.0 * b17)));
    // derivative multipliers 3,7,11,15
    if (f3 != nullptr)
        *f3 = ax2 * (60.0 * b5 + ax4 * (504.0 * b9 + ax4 * (1716.0 * b13 + ax4 * 4080.0 * b17)));
    return f;
    }
double PolishDistanceApproximation::XToApproximateDistanceD(double radius1, double length1, double x, double *d1Sdx, double *d2Sdx2, double *d3Sdx3)
    {               // C31 * ( 1 + 1 / 10 * E31 - 1 / 72 * E31^2 + 1 / 208 * E31^3 - 5 / 2176 * E31^4 )
    double a4 = SeriesX4Coefficient(radius1, length1);
    double Ax2 = a4 * x * x;
    double Ax3 = Ax2 * x;
    double Ax4 = Ax3 * x;
    double s0 =  x * (1.0 + Ax4 * (0.1 + Ax4 * (-1.0 / 72.0 + Ax4 * (1.0 / 208.0 - 5.0 * Ax4 / 2176.0))));
    double s = EvaluateHornerSeries14(x, a4, 1.0, 0.1, -1.0/72, 1.0 / 208.0, - 5.0 / 2176.0, d1Sdx, d2Sdx2, d3Sdx3);
    check (s0, s);
    return s;
    }

double PolishDistanceApproximation::XToY(double x)
    {
    return x * x * x * m_cubicYCoefficient;
    }
void PolishDistanceApproximation::SetTargetDistance(double distance)
    {
    m_targetDistance = distance;
    }

bool PolishDistanceApproximation::EvaluateRToRD(double x, double &f, double &dfdx)
    {
    double Aa = SeriesX4Term(x);
    f =  x * (1.0 + Aa * (1.0 / 10.0 + Aa * (-1.0 / 72.0 + Aa * (1.0 / 208.0 - 5.0 * Aa / 2176.0))))   - m_targetDistance;
    dfdx = 1.0 + Aa * (5.0 / 10.0 + Aa * (-9.0 / 72.0 + Aa * (13.0 / 208.0 - 85.0 * Aa / 2176.0)));
    s_evaluationCount++;
    return true;
    }
size_t PolishDistanceApproximation::s_evaluationCount = 0;
/** INSTANCE method (accessign precomputed coefficients) to invert the distance function */
ValidatedDouble PolishDistanceApproximation::InvertXToApproximateDistance(double targetDistance)
    {
    s_evaluationCount = 0;
    NewtonIterationsRRToRR newton(1.0e-12);
    double x = targetDistance;
    SetTargetDistance (targetDistance);
    // use the target f as the initial guess
    bool stat = newton.RunNewton(x, *this);
    return ValidatedDouble(x, stat);
    }
double DSpiral2dPolish::ValidateSeriesInversion()
    {
    double maxDiff = 0.0;
    for (double r1 : { 1000.0, 2000.0})
        {
        for (double l1 : { 100.0, 50.0})
            {
            PolishDistanceApproximation cubic(r1, l1);
            for (double fx : {0.2, 0.4, 0.8, 0.9834, 1.0})
                {
                double x = fx * l1;
                // Verify that forward and reverse distance evaluations match ..
                double distanceA = PolishDistanceApproximation::XToApproximateDistance(r1, l1, x);
                auto xA = cubic.InvertXToApproximateDistance(distanceA);
                maxDiff = DoubleOps::MaxAbs (maxDiff, xA - x);
                }
            }
        }
    return maxDiff;
    }

DSpiral2dPolish::DSpiral2dPolish(double nominalCurveLength) : DSpiral2dDirectEvaluation(nominalCurveLength) {}

DSpiral2dBaseP DSpiral2dPolish::Clone() const
    {
    double nominalLength = m_nominalLength;
    if (nominalLength == 0.0 && mLength != 0.0)
        nominalLength = mLength;
    DSpiral2dPolish *pClone = new DSpiral2dPolish(nominalLength);
    pClone->CopyBaseParameters(this);
    return pClone;
    }
//! return true if valid evaluation.
bool DSpiral2dPolish::EvaluateAtFraction
(
    double fraction, //!< [in] fraction for evaluation
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt fraction
    DVec2dP d2XY,   //!< [out] second derivative wrt fraction
    DVec2dP d3XY    //!< [out] third derivative wrt fraction
) const
    {
    if (mCurvature0 == 0.0)
        {
        static bool s_applyRotation = true;
        double R = 1.0 / mCurvature1;
        double Ls = m_nominalLength;
        double Lx = 0.0;        // Let it get recomputed ... ugh . .. 3 to 5 iterations is pretty good!!!
        bool stat = EvaluateAtFractionOfAxisLengthInStandardOrientation(fraction, Lx, Ls, R, false, xy, d1XY, d2XY, d3XY);
        ScaleDerivatives(Lx, d1XY, d2XY, d3XY);
        if (stat && s_applyRotation)
            DSpiral2dDirectEvaluation::ApplyCCWRotation(mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }
    return false;
    }

//! Return poles for preferred representation as a bezier curve
//! These are in the local coordinates of the standard orientation
//! 
bool DSpiral2dPolish::GetBezierPoles
(
bvector<DPoint3d> &poles,   //!< [out] poles
double startFraction, //!< [in] start fraction for active intervale
double endFraction //!< [in] end fraction for active interval
)
    {
    DPoint2d xy1;
    double R = 1.0 / mCurvature1;
    double Ls = m_nominalLength;
    double Lx = 0.0;        // Let it get recomputed ... ugh . .. 3 to 5 iterations is pretty good!!!
    bool stat = EvaluateAtFractionOfAxisLengthInStandardOrientation(1.0, Lx, Ls, R, false, xy1, nullptr, nullptr, nullptr);
    if (stat)
        {
        poles.push_back(DPoint3d::From (0,0,0));
        poles.push_back(DPoint3d::From (xy1.x / 3.0, 0.0));
        poles.push_back(DPoint3d::From(2.0 * xy1.x / 3.0, 0.0));
        poles.push_back(DPoint3d::From (xy1.x, xy1.y, 0.0));
        bsiBezierDPoint3d_subdivideToIntervalInPlace (&poles[0], 4, startFraction, endFraction);
        if (mTheta0 != 0.0)
            {
            auto transform = Transform::From (RotMatrix::FromAxisAndRotationAngle(2, mTheta0));
            transform.Multiply (poles);
            }
        return true;
        }
    return false;
    }

//! Evaluate at distance a spiral in standard orientation -- zero curvature at origin.
// STATIC
bool DSpiral2dPolish::EvaluateAtFractionOfAxisLengthInStandardOrientation
(
    double fraction,           //!< [in] fraction
    double &Lx,      //! [out] x axis.  Recomputed from L1 and R1
    double L1,      //! [in] strictly nonzero nominal length
    double R1,  //! [in] strictly nonzero exit radius
    bool mapDerivativesWRTSeriesDistance, //< [in] if true, map derivatives wrt power series distance.  If false wrt axis distance
    DPoint2dR xy,      //!< [out] coordinates on spiral
    DVec2dP d1XYds,   //!< [out] first derivative wrt distance or fraction, per mapDerivativesWRTSeriesDistance
    DVec2dP d2XYds,   //!< [out] second derivative wrt distance or fraction, per mapDerivativesWRTSeriesDistance
    DVec2dP d3XYds   //!< [out] third derivative wrt distance or fraction, per mapDerivativesWRTSeriesDistance
)
    {
    double coffY = PolishDistanceApproximation::XToYCubicCoefficient(R1, L1);
    if (Lx == 0.0)
        {
        PolishDistanceApproximation cubic(R1, L1);
        auto validatedLx = cubic.InvertXToApproximateDistance(L1);
        if (!validatedLx.IsValid ())
            {
            xy.Init (0,0);
            return false;
            }
        Lx = validatedLx.Value ();
        }
    double x = fraction * Lx;
    double xx = x * x;
    xy.Init (x, coffY * x * x * x);
    if (d1XYds == nullptr && d2XYds == nullptr && d3XYds == nullptr)
        return true;
    if (!mapDerivativesWRTSeriesDistance)
        {
        if (d1XYds)
            {
            double d1ydx = 3.0 * coffY * xx;
            double d2ydx = 6.0 * coffY * x;
            double d3ydx = 6.0 * coffY;
            d1XYds->Init (1.0, d1ydx);
            if (d2XYds)
                d2XYds->Init (0.0, d2ydx);
            if (d3XYds)
                d3XYds->Init(0.0, d3ydx);
            }
        return true;
        }
    double d1sdx, d2sdx, d3sdx;
    PolishDistanceApproximation::XToApproximateDistanceD (R1, L1, x, &d1sdx, &d2sdx, &d3sdx);   // Throw away s !!!!
    if (d1XYds) {
        double d1xds, d2xds, d3xds;
        InvertDerivatives(d1sdx, d2sdx, d3sdx, d1xds, d2xds, d3xds);
        double d1ydx = 3.0 * coffY * xx;
        double d2ydx = 6.0 * coffY * x;
        double d3ydx = 6.0 * coffY;

        double d1xds2 = d1xds * d1xds;
        double d1yds = d1ydx * d1xds;
        // apply dds to d1yds.   Right side is product rule, and chain rule when dds applies to a dNydx
        double d2yds = d2ydx * d1xds2 + d1ydx * d2xds;
        double d3yds = d3ydx * d1xds2 * d1xds
            + d2ydx * 2.0 * d1xds * d2xds
            + d2ydx * d1xds * d2xds
            + d1ydx * d3xds;
        d1XYds->Init(d1xds, d1yds);
        if (d2XYds)
            d2XYds->Init(d2xds, d2yds);
        if (d3XYds)
            d3XYds->Init(d3xds, d3yds);
        }
    return true;
    }
#ifdef CivAlgCode_Polish

static double aecAlg_computePolishAa(double radius, double lengthAlongArc, double partialLengthAlongTangent)
    {               // D6^4 / ( 4 * D5^2 * D6^2 )
    //return pow(partialLengthAlongTangent, 4) / (4.0 * pow(radius, 2) * pow(lengthAlongArc, 2));
    double x2 = partialLengthAlongTangent * partialLengthAlongTangent;
    return x2 * x2 / (4.0 * radius * radius * lengthAlongArc * lengthAlongArc);
    }

static double aecAlg_computePolishPartialLs(double radius, double lengthAlongArc, double partialLengthAlongTangent)
    {               // C31 * ( 1 + 1 / 10 * E31 - 1 / 72 * E31^2 + 1 / 208 * E31^3 - 5 / 2176 * E31^4 )
    double Aa = aecAlg_computePolishAa(radius, lengthAlongArc, partialLengthAlongTangent);

    return partialLengthAlongTangent * (1.0 + 1.0 / 10.0 * Aa - 1.0 / 72.0 * pow(Aa, 2) + 1.0 / 208.0 * pow(Aa, 3) - 5.0 / 2176.0 * pow(Aa, 4));
    }
static double aecAlg_computePolishPartialLs_noPow(double radius, double lengthAlongArc, double partialLengthAlongTangent)
    {               // C31 * ( 1 + 1 / 10 * E31 - 1 / 72 * E31^2 + 1 / 208 * E31^3 - 5 / 2176 * E31^4 )
    double Aa = aecAlg_computePolishAa(radius, lengthAlongArc, partialLengthAlongTangent);
    double s = -5.0 / 2176.0 * Aa;
    s = Aa * (s + 1.0 / 208.0);
    s = Aa * (s - 1.0 / 72.0);
    s = Aa * (s + 1.0 / 10.0);
    s += 1.0;
    return partialLengthAlongTangent * s;
    }

static double aecAlg_computePolishYs(double radius, double totalArcLength, double partialLengthAlongTangent)
    {               // C31^3 / ( 6 * D5 * D6 )
    return pow(partialLengthAlongTangent, 3) / (6.0 * radius * totalArcLength);
    }



static double aecAlg_computePolishYs(double radius, double totalArcLength)
    {               // C31^3 / ( 6 * D5 * D6 )
    double lengthAlongTangent = aecAlg_computePolishXs(radius, totalArcLength);

    return pow(lengthAlongTangent, 3) / (6.0 * radius * totalArcLength);
    }

static double aecAlg_computePolishXs(double radius, double totalArcLength)
    {
    int cycles = 0, index;
    double arcLength[3], tangentLength[3] = { 0.0, 0.0, 0.0 }, differences[3];

    tangentLength[0] = 0.8 * totalArcLength;
    arcLength[0] = aecAlg_computePolishPartialLs(radius, totalArcLength, tangentLength[0]);
    differences[0] = arcLength[0] - totalArcLength;

    tangentLength[2] = 1.2 * totalArcLength;
    arcLength[2] = aecAlg_computePolishPartialLs(radius, totalArcLength, tangentLength[2]);
    differences[2] = arcLength[2] - totalArcLength;

    while ((fabs(arcLength[0] - arcLength[2]) > LENGTH_TOLERANCE) && (++cycles < 100))
        {
        tangentLength[1] = .5 * (tangentLength[0] + tangentLength[2]);
        arcLength[1] = aecAlg_computePolishPartialLs(radius, totalArcLength, tangentLength[1]);
        differences[1] = arcLength[1] - totalArcLength;

        if (LIKE_SIGNS(differences[0], differences[2]))
            index = (fabs(differences[0]) < fabs(differences[2])) ? 2 : 0;
        else
            index = (LIKE_SIGNS(differences[1], differences[2])) ? 2 : 0;

        differences[index] = differences[1];
        arcLength[index] = arcLength[1];
        tangentLength[index] = tangentLength[1];
        }

    tangentLength[1] = .5 * (tangentLength[0] + tangentLength[2]);

    return tangentLength[1];
    }

static double aecAlg_computePolishTheta(double radius, double totalArcLength, double partialArcLength)
    {
    return pow(partialArcLength, 2) / (2.0 * radius * totalArcLength);
    }

static double aecAlg_computePolishPartialTangentFromPartialLength(double radius, double lengthAlongTangent, double totalArcLength, double partialArcLength)
    {
    int cycles = 0, index;
    double arcLength[3], tangentLength[3] = { 0.0, 0.0, 0.0 }, differences[3];

    tangentLength[0] = 0.8 * partialArcLength;
    arcLength[0] = aecAlg_computePolishPartialLs(radius, totalArcLength, tangentLength[0]);
    differences[0] = arcLength[0] - partialArcLength;

    tangentLength[2] = 1.2 * partialArcLength;
    arcLength[2] = aecAlg_computePolishPartialLs(radius, totalArcLength, tangentLength[2]);
    differences[2] = arcLength[2] - partialArcLength;

    while ((fabs(arcLength[0] - arcLength[2]) > LENGTH_TOLERANCE) && (++cycles < 100))
        {
        tangentLength[1] = .5 * (tangentLength[0] + tangentLength[2]);
        arcLength[1] = aecAlg_computePolishPartialLs(radius, totalArcLength, tangentLength[1]);
        differences[1] = arcLength[1] - partialArcLength;

        if (LIKE_SIGNS(differences[0], differences[2]))
            index = (fabs(differences[0]) < fabs(differences[2])) ? 2 : 0;
        else
            index = (LIKE_SIGNS(differences[1], differences[2])) ? 2 : 0;

        differences[index] = differences[1];
        arcLength[index] = arcLength[1];
        tangentLength[index] = tangentLength[1];
        }

    tangentLength[1] = .5 * (tangentLength[0] + tangentLength[2]);

    return tangentLength[1];
    }

static void UpdateMaxAbsDiff(double &maxDiff, double a, double b)
    {
    maxDiff = DoubleOps::MaxAbs(maxDiff, b - a);
    }

double DSpiral2dPolish::ValidateCodePort()
    {
    double maxDiff1 = 0.0;
    double maxDiff2 = 0.0;
    double maxDiff3 = 0.0;
    double maxDiff4 = 0.0;
    double maxDiff5 = 0.0;
    double maxDiff23 = 0.0;
    for (double r1 : { 1000.0, 2000.0})
        {
        for (double l1 : { 100.0, 50.0})
            {
            PolishDistanceApproximation cubic(r1, l1);
            for (double fx : {0.2, 0.8, 1.0})
                {
                double x = fx * l1;
                double distance, dDistanceDX;
                UpdateMaxAbsDiff(maxDiff1, aecAlg_computePolishAa(r1, l1, x), cubic.SeriesX4Term(x));
                double distance1 = PolishDistanceApproximation::XToApproximateDistance(r1, l1, x);
                cubic.SetTargetDistance(0.0);
                cubic.EvaluateRToRD(x, distance, dDistanceDX);
                double distance2 = aecAlg_computePolishPartialLs(r1, l1, x);
                UpdateMaxAbsDiff(maxDiff2, distance2, distance);
                double distance3 = aecAlg_computePolishPartialLs_noPow(r1, l1, x);
                UpdateMaxAbsDiff(maxDiff23, distance2, distance3);

                UpdateMaxAbsDiff(maxDiff3, aecAlg_computePolishYs(r1, l1, x), cubic.XToY(x));
                UpdateMaxAbsDiff(maxDiff4, distance, distance1);
                auto xA = cubic.InvertXToApproximateDistance(distance1);
                UpdateMaxAbsDiff(maxDiff5, xA, x);
                }
            }
        }
    return DoubleOps::MaxAbs(maxDiff1, maxDiff2, DoubleOps::MaxAbs(maxDiff3, maxDiff4, maxDiff5));
    }

#endif


END_BENTLEY_GEOMETRY_NAMESPACE
