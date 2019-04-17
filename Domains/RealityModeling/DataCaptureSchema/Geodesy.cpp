/*--------------------------------------------------------------------------------------+
|
|  Adaptation of code found in A3D Libraries (A3D/Geometry/ )
|  Copyright (C) acute3d.com
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DataCaptureSchemaInternal.h"

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

// Parameters from http://en.wikipedia.org/wiki/Geodetic_system
Ellipsoid Ellipsoid::WGS84()
    {
    return Ellipsoid(6378137.0, 298.257223563);
    }
Ellipsoid Ellipsoid::GRS80()
    {
    return Ellipsoid(6378137.0, 298.257222101);
    }

// Parameters from http://fr.wikipedia.org/wiki/Projection_conique_conforme_de_Lambert
ConformalConicProjection ConformalConicProjection::Lambert93()
    {
    return ConformalConicProjection(46.5, 44.0, 49.0, 3.0, 700000.0, 6600000.0, Ellipsoid::GRS80());
    }
ConformalConicProjection ConformalConicProjection::RGF93_CC(int i)
    {
    assert(i >= 42 && i <= 50);
    return ConformalConicProjection(i, i - .75, i + .75, 3.0, 1700000.0, 1000000.*i - 40800000., Ellipsoid::GRS80());
    }

Ellipsoid::Ellipsoid(double _a, double _invf) : a(_a), invf(_invf)
    {
    b = (a * (invf - 1.)) / invf;
    e = sqrt((a*a - b*b) / (a*a));
    eprime = sqrt((a*a - b*b) / (b*b));
    }


// Convert latitude,longitude,altitude (in degree) in the WGS84 system to/from cartesian coordinates in the Earth-centered Earth-fixed cartesian referential
DPoint3d Ellipsoid::LLH2ECEF(GeoPointCR llh) const
    {
    const double
        cos_phi = cos(Angle::DegreesToRadians(llh.latitude)),
        sin_phi = sin(Angle::DegreesToRadians(llh.latitude)),
        cos_lambda = cos(Angle::DegreesToRadians(llh.longitude)),
        sin_lambda = sin(Angle::DegreesToRadians(llh.longitude)),
        N = a / sqrt(1. - (e*sin_phi)*(e*sin_phi)),
        h = llh.elevation;

    DPoint3d result(DPoint3d::From((N + h) * cos_phi * cos_lambda,
                                   (N + h) * cos_phi * sin_lambda,
                                   ((b*b) * N / (a*a) + h) * sin_phi));
    return result;
    }


GeoPoint Ellipsoid::ECEF2LLH(DPoint3dCR XYZ) const
    {
    const double
        X = XYZ.x, Y = XYZ.y, Z = XYZ.z,
        p = sqrt(X*X + Y*Y),
        theta = atan((Z * a) / (p * b)),
        phi = atan((Z + eprime*eprime*b*pow(sin(theta), 3.)) / (p - e*e*a*pow(cos(theta), 3.))),
        cos_phi = cos(phi), sin_phi = sin(phi),
        N = a / sqrt(1. - (e*sin_phi)*(e*sin_phi));

    GeoPoint result;
    result.Init(
        Angle::RadiansToDegrees (atan2(Y, X)),
        Angle::RadiansToDegrees(phi),
        Angle::RadiansToDegrees(p / cos_phi - N));

    return result;
    }


// From ECEF to local ENU referential and inverse (in degree)
Transform Ellipsoid::ECEF2ENU(double latitude, double longitude) const
    {
    RotMatrix rot = ContextCaptureFacility::EastNorthUp(Angle::DegreesToRadians(latitude), Angle::DegreesToRadians(longitude));
    rot.Transpose();
    GeoPoint llh;
    llh.Init(longitude, latitude, 0.0);
    DPoint3d center = LLH2ECEF(llh);
    return Transform::From(rot, center);
    }

Transform Ellipsoid::ENU2ECEF(double latitude, double longitude) const
    {
    Transform  enu2ECEFTransform;
    enu2ECEFTransform.InverseOf(ECEF2ENU(latitude, longitude));
    return enu2ECEFTransform;
    }


ConformalConicProjection::ConformalConicProjection(double _phi0, double _phi1, double _phi2, double _lambda0, double _X0, double _Y0, Ellipsoid _ellipsoid) : X0(_X0), Y0(_Y0), ellipsoid(_ellipsoid)
    {
    phi0 = Angle::DegreesToRadians(_phi0);
    phi1 = Angle::DegreesToRadians(_phi1);
    phi2 = Angle::DegreesToRadians(_phi2);
    lambda0 = Angle::DegreesToRadians(_lambda0);

    const double
        cos1 = cos(phi1), sin1 = sin(phi1), cos2 = cos(phi2), sin2 = sin(phi2), sin0 = sin(phi0),
        esin1 = ellipsoid.e * sin1, esin2 = ellipsoid.e * sin2, esin0 = ellipsoid.e * sin0,
        pi4 = Angle::Pi() / 4.;
    n =
        (log(cos2 / cos1) + log((1. - esin1*esin1) / (1. - esin2*esin2)) / 2.)
        /
        log(
        (tan(phi1 / 2. + pi4) * pow((1. - esin1) * (1. + esin2), ellipsoid.e / 2.)) /
        (tan(phi2 / 2. + pi4) * pow((1. + esin1) * (1. - esin2), ellipsoid.e / 2.))
        );

    rho_0 =
        ellipsoid.a * cos1 *
        pow(tan(phi1 / 2. + pi4) * pow((1 - esin1) / (1 + esin1), ellipsoid.e / 2.), n)
        / (n * sqrt(1. - esin1*esin1));

    rho0 = rho_0 * pow(pow((1. + esin0) / (1. - esin0), ellipsoid.e / 2.) / tan(phi0 / 2. + pi4), n);
    }


// Convert conformal conic coordinates to geodetic latitude,longitude,altitude (in radians)
GeoPoint ConformalConicProjection::CCP2LLH(DPoint3dCR XYZ) const
    {
    const double
        X = XYZ.x, Y = XYZ.y,
        dX = X - X0, dY = Y0 - Y + rho0,
        rho = sqrt(dX*dX + dY*dY),
        theta = atan(dX / dY),
        lambda = theta / n + lambda0,
        rho_0_rho = pow(rho_0 / rho, 1. / n);

    double phi = 2. * atan(rho_0_rho) - Angle::Pi() / 2.;
    for (int i = 0; i < 10; i++) // Convergence is usually attained earlier
        {
        const double esin = ellipsoid.e * sin(phi);
        phi = 2. * atan(rho_0_rho * pow((1. + esin) / (1. - esin), ellipsoid.e / 2.)) - Angle::Pi() / 2.;
        }

    GeoPoint result;
    result.Init(Angle::RadiansToDegrees(lambda), Angle::RadiansToDegrees(phi), XYZ.z);
    return result;
    }


DPoint3d ConformalConicProjection::LLH2CCP(GeoPointCR llh) const
    {
    const double
        phi = Angle::DegreesToRadians(llh.latitude), lambda = Angle::DegreesToRadians(llh.longitude), z = llh.elevation,
        theta = n * (lambda - lambda0),
        esin = ellipsoid.e * sin(phi),
        pi4 = Angle::Pi() / 4.,
        rho = rho_0 * pow(pow((1. + esin) / (1. - esin), ellipsoid.e / 2.) / tan(phi / 2. + pi4), n);

    DPoint3d result;
    result.Init(
        X0 + rho * sin(theta),
        Y0 + rho0 - rho * cos(theta),
        z);
    return result;
    }


// Compute North-East-Down and East-North-Up referential at a given geodetic latitude,longitude (in radians)
// The matrix maps vectors in local tangent plane coordinates to cartesian coordinates
RotMatrix ContextCaptureFacility::NorthEastDown(double lat, double lon)
    {
    const double
        cos_phi = cos(lat),
        sin_phi = sin(lat),
        cos_lambda = cos(lon),
        sin_lambda = sin(lon);

    RotMatrix M;
    M.Zero();
    M.form3d[0][0] = -sin_phi * cos_lambda;
    M.form3d[0][1] = -sin_phi * sin_lambda;
    M.form3d[0][2] = cos_phi;
    M.form3d[1][0] = -sin_lambda;
    M.form3d[1][1] = cos_lambda;
    M.form3d[2][0] = -cos_phi * cos_lambda;
    M.form3d[2][1] = -cos_phi * sin_lambda;
    M.form3d[2][2] = -sin_phi;

    M.Transpose();
    return M;
    }


RotMatrix ContextCaptureFacility::EastNorthUp(double lat, double lon)
    {
    const double
        cos_phi = cos(lat),
        sin_phi = sin(lat),
        cos_lambda = cos(lon),
        sin_lambda = sin(lon);

    RotMatrix M;
    M.Zero();

    M.form3d[0][0] = -sin_lambda;
    M.form3d[0][1] = cos_lambda;
    M.form3d[1][0] = -sin_phi * cos_lambda;
    M.form3d[1][1] = -sin_phi * sin_lambda;
    M.form3d[1][2] = cos_phi;
    M.form3d[2][0] = cos_phi * cos_lambda;
    M.form3d[2][1] = cos_phi * sin_lambda;
    M.form3d[2][2] = sin_phi;

    M.Transpose();
    return M;
    }

// ==================================================================================

// Euler angles (in radians) to rotation matrix
// More precisely, Tait-Bryan angles, Z(rz) Y(ry) X(rx) (http://en.wikipedia.org/wiki/Euler_angles)
RotMatrix ContextCaptureFacility::EulerZYX2Matrix(double rx, double ry, double rz)
    {
    const double
        c1 = cos(rz),
        s1 = sin(rz),
        c2 = cos(ry),
        s2 = sin(ry),
        c3 = cos(rx),
        s3 = sin(rx);

    RotMatrix M;
    M.form3d[0][0] = c1*c2;
    M.form3d[0][1] = c1*s2*s3 - s1*c3;
    M.form3d[0][2] = s1*s3 + c1*s2*c3;
    M.form3d[1][0] = s1*c2;
    M.form3d[1][1] = s1*s2*s3 + c1*c3;
    M.form3d[1][2] = s1*s2*c3 - c1*s3;
    M.form3d[2][0] = -s2;
    M.form3d[2][1] = c2*s3;
    M.form3d[2][2] = c2*c3;
    return M;
    }

void ContextCaptureFacility::Matrix2EulerZYX(RotMatrixCR M, double& rx, double& ry, double& rz)
    {
    rx = atan2(M.form3d[2][1], M.form3d[2][2]);
    ry = -asin(M.form3d[2][0]);
    rz = atan2(M.form3d[1][0], M.form3d[0][0]);
    }


// Euler angles (in radians) to rotation matrix
// More precisely, Tait-Bryan angles, Y(ry) X(rx) Z(rz) (http://en.wikipedia.org/wiki/Euler_angles)
RotMatrix ContextCaptureFacility::EulerYXZ2Matrix(double rx, double ry, double rz)
    {
    const double
        c1 = cos(ry),
        s1 = sin(ry),
        c2 = cos(rx),
        s2 = sin(rx),
        c3 = cos(rz),
        s3 = sin(rz);

    RotMatrix M;
    M.form3d[0][0] = c1*c3 + s1*s2*s3;
    M.form3d[0][1] = c3*s1*s2 - c1*s3;
    M.form3d[0][2] = c2*s1;
    M.form3d[1][0] = c2*s3;
    M.form3d[1][1] = c2*c3;
    M.form3d[1][2] = -s2;
    M.form3d[2][0] = c1*s2*s3 - c3*s1;
    M.form3d[2][1] = s1*s3 + c1*c3*s2;
    M.form3d[2][2] = c1*c2;
    return M;
    }

void ContextCaptureFacility::Matrix2EulerYXZ(RotMatrixCR M, double& rx, double& ry, double& rz)
    {
    rx = -asin(M.form3d[1][2]);
    ry = atan2(M.form3d[0][2], M.form3d[2][2]);
    rz = atan2(M.form3d[1][0], M.form3d[1][1]);
    }

// Euler angles (in radians) to rotation matrix
// More precisely, Tait-Bryan angles, Z(rz) X(rx) Y(ry) (http://en.wikipedia.org/wiki/Euler_angles)
RotMatrix ContextCaptureFacility::EulerZXY2Matrix(double rx, double ry, double rz)
    {
    const double
        c1 = cos(rz),
        s1 = sin(rz),
        c2 = cos(rx),
        s2 = sin(rx),
        c3 = cos(ry),
        s3 = sin(ry);

    RotMatrix M;
    M.form3d[0][0] = c1*c3 - s1*s2*s3;
    M.form3d[0][1] = -c2*s1;
    M.form3d[0][2] = c1*s3 + c3*s1*s2;
    M.form3d[1][0] = c3*s1 + c1*s2*s3;
    M.form3d[1][1] = c1*c2;
    M.form3d[1][2] = s1*s3 - c1*c3*s2;
    M.form3d[2][0] = -c2*s3;
    M.form3d[2][1] = s2;
    M.form3d[2][2] = c2*c3;
    return M;
    }

void ContextCaptureFacility::Matrix2EulerZXY(RotMatrixCR M, double& rx, double& ry, double& rz)
    {
    rx = asin(M.form3d[2][1]);
    ry = -atan2(M.form3d[2][0], M.form3d[2][2]);
    rz = -atan2(M.form3d[0][1], M.form3d[1][1]);
    }

// ======= O,P,K ================================================================================

// Omega,phi,kappa angles (in radian) to rotation matrix
RotMatrix ContextCaptureFacility::OmegaPhiKappa2Matrix(double omega, double phi, double kappa)
    {
    return EulerZYX2Matrix(-omega, -phi, -kappa);
    }

void ContextCaptureFacility::Matrix2OmegaPhiKappa(RotMatrixCR M, double& omega, double& phi, double& kappa)
    {
    double rx, ry, rz;
    Matrix2EulerZYX(M, rx, ry, rz);
    omega = -rx;
    phi = -ry;
    kappa = -rz;
    }

// ======== H,P,R ===============================================================================

// CC convention. 0,0,0 = looking above (z direction)
RotMatrix ContextCaptureFacility::HeadingPitchRoll2Matrix(double heading, double pitch, double roll)
    {
    return EulerYXZ2Matrix(-pitch, -roll, heading);
    }

void ContextCaptureFacility::Matrix2HeadingPitchRoll(RotMatrixCR M, double& heading, double& pitch, double& roll)
    {
    double rx, ry, rz;
    Matrix2EulerYXZ(M, rx, ry, rz);
    pitch = -rx;
    roll = -ry;
    heading = rz;
    }

// ========== Y,P,R =============================================================================

// 0,0,0 = looking horizontally in y direction, z=top, x=right (If georeferenced XYZ=ENU, this means looking north horizontally)
// Order: 
// 1) turn right yaw radians (z-)
// 2) turn up pitch radians (x+)
// 3) tilt roll radians counter clockwise (y-) (NB: might be the opposite, waiting for a non zero roll to check!)
// Warning: in RADIAN!

// Worked with:
//
// I) tags: Xmp.drone-dji.GimbalYawDegree, Xmp.drone-dji.GimbalPitchDegree, Xmp.drone-dji.GimbalRollDegree
//    use directly:
//			DJI Phantom 4 Pro with DJI FC6310 (warning for one dataset) (Datasets: David, iGlob tower cell)
//			DJI Inspire with DJI FC550 (Datasets: iGlob tower cell)
//			DJI Inspire with DJI FC350 (Datasets: barrage_CC)
//			DJI Inspire with DJI FC330 (Datasets: cell tower peter fisher)
//	  apply yaw -> yaw + 180:
//			One data set from DJI Phantom 4 Pro with DJI FC6310 (iGlob cell tower 2) 
//
//  II) tags: Xmp.Camera.Yaw, Xmp.Camera.Pitch, Xmp.Camera.Roll
//		apply pitch -> - pitch:
//  		DJI PHANTOM VISION with DJI FC200 (Datasets: Pix4D house)
//		apply pitch -> pitch - 90:
//			Sensefly eBee with Sony DSC-WX220 (Dataset: David)
//			Sensefly eXom (Dataset: GSM tower)

// Did not work with:
//
// - Parrot Bebop 2 (tags Xmp.Camera.Yaw, Xmp.Camera.Pitch, Xmp.Camera.Roll): should probably add pitch -> pitch + cte (Dataset: Pix4D house)



void ContextCaptureFacility::Matrix2YawPitchRoll(RotMatrixCR M, double& yaw, double& pitch, double& roll)
    {
    RotMatrix invR;
    invR.Zero();

    invR.form3d[0][0] = 1;
    invR.form3d[2][1] = -1;
    invR.form3d[1][2] = 1;
    double rx, ry, rz;
    Matrix2EulerZXY(M*invR, rx, ry, rz);
    pitch = -rx;
    yaw = -ry;
    roll = rz;
    }

RotMatrix ContextCaptureFacility::YawPitchRoll2Matrix(double yaw, double pitch, double roll)
    {
    // Not obvious since our default camera in looking in z direction (top)
    RotMatrix M = EulerZXY2Matrix(-pitch, -yaw, roll);
    RotMatrix R;
    R.Zero();

    // rotate to get an horizontal cam
    R.form3d[0][0] = 1;
    R.form3d[1][2] = -1;
    R.form3d[2][1] = 1;
    return M*R;
    }

END_BENTLEY_DATACAPTURE_NAMESPACE
