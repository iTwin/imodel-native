#include "testharness.h"

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