#include "../BackDoorORDBridge/PublicApi/BackDoor/ORDBridge/BackDoor.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDHelloWorldGeometryConversionTest)
    {
    ASSERT_TRUE(RunTestApp(WCharCP(L"Geometry.dgn"), WCharCP(L"ORDGeometryTest.bim"), false));
    VerifyConvertedElementCount("ORDGeometryTest.bim", 1, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDHelloWorldCorridorConversionTest)
    {
    ASSERT_TRUE(RunTestApp(WCharCP(L"Corridor.dgn"), WCharCP(L"ORDCorridorTest.bim"), false));
    VerifyConvertedElementCount("ORDCorridorTest.bim", 1, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDFullyFederatedConversionTest)
    {
    ASSERT_TRUE(RunTestApp(WCharCP(L"Fully Federated\\container 2.dgn"), WCharCP(L"ORDFullyFederatedTest.bim"), false));
    VerifyConvertedElementCount("ORDFullyFederatedTest.bim", 2, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDIHLConversionTest)
    {
    ASSERT_TRUE(RunTestApp(WCharCP(L"IHL.dgn"), WCharCP(L"ORDIHLTest.bim"), false));
    VerifyConvertedElementCount("ORDIHLTest.bim", 11, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDIntersectionWithEditsTest)
    {
    ASSERT_TRUE(CopyTestFile("Intersection\\NewIntersection-Original.dgn", "NewIntersection.dgn"));
    ASSERT_TRUE(RunTestApp(WCharCP(L"NewIntersection.dgn"), WCharCP(L"ORDIntersectionTest.bim"), false));
    VerifyConvertedElementCount("ORDIntersectionTest.bim", 1, 1);

    ASSERT_TRUE(CopyTestFile("Intersection\\NewIntersection-Edits.dgn", "NewIntersection.dgn"));
    ASSERT_TRUE(RunTestApp(WCharCP(L"NewIntersection.dgn"), WCharCP(L"ORDIntersectionTest.bim"), true));
    VerifyConvertedElementCount("ORDIntersectionTest.bim", 3, 4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                               Jonathan.DeCarlo                     07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDAllOneConversionTest)
    {
    ASSERT_TRUE(RunTestApp(WCharCP(L"All-One.dgn"), WCharCP(L"ORDAllOneTest.bim"), false));
    VerifyConvertedElementCount("ORDAllOneTest.bim", 11, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDGeometryComparisonAlignmentCountTest)
    {
    //--gtest_filter=*AlignmentCount*
    WCharCP wDgnFileName = WCharCP(L"GeometryComparison.dgn");
    WCharCP wFileName = WCharCP(L"ORDGeometryComparison.bim");
    ASSERT_TRUE(RunTestApp(wDgnFileName, wFileName, false));

    Utf8String bimFile;
    BeStringUtilities::WCharToUtf8(bimFile, wFileName);
    Utf8CP bim = bimFile.c_str();

    VerifyConvertedElementCount(bim, 19, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDGeometryComparisonElementCountAndEndsTest)
    {
    //--gtest_filter=*ComparisonElementCount*
    WCharCP wDgnFileName = WCharCP(L"GeometryComparison.dgn");
    WCharCP wFileName = WCharCP(L"ORDGeometryComparison.bim");
    ASSERT_TRUE(RunTestApp(wDgnFileName, wFileName, false));

    Utf8String bimFile;
    BeStringUtilities::WCharToUtf8(bimFile, wFileName);
    Utf8CP bim = bimFile.c_str();

    VerifyConvertedElementCount(bim, 19, 0);

    double x, y;
    DPoint3d zero = DPoint3d::FromZero();

    /// Non-Ruled Alignments
    x = 101628.44940268152; y = 101184.39207795920;
    VerifyConvertedGeometryElementCountAndEnds(bim, "LL1R", 3, DPoint3d::From(101000, 101000, 0), DPoint3d::From(x, y, 0), 0, zero, zero);
    VerifyConvertedGeometryElementCountAndEnds(bim, "LL1L", 3, DPoint3d::From(101000, 101000, 0), DPoint3d::From(y, x, 0), 0, zero, zero);

    x = 102950.76203827361; y = 102081.39128620713;
    VerifyConvertedGeometryElementCountAndEnds(bim, "LL2R", 5, DPoint3d::From(102000, 102000, 0), DPoint3d::From(x, y, 0), 0, zero, zero);
    VerifyConvertedGeometryElementCountAndEnds(bim, "LL2L", 5, DPoint3d::From(102000, 102000, 0), DPoint3d::From(y, x, 0), 0, zero, zero);

    x = 104061.48655258317; y = 103411.37583895023;
    VerifyConvertedGeometryElementCountAndEnds(bim, "LL3R", 4, DPoint3d::From(103000, 103000, 0), DPoint3d::From(x, y, 0), 0, zero, zero);
    VerifyConvertedGeometryElementCountAndEnds(bim, "LL3L", 4, DPoint3d::From(103000, 103000, 0), DPoint3d::From(y, x, 0), 0, zero, zero);

    x = 104441.93716560971; y = 104404.33241651095;
    VerifyConvertedGeometryElementCountAndEnds(bim, "LL4R", 4, DPoint3d::From(104000, 104000, 0), DPoint3d::From(x, y, 0), 0, zero, zero);
    VerifyConvertedGeometryElementCountAndEnds(bim, "LL4L", 4, DPoint3d::From(104000, 104000, 0), DPoint3d::From(y, x, 0), 0, zero, zero);

    /// Ruled Alignments
    x = 111628.44940268152; y = 111184.39207795922;
    VerifyConvertedGeometryElementCountAndEnds(bim, "RR1R", 3, DPoint3d::From(111000, 111000, 0), DPoint3d::From(x, y, 0), 0, zero, zero);
    VerifyConvertedGeometryElementCountAndEnds(bim, "RR1L", 3, DPoint3d::From(111000, 111000, 0), DPoint3d::From(y, x, 0), 0, zero, zero);

    x = 112950.76203827358; y = 112081.39128620709;
    VerifyConvertedGeometryElementCountAndEnds(bim, "RR2R", 5, DPoint3d::From(112000, 112000, 0), DPoint3d::From(x, y, 0), 0, zero, zero);
    VerifyConvertedGeometryElementCountAndEnds(bim, "RR2L", 5, DPoint3d::From(112000, 112000, 0), DPoint3d::From(y, x, 0), 0, zero, zero);

    x = 114061.48655258316; y = 113411.37583895026;
    VerifyConvertedGeometryElementCountAndEnds(bim, "RR3R", 4, DPoint3d::From(113000, 113000, 0), DPoint3d::From(x, y, 0), 0, zero, zero);
    VerifyConvertedGeometryElementCountAndEnds(bim, "RR3L", 4, DPoint3d::From(113000, 113000, 0), DPoint3d::From(y, x, 0), 0, zero, zero);

    /// Filleted Elements - Trimed Intervals
    DPoint3d bl1 = DPoint3d::From(121000, 121000, 0);
    DPoint3d tr1 = DPoint3d::From(121070.71067811869, 121070.71067811869, 0);
    DPoint3d tr2 = DPoint3d::From(121530.74327629156, 121205.68791947514, 0);
    DPoint3d bl2 = DPoint3d::From(121628.44940268152, 121184.39207795922, 0);
    VerifyConvertedGeometryElementCountAndEnds(bim, "TRM1", 1, bl1, tr1, 0, zero, zero);
    VerifyConvertedGeometryElementCountAndEnds(bim, "CPFT", 1, tr1, tr2, 0, zero, zero);
    VerifyConvertedGeometryElementCountAndEnds(bim, "TRM2", 1, bl2, tr2, 0, zero, zero);

    /// Alignment With and Without Profile
    DPoint3d hBeg, hEnd, vBeg, vEnd;
    DPoint3d diff = DPoint3d::From(1819.8237659554579, 220.44589914100652, 0);

    hBeg = DPoint3d::From(122000, 122000, 0); hEnd = DPoint3d::FromSumOf(hBeg, diff);
    vBeg = zero; vEnd = zero;
    VerifyConvertedGeometryElementCountAndEnds(bim, "CPNP", 9, hBeg, hEnd, 0, vBeg, vEnd);

    hBeg = DPoint3d::From(123000, 123000, 0); hEnd = DPoint3d::FromSumOf(hBeg, diff);
    vBeg = zero; vEnd = DPoint3d::From(2100, 50, 0);
    VerifyConvertedGeometryElementCountAndEnds(bim, "CPWP", 9, hBeg, hEnd, 11, vBeg, vEnd);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDGeometryComparisonElementLengthsTest)
    {
    //--gtest_filter=*ElementLengths*
    WCharCP wDgnFileName = WCharCP(L"GeometryComparison.dgn");
    WCharCP wFileName = WCharCP(L"ORDGeometryComparison.bim");
    ASSERT_TRUE(RunTestApp(wDgnFileName, wFileName, false));

    Utf8String bimFile;
    BeStringUtilities::WCharToUtf8(bimFile, wFileName);
    Utf8CP bim = bimFile.c_str();

    VerifyConvertedElementCount(bim, 19, 0);

    /// Non-Ruled Alignments
    VerifyConvertedGeometryElementLengths(bim, "LL1R", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "LL1L", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "LL2R", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "LL2L", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "LL3R", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "LL3L", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "LL4R", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "LL4L", true, 100, 500, 500, 200, 100, 200, 200);

    /// Ruled Alignments
    VerifyConvertedGeometryElementLengths(bim, "RR1R", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "RR1L", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "RR2R", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "RR2L", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "RR3R", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "RR3L", true, 100, 500, 500, 200, 100, 200, 200);

    /// Filleted Elements - Trimed Intervals
    VerifyConvertedGeometryElementLengths(bim, "TRM1", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "CPFT", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "TRM2", true, 100, 500, 500, 200, 100, 200, 200);

    /// Alignment With and Without Profile
    VerifyConvertedGeometryElementLengths(bim, "CPNP", true, 100, 500, 500, 200, 100, 200, 200);
    VerifyConvertedGeometryElementLengths(bim, "CPWP", true, 100, 500, 500, 200, 100, 200, 200);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDGeometryGapLengthsTest)
    {
    //--gtest_filter=*Gap*
    WCharCP wDgnFileName = WCharCP(L"GeometryGap.dgn");
    WCharCP wFileName = WCharCP(L"ORDGeometryGap.bim");
    ASSERT_TRUE(RunTestApp(wDgnFileName, wFileName, false));

    Utf8String bimFile;
    BeStringUtilities::WCharToUtf8(bimFile, wFileName);
    Utf8CP bim = bimFile.c_str();

    VerifyConvertedElementCount(bim, 1, 0);

    VerifyConvertedGeometryElementLengths(bim, "LLGAP", true, 1000, 0, 0, 0, 0, 0, 0);

    DPoint3d zero = DPoint3d::FromZero();
    DPoint3d beg = DPoint3d::From(1000, 1000, 0);
    DPoint3d end = DPoint3d::From(3000, 3000, 0);
    VerifyConvertedGeometryElementCountAndEnds(bim, "LLGAP", 2, beg, end, 0, zero, zero);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDGeometrySpiralTypesAndLengthsTest)
    {
    //--gtest_filter=*SpiralTypes*
    WCharCP wDgnFileName = WCharCP(L"GeometrySpiral.dgn");
    WCharCP wFileName = WCharCP(L"ORDGeometrySpiral.bim");
    ASSERT_TRUE(RunTestApp(wDgnFileName, wFileName, false));

    Utf8String bimFile;
    BeStringUtilities::WCharToUtf8(bimFile, wFileName);
    Utf8CP bim = bimFile.c_str();

    // JGATODO Add back tests if Earlin pushes spiral changes back to 13

    /// true transition spirals
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "XXXX", DSpiral2dBase::TransitionType_Unknown, 200.0);
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLClothoid", DSpiral2dBase::TransitionType_Clothoid, 200.0);
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLBloss", DSpiral2dBase::TransitionType_Bloss, 200.0);
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLBiquadratic", DSpiral2dBase::TransitionType_Biquadratic, 200.0);
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLCosine", DSpiral2dBase::TransitionType_Cosine, 200.0);
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLSinusoid", DSpiral2dBase::TransitionType_Sine, 200.0);
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "XXXX", DSpiral2dBase::TransitionType_Viennese, 200.0);             // Not implemented in CIF? 
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "XXXX", DSpiral2dBase::TransitionType_WeightedViennese, 200.0);     // Not implemented in CIF?
 
    /// convention: spirals that really have direct evaluations start at 50.
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "XXXX", DSpiral2dBase::TransitionType_FirstDirectEvaluate, 200.0);  // Not implemented in CIF?
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLWACubic", DSpiral2dBase::TransitionType_WesternAustralian, 200.0);
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLCzechCubic", DSpiral2dBase::TransitionType_Czech, 200.0);        // NOT IMPLEMENTED in GeomLib
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLNSWCubic", DSpiral2dBase::TransitionType_AustralianRailCorp, 200.0);
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLItalianCubic", DSpiral2dBase::TransitionType_Italian, 200.0);    // NOT IMPLEMENTED in GeomLib
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLPolishCubic", DSpiral2dBase::TransitionType_PolishCubic, 200.0); // NOT IMPLEMENTED in GeomLib
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLArema", DSpiral2dBase::TransitionType_AremaCubic, 200.0);        // NOT IMPLEMENTED in GeomLib
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLMXCubic", DSpiral2dBase::TransitionType_MXCubicAlongArc, 200.0);
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "XXXX", DSpiral2dBase::TransitionType_MXCubicAlongTangent, 200.0);  // Not implemented in CIF?
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLChineseCubic", DSpiral2dBase::TransitionType_ChineseCubic, 200.0);
 
    /// convention: spirals with nominal length parameter start at 60
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLJapaneseSine", DSpiral2dBase::TransitionType_DirectHalfCosine, 200.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDGeometryRailTurnoutTest)
    {
    //--gtest_filter=*RailTurnout*
    WCharCP wDgnFileName = WCharCP(L"GeometryRailTurnout.dgn");
    WCharCP wFileName = WCharCP(L"ORDGeometryRailTurnout.bim");
    ASSERT_TRUE(RunTestApp(wDgnFileName, wFileName, false));

    Utf8String bimFile;
    BeStringUtilities::WCharToUtf8(bimFile, wFileName);
    Utf8CP bim = bimFile.c_str();

    /// 3 Alignments - 4 Turnouts (2 alignments each ) = 11 total
    VerifyConvertedElementCount(bim, 3, 0); //(bim, 11, 0, 4); add turnout to count when done

    VerifyConvertedGeometryUniqueAlignmentNameExists(bim, "LLL1");
    VerifyConvertedGeometryUniqueAlignmentNameExists(bim, "LLL2");
    VerifyConvertedGeometryUniqueAlignmentNameExists(bim, "CONNECT1");

    // JGATODO Add Turnouts and branches to imodel ??? 
    //VerifyConvertedGeometryTurnoutBranchCount(bim, "Branch1 Geometry", 4);
    //VerifyConvertedGeometryTurnoutBranchCount(bim, "Branch2 Geometry", 4);
    }
