/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
void forwardReverseCheck(DMatrix4d tstMatrix, DMap4d map) 
    {
    DMatrix4d mat4d, mat4dExp;
    mat4d.InitProduct(tstMatrix, map.M0);
    mat4dExp.InitProduct(mat4d, map.M1);

    Check::Near(mat4dExp, tstMatrix);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMap4d, ForwardReverseTransformation)
    {
    DMatrix4d tstMatrix = DMatrix4d::FromColumns(DPoint4d::From(0.4, 1, 0.1, 0),
                                                 DPoint4d::From(2, 1, 0.1, 3),
                                                 DPoint4d::From(1.5, 8, 2, 0),
                                                 DPoint4d::From(9, 9, 0.1, 1));
    
    forwardReverseCheck(tstMatrix, DMap4d::FromTranslation(4, 3, 2));
    forwardReverseCheck(tstMatrix, DMap4d::FromTranslation(0.4, 0.3, 1.2));
    forwardReverseCheck(tstMatrix, DMap4d::FromScale(4, 3, 2));
    forwardReverseCheck(tstMatrix, DMap4d::FromScale(0.1, 0.002, -0.3));
    forwardReverseCheck(tstMatrix, DMap4d::FromScale(3.4));
    Angle angle = Angle::FromDegrees(45);
    forwardReverseCheck(tstMatrix, DMap4d::FromRotation(angle.Cos(), angle.Sin(), 2, 3, 4));
    forwardReverseCheck(tstMatrix, DMap4d::FromRotation(15, 1.2, 0.2, 0.4));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMap4d, Explode)
    {
    DMap4d transformation = DMap4d::FromScale(1, 2, 9);
    transformation = DMap4d::FromTranslation(4, 3, 2);
    RotMatrix rotMatrix;
    DPoint3d translation;
    DPoint4d perspective;
    transformation.Explode(rotMatrix, translation, perspective, false);

    Check::True(rotMatrix.IsIdentity());
    Check::Near(translation, DPoint3d::From(4, 3, 2));
    Check::Near(perspective, DPoint4d::From(0, 0, 0, 1));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMap4d, IdentityMap)
    {
    DMap4d identityMap = DMap4d::FromIdentity();
    Check::Near(identityMap.M0, identityMap.M1);
    
    DPoint3d point = DPoint3d::From(4, 1, 9);
    DPoint4d retPoint = identityMap.M0.Multiply(point, 1);
    Check::Near(retPoint, DPoint4d::From(point, 1), "Not an Identity Transform");
    }

void isAffinePerspective(DMap4d mat) 
    {
    if (mat.M0.coff[3][3] == 1)
        Check::True(mat.IsAffine());
    else
        Check::True(mat.IsPerspective());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMap4d, CheckAffine)
    {
    isAffinePerspective(DMap4d::FromIdentity());
    isAffinePerspective(DMap4d::FromTranslation(4, 3, 2));
    isAffinePerspective(DMap4d::FromRotation(15, 1.2, 0.2, 0.4));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMap4d, Identity)
    {
    DMatrix4d forwardMat;
    forwardMat.InitIdentity();

    DMatrix4d reverseMat;
    reverseMat.InitIdentity(); 
    Check::True(reverseMat.IsIdentity());
    Check::True(forwardMat.IsIdentity());
    DMap4d map = DMap4d::From(forwardMat, 
                              reverseMat);
    Check::True(map.IsIdentity());
    }

void transformMap(DMap4d map, DPoint3d pnt) 
    {

    DPoint4d res =  map.M0.Multiply(pnt, 1);
    //reverse
    DPoint4d resReversed =  map.M1.Multiply(DPoint3d::From(res.x, res.y, res.z), 1);
    Check::Near(resReversed.x, pnt.x);
    Check::Near(resReversed.y, pnt.y);
    Check::Near(resReversed.z, pnt.z);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMap4d, QuadrantRotation)
    {
    DMap4d map;
    DPoint3d pnt = DPoint3d::From(4, 3, 4);
    map.InitFromTransform(Transform::FromRowValues(1, 0, 0, 2,
                                                   0, 1, 0, 2,
                                                   0, 0, 1, 2), false);
    transformMap( map, pnt);
    map.InitFromTransform(Transform::From(DPoint3d::From(3, 2, 2)), false);
    transformMap( map, pnt);
    map.InitFromTransform(Transform::From(RotMatrix::FromAxisAndRotationAngle(2, Angle::FromDegrees(60).Radians())), false);
    transformMap( map, pnt);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Earlin.Lutz                   02/18
//---------------------------------------------------------------------------------------
TEST(DMap4d, VectorFrustumExample)
    {
    auto origin = DPoint3d::From(111.55210256687462, -8.3610081610513802, -10.253043196228713);
    auto uVector = DVec3d::From(62.386308713014060, -0.00000000000000000, 0.00000000000000000);
    auto vVector = DVec3d::From(0.00000000000000000, 62.386308713014060, -0.00000000000000000);
    auto wVector = DVec3d::From(31.041154356507047, 31.041154356507032, 31.041154356507036);
    auto fraction = 0.0048728640349349457;
    DMap4d map;
    bsiDMap4d_initFromVectorFrustum(&map, &origin, &uVector, &vVector, &wVector, fraction);
    auto product = map.M0 * map.M1;
    Check::True (product.IsIdentity (), "Vector frustum has inverse");
    }