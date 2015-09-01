//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HNumericTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"

#define MYEPSILON   0.000001f

//==================================================================================
// EQUAL
//==================================================================================
TEST (HNumericTester, EQUAL)
    {

    //Int32
    int32_t A=1;
    ASSERT_TRUE(HNumeric<int32_t>::EQUAL(A, A, A));
    ASSERT_FALSE(HNumeric<int32_t>::EQUAL(A, A + 2 * A, A));

    //UInt32
    uint32_t Au=1;
    ASSERT_TRUE(HNumeric<uint32_t>::EQUAL(Au, Au, Au));
    ASSERT_FALSE(HNumeric<uint32_t>::EQUAL(Au, Au + 2 * Au, Au));

    //Short
    short B=1;
    ASSERT_TRUE(HNumeric<short>::EQUAL(B, B, B));
    ASSERT_FALSE(HNumeric<short>::EQUAL(B, B + 2 * B, B));

    //UShort
    unsigned short Bu=1;
    ASSERT_TRUE(HNumeric<unsigned short>::EQUAL(Bu, Bu, Bu));
    ASSERT_FALSE(HNumeric<unsigned short>::EQUAL(Bu, Bu + 2 * Bu, Bu));

    //int32_t
    int32_t C=1;
    ASSERT_TRUE(HNumeric<int32_t>::EQUAL(C, C, C));
    ASSERT_FALSE(HNumeric<int32_t>::EQUAL(C, C + 2 * C, C));

    //uint32_t
    uint32_t Cu=1;
    ASSERT_TRUE(HNumeric<uint32_t>::EQUAL(Cu, Cu, Cu));
    ASSERT_FALSE(HNumeric<uint32_t>::EQUAL(Cu, Cu + 2 * Cu, Cu));

    //Int64
    int64_t D=1;
    ASSERT_TRUE(HNumeric<int64_t>::EQUAL(D, D, D));
    ASSERT_FALSE(HNumeric<int64_t>::EQUAL(D, D + 2 * D, D));

    //UInt64
    uint64_t Du=1;
    ASSERT_TRUE(HNumeric<uint64_t>::EQUAL(Du, Du, Du));
    ASSERT_FALSE(HNumeric<uint64_t>::EQUAL(Du, Du + 2 * Du, Du));

    //float
    float E=1.0;
    ASSERT_TRUE(HNumeric<float>::EQUAL(E, E, E));
    ASSERT_FALSE(HNumeric<float>::EQUAL(E, E + 2 * MYEPSILON, MYEPSILON));

    //double
    double F=1.0;
    ASSERT_TRUE(HNumeric<double>::EQUAL(F, F, F));
    ASSERT_FALSE(HNumeric<double>::EQUAL(F, F + 2 * MYEPSILON, MYEPSILON));

    }

//==================================================================================
// EQUAL_EPSILON
//==================================================================================
TEST (HNumericTester, EQUAL_EPSILON)
    {

    //Int32
    int32_t A=1;
    ASSERT_TRUE(HNumeric<int32_t>::EQUAL_EPSILON(A, A));
    ASSERT_FALSE(HNumeric<int32_t>::EQUAL_EPSILON(A, A + A));

    //Short
    short B=1;
    ASSERT_TRUE(HNumeric<short>::EQUAL_EPSILON(B, B));
    ASSERT_FALSE(HNumeric<short>::EQUAL_EPSILON(B, B + B));

    //int32_t
    int32_t C=1;
    ASSERT_TRUE(HNumeric<int32_t>::EQUAL_EPSILON(C, C));
    ASSERT_FALSE(HNumeric<int32_t>::EQUAL_EPSILON(C, C + C));

    //Int64
    int64_t D=1;
    ASSERT_TRUE(HNumeric<int64_t>::EQUAL_EPSILON(D, D));
    ASSERT_FALSE(HNumeric<int64_t>::EQUAL_EPSILON(D, D + D));

    //float
    float E=1.5;
    ASSERT_TRUE(HNumeric<float>::EQUAL_EPSILON(E, E));
    ASSERT_FALSE(HNumeric<float>::EQUAL_EPSILON(E, E + 2 * MYEPSILON));

    //double
    double F=1.0;
    ASSERT_TRUE(HNumeric<double>::EQUAL_EPSILON(F, F));
    ASSERT_FALSE(HNumeric<double>::EQUAL_EPSILON(F, F + 2 * MYEPSILON));

    }


//==================================================================================
// EQUAL_AUTO_EPSILON
//==================================================================================
TEST (HNumericTester, EQUAL_AUTO_EPSILON)
    {

    //Int32
    int32_t A=1;
    ASSERT_TRUE(HNumeric<int32_t>::EQUAL_AUTO_EPSILON(A, A));
    ASSERT_FALSE(HNumeric<int32_t>::EQUAL_AUTO_EPSILON(A, A + A));

    //Short
    short B=1;
    ASSERT_TRUE(HNumeric<short>::EQUAL_AUTO_EPSILON(B, B));
    ASSERT_FALSE(HNumeric<short>::EQUAL_AUTO_EPSILON(B, B + B));

    //int32_t
    int32_t C=1;
    ASSERT_TRUE(HNumeric<int32_t>::EQUAL_AUTO_EPSILON(C, C));
    ASSERT_FALSE(HNumeric<int32_t>::EQUAL_AUTO_EPSILON(C, C + C));

    //Int64
    int64_t D=1;
    ASSERT_TRUE(HNumeric<int64_t>::EQUAL_AUTO_EPSILON(D, D));
    ASSERT_FALSE(HNumeric<int64_t>::EQUAL_AUTO_EPSILON(D, D + D));

    //float
    float E=1.0;
    ASSERT_TRUE(HNumeric<float>::EQUAL_AUTO_EPSILON(E, E));
    ASSERT_FALSE(HNumeric<float>::EQUAL_AUTO_EPSILON(E, E + 2 * MYEPSILON));

    //double
    double F=1.0;
    ASSERT_TRUE(HNumeric<double>::EQUAL_AUTO_EPSILON(F, F));
    ASSERT_FALSE(HNumeric<double>::EQUAL_AUTO_EPSILON(F, F + 2 * MYEPSILON));

    }

//==================================================================================
// GREATER_OR_EQUAL
//==================================================================================
TEST (HNumericTester, GREATER_OR_EQUAL)
    {

    //Int32
    int32_t A=1;
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_OR_EQUAL(A, A, A));
    ASSERT_FALSE(HNumeric<int32_t>::GREATER_OR_EQUAL(A, A + 2 * A, A));
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_OR_EQUAL(A, -3 * A, A));

    //UInt32
    uint32_t Au=1;
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_OR_EQUAL(Au, Au, Au));
    ASSERT_FALSE(HNumeric<int32_t>::GREATER_OR_EQUAL(Au, Au + 2 * Au, Au));
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_OR_EQUAL(Au, -3 * Au, Au));

    //Short
    short B=1;
    ASSERT_TRUE(HNumeric<short>::GREATER_OR_EQUAL(B, B, B));
    ASSERT_FALSE(HNumeric<short>::GREATER_OR_EQUAL(B, B + 2 * B, B));
    ASSERT_TRUE(HNumeric<short>::GREATER_OR_EQUAL(B, -3 * B, B));

    //UShort
    unsigned short Bu=1;
    ASSERT_TRUE(HNumeric<short>::GREATER_OR_EQUAL(Bu, Bu, Bu));
    ASSERT_FALSE(HNumeric<short>::GREATER_OR_EQUAL(Bu, Bu + 2 * Bu, Bu));
    ASSERT_TRUE(HNumeric<short>::GREATER_OR_EQUAL(Bu, -3 * Bu, Bu));

    //int32_t
    int32_t C=1;
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_OR_EQUAL(C, C, C));
    ASSERT_FALSE(HNumeric<int32_t>::GREATER_OR_EQUAL(C, C + 2 * C, C));
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_OR_EQUAL(C, -3 * C, C));

    //uint32_t
    uint32_t Cu=1;
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_OR_EQUAL(Cu, Cu, Cu));
    ASSERT_FALSE(HNumeric<int32_t>::GREATER_OR_EQUAL(Cu, Cu + 2 * Cu, Cu));
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_OR_EQUAL(Cu, -3 * Cu, Cu));

    //Int64
    int64_t D=1;
    ASSERT_TRUE(HNumeric<int64_t>::GREATER_OR_EQUAL(D, D, D));
    ASSERT_FALSE(HNumeric<int64_t>::GREATER_OR_EQUAL(D, D + 2 * D, D));
    ASSERT_TRUE(HNumeric<int64_t>::GREATER_OR_EQUAL(D, -3 * D, D));

    //UInt64
    uint64_t Du=1;
    ASSERT_TRUE(HNumeric<int64_t>::GREATER_OR_EQUAL(Du, Du, Du));
    ASSERT_FALSE(HNumeric<int64_t>::GREATER_OR_EQUAL(Du, Du + 2 * Du, Du));
    ASSERT_TRUE(HNumeric<int64_t>::GREATER_OR_EQUAL(Du, -3 * Du, Du));

    //float
    float E=1.0;
    ASSERT_TRUE(HNumeric<float>::GREATER_OR_EQUAL(E, E, E));
    ASSERT_FALSE(HNumeric<float>::GREATER_OR_EQUAL(E, E + 2 * MYEPSILON, MYEPSILON));
    ASSERT_TRUE(HNumeric<float>::GREATER_OR_EQUAL(E, E - 2 * MYEPSILON, MYEPSILON));

    //double
    double F=1.0;
    ASSERT_TRUE(HNumeric<double>::GREATER_OR_EQUAL(F, F, MYEPSILON));
    ASSERT_FALSE(HNumeric<double>::GREATER_OR_EQUAL(F, F + 2 * MYEPSILON, MYEPSILON));
    ASSERT_TRUE(HNumeric<double>::GREATER_OR_EQUAL(F, F - 2 * MYEPSILON, MYEPSILON));

    }

//==================================================================================
// GREATER_OR_EQUAL_EPSILON
//==================================================================================
TEST (HNumericTester, GREATER_OR_EQUAL_EPSILON)
    {

    //Int32
    int32_t A=1;
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_OR_EQUAL_EPSILON(A, A));
    ASSERT_FALSE(HNumeric<int32_t>::GREATER_OR_EQUAL_EPSILON(A, A + 2 * A));
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_OR_EQUAL_EPSILON(A, -3 * A));

    //Short
    short B=1;
    ASSERT_TRUE(HNumeric<short>::GREATER_OR_EQUAL_EPSILON(B, B));
    ASSERT_FALSE(HNumeric<short>::GREATER_OR_EQUAL_EPSILON(B, B + 2 * B));
    ASSERT_TRUE(HNumeric<short>::GREATER_OR_EQUAL_EPSILON(B, -3 * B));

    //int32_t
    int32_t C=1;
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_OR_EQUAL_EPSILON(C, C));
    ASSERT_FALSE(HNumeric<int32_t>::GREATER_OR_EQUAL_EPSILON(C, C + 2 * C));
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_OR_EQUAL_EPSILON(C, -3 * C));

    //Int64
    int64_t D=1;
    ASSERT_TRUE(HNumeric<int64_t>::GREATER_OR_EQUAL_EPSILON(D, D));
    ASSERT_FALSE(HNumeric<int64_t>::GREATER_OR_EQUAL_EPSILON(D, D + 2 * D));
    ASSERT_TRUE(HNumeric<int64_t>::GREATER_OR_EQUAL_EPSILON(D, -3 * D));

    //float
    float E=1.0;
    ASSERT_TRUE(HNumeric<float>::GREATER_OR_EQUAL_EPSILON(E, E));
    ASSERT_FALSE(HNumeric<float>::GREATER_OR_EQUAL_EPSILON(E, E + 2 * MYEPSILON));
    ASSERT_TRUE(HNumeric<float>::GREATER_OR_EQUAL_EPSILON(E, E - 2 * MYEPSILON));

    //double
    double F=1.0;
    ASSERT_TRUE(HNumeric<double>::GREATER_OR_EQUAL_EPSILON(F, F));
    ASSERT_FALSE(HNumeric<double>::GREATER_OR_EQUAL_EPSILON(F, F + 2 * MYEPSILON));
    ASSERT_TRUE(HNumeric<double>::GREATER_OR_EQUAL_EPSILON(F, F - 2 * MYEPSILON));

    }

//==================================================================================
// SMALLER_OR_EQUAL
//==================================================================================
TEST (HNumericTester, SMALLER_OR_EQUAL)
    {

    //Int32
    int32_t A=1;
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_OR_EQUAL(A, A, A));
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_OR_EQUAL(A, A + 2 * A, A));
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER_OR_EQUAL(A, -3 * A, A));

    //UInt32
    uint32_t Au=1;
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_OR_EQUAL(Au, Au, Au));
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_OR_EQUAL(Au, Au + 2 * Au, Au));
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER_OR_EQUAL(Au, -3 * Au, Au));

    //Short
    short B=1;
    ASSERT_TRUE(HNumeric<short>::SMALLER_OR_EQUAL(B, B, B));
    ASSERT_TRUE(HNumeric<short>::SMALLER_OR_EQUAL(B, B + 2 * B, B));
    ASSERT_FALSE(HNumeric<short>::SMALLER_OR_EQUAL(B, -3 * B, B));

    //UShort
    unsigned short Bu=1;
    ASSERT_TRUE(HNumeric<short>::SMALLER_OR_EQUAL(Bu, Bu, Bu));
    ASSERT_TRUE(HNumeric<short>::SMALLER_OR_EQUAL(Bu, Bu + 2 * Bu, Bu));
    ASSERT_FALSE(HNumeric<short>::SMALLER_OR_EQUAL(Bu, -3 * Bu, Bu));

    //int32_t
    int32_t C=1;
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_OR_EQUAL(C, C, C));
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_OR_EQUAL(C, C + 2 * C, C));
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER_OR_EQUAL(C, -3 * C, C));

    //uint32_t
    uint32_t Cu=1;
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_OR_EQUAL(Cu, Cu, Cu));
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_OR_EQUAL(Cu, Cu + 2 * Cu, Cu));
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER_OR_EQUAL(Cu, -3 * Cu, Cu));

    //Int64
    int64_t D=1;
    ASSERT_TRUE(HNumeric<int64_t>::SMALLER_OR_EQUAL(D, D, D));
    ASSERT_TRUE(HNumeric<int64_t>::SMALLER_OR_EQUAL(D, D + 2 * D, D));
    ASSERT_FALSE(HNumeric<int64_t>::SMALLER_OR_EQUAL(D, -3 * D, D));

    //UInt64
    uint64_t Du=1;
    ASSERT_TRUE(HNumeric<int64_t>::SMALLER_OR_EQUAL(Du, Du, Du));
    ASSERT_TRUE(HNumeric<int64_t>::SMALLER_OR_EQUAL(Du, Du + 2 * Du, Du));
    ASSERT_FALSE(HNumeric<int64_t>::SMALLER_OR_EQUAL(Du, -3 * Du, Du));

    //float
    float E=1.0;
    ASSERT_TRUE(HNumeric<float>::SMALLER_OR_EQUAL(E, E, E));
    ASSERT_TRUE(HNumeric<float>::SMALLER_OR_EQUAL(E, E + 2 * MYEPSILON, MYEPSILON));
    ASSERT_FALSE(HNumeric<float>::SMALLER_OR_EQUAL(E, E - 2 * MYEPSILON, MYEPSILON));

    //double
    double F=1.0;
    ASSERT_TRUE(HNumeric<double>::SMALLER_OR_EQUAL(F, F, MYEPSILON));
    ASSERT_TRUE(HNumeric<double>::SMALLER_OR_EQUAL(F, F + 2 * MYEPSILON, MYEPSILON));
    ASSERT_FALSE(HNumeric<double>::SMALLER_OR_EQUAL(F, F - 2 * MYEPSILON, MYEPSILON));

    }

//==================================================================================
// SMALLER_OR_EQUAL_EPSILON
//==================================================================================
TEST (HNumericTester, SMALLER_OR_EQUAL_EPSILON)
    {

    //Int32
    int32_t A=1;
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_OR_EQUAL_EPSILON(A, A));
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_OR_EQUAL_EPSILON(A, A + 2 * A));
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER_OR_EQUAL_EPSILON(A, -3 * A));

    //Short
    short B=1;
    ASSERT_TRUE(HNumeric<short>::SMALLER_OR_EQUAL_EPSILON(B, B));
    ASSERT_TRUE(HNumeric<short>::SMALLER_OR_EQUAL_EPSILON(B, B + 2 * B));
    ASSERT_FALSE(HNumeric<short>::SMALLER_OR_EQUAL_EPSILON(B, -3 * B));

    //int32_t
    int32_t C=1;
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_OR_EQUAL_EPSILON(C, C));
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_OR_EQUAL_EPSILON(C, C + 2 * C));
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER_OR_EQUAL_EPSILON(C, -3 * C));

    //Int64
    int64_t D=1;
    ASSERT_TRUE(HNumeric<int64_t>::SMALLER_OR_EQUAL_EPSILON(D, D));
    ASSERT_TRUE(HNumeric<int64_t>::SMALLER_OR_EQUAL_EPSILON(D, D + 2 * D));
    ASSERT_FALSE(HNumeric<int64_t>::SMALLER_OR_EQUAL_EPSILON(D, -3 * D));

    //float
    float E=1.0;
    ASSERT_TRUE(HNumeric<float>::SMALLER_OR_EQUAL_EPSILON(E, E));
    ASSERT_TRUE(HNumeric<float>::SMALLER_OR_EQUAL_EPSILON(E, E + 2 * MYEPSILON));
    ASSERT_FALSE(HNumeric<float>::SMALLER_OR_EQUAL_EPSILON(E, E - 2 * MYEPSILON));

    //double
    double F=1.0;
    ASSERT_TRUE(HNumeric<double>::SMALLER_OR_EQUAL_EPSILON(F, F));
    ASSERT_TRUE(HNumeric<double>::SMALLER_OR_EQUAL_EPSILON(F, F + 2 * MYEPSILON));
    ASSERT_FALSE(HNumeric<double>::SMALLER_OR_EQUAL_EPSILON(F, F - 2 * MYEPSILON));

    }

//==================================================================================
// GREATER
//==================================================================================
TEST (HNumericTester, GREATER)
    {

    //Int32
    int32_t A=1;
    ASSERT_FALSE(HNumeric<int32_t>::GREATER(A, A, A));
    ASSERT_FALSE(HNumeric<int32_t>::GREATER(A, A + 2 * A, A));
    ASSERT_TRUE(HNumeric<int32_t>::GREATER(A, -3 * A, A));

    //UInt32
    uint32_t Au=1;
    ASSERT_FALSE(HNumeric<int32_t>::GREATER(Au, Au, Au));
    ASSERT_FALSE(HNumeric<int32_t>::GREATER(Au, Au + 2 * Au, Au));
    ASSERT_TRUE(HNumeric<int32_t>::GREATER(Au, -3 * Au, Au));

    //Short
    short B=1;
    ASSERT_FALSE(HNumeric<short>::GREATER(B, B, B));
    ASSERT_FALSE(HNumeric<short>::GREATER(B, B + 2 * B, B));
    ASSERT_TRUE(HNumeric<short>::GREATER(B, -3 * B, B));

    //UShort
    unsigned short Bu=1;
    ASSERT_FALSE(HNumeric<short>::GREATER(Bu, Bu, Bu));
    ASSERT_FALSE(HNumeric<short>::GREATER(Bu, Bu + 2 * Bu, Bu));
    ASSERT_TRUE(HNumeric<short>::GREATER(Bu, -3 * Bu, Bu));

    //int32_t
    int32_t C=1;
    ASSERT_FALSE(HNumeric<int32_t>::GREATER(C, C, C));
    ASSERT_FALSE(HNumeric<int32_t>::GREATER(C, C + 2 * C, C));
    ASSERT_TRUE(HNumeric<int32_t>::GREATER(C, -3 * C, C));

    //uint32_t
    uint32_t Cu=1;
    ASSERT_FALSE(HNumeric<int32_t>::GREATER(Cu, Cu, Cu));
    ASSERT_FALSE(HNumeric<int32_t>::GREATER(Cu, Cu + 2 * Cu, Cu));
    ASSERT_TRUE(HNumeric<int32_t>::GREATER(Cu, -3 * Cu, Cu));

    //Int64
    int64_t D=1;
    ASSERT_FALSE(HNumeric<int64_t>::GREATER(D, D, D));
    ASSERT_FALSE(HNumeric<int64_t>::GREATER(D, D + 2 * D, D));
    ASSERT_TRUE(HNumeric<int64_t>::GREATER(D, -3 * D, D));

    //UInt64
    uint64_t Du=1;
    ASSERT_FALSE(HNumeric<int64_t>::GREATER(Du, Du, Du));
    ASSERT_FALSE(HNumeric<int64_t>::GREATER(Du, Du + 2 * Du, Du));
    ASSERT_TRUE(HNumeric<int64_t>::GREATER(Du, -3 * Du, Du));

    //float
    float E=1.0;
    ASSERT_FALSE(HNumeric<float>::GREATER(E, E, E));
    ASSERT_FALSE(HNumeric<float>::GREATER(E, E + 2 * MYEPSILON, MYEPSILON));
    ASSERT_TRUE(HNumeric<float>::GREATER(E, E - 2 * MYEPSILON, MYEPSILON));

    //double
    double F=1.0;
    ASSERT_FALSE(HNumeric<double>::GREATER(F, F, MYEPSILON));
    ASSERT_FALSE(HNumeric<double>::GREATER(F, F + 2 * MYEPSILON, MYEPSILON));
    ASSERT_TRUE(HNumeric<double>::GREATER(F, F - 2 * MYEPSILON, MYEPSILON));

    }

//==================================================================================
// GREATER_EPSILON
//==================================================================================
TEST (HNumericTester, GREATER_EPSILON)
    {

    //Int32
    int32_t A=1;
    ASSERT_FALSE(HNumeric<int32_t>::GREATER_EPSILON(A, A));
    ASSERT_FALSE(HNumeric<int32_t>::GREATER_EPSILON(A, A + 2 * A));
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_EPSILON(A, -3 * A));

    //Short
    short B=1;
    ASSERT_FALSE(HNumeric<short>::GREATER_EPSILON(B, B));
    ASSERT_FALSE(HNumeric<short>::GREATER_EPSILON(B, B + 2 * B));
    ASSERT_TRUE(HNumeric<short>::GREATER_EPSILON(B, -3 * B));

    //int32_t
    int32_t C=1;
    ASSERT_FALSE(HNumeric<int32_t>::GREATER_EPSILON(C, C));
    ASSERT_FALSE(HNumeric<int32_t>::GREATER_EPSILON(C, C + 2 * C));
    ASSERT_TRUE(HNumeric<int32_t>::GREATER_EPSILON(C, -3 * C));

    //Int64
    int64_t D=1;
    ASSERT_FALSE(HNumeric<int64_t>::GREATER_EPSILON(D, D));
    ASSERT_FALSE(HNumeric<int64_t>::GREATER_EPSILON(D, D + 2 * D));
    ASSERT_TRUE(HNumeric<int64_t>::GREATER_EPSILON(D, -3 * D));

    //float
    float E=1.0;
    ASSERT_FALSE(HNumeric<float>::GREATER_EPSILON(E, E));
    ASSERT_FALSE(HNumeric<float>::GREATER_EPSILON(E, E + 2 * MYEPSILON));
    ASSERT_TRUE(HNumeric<float>::GREATER_EPSILON(E, E - 2 * MYEPSILON));

    //double
    double F=1.0;
    ASSERT_FALSE(HNumeric<double>::GREATER_EPSILON(F, F));
    ASSERT_FALSE(HNumeric<double>::GREATER_EPSILON(F, F + 2 * MYEPSILON));
    ASSERT_TRUE(HNumeric<double>::GREATER_EPSILON(F, F - 2 * MYEPSILON));

    }

//==================================================================================
// SMALLER
//==================================================================================
TEST (HNumericTester, SMALLER)
    {

    //Int32
    int32_t A=1;
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER(A, A, A));
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER(A, A + 2 * A, A));
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER(A, -3 * A, A));

    //UInt32
    uint32_t Au=1;
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER(Au, Au, Au));
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER(Au, Au + 2 * Au, Au));
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER(Au, -3 * Au, Au));

    //Short
    short B=1;
    ASSERT_FALSE(HNumeric<short>::SMALLER(B, B, B));
    ASSERT_TRUE(HNumeric<short>::SMALLER(B, B + 2 * B, B));
    ASSERT_FALSE(HNumeric<short>::SMALLER(B, -3 * B, B));

    //UShort
    unsigned short Bu=1;
    ASSERT_FALSE(HNumeric<short>::SMALLER(Bu, Bu, Bu));
    ASSERT_TRUE(HNumeric<short>::SMALLER(Bu, Bu + 2 * Bu, Bu));
    ASSERT_FALSE(HNumeric<short>::SMALLER(Bu, -3 * Bu, Bu));

    //int32_t
    int32_t C=1;
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER(C, C, C));
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER(C, C + 2 * C, C));
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER(C, -3 * C, C));

    //uint32_t
    uint32_t Cu=1;
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER(Cu, Cu, Cu));
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER(Cu, Cu + 2 * Cu, Cu));
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER(Cu, -3 * Cu, Cu));

    //Int64
    int64_t D=1;
    ASSERT_FALSE(HNumeric<int64_t>::SMALLER(D, D, D));
    ASSERT_TRUE(HNumeric<int64_t>::SMALLER(D, D + 2 * D, D));
    ASSERT_FALSE(HNumeric<int64_t>::SMALLER(D, -3 * D, D));

    //UInt64
    uint64_t Du=1;
    ASSERT_FALSE(HNumeric<int64_t>::SMALLER(Du, Du, Du));
    ASSERT_TRUE(HNumeric<int64_t>::SMALLER(Du, Du + 2 * Du, Du));
    ASSERT_FALSE(HNumeric<int64_t>::SMALLER(Du, -3 * Du, Du));

    //float
    float E=1.0;
    ASSERT_FALSE(HNumeric<float>::SMALLER(E, E, E));
    ASSERT_TRUE(HNumeric<float>::SMALLER(E, E + 2 * MYEPSILON, MYEPSILON));
    ASSERT_FALSE(HNumeric<float>::SMALLER(E, E - 2 * MYEPSILON, MYEPSILON));

    //double
    double F=1.0;
    ASSERT_FALSE(HNumeric<double>::SMALLER(F, F, MYEPSILON));
    ASSERT_TRUE(HNumeric<double>::SMALLER(F, F + 2 * MYEPSILON, MYEPSILON));
    ASSERT_FALSE(HNumeric<double>::SMALLER(F, F - 2 * MYEPSILON, MYEPSILON));

    }

//==================================================================================
// SMALLER_EPSILON
//==================================================================================
TEST (HNumericTester, SMALLER_EPSILON)
    {

    //Int32
    int32_t A=1;
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_EPSILON(A, A));
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_EPSILON(A, A + 2 * A));
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER_EPSILON(A, -3 * A));

    //Short
    short B=1;
    ASSERT_TRUE(HNumeric<short>::SMALLER_EPSILON(B, B));
    ASSERT_TRUE(HNumeric<short>::SMALLER_EPSILON(B, B + 2 * B));
    ASSERT_FALSE(HNumeric<short>::SMALLER_EPSILON(B, -3 * B));

    //int32_t
    int32_t C=1;
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_EPSILON(C, C));
    ASSERT_TRUE(HNumeric<int32_t>::SMALLER_EPSILON(C, C + 2 * C));
    ASSERT_FALSE(HNumeric<int32_t>::SMALLER_EPSILON(C, -3 * C));

    //Int64
    int64_t D=1;
    ASSERT_TRUE(HNumeric<int64_t>::SMALLER_EPSILON(D, D));
    ASSERT_TRUE(HNumeric<int64_t>::SMALLER_EPSILON(D, D + 2 * D));
    ASSERT_FALSE(HNumeric<int64_t>::SMALLER_EPSILON(D, -3 * D));

    //float
    float E=1.0;
    ASSERT_FALSE(HNumeric<float>::SMALLER_EPSILON(E, E));
    ASSERT_TRUE(HNumeric<float>::SMALLER_EPSILON(E, E + 2 * MYEPSILON));
    ASSERT_FALSE(HNumeric<float>::SMALLER_EPSILON(E, E - 2 * MYEPSILON));

    //double
    double F=1.0;
    ASSERT_FALSE(HNumeric<double>::SMALLER_EPSILON(F, F));
    ASSERT_TRUE(HNumeric<double>::SMALLER_EPSILON(F, F + 2 * MYEPSILON));
    ASSERT_FALSE(HNumeric<double>::SMALLER_EPSILON(F, F - 2 * MYEPSILON));

    }
