/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Service functions
---------------------------------------------------------------------------------------*/
RotMatrix YawPitchRollAngles::ToRotMatrix() const
    {
    double c0 = m_yaw.Cos(), s0 = m_yaw.Sin();
    double c1 = m_pitch.Cos(), s1 = m_pitch.Sin();
    double c2 = m_roll.Cos(), s2 = m_roll.Sin();
    return RotMatrix::FromRowValues
        (
        c0 * c1,    -(s0 * c2 + c0 * s1 * s2),    (s0 * s2 - c0 * s1 * c2),
        s0 * c1,    (c0 * c2 - s0 * s1 * s2),    -(c0 * s2 + s0 * s1 * c2),
        s1,         c1 * s2,                      c1 * c2
        );
    }

double YawPitchRollAngles::MaxAbsRadians() const
    {
    return DoubleOps::MaxAbs(
        m_yaw.Radians(),
        m_pitch.Radians(),
        m_roll.Radians()
        );
    }

bool YawPitchRollAngles::IsIdentity() const
    {
    return Angle::IsNearZeroAllowPeriodShift(m_yaw.Radians())
        && Angle::IsNearZeroAllowPeriodShift(m_pitch.Radians())
        && Angle::IsNearZeroAllowPeriodShift(m_roll.Radians());
    }

double YawPitchRollAngles::MaxDiffRadians(YawPitchRollAngles const &other) const
    {
    return DoubleOps::MaxAbs(
        m_yaw.Radians() - other.m_yaw.Radians(),
        m_pitch.Radians() - other.m_pitch.Radians(),
        m_roll.Radians() - other.m_roll.Radians()
        );
    }

double YawPitchRollAngles::MaxAbsDegrees() const
    {
    return DoubleOps::MaxAbs(
        m_yaw.Degrees(),
        m_pitch.Degrees(),
        m_roll.Degrees()
        );
    }

double YawPitchRollAngles::MaxDiffDegrees(YawPitchRollAngles const &other) const
    {
    return DoubleOps::MaxAbs(
        m_yaw.Degrees() - other.m_yaw.Degrees(),
        m_pitch.Degrees() - other.m_pitch.Degrees(),
        m_roll.Degrees() - other.m_roll.Degrees()
        );
    }

double SumSquaredDegrees(YawPitchRollAngles const &ypr)
    {
    double a = ypr.GetYaw().Degrees();
    double b = ypr.GetPitch().Degrees();
    double c = ypr.GetRoll().Degrees();
    return a * a + b * b + c * c;
    }

bool YawPitchRollAngles::TryFromTransform(DPoint3dR origin, YawPitchRollAnglesR angles, TransformCR transform)
    {
    transform.GetTranslation(origin);
    return TryFromRotMatrix(angles, transform.Matrix());
    }

bool YawPitchRollAngles::TryFromRotMatrix(YawPitchRollAnglesR angles, RotMatrixCR matrix)
    {
    double s1 = matrix.form3d[2][0];
    double c1 = sqrt(matrix.form3d[2][1] * matrix.form3d[2][1] + matrix.form3d[2][2] * matrix.form3d[2][2]);       // but it might be the negative of this..
    
    AngleInDegrees pitchA = AngleInDegrees::FromAtan2(s1, c1);   // with postive cosine
    AngleInDegrees pitchB = AngleInDegrees::FromAtan2(s1, -c1);  // with negative cosine
    if ( c1 < Angle::SmallAngle())   // This is a radians test !!!
        {
        angles = YawPitchRollAngles(
              AngleInDegrees::FromAtan2(-matrix.form3d[0][1], matrix.form3d[1][1]),
              pitchA,
              AngleInDegrees::FromRadians(0.0)
              );
        }
    else
        {
        AngleInDegrees yawA = AngleInDegrees::FromAtan2(matrix.form3d[1][0], matrix.form3d[0][0]);
        AngleInDegrees rollA = AngleInDegrees::FromAtan2(matrix.form3d[2][1], matrix.form3d[2][2]);

        AngleInDegrees yawB = AngleInDegrees::FromAtan2(-matrix.form3d[1][0], -matrix.form3d[0][0]);
        AngleInDegrees rollB = AngleInDegrees::FromAtan2(-matrix.form3d[2][1], -matrix.form3d[2][2]);

        YawPitchRollAngles yprA (yawA, pitchA, rollA);
        YawPitchRollAngles yprB (yawB, pitchB, rollB);
        static double s_absFactor = 0.95;
        double radiansA = yprA.MaxAbsDegrees();
        double radiansB = yprB.MaxAbsDegrees();
        if (radiansA < s_absFactor * radiansB)
            angles = yprA;
        else if (radiansB < s_absFactor *radiansA)
            angles = yprB;
        else
            {
            double sumA = SumSquaredDegrees(yprA);
            double sumB = SumSquaredDegrees(yprB);
            if (sumA <= sumB)
                angles = yprA;
            else
                angles = yprB;
            }
        }
    RotMatrix matrix1 = angles.ToRotMatrix();        
    return matrix.MaxDiff(matrix1) < Angle::SmallAngle();
    }


END_BENTLEY_GEOMETRY_NAMESPACE
