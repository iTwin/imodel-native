/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifndef SKIP_SHORT_RUNNING_TESTS
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
    auto dgnDbPtr = VerifyConvertedElementCount("ORDFullyFederatedTest.bim", 2, 0);
    
    // Testing conversion of view-groups and Saved Views...
    ECSqlStatement stmt;
    stmt.Prepare(*dgnDbPtr, "SELECT CodeValue FROM " BIS_SCHEMA(BIS_CLASS_ViewDefinition));
    ASSERT_TRUE(stmt.IsPrepared());

    size_t index = 0;
    bvector<Utf8String> expectedViewNames = { "Default - View 1", "Curve", "Plan-view Models" };

    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        ASSERT_TRUE(expectedViewNames[index++].Equals(stmt.GetValueText(0)));

    ASSERT_EQ(expectedViewNames.size(), index);
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
    auto dgnDbPtr = VerifyConvertedElementCount("ORDIntersectionTest.bim", 1, 1);

    ECSqlStatement stmt;
    stmt.Prepare(*dgnDbPtr, "SELECT COUNT(*) FROM CivilDesignerProductsDynamic.Alignment__x005C__DummyFD");
    ASSERT_TRUE(stmt.IsPrepared());
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(2, stmt.GetValueInt(0));

    stmt.Finalize();
    dgnDbPtr->CloseDb();

    ASSERT_TRUE(CopyTestFile("Intersection\\NewIntersection-Edits.dgn", "NewIntersection.dgn"));
    ASSERT_TRUE(RunTestApp(WCharCP(L"NewIntersection.dgn"), WCharCP(L"ORDIntersectionTest.bim"), true));
    dgnDbPtr = VerifyConvertedElementCount("ORDIntersectionTest.bim", 3, 1);
    
    stmt.Prepare(*dgnDbPtr, "SELECT COUNT(*) FROM CivilDesignerProductsDynamic.Alignment__x005C__DummyFD");
    ASSERT_TRUE(stmt.IsPrepared());
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(1, stmt.GetValueInt(0));

    stmt.Finalize();
    dgnDbPtr->CloseDb();
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
// JGATODO Spiral Types ... Need later or newer Geomlib Something weird with lengths in geomLib ?
// --gtest_filter=*SpiralTypes*
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDGeometrySpiralTypesAndLengthsTest)
    {
    WCharCP wDgnFileName = WCharCP(L"GeometrySpiral.dgn");
    WCharCP wFileName = WCharCP(L"ORDGeometrySpiral.bim");
    ASSERT_TRUE(RunTestApp(wDgnFileName, wFileName, false));

    Utf8String bimFile;
    BeStringUtilities::WCharToUtf8(bimFile, wFileName);
    Utf8CP bim = bimFile.c_str();

    /// true transition spirals
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLClothoid", DSpiral2dBase::TransitionType_Clothoid, 200.0);
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLBloss", DSpiral2dBase::TransitionType_Bloss, 200.0);
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLBiquadratic", DSpiral2dBase::TransitionType_Biquadratic, 200.0);
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLCosine", DSpiral2dBase::TransitionType_Cosine, 200.0);
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLSinusoid", DSpiral2dBase::TransitionType_Sine, 200.0);

    /// convention: spirals that really have direct evaluations start at 50.
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLWACubic", DSpiral2dBase::TransitionType_WesternAustralian, 200.0);
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLNSWCubic", DSpiral2dBase::TransitionType_AustralianRailCorp, 200.0);
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLMXCubic", DSpiral2dBase::TransitionType_MXCubicAlongArc, 200.0);
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLChineseCubic", DSpiral2dBase::TransitionType_ChineseCubic, 200.0);
 
    /// convention: spirals with nominal length parameter start at 60
    VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLJapaneseSine", DSpiral2dBase::TransitionType_DirectHalfCosine, 200.0);

    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLCzechCubic", DSpiral2dBase::TransitionType_Czech, 200.0);            // NOT IMPLEMENTED in GeomLib
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLItalianCubic", DSpiral2dBase::TransitionType_Italian, 200.0);        // NOT IMPLEMENTED in GeomLib
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLPolishCubic", DSpiral2dBase::TransitionType_PolishCubic, 200.0);     // NOT IMPLEMENTED in GeomLib
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLArema", DSpiral2dBase::TransitionType_AremaCubic, 200.0);            // NOT IMPLEMENTED in GeomLib

    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLViennese", DSpiral2dBase::TransitionType_Viennese, 200.0);               // Not implemented in CIF? 
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLWViennese", DSpiral2dBase::TransitionType_WeightedViennese, 200.0);      // Not implemented in CIF?
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLFirstDirect", DSpiral2dBase::TransitionType_FirstDirectEvaluate, 200.0); // Not implemented in CIF?
    //VerifyConvertedGeometrySpiralTypesAndLengths(bim, "LLMXCubicA", DSpiral2dBase::TransitionType_MXCubicAlongTangent, 200.0);    // Not implemented in CIF?
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
// JGATODO Add Turnouts and branches to imodel
// --gtest_filter=*RailTurnout*
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDGeometryRailTurnoutTest)
    {
    WCharCP wDgnFileName = WCharCP(L"GeometryRailTurnout.dgn");
    WCharCP wFileName = WCharCP(L"ORDGeometryRailTurnout.bim");
    ASSERT_TRUE(RunTestApp(wDgnFileName, wFileName, false));

    Utf8String bimFile;
    BeStringUtilities::WCharToUtf8(bimFile, wFileName);
    Utf8CP bim = bimFile.c_str();
   
    VerifyConvertedElementCount(bim, 3, 0); //(bim, 11, 0, 4); add turnout to count when done --- 3 Alignments - 4 Turnouts (2 alignments each ) = 11 total

    VerifyConvertedGeometryUniqueAlignmentNameExists(bim, "LLL1");
    VerifyConvertedGeometryUniqueAlignmentNameExists(bim, "LLL2");
    VerifyConvertedGeometryUniqueAlignmentNameExists(bim, "CONNECT1");

    //VerifyConvertedGeometryTurnoutBranchCount(bim, "Branch1 Geometry", 4);
    //VerifyConvertedGeometryTurnoutBranchCount(bim, "Branch2 Geometry", 4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDGeometryStationingTest)
    {
    WCharCP wDgnFileName = WCharCP(L"GeometryStationing.dgn");
    WCharCP wFileName = WCharCP(L"ORDGeometryStationing.bim");
    ASSERT_TRUE(RunTestApp(wDgnFileName, wFileName, false));

    Utf8String bimFile;
    BeStringUtilities::WCharToUtf8(bimFile, wFileName);
    Utf8CP bim = bimFile.c_str();

    VerifyConvertedElementCount(bim, 7, 0);

    VerifyConvertedGeometryStationStart(bim, "ZERO", 10000, 0);
    VerifyConvertedGeometryStationStart(bim, "LINE", 10000, 150);
    VerifyConvertedGeometryStationStart(bim, "SPIRAL", 10000, 370);
    VerifyConvertedGeometryStationStart(bim, "ARC", 10000, 770);

    VerifyConvertedGeometryStationEquation(bim, "EQNLINE", 10000, 200, 1, 1200, 1000);
    VerifyConvertedGeometryStationEquation(bim, "EQNSPIRAL", 10000, 200, 1, 540, 1000);
    VerifyConvertedGeometryStationEquation(bim, "EQNARC", 10000, 100, 1, 600, 1000);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDItemTypesTest)
    {
    WCharCP wDgnFileName = WCharCP(L"ItemTypes.dgn");
    WCharCP wFileName = WCharCP(L"ORDItemTypes.bim");
    ASSERT_TRUE(RunTestApp(wDgnFileName, wFileName, false));

    Utf8String bimFile;
    BeStringUtilities::WCharToUtf8(bimFile, wFileName);
    Utf8CP bim = bimFile.c_str();

    VerifyConvertedElementCount(bim, 4, 0);
  
    VerifyConvertedGeometryUniqueAlignmentNameExists(bim, "L1");
    VerifyConvertedGeometryUniqueAlignmentNameExists(bim, "L2");
    VerifyConvertedGeometryUniqueAlignmentNameExists(bim, "L3");
    VerifyConvertedGeometryUniqueAlignmentNameExists(bim, "L4");

    ///"Type1ElementAspect"
    ///"Type2ElementAspect"
    ///"Type3ElementAspect"

    VerifyConvertedElementItemTypes(bim, "L1", "ItemTypeLibrary1", "Type1ElementAspect", 6, "TextProp", "CIFTextValue", 0, 0); 
    VerifyConvertedElementItemTypes(bim, "L1", "ItemTypeLibrary1", "Type1ElementAspect", 6, "IntegerProp", "", 111123, 0); 
    VerifyConvertedElementItemTypes(bim, "L1", "ItemTypeLibrary1", "Type1ElementAspect", 6, "NumberProp", "", 0, 111123.0); 
    VerifyConvertedElementItemTypes(bim, "L1", "ItemTypeLibrary1", "Type1ElementAspect", 6, "NumberUnitProp","", 0, 111321.0); 
    VerifyConvertedElementItemTypes(bim, "L1", "ItemTypeLibrary1", "Type1ElementAspect", 6, "PointProp", "", 0, 1111230000.0); 

    VerifyConvertedElementItemTypes(bim, "L2", "ItemTypeLibrary1", "Type2ElementAspect", 2, "Name", "CIFNameNotForced", 0, 0); 

    VerifyConvertedElementItemTypes(bim, "L3", "ItemTypeLibrary1", "Type3ElementAspect", 2, "Name", "CIFNameForced", 0, 0); 

    VerifyConvertedElementItemTypes(bim, "L4", "ItemTypeLibrary1", "Type1ElementAspect", 6, "TextProp", "CIFTextValue", 0, 0); 
    VerifyConvertedElementItemTypes(bim, "L4", "ItemTypeLibrary1", "Type1ElementAspect", 6, "IntegerProp", "", 111123, 0); 
    VerifyConvertedElementItemTypes(bim, "L4", "ItemTypeLibrary1", "Type1ElementAspect", 6, "NumberProp", "", 0, 111123.0); 
    VerifyConvertedElementItemTypes(bim, "L4", "ItemTypeLibrary1", "Type1ElementAspect", 6, "NumberUnitProp","", 0, 111321.0); 
    VerifyConvertedElementItemTypes(bim, "L4", "ItemTypeLibrary1", "Type1ElementAspect", 6, "PointProp", "", 0, 1111230000.0); 

    VerifyConvertedElementItemTypes(bim, "L4", "ItemTypeLibrary1", "Type2ElementAspect", 2, "Name", "CIFNameNotForced", 0, 0); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDDebugTest)
    {
    WString name = L"Debug";
    //WString name = L"ItemTypesPoint";

    WString dgn = name + L".dgn";
    WString sql = L"ORD" + name + L".bim";

    Utf8String dgnFile;
    BeStringUtilities::WCharToUtf8(dgnFile, dgn.c_str());
    if (!TestFileName(dgnFile.c_str()))
        return;

    ASSERT_TRUE(RunTestApp(dgn.c_str(), sql.c_str(), false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CiviliModelBridgesORDBridgeTests, ORDFullLocalPathTest)
    {
    WString dgn = L"";// L"D:\\Scratch\\Coffs Harbour Bypass\\Detail Design\\Corridor\\Master Corridor1.dgn";
    WString sql = L"";// L"ORDCoffsHarbour.bim";

    BeFileName sourcePath(dgn);
    if (!sourcePath.DoesPathExist())
        return;

    ASSERT_TRUE(RunTestAppFullLocalPath(dgn.c_str(), sql.c_str(), false));
    }

#endif