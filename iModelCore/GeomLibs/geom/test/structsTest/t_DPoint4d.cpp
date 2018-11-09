#include "testHarness.h"

//inherit from SampleFixture class
class DPoint4d_F : public GeomFixture
    {
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, Add)
    {
    DPoint4d refPoint = DPoint4d::From(9.2, 2.9, 2.2, 6.3);
    DPoint4d point4D = DPoint4d::From(2.2, 5.2, 5.6, 5.2);
    point4D.Add(refPoint);
    Check::Near (11.4, point4D.GetComponent(0));
    Check::Near (8.1, point4D.GetComponent(1));
    Check::Near (7.8, point4D.GetComponent(2));
    Check::Near (11.5, point4D.GetComponent(3));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, DotProduct)
    {
    DPoint4d refPoint = DPoint4d::From(9.2, 2.9, 2.2, 6.3);
    DPoint4d point4D = DPoint4d::From(2.2, 5.2, 5.6, 5.2);
    Check::Near (35.32, point4D.DotProductXY(refPoint));
    Check::Near (47.64, point4D.DotProductXYZ(refPoint));
    Check::Near (68.08, point4D.DotProductXYW(refPoint));
    Check::Near (80.4, point4D.DotProduct(refPoint));

    DPoint4d point4d = DPoint4d::From(4, 6, 8, 1);
    double prod0 = point4d.DotProduct(DPoint4d::From(0.2, 0.2, -0.4, 2));

    double prod1 = point4d.DotProduct(DPoint3d::From(0.2, 0.2, -0.4), 2);
    Check::Near(prod0, prod1);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, GetSetComponents)
    {
    DPoint4d point4D = DPoint4d::From(9.2, 2.9, 2.2, 6.3);
    point4D.SetComponent(20.2, 1);
    Check::ExactDouble(20.2, point4D.GetComponent(1));
    point4D.SetComponents(9.8, 12.3, 8.2, 22.1);
    double x_comp;
    double y_comp;
    double z_comp;
    double w_comp;
    point4D.GetComponents(x_comp, y_comp, z_comp, w_comp);
    Check::ExactDouble(point4D.GetComponent(0), x_comp);
    Check::ExactDouble(point4D.GetComponent(1), y_comp);
    Check::ExactDouble(point4D.GetComponent(2), z_comp);
    Check::ExactDouble(point4D.GetComponent(3), w_comp);
    Check::ExactDouble(point4D.GetComponent(4), x_comp);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, GetXYZ)
    {
    DPoint3d point3D = DPoint3d::From(0, 0, 0);
    DPoint4d pnt4 = DPoint4d::From(9.2, 2.9, 2.2, 8.2);
    pnt4.GetXYZ(point3D, 2, 0, 1);
    Check::ExactDouble(2.2, point3D.GetComponent(0));
    Check::ExactDouble(9.2, point3D.GetComponent(1));
    Check::ExactDouble(2.9, point3D.GetComponent(2));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, Disconnect)
    {
    DPoint4d pnt4 = DPoint4d::From(9.2, 2.9, 2.2, 8.2);
    Check::False(pnt4.IsDisconnect());
    pnt4.InitDisconnect();
    Check::ExactDouble(DISCONNECT, pnt4.GetComponent(0));
    Check::ExactDouble(DISCONNECT, pnt4.GetComponent(1));
    Check::ExactDouble(DISCONNECT, pnt4.GetComponent(2));
    Check::ExactDouble(DISCONNECT, pnt4.GetComponent(3));
    Check::True(pnt4.IsDisconnect());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, InitFromArray)
    {
    double ptArray[] = { 9.2, 2.9, 2.2, 8.2 };
    DPoint4d pnt4;
    pnt4.InitFromArray(ptArray);
    Check::ExactDouble(9.2, pnt4.GetComponent(0));
    Check::ExactDouble(2.9, pnt4.GetComponent(1));
    Check::ExactDouble(2.2, pnt4.GetComponent(2));
    Check::ExactDouble(8.2, pnt4.GetComponent(3));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, IsEqual)
    {
    DPoint4d point1 = DPoint4d::From(9.2, 2.9, 2.2, 8.2);
    DPoint4d point2 = DPoint4d::From(9.1, 2.7, 2.25, 8.45);
    Check::False(point1.IsEqual(point2));
    double tolerance = 0.4;
    Check::True(point1.IsEqual(point2, tolerance));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, Negate)
    {
    DPoint4d point1 = DPoint4d::From(9.2, -2.9, 2.2, -8.2);
    DPoint4d point2 = DPoint4d::From(9.1, -2.7, 2.25, -8.45);
    point1.Negate(point2);
    Check::ExactDouble(-9.1, point1.GetComponent(0));
    Check::ExactDouble(2.7, point1.GetComponent(1));
    Check::ExactDouble(-2.25, point1.GetComponent(2));
    Check::ExactDouble(8.45, point1.GetComponent(3));
    point1.Negate();
    Check::ExactDouble(9.1, point1.GetComponent(0));
    Check::ExactDouble(-2.7, point1.GetComponent(1));
    Check::ExactDouble(2.25, point1.GetComponent(2));
    Check::ExactDouble(-8.45, point1.GetComponent(3));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, Scale)
    {
    DPoint4d point1;
    DPoint4d point2 = DPoint4d::From(9.1, -2.7, 2.25, -8.45);
    double scale = 3;
    point1.Scale(point2, scale);
    Check::Near (27.3, point1.GetComponent(0));
    Check::Near (-8.1, point1.GetComponent(1));
    Check::Near (6.75, point1.GetComponent(2));
    Check::Near (-25.35, point1.GetComponent(3));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, AddSubtract)
    {
    DPoint4d point1 = DPoint4d::From(2.3, 5.2, 9.7, -8.2);
    DPoint4d point2 = DPoint4d::From(9.1, -2.7, 2.25, -8.45);
    point1.Subtract(point2);
    Check::Near(-6.8, point1.GetComponent(0));
    Check::Near(7.9, point1.GetComponent(1));
    Check::Near(7.45, point1.GetComponent(2));
    Check::Near(0.25, point1.GetComponent(3));
    point1.SumOf(point1, point2);
    Check::Near(2.3, point1.GetComponent(0));
    Check::Near(5.2, point1.GetComponent(1));
    Check::Near(9.7, point1.GetComponent(2));
    Check::Near(-8.2, point1.GetComponent(3));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, WeightedDifference)
    {
    DPoint3d point1 = DPoint3d::From(2.3, 5.2, 9.7);
    DPoint4d point2 = DPoint4d::From(9.1, -2.7, 2.25, 2);
    DPoint4d fin;
    double wA = 3;
    fin.WeightedDifferenceOf(point1, wA, point2);
    Check::Near(-22.7, fin.GetComponent(0));
    Check::Near(18.5, fin.GetComponent(1));
    Check::Near(12.65, fin.GetComponent(2));
    Check::Near(0, fin.GetComponent(3));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, Normalize)
    {
    DPoint4d point = DPoint4d::From(3, 6, 9, 2);
    Check::True(point.NormalizeWeightInPlace());
    Check::Near(1.5, point.x);
    Check::Near(3, point.y);
    Check::Near(4.5, point.z);
    Check::Near(1, point.w);

    DPoint4d point2 = DPoint4d::From(3, 6, 9, 2);
    double magnit = 11.2249721603;
    point.NormalizePlaneOf(point2);
    Check::Near(1.5 / magnit, point.GetComponent(0));
    Check::Near(3 / magnit, point.GetComponent(1));
    Check::Near(4.5 / magnit, point.GetComponent(2));
    Check::Near(1 / magnit, point.GetComponent(3));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, WeightedDifferenceCmp)
    {
    DPoint3d pnt3D = DPoint3d::FromXYZ(5, 8, 6);
    DPoint4d pnt4D = DPoint4d::From(pnt3D, 0.6);
    DPoint3d pnt3D_1 = DPoint3d::FromXYZ(5, 8, 6);
    double weight = 0.3;
    DPoint4d pnt4D_1 = DPoint4d::From(pnt3D_1, weight);

    DPoint4d pnt_1;
    DPoint4d pnt_2;

    pnt_1.WeightedDifferenceOf(pnt4D, pnt4D_1);
    pnt_2.WeightedDifferenceOf(pnt4D, pnt3D_1, weight);
    Check::Near(pnt_1, pnt_2);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, Random)
    {
    double pArr[] = { 2.3, 1.4, 6.5, 9.3 };
    DPoint4d dpoint1;
    dpoint1.InitFromArray(pArr);

    DPoint4d dpoint2;
    dpoint2.Negate(dpoint1);
    dpoint2.Negate();
    Check::Near(dpoint1, dpoint2);
    dpoint2.Add(dpoint1);
    DPoint4d dpoint3 = dpoint1;
    dpoint3.Scale(2);
    Check::Near(dpoint2, dpoint3);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, Mixing)
    {
    double pArr[] = { 2.3, 1.4, 6.5, 9.3 };
    DPoint4d dpoint1;
    dpoint1.InitFromArray(pArr);
    DPoint4d dpoint2;
    dpoint2.InitFromArray(pArr);
    Check::Near(dpoint1, dpoint2);
    dpoint2.SetComponent(3.4, 1);
    Check::False(dpoint2.IsEqual(dpoint1));
    dpoint1.SetComponent(dpoint2.GetComponent(1), 1);
    Check::Near(dpoint1, dpoint2);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, IsEqualCheck)
    {
    double pArr[] = { 2.3, 1.4, 6.5, 9.3 };
    DPoint4d dpnt1;
    dpnt1.InitFromArray(pArr);
    double pArr2[] = { 2.3, 1.4, 6.4, 8.3 };
    DPoint4d dpnt2;
    dpnt2.InitFromArray(pArr2);
    Check::False(dpnt2.IsEqual(dpnt1));
    Check::False(dpnt2.IsEqual(dpnt1, 0.1));
    Check::True(dpnt2.IsEqual(dpnt1, 0.1, 1.0));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, MatrixInverseMultiple)
    {
    DMatrix4d mat = DMatrix4d::FromRowValues(1, 0, 0, 0,
                                             0, 1, 0, 0,
                                             0, 0, 1, 0,
                                             0, 0, 0, 1);
    DMatrix4d matInv;
    matInv.QrInverseOf(mat);
    DMatrix4d mat4d[2] = { mat,  matInv};
    DPoint4d point4d = DPoint4d::FromMultiply(mat4d, DPoint4d::From(4, 7, 4, 1));
    Check::Near(point4d, DPoint4d::From(4, 7, 4, 1));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, AlmostEqual)
    {
    bvector< DPoint4d> dataA = { DPoint4d::From(0.3,0.5,-0.3,0),
                                 DPoint4d::From(2, 1, 4, 1),
                                 DPoint4d::From(8, 1, 2, 3), };
    
    bvector< DPoint4d> dataB = { DPoint4d::From(0.354,0.5,-0.3,0.0040),
                                 DPoint4d::From(2, 1.052, 4, 1.099),
                                 DPoint4d::From(8.00000002, 1, 2, 3.01133), };
    DPoint4d pnt = DPoint4d::From(0, 0, 0, 0);
    bool flage = pnt.AlmostEqual(dataA, dataB, 0.055, 0.1);
    Check::True(flage);
    flage = pnt.AlmostEqualReversed(dataA, dataB, 0.055, 0.1);
    Check::False(flage);
    dataA = { DPoint4d::From(8, 1, 2, 3),
              DPoint4d::From(2, 1, 4, 1),
              DPoint4d::From(0.3,0.5,-0.3,0), };
    flage = pnt.AlmostEqualReversed(dataA, dataB, 0.055, 0.1);
    Check::True(flage);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, RealSquaredDistance)
    {
    DPoint4d pont4= DPoint4d::From(0.4, 2, 99, 2);
    double sqrdDistance, sqrdDistance4d;
    pont4.RealDistanceSquared(&sqrdDistance, DPoint3d::From(4, 3, 1));

    pont4.RealDistanceSquared(&sqrdDistance4d, DPoint4d::From(DPoint3d::From(4, 3, 1), 1));
    Check::Near(sqrdDistance, sqrdDistance4d);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST_F(DPoint4d_F, RealSquaredDistanceXY)
    {
    DPoint4d pont = DPoint4d::From(0.3, 0.4, 0.9, 1);
    double sqrd;
    pont.RealDistanceSquaredXY(&sqrd, DPoint3d::From(4.2, 2.3, 5.9));
    double distance2d = DPoint2d::From(0.3, 0.4).DistanceSquared(DPoint2d::From(4.2, 2.3));
    Check::Near(sqrd, distance2d);
    }