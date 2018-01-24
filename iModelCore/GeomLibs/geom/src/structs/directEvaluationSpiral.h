/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/directEvaluationSpiral.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#ifdef CompileCZECH
// Specialize spiral for CZECH ....
DSpiral2dCzech::DSpiral2dCzech () : DSpiral2dBase () {}
DSpiral2dBaseP DSpiral2dCzech::Clone () const
    {
    DSpiral2dCzech *pClone = new DSpiral2dCzech ();
    pClone->CopyBaseParameters (this);
    return pClone;
    }
double DSpiral2dCzech::DistanceToLocalAngle (double distance) const
    {
    double u = DistanceToFraction (distance);
    return   mLength * u * (mCurvature0
           + u * u * (1.0 - 0.5 * u) * (mCurvature1 - mCurvature0));
    }

double DSpiral2dCzech::DistanceToCurvature (double distance) const
    {
    double u = DistanceToFraction (distance);
    double f = u * u * (3.0 - 2.0 * u);
    return mCurvature0 + f * (mCurvature1 - mCurvature0);
    }

double DSpiral2dCzech::DistanceToCurvatureDerivative (double distance) const
    {
    if (mLength == 0.0)
        return 0.0;
    double u = DistanceToFraction (distance);
    double dfdu = 6.0 * u * (1.0 - u);
    return dfdu * (mCurvature1 - mCurvature0) / mLength;
    }
#endif
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


double DSpiral2dDirectEvaluation::DistanceToLocalAngle (double distance) const
    {
    DPoint2d xy;
    DVec2d d1xy;
    EvaluateAtDistance (distance, xy, &d1xy, nullptr, nullptr);
    return   atan2 (d1xy.y, d1xy.x) - mTheta0;
    }

double DSpiral2dDirectEvaluation::DistanceToCurvature (double distance) const
    {
    DPoint2d xy;
    DVec2d d1xy, d2xy;
    EvaluateAtDistance (distance, xy, &d1xy, &d2xy, nullptr);
    return parametricCurvature (d1xy.x, d1xy.y, d2xy.x, d2xy.y);
    }

double DSpiral2dDirectEvaluation::DistanceToCurvatureDerivative (double distance) const
    {
    DPoint2d xy;
    DVec2d d1xy, d2xy, d3xy;
    EvaluateAtDistance (distance, xy, &d1xy, &d2xy, &d3xy);
    return parametricCurvatureDerivative (d1xy.x, d1xy.y, d2xy.x, d2xy.y, d3xy.x, d3xy.y);
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

/*-----------------------------------------------------------------*//**
@description BSIVectorIntegrand query function.
@param distance IN distance parameter
@param pF OUT array of two doubles x,y for integration.
+---------------+---------------+---------------+---------------+------*/
void DSpiral2dDirectEvaluation::EvaluateVectorIntegrand (double distance, double *pF)
    {
    DPoint2d xy;
    DVec2d dxy;
    EvaluateAtDistance (distance, xy, &dxy, nullptr, nullptr);
    pF[0] = dxy.x;
    pF[1] = dxy.y;
    }



// Specialize spiral for NEWSOUTHWALES ....
DSpiral2dNewSouthWales::DSpiral2dNewSouthWales () : DSpiral2dDirectEvaluation () {}
// STATIC method
bool DSpiral2dNewSouthWales::EvaluateAtDistanceInStandardOrientation
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
        d3XY->Init (-60.0 * factorX, 6.0 * factorY);
    return true;
    }

