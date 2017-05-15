#include "testHarness.h"


TEST(FVec3d,HelloWorld)
    {
    FVec3d point0 = FVec3d::From (1.0, 2.0, 3.0);
    Check::Near (1.0, point0.x);
    Check::Near (2.0, point0.y);
    Check::Near (3.0, point0.z);
    }



#define CompileFVec3dDotProductPrecisionTables_not
#ifdef CompileFVec3dDotProductPrecisionTables
// EDL May 15, 2017
// This is
// Form vectors that should be unit vectors (in double precision)
// Truncate them to single precision (vector0F, vector1F)
// Compare dot products with
//    (a) FVec3d::FDotProduct -- internally work with floats
//    (b) FVec3d::DotProduct -- internally promote to double (in inlined template)
//    (c) DVec3d::DotProduct (DVec3d) -- promote to double in caller, all double internally.
// Visually observable:
//  ** (b) and (c) match.  The template expander really does seem to up-cast to double and accumulate double products.
//  ** (a) differs.
//  ** (a) has largest relative error for when the two vectors being dotted are close to 90 degrees apart.
// BUT ... the (a) differences are (subjectively) less than I expected.
//    I think the FVec3d::DotProduct values are "better" than FVec3d::FDotProduct.   But I can't say definitively it's catastrophic.
// 
void breakPoint (){}  // a place to put a break.
TEST(FVec3d,DotProductPrecison)
    {
    for (double z : bvector<double> {0,1,4})
        {
        GEOMAPI_PRINTF ("z = %g\n  (theta0    sweep    theta1)      dotDD    (eFD)    (eFF)  (eFD/eFF) (eFF/dotDD)\n", z);
        for (double theta0 : bvector<double> {0.0, 0.1, 0.213892901389, 0.3, 0.5, 0.9, Angle::DegreesToRadians (89.0) })
            {
            for (double delta: bvector<double> {
                        Angle::DegreesToRadians (100.0),
                        Angle::DegreesToRadians (91.0),
                        Angle::DegreesToRadians (90.1),
                        Angle::DegreesToRadians (90.01),
                        Angle::DegreesToRadians (90.001),
                        Angle::DegreesToRadians (90.0001),
                        Angle::DegreesToRadians (10.0),
                        Angle::DegreesToRadians (1.0),
                        Angle::DegreesToRadians (0.1),
                        Angle::DegreesToRadians (0.01),
                        Angle::DegreesToRadians (0.001),
                        Angle::DegreesToRadians (0.0001)
                        }
                    )
                {
                double theta1 = theta0 + delta;
                DVec3d vector0D = DVec3d::From (cos(theta0), sin(theta0));
                DVec3d vector1D = DVec3d::From (cos(theta1), sin(theta1));
                FVec3d vector0F = FVec3d::From (vector0D);
                FVec3d vector1F = FVec3d::From (vector1D);
                //double dotDD = vector0D.DotProduct (vector1D);
                float dotFF = vector0F.FDotProduct (vector1F);
                double dotDD = DVec3d::From (vector0F).DotProduct (DVec3d::From (vector1F));
                double dotFD = vector0F.DotProduct (vector1F);
                double eFF = fabs (dotFF - dotDD);
                double eFD = fabs (dotFD - dotDD);
                GEOMAPI_PRINTF (" (%9.6lg %9.6lg %9.6lg)    %12.7le (%8.2le) (%8.2le) (%8.2le) (%8.2le)\n",
                            Angle::RadiansToDegrees (theta0),
                            Angle::RadiansToDegrees (delta),
                            Angle::RadiansToDegrees (theta1),
                            dotDD,
                            eFD, eFF,
                            eFF == 0.0 ? 9999.0 : eFD / eFF,
                            eFF / dotDD
                            );
                breakPoint ();

                }
            }
        }
    }
#endif