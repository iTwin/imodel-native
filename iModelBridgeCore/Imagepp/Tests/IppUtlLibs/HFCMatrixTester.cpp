//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppUtlLibs/HFCMatrixTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())

// Preparation of required environement
class HFCMatrixTester : public ::testing::Test
    {   

protected :

    HFCMatrixTester();
    
    typedef HFCMatrix<3, 3> A3By3Matrix;

    int32_t i;
    int32_t j;

    A3By3Matrix Mat1;
    A3By3Matrix Mat1A;

    typedef HFCMatrix<3, 3, int32_t> A3By3int32_tMatrix;

    A3By3int32_tMatrix Mat2;
    A3By3int32_tMatrix Mat2A;
    
    };

HFCMatrixTester::HFCMatrixTester()
    {

    for(i = 0 ; i < 3 ; ++i)
        for(j = 0; j < 3 ; ++j)
            Mat1[i][j] = i+j;

    for(i = 0 ; i < 3 ; ++i)
        for(j = 0; j < 3 ; ++j)
            Mat2[i][j] = i+j;
            
    Mat1A = Mat1;
    Mat2A = Mat2;

    }

//==================================================================================
// Default construction test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixConstructor)
    {

    A3By3Matrix DefaultMat;

    ASSERT_NEAR(0.0, DefaultMat[0][0] , MYEPSILON);
    ASSERT_NEAR(0.0, DefaultMat[0][1] , MYEPSILON);
    ASSERT_NEAR(0.0, DefaultMat[0][2] , MYEPSILON);
    ASSERT_NEAR(0.0, DefaultMat[1][0] , MYEPSILON);
    ASSERT_NEAR(0.0, DefaultMat[1][1] , MYEPSILON);
    ASSERT_NEAR(0.0, DefaultMat[1][2] , MYEPSILON);
    ASSERT_NEAR(0.0, DefaultMat[2][0] , MYEPSILON);
    ASSERT_NEAR(0.0, DefaultMat[2][1] , MYEPSILON);
    ASSERT_NEAR(0.0, DefaultMat[2][2] , MYEPSILON);

    }

//==================================================================================
// Setting test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixSetting)
    {
    
    ASSERT_NEAR(0.0, Mat1[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Mat1[0][1]);
    ASSERT_DOUBLE_EQ(2.0, Mat1[0][2]);
    ASSERT_DOUBLE_EQ(1.0, Mat1[1][0]);
    ASSERT_DOUBLE_EQ(2.0, Mat1[1][1]);
    ASSERT_DOUBLE_EQ(3.0, Mat1[1][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat1[2][0]);
    ASSERT_DOUBLE_EQ(3.0, Mat1[2][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat1[2][2]);

    }

//==================================================================================
// Copy construction test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixCopyConstructor)
    {
  
    const A3By3Matrix Mat1Const(Mat1);
    ASSERT_NEAR(0.0, Mat1Const[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Mat1Const[0][1]);
    ASSERT_DOUBLE_EQ(2.0, Mat1Const[0][2]);
    ASSERT_DOUBLE_EQ(1.0, Mat1Const[1][0]);
    ASSERT_DOUBLE_EQ(2.0, Mat1Const[1][1]);
    ASSERT_DOUBLE_EQ(3.0, Mat1Const[1][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat1Const[2][0]);
    ASSERT_DOUBLE_EQ(3.0, Mat1Const[2][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat1Const[2][2]);

    }

//==================================================================================
// Assigment test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixAssigment)
    {

    ASSERT_NEAR(0.0, Mat1A[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Mat1A[0][1]);
    ASSERT_DOUBLE_EQ(2.0, Mat1A[0][2]);
    ASSERT_DOUBLE_EQ(1.0, Mat1A[1][0]);
    ASSERT_DOUBLE_EQ(2.0, Mat1A[1][1]);
    ASSERT_DOUBLE_EQ(3.0, Mat1A[1][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat1A[2][0]);
    ASSERT_DOUBLE_EQ(3.0, Mat1A[2][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat1A[2][2]);

    }

//==================================================================================
// Value related method test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixValue)
    {
    
    for(i = 0 ; i < 3 ; ++i)
        for(j = 0; j < 3 ; ++j)
            Mat1[i][j] = Mat1A[i][j] * 3.0;

    ASSERT_NEAR(0.0, Mat1[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(3.0, Mat1[0][1]);
    ASSERT_DOUBLE_EQ(6.0, Mat1[0][2]);
    ASSERT_DOUBLE_EQ(3.0, Mat1[1][0]);
    ASSERT_DOUBLE_EQ(6.0, Mat1[1][1]);
    ASSERT_DOUBLE_EQ(9.0, Mat1[1][2]);
    ASSERT_DOUBLE_EQ(6.0, Mat1[2][0]);
    ASSERT_DOUBLE_EQ(9.0, Mat1[2][1]);
    ASSERT_DOUBLE_EQ(12.0, Mat1[2][2]);

    }

//==================================================================================
// Value related method by row
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixValueByRow)
    {

    const A3By3Matrix Mat1Const(Mat1);
    Mat1[0] = Mat1Const[0];
    Mat1[1] = Mat1Const[1];
    Mat1[2] = Mat1Const[2];

    ASSERT_NEAR(0.0, Mat1[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Mat1[0][1]);
    ASSERT_DOUBLE_EQ(2.0, Mat1[0][2]);
    ASSERT_DOUBLE_EQ(1.0, Mat1[1][0]);
    ASSERT_DOUBLE_EQ(2.0, Mat1[1][1]);
    ASSERT_DOUBLE_EQ(3.0, Mat1[1][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat1[2][0]);
    ASSERT_DOUBLE_EQ(3.0, Mat1[2][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat1[2][2]);

    }

//==================================================================================
// Array multiplication test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixMultiplication)
    {

    HFCMatrix<3, 1> SmallArray;

    SmallArray[0][0] = 2.0;
    SmallArray[1][0] = 4.0;
    SmallArray[2][0] = 6.0;

    HFCMatrix<3, 1> ResultArray;

    ResultArray = Mat1 * SmallArray;

    ASSERT_DOUBLE_EQ(16.0, ResultArray[0][0]);
    ASSERT_DOUBLE_EQ(28.0, ResultArray[1][0]);
    ASSERT_DOUBLE_EQ(40.0, ResultArray[2][0]);

    }

//==================================================================================
// operator * a constant test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixOperator1)
    {

    const A3By3Matrix Mat1Const(Mat1);
    Mat1 = Mat1Const * 2.0;

    ASSERT_NEAR(0.0, Mat1[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0, Mat1[0][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat1[0][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat1[1][0]);
    ASSERT_DOUBLE_EQ(4.0, Mat1[1][1]);
    ASSERT_DOUBLE_EQ(6.0, Mat1[1][2]);
    ASSERT_DOUBLE_EQ(4.0, Mat1[2][0]);
    ASSERT_DOUBLE_EQ(6.0, Mat1[2][1]);
    ASSERT_DOUBLE_EQ(8.0, Mat1[2][2]);

    }

//==================================================================================
// operator / a constant test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixOperator2)
    {
   
    Mat1 = Mat1 / 2.0;

    ASSERT_NEAR(0.0, Mat1[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(0.5, Mat1[0][1]);
    ASSERT_DOUBLE_EQ(1.0, Mat1[0][2]);
    ASSERT_DOUBLE_EQ(0.5, Mat1[1][0]);
    ASSERT_DOUBLE_EQ(1.0, Mat1[1][1]);
    ASSERT_DOUBLE_EQ(1.5, Mat1[1][2]);
    ASSERT_DOUBLE_EQ(1.0, Mat1[2][0]);
    ASSERT_DOUBLE_EQ(1.5, Mat1[2][1]);
    ASSERT_DOUBLE_EQ(2.0, Mat1[2][2]);

    }

//==================================================================================
// Constant * a matrix test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixOperator3)
    {
    
    const A3By3Matrix Mat1Const(Mat1);
    Mat1A = 2.0 * Mat1Const;

    ASSERT_NEAR(0.0, Mat1A[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0, Mat1A[0][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat1A[0][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat1A[1][0]);
    ASSERT_DOUBLE_EQ(4.0, Mat1A[1][1]);
    ASSERT_DOUBLE_EQ(6.0, Mat1A[1][2]);
    ASSERT_DOUBLE_EQ(4.0, Mat1A[2][0]);
    ASSERT_DOUBLE_EQ(6.0, Mat1A[2][1]);
    ASSERT_DOUBLE_EQ(8.0, Mat1A[2][2]);

    }

//==================================================================================
// Increment test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixIncrement)
    {
    
    const A3By3Matrix Mat1Const(Mat1);
    Mat1 += Mat1Const;

    ASSERT_NEAR(0.0, Mat1[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0, Mat1[0][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat1[0][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat1[1][0]);
    ASSERT_DOUBLE_EQ(4.0, Mat1[1][1]);
    ASSERT_DOUBLE_EQ(6.0, Mat1[1][2]);
    ASSERT_DOUBLE_EQ(4.0, Mat1[2][0]);
    ASSERT_DOUBLE_EQ(6.0, Mat1[2][1]);
    ASSERT_DOUBLE_EQ(8.0, Mat1[2][2]);

    }

//==================================================================================
// Decrement test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixDecrement)
    {

    const A3By3Matrix Mat1Const(Mat1); 
    Mat1 -= Mat1Const;

    ASSERT_NEAR(0.0, Mat1[0][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1[0][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1[0][2] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1[1][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1[1][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1[1][2] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1[2][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1[2][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1[2][2] , MYEPSILON);

    }

//==================================================================================
// Scale tests
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixScale)
    {
    
    Mat1 *= 2.0;
    ASSERT_NEAR(0.0, Mat1[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0, Mat1[0][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat1[0][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat1[1][0]);
    ASSERT_DOUBLE_EQ(4.0, Mat1[1][1]);
    ASSERT_DOUBLE_EQ(6.0, Mat1[1][2]);
    ASSERT_DOUBLE_EQ(4.0, Mat1[2][0]);
    ASSERT_DOUBLE_EQ(6.0, Mat1[2][1]);
    ASSERT_DOUBLE_EQ(8.0, Mat1[2][2]);

    Mat1 /= -2.0;
    ASSERT_NEAR(0.0, Mat1[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.0, Mat1[0][1]);
    ASSERT_DOUBLE_EQ(-2.0, Mat1[0][2]);
    ASSERT_DOUBLE_EQ(-1.0, Mat1[1][0]);
    ASSERT_DOUBLE_EQ(-2.0, Mat1[1][1]);
    ASSERT_DOUBLE_EQ(-3.0, Mat1[1][2]);
    ASSERT_DOUBLE_EQ(-2.0, Mat1[2][0]);
    ASSERT_DOUBLE_EQ(-3.0, Mat1[2][1]);
    ASSERT_DOUBLE_EQ(-4.0, Mat1[2][2]);

    }

//==================================================================================
// Negation test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixNegation)
    {

    const A3By3Matrix Mat1Const(Mat1);
    Mat1 = -Mat1Const;

    ASSERT_NEAR(0.0, Mat1[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.0, Mat1[0][1]);
    ASSERT_DOUBLE_EQ(-2.0, Mat1[0][2]);
    ASSERT_DOUBLE_EQ(-1.0, Mat1[1][0]);
    ASSERT_DOUBLE_EQ(-2.0, Mat1[1][1]);
    ASSERT_DOUBLE_EQ(-3.0, Mat1[1][2]);
    ASSERT_DOUBLE_EQ(-2.0, Mat1[2][0]);
    ASSERT_DOUBLE_EQ(-3.0, Mat1[2][1]);
    ASSERT_DOUBLE_EQ(-4.0, Mat1[2][2]);

    }

//==================================================================================
// Addition test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixAddition)
    {
    
    Mat1A = Mat1 + Mat1;

    ASSERT_DOUBLE_EQ(2.0, Mat1A[0][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat1A[0][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat1A[1][0]);
    ASSERT_DOUBLE_EQ(4.0, Mat1A[1][1]);
    ASSERT_DOUBLE_EQ(6.0, Mat1A[1][2]);
    ASSERT_DOUBLE_EQ(4.0, Mat1A[2][0]);
    ASSERT_DOUBLE_EQ(6.0, Mat1A[2][1]);
    ASSERT_DOUBLE_EQ(8.0, Mat1A[2][2]);

    }

//==================================================================================
// Substraction test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixSubstraction)
    {
    
    Mat1A = Mat1 - Mat1;

    ASSERT_NEAR(0.0, Mat1A[0][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1A[0][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1A[0][2] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1A[1][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1A[1][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1A[1][2] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1A[2][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1A[2][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat1A[2][2] , MYEPSILON);

    }

//==================================================================================
// Transpose test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixTranspose)
    {
    
    Mat1[2][0] = 35.0;
    A3By3Matrix Mat4(Mat1.CalculateTranspose());

    ASSERT_NEAR(0.0, Mat4[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Mat4[0][1]);
    ASSERT_DOUBLE_EQ(35.0, Mat4[0][2]);
    ASSERT_DOUBLE_EQ(1.0, Mat4[1][0]);
    ASSERT_DOUBLE_EQ(2.0, Mat4[1][1]);
    ASSERT_DOUBLE_EQ(3.0, Mat4[1][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat4[2][0]);
    ASSERT_DOUBLE_EQ(3.0, Mat4[2][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat4[2][2]);

    }

//==================================================================================
// Compare operation test
//==================================================================================
TEST_F (HFCMatrixTester, DoubleMatrixCompare)
    {

    ASSERT_TRUE(Mat1A == Mat1);
    ASSERT_FALSE((Mat1A != Mat1));
    ASSERT_TRUE(Mat1.IsEqualTo(Mat1A));
    ASSERT_TRUE(Mat1.IsEqualTo(Mat1A, 1E-6));

    Mat1A[0][0] = 23.6;
    ASSERT_TRUE(Mat1 != Mat1A);
    ASSERT_FALSE((Mat1 == Mat1A));
    ASSERT_FALSE(Mat1.IsEqualTo(Mat1A));
    ASSERT_FALSE(Mat1.IsEqualTo(Mat1A, 1E-6));

    Mat1A[0][0] = Mat1[0][0] + 1E-12;
    ASSERT_TRUE(Mat1 != Mat1A);
    ASSERT_FALSE((Mat1 == Mat1A));
    ASSERT_TRUE(Mat1.IsEqualTo(Mat1A));
    ASSERT_FALSE(Mat1.IsEqualTo(Mat1A, 1E-14));
    ASSERT_TRUE(Mat1.IsEqualTo(Mat1A, 1E-11));

    }

//==================================================================================
// Default construction test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongConstructor)
    {

    A3By3int32_tMatrix Mat2;

    ASSERT_NEAR(0.0, Mat2[0][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[0][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[0][2] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[1][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[1][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[1][2] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[2][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[2][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[2][2] , MYEPSILON);

    }

//==================================================================================
// Setting test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongSetting)
    {
       
    ASSERT_NEAR(0.0, Mat2[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Mat2[0][1]);
    ASSERT_DOUBLE_EQ(2.0, Mat2[0][2]);
    ASSERT_DOUBLE_EQ(1.0, Mat2[1][0]);
    ASSERT_DOUBLE_EQ(2.0, Mat2[1][1]);
    ASSERT_DOUBLE_EQ(3.0, Mat2[1][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat2[2][0]);
    ASSERT_DOUBLE_EQ(3.0, Mat2[2][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat2[2][2]);

    }

//==================================================================================
// Copy construction test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongCopyConstructor)
    {
  
    const A3By3int32_tMatrix Mat2Const(Mat2);

    ASSERT_NEAR(0.0, Mat2Const[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Mat2Const[0][1]);
    ASSERT_DOUBLE_EQ(2.0, Mat2Const[0][2]);
    ASSERT_DOUBLE_EQ(1.0, Mat2Const[1][0]);
    ASSERT_DOUBLE_EQ(2.0, Mat2Const[1][1]);
    ASSERT_DOUBLE_EQ(3.0, Mat2Const[1][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat2Const[2][0]);
    ASSERT_DOUBLE_EQ(3.0, Mat2Const[2][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat2Const[2][2]);

    }

//==================================================================================
// Assigment test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongAssignement)
    {

    ASSERT_NEAR(0.0, Mat2A[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Mat2A[0][1]);
    ASSERT_DOUBLE_EQ(2.0, Mat2A[0][2]);
    ASSERT_DOUBLE_EQ(1.0, Mat2A[1][0]);
    ASSERT_DOUBLE_EQ(2.0, Mat2A[1][1]);
    ASSERT_DOUBLE_EQ(3.0, Mat2A[1][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat2A[2][0]);
    ASSERT_DOUBLE_EQ(3.0, Mat2A[2][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat2A[2][2]);

    }

//==================================================================================
// Value related method test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongValue)
    {
    
    for(i = 0 ; i < 3 ; ++i)
        for(j = 0; j < 3 ; ++j)
            Mat2[i][j] = Mat2A[i][j] * 3;

    ASSERT_NEAR(0.0, Mat2[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(3.0, Mat2[0][1]);
    ASSERT_DOUBLE_EQ(6.0, Mat2[0][2]);
    ASSERT_DOUBLE_EQ(3.0, Mat2[1][0]);
    ASSERT_DOUBLE_EQ(6.0, Mat2[1][1]);
    ASSERT_DOUBLE_EQ(9.0, Mat2[1][2]);
    ASSERT_DOUBLE_EQ(6.0, Mat2[2][0]);
    ASSERT_DOUBLE_EQ(9.0, Mat2[2][1]);
    ASSERT_DOUBLE_EQ(12.0, Mat2[2][2]);

    }

//==================================================================================
// Value related method by row
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongValueByRow)
    {
    
    const A3By3int32_tMatrix Mat2Const(Mat2);
    Mat2[0] = Mat2Const[0];
    Mat2[1] = Mat2Const[1];
    Mat2[2] = Mat2Const[2];

    ASSERT_NEAR(0.0, Mat1A[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Mat2[0][1]);
    ASSERT_DOUBLE_EQ(2.0, Mat2[0][2]);
    ASSERT_DOUBLE_EQ(1.0, Mat2[1][0]);
    ASSERT_DOUBLE_EQ(2.0, Mat2[1][1]);
    ASSERT_DOUBLE_EQ(3.0, Mat2[1][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat2[2][0]);
    ASSERT_DOUBLE_EQ(3.0, Mat2[2][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat2[2][2]);

    }

//==================================================================================
// Array multiplication test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongMultiplication)
    {

    HFCMatrix<3, 1, int32_t> SmallArray;

    SmallArray[0][0] = 2;
    SmallArray[1][0] = 4;
    SmallArray[2][0] = 6;

    HFCMatrix<3, 1, int32_t> ResultArray;

    ResultArray = Mat2 * SmallArray;

    ASSERT_DOUBLE_EQ(16.0, ResultArray[0][0]);
    ASSERT_DOUBLE_EQ(28.0, ResultArray[1][0]);
    ASSERT_DOUBLE_EQ(40.0, ResultArray[2][0]);

    }

//==================================================================================
// operator * a constant test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongOperator1)
    {
    
    #ifdef WIP_IPPTEST_BUG_20

    //const A3By3int32_tMatrix Mat2Const(Mat2);
    //Mat2A = 2.0 * Mat2Const;

    //ASSERT_DOUBLE_EQ(Mat2[0][0] , 0);
    //ASSERT_DOUBLE_EQ(Mat2[0][1] , 2);
    //ASSERT_DOUBLE_EQ(Mat2[0][2] , 4);
    //ASSERT_DOUBLE_EQ(Mat2[1][0] , 2);
    //ASSERT_DOUBLE_EQ(Mat2[1][1] , 4);
    //ASSERT_DOUBLE_EQ(Mat2[1][2] , 6);
    //ASSERT_DOUBLE_EQ(Mat2[2][0] , 4);
    //ASSERT_DOUBLE_EQ(Mat2[2][1] , 6);
    //ASSERT_DOUBLE_EQ(Mat2[2][2] , 8);

    #endif

    }

//==================================================================================
// operator / a constant test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongOperator2)
    {
    
    Mat2 = Mat2 / 2;

    ASSERT_DOUBLE_EQ(Mat2[0][0] , 0);
    ASSERT_DOUBLE_EQ(Mat2[0][1] , 0);
    ASSERT_DOUBLE_EQ(Mat2[0][2] , 1);
    ASSERT_DOUBLE_EQ(Mat2[1][0] , 0);
    ASSERT_DOUBLE_EQ(Mat2[1][1] , 1);
    ASSERT_DOUBLE_EQ(Mat2[1][2] , 1);
    ASSERT_DOUBLE_EQ(Mat2[2][0] , 1);
    ASSERT_DOUBLE_EQ(Mat2[2][1] , 1);
    ASSERT_DOUBLE_EQ(Mat2[2][2] , 2);

    }

//==================================================================================
// Constant * a matrix test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongOperator3)
    {
   
    #ifdef WIP_IPPTEST_BUG_20

    //const A3By3int32_tMatrix Mat2Const(Mat2);
    //Mat2A = 2.0 * Mat2Const;

    //ASSERT_DOUBLE_EQ(Mat2A[0][0] , 0);
    //ASSERT_DOUBLE_EQ(Mat2A[0][1] , 2);
    //ASSERT_DOUBLE_EQ(Mat2A[0][2] , 4);
    //ASSERT_DOUBLE_EQ(Mat2A[1][0] , 2);
    //ASSERT_DOUBLE_EQ(Mat2A[1][1] , 4);
    //ASSERT_DOUBLE_EQ(Mat2A[1][2] , 6);
    //ASSERT_DOUBLE_EQ(Mat2A[2][0] , 4);
    //ASSERT_DOUBLE_EQ(Mat2A[2][1] , 6);
    //ASSERT_DOUBLE_EQ(Mat2A[2][2] , 8);

    #endif

    }

//==================================================================================
// Increment test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongIncrement)
    {    
    const A3By3int32_tMatrix Mat2Const(Mat2);
    
    Mat2 += Mat2Const;

    ASSERT_NEAR(0.0, Mat2[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0, Mat2[0][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat2[0][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat2[1][0]);
    ASSERT_DOUBLE_EQ(4.0, Mat2[1][1]);
    ASSERT_DOUBLE_EQ(6.0, Mat2[1][2]);
    ASSERT_DOUBLE_EQ(4.0, Mat2[2][0]);
    ASSERT_DOUBLE_EQ(6.0, Mat2[2][1]);
    ASSERT_DOUBLE_EQ(8.0, Mat2[2][2]);

    }

//==================================================================================
// Decrement test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongDecrement)
    {
    
    const A3By3int32_tMatrix Mat2Const(Mat2);
    Mat2 -= Mat2Const;

    ASSERT_NEAR(0.0, Mat2[0][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[0][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[0][2] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[1][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[1][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[1][2] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[2][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[2][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2[2][2] , MYEPSILON);

    }

//==================================================================================
// Scale tests
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongScale)
    {
    
    Mat2 *= 2;
    ASSERT_NEAR(0.0, Mat2[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0, Mat2[0][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat2[0][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat2[1][0]);
    ASSERT_DOUBLE_EQ(4.0, Mat2[1][1]);
    ASSERT_DOUBLE_EQ(6.0, Mat2[1][2]);
    ASSERT_DOUBLE_EQ(4.0, Mat2[2][0]);
    ASSERT_DOUBLE_EQ(6.0, Mat2[2][1]);
    ASSERT_DOUBLE_EQ(8.0, Mat2[2][2]);

    Mat2 /= -2;
    ASSERT_NEAR(0.0, Mat2[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.0, Mat2[0][1]);
    ASSERT_DOUBLE_EQ(-2.0, Mat2[0][2]);
    ASSERT_DOUBLE_EQ(-1.0, Mat2[1][0]);
    ASSERT_DOUBLE_EQ(-2.0, Mat2[1][1]);
    ASSERT_DOUBLE_EQ(-3.0, Mat2[1][2]);
    ASSERT_DOUBLE_EQ(-2.0, Mat2[2][0]);
    ASSERT_DOUBLE_EQ(-3.0, Mat2[2][1]);
    ASSERT_DOUBLE_EQ(-4.0, Mat2[2][2]);

    }

//==================================================================================
// Negation test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongNegation)
    {
    
    const A3By3int32_tMatrix Mat2Const(Mat2);
    Mat2 = -Mat2Const;

    ASSERT_NEAR(0.0, Mat2[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.0, Mat2[0][1]);
    ASSERT_DOUBLE_EQ(-2.0, Mat2[0][2]);
    ASSERT_DOUBLE_EQ(-1.0, Mat2[1][0]);
    ASSERT_DOUBLE_EQ(-2.0, Mat2[1][1]);
    ASSERT_DOUBLE_EQ(-3.0, Mat2[1][2]);
    ASSERT_DOUBLE_EQ(-2.0, Mat2[2][0]);
    ASSERT_DOUBLE_EQ(-3.0, Mat2[2][1]);
    ASSERT_DOUBLE_EQ(-4.0, Mat2[2][2]);

    }

//==================================================================================
// Addition test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongAddition)
    {
    
    A3By3int32_tMatrix Mat2A;
    Mat2A = Mat2 + Mat2;

    ASSERT_NEAR(0.0, Mat2[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0, Mat2A[0][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat2A[0][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat2A[1][0]);
    ASSERT_DOUBLE_EQ(4.0, Mat2A[1][1]);
    ASSERT_DOUBLE_EQ(6.0, Mat2A[1][2]);
    ASSERT_DOUBLE_EQ(4.0, Mat2A[2][0]);
    ASSERT_DOUBLE_EQ(6.0, Mat2A[2][1]);
    ASSERT_DOUBLE_EQ(8.0, Mat2A[2][2]);

    }

//==================================================================================
// Substraction test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongSubstraction)
    {
    
    Mat2A = Mat2 - Mat2;

    ASSERT_NEAR(0.0, Mat2A[0][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2A[0][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2A[0][2] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2A[1][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2A[1][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2A[1][2] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2A[2][0] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2A[2][1] , MYEPSILON);
    ASSERT_NEAR(0.0, Mat2A[2][2] , MYEPSILON);

    }

//==================================================================================
// Transpose test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongTranspose)
    {
    
    Mat2[2][0] = 35;

    A3By3int32_tMatrix Mat4(Mat2.CalculateTranspose());

    ASSERT_NEAR(0.0, Mat4[0][0] , MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, Mat4[0][1]);
    ASSERT_DOUBLE_EQ(35.0, Mat4[0][2]);
    ASSERT_DOUBLE_EQ(1.0, Mat4[1][0]);
    ASSERT_DOUBLE_EQ(2.0, Mat4[1][1]);
    ASSERT_DOUBLE_EQ(3.0, Mat4[1][2]);
    ASSERT_DOUBLE_EQ(2.0, Mat4[2][0]);
    ASSERT_DOUBLE_EQ(3.0, Mat4[2][1]);
    ASSERT_DOUBLE_EQ(4.0, Mat4[2][2]);

    }

//==================================================================================
// Compare operation test
//==================================================================================
TEST_F (HFCMatrixTester, TestMatrixSignedLongCompare)
    {

    const A3By3int32_tMatrix Mat2Const(Mat2);
    Mat2 = Mat2Const;
    Mat2A = Mat2Const;

    ASSERT_TRUE(Mat2 == Mat2A);
    ASSERT_FALSE(Mat2 != Mat2A);
    ASSERT_TRUE(Mat2.IsEqualTo(Mat2A));
    ASSERT_TRUE(Mat2.IsEqualTo(Mat2A, 1E-6));

    Mat2[0][0] = 23;
    ASSERT_TRUE(Mat2 != Mat2A);
    ASSERT_FALSE(Mat2 == Mat2A);
    ASSERT_FALSE(Mat2.IsEqualTo(Mat2A));
    ASSERT_FALSE(Mat2.IsEqualTo(Mat2A, 1E-6));

    Mat2A[0][0] = Mat2[0][0] + 1;
    ASSERT_TRUE(Mat2 != Mat2A);
    ASSERT_FALSE(Mat2 == Mat2A);
    ASSERT_FALSE(Mat2.IsEqualTo(Mat2A, 0.1));
    ASSERT_TRUE(Mat2.IsEqualTo(Mat2A, 1));

    }