bool DSpiral2dNewSouthWales::EvaluateAtDistance
    (
    double s, //!< [in] distance for evaluation
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
    ) const
    {
    if (mCurvature0 == 0.0)
        {
        static bool s_applyRotation = true;
        bool stat = EvaluateAtDistanceInStandardOrientation (s, mLength, mCurvature1, xy, d1XY, d2XY, d3XY);
        if (stat && s_applyRotation)
            ApplyCCWRotation (mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }
    return false;
    }
DSpiral2dBaseP DSpiral2dNewSouthWales::Clone () const
    {
    DSpiral2dNewSouthWales *pClone = new DSpiral2dNewSouthWales ();
    pClone->CopyBaseParameters (this);
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
    expr3 = DEGRAD( 240. );

    phi = asin( expr1 * cos( acos( expr2 ) / 3. + expr3 ));

    return( phi );
}

double aecAlg_computeAustralianPhiFromXcR( double R, double xc )
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

DSpiral2dAustralian::DSpiral2dAustralian () : DSpiral2dDirectEvaluation () {}
// STATIC method ...
bool DSpiral2dAustralian::EvaluateAtDistanceInStandardOrientation
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
    double xc = aecAlg_computeAustralianXcFromRL (length, radius1);
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
            double d2yds2 = m * (6.0 * x * dxds + 3.0 * x * x * d2xds2);
            d2XY->Init (d2xds2, d2yds2);
            if (d3XY)
                {
                double m2s2 = m * m * s * s;
                double d3xds3 = -m2s2 * (60.0 * a1 - m2s4 * (504.0 * a2 - m2s4 * (1716.0 * a3 - m2s4 * 4080.0 * a4 )));
                double d3yds3 = m * (6.0 * dxds + 12.0 * x * d2xds2 + 3.0 * x * x * d3xds3);
                d2XY->Init (d3xds3, d3yds3);
                }
            }
        }
        
    return true;
    }

bool DSpiral2dAustralian::EvaluateAtDistance
    (
    double s, //!< [in] distance for evaluation
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
    ) const
    {
    if (mCurvature0 == 0.0)
        {
        bool stat = EvaluateAtDistanceInStandardOrientation (s, mLength, mCurvature1, xy, d1XY, d2XY, d3XY);
        if (stat)
            ApplyCCWRotation (mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }       
    return false;
    }
DSpiral2dBaseP DSpiral2dAustralian::Clone () const
    {
    DSpiral2dAustralian *pClone = new DSpiral2dAustralian ();
    pClone->CopyBaseParameters (this);
    return pClone;
    }


// Specialize spiral for MXCubic ....
DSpiral2dMXCubic::DSpiral2dMXCubic () : DSpiral2dDirectEvaluation () {}
// STATIC method
bool DSpiral2dMXCubic::EvaluateAtDistanceInStandardOrientation
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
        if (d1XY)
            d1XY->Init (dx1, dy1 * dx1);

        if (d2XY)
            d2XY->Init (dx2, dy2 * dx1 * dx1 + dy1 * dx2);

        if (d3XY)
            d3XY->Init (dx3,  dy3 * dx1 * dx1 * dx1 + 3.0 * dy2 * dx1 * dx2 + dy1 * dx3);
        }
    return true;
    }

bool DSpiral2dMXCubic::EvaluateAtDistance
    (
    double s, //!< [in] distance for evaluation
    DPoint2dR xy,          //!< [out] coordinates on spiral
    DVec2dP d1XY,   //!< [out] first derivative wrt distance
    DVec2dP d2XY,   //!< [out] second derivative wrt distance
    DVec2dP d3XY   //!< [out] third derivative wrt distance
    ) const
    {
    if (mCurvature0 == 0.0)
        {
        static bool s_applyRotation = true;
        bool stat = EvaluateAtDistanceInStandardOrientation (s, mLength, mCurvature1, xy, d1XY, d2XY, d3XY);
        if (stat && s_applyRotation)
            ApplyCCWRotation (mTheta0, xy, d1XY, d2XY, d3XY);;
        return stat;
        }
    return false;
    }
DSpiral2dBaseP DSpiral2dMXCubic::Clone () const
    {
    DSpiral2dMXCubic *pClone = new DSpiral2dMXCubic ();
    pClone->CopyBaseParameters (this);
    return pClone;
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
ValidatedDouble ClothoidCosineApproximation::Invert40R2L2Map (double alpha, double R, double L, double f)
    {
    s_evaluationCount = 0;
    ClothoidCosineApproximation callback (f,alpha / (40.0 * R * R * L * L));
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
END_BENTLEY_GEOMETRY_NAMESPACE
