/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_DATACAPTURE_NAMESPACE

// Notations and formulas from http://en.wikipedia.org/wiki/Geodetic_system
struct Ellipsoid
    {
    Ellipsoid(double _a, double _invf);
    double a, invf, b, e, eprime;

    // Convert geodetic latitude,longitude,height (in degree) to/from cartesian coordinates in the Earth-centered Earth-fixed cartesian referential
    DPoint3d LLH2ECEF(GeoPointCR LatitudeLongitudeHeight) const;
    GeoPoint ECEF2LLH(DPoint3dCR XYZ) const;
    static Ellipsoid WGS84();
    static Ellipsoid GRS80();

    // From ECEF to local ENU referential and inverse
    Transform ECEF2ENU(double latitude, double longitude) const;
    Transform ENU2ECEF(double latitude, double longitude) const;
    };

// Notations and formulas from http://fr.wikipedia.org/wiki/Projection_conique_conforme_de_Lambert
struct ConformalConicProjection
    {
    ConformalConicProjection(double _phi0, double _phi1, double _phi2, double _lambda0, double _X0, double _Y0, Ellipsoid _ellipsoid);
    double phi0, phi1, phi2, lambda0, X0, Y0, n, rho_0, rho0;
    Ellipsoid ellipsoid;

    // Convert conformal conic coordinates to/from geodetic latitude,longitude,height (in degree)
    GeoPoint CCP2LLH(DPoint3dCR XYZ) const;
    DPoint3d LLH2CCP(GeoPointCR LatitudeLongitudeHeight) const;
    static ConformalConicProjection Lambert93();
    static ConformalConicProjection RGF93_CC(int i);
    };

struct ContextCaptureFacility
    {
    // Compute North-East-Down and East-North-Up referential at a given geodetic latitude,longitude (in radians)
    // The matrix maps vectors in local tangent plane coordinates to cartesian coordinates
    DATACAPTURE_EXPORT static RotMatrix NorthEastDown(double latitude, double longitude);
    DATACAPTURE_EXPORT static RotMatrix EastNorthUp(double latitude, double longitude);

    // Euler angles (in radians) to rotation matrix
    DATACAPTURE_EXPORT static RotMatrix EulerZYX2Matrix(double rx, double ry, double rz);
    DATACAPTURE_EXPORT static void Matrix2EulerZYX(RotMatrixCR M, double& rx, double& ry, double& rz);

    // Euler angles (in radians) to rotation matrix
    DATACAPTURE_EXPORT static RotMatrix EulerYXZ2Matrix(double rx, double ry, double rz);
    DATACAPTURE_EXPORT static void Matrix2EulerYXZ(RotMatrixCR M, double& rx, double& ry, double& rz);

    // Euler angles (in radians) to rotation matrix
    DATACAPTURE_EXPORT static RotMatrix EulerZXY2Matrix(double rx, double ry, double rz);
    DATACAPTURE_EXPORT static void Matrix2EulerZXY(RotMatrixCR M, double& rx, double& ry, double& rz);

    // Omega,phi,kappa angles (in radian) to rotation matrix
    DATACAPTURE_EXPORT static RotMatrix OmegaPhiKappa2Matrix(double omega, double phi, double kappa);
    DATACAPTURE_EXPORT static void Matrix2OmegaPhiKappa(RotMatrixCR M, double& omega, double& phi, double& kappa);

    // Heading,pitch,roll (in radian) to rotation matrix
    DATACAPTURE_EXPORT static RotMatrix HeadingPitchRoll2Matrix(double heading, double pitch, double roll);
    DATACAPTURE_EXPORT static void Matrix2HeadingPitchRoll(RotMatrixCR M, double& heading, double& pitch, double& roll);

    // Yaw,pitch,roll (in radian) to rotation matrix
    DATACAPTURE_EXPORT static RotMatrix YawPitchRoll2Matrix(double yaw, double pitch, double roll);
    DATACAPTURE_EXPORT static void Matrix2YawPitchRoll(RotMatrixCR M, double& yaw, double& pitch, double& roll);
    };

END_BENTLEY_DATACAPTURE_NAMESPACE
