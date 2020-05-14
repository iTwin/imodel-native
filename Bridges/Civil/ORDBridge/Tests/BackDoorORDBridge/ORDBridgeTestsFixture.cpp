/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../BackDoorORDBridge/PublicApi/BackDoor/ORDBridge/BackDoor.h"

PUSH_DISABLE_DEPRECATION_WARNINGS

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TestKnownLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
    {
    BeFileName m_tempDir;
    BeFileName m_assetsDir;

    virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override
        {
        return m_tempDir;
        }
    virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override
        {
        return m_assetsDir;
        }

    TestKnownLocationsAdmin()
        {
        BeTest::GetHost().GetTempDir(m_tempDir);
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(m_assetsDir);
        }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ORDBridgeTestsHostImpl : DgnPlatformLib::Host
    {
    bool m_isInitialized;

    ORDBridgeTestsHostImpl();
    ~ORDBridgeTestsHostImpl();

    virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override
        {
        return *new TestKnownLocationsAdmin();
        }
    //virtual RepositoryAdmin & _SupplyRepositoryAdmin() override { return *new TestRepositoryAdmin(); }

    virtual void _SupplyProductName(Utf8StringR name) override
        {
        name.assign("ORDBridgeTestsHostImpl");
        }
    virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles()
        {
        return BeSQLite::L10N::SqlangFiles(BeFileName());
        } // no translatable strings
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ORDBridgeTestsHost::GetTestAppProductDirectory()
    {
    BeFileName outputDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(outputDir);
    outputDir.AppendA("..\\..\\ORDBridge\\");

    return outputDir;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName ORDBridgeTestsHost::GetOutputDirectory()
    {
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    outputDir.AppendToPath(L"CiviliModelBridges\\ORDBridge\\");
    return outputDir;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName ORDBridgeTestsHost::GetDgnPlatformAssetsDirectory()
    {
    BeFileName assetsRootDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDirectory);
    return assetsRootDirectory;
    }

WString* ORDBridgeTestsHost::GetInputFileArgument(BeFileName inputPath, WCharCP input, bool isLargeTestFile)
    {
    BeFileName assetsRootDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsRootDirectory);

    BeFileName inputPath1(assetsRootDirectory);
    if (isLargeTestFile)
        inputPath1.AppendString(WCharCP(L"LargeTestFiles\\ORD\\"));
    else
        inputPath1.AppendString(WCharCP(L"TestFiles\\ORD\\"));
    inputPath1.AppendString(WCharCP(input));

    WString inArg(WString(L"--input=\"").append(inputPath1.c_str()).append(L"\"").c_str());
    WCharCP inputArgument(inArg.c_str());
    auto inputArgumentAllocated = new WString(inputArgument);
    return inputArgumentAllocated;
    }

WString* ORDBridgeTestsHost::GetOutputFileArgument(BeFileName outputPath1, WCharCP bimFileName)
    {
    BeFileName outputPath = GetOutputDirectory();
    outputPath.AppendString(WCharCP(bimFileName));
    WString outArg(WString(L"--output=\"").append(outputPath.c_str()).append(L"\"").c_str());
    WCharCP outputArgument(outArg.c_str());
    auto outputArgumentAllocated = new WString(outputArgument);
    return outputArgumentAllocated;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
BeFileName ORDBridgeTestsHost::BuildProjectFileName(WCharCP baseName)
    {
    BeFileName projectFileName = GetOutputDirectory();
    projectFileName.AppendToPath(baseName);
    return projectFileName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
ORDBridgeTestsHost::ORDBridgeTestsHost()
    {
    m_pimpl = new ORDBridgeTestsHostImpl;
    CleanOutputDirectory();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
ORDBridgeTestsHost::~ORDBridgeTestsHost()
    {
    delete m_pimpl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
void ORDBridgeTestsHost::CleanOutputDirectory()
    {
    static bool wasOutputDirectoryCleaned = false;
    if (!wasOutputDirectoryCleaned)
        {
        // clean up files from last run
        BeFileName outputDir = GetOutputDirectory();
        BeFileName::EmptyAndRemoveDirectory(outputDir.GetName());
        BeFileName::CreateNewDirectory(outputDir.GetName());
        wasOutputDirectoryCleaned = true;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonathan.DeCarlo                07/2019
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::IKnownLocationsAdmin& ORDBridgeTestsHost::_SupplyIKnownLocationsAdmin()
    {
    return m_pimpl->_SupplyIKnownLocationsAdmin();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonathan.DeCarlo                07/2019
//---------------------------------------------------------------------------------------
void ORDBridgeTestsHost::_SupplyProductName(Utf8StringR name)
    {
    m_pimpl->_SupplyProductName(name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Jonathan.DeCarlo                07/2019
//---------------------------------------------------------------------------------------
BeSQLite::L10N::SqlangFiles ORDBridgeTestsHost::_SupplySqlangFiles()
    {
    return m_pimpl->_SupplySqlangFiles();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
ORDBridgeTestsHostImpl::ORDBridgeTestsHostImpl() : m_isInitialized(false)
    {
    m_isInitialized = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    11/2014
//---------------------------------------------------------------------------------------
ORDBridgeTestsHostImpl::~ORDBridgeTestsHostImpl()
    {
    }

ORDBridgeTestsHost* CiviliModelBridgesORDBridgeTestsFixture::m_host = nullptr;

//---------------------------------------------------------------------------------------
// Automatically called by gTest framework before running every test
//---------------------------------------------------------------------------------------
void CiviliModelBridgesORDBridgeTestsFixture::SetUpTestCase()
    {
    m_host = new ORDBridgeTestsHost();
    }
//---------------------------------------------------------------------------------------
// Automatically called by gTest framework after running every test
//---------------------------------------------------------------------------------------
void CiviliModelBridgesORDBridgeTestsFixture::TearDownTestCase()
    {
    delete m_host;
    m_host = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool CiviliModelBridgesORDBridgeTestsFixture::TestFileName(Utf8CP source)
    {
    char* outPath = getenv("OutRoot");

    BeFileName sourcePath(outPath);
    sourcePath.AppendA("Winx64\\Product\\CiviliModelBridges-Tests\\Assets\\TestFiles\\ORD\\");
    sourcePath.AppendA(source);

    return sourcePath.DoesPathExist();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CiviliModelBridgesORDBridgeTestsFixture::CopyTestFile(Utf8CP source, Utf8CP target)
    {
    BeFileName assetsPath = m_host->GetDgnPlatformAssetsDirectory();

    BeFileName sourcePath(assetsPath);
    sourcePath.AppendA("TestFiles\\ORD\\");
    sourcePath.AppendA(source);

    BeFileName targetPath(assetsPath);
    targetPath.AppendA("TestFiles\\ORD\\");
    targetPath.AppendA(target);

    return (BeFileNameStatus::Success == BeFileName::BeCopyFile(sourcePath, targetPath, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool CiviliModelBridgesORDBridgeTestsFixture::RunTestApp(WCharCP input, WCharCP bimFileName, bool updateMode, 
    bool isLargeTestFile)
    {
    BeFileName testAppPath = m_host->GetTestAppProductDirectory();
    testAppPath.AppendA("PublishORDToBim.exe");

    BeFileName assetsPath = m_host->GetDgnPlatformAssetsDirectory();
    BeFileName inputPath(assetsPath);
    if (isLargeTestFile)
        inputPath.AppendString(WCharCP(L"LargeTestFiles\\ORD\\"));
    else
        inputPath.AppendString(WCharCP(L"TestFiles\\ORD\\"));
    inputPath.AppendString(WCharCP(input));

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendString(WCharCP(bimFileName));

    if (!updateMode && outputPath.DoesPathExist())
        outputPath.BeDeleteFile();

    WCharCP testAppPathArgument = testAppPath;
    auto inputArgument = m_host->GetInputFileArgument(inputPath, input, isLargeTestFile);
    auto outputArgument = m_host->GetOutputFileArgument(outputPath, bimFileName);
    WCharCP noAssertDialoguesArgument = WCharCP(L"--no-assert-dialogs");
    WCharCP command[4] = {testAppPathArgument, WCharCP((*inputArgument).c_str()), WCharCP((*outputArgument).c_str()), noAssertDialoguesArgument};

    _wputenv(L"ORDBRIDGE_TESTING=1");

    int errcode = PublishORDToBimDLL::RunBridge(4, command);

    //had to store the arguments on the heap to prevent the strings from colliding
    delete inputArgument;
    delete outputArgument;

    bool retVal = (0 == errcode);

    if (retVal)
        {
        if (!outputPath.DoesPathExist())
            EXPECT_TRUE(false);
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName CiviliModelBridgesORDBridgeTestsFixture::GetOutputDir()
    {
    return m_host->GetOutputDirectory();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName CiviliModelBridgesORDBridgeTestsFixture::GetTestAppProductDir()
    {
    return m_host->GetTestAppProductDirectory();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName CiviliModelBridgesORDBridgeTestsFixture::GetDgnPlatformAssetsDir()
    {
    return m_host->GetDgnPlatformAssetsDirectory();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool CiviliModelBridgesORDBridgeTestsFixture::RunTestAppFullLocalPath(WCharCP inputFullLocalPath, WCharCP bimFileName, bool updateMode)
    {
    BeFileName testAppPath = m_host->GetTestAppProductDirectory();
    testAppPath.AppendA("PublishORDToBim.exe");

    BeFileName inputPath(inputFullLocalPath);

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendString(WCharCP(bimFileName));

    if (!updateMode && outputPath.DoesPathExist())
        outputPath.BeDeleteFile();

    WCharCP testAppPathArgument = testAppPath;

    WString inputArgument(WString(WCharCP(L"--input=\"")).append(inputPath.c_str()).append(WCharCP(L"\"")).c_str());
    //auto inputArgument = m_host->GetInputFileArgument(inputPath, input);
    auto outputArgument = m_host->GetOutputFileArgument(outputPath, bimFileName);
    WCharCP noAssertDialoguesArgument = WCharCP(L"--no-assert-dialogs");
    WCharCP command[4] = {testAppPathArgument, WCharCP((inputArgument).c_str()), WCharCP((*outputArgument).c_str()), noAssertDialoguesArgument};

    int errcode = PublishORDToBimDLL::RunBridge(4, command);

    //had to store the arguments on the heap to prevent the strings from colliding
    //delete inputArgument;
    delete outputArgument;

    bool retVal = (0 == errcode);

    if (retVal)
        {
        if (!outputPath.DoesPathExist())
            EXPECT_TRUE(false);
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentCPtr getRoadRailAlignmentByName(DgnDbP dgnDbPtr, Utf8CP alignmentName)
    {
    ECSqlStatement stmt;
    stmt.Prepare(*dgnDbPtr, "SELECT a.ECInstanceId, a.UserLabel FROM "
        BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " d "
        "WHERE m.ModeledElement.Id = d.ECInstanceId AND a.Model.Id = m.ECInstanceId ");
    EXPECT_TRUE(stmt.IsPrepared());

    //ECSqlStatement stmt;
    //stmt.Prepare(*dgnDbPtr, "SELECT a.ECInstanceId, a.UserLabel, a.StartStation, a.StartValue FROM "
    //    BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " d "
    //    "WHERE m.ModeledElement.Id = d.ECInstanceId AND a.Model.Id = m.ECInstanceId ");
    //EXPECT_TRUE(stmt.IsPrepared());

    bool found = false;
    AlignmentCPtr roadRailAlignmentCPtr = nullptr;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        auto alignmentId = stmt.GetValueId<DgnElementId>(0);
        auto alignmentNameCP = stmt.GetValueText(1);
        //auto startStation = stmt.GetValueDouble(2);
        //auto StartValue = stmt.GetValueDouble(3);

        if (alignmentName == nullptr || alignmentNameCP == nullptr || 0 != Utf8String(alignmentName).CompareTo(alignmentNameCP))
            continue;
        found = true;

        roadRailAlignmentCPtr = BentleyM0200::RoadRailAlignment::Alignment::Get(*dgnDbPtr, alignmentId);
        //auto alignmentPairPtr = roadRailAlignmentCPtr->QueryMainPair();
        }
    stmt.Finalize();

    if (!found || roadRailAlignmentCPtr.IsNull())
        EXPECT_TRUE(false && "Alignment not found");

    return roadRailAlignmentCPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesORDBridgeTestsFixture::VerifyConvertedElementCount(Utf8CP bimFileName, size_t alignmentCount, size_t corridorCount)
    {
    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    EXPECT_TRUE(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    if (alignmentCount >= 0)
        {
        ECSqlStatement stmt;
        stmt.Prepare(*dgnDbPtr, "SELECT COUNT(*) FROM "
            BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " d "
            "WHERE m.ModeledElement.Id = d.ECInstanceId AND a.Model.Id = m.ECInstanceId ");
        EXPECT_TRUE(stmt.IsPrepared());
        DbResult stepResult = stmt.Step();
        EXPECT_TRUE(stepResult == DbResult::BE_SQLITE_ROW || stepResult == DbResult::BE_SQLITE_DONE);
        int nm = stepResult == DbResult::BE_SQLITE_ROW ? stmt.GetValueInt(0) : 0;
        if (nm != alignmentCount)
            EXPECT_TRUE(false == nm && "Alignment Count failed.");
        stmt.Finalize();
        }

    if (corridorCount >= 0)
        {
        ECSqlStatement stmt;
        stmt.Prepare(*dgnDbPtr, "SELECT COUNT(*) FROM "
            BRRP_SCHEMA(BRRP_CLASS_Corridor) " c ");
        EXPECT_TRUE(stmt.IsPrepared());
        DbResult stepResult = stmt.Step();
        EXPECT_TRUE(stepResult == DbResult::BE_SQLITE_ROW || stepResult == DbResult::BE_SQLITE_DONE);
        int nm = stepResult == DbResult::BE_SQLITE_ROW ? stmt.GetValueInt(0) : 0;
        if (nm != corridorCount)
            EXPECT_TRUE(false && "Corridor Count failed.");
        stmt.Finalize();
        }

    return dgnDbPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesORDBridgeTestsFixture::VerifyConvertedGeometryUniqueAlignmentNameExists(Utf8CP bimFileName, Utf8CP alignmentName)
    {
    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    EXPECT_TRUE(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    ECSqlStatement stmt;
    stmt.Prepare(*dgnDbPtr, "SELECT a.ECInstanceId, a.UserLabel FROM "
        BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " d "
        "WHERE m.ModeledElement.Id = d.ECInstanceId AND a.Model.Id = m.ECInstanceId ");
    EXPECT_TRUE(stmt.IsPrepared());

    int count = 0;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        auto alignmentId = stmt.GetValueId<DgnElementId>(0);
        auto alignmentNameCP = stmt.GetValueText(1);
        if (alignmentName == nullptr || alignmentNameCP == nullptr || 0 != Utf8String(alignmentName).CompareTo(alignmentNameCP))
            continue;
        count++;
        }

    if (count != 1)
        EXPECT_TRUE(false && "Alignment not found");

    stmt.Finalize();

    return dgnDbPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesORDBridgeTestsFixture::VerifyConvertedGeometryTurnoutBranchCount(Utf8CP bimFileName, Utf8CP branchName, size_t branchCount)
    {
    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    EXPECT_TRUE(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    ECSqlStatement stmt;
    stmt.Prepare(*dgnDbPtr, "SELECT a.ECInstanceId, a.UserLabel FROM "
        BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " d "
        "WHERE m.ModeledElement.Id = d.ECInstanceId AND a.Model.Id = m.ECInstanceId ");
    EXPECT_TRUE(stmt.IsPrepared());

    int count = 0;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        auto branchId = stmt.GetValueId<DgnElementId>(0);
        auto branchNameCP = stmt.GetValueText(1);
        if (branchName == nullptr || branchNameCP == nullptr || 0 != Utf8String(branchName).CompareTo(branchNameCP))
            continue;
        count++;
        }

    if (branchCount != count)
        EXPECT_TRUE(false && "Branch not found");

    stmt.Finalize();

    return dgnDbPtr;
    }

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
static bool checkCurveVectorElementCountAndEnds
(
    CurveVectorCR curves, bool isVertical, size_t& elementCount,
    DPoint3dCR beg,
    DPoint3dCR end,
    double tolerance
)
    {
    if (curves.empty())
        return false;

    DPoint3d cBeg, cEnd;
    curves.GetStartEnd(cBeg, cEnd);

    if (beg.DistanceXY(cBeg) > tolerance)
        return false;

    if (end.DistanceXY(cEnd) > tolerance)
        return false;

    for (ICurvePrimitivePtr curve : curves)
        {
        if (!curve.IsValid())
            return false;

        switch (curve->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                DSegment3dCP segment = (DSegment3dCP) curve->GetLineCP();
                if (segment == nullptr)
                    return false;
                elementCount++;
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                DEllipse3dCP arc = (DEllipse3dCP) curve->GetArcCP();
                if (arc == nullptr)
                    return false;
                elementCount++;
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                {
                DSpiral2dPlacementCP spiralData = (DSpiral2dPlacementCP) curve->GetSpiralPlacementCP();
                if (spiralData == nullptr)
                    return false;
                elementCount++;
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                {
                MSBsplineCurveCP bcurve = (MSBsplineCurveCP) curve->GetProxyBsplineCurveCP();
                if (bcurve == nullptr || !isVertical)
                    return false;
                elementCount++;
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                {
                return checkCurveVectorElementCountAndEnds(*curve->GetChildCurveVectorCP(), isVertical, elementCount, beg, end, tolerance);
                }

            default:
                {
                EXPECT_TRUE(false && "Unexpected entry in CurveVector.");
                return false;
                }
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesORDBridgeTestsFixture::VerifyConvertedGeometryElementCountAndEnds
(
    Utf8CP bimFileName,
    Utf8CP alignmentName,
    int hElementCount,
    DPoint3dCR hBeg,
    DPoint3dCR hEnd,
    int vElementCount,
    DPoint3dCR vBeg,
    DPoint3dCR vEnd
)
    {
    const double tolerance = 1.0E-8;

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    EXPECT_TRUE(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    auto roadRailAlignmentCPtr = getRoadRailAlignmentByName(dgnDbPtr.get(), alignmentName);
    if (roadRailAlignmentCPtr.IsValid())
        {
        auto horizontalAlignmentCPtr = roadRailAlignmentCPtr->GetHorizontal();
        if (horizontalAlignmentCPtr.IsValid())
            {
            auto curveVectorCR = horizontalAlignmentCPtr->GetGeometry();
            size_t elementCountCheck = 0;

            bool verified = checkCurveVectorElementCountAndEnds(curveVectorCR, false, elementCountCheck, hBeg, hEnd, tolerance);
            if (!verified)
                EXPECT_TRUE(verified && "Horizontal Check Ends Verification failed.");

            bool checkCount = elementCountCheck == hElementCount;
            if (!checkCount)
                EXPECT_TRUE(checkCount && "Horizontal Check Element Count failed.");
            }

        auto verticalAlignmentCPtr = roadRailAlignmentCPtr->GetMainVertical();
        if (verticalAlignmentCPtr.IsValid() && horizontalAlignmentCPtr.IsValid())
            {
            auto curveVectorPtr = verticalAlignmentCPtr->GetGeometry();
            size_t elementCountCheck = 0;

            bool verified = checkCurveVectorElementCountAndEnds(*curveVectorPtr, true, elementCountCheck, vBeg, vEnd, tolerance);
            if (!verified)
                EXPECT_TRUE(verified && "Vertical Check Ends Verification failed.");

            bool checkCount = elementCountCheck == vElementCount;
            if (!checkCount)
                EXPECT_TRUE(checkCount && "Vertical Check Element Count failed.");
            }
        }

    return dgnDbPtr;
    }

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
static bool checkCurveVectorElementLengths
(
    CurveVectorCR curves, bool isVertical, double& lengthCheckSum,
    bool checkExactElementLengthValues,
    double hLineLength,
    double hArcLength,
    double hArcRadius,
    double hSpiralLength,
    double vLineLength,
    double vArcLength,
    double vParabolaLength,
    double tolerance
)
    {
    if (curves.empty())
        return false;

    for (ICurvePrimitivePtr curve : curves)
        {
        if (!curve.IsValid())
            return false;

        switch (curve->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                DSegment3dCP segment = (DSegment3dCP) curve->GetLineCP();
                double length = segment->Length();
                double lenCmp = isVertical ? vLineLength : hLineLength;
                lengthCheckSum -= length;

                if (checkExactElementLengthValues)
                    {
                    if (isVertical)
                        {
                        DPoint3d beg, end;
                        segment->GetEndPoints(beg, end);
                        length = end.x - beg.x;
                        }

                    if (fabs(lenCmp - length) > tolerance)
                        return false;
                    }
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                DEllipse3dCP arc = (DEllipse3dCP) curve->GetArcCP();
                double length = arc->ArcLength();
                double lenCmp = isVertical ? vArcLength : hArcLength;
                lengthCheckSum -= length;

                if (checkExactElementLengthValues)
                    {
                    if (isVertical)
                        {
                        DPoint3d beg, end;
                        DVec3d begTangent, endTangent;
                        curve->GetStartEnd(beg, end, begTangent, endTangent);
                        length = end.x - beg.x;
                        }

                    if (fabs(lenCmp - length) > tolerance)
                        return false;
                    }
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                {
                DSpiral2dPlacementCP spiralData = (DSpiral2dPlacementCP) curve->GetSpiralPlacementCP();
                double length = spiralData->SpiralLengthActiveInterval();
                double lenCmp = hSpiralLength;
                lengthCheckSum -= length;

                if (checkExactElementLengthValues)
                    {
                    if (fabs(lenCmp - length) > tolerance)
                        return false;
                    }
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                {
                MSBsplineCurveCP bcurve = (MSBsplineCurveCP) curve->GetProxyBsplineCurveCP();

                size_t count = bcurve->GetNumPoles();
                if (3 != count)
                    return false;

                bvector<DPoint3d> poles;
                bcurve->GetPoles(poles);

                DPoint3d pvc, pvi, pvt;
                pvc = DPoint3d::From(poles[0].x, poles[0].y, 0.0);
                pvi = DPoint3d::From(poles[1].x, poles[1].y, 0.0);
                pvt = DPoint3d::From(poles[2].x, poles[2].y, 0.0);

                double length = bcurve->Length();
                double lenCmp = vParabolaLength;
                lengthCheckSum -= length;

                length = fabs(poles[2].x - poles[0].x);
                if (checkExactElementLengthValues)
                    {
                    if (fabs(lenCmp - length) > tolerance)
                        return false;
                    }
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                {
                /// Nested curve vector can occur with BOUNDARY_TYPE_None and not just union/parity regions...
                return checkCurveVectorElementLengths(*curve->GetChildCurveVectorCP(), isVertical, lengthCheckSum,
                    checkExactElementLengthValues,
                    hLineLength,
                    hArcLength,
                    hArcRadius,
                    hSpiralLength,
                    vLineLength,
                    vArcLength,
                    vParabolaLength,
                    tolerance);
                }

            default:
                {
                EXPECT_TRUE(false && "Unexpected entry in CurveVector.");
                return false;
                }
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesORDBridgeTestsFixture::VerifyConvertedGeometryElementLengths
(
    Utf8CP bimFileName,
    Utf8CP alignmentName,
    bool checkExactElementLengthValues,
    double hLineLength,
    double hArcLength,
    double hArcRadius,
    double hSpiralLength,
    double vLineLength,
    double vArcLength,
    double vParabolaLength
)
    {
    const double tolerance = 1.0E-8;

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    EXPECT_TRUE(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    auto roadRailAlignmentCPtr = getRoadRailAlignmentByName(dgnDbPtr.get(), alignmentName);
    if (roadRailAlignmentCPtr.IsValid())
        {
        auto horizontalAlignmentCPtr = roadRailAlignmentCPtr->GetHorizontal();
        if (horizontalAlignmentCPtr.IsValid())
            {
            auto curveVectorCR = horizontalAlignmentCPtr->GetGeometry();
            double length = curveVectorCR.Length();

            bool verified = checkCurveVectorElementLengths(curveVectorCR, false, length,
                checkExactElementLengthValues,
                hLineLength,
                hArcLength,
                hArcRadius,
                hSpiralLength,
                vLineLength,
                vArcLength,
                vParabolaLength,
                tolerance);

            bool checkSum = fabs(length) < tolerance;

            if (!verified)
                EXPECT_TRUE(verified && "Horizontal Length Verification failed.");

            if (!checkSum)
                EXPECT_TRUE(checkSum && "Horizontal Length Checksum failed.");
            }

        auto verticalAlignmentCPtr = roadRailAlignmentCPtr->GetMainVertical();
        if (verticalAlignmentCPtr.IsValid() && horizontalAlignmentCPtr.IsValid())
            {
            auto curveVectorPtr = verticalAlignmentCPtr->GetGeometry();
            double length = curveVectorPtr->Length();

            bool verified = checkCurveVectorElementLengths(*curveVectorPtr, true, length,
                checkExactElementLengthValues,
                hLineLength,
                hArcLength,
                hArcRadius,
                hSpiralLength,
                vLineLength,
                vArcLength,
                vParabolaLength,
                tolerance);

            bool checkSum = fabs(length) < tolerance;

            if (!verified)
                EXPECT_TRUE(verified && "Vertical Length Verification failed.");

            if (!checkSum)
                EXPECT_TRUE(checkSum && "Vertical Length Checksum failed.");
            }
        }

    return dgnDbPtr;
    }

/*---------------------------------------------------------------------------------**//**
// JGATODO Spiral Types ... Need later or newer Geomlib Something weird with lengths in geomLib ?
+---------------+---------------+---------------+---------------+---------------+------*/
static bool checkCurveVectorSpiralTypesAndLengths(BentleyM0200::CurveVectorCR curves, int spiralType, double spiralLength, double tolerance)
    {
    if (curves.empty())
        return false;

    for (BentleyM0200::ICurvePrimitivePtr curve : curves)
        {
        if (!curve.IsValid())
            return false;

        switch (curve->GetCurvePrimitiveType())
            {
            case BentleyM0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                break;

            case BentleyM0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                break;

            case BentleyM0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                {
                BentleyM0200::DSpiral2dPlacementCP spiralData = (BentleyM0200::DSpiral2dPlacementCP) curve->GetSpiralPlacementCP();
                double length = spiralData->SpiralLengthActiveInterval();

                if (spiralType != spiralData->spiral->GetTransitionTypeCode())
                    return false;

                if (spiralType == DSpiral2dBase::TransitionType_WesternAustralian ||
                    spiralType == DSpiral2dBase::TransitionType_MXCubicAlongArc ||
                    spiralType == DSpiral2dBase::TransitionType_ChineseCubic)
                    {
                    // Need to ask Claude or Earlin but length do not match very well?
                    /*double length01 = */spiralData->SpiralLength01();
                    /*double lengthMap = */spiralData->MappedSpiralLengthActiveInterval(RotMatrix::FromIdentity());
                    if (fabs(length - spiralLength) > 0.1)
                        return false;
                    }
                else
                    {
                    if (fabs(length - spiralLength) > tolerance)
                        return false;
                    }
                break;
                }
                //case BentleyM0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                //case BentleyM0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
                //case BentleyM0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                //case BentleyM0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                //case BentleyM0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:

            case BentleyM0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                {
                /// Nested curve vector can occur with BOUNDARY_TYPE_None and not just union/parity regions...
                return checkCurveVectorSpiralTypesAndLengths(*curve->GetChildCurveVectorCP(), spiralType, spiralLength, tolerance);
                }

            default:
                {
                EXPECT_TRUE(false && "Unexpected entry in CurveVector.");
                return false;
                }
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
// JGATODO Spiral Types ... Need later or newer Geomlib Something weird with lengths in geomLib ?
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesORDBridgeTestsFixture::VerifyConvertedGeometrySpiralTypesAndLengths(Utf8CP bimFileName, Utf8CP alignmentName, int spiralType, double spiralLength)
    {
    const double tolerance = 1.0E-8;

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    EXPECT_TRUE(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    auto roadRailAlignmentCPtr = getRoadRailAlignmentByName(dgnDbPtr.get(), alignmentName);
    if (roadRailAlignmentCPtr.IsValid())
        {
        auto horizontalAlignmentCPtr = roadRailAlignmentCPtr->GetHorizontal();
        if (horizontalAlignmentCPtr.IsValid())
            {
            auto curveVectorCR = horizontalAlignmentCPtr->GetGeometry();

            bool verified = checkCurveVectorSpiralTypesAndLengths(curveVectorCR, spiralType, spiralLength, tolerance);

            if (!verified)
                EXPECT_TRUE(verified && "Horizontal Spiral Type and Length Verification failed.");
            }
        }

    return dgnDbPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesORDBridgeTestsFixture::VerifyConvertedGeometryStationStart(Utf8CP bimFileName, Utf8CP alignmentName, double startingStation, double startingDistance)
    {
    const double tolerance = 1.0E-8;

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    EXPECT_TRUE(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    auto roadRailAlignmentCPtr = getRoadRailAlignmentByName(dgnDbPtr.get(), alignmentName);
    if (roadRailAlignmentCPtr.IsValid())
        {
        auto startStation = roadRailAlignmentCPtr->GetStartStation();
        auto startDistance = roadRailAlignmentCPtr->GetStartValue();
        if (fabs(startStation - startingStation) > tolerance)
            EXPECT_TRUE(false && "Horizontal Start Station Verification failed.");
        if (fabs(startDistance - startingDistance) > tolerance)
            EXPECT_TRUE(false && "Horizontal Start Distance Verification failed.");
        }

    return dgnDbPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesORDBridgeTestsFixture::VerifyConvertedGeometryStationEquation(Utf8CP bimFileName, Utf8CP alignmentName, double startingStation, double startingDistance, int eqnCount, double eqnDistanceAlong, double eqnStationAhead)
    {
    const double tolerance = 1.0E-8;

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    EXPECT_TRUE(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    auto roadRailAlignmentCPtr = getRoadRailAlignmentByName(dgnDbPtr.get(), alignmentName);
    if (roadRailAlignmentCPtr.IsValid())
        {
        auto startStation = roadRailAlignmentCPtr->GetStartStation();
        auto startDistance = roadRailAlignmentCPtr->GetStartValue();
        if (fabs(startStation - startingStation) > tolerance)
            EXPECT_TRUE(false && "Horizontal Start Station Verification failed.");
        if (fabs(startDistance - startingDistance) > tolerance)
            EXPECT_TRUE(false && "Horizontal Start Distance Verification failed.");

        int count = 0;
        for (auto distanceAlongStationPair : roadRailAlignmentCPtr->QueryOrderedStations())
            {
            auto eqnStation = distanceAlongStationPair.GetStation();
            auto eqnDistance = distanceAlongStationPair.GetDistanceAlongFromStart();

            if (count == 1)// Skip first and last since is start station and end station ... only test first eqn for now ?
                {
                if (fabs(eqnStation - eqnStationAhead) > tolerance)
                    EXPECT_TRUE(false && "Horizontal First Equation Station Verification failed.");
                if (fabs(eqnDistance - eqnDistanceAlong) > tolerance)
                    EXPECT_TRUE(false && "Horizontal First Equation Distance Verification failed.");
                }
            count++;
            }

        if (count != eqnCount + 2)
            EXPECT_TRUE(false && "Horizontal Equation count failed.");
        }

    return dgnDbPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesORDBridgeTestsFixture::VerifyConvertedElementItemTypes(Utf8CP bimFileName, Utf8CP alignmentName, Utf8CP itemTypeLibName, Utf8CP typeClassName, size_t typePropCount, Utf8CP typePropName, Utf8CP typePropStringValue, int typePropIntegerValue, double typePropDoubleValue)
    {
    const double tolerance = 1.0E-8;

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    EXPECT_TRUE(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    /// "DgnCustomItemTypes_ItemTypeLibrary1" "DgnCustomItemTypes_Converter"
    Utf8PrintfString typeSchemaName("DgnCustomItemTypes_%s", itemTypeLibName);
    BentleyApi::ECN::ECSchemaCP dynSchema = dgnDbPtr->Schemas().GetSchema(Utf8String(typeSchemaName));
    EXPECT_TRUE(NULL != dynSchema);
    EXPECT_TRUE(dynSchema->IsDynamicSchema());
    int classCount = dynSchema->GetClassCount();
    EXPECT_TRUE(classCount > 0);

    BentleyApi::ECN::ECClassCP ecClass = dynSchema->GetClassCP(typeClassName);
    EXPECT_TRUE(ecClass != NULL);
    size_t classPropCount = ecClass->GetPropertyCount();
    EXPECT_TRUE(typePropCount == classPropCount);

    ECSqlStatement stmt;
    Utf8PrintfString ecSql("SELECT itemType.%s, itemType.* FROM " BRRA_SCHEMA(BRRA_CLASS_Alignment) " alg, " BIS_SCHEMA("GraphicalElement3dRepresentsElement") " rel, %s.%s itemType WHERE alg.UserLabel = ? AND alg.ECInstanceId = rel.TargetECInstanceId AND itemType.Element.Id = rel.SourceECInstanceId",
        typePropName,
        typeSchemaName.c_str(),
        typeClassName);
    stmt.Prepare(*dgnDbPtr, ecSql.c_str());
    EXPECT_TRUE(stmt.IsPrepared());

    stmt.BindText(1, alignmentName, IECSqlBinder::MakeCopy::No); /// Binds to 'alg.UserLabel = ?'
    if (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        int count = stmt.GetColumnCount();
        EXPECT_TRUE(count == typePropCount + 3); /// +3 additional columns

        auto colInfo = stmt.GetColumnInfo(0);
        ECN::ECPropertyCP colProp = colInfo.GetProperty();
        auto typProp = colProp->GetTypeName();

        if (0 == Utf8String("string").CompareTo(typProp.c_str()))
            {
            auto str = stmt.GetValueText(0);
            EXPECT_TRUE(0 == Utf8String(typePropStringValue).CompareTo(str));
            }
        else if (0 == Utf8String("int").CompareTo(typProp.c_str()))
            {
            auto val = stmt.GetValueInt64(0);
            EXPECT_TRUE(typePropIntegerValue == val);
            }
        else if (0 == Utf8String("double").CompareTo(typProp.c_str()))
            {
            auto val = stmt.GetValueDouble(0);
            EXPECT_TRUE(tolerance > fabs(typePropDoubleValue - val));
            }
        else if (0 == Utf8String("point3d").CompareTo(typProp.c_str()))
            {
            auto point = stmt.GetValuePoint3d(0);
            EXPECT_TRUE(tolerance > fabs(typePropDoubleValue - point.x));
            }
        else
            {
            EXPECT_TRUE(false && "Property type was not expected.");
            }
        }
    stmt.Finalize();

    return dgnDbPtr;
    }
POP_DISABLE_DEPRECATION_WARNINGS
