#include "testHarness.h"

struct TransformSamples
{
static Transform Jumble0 ()
    {
    return Transform::FromRowValues (
        2, 3, 4, -3,
        -1, 9, 5, 2,
        -2.5, 3.5, 1.5, 4.01
        );
    };
//
// A transform with diagonals larger than 1, and all off-diagonal entries equal the parameter a.
// (Hence for small a the matrix part is diagonally dominant)
//
static Transform StrongDiagonal0 (double a, double tx = 12, double ty = 13, double tz = 7)
    {
    return Transform::FromRowValues (
        2.0, a, a, 12,
        a, 3.0, a, 13,
        a, a, 11.0, 7
        );
    };

// Return a transform that is strong on the diagonal with distinct values elsewhere.
static Transform StrongDiagonal1 ()
    {
    return Transform::FromRowValues (
        12, 2, 1, 4,
        3, 15, -2, 7,
        5, 3.5,20, 13
        );
    };

};
void CheckOperators (TransformCR transform, DPoint3dCR A, DVec3d T)
    {
    DPoint3d origin;
    DVec3d U, V, W;
    transform.GetOriginAndVectors (origin, U, V, W);
    RotMatrix matrix = RotMatrix::From (transform);
    DPoint3d A1x, A1z;
    transform.Multiply (A1x, A);
    transform.Multiply (A1z, A.x, A.y, A.z);
    Check::Near (A1x, A1z);
    DVec3d  T1x;
    transform.MultiplyMatrixOnly (T1x, T);
    DVec3d T1y;
    matrix.Multiply (T1y, T);
    Check::Near (T1x, T1y);
    DPoint3d A2x = DPoint3d::FromSumOf (origin, U, A.x, V, A.y, W, A.z);
    Check::Near (A1x, A2x);
    DVec3d T2x = DVec3d::FromSumOf (U, T.x, V, T.y, W, T.z);
    Check::Near (T1x, T2x);


    auto A1 = transform * A;
    auto T1 = transform * T;
    Check::Near (A1x, A1);

    auto A2 = origin + A.x * U + A.y * V + A.z * W;
    UNUSED_VARIABLE(A2);
    auto T2 = T.x * U + T.y * V + T.z * W;
    Check::Near (T1x, T2);
    Check::Near (T1x, T1);

    auto T3 = U * T.x + V * T.y  + W * T.z;
    UNUSED_VARIABLE(T3);

    DVec3d B2 = transform * DVec3d::From (-1,0,0);
    UNUSED_VARIABLE(B2);

    auto B0 = origin - U;
    //auto U2 = -U;
    DPoint3d B1 = transform * DPoint3d::From (-1,0,0);
    Check::Near (B0, B1, "Transform * Point  (-1,0,0)");
    //Check::Near (U2, B2, "Transform * Vector (-1,0,0)");
    
    DVec3d U_times2 = U;    // BUT NEXT LINE DOUBLES IT IN PLACE.
    U_times2 *= 2.0;
    Check::Near (U_times2, U + U);
    Check::Near (U_times2, U * 2.0);
    Check::Near (U_times2, 2.0 * U);
    Check::Near (U, U_times2 - U);
    Check::Near (U, U_times2 / 2.0);

    Check::False ((U / 0.0).IsValid ());
    Check::True ((U / 1.0e-6).IsValid ());

    DVec3d TA1, TM1;
    transform.MultiplyTransposeMatrixOnly (TA1, T);
    matrix.MultiplyTranspose (TM1, T);
    Check::Near (TM1, DVec3d::From (T.DotProduct (U), T.DotProduct(V), T.DotProduct (W)));
    Check::Near (TA1, TM1);
    Check::Near (TA1, T*transform);
    Check::Near (TA1, T*matrix);

    DVec3d matrixTimesVector, transformTimesVector;
    transform.MultiplyMatrixOnly (transformTimesVector, T);
    matrix.Multiply (matrixTimesVector, T);
    Check::Near (matrixTimesVector, transformTimesVector);
    Check::Near (matrixTimesVector, matrix * T);
    Check::Near (transformTimesVector, transform * T);


    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Operators, VersusTransformOps)
    {
    CheckOperators (Transform::FromRowValues (
                    1,0,0,-3,
                    0,1,0,2,
                    0,0,1,4
                    ),
                    DPoint3d::From (1,2,3),
                    DVec3d::From (0.2,0.3,-0.4)
                    );
    CheckOperators (TransformSamples::Jumble0 (),
                    DPoint3d::From (1,2,3),
                    DVec3d::From (0.2,0.3,-0.4)
                    );
    }

    // Input a pair of transforms, and a third claimed to be their product.
    // Check the associativity condition {transformA*(transformB*X)=product*X}
void CheckAssociativity (TransformCR transformA, TransformCR transformB, TransformCR product)
    {
    bvector<DPoint3d> points {
        DPoint3d::From (1,0,0),
        DPoint3d::From (0,1,0),
        DPoint3d::From (0,0,1),
        DPoint3d::From (1,2,3),
        DPoint3d::From (-10, 2, 3)
        };
    for (DPoint3d xyz : points)
        {
        DPoint3d BX = transformB * xyz;
        DPoint3d ABX = transformA * BX;
        Check::Near (ABX, product * xyz);
        }
    }


// Check that products of all combinations of Transform and (extracted) RotMatrix behave as expected.
void CheckProductOperators (TransformCR transformA, TransformCR transformB)
    {
    RotMatrix matrixA = RotMatrix::From (transformA);
    RotMatrix matrixB = RotMatrix::From (transformB);
    Transform TATB = transformA * transformB;
    Transform TARB = transformA * matrixB;
    Transform RATB = matrixA * transformB;
    Transform TATB_old, TARB_old, RATB_old;
    TATB_old.InitProduct (transformA, transformB);
    TARB_old.InitProduct (transformA, matrixB);
    RATB_old.InitProduct (matrixA, transformB);
    CheckAssociativity (transformA, transformB, TATB);
    CheckAssociativity (transformA, Transform::From (matrixB), TARB);
    CheckAssociativity (Transform::From (matrixA), transformB, RATB);
    Check::Near (TATB_old, TATB);
    Check::Near (TARB_old, TARB);
    Check::Near (RATB_old, RATB);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Operators,MatrixProducts)
     {
     CheckProductOperators (
         TransformSamples::Jumble0 (), TransformSamples::StrongDiagonal0 (0.1));
     CheckProductOperators (
         TransformSamples::Jumble0 (), TransformSamples::StrongDiagonal1 ());
    }

struct My20Bits
    {
    // these pack to 4 bytes:
    uint32_t bit0 : 1;
    uint32_t bit1 : 1;
    uint32_t bit2 : 1;
    uint32_t bit3 : 1;
    uint32_t bit4 : 1;
    uint32_t bit5 : 4;
    uint32_t bit9 : 1;
    uint32_t bit10 : 10;
    };



struct My35Bits
    {
    // these pack to 4 bytes:
    uint32_t bit0:1;
    uint32_t bit1:1;
    uint32_t bit2:1;
    uint32_t bit3:1;
    uint32_t bit4:1;
    uint32_t bit5:4;
    uint32_t bit9:1;
    uint32_t bit10 : 21;
    uint32_t bit31:1;
    // expect these to spill to step up beyond 4 bytes:
    uint32_t bitB0 : 1;
    uint32_t bitB1 : 1;
    uint32_t bitB2 : 1;
    };

struct My20Bools
    {
    bool bit0;
    bool bit1;
    bool bit2;
    bool bit3;
    bool bit4;
    bool bit5[4];
    bool bit9;
    bool bit10[10];
    };

struct My35Bools
    {
    bool bit0;
    bool bit1;
    bool bit2;
    bool bit3;
    bool bit4;
    bool bit5[4];
    bool bit9;
    bool bit10[21];
    bool bit31;
    bool bitB0;
    bool bitB1;
    bool bitB2;
    };

struct MyBits
    {
    My20Bits a;
    My35Bits b;
    };

struct MyBools
    {
    My20Bools a;
    My35Bools b;
    };

static void logInt(int v, char const* msg)
    {
    LOG.errorv("(%s %d)", msg, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Compiler,BitFieldSize)
{
    logInt((int)sizeof (My20Bools), "My20Bools");
    logInt((int)sizeof (My35Bools), "My35Bools");
    logInt((int)sizeof (MyBools), "MyBools");
    LOG.errorv("--");
    logInt((int)sizeof (My20Bits), "My20Bits");
    logInt((int)sizeof (My35Bits), "My35Bits");
    logInt((int)sizeof (MyBits), "MyBits");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Compiler,BSizes)
    {
    bvector<DPoint3d> bvector_DPoint3d;
    bvector<int> bvector_int;
    bmap<int,DPoint3d> bmap_DPoint3d;
    bmap<int,int> bmap_int;
    Check::PrintIndent (0);
    Check::Print ((uint64_t)sizeof (bvector_DPoint3d), "sizeof (bvector_DPoint3d)");
    Check::Print ((uint64_t)sizeof (bvector_int), "sizeof(bvector_int)");
    Check::PrintIndent (0);
    Check::Print ((uint64_t)sizeof (bmap_DPoint3d), "sizeof(bmap<int,DPoint3d>)");
    Check::Print ((uint64_t)sizeof (bmap_int), "sizeof(bmap<int,int>)");
    Check::PrintIndent (0);
    bmap_int[0] = 0;
    //Check::Print (sizeof(bmap_int.root_), "bmap<int,int> singleton size");
#ifdef mutable_size_accessible
    for (size_t i = 0; i < 40; i++)
        {
        bmap_int[(int)i] = 2 * (int)i;
        Check::Print (bmap_int.root ().mutable_size (), "size");
        }
#endif

    Check::Print ((uint64_t)bvector_int.capacity (), "bvector<int> initial capacity");
    Check::Print ((uint64_t)bvector_DPoint3d.capacity (), "bvector<DPoint3d> initial capacity");
    bvector_int.push_back (0);
    bvector_DPoint3d.push_back (DPoint3d::FromZero ());
    Check::Print ((uint64_t)bvector_int.capacity (), "bvector<int> singleton capacity");
    Check::Print ((uint64_t)bvector_DPoint3d.capacity (), "bvector<DPoint3d> singleton capacity");
    Check::PrintIndent (0);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Compiler,bvectorCapacity)
    {
    bvector<DPoint3d> data;
    size_t n0 = data.capacity ();
    for (size_t i = 0; i < 20000; i++)
        {
        data.push_back (DPoint3d::From (0,0,0));
        size_t n1 = data.capacity ();
        if (n1 != n0)
            {
            Check::PrintIndent (2);
            Check::Print ((uint64_t)i, "insert count");
            Check::Print ((uint64_t)n1, "bvector capacity");
            }
        n0 = n1;            
        }
    }