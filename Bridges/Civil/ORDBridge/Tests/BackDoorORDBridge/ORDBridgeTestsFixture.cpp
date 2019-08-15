#include "../BackDoorORDBridge/PublicApi/BackDoor/ORDBridge/BackDoor.h"

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
//! @bsiclass
//=======================================================================================
struct TestViewManager : ViewManager
    {
    protected:
        virtual Display::SystemContext* _GetSystemContext() override
            {
            return nullptr;
            }
        virtual bool _DoesHostHaveFocus() override
            {
            return true;
            }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ORDBridgeTestsHostImpl : DgnViewLib::Host
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
    virtual ViewManager& _SupplyViewManager() override
        {
        return *new TestViewManager();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ORDBridgeTestsHost::GetTestAppProductDirectory()
    {
    char* outPath = getenv("OutRoot");

    BeFileName testAppPath(outPath);
    testAppPath.AppendA("Winx64\\Product\\ORDBridge\\");

    return testAppPath;
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

WString* ORDBridgeTestsHost::GetInputFileArgument(BeFileName inputPath, WCharCP input)
    {
    char* outPath = getenv("OutRoot");

    BeFileName inputPath1(outPath);
    inputPath1.AppendString(WCharCP(L"Winx64\\Product\\CiviliModelBridges-Tests\\Assets\\TestFiles\\ORD\\"));
    inputPath1.AppendString(WCharCP(input));

    WCharCP inputArgument(WString(WCharCP(L"--input=\"")).append(inputPath1.c_str()).append(WCharCP(L"\"")).c_str());
    auto inputArgumentAllocated = new WString(inputArgument);
    return inputArgumentAllocated;
    }

WString* ORDBridgeTestsHost::GetOutputFileArgument(BeFileName outputPath1, WCharCP bimFileName)
    {
    char* outPath = getenv("OutRoot");

    BeFileName outputPath = GetOutputDirectory();
    outputPath.AppendString(WCharCP(bimFileName));
    WCharCP outputArgument(WString(WCharCP(L"--output=\"")).append(outputPath.c_str()).append(WCharCP(L"\"")).c_str());
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
// @bsimethod                                   Jonathan.DeCarlo                07/2019
//---------------------------------------------------------------------------------------
Dgn::ViewManager& ORDBridgeTestsHost::_SupplyViewManager()
    {
    return m_pimpl->_SupplyViewManager();
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
* @bsimethod                                    Diego.Diaz                      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CiviliModelBridgesORDBridgeTestsFixture::CopyTestFile(Utf8CP source, Utf8CP target)
    {
    char* outPath = getenv("OutRoot");

    BeFileName sourcePath(outPath);
    sourcePath.AppendA("Winx64\\Product\\CiviliModelBridges-Tests\\Assets\\TestFiles\\ORD\\");
    sourcePath.AppendA(source);

    BeFileName targetPath(outPath);
    targetPath.AppendA("Winx64\\Product\\CiviliModelBridges-Tests\\Assets\\TestFiles\\ORD\\");
    targetPath.AppendA(target);

    return (BeFileNameStatus::Success == BeFileName::BeCopyFile(sourcePath, targetPath, false));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool CiviliModelBridgesORDBridgeTestsFixture::RunTestApp(WCharCP input, WCharCP bimFileName, bool updateMode)
    {
    char* outPath = getenv("OutRoot");
    BeFileName testAppPath = m_host->GetTestAppProductDirectory();
    testAppPath.AppendA("PublishORDToBim.exe");

    BeFileName inputPath(outPath);
    inputPath.AppendString(WCharCP(L"Winx64\\Product\\CiviliModelBridges-Tests\\Assets\\TestFiles\\ORD\\"));
    inputPath.AppendString(WCharCP(input));

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendString(WCharCP(bimFileName));

    WCharCP testAppPathArgument = testAppPath;
    auto inputArgument = m_host->GetInputFileArgument(inputPath, input);
    auto outputArgument = m_host->GetOutputFileArgument(outputPath, bimFileName);
    WCharCP noAssertDialoguesArgument = WCharCP(L"--no-assert-dialogs");
    WCharCP command[4] = {testAppPathArgument, WCharCP((*inputArgument).c_str()), WCharCP((*outputArgument).c_str()), noAssertDialoguesArgument};

    int errcode = PublishORDToBimDLL::RunBridge(4, command);

    //had to store the arguments on the heap to prevent the strings from colliding
    delete inputArgument;
    delete outputArgument;

    bool retVal = (0 == errcode);

    if (retVal)
        {
        if (!outputPath.DoesPathExist())
            BeAssert(false);
        }

    return retVal;
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

    BeAssert(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    if (alignmentCount >= 0)
        {
        ECSqlStatement stmt;
        stmt.Prepare(*dgnDbPtr, "SELECT COUNT(*) FROM "
            BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " d "
            "WHERE m.ModeledElement.Id = d.ECInstanceId AND a.Model.Id = m.ECInstanceId ");
        BeAssert(stmt.IsPrepared());
        DbResult stepResult = stmt.Step();
        BeAssert(stepResult == DbResult::BE_SQLITE_ROW || stepResult == DbResult::BE_SQLITE_DONE);
        int nm = stepResult == DbResult::BE_SQLITE_ROW ? stmt.GetValueInt(0) : 0;
        if (nm != alignmentCount)
            BeAssert(false == nm && "Alignment Count failed.");
        stmt.Finalize();
        }

    if (corridorCount >= 0)
        {
        ECSqlStatement stmt;
        stmt.Prepare(*dgnDbPtr, "SELECT COUNT(*) FROM "
            BRRP_SCHEMA(BRRP_CLASS_Corridor) " c ");
        BeAssert(stmt.IsPrepared());
        DbResult stepResult = stmt.Step();
        BeAssert(stepResult == DbResult::BE_SQLITE_ROW || stepResult == DbResult::BE_SQLITE_DONE);
        int nm = stepResult == DbResult::BE_SQLITE_ROW ? stmt.GetValueInt(0) : 0;
        if (nm != corridorCount)
            BeAssert(false && "Corridor Count failed.");
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

    BeAssert(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    ECSqlStatement stmt;
    stmt.Prepare(*dgnDbPtr, "SELECT a.ECInstanceId, a.UserLabel FROM "
        BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " d "
        "WHERE m.ModeledElement.Id = d.ECInstanceId AND a.Model.Id = m.ECInstanceId ");
    BeAssert(stmt.IsPrepared());

    int count = 0;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        auto alignmentId = stmt.GetValueId<DgnElementId>(0);
        auto alignmentNameCP = stmt.GetValueText(1);
        if (alignmentName == nullptr || alignmentNameCP == nullptr || 0 != Utf8String(alignmentName).CompareTo(alignmentNameCP))
            continue;
        count++;
        }

    BeAssert(1 == count && alignmentName);
    stmt.Finalize();

    return dgnDbPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesORDBridgeTestsFixture::VerifyConvertedGeometryTurnoutBranchCount(Utf8CP bimFileName, Utf8CP branchName, size_t branchCount)
    {
    // JGATODO Add Turnouts and branches to imodel ... just using Alignments for now

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    BeAssert(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    ECSqlStatement stmt;
    stmt.Prepare(*dgnDbPtr, "SELECT a.ECInstanceId, a.UserLabel FROM "
        BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " d "
        "WHERE m.ModeledElement.Id = d.ECInstanceId AND a.Model.Id = m.ECInstanceId ");
    BeAssert(stmt.IsPrepared());

    int count = 0;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        auto branchId = stmt.GetValueId<DgnElementId>(0);
        auto branchNameCP = stmt.GetValueText(1);
        if (branchName == nullptr || branchNameCP == nullptr || 0 != Utf8String(branchName).CompareTo(branchNameCP))
            continue;
        count++;
        }

    BeAssert(branchCount == count);
    stmt.Finalize();

    return dgnDbPtr;
    }

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
static bool checkCurveVectorElementCountAndEnds
(
    BentleyB0200::CurveVectorCR curves, bool isVertical, size_t& elementCount,
    BentleyB0200::DPoint3dCR beg,
    BentleyB0200::DPoint3dCR end,
    double tolerance
)
    {
    if (curves.empty())
        return false;

    BentleyB0200::DPoint3d cBeg, cEnd;
    curves.GetStartEnd(cBeg, cEnd);

    if (beg.DistanceXY(cBeg) > tolerance)
        return false;

    // JGATODO Z vs Y ...  Use 3d distance for vertical until get Diego fix
    //if (isVertical ? end.Distance(cEnd) > tolerance : end.DistanceXY(cEnd) > tolerance)
    //    return false; 
    if (end.DistanceXY(cEnd) > tolerance)
        return false;

    for (BentleyB0200::ICurvePrimitivePtr curve : curves)
        {
        if (!curve.IsValid())
            return false;

        switch (curve->GetCurvePrimitiveType())
            {
            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                BentleyB0200::DSegment3dCP segment = (BentleyB0200::DSegment3dCP) curve->GetLineCP();
                if (segment == nullptr)
                    return false;
                elementCount++;
                break;
                }

            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                BentleyB0200::DEllipse3dCP arc = (BentleyB0200::DEllipse3dCP) curve->GetArcCP();
                if (arc == nullptr)
                    return false;
                elementCount++;
                break;
                }

            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                {
                BentleyB0200::DSpiral2dPlacementCP spiralData = (BentleyB0200::DSpiral2dPlacementCP) curve->GetSpiralPlacementCP();
                if (spiralData == nullptr)
                    return false;
                elementCount++;
                break;
                }

            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                {
                BentleyB0200::MSBsplineCurveCP bcurve = (BentleyB0200::MSBsplineCurveCP) curve->GetProxyBsplineCurveCP();
                if (bcurve == nullptr || !isVertical)
                    return false;
                elementCount++;
                break;
                }

            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                {
                return checkCurveVectorElementCountAndEnds(*curve->GetChildCurveVectorCP(), isVertical, elementCount, beg, end, tolerance);
                }

            default:
                {
                BeAssert(false && "Unexpected entry in CurveVector.");
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
    BentleyB0200::DPoint3dCR hBeg,
    BentleyB0200::DPoint3dCR hEnd,
    int vElementCount,
    BentleyB0200::DPoint3dCR vBeg,
    BentleyB0200::DPoint3dCR vEnd
)
    {
    const double tolerance = 1.0E-8;

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    BeAssert(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    ECSqlStatement stmt;
    stmt.Prepare(*dgnDbPtr, "SELECT a.ECInstanceId, a.UserLabel FROM "
        BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " d "
        "WHERE m.ModeledElement.Id = d.ECInstanceId AND a.Model.Id = m.ECInstanceId ");
    BeAssert(stmt.IsPrepared());

    bool found = false;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        auto alignmentId = stmt.GetValueId<DgnElementId>(0);
        auto alignmentNameCP = stmt.GetValueText(1);

        if (alignmentName == nullptr || alignmentNameCP == nullptr || 0 != Utf8String(alignmentName).CompareTo(alignmentNameCP))
            continue;

        found = true;

        auto roadRailAlignmentCPtr = BentleyB0200::RoadRailAlignment::Alignment::Get(*dgnDbPtr, alignmentId);
        auto alignmentPairPtr = roadRailAlignmentCPtr->QueryMainPair();

        auto horizontalAlignmentCPtr = roadRailAlignmentCPtr->GetHorizontal();
        if (horizontalAlignmentCPtr.IsValid())
            {
            auto curveVectorCR = horizontalAlignmentCPtr->GetGeometry();
            size_t elementCountCheck = 0;

            bool verified = checkCurveVectorElementCountAndEnds(curveVectorCR, false, elementCountCheck, hBeg, hEnd, tolerance);
            if (!verified)
                BeAssert(verified && "Horizontal Check Ends Verification failed.");

            bool checkCount = elementCountCheck == hElementCount;
            if (!checkCount)
                BeAssert(checkCount && "Horizontal Check Element Count failed.");
            }

        auto verticalAlignmentCPtr = roadRailAlignmentCPtr->GetMainVertical();
        if (verticalAlignmentCPtr.IsValid() && horizontalAlignmentCPtr.IsValid())
            {
            auto curveVectorPtr = verticalAlignmentCPtr->GetGeometry();
            size_t elementCountCheck = 0;

            bool verified = checkCurveVectorElementCountAndEnds(*curveVectorPtr, true, elementCountCheck, vBeg, vEnd, tolerance);
            if (!verified)
                BeAssert(verified && "Vertical Check Ends Verification failed.");

            bool checkCount = elementCountCheck == vElementCount;
            if (!checkCount)
                BeAssert(checkCount && "Vertical Check Element Count failed.");
            }
        }

    if (!found)
        BeAssert(found && alignmentName);

    stmt.Finalize();

    return dgnDbPtr;
    }

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
static bool checkCurveVectorElementLengths
(
    BentleyB0200::CurveVectorCR curves, bool isVertical, double& lengthCheckSum,
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

    for (BentleyB0200::ICurvePrimitivePtr curve : curves)
        {
        if (!curve.IsValid())
            return false;

        switch (curve->GetCurvePrimitiveType())
            {
            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                BentleyB0200::DSegment3dCP segment = (BentleyB0200::DSegment3dCP) curve->GetLineCP();
                double length = segment->Length();
                double lenCmp = isVertical ? vLineLength : hLineLength;
                lengthCheckSum -= length;

                if (checkExactElementLengthValues)
                    {
                    if (isVertical)
                        {
                        BentleyB0200::DPoint3d beg, end;
                        segment->GetEndPoints(beg, end);
                        length = end.x - beg.x;
                        }

                    if (fabs(lenCmp - length) > tolerance)
                        return false;
                    }
                break;
                }

            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                BentleyB0200::DEllipse3dCP arc = (BentleyB0200::DEllipse3dCP) curve->GetArcCP();
                double length = arc->ArcLength();
                double lenCmp = isVertical ? vArcLength : hArcLength;
                lengthCheckSum -= length;

                if (checkExactElementLengthValues)
                    {
                    if (isVertical)
                        {
                        BentleyB0200::DPoint3d beg, end;
                        BentleyB0200::DVec3d begTangent, endTangent;
                        curve->GetStartEnd(beg, end, begTangent, endTangent);
                        length = end.x - beg.x;
                        }

                    if (fabs(lenCmp - length) > tolerance)
                        return false;
                    }
                break;
                }

            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                {
                BentleyB0200::DSpiral2dPlacementCP spiralData = (BentleyB0200::DSpiral2dPlacementCP) curve->GetSpiralPlacementCP();
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

            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                {
                BentleyB0200::MSBsplineCurveCP bcurve = (BentleyB0200::MSBsplineCurveCP) curve->GetProxyBsplineCurveCP();

                size_t count = bcurve->GetNumPoles();
                if (3 != count)
                    return false;

                bvector<DPoint3d> poles;
                bcurve->GetPoles(poles);

                BentleyB0200::DPoint3d pvc, pvi, pvt;
                // JGATODO Z vs Y
                //pvc = BentleyB0200::DPoint3d::From(poles[0].x, 0.0, poles[0].z);
                //pvi = BentleyB0200::DPoint3d::From(poles[1].x, 0.0, poles[1].z);
                //pvt = BentleyB0200::DPoint3d::From(poles[2].x, 0.0, poles[2].z);
                pvc = BentleyB0200::DPoint3d::From(poles[0].x, poles[0].y, 0.0);
                pvi = BentleyB0200::DPoint3d::From(poles[1].x, poles[1].y, 0.0);
                pvt = BentleyB0200::DPoint3d::From(poles[2].x, poles[2].y, 0.0);

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

            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
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
                BeAssert(false && "Unexpected entry in CurveVector.");
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

    BeAssert(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    ECSqlStatement stmt;
    stmt.Prepare(*dgnDbPtr, "SELECT a.ECInstanceId, a.UserLabel FROM "
        BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " d "
        "WHERE m.ModeledElement.Id = d.ECInstanceId AND a.Model.Id = m.ECInstanceId ");
    BeAssert(stmt.IsPrepared());

    bool found = false;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        auto alignmentId = stmt.GetValueId<DgnElementId>(0);
        auto alignmentNameCP = stmt.GetValueText(1);

        if (alignmentName == nullptr || alignmentNameCP == nullptr || 0 != Utf8String(alignmentName).CompareTo(alignmentNameCP))
            continue;

        found = true;

        auto roadRailAlignmentCPtr = BentleyB0200::RoadRailAlignment::Alignment::Get(*dgnDbPtr, alignmentId);
        auto alignmentPairPtr = roadRailAlignmentCPtr->QueryMainPair();

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
                BeAssert(verified && "Horizontal Length Verification failed.");
            if (!checkSum)
                BeAssert(checkSum && "Horizontal Length Checksum failed.");
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
                BeAssert(verified && "Vertical Length Verification failed.");
            if (!checkSum)
                BeAssert(checkSum && "Vertical Length Checksum failed.");
            }
        }
    stmt.Finalize();

    if (!found)
        BeAssert(found && alignmentName);

    return dgnDbPtr;
    }

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
static bool checkCurveVectorSpiralTypesAndLengths(BentleyB0200::CurveVectorCR curves, bool isVertical, double tolerance)
    {
    //JGATODO Spirals

    if (curves.empty())
        return false;

    for (BentleyB0200::ICurvePrimitivePtr curve : curves)
        {
        if (!curve.IsValid())
            return false;

        switch (curve->GetCurvePrimitiveType())
            {
            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                break;

            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                break;

            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                {
                BentleyB0200::DSpiral2dPlacementCP spiralData = (BentleyB0200::DSpiral2dPlacementCP) curve->GetSpiralPlacementCP();
                double length = spiralData->SpiralLengthActiveInterval();

                if (fabs(length) < tolerance)
                    return false;
                break;
                }
                //case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                //case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
                //case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                //case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                //case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:

            case BentleyB0200::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                {
                /// Nested curve vector can occur with BOUNDARY_TYPE_None and not just union/parity regions...
                return checkCurveVectorSpiralTypesAndLengths(*curve->GetChildCurveVectorCP(), isVertical, tolerance);
                }

            default:
                {
                BeAssert(false && "Unexpected entry in CurveVector.");
                return false;
                }
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Greg.Ashe       08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr CiviliModelBridgesORDBridgeTestsFixture::VerifyConvertedGeometrySpiralTypesAndLengths(Utf8CP bimFileName)
    {
    // JGATODO Spirals Need later or newer Geomlib NOT bim02

    const double tolerance = 1.0E-8;

    BeFileName outputPath = m_host->GetOutputDirectory();
    outputPath.AppendA(bimFileName);

    DbResult result;
    DgnDbPtr dgnDbPtr = DgnDb::OpenDgnDb(&result, outputPath, DgnDb::OpenParams(Db::OpenMode::Readonly));

    BeAssert(dgnDbPtr.IsValid() && "OpenDgnDb failed.");
    if (dgnDbPtr.IsNull())
        return nullptr;

    ECSqlStatement stmt;
    stmt.Prepare(*dgnDbPtr, "SELECT a.ECInstanceId, a.UserLabel FROM "
        BRRA_SCHEMA(BRRA_CLASS_Alignment) " a," BIS_SCHEMA(BIS_CLASS_Model) " m, " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " d "
        "WHERE m.ModeledElement.Id = d.ECInstanceId AND a.Model.Id = m.ECInstanceId ");
    BeAssert(stmt.IsPrepared());

    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        auto alignmentId = stmt.GetValueId<DgnElementId>(0);
        auto alignmentNameCP = stmt.GetValueText(1);

        auto roadRailAlignmentCPtr = BentleyB0200::RoadRailAlignment::Alignment::Get(*dgnDbPtr, alignmentId);
        auto alignmentPairPtr = roadRailAlignmentCPtr->QueryMainPair();

        auto horizontalAlignmentCPtr = roadRailAlignmentCPtr->GetHorizontal();
        if (horizontalAlignmentCPtr.IsValid())
            {
            auto curveVectorCR = horizontalAlignmentCPtr->GetGeometry();
            double length = curveVectorCR.Length();

            //Clothoid
            //Biquadratic
            //Bloss
            //Sinusoid
            //Cosine
            //Chinese Cubic
            //Czech Cubic
            //Japanese Sine
            //Italian Cubic
            //Polish Cubic
            //Arema
            //NSW Cubic
            //WA Cubic
            //MX Cubic

            // All Lengths are 200 ... verify type and length of each

            bool verified = checkCurveVectorSpiralTypesAndLengths(curveVectorCR, false, tolerance);

            if (!verified)
                BeAssert(verified && "Horizontal Spiral Type and Length Verification failed.");
            }
        }
    stmt.Finalize();

    return dgnDbPtr;
    }
