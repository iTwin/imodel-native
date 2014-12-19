//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/tst/HGF2DCoordTester.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Main program testing Shape
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HGF2DCoord.h>

template<class DataType> int TestCoordData(DataType pi_Example)
    {
    HGF2DCoord<DataType> C1(0.0, 0.0);
    HGF2DCoord<DataType> C2(0.0, 10.0);
    HGF2DCoord<DataType> C3(10.0, 10.0);
    HGF2DCoord<DataType> C4(10.0, 0.0);
    HGF2DCoord<DataType> C5(0.0, 0.0);

    // Test each direct function

    // Constructor
    HGF2DCoord<DataType> CTemp(C3);
    HASSERT(C3 == CTemp);

    // Operator =
    CTemp = C2;
    HASSERT(C2 == CTemp);

    // Operator ==
    HASSERT(C2 == CTemp);

    // Operator!=
    HASSERT(C2 != C3);

    // GetX()
    char TempData;
    TempData = C2.GetX();
    HASSERT(TempData == C2.m_XValue);

    // GetY()
    TempData = C2.GetY();
    HASSERT(TempData == C2.m_YValue);

    // SetX()
    C2.SetX(C3.GetX());
    HASSERT(C2.GetX() == C3.GetX());

    // SetY()
    C2.SetY(C3.GetY());
    HASSERT(C2.GetY() == C3.GetY());

    // Operator[] non-const
    C4[0] = C2[0];
    HASSERT(C4.GetX() == C2.GetX());
    C4[1] = C2[1];
    HASSERT(C4.GetY() == C2.GetY());

    C4[HGF2DCoord<char>::X] = C3[HGF2DCoord<DataType>::X];
    HASSERT(C4.GetX() == C3.GetX());

    C4[HGF2DCoord<char>::Y] = C3[HGF2DCoord<DataType>::Y];
    HASSERT(C4.GetY() == C3.GetY());

    // Operator[] const
    const HGF2DCoord<DataType> TempCoord(C1);
    C2.SetX(TempCoord[0]);
    C2.SetY(TempCoord[1]);
    HASSERT(C2 == TempCoord);
    C4.SetX(TempCoord[HGF2DCoord<DataType>::X]);
    C4.SetY(TempCoord[HGF2DCoord<DataType>::Y]);
    HASSERT(C4 == TempCoord);

    return(0);

    }

