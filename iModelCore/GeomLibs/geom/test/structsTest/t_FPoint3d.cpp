#include "testHarness.h"


TEST(FPoint3d,HelloWorld)
    {
    FPoint3d point0 = FPoint3d::From (1.0, 2.0, 3.0);
    Check::Near (1.0, point0.x);
    }

TEST(FPoint3d,SumOf)
    {
    // THIS IS ALL THE INPUT VALUES NEEDED TO TEST ALL SUMOF METHODS ....
    auto originF = FPoint3d::From (1,2,3);
    auto originD = DPoint3d::From (originF);
    auto vector0 = DVec3d::From (4,5,6);
    auto vector1 = DVec3d::From (7,8,9);
    auto vector2 = DVec3d::From (10,11,12);

    double scale0 = -3.0;
    double scale1 = 17.0;
    //double scale2 = -13.0;
    Check::Exact (
        DPoint3d::FromSumOf (originD, vector0, scale0, vector1, scale1),
        DPoint3d::From (
                FPoint3d::FromSumOf (originF, vector0, scale0, vector1, scale1)
                ),
        "SumOf DPoint3d matches FPoint3d");

    auto transform = Transform::FromOriginAndVectors (originD, vector0, vector1, vector2);
    Check::Exact (
        transform * DPoint3d::From (originF),
        DPoint3d::From (FPoint3d::FromMultiply (transform, originD)),
        "Transform * point");

    // Farahad -- you can test lots and lots of FPoint3d methods (SumOf, FromSumOf, FromMultiply) .. with just originD, vector0, vector1, vector2, and this transform
    // You'll need a few more points for Distance and others that have multiple FPoint3d inputs.
    }

TEST(FPoint3d,ROUND_AWAY)
    {
    int one = 1;
    int a = one << 24;
    double ed = 1.0 / (double)a;
    float  ef = (float)1.0 / (float)a;
    float onePlus = 1.0F + 2.0F * ef;
    float oneMinus = 1.0F - ef;
    float roundAway = 1.000000119;
    float roundToward = 0.9999999404;
    GEOMAPI_PRINTF ("Single Precision Unit Roundoff = %.10lg %.10lg\n", ed, (double)ef); // arg passing is always double.
    GEOMAPI_PRINTF ("Single Precision Rounders = %.10lg %.10lg\n", (double)onePlus, (double)oneMinus); // arg passing is always double.
    GEOMAPI_PRINTF ("Single Precision nextAfter neighbors= %.10lg %.10lg\n",
                (double)_nextafterf (1.0, 2.0), (double)_nextafterf (1.0, 0.0)); // arg passing is always double.

    GEOMAPI_PRINTF ("Single Precision roundTowards roundAway= %.10lg %.10lg\n",
                (double)roundToward, (double)roundAway); // arg passing is always double.
    }