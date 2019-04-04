/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/GeometryTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids/GridsApi.h>
#include <Bentley/BeTest.h>
#include <DgnView\DgnViewLib.h>
#include <DgnPlatform\DesktopTools\KnownDesktopLocationsAdmin.h>
#include <DgnPlatform\GenericDomain.h>
#include <BuildingShared\DgnUtils\BuildingUtils.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_DGNCLIENTFX
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_GRIDS
USING_NAMESPACE_BUILDING_SHARED

#pragma region TestHost

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestHost : DgnViewLib::Host
    {
    private:
        TestHost();

    public:
        static TestHost& Instance();
        virtual ~TestHost();

        BeFileName GetBaseDbPath() const { return s_baseDbPath; }

    protected:
        virtual void _SupplyProductName (BentleyApi::Utf8StringR name) override;
        virtual NotificationAdmin& _SupplyNotificationAdmin() override;
        virtual Dgn::ViewManager& _SupplyViewManager() override;
        virtual BentleyApi::BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;
        virtual IKnownLocationsAdmin& _SupplyIKnownLocationsAdmin() override;

    private:
        static BeFileName s_baseDbPath;
    };

BeFileName TestHost::s_baseDbPath;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConverterViewManager : ViewManager
    {
    protected:
        virtual Display::SystemContext* _GetSystemContext() override { return nullptr; }
        virtual bool _DoesHostHaveFocus() override { return true; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName getOutputDirectory()
    {
    BeFileName outputDirectory;
    BeTest::GetHost().GetTempDir (outputDirectory);

    if (!BeFileName::DoesPathExist (outputDirectory.c_str()))
        BeFileName::CreateNewDirectory (outputDirectory.c_str());

    return outputDirectory;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbPtr createDgnDb (BeFileName const& bimFilename)
    {
    CreateDgnDbParams createProjectParams;
    createProjectParams.SetOverwriteExisting (true);
    createProjectParams.SetRootSubjectName ("GridsTests");
    createProjectParams.SetRootSubjectDescription ("Tests for Grids domain");
    createProjectParams.SetOpenMode (Db::OpenMode::ReadWrite);
    createProjectParams.SetDbType (Db::CreateParams::DbType::Standalone);

    DbResult status = BeSQLite::DbResult::BE_SQLITE_ERROR;
    DgnDbPtr db = DgnDb::CreateDgnDb (&status, bimFilename, createProjectParams);
    BeAssert (status == BE_SQLITE_OK);

    return db;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TestHost::TestHost()
    {
    DgnViewLib::Initialize (*this, true);

    BentleyStatus registrationStatus = DgnDomains::RegisterDomain (GridsDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    BeAssert (BentleyStatus::SUCCESS == registrationStatus);

    BeFileName::EmptyDirectory (getOutputDirectory());

    DgnDbPtr dbPtr = createDgnDb (getOutputDirectory().AppendToPath (L"GridsTests"));
    BeAssert (dbPtr.IsValid());

    // Get fully qualified path (including db extension)
    s_baseDbPath = dbPtr->GetFileName();
    dbPtr->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TestHost::~TestHost()
    {
    Terminate (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TestHost& TestHost::Instance()
    {
    static TestHost testHost;
    return testHost;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void TestHost::_SupplyProductName (BentleyApi::Utf8StringR name)
    {
    name.assign ("GridsDomainTests");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::NotificationAdmin& TestHost::_SupplyNotificationAdmin()
    {
    return *new DgnPlatformLib::Host::NotificationAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct GridsViewManager : ViewManager
    {
    protected:
        virtual Display::SystemContext* _GetSystemContext() override { return nullptr; }
        virtual bool _DoesHostHaveFocus() override { return true; }
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ViewManager& TestHost::_SupplyViewManager()
    {
    return *new GridsViewManager();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
L10N::SqlangFiles TestHost::_SupplySqlangFiles()
    {
    BentleyApi::BeFileName sqlangFile (GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath (L"sqlang/GridsTests_en-US.sqlang.db3");
    //BeAssert (sqlangFile.DoesPathExist());

    return L10N::SqlangFiles (sqlangFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewLib::Host::IKnownLocationsAdmin& TestHost::_SupplyIKnownLocationsAdmin()
    {
    return *new KnownDesktopLocationsAdmin();
    }

#pragma endregion TestHost

//=======================================================================================
// Tests for visualizing grids
// @bsiclass                                                                     03/2019
//=======================================================================================
struct GeometryTests : testing::Test
    {
    private:
        DgnDbPtr m_dbPtr;
        SpatialLocationModelPtr m_model;
        Dgn::PhysicalModelPtr m_physicalModelPtr;
        DgnCategoryId m_categoryId;
        Render::GeometryParams m_shapeGeometryParams;

    protected:
        GeometryTests();
        virtual ~GeometryTests();

        void SetUpForGeometryPresentation();
        void InsertPhysicalCurveElement (GridCurvePtr curvePtr);

        DgnDb& GetDb();
        SpatialLocationModel& GetModel();
        PhysicalModel& GetPhysicalModel() { return *m_physicalModelPtr; }
        DgnCategoryId GetCategoryId();
        Render::GeometryParams& GetGeometryParams();

        ElevationGridPtr InsertElevationGrid (DPoint3d origin, double length, double width,  double elevationIncrement, int count, Utf8CP gridName);
        OrthogonalGridPtr InsertOrthogonalGrid (DPoint3d origin, double incrementX, double incrementY, double minExtentX, double maxExtentX,
                                                double minExtentY, double maxExtentY, double elevation, int countX, int countY, Utf8CP gridName);
        RadialGridPtr InsertRadialGrid (DPoint3d origin, Angle angleIncrement, double radiusIncrement, Angle startAngle, Angle endAngle,
                                        double startRadius, double endRadius, double elevation, int radialCount, int circularCount, Utf8CP gridName);

        bvector<GridCurvePtr> InsertIntersectionCurves (ElevationGridPtr elevationGrid, GridPtr otherGrid);

        /*---------------------------------------------------------------------------------**//**
        * Create and insert a DgnModel.
        * @bsimethod                                                                     03/2019
        +---------------+---------------+---------------+---------------+---------------+------*/
        template<typename T_Model, typename T_Partition>
        RefCountedPtr<T_Model> InsertDgnModel (Utf8CP pPartitionName)
            {
            Dgn::SubjectCPtr rootSubjectPtr = GetDb().Elements().GetRootSubject();

            RefCountedPtr<T_Partition> partitionPtr = typename T_Partition::Create (*rootSubjectPtr, pPartitionName);
            GetDb().BriefcaseManager().AcquireForElementInsert (*partitionPtr);

            Dgn::DgnDbStatus status;
            partitionPtr->Insert (&status);
            if (status != Dgn::DgnDbStatus::Success)
                return nullptr;

            RefCountedPtr<T_Model> modelPtr = typename T_Model::Create (*partitionPtr);
            status = modelPtr->Insert();
            if (status != Dgn::DgnDbStatus::Success)
                return nullptr;

            return modelPtr;
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName copyWorkingDb()
    {
    BeFileName baseDbPath = TestHost::Instance().GetBaseDbPath();
    WString directory = baseDbPath.GetDirectoryName();
    WString name = baseDbPath.GetFileNameWithoutExtension();
    WString extension = baseDbPath.GetExtension();

    BeFileName workingDbPath (directory + name + L"_Working." + extension);

    BeFileNameStatus fileCopyStatus = BeFileName::BeCopyFile (baseDbPath, workingDbPath);
    BeAssert (fileCopyStatus == BeFileNameStatus::Success);

    return workingDbPath;
    }

#pragma region GeometryTests Methods

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryTests::SetUpForGeometryPresentation()
    {
    m_physicalModelPtr = InsertDgnModel<PhysicalModel, PhysicalPartition> ("GridsTestPartition_Physical");
    BeAssert (m_physicalModelPtr.IsValid());

    // Create default model and partition for grid elements
    m_dbPtr->BriefcaseManager().StartBulkOperation();
    SubjectCPtr rootSubject = m_dbPtr->Elements().GetRootSubject();
    SpatialLocationPartitionPtr partition = SpatialLocationPartition::Create (*rootSubject, "GridSpatialPartition");
    m_dbPtr->Elements().Insert<SpatialLocationPartition> (*partition);
    m_model = SpatialLocationModel::CreateAndInsert (*partition);
    m_dbPtr->SaveChanges();

    // Create a SpatialCategory used for creating Elements
    SpatialCategory category (GetDb().GetDictionaryModel(), "GridTestCategory", DgnCategory::Rank::Application);

    DgnDbStatus status;
    SpatialCategoryCPtr categoryPtr = category.Insert (DgnSubCategory::Appearance(), &status);
    BeAssert (status == DgnDbStatus::Success);
    m_categoryId = categoryPtr->GetCategoryId();

    m_shapeGeometryParams.SetCategoryId (m_categoryId);
    m_shapeGeometryParams.SetFillDisplay (Render::FillDisplay::Always);
    m_shapeGeometryParams.SetLineColor (ColorDef::ColorDef (66, 134, 244));
    m_shapeGeometryParams.SetFillColor (ColorDef::ColorDef (66, 134, 244));
    m_shapeGeometryParams.SetWeight (1);

    DefinitionModelR dictionaryModel = GetDb().GetDictionaryModel();
    CategorySelectorPtr categorySelectorPtr = new CategorySelector (dictionaryModel, "GridsTest");
    categorySelectorPtr->AddCategory (m_categoryId);

    ModelSelectorPtr modelSelectorPtr = new ModelSelector (dictionaryModel, "GridsTest");
    modelSelectorPtr->AddModel (GetModel().GetModelId());
    modelSelectorPtr->AddModel (GetPhysicalModel().GetModelId());

    DisplayStyle3dPtr displayStylePtr = new DisplayStyle3d (dictionaryModel, "GridsTest");
    displayStylePtr->SetBackgroundColor (ColorDef::LightGrey());
    displayStylePtr->SetSkyBoxEnabled (false);
    displayStylePtr->SetGroundPlaneEnabled (false);

    Render::ViewFlags viewFlags = displayStylePtr->GetViewFlags();
    viewFlags.SetRenderMode (Render::RenderMode::Wireframe);
    viewFlags.SetShowTransparency (false);
    viewFlags.SetShowGrid (true);
    viewFlags.SetShowAcsTriad (true);
    displayStylePtr->SetViewFlags (viewFlags);

    OrthographicViewDefinition view (dictionaryModel, "Grids View", *categorySelectorPtr, *displayStylePtr, *modelSelectorPtr);
    view.SetCategorySelector (*categorySelectorPtr);
    view.SetStandardViewRotation (StandardView::Top);
    view.LookAtVolume (GetDb().GeoLocation().GetProjectExtents());
    view.Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryTests::GeometryTests()
    : m_dbPtr (nullptr)
    , m_model (nullptr)
    {
    // Copy and open a fresh DB
    BeFileName workingDbPath = copyWorkingDb();

    DgnDb::OpenParams openParams (BeSQLite::Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade);
    BeSQLite::DbResult openStatus;
    m_dbPtr = DgnDb::OpenDgnDb (&openStatus, workingDbPath, openParams);
    BeAssert (m_dbPtr.IsValid());

    SetUpForGeometryPresentation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryTests::~GeometryTests()
    {
    m_dbPtr->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDb& GeometryTests::GetDb()
    {
    BeAssert (m_dbPtr.IsValid());
    return *m_dbPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationModel& GeometryTests::GetModel()
    {
    BeAssert (m_model.IsValid());
    return *m_model;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId GeometryTests::GetCategoryId()
    {
    BeAssert (m_categoryId.IsValid());
    return m_categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GeometryParams& GeometryTests::GetGeometryParams()
    {
    return m_shapeGeometryParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryTests::InsertPhysicalCurveElement (GridCurvePtr curvePtr)
    {
    PhysicalElementPtr physicalElementPtr = GenericPhysicalObject::Create (GetPhysicalModel(), GetCategoryId());
    physicalElementPtr->SetUserLabel (curvePtr->GetUserLabel());
    physicalElementPtr->SetPlacement (curvePtr->GetPlacement());

    GeometrySource* pGeometrySource = physicalElementPtr->ToGeometrySourceP();
    GeometryBuilderPtr builder = GeometryBuilder::Create (*pGeometrySource);
    builder->Append (GetGeometryParams());

    GeometryCollection geomData = *curvePtr->ToGeometrySource();
    for (auto itGeometricPrimitive : geomData)
        {
        GeometricPrimitivePtr geometricPrimitivePtr = itGeometricPrimitive.GetGeometryPtr();
        ASSERT_TRUE (geometricPrimitivePtr.IsValid());
        builder->Append (*geometricPrimitivePtr);
        }
    
    builder->Finish (*pGeometrySource);

    DgnDbStatus insertStatus;
    physicalElementPtr->Insert (&insertStatus);
    ASSERT_TRUE (insertStatus == DgnDbStatus::Success);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ElevationGridPtr GeometryTests::InsertElevationGrid (DPoint3d origin, double length, double width, double elevationIncrement, int count, Utf8CP gridName)
    {
    bvector<DPoint3d> baseShape = { {0, 0, 0}, {length, 0, 0}, {length, width, 0}, {0, width, 0}, {0, 0, 0} };
    bvector<CurveVectorPtr> floorPlaneCurves = bvector<CurveVectorPtr> (count);
    int gridIteration = 0;
    for (CurveVectorPtr& curveShape : floorPlaneCurves)
        {
        bvector<DPoint3d> thisShape = baseShape;
        std::transform (thisShape.begin(), thisShape.end(), thisShape.begin(), [&] (DPoint3d point) -> DPoint3d {point.z = elevationIncrement * gridIteration; return point; });
        curveShape = CurveVector::CreateLinear (thisShape, CurveVector::BOUNDARY_TYPE_Outer);
        ++gridIteration;
        }

    ElevationGridPtr grid = ElevationGrid::CreateAndInsertWithSurfaces (ElevationGrid::CreateParams (GetModel(),
        GetDb().Elements().GetRootSubject()->GetElementId(),
        gridName),
        floorPlaneCurves);

    Placement3d gridPlacement = grid->GetPlacement();
    gridPlacement.SetOrigin (origin);

    grid->SetPlacement (gridPlacement);
    grid->Update();
    return grid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGridPtr GeometryTests::InsertOrthogonalGrid (DPoint3d origin, 
                                                        double incrementX, double incrementY, 
                                                        double minExtentX, double maxExtentX, 
                                                        double minExtentY, double maxExtentY, 
                                                        double elevation, 
                                                        int countX, int countY,
                                                        Utf8CP gridName)
    {
    OrthogonalGrid::CreateParams orthogonalParams = OrthogonalGrid::CreateParams (GetModel(),
        GetDb().Elements().GetRootSubject()->GetElementId(), /*parent element*/
        gridName,
        incrementX, /*defaultCoordIncX*/
        incrementY, /*defaultCoordIncY*/
        minExtentX, /*defaultStaExtX*/
        maxExtentX, /*defaultEndExtX*/
        minExtentY, /*defaultStaExtY*/
        maxExtentY, /*defaultEndExtY*/
        -2 * BUILDING_TOLERANCE, /*defaultStaElevation*/
        elevation + 2 * BUILDING_TOLERANCE /*defaultEndElevation*/
    );

    OrthogonalGridPtr grid = OrthogonalGrid::CreateAndInsertWithSurfaces (orthogonalParams, countX, countY);
    
    Placement3d gridPlacement = grid->GetPlacement();
    gridPlacement.SetOrigin (origin);

    grid->SetPlacement (gridPlacement);
    grid->Update();
    return grid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
RadialGridPtr GeometryTests::InsertRadialGrid (DPoint3d origin,
                                               Angle angleIncrement, double radiusIncrement,
                                               Angle startAngle, Angle endAngle,
                                               double startRadius, double endRadius,
                                               double elevation,
                                               int radialCount, int circularCount,
                                               Utf8CP gridName)
    {
    RadialGrid::CreateParams createParams = RadialGrid::CreateParams (GetModel(),
        GetDb().Elements().GetRootSubject()->GetElementId(), /*scope element*/
        gridName,   /*name*/
        angleIncrement.Radians(), /*defaultAngleIncrement*/
        radiusIncrement, /*defaultRadiusIncrement*/
        startAngle.Radians(), /*defaultStartAngle*/
        endAngle.Radians(), /*defaultEndAngle*/
        startRadius, /*defaultStartRadius*/
        endRadius, /*defaultEndRadius*/
        -2 * BUILDING_TOLERANCE, /*defaultstaElevation*/
        elevation + 2 * BUILDING_TOLERANCE /*defaultendElevation*/
    );

    RadialGridPtr grid = RadialGrid::CreateAndInsertWithSurfaces(createParams, radialCount, circularCount);

    Placement3d gridPlacement = grid->GetPlacement();
    gridPlacement.SetOrigin (origin);

    grid->SetPlacement (gridPlacement);
    grid->Update();
    return grid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<GridCurvePtr> GeometryTests::InsertIntersectionCurves (ElevationGridPtr elevationGrid, GridPtr otherGrid)
    {
    bvector<GridCurvePtr> curves;

    GridCurvesSetPtr curvesPortion = GridCurvesSet::Create (GetModel());
    curvesPortion->Insert();

    for (DgnElementId elevationSurfaceId : elevationGrid->GetSurfacesModel()->MakeIterator().BuildIdList<DgnElementId>())
        {
        GridPlanarSurfaceCPtr floorGridSurface = GridPlanarSurface::Get (GetDb(), elevationSurfaceId);
        BeAssert (BentleyStatus::SUCCESS == otherGrid->IntersectGridSurface (floorGridSurface.get(), *curvesPortion));
        GetDb().SaveChanges();

        for (DgnElementId curveBundleId : floorGridSurface->MakeGridCurveBundleIterator().BuildIdList<DgnElementId>())
            {
            GridCurveBundlePtr curveBundle = GridCurveBundle::GetForEdit (GetDb(), curveBundleId);
            GridCurveCPtr gridCurveReadOnly = curveBundle->GetGridCurve();
            if (gridCurveReadOnly.IsNull())
                continue;

            curves.push_back(dynamic_cast<GridCurve*>(gridCurveReadOnly->CopyForEdit().get()));
            }
        }

    return curves;
    }

#pragma endregion GeometryTests Methods

//---------------------------------------------------------------------------------------
// @betest                                                                       03/2019
//--------------+---------------+---------------+---------------+---------------+-------- 
TEST_F (GeometryTests, CreateGridsGeometry)
    {
        { // orthogonal grid without extents
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (0, 0, 5), 40, 40, 0, 1, "Elevation-O0");
        OrthogonalGridPtr orthogonal = InsertOrthogonalGrid (DPoint3d::From (0, 0, 0), 10, 5, 0, 15, 0, 30, 10, 4, 4, "Orthogonal-Basic");
        bvector<GridCurvePtr> curves = InsertIntersectionCurves (elevation, orthogonal);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            curve->SetUserLabel (Utf8PrintfString("%d", curveNo++).c_str());
            curve->SetBubbleAtStart (curveNo % 2 != 0);
            curve->SetBubbleAtEnd (curveNo % 3 != 0);
            curve->Update();
            InsertPhysicalCurveElement (curve);
            }
        }

        { // orthogonal grid with extents
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (35, -5, 5), 40, 40, 0, 1, "Elevation-O1");
        OrthogonalGridPtr orthogonal = InsertOrthogonalGrid (DPoint3d::From (40, 0, 0), 10, 5, -1, 16, -1, 31, 10, 4, 4, "Orthogonal-Extended");
        bvector<GridCurvePtr> curves = InsertIntersectionCurves (elevation, orthogonal);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            curve->SetUserLabel (Utf8PrintfString ("%d", curveNo++).c_str());
            curve->SetBubbleAtStart (curveNo % 2 != 0);
            curve->SetBubbleAtEnd (curveNo % 3 != 0);
            curve->Update();
            InsertPhysicalCurveElement (curve);
            }
        }

        { // orthogonal grid altered individual extents
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (75, -5, 5), 40, 40, 0, 1, "Elevation-O2");
        OrthogonalGridPtr orthogonal = InsertOrthogonalGrid (DPoint3d::From (80, 0, 0), 10, 5, 0, 15, 0, 30, 10, 4, 4, "Orthogonal-CustomExtended");
        for (DgnElementId axisId : orthogonal->MakeAxesIterator().BuildIdList<DgnElementId>())
            {
            GridAxisCPtr axis = GridAxis::Get (GetDb(), axisId);
            bvector<DgnElementId> surfacesIds = axis->MakeIterator().BuildIdList<DgnElementId>();
            bvector<PlanCartesianGridSurfacePtr> surfaces = bvector<PlanCartesianGridSurfacePtr> (4);
            std::transform (surfacesIds.begin(), surfacesIds.end(), surfaces.begin(), [this] (DgnElementId elementId) -> PlanCartesianGridSurfacePtr {return PlanCartesianGridSurface::GetForEdit (GetDb(), elementId); });
            
            surfaces[0]->SetStartExtent (-5);
            surfaces[0]->SetEndExtent (35);
            surfaces[0]->Update();

            surfaces[1]->SetStartExtent (0);
            surfaces[1]->SetEndExtent (30);
            surfaces[1]->Update();

            surfaces[2]->SetStartExtent (-5);
            surfaces[2]->SetEndExtent (25);
            surfaces[2]->Update();

            surfaces[3]->SetStartExtent (5);
            surfaces[3]->SetEndExtent (20);
            surfaces[3]->Update();
            }
        bvector<GridCurvePtr> curves = InsertIntersectionCurves (elevation, orthogonal);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            curve->SetUserLabel (Utf8PrintfString ("%d", curveNo++).c_str());
            curve->SetBubbleAtStart (curveNo % 2 != 0);
            curve->SetBubbleAtEnd (curveNo % 3 != 0);
            curve->Update();
            InsertPhysicalCurveElement (curve);
            }
        }

        { // radial grid without extents
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (0, 40, 5), 40, 40, 0, 1, "Elevation-R0");
        RadialGridPtr radial = InsertRadialGrid (DPoint3d::From (0, 40, 0), Angle::FromDegrees (30), 5, Angle::FromDegrees (0), Angle::FromDegrees (90), 0, 30, 10, 4, 6, "Radial-Basic");
        bvector<GridCurvePtr> curves = InsertIntersectionCurves (elevation, radial);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            curve->SetUserLabel (Utf8PrintfString ("%04d", curveNo++).c_str());
            curve->SetBubbleAtStart (curveNo % 2 != 0);
            curve->SetBubbleAtEnd (curveNo % 3 != 0);
            curve->Update();
            InsertPhysicalCurveElement (curve);
            }
        }

        { // radial grid with extents
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (35, 35, 5), 40, 40, 0, 1, "Elevation-R1");
        RadialGridPtr radial = InsertRadialGrid (DPoint3d::From (40, 40, 0), Angle::FromDegrees (30), 5, Angle::FromDegrees (-10), Angle::FromDegrees (100), -5, 35, 10, 4, 6, "Radial-Extended");
        bvector<GridCurvePtr> curves = InsertIntersectionCurves (elevation, radial);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            curve->SetUserLabel (Utf8PrintfString ("%d", curveNo++).c_str());
            curve->SetBubbleAtStart (curveNo % 2 != 0);
            curve->SetBubbleAtEnd (curveNo % 3 != 0);
            curve->Update();
            InsertPhysicalCurveElement (curve);
            }
        }

        { // radial grid altered individual extents
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (75, 35, 5), 40, 40, 0, 1, "Elevation-R2");
        RadialGridPtr radial = InsertRadialGrid (DPoint3d::From (80, 40, 0), Angle::FromDegrees (30), 5, Angle::FromDegrees (0), Angle::FromDegrees (90), 0, 30, 10, 4, 6, "Radial-CustomExtended");
        
        RadialAxisCPtr radialAxis = radial->GetRadialAxis();
        bvector<DgnElementId> radialSurfacesIds = radialAxis->MakeIterator().BuildIdList<DgnElementId>();
        bvector<PlanRadialGridSurfacePtr> radialSurfaces = bvector<PlanRadialGridSurfacePtr> (4);
        std::transform (radialSurfacesIds.begin(), radialSurfacesIds.end(), radialSurfaces.begin(), [this] (DgnElementId elementId) -> PlanRadialGridSurfacePtr {return PlanRadialGridSurface::GetForEdit (GetDb(), elementId); });
        radialSurfaces[0]->SetStartRadius (5);
        radialSurfaces[0]->SetEndRadius (35);
        radialSurfaces[0]->Update();

        radialSurfaces[1]->SetStartRadius (0);
        radialSurfaces[1]->SetEndRadius (30);
        radialSurfaces[1]->Update();

        radialSurfaces[2]->SetStartRadius (-5);
        radialSurfaces[2]->SetEndRadius (25);
        radialSurfaces[2]->Update();

        radialSurfaces[3]->SetStartRadius (-5);
        radialSurfaces[3]->SetEndRadius (20);
        radialSurfaces[3]->Update();

        CircularAxisCPtr circularAxis = radial->GetCircularAxis();
        bvector<DgnElementId> circularSurfacesIds = circularAxis->MakeIterator().BuildIdList<DgnElementId>();
        bvector<PlanCircumferentialGridSurfacePtr> circularSurfaces = bvector<PlanCircumferentialGridSurfacePtr> (6);
        std::transform (circularSurfacesIds.begin(), circularSurfacesIds.end(), circularSurfaces.begin(), [this] (DgnElementId elementId) -> PlanCircumferentialGridSurfacePtr {return PlanCircumferentialGridSurface::GetForEdit (GetDb(), elementId); });
        circularSurfaces[0]->SetStartAngle (Angle::DegreesToRadians(-10));
        circularSurfaces[0]->SetEndAngle (Angle::DegreesToRadians(100));
        circularSurfaces[0]->Update();

        circularSurfaces[1]->SetStartAngle (Angle::DegreesToRadians (0));
        circularSurfaces[1]->SetEndAngle (Angle::DegreesToRadians (90));
        circularSurfaces[1]->Update();

        circularSurfaces[2]->SetStartAngle (Angle::DegreesToRadians (-10));
        circularSurfaces[2]->SetEndAngle (Angle::DegreesToRadians (80));
        circularSurfaces[2]->Update();

        circularSurfaces[3]->SetStartAngle (Angle::DegreesToRadians (10));
        circularSurfaces[3]->SetEndAngle (Angle::DegreesToRadians (80));
        circularSurfaces[3]->Update();

        circularSurfaces[4]->SetStartAngle (Angle::DegreesToRadians (25));
        circularSurfaces[4]->SetEndAngle (Angle::DegreesToRadians (115));
        circularSurfaces[4]->Update();

        circularSurfaces[5]->SetStartAngle (Angle::DegreesToRadians (-30));
        circularSurfaces[5]->SetEndAngle (Angle::DegreesToRadians (75));
        circularSurfaces[5]->Update();

        bvector<GridCurvePtr> curves = InsertIntersectionCurves (elevation, radial);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            curve->SetUserLabel (Utf8PrintfString ("%d", curveNo++).c_str());
            curve->SetBubbleAtStart (curveNo % 2 != 0);
            curve->SetBubbleAtEnd (curveNo % 3 != 0);
            curve->Update();
            InsertPhysicalCurveElement (curve);
            }
        }

        { // spline grid
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (0, 80, 5), 40, 40, 0, 1, "Elevation-S0");
        SketchGridPtr sketchGrid = SketchGrid::Create (GetModel(), GetModel().GetModeledElementId(), "Sketch Grid", 0.0, 10.0);
        sketchGrid->Insert();

        Dgn::DgnModelCR defModel = BuildingUtils::GetGroupInformationModel (GetDb());
        Grids::GeneralGridAxisPtr gridAxis = GeneralGridAxis::CreateAndInsert (defModel, *sketchGrid);
        
        SketchLineGridSurface::CreateParams params (*sketchGrid->GetSurfacesModel().get(), *gridAxis, 0.0, 10, DPoint2d::From (0, 20), DPoint2d::From (0, 70));
        GridPlanarSurfacePtr plane = SketchLineGridSurface::Create (params);
        plane->Insert();

        SketchArcGridSurface::CreateParams arcParams (*sketchGrid->GetSurfacesModel().get(), *gridAxis, 0.0, 10, GeometryUtils::CreateArc (10, Angle::Pi(), 0));
        GridArcSurfacePtr arc = SketchArcGridSurface::Create (arcParams);
        arc->Insert();

        ICurvePrimitivePtr splinePrimitive = GeometryUtils::CreateSplinePrimitive ({ { 0, 0, 0 },{ 10, 0, 0 },{ 0, 10, 0 }, {10, 10, 0}, {20, 0, 0}, {0, 20, 0}, {20, 20, 0} });
        SketchSplineGridSurface::CreateParams splineParams (*sketchGrid->GetSurfacesModel().get(), *gridAxis, 0.0, 10.0, *splinePrimitive);
        GridSplineSurfacePtr spline = SketchSplineGridSurface::Create (splineParams);
        spline->Insert();

        Placement3d gridPlacement = sketchGrid->GetPlacement();
        gridPlacement.SetOrigin (DPoint3d::From (0, 80, 5));
        sketchGrid->SetPlacement (gridPlacement);
        sketchGrid->Update();

        GetDb().SaveChanges();

        bvector<GridCurvePtr> curves = InsertIntersectionCurves (elevation, sketchGrid);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            curve->SetUserLabel (Utf8PrintfString ("%04d", curveNo++).c_str());
            curve->SetBubbleAtStart (true);
            curve->SetBubbleAtEnd (true);
            curve->Update();
            InsertPhysicalCurveElement (curve);
            }
        }
    
    GetDb().SaveChanges();
    }