int CoordTest()
    {
    // Unitary coord test
    char A='\0';
    TestCoordData(A);
    signed char B='\0';
    TestCoordData(B);
    unsigned char C='\0';
    TestCoordData(C);

    int32_t D=1;
    TestCoordData(D);
    int32_t E=1;
    TestCoordData(E);
    uint32_t F=1;
    TestCoordData(F);

    short G=1;
    TestCoordData(G);
    unsigned short H=1;
    TestCoordData(H);
    short I=1;
    TestCoordData(I);

    int32_t J=0;
    TestCoordData(J);
    uint32_t K=0;
    TestCoordData(K);
    int32_t L=0;
    TestCoordData(L);

#if (0)
    int64_t M;
    TestCoordData(M);
    int64_t N;
    TestCoordData(N);
    uint64_t O;
    TestCoordData(O);
#endif

    float P=0.0;
    TestCoordData(P);

    double Q=0.0;
    TestCoordData(Q);

    double R=0.0;
    TestCoordData(R);



    // Interation test

    // Construction of a char coord from others
    HGF2DCoord<char> A0;
    HGF2DCoord<signed char> B0;
    HGF2DCoord<unsigned char> C0;
    HGF2DCoord<int32_t> D0;
    HGF2DCoord<int32_t> E0;
    HGF2DCoord<uint32_t> F0;
    HGF2DCoord<short> G0;
    HGF2DCoord<unsigned short> H0;
    HGF2DCoord<short> I0;
    HGF2DCoord<int32_t> J0;
    HGF2DCoord<uint32_t> K0;
    HGF2DCoord<int32_t> L0;
#if (0)
    HGF2DCoord<int64_t> M0;
    HGF2DCoord<int64_t> N0;
    HGF2DCoord<uint64_t> O0;
#endif
    HGF2DCoord<float> P0;
    HGF2DCoord<double> Q0;
    HGF2DCoord<double> R0;

    HGF2DCoord<char> A1(A0);
    HGF2DCoord<char> A2(B0);
    HGF2DCoord<char> A3(C0);
    HGF2DCoord<char> A4(D0);
    HGF2DCoord<char> A5(E0);
    HGF2DCoord<char> A6(F0);
    HGF2DCoord<char> A7(G0);
    HGF2DCoord<char> A8(H0);
    HGF2DCoord<char> A9(I0);
    HGF2DCoord<char> A10(J0);
    HGF2DCoord<char> A11(K0);
    HGF2DCoord<char> A12(L0);
#if (0)
    HGF2DCoord<char> A13(M0);
    HGF2DCoord<char> A14(N0);
    HGF2DCoord<char> A15(O0);
#endif
    HGF2DCoord<char> A16(P0);
    HGF2DCoord<char> A17(Q0);
    HGF2DCoord<char> A18(R0);
//    HGF2DCoord<char> A19(S0);
//    HGF2DCoord<char> A20(T0);
//    HGF2DCoord<char> A21(U0);
//    HGF2DCoord<char> A22(V0);


    // Construction of a signed char coord from others
    HGF2DCoord<signed char> B1;
    HGF2DCoord<signed char> B2(B0);
    HGF2DCoord<signed char> B3(C0);
    HGF2DCoord<signed char> B4(D0);
    HGF2DCoord<signed char> B5(E0);
    HGF2DCoord<signed char> B6(F0);
    HGF2DCoord<signed char> B7(G0);
    HGF2DCoord<signed char> B8(H0);
    HGF2DCoord<signed char> B9(I0);
    HGF2DCoord<signed char> B10(J0);
    HGF2DCoord<signed char> B11(K0);
    HGF2DCoord<signed char> B12(L0);
#if (0)

    HGF2DCoord<signed char> B13(M0);
    HGF2DCoord<signed char> B14(N0);
    HGF2DCoord<signed char> B15(O0);
#endif
    HGF2DCoord<signed char> B16(P0);
    HGF2DCoord<signed char> B17(Q0);
    HGF2DCoord<signed char> B18(R0);
//    HGF2DCoord<signed char> B19(S0);
//    HGF2DCoord<signed char> B20(T0);
//    HGF2DCoord<signed char> B21(U0);
//    HGF2DCoord<signed char> B22(V0);

    // Construction of a unsigned char coord from others
    HGF2DCoord<unsigned char> C1;
    HGF2DCoord<unsigned char> C2(B0);
    HGF2DCoord<unsigned char> C3(C0);
    HGF2DCoord<unsigned char> C4(D0);
    HGF2DCoord<unsigned char> C5(E0);
    HGF2DCoord<unsigned char> C6(F0);
    HGF2DCoord<unsigned char> C7(G0);
    HGF2DCoord<unsigned char> C8(H0);
    HGF2DCoord<unsigned char> C9(I0);
    HGF2DCoord<unsigned char> C10(J0);
    HGF2DCoord<unsigned char> C11(K0);
    HGF2DCoord<unsigned char> C12(L0);
#if (0)
    HGF2DCoord<unsigned char> C13(M0);
    HGF2DCoord<unsigned char> C14(N0);
    HGF2DCoord<unsigned char> C15(O0);
#endif
    HGF2DCoord<unsigned char> C16(P0);
    HGF2DCoord<unsigned char> C17(Q0);
    HGF2DCoord<unsigned char> C18(R0);
//    HGF2DCoord<unsigned char> C19(S0);
//    HGF2DCoord<unsigned char> C20(T0);
//    HGF2DCoord<unsigned char> C21(U0);
//    HGF2DCoord<unsigned char> C22(V0);

    // Construction of a Int32 coord from others
    HGF2DCoord<int32_t> D1;
    HGF2DCoord<int32_t> D2(B0);
    HGF2DCoord<int32_t> D3(C0);
    HGF2DCoord<int32_t> D4(D0);
    HGF2DCoord<int32_t> D5(E0);
    HGF2DCoord<int32_t> D6(F0);
    HGF2DCoord<int32_t> D7(G0);
    HGF2DCoord<int32_t> D8(H0);
    HGF2DCoord<int32_t> D9(I0);
    HGF2DCoord<int32_t> D10(J0);
    HGF2DCoord<int32_t> D11(K0);
    HGF2DCoord<int32_t> D12(L0);
#if (0)
    HGF2DCoord<int32_t> D13(M0);
    HGF2DCoord<int32_t> D14(N0);
    HGF2DCoord<int32_t> D15(O0);
#endif
    HGF2DCoord<int32_t> D16(P0);
    HGF2DCoord<int32_t> D17(Q0);
    HGF2DCoord<int32_t> D18(R0);
//    HGF2DCoord<Int32> D19(S0);
//    HGF2DCoord<Int32> D20(T0);
//    HGF2DCoord<Int32> D21(U0);
//    HGF2DCoord<Int32> D22(V0);

    // Construction of a Int32 coord from others
    HGF2DCoord<int32_t> E1;
    HGF2DCoord<int32_t> E2(B0);
    HGF2DCoord<int32_t> E3(C0);
    HGF2DCoord<int32_t> E4(D0);
    HGF2DCoord<int32_t> E5(E0);
    HGF2DCoord<int32_t> E6(F0);
    HGF2DCoord<int32_t> E7(G0);
    HGF2DCoord<int32_t> E8(H0);
    HGF2DCoord<int32_t> E9(I0);
    HGF2DCoord<int32_t> E10(J0);
    HGF2DCoord<int32_t> E11(K0);
    HGF2DCoord<int32_t> E12(L0);
#if (0)
    HGF2DCoord<int32_t> E13(M0);
    HGF2DCoord<int32_t> E14(N0);
    HGF2DCoord<int32_t> E15(O0);
#endif
    HGF2DCoord<int32_t> E16(P0);
    HGF2DCoord<int32_t> E17(Q0);
    HGF2DCoord<int32_t> E18(R0);
//    HGF2DCoord<Int32> E19(S0);
//    HGF2DCoord<Int32> E20(T0);
//    HGF2DCoord<Int32> E21(U0);
//    HGF2DCoord<Int32> E22(V0);

    // Construction of a UInt32 coord from others
    HGF2DCoord<uint32_t> F1;
    HGF2DCoord<uint32_t> F2(B0);
    HGF2DCoord<uint32_t> F3(C0);
    HGF2DCoord<uint32_t> F4(D0);
    HGF2DCoord<uint32_t> F5(E0);
    HGF2DCoord<uint32_t> F6(F0);
    HGF2DCoord<uint32_t> F7(G0);
    HGF2DCoord<uint32_t> F8(H0);
    HGF2DCoord<uint32_t> F9(I0);
    HGF2DCoord<uint32_t> F10(J0);
    HGF2DCoord<uint32_t> F11(K0);
    HGF2DCoord<uint32_t> F12(L0);
#if (0)
    HGF2DCoord<uint32_t> F13(M0);
    HGF2DCoord<uint32_t> F14(N0);
    HGF2DCoord<uint32_t> F15(O0);
#endif
    HGF2DCoord<uint32_t> F16(P0);
    HGF2DCoord<uint32_t> F17(Q0);
    HGF2DCoord<uint32_t> F18(R0);
//    HGF2DCoord<UInt32> F19(S0);
//    HGF2DCoord<UInt32> F20(T0);
//    HGF2DCoord<UInt32> F21(U0);
//    HGF2DCoord<UInt32> F22(V0);


    // Construction of a Short coord from others
    HGF2DCoord<short> G1;
    HGF2DCoord<short> G2(B0);
    HGF2DCoord<short> G3(C0);
    HGF2DCoord<short> G4(D0);
    HGF2DCoord<short> G5(E0);
    HGF2DCoord<short> G6(F0);
    HGF2DCoord<short> G7(G0);
    HGF2DCoord<short> G8(H0);
    HGF2DCoord<short> G9(I0);
    HGF2DCoord<short> G10(J0);
    HGF2DCoord<short> G11(K0);
    HGF2DCoord<short> G12(L0);
#if (0)
    HGF2DCoord<short> G13(M0);
    HGF2DCoord<short> G14(N0);
    HGF2DCoord<short> G15(O0);
#endif
    HGF2DCoord<short> G16(P0);
    HGF2DCoord<short> G17(Q0);
    HGF2DCoord<short> G18(R0);
//    HGF2DCoord<Short> G19(S0);
//    HGF2DCoord<Short> G20(T0);
//    HGF2DCoord<Short> G21(U0);
//    HGF2DCoord<Short> G22(V0);

    // Construction of a UShort coord from others
    HGF2DCoord<unsigned short> H1;
    HGF2DCoord<unsigned short> H2(B0);
    HGF2DCoord<unsigned short> H3(C0);
    HGF2DCoord<unsigned short> H4(D0);
    HGF2DCoord<unsigned short> H5(E0);
    HGF2DCoord<unsigned short> H6(F0);
    HGF2DCoord<unsigned short> H7(G0);
    HGF2DCoord<unsigned short> H8(H0);
    HGF2DCoord<unsigned short> H9(I0);
    HGF2DCoord<unsigned short> H10(J0);
    HGF2DCoord<unsigned short> H11(K0);
    HGF2DCoord<unsigned short> H12(L0);
#if (0)
    HGF2DCoord<unsigned short> H13(M0);
    HGF2DCoord<unsigned short> H14(N0);
    HGF2DCoord<unsigned short> H15(O0);
#endif
    HGF2DCoord<unsigned short> H16(P0);
    HGF2DCoord<unsigned short> H17(Q0);
    HGF2DCoord<unsigned short> H18(R0);
//    HGF2DCoord<UShort> H19(S0);
//    HGF2DCoord<UShort> H20(T0);
//    HGF2DCoord<UShort> H21(U0);
//    HGF2DCoord<UShort> H22(V0);

    // Construction of a Short coord from others
    HGF2DCoord<short> I1;
    HGF2DCoord<short> I2(B0);
    HGF2DCoord<short> I3(C0);
    HGF2DCoord<short> I4(D0);
    HGF2DCoord<short> I5(E0);
    HGF2DCoord<short> I6(F0);
    HGF2DCoord<short> I7(G0);
    HGF2DCoord<short> I8(H0);
    HGF2DCoord<short> I9(I0);
    HGF2DCoord<short> I10(J0);
    HGF2DCoord<short> I11(K0);
    HGF2DCoord<short> I12(L0);
#if (0)
    HGF2DCoord<short> I13(M0);
    HGF2DCoord<short> I14(N0);
    HGF2DCoord<short> I15(O0);
#endif
    HGF2DCoord<short> I16(P0);
    HGF2DCoord<short> I17(Q0);
    HGF2DCoord<short> I18(R0);
//    HGF2DCoord<Short> I19(S0);
//    HGF2DCoord<Short> I20(T0);
//    HGF2DCoord<Short> I21(U0);
//    HGF2DCoord<Short> I22(V0);


    // Construction of a Int32 coord from others
    HGF2DCoord<int32_t> J1;
    HGF2DCoord<int32_t> J2(B0);
    HGF2DCoord<int32_t> J3(C0);
    HGF2DCoord<int32_t> J4(D0);
    HGF2DCoord<int32_t> J5(E0);
    HGF2DCoord<int32_t> J6(F0);
    HGF2DCoord<int32_t> J7(G0);
    HGF2DCoord<int32_t> J8(H0);
    HGF2DCoord<int32_t> J9(I0);
    HGF2DCoord<int32_t> J10(J0);
    HGF2DCoord<int32_t> J11(K0);
    HGF2DCoord<int32_t> J12(L0);
#if (0)
    HGF2DCoord<int32_t> J13(M0);
    HGF2DCoord<int32_t> J14(N0);
    HGF2DCoord<int32_t> J15(O0);
#endif
    HGF2DCoord<int32_t> J16(P0);
    HGF2DCoord<int32_t> J17(Q0);
    HGF2DCoord<int32_t> J18(R0);
//    HGF2DCoord<Int32> J19(S0);
//    HGF2DCoord<Int32> J20(T0);
//    HGF2DCoord<Int32> J21(U0);
//    HGF2DCoord<Int32> J22(V0);

    // Construction of a UInt32 coord from others
    HGF2DCoord<uint32_t> K1;
    HGF2DCoord<uint32_t> K2(B0);
    HGF2DCoord<uint32_t> K3(C0);
    HGF2DCoord<uint32_t> K4(D0);
    HGF2DCoord<uint32_t> K5(E0);
    HGF2DCoord<uint32_t> K6(F0);
    HGF2DCoord<uint32_t> K7(G0);
    HGF2DCoord<uint32_t> K8(H0);
    HGF2DCoord<uint32_t> K9(I0);
    HGF2DCoord<uint32_t> K10(J0);
    HGF2DCoord<uint32_t> K11(K0);
    HGF2DCoord<uint32_t> K12(L0);
#if (0)
    HGF2DCoord<uint32_t> K13(M0);
    HGF2DCoord<uint32_t> K14(N0);
    HGF2DCoord<uint32_t> K15(O0);
#endif
    HGF2DCoord<uint32_t> K16(P0);
    HGF2DCoord<uint32_t> K17(Q0);
    HGF2DCoord<uint32_t> K18(R0);
//    HGF2DCoord<UInt32> K19(S0);
//    HGF2DCoord<UInt32> K20(T0);
//    HGF2DCoord<UInt32> K21(U0);
//    HGF2DCoord<UInt32> K22(V0);


    // Construction of a Int32 coord from others
    HGF2DCoord<int32_t> L1;
    HGF2DCoord<int32_t> L2(B0);
    HGF2DCoord<int32_t> L3(C0);
    HGF2DCoord<int32_t> L4(D0);
    HGF2DCoord<int32_t> L5(E0);
    HGF2DCoord<int32_t> L6(F0);
    HGF2DCoord<int32_t> L7(G0);
    HGF2DCoord<int32_t> L8(H0);
    HGF2DCoord<int32_t> L9(I0);
    HGF2DCoord<int32_t> L10(J0);
    HGF2DCoord<int32_t> L11(K0);
    HGF2DCoord<int32_t> L12(L0);
#if (0)
    HGF2DCoord<int32_t> L13(M0);
    HGF2DCoord<int32_t> L14(N0);
    HGF2DCoord<int32_t> L15(O0);
#endif
    HGF2DCoord<int32_t> L16(P0);
    HGF2DCoord<int32_t> L17(Q0);
    HGF2DCoord<int32_t> L18(R0);
//    HGF2DCoord<Int32> L19(S0);
//    HGF2DCoord<Int32> L20(T0);
//    HGF2DCoord<Int32> L21(U0);
//    HGF2DCoord<Int32> L22(V0);


#if (0)
    // Construction of a Int64 coord from others
    HGF2DCoord<int64_t> M1;
    HGF2DCoord<int64_t> M2(B0);
    HGF2DCoord<int64_t> M3(C0);
    HGF2DCoord<int64_t> M4(D0);
    HGF2DCoord<int64_t> M5(E0);
    HGF2DCoord<int64_t> M6(F0);
    HGF2DCoord<int64_t> M7(G0);
    HGF2DCoord<int64_t> M8(H0);
    HGF2DCoord<int64_t> M9(I0);
    HGF2DCoord<int64_t> M10(J0);
    HGF2DCoord<int64_t> M11(K0);
    HGF2DCoord<int64_t> M12(L0);
#if (0)
    HGF2DCoord<int64_t> M13(M0);
    HGF2DCoord<int64_t> M14(N0);
    HGF2DCoord<int64_t> M15(O0);
#endif
    HGF2DCoord<int64_t> M16(P0);
    HGF2DCoord<int64_t> M17(Q0);
    HGF2DCoord<int64_t> M18(R0);
//    HGF2DCoord<Int64> M19(S0);
//    HGF2DCoord<Int64> M20(T0);
//    HGF2DCoord<Int64> M21(U0);
//    HGF2DCoord<Int64> M22(V0);

    // Construction of a Int64 coord from others
    HGF2DCoord<int64_t> N1;
    HGF2DCoord<int64_t> N2(B0);
    HGF2DCoord<int64_t> N3(C0);
    HGF2DCoord<int64_t> N4(D0);
    HGF2DCoord<int64_t> N5(E0);
    HGF2DCoord<int64_t> N6(F0);
    HGF2DCoord<int64_t> N7(G0);
    HGF2DCoord<int64_t> N8(H0);
    HGF2DCoord<int64_t> N9(I0);
    HGF2DCoord<int64_t> N10(J0);
    HGF2DCoord<int64_t> N11(K0);
    HGF2DCoord<int64_t> N12(L0);
#if (0)
    HGF2DCoord<int64_t> N13(M0);
    HGF2DCoord<int64_t> N14(N0);
    HGF2DCoord<int64_t> N15(O0);
#endif
    HGF2DCoord<int64_t> N16(P0);
    HGF2DCoord<int64_t> N17(Q0);
    HGF2DCoord<int64_t> N18(R0);
//    HGF2DCoord<Int64> N19(S0);
//    HGF2DCoord<Int64> N20(T0);
//    HGF2DCoord<Int64> N21(U0);
//    HGF2DCoord<Int64> N22(V0);

    // Construction of a UInt64 coord from others
    HGF2DCoord<uint64_t> O1;
    HGF2DCoord<uint64_t> O2(B0);
    HGF2DCoord<uint64_t> O3(C0);
    HGF2DCoord<uint64_t> O4(D0);
    HGF2DCoord<uint64_t> O5(E0);
    HGF2DCoord<uint64_t> O6(F0);
    HGF2DCoord<uint64_t> O7(G0);
    HGF2DCoord<uint64_t> O8(H0);
    HGF2DCoord<uint64_t> O9(I0);
    HGF2DCoord<uint64_t> O10(J0);
    HGF2DCoord<uint64_t> O11(K0);
    HGF2DCoord<uint64_t> O12(L0);
#if (0)
    HGF2DCoord<uint64_t> O13(M0);
    HGF2DCoord<uint64_t> O14(N0);
    HGF2DCoord<uint64_t> O15(O0);
#endif
    HGF2DCoord<uint64_t> O16(P0);
    HGF2DCoord<uint64_t> O17(Q0);
    HGF2DCoord<uint64_t> O18(R0);
//    HGF2DCoord<UInt64> O19(S0);
//    HGF2DCoord<UInt64> O20(T0);
//    HGF2DCoord<UInt64> O21(U0);
//    HGF2DCoord<UInt64> O22(V0);
#endif

    // Construction of a float coord from others
    HGF2DCoord<float> P1;
    HGF2DCoord<float> P2(B0);
    HGF2DCoord<float> P3(C0);
    HGF2DCoord<float> P4(D0);
    HGF2DCoord<float> P5(E0);
    HGF2DCoord<float> P6(F0);
    HGF2DCoord<float> P7(G0);
    HGF2DCoord<float> P8(H0);
    HGF2DCoord<float> P9(I0);
    HGF2DCoord<float> P10(J0);
    HGF2DCoord<float> P11(K0);
    HGF2DCoord<float> P12(L0);
#if (0)
    HGF2DCoord<float> P13(M0);
    HGF2DCoord<float> P14(N0);
    HGF2DCoord<float> P15(O0);
#endif
    HGF2DCoord<float> P16(P0);
    HGF2DCoord<float> P17(Q0);
    HGF2DCoord<float> P18(R0);
//    HGF2DCoord<float> P19(S0);
//    HGF2DCoord<float> P20(T0);
//    HGF2DCoord<float> P21(U0);
//    HGF2DCoord<float> P22(V0);

    // Construction of a double coord from others
    HGF2DCoord<double> Q1;
    HGF2DCoord<double> Q2(B0);
    HGF2DCoord<double> Q3(C0);
    HGF2DCoord<double> Q4(D0);
    HGF2DCoord<double> Q5(E0);
    HGF2DCoord<double> Q6(F0);
    HGF2DCoord<double> Q7(G0);
    HGF2DCoord<double> Q8(H0);
    HGF2DCoord<double> Q9(I0);
    HGF2DCoord<double> Q10(J0);
    HGF2DCoord<double> Q11(K0);
    HGF2DCoord<double> Q12(L0);
#if (0)
    HGF2DCoord<double> Q13(M0);
    HGF2DCoord<double> Q14(N0);
    HGF2DCoord<double> Q15(O0);
#endif
    HGF2DCoord<double> Q16(P0);
    HGF2DCoord<double> Q17(Q0);
    HGF2DCoord<double> Q18(R0);
//    HGF2DCoord<double> Q19(S0);
//    HGF2DCoord<double> Q20(T0);
//    HGF2DCoord<double> Q21(U0);
//    HGF2DCoord<double> Q22(V0);


    // Construction of a double coord from others
    HGF2DCoord<double> R1;
    HGF2DCoord<double> R2(B0);
    HGF2DCoord<double> R3(C0);
    HGF2DCoord<double> R4(D0);
    HGF2DCoord<double> R5(E0);
    HGF2DCoord<double> R6(F0);
    HGF2DCoord<double> R7(G0);
    HGF2DCoord<double> R8(H0);
    HGF2DCoord<double> R9(I0);
    HGF2DCoord<double> R10(J0);
    HGF2DCoord<double> R11(K0);
    HGF2DCoord<double> R12(L0);
#if (0)
    HGF2DCoord<double> R13(M0);
    HGF2DCoord<double> R14(N0);
    HGF2DCoord<double> R15(O0);
#endif
    HGF2DCoord<double> R16(P0);
    HGF2DCoord<double> R17(Q0);
    HGF2DCoord<double> R18(R0);
//    HGF2DCoord<double> R19(S0);
//    HGF2DCoord<double> R20(T0);
//    HGF2DCoord<double> R21(U0);
//    HGF2DCoord<double> R22(V0);



    // Implicit cast operation

    // Compare operation (operator ==)

    // char
    HASSERT(A0 == B0);
    HASSERT(A0 == C0);
    HASSERT(A0 == D0);
    HASSERT(A0 == E0);
    HASSERT(A0 == F0);
    HASSERT(A0 == G0);
    HASSERT(A0 == H0);
    HASSERT(A0 == I0);
    HASSERT(A0 == J0);
    HASSERT(A0 == K0);
    HASSERT(A0 == L0);
#if (0)
    HASSERT(A0 == M0);
    HASSERT(A0 == N0);
    HASSERT(A0 == O0);
#endif
    HASSERT(A0 == P0);
    HASSERT(A0 == Q0);
    HASSERT(A0 == R0);
//    HASSERT(A0 == S0);
//    HASSERT(A0 == T0);
//    HASSERT(A0 == U0);
//    HASSERT(A0 == V0);

    // signed char
    HASSERT(B0 == A0);
    HASSERT(B0 == C0);
    HASSERT(B0 == D0);
    HASSERT(B0 == E0);
    HASSERT(B0 == F0);
    HASSERT(B0 == G0);
    HASSERT(B0 == H0);
    HASSERT(B0 == I0);
    HASSERT(B0 == J0);
    HASSERT(B0 == K0);
    HASSERT(B0 == L0);
#if (0)
    HASSERT(B0 == M0);
    HASSERT(B0 == N0);
    HASSERT(B0 == O0);
#endif
    HASSERT(B0 == P0);
    HASSERT(B0 == Q0);
    HASSERT(B0 == R0);
//    HASSERT(B0 == S0);
//    HASSERT(B0 == T0);
//    HASSERT(B0 == U0);
//    HASSERT(B0 == V0);

    // unsigned char
    HASSERT(C0 == A0);
    HASSERT(C0 == B0);
    HASSERT(C0 == D0);
    HASSERT(C0 == E0);
    HASSERT(C0 == F0);
    HASSERT(C0 == G0);
    HASSERT(C0 == H0);
    HASSERT(C0 == I0);
    HASSERT(C0 == J0);
    HASSERT(C0 == K0);
    HASSERT(C0 == L0);
#if (0)
    HASSERT(C0 == M0);
    HASSERT(C0 == N0);
    HASSERT(C0 == O0);
#endif
    HASSERT(C0 == P0);
    HASSERT(C0 == Q0);
    HASSERT(C0 == R0);
//    HASSERT(C0 == S0);
//    HASSERT(C0 == T0);
//    HASSERT(C0 == U0);
//    HASSERT(C0 == V0);

    // Int32
    HASSERT(D0 == A0);
    HASSERT(D0 == B0);
    HASSERT(D0 == C0);
    HASSERT(D0 == E0);
    HASSERT(D0 == F0);
    HASSERT(D0 == G0);
    HASSERT(D0 == H0);
    HASSERT(D0 == I0);
    HASSERT(D0 == J0);
    HASSERT(D0 == K0);
    HASSERT(D0 == L0);
#if (0)
    HASSERT(D0 == M0);
    HASSERT(D0 == N0);
    HASSERT(D0 == O0);
#endif
    HASSERT(D0 == P0);
    HASSERT(D0 == Q0);
    HASSERT(D0 == R0);
//    HASSERT(D0 == S0);
//    HASSERT(D0 == T0);
//    HASSERT(D0 == U0);
//    HASSERT(D0 == V0);

    // Int32
    HASSERT(E0 == A0);
    HASSERT(E0 == B0);
    HASSERT(E0 == C0);
    HASSERT(E0 == D0);
    HASSERT(E0 == F0);
    HASSERT(E0 == G0);
    HASSERT(E0 == H0);
    HASSERT(E0 == I0);
    HASSERT(E0 == J0);
    HASSERT(E0 == K0);
    HASSERT(E0 == L0);
#if (0)
    HASSERT(E0 == M0);
    HASSERT(E0 == N0);
    HASSERT(E0 == O0);
#endif
    HASSERT(E0 == P0);
    HASSERT(E0 == Q0);
    HASSERT(E0 == R0);
//    HASSERT(E0 == S0);
//    HASSERT(E0 == T0);
//    HASSERT(E0 == U0);
//    HASSERT(E0 == V0);

    // UInt32
    HASSERT(F0 == A0);
    HASSERT(F0 == B0);
    HASSERT(F0 == C0);
    HASSERT(F0 == D0);
    HASSERT(F0 == E0);
    HASSERT(F0 == G0);
    HASSERT(F0 == H0);
    HASSERT(F0 == I0);
    HASSERT(F0 == J0);
    HASSERT(F0 == K0);
    HASSERT(F0 == L0);
#if (0)
    HASSERT(F0 == M0);
    HASSERT(F0 == N0);
    HASSERT(F0 == O0);
#endif
    HASSERT(F0 == P0);
    HASSERT(F0 == Q0);
    HASSERT(F0 == R0);
//    HASSERT(F0 == S0);
//    HASSERT(F0 == T0);
//    HASSERT(F0 == U0);
//    HASSERT(F0 == V0);

    // Short
    HASSERT(G0 == A0);
    HASSERT(G0 == B0);
    HASSERT(G0 == C0);
    HASSERT(G0 == D0);
    HASSERT(G0 == E0);
    HASSERT(G0 == F0);
    HASSERT(G0 == H0);
    HASSERT(G0 == I0);
    HASSERT(G0 == J0);
    HASSERT(G0 == K0);
    HASSERT(G0 == L0);
#if (0)
    HASSERT(G0 == M0);
    HASSERT(G0 == N0);
    HASSERT(G0 == O0);
#endif
    HASSERT(G0 == P0);
    HASSERT(G0 == Q0);
    HASSERT(G0 == R0);
//    HASSERT(G0 == S0);
//    HASSERT(G0 == T0);
//    HASSERT(G0 == U0);
//    HASSERT(G0 == V0);

    // UShort
    HASSERT(H0 == A0);
    HASSERT(H0 == B0);
    HASSERT(H0 == C0);
    HASSERT(H0 == D0);
    HASSERT(H0 == E0);
    HASSERT(H0 == F0);
    HASSERT(H0 == G0);
    HASSERT(H0 == I0);
    HASSERT(H0 == J0);
    HASSERT(H0 == K0);
    HASSERT(H0 == L0);
#if (0)
    HASSERT(H0 == M0);
    HASSERT(H0 == N0);
    HASSERT(H0 == O0);
#endif
    HASSERT(H0 == P0);
    HASSERT(H0 == Q0);
    HASSERT(H0 == R0);
//    HASSERT(H0 == S0);
//    HASSERT(H0 == T0);
//    HASSERT(H0 == U0);
//    HASSERT(H0 == V0);

    // Short
    HASSERT(I0 == A0);
    HASSERT(I0 == B0);
    HASSERT(I0 == C0);
    HASSERT(I0 == D0);
    HASSERT(I0 == E0);
    HASSERT(I0 == F0);
    HASSERT(I0 == G0);
    HASSERT(I0 == H0);
    HASSERT(I0 == J0);
    HASSERT(I0 == K0);
    HASSERT(I0 == L0);
#if (0)
    HASSERT(I0 == M0);
    HASSERT(I0 == N0);
    HASSERT(I0 == O0);
#endif
    HASSERT(I0 == P0);
    HASSERT(I0 == Q0);
    HASSERT(I0 == R0);
//    HASSERT(I0 == S0);
//    HASSERT(I0 == T0);
//    HASSERT(I0 == U0);
//    HASSERT(I0 == V0);

    // Int32
    HASSERT(J0 == A0);
    HASSERT(J0 == B0);
    HASSERT(J0 == C0);
    HASSERT(J0 == D0);
    HASSERT(J0 == E0);
    HASSERT(J0 == F0);
    HASSERT(J0 == G0);
    HASSERT(J0 == H0);
    HASSERT(J0 == I0);
    HASSERT(J0 == K0);
    HASSERT(J0 == L0);
#if (0)
    HASSERT(J0 == M0);
    HASSERT(J0 == N0);
    HASSERT(J0 == O0);
#endif
    HASSERT(J0 == P0);
    HASSERT(J0 == Q0);
    HASSERT(J0 == R0);
//    HASSERT(J0 == S0);
//    HASSERT(J0 == T0);
//    HASSERT(J0 == U0);
//    HASSERT(J0 == V0);

    // UInt32
    HASSERT(K0 == A0);
    HASSERT(K0 == B0);
    HASSERT(K0 == C0);
    HASSERT(K0 == D0);
    HASSERT(K0 == E0);
    HASSERT(K0 == F0);
    HASSERT(K0 == G0);
    HASSERT(K0 == H0);
    HASSERT(K0 == I0);
    HASSERT(K0 == J0);
    HASSERT(K0 == L0);
#if (0)
    HASSERT(K0 == M0);
    HASSERT(K0 == N0);
    HASSERT(K0 == O0);
#endif
    HASSERT(K0 == P0);
    HASSERT(K0 == Q0);
    HASSERT(K0 == R0);
//    HASSERT(K0 == S0);
//    HASSERT(K0 == T0);
//    HASSERT(K0 == U0);
//    HASSERT(K0 == V0);

    // Int32
    HASSERT(L0 == A0);
    HASSERT(L0 == B0);
    HASSERT(L0 == C0);
    HASSERT(L0 == D0);
    HASSERT(L0 == E0);
    HASSERT(L0 == F0);
    HASSERT(L0 == G0);
    HASSERT(L0 == H0);
    HASSERT(L0 == I0);
    HASSERT(L0 == J0);
    HASSERT(L0 == K0);
#if (0)
    HASSERT(L0 == M0);
    HASSERT(L0 == N0);
    HASSERT(L0 == O0);
#endif
    HASSERT(L0 == P0);
    HASSERT(L0 == Q0);
    HASSERT(L0 == R0);
//    HASSERT(L0 == S0);
//    HASSERT(L0 == T0);
//    HASSERT(L0 == U0);
//    HASSERT(L0 == V0);

#if (0)
    // Int64
    HASSERT(M0 == A0);
    HASSERT(M0 == B0);
    HASSERT(M0 == C0);
    HASSERT(M0 == D0);
    HASSERT(M0 == E0);
    HASSERT(M0 == F0);
    HASSERT(M0 == G0);
    HASSERT(M0 == H0);
    HASSERT(M0 == I0);
    HASSERT(M0 == J0);
    HASSERT(M0 == K0);
    HASSERT(M0 == L0);
    HASSERT(M0 == N0);
    HASSERT(M0 == O0);
    HASSERT(M0 == P0);
    HASSERT(M0 == Q0);
    HASSERT(M0 == R0);
//    HASSERT(M0 == S0);
//    HASSERT(M0 == T0);
//    HASSERT(M0 == U0);
//    HASSERT(M0 == V0);

    // Int64
    HASSERT(N0 == A0);
    HASSERT(N0 == B0);
    HASSERT(N0 == C0);
    HASSERT(N0 == D0);
    HASSERT(N0 == E0);
    HASSERT(N0 == F0);
    HASSERT(N0 == G0);
    HASSERT(N0 == H0);
    HASSERT(N0 == I0);
    HASSERT(N0 == J0);
    HASSERT(N0 == K0);
    HASSERT(N0 == L0);
    HASSERT(N0 == M0);
    HASSERT(N0 == O0);
    HASSERT(N0 == P0);
    HASSERT(N0 == Q0);
    HASSERT(N0 == R0);
//    HASSERT(N0 == S0);
//    HASSERT(N0 == T0);
//    HASSERT(N0 == U0);
//    HASSERT(N0 == V0);

    // UInt64
    HASSERT(O0 == A0);
    HASSERT(O0 == B0);
    HASSERT(O0 == C0);
    HASSERT(O0 == D0);
    HASSERT(O0 == E0);
    HASSERT(O0 == F0);
    HASSERT(O0 == G0);
    HASSERT(O0 == H0);
    HASSERT(O0 == I0);
    HASSERT(O0 == J0);
    HASSERT(O0 == K0);
    HASSERT(O0 == L0);
    HASSERT(O0 == M0);
    HASSERT(O0 == N0);
    HASSERT(O0 == P0);
    HASSERT(O0 == Q0);
    HASSERT(O0 == R0);
//    HASSERT(O0 == S0);
//    HASSERT(O0 == T0);
//    HASSERT(O0 == U0);
//    HASSERT(O0 == V0);
#endif

    // float
    HASSERT(P0 == A0);
    HASSERT(P0 == B0);
    HASSERT(P0 == C0);
    HASSERT(P0 == D0);
    HASSERT(P0 == E0);
    HASSERT(P0 == F0);
    HASSERT(P0 == G0);
    HASSERT(P0 == H0);
    HASSERT(P0 == I0);
    HASSERT(P0 == J0);
    HASSERT(P0 == K0);
    HASSERT(P0 == L0);
#if (0)
    HASSERT(P0 == M0);
    HASSERT(P0 == N0);
    HASSERT(P0 == O0);
#endif
    HASSERT(P0 == Q0);
    HASSERT(P0 == R0);
//    HASSERT(P0 == S0);
//    HASSERT(P0 == T0);
//    HASSERT(P0 == U0);
//    HASSERT(P0 == V0);

    // double
    HASSERT(Q0 == A0);
    HASSERT(Q0 == B0);
    HASSERT(Q0 == C0);
    HASSERT(Q0 == D0);
    HASSERT(Q0 == E0);
    HASSERT(Q0 == F0);
    HASSERT(Q0 == G0);
    HASSERT(Q0 == H0);
    HASSERT(Q0 == I0);
    HASSERT(Q0 == J0);
    HASSERT(Q0 == K0);
    HASSERT(Q0 == L0);
#if (0)
    HASSERT(Q0 == M0);
    HASSERT(Q0 == N0);
    HASSERT(Q0 == O0);
#endif
    HASSERT(Q0 == P0);
    HASSERT(Q0 == R0);
//    HASSERT(Q0 == S0);
//    HASSERT(Q0 == T0);
//    HASSERT(Q0 == U0);
//    HASSERT(Q0 == V0);

    // double
    HASSERT(R0 == A0);
    HASSERT(R0 == B0);
    HASSERT(R0 == C0);
    HASSERT(R0 == D0);
    HASSERT(R0 == E0);
    HASSERT(R0 == F0);
    HASSERT(R0 == G0);
    HASSERT(R0 == H0);
    HASSERT(R0 == I0);
    HASSERT(R0 == J0);
    HASSERT(R0 == K0);
    HASSERT(R0 == L0);
#if (0)
    HASSERT(R0 == M0);
    HASSERT(R0 == N0);
    HASSERT(R0 == O0);
#endif
    HASSERT(R0 == P0);
    HASSERT(R0 == Q0);
//    HASSERT(R0 == S0);
//    HASSERT(R0 == T0);
//    HASSERT(R0 == U0);
//    HASSERT(R0 == V0);

//    HASSERT(S0 == A0);
//    HASSERT(S0 == B0);
//    HASSERT(S0 == C0);
//    HASSERT(S0 == D0);
//    HASSERT(S0 == E0);
//    HASSERT(S0 == F0);
//    HASSERT(S0 == G0);
//    HASSERT(S0 == H0);
//    HASSERT(S0 == I0);
//    HASSERT(S0 == J0);
//    HASSERT(S0 == K0);
//    HASSERT(S0 == L0);
//    HASSERT(S0 == M0);
//    HASSERT(S0 == N0);
//    HASSERT(S0 == O0);
//    HASSERT(S0 == P0);
//    HASSERT(S0 == Q0);
//    HASSERT(S0 == R0);
//    HASSERT(S0 == T0);
//    HASSERT(S0 == U0);
//    HASSERT(S0 == V0);



    //
#if (0)
    HGF2DCoord<uint32_t> CHU1(0, 0);
    HGF2DCoord<uint32_t> CHU2(0, 10);
    HGF2DCoord<uint32_t> CHU3(10, 10);
    HGF2DCoord<uint32_t> CHU4(10, 0);
    HGF2DCoord<uint32_t> CHU5(0, 0);

    HGF2DCoord<int32_t> CHS1(0, 0);
    HGF2DCoord<int32_t> CHS2(0, 10);
    HGF2DCoord<int32_t> CHS3(10, 10);
    HGF2DCoord<int32_t> CHS4(10, 0);
    HGF2DCoord<int32_t> CHS5(0, 0);



    HGF2DCoord<char> CHC1(0.0, 0.0);
    HGF2DCoord<char> CHC2(0.0, 10.0);
    HGF2DCoord<char> CHC3(10.0, 10.0);
    HGF2DCoord<char> CHC4(10.0, 0.0);
    HGF2DCoord<char> CHC5(0.0, 0.0);

    HGF2DCoord<signed char> CHC1(0.0, 0.0);
    HGF2DCoord<signed char> CHC2(0.0, 10.0);
    HGF2DCoord<signed char> CHC3(10.0, 10.0);
    HGF2DCoord<signed char> CHC4(10.0, 0.0);
    HGF2DCoord<signed char> CHC5(0.0, 0.0);

    HGF2DCoord<unsigned char> CHC1(0.0, 0.0);
    HGF2DCoord<unsigned char> CHC2(0.0, 10.0);
    HGF2DCoord<unsigned char> CHC3(10.0, 10.0);
    HGF2DCoord<unsigned char> CHC4(10.0, 0.0);
    HGF2DCoord<unsigned char> CHC5(0.0, 0.0);


    HGF2DCoord<short> CHS1(0.0, 0.0);
    HGF2DCoord<short> CHS2(0.0, 10.0);
    HGF2DCoord<short> CHS3(10.0, 10.0);
    HGF2DCoord<short> CHS4(10.0, 0.0);
    HGF2DCoord<short> CHS5(0.0, 0.0);

    HGF2DCoord<short> CHSS1(0.0, 0.0);
    HGF2DCoord<short> CHSS2(0.0, 10.0);
    HGF2DCoord<short> CHSS3(10.0, 10.0);
    HGF2DCoord<short> CHSS4(10.0, 0.0);
    HGF2DCoord<short> CHSS5(0.0, 0.0);

    HGF2DCoord<unsigned short> CHUS1(0.0, 0.0);
    HGF2DCoord<unsigned short> CHUS2(0.0, 10.0);
    HGF2DCoord<unsigned short> CHUS3(10.0, 10.0);
    HGF2DCoord<unsigned short> CHUS4(10.0, 0.0);
    HGF2DCoord<unsigned short> CHUS5(0.0, 0.0);

    HGF2DCoord<int32_t> CHI1(0.0, 0.0);
    HGF2DCoord<int32_t> CHI2(0.0, 10.0);
    HGF2DCoord<int32_t> CHI3(10.0, 10.0);
    HGF2DCoord<int32_t> CHI4(10.0, 0.0);
    HGF2DCoord<int32_t> CHI5(0.0, 0.0);

    HGF2DCoord<int32_t> CHSI1(0.0, 0.0);
    HGF2DCoord<int32_t> CHSI2(0.0, 10.0);
    HGF2DCoord<int32_t> CHSI3(10.0, 10.0);
    HGF2DCoord<int32_t> CHSI4(10.0, 0.0);
    HGF2DCoord<int32_t> CHSI5(0.0, 0.0);

    HGF2DCoord<uint32_t> CHUI1(0.0, 0.0);
    HGF2DCoord<uint32_t> CHUI2(0.0, 10.0);
    HGF2DCoord<uint32_t> CHUI3(10.0, 10.0);
    HGF2DCoord<uint32_t> CHUI4(10.0, 0.0);
    HGF2DCoord<uint32_t> CHUI5(0.0, 0.0);

    HGF2DCoord<int32_t> CHL1(0.0, 0.0);
    HGF2DCoord<int32_t> CHL2(0.0, 10.0);
    HGF2DCoord<int32_t> CHL3(10.0, 10.0);
    HGF2DCoord<int32_t> CHL4(10.0, 0.0);
    HGF2DCoord<int32_t> CHL5(0.0, 0.0);

    HGF2DCoord<int32_t> CHSL1(0.0, 0.0);
    HGF2DCoord<int32_t> CHSL2(0.0, 10.0);
    HGF2DCoord<int32_t> CHSL3(10.0, 10.0);
    HGF2DCoord<int32_t> CHSL4(10.0, 0.0);
    HGF2DCoord<int32_t> CHSL5(0.0, 0.0);

    HGF2DCoord<uint32_t> CHUL1(0.0, 0.0);
    HGF2DCoord<uint32_t> CHUL2(0.0, 10.0);
    HGF2DCoord<uint32_t> CHUL3(10.0, 10.0);
    HGF2DCoord<uint32_t> CHUL4(10.0, 0.0);
    HGF2DCoord<uint32_t> CHUL5(0.0, 0.0);

#if (0)
    HGF2DCoord<int64_t> CHLL1(0.0, 0.0);
    HGF2DCoord<int64_t> CHLL2(0.0, 10.0);
    HGF2DCoord<int64_t> CHLL3(10.0, 10.0);
    HGF2DCoord<int64_t> CHLL4(10.0, 0.0);
    HGF2DCoord<int64_t> CHLL5(0.0, 0.0);

    HGF2DCoord<uint64_t> CHULL1(0.0, 0.0);
    HGF2DCoord<uint64_t> CHULL2(0.0, 10.0);
    HGF2DCoord<uint64_t> CHULL3(10.0, 10.0);
    HGF2DCoord<uint64_t> CHULL4(10.0, 0.0);
    HGF2DCoord<uint64_t> CHULL5(0.0, 0.0);

    HGF2DCoord<int64_t> CHSLL1(0.0, 0.0);
    HGF2DCoord<int64_t> CHSLL2(0.0, 10.0);
    HGF2DCoord<int64_t> CHSLL3(10.0, 10.0);
    HGF2DCoord<int64_t> CHSLL4(10.0, 0.0);
    HGF2DCoord<int64_t> CHSLL5(0.0, 0.0);
#endif

    HGF2DCoord<float> CHF1(0.0, 0.0);
    HGF2DCoord<float> CHF2(0.0, 10.0);
    HGF2DCoord<float> CHF3(10.0, 10.0);
    HGF2DCoord<float> CHF4(10.0, 0.0);
    HGF2DCoord<float> CHF5(0.0, 0.0);

    HGF2DCoord<double> CHD1(0.0, 0.0);
    HGF2DCoord<double> CHD2(0.0, 10.0);
    HGF2DCoord<double> CHD3(10.0, 10.0);
    HGF2DCoord<double> CHD4(10.0, 0.0);
    HGF2DCoord<double> CHD5(0.0, 0.0);

    HGF2DCoord<double> CHLD1(0.0, 0.0);
    HGF2DCoord<double> CHLD2(0.0, 10.0);
    HGF2DCoord<double> CHLD3(10.0, 10.0);
    HGF2DCoord<double> CHLD4(10.0, 0.0);
    HGF2DCoord<double> CHLD5(0.0, 0.0);



    // char
    // Test each direct function

    // Constructor
    HGF2DCoord<char> CHCTemp(CHC1);
    HASSERT(CHC1 == CHCTemp);

    // Operator =
    CHCTemp = CHC2;
    HASSERT(CHC2 == CHCTemp);

    // Operator ==
    HASSERT(CHC2 == CHCTemp);

    // Operator!=
    HASSERT(CHC2 != CHC3);

    // GetX()
    char TempData;
    TempData = CHC2.GetX();
    HASSERT(TempData == CHC2.m_XValue);

    // GetY()
    TempData = CHC2.GetY();
    HASSERT(TempData == CHC2.m_YValue);

    // SetX()
    CHC2.SetX(CHC3.GetX());
    HASSERT(CHC2.GetX() == CHC3.GetX());

    // SetY()
    CHC2.SetY(CHC3.GetY());
    HASSERT(CHC2.GetY() == CHC3.GetY());

    // Operator[] non-const
    CHC4[0] == CHC2[0];
    HASSERT(CHC4.GetX() == CHC2.GetX());
    CHC4[1] == CHC2[1];
    HASSERT(CHC4.GetY() == CHC2.GetY());

    CHC4[HGF2DCoord<char>::X] == CHC3[HGF2DCoord<char>::X];
    HASSERT(CHC4.GetX() == CHC3.GetX());

    CHC4[HGF2DCoord<char>::Y] == CHC3[HGF2DCoord<char>::Y];
    HASSERT(CHC4.GetY() == CHC3.GetY());

    // Operator[] const
    const TempCoord(CHC1);
    CHC2.SetX(TempCoord[0]);
    CHC2.SetY(TempCoord[1]);
    HASSERT(CHC2 == TempCoord);
    CHC4.SetX(TempCoord[HGF2DCoord<char>::X]);
    CHC4.SetY(TempCoord[HGF2DCoord<char>::Y]);
    HASSERT(CHC4 == TempCoord);




#endif

    return(0);
    }