/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/GeometryTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids/GridsApi.h>
#include <Bentley/BeTest.h>
#include <BuildingShared/DgnUtils/BuildingUtils.h>
#include "GridsTestFixtureBase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_GRIDS
USING_NAMESPACE_BUILDING_SHARED

#define BUBBLE_RADIUS 1.5

//=======================================================================================
// Tests for visualizing grids
// @bsiclass                                                                     04/2019
//=======================================================================================
struct GeometryTests : GridsTestFixtureBase
    {
    private:
        DgnDbPtr m_dbPtr;
        SpatialLocationModelPtr m_model;
        PhysicalModelPtr m_physicalModelPtr;
        DgnCategoryId m_categoryId;
        Render::GeometryParams m_shapeGeometryParams;

    protected:
        GeometryTests () = default;
        virtual ~GeometryTests () = default;

        PhysicalPartitionPtr SetUpPhysicalModelPartition () const;
        void SetUpPhysicalModel ();
        SpatialLocationPartitionPtr SetUpGridPartition () const;
        void SetUpGridModel ();
        void SetUpGridTestCategory ();
        void SetUpGeometryParams ();
        CategorySelectorPtr SetUpCategorySelector ( DictionaryModelR dictionaryModel ) const;
        ModelSelectorPtr SetUpModelSelector ( DictionaryModelR dictionaryModel ) const;
        Render::ViewFlags SetUpViewFlags ( DisplayStylePtr displayStylePtr ) const;
        DisplayStyle3dPtr SetUpDisplayStyle ( DictionaryModelR dictionaryModel ) const;
        void SetUpViewDefinition ( DictionaryModelR dictionaryModel,
                                   CategorySelectorR categorySelector,
                                   ModelSelectorR modelSelector,
                                   DisplayStyle3dR displayStyle ) const;
        void SetUpForGeometryPresentation ();
        void InsertPhysicalCurveElement ( GridCurveCR curve ) const;

        DgnDb& GetDb () const { return *m_dbPtr; }
        SpatialLocationModel& GetModel () const { return *m_model; }
        PhysicalModel& GetPhysicalModel () const { return *m_physicalModelPtr; }
        DgnCategoryId GetCategoryId () const { return m_categoryId; }
        Render::GeometryParamsCR GetGeometryParams () const { return m_shapeGeometryParams; }
        bool AppendGridBubbleGraphics ( GeometryBuilderR builder, GridCurveCR curve ) const;

        ElevationGridPtr InsertElevationGrid ( DPoint3d origin,
                                               double length,
                                               double width,
                                               double elevationIncrement,
                                               int count,
                                               Utf8CP gridName ) const;
        OrthogonalGridPtr InsertOrthogonalGrid ( DPoint3d origin,
                                                 double incrementX,
                                                 double incrementY,
                                                 double minExtentX,
                                                 double maxExtentX,
                                                 double minExtentY,
                                                 double maxExtentY,
                                                 double elevation,
                                                 int countX,
                                                 int countY,
                                                 Utf8CP gridName ) const;
        RadialGridPtr InsertRadialGrid ( DPoint3d origin,
                                         Angle angleIncrement,
                                         double radiusIncrement,
                                         Angle startAngle,
                                         Angle endAngle,
                                         double startRadius,
                                         double endRadius,
                                         double elevation,
                                         int radialCount,
                                         int circularCount,
                                         Utf8CP gridName ) const;

        bvector <GridCurvePtr> InsertIntersectionCurves ( ElevationGridCR elevationGrid, GridCR otherGrid ) const;

        bool CreateGridLabel ( GridCurveCR curve, Utf8CP label, bool atStart, bool atEnd ) const;

        virtual void SetUp () override;
        virtual void TearDown () override;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
static BeFileName GetWorkingDatabasePath ( BeFileName baseDbPath )
    {
    WString directory = baseDbPath.GetDirectoryName();
    WString name = baseDbPath.GetFileNameWithoutExtension();
    WString extension = baseDbPath.GetExtension();

    return BeFileName (directory + name + L"_Geometry." + extension);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
static BeFileName copyWorkingDb (BeFileName baseDbPath)
    {
    BeFileName workingDbPath = GetWorkingDatabasePath (baseDbPath);

    BeFileNameStatus fileCopyStatus = BeFileName::BeCopyFile (baseDbPath, workingDbPath);
    BeAssert (fileCopyStatus == BeFileNameStatus::Success);

    return workingDbPath;
    }

#pragma region GeometryTests Methods

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
PhysicalPartitionPtr GeometryTests::SetUpPhysicalModelPartition () const
    {
    SubjectCPtr rootSubjectPtr = GetDb().Elements().GetRootSubject();
    PhysicalPartitionPtr partitionPtr = PhysicalPartition::Create (*rootSubjectPtr, "GridsTestPartition_Physical");
    GetDb().BriefcaseManager().AcquireForElementInsert (*partitionPtr);
    DgnDbStatus status;
    partitionPtr->Insert (&status);
    BeAssert(DgnDbStatus::Success == status);
    return partitionPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
void GeometryTests::SetUpPhysicalModel ()
    {
    PhysicalPartitionCPtr partitionPtr = SetUpPhysicalModelPartition();
    m_physicalModelPtr = PhysicalModel::Create (*partitionPtr);
    BeAssert (m_physicalModelPtr.IsValid());
    DgnDbStatus status = m_physicalModelPtr->Insert();
    BeAssert (DgnDbStatus::Success == status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
SpatialLocationPartitionPtr GeometryTests::SetUpGridPartition () const
    {
    SubjectCPtr rootSubject = m_dbPtr->Elements().GetRootSubject();
    SpatialLocationPartitionPtr partition = SpatialLocationPartition::Create (*rootSubject, "GridSpatialPartition");
    m_dbPtr->Elements().Insert <SpatialLocationPartition> (*partition);
    return partition;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
void GeometryTests::SetUpGridModel ()
    {
    SpatialLocationPartitionCPtr partition = SetUpGridPartition();
    m_model = SpatialLocationModel::CreateAndInsert (*partition);
    m_dbPtr->SaveChanges();
    m_dbPtr->BriefcaseManager ().StartBulkOperation ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
void GeometryTests::SetUpGridTestCategory ()
    {
    SpatialCategory category (GetDb().GetDictionaryModel(), "GridTestCategory", DgnCategory::Rank::Application);

    DgnDbStatus status;
    SpatialCategoryCPtr categoryPtr = category.Insert (DgnSubCategory::Appearance(), &status);
    BeAssert (status == DgnDbStatus::Success);
    m_categoryId = categoryPtr->GetCategoryId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
void GeometryTests::SetUpGeometryParams ()
    {
    m_shapeGeometryParams.SetCategoryId (m_categoryId);
    m_shapeGeometryParams.SetFillDisplay (Render::FillDisplay::Always);
    m_shapeGeometryParams.SetLineColor (ColorDef (66, 134, 244));
    m_shapeGeometryParams.SetFillColor (ColorDef (66, 134, 244));
    m_shapeGeometryParams.SetWeight (1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
CategorySelectorPtr GeometryTests::SetUpCategorySelector ( DictionaryModelR dictionaryModel ) const
    {
    CategorySelectorPtr categorySelectorPtr = new CategorySelector (dictionaryModel, "GridsTest");
    categorySelectorPtr->AddCategory (m_categoryId);
    return categorySelectorPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
ModelSelectorPtr GeometryTests::SetUpModelSelector ( DictionaryModelR dictionaryModel ) const
    {
    ModelSelectorPtr modelSelectorPtr = new ModelSelector (dictionaryModel, "GridsTest");
    modelSelectorPtr->AddModel (GetModel().GetModelId());
    modelSelectorPtr->AddModel (GetPhysicalModel().GetModelId());
    return modelSelectorPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
Render::ViewFlags GeometryTests::SetUpViewFlags ( DisplayStylePtr displayStylePtr ) const
    {
    Render::ViewFlags viewFlags = displayStylePtr->GetViewFlags();
    viewFlags.SetRenderMode (Render::RenderMode::Wireframe);
    viewFlags.SetShowTransparency (false);
    viewFlags.SetShowGrid (true);
    viewFlags.SetShowAcsTriad (true);
    return viewFlags;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
DisplayStyle3dPtr GeometryTests::SetUpDisplayStyle ( DictionaryModelR dictionaryModel ) const
    {
    DisplayStyle3dPtr displayStylePtr = new DisplayStyle3d (dictionaryModel, "GridsTest");
    displayStylePtr->SetBackgroundColor (ColorDef::LightGrey());
    displayStylePtr->SetSkyBoxEnabled (false);
    displayStylePtr->SetGroundPlaneEnabled (false);

    Render::ViewFlags viewFlags = SetUpViewFlags (displayStylePtr);
    displayStylePtr->SetViewFlags (viewFlags);
    return displayStylePtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
void GeometryTests::SetUpViewDefinition ( DictionaryModelR dictionaryModel,
                                          CategorySelectorR categorySelector,
                                          ModelSelectorR modelSelector,
                                          DisplayStyle3dR displayStyle ) const
    {
    OrthographicViewDefinition view (dictionaryModel, "Grids View", categorySelector, displayStyle, modelSelector);
    view.SetCategorySelector (categorySelector);
    view.SetStandardViewRotation (StandardView::Top);
    view.LookAtVolume (GetDb().GeoLocation().GetProjectExtents());
    view.Insert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
void GeometryTests::SetUpForGeometryPresentation ()
    {
    SetUpPhysicalModel();
    SetUpGridModel();
    SetUpGridTestCategory();
    SetUpGeometryParams();

    DictionaryModelR dictionaryModel = GetDb().GetDictionaryModel();
    CategorySelectorPtr categorySelectorPtr = SetUpCategorySelector (dictionaryModel);
    ModelSelectorPtr modelSelectorPtr = SetUpModelSelector (dictionaryModel);
    DisplayStyle3dPtr displayStylePtr = SetUpDisplayStyle (dictionaryModel);
    SetUpViewDefinition (dictionaryModel, *categorySelectorPtr, *modelSelectorPtr, *displayStylePtr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
void GeometryTests::InsertPhysicalCurveElement ( GridCurveCR curve ) const
    {
    PhysicalElementPtr physicalElementPtr = GenericPhysicalObject::Create (GetPhysicalModel(), GetCategoryId());
    physicalElementPtr->SetUserLabel (curve.GetNonElevationSurfaceGridLabel()->GetLabel().c_str());
    physicalElementPtr->SetPlacement (curve.GetPlacement());

    GeometrySource* pGeometrySource = physicalElementPtr->ToGeometrySourceP();
    GeometryBuilderPtr builder = GeometryBuilder::Create (*pGeometrySource);
    builder->Append (GetGeometryParams());

    GeometryCollection geomData = *curve.ToGeometrySource();
    for (auto itGeometricPrimitive : geomData)
        {
        GeometricPrimitivePtr geometricPrimitivePtr = itGeometricPrimitive.GetGeometryPtr();
        ASSERT_TRUE (geometricPrimitivePtr.IsValid());
        builder->Append (*geometricPrimitivePtr);
        ASSERT_TRUE(AppendGridBubbleGraphics (*builder, curve));
        }

    builder->Finish (*pGeometrySource);

    DgnDbStatus insertStatus;
    physicalElementPtr->Insert (&insertStatus);
    ASSERT_TRUE (insertStatus == DgnDbStatus::Success);
    }

#pragma region Bubble Graphics

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
static DVec3d getDirectionAtEndPoint ( ICurvePrimitiveCR primitive, bool atStart )
    {
    // FractionToPointAndUnitTangent doesn't work on child curve vector, so extract very first leaf curve primitive
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == primitive.GetCurvePrimitiveType())
        {
        CurveVectorCPtr curve = primitive.GetChildCurveVectorCP();
        if (curve->empty() || curve->at (0).IsNull())
            return DVec3d::FromZero();

        return getDirectionAtEndPoint (*curve->at (0), atStart);
        }

    ValidatedDRay3d pointAndTangent = primitive.FractionToPointAndUnitTangent (atStart
                                                                                   ? 0.0
                                                                                   : 1.0);
    DVec3d direction = pointAndTangent.IsValid()
                           ? pointAndTangent.Value().direction
                           : DVec3d::FromZero();

    if (!atStart)
        direction.Negate();

    return direction;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
static bool getBubbleOrigin ( DPoint3dR origin, ICurvePrimitiveCR curve, double bubbleRadius, bool atStart )
    {
    // Put bubble so that it touches the curve start point
    // And its center-to-curveStart direction is in the same as beginning of the curve.
    DPoint3d start, end;
    if (!curve.GetStartEnd (start, end))
        return false;

    DPoint3d point = atStart
                         ? start
                         : end;
    DVec3d direction = getDirectionAtEndPoint (curve, atStart);
    if (direction.IsZero())
        return false;

    direction.ScaleToLength (bubbleRadius);
    point.Subtract (direction);
    origin = point;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
static bool addBubble ( GeometryBuilderR builder,
                        TextString labelGeometry,
                        ICurvePrimitiveCR bubbleGeometry,
                        ICurvePrimitiveCR curve,
                        TransformCR toWorld,
                        bool atStart )
    {
    DPoint3d origin;
    if (!getBubbleOrigin (origin, curve, bubbleGeometry.GetArcCP()->vector0.Magnitude(), atStart))
        return false;

    // Bubble position is at world coordinates 
    // and geometry stream is at local coordinates 
    // so bubble and label geometry needs to be adjusted.
    Transform toLocal;
    if (!toLocal.InverseOf (toWorld))
        return false;

    toLocal.Multiply (origin);
    ICurvePrimitivePtr bubbleGeometryCopy = bubbleGeometry.Clone();
    bubbleGeometryCopy->TransformInPlace (Transform::From (origin));
    labelGeometry.SetOriginFromJustificationOrigin (origin,
                                                    TextString::HorizontalJustification::Center,
                                                    TextString::VerticalJustification::Middle);

    builder.SetAppendAsSubGraphics();
    return builder.Append (labelGeometry) && builder.Append (*bubbleGeometryCopy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
static void setUpBubbleAndLabelGeometry ( TextStringPtr& labelGeometry,
                                          ICurvePrimitivePtr& bubbleGeometry,
                                          Utf8CP text )
    {
    TextStringStylePtr labelTextStyle = TextStringStyle::Create();
    labelTextStyle->SetWidth (BUBBLE_RADIUS / 2);
    labelTextStyle->SetHeight (BUBBLE_RADIUS / 2);

    labelGeometry = TextString::Create();
    labelGeometry->SetText (text);
    labelGeometry->SetStyle (*labelTextStyle);

    bubbleGeometry = ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (DPoint3d::FromZero(), BUBBLE_RADIUS));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
static bool addBubbles ( GeometryBuilderR builder,
                         ICurvePrimitiveCR geometry,
                         TransformCR toWorld,
                         Utf8CP label,
                         bool addAtStart,
                         bool addAtEnd )
    {
    TextStringPtr labelGeometry;
    ICurvePrimitivePtr bubbleGeometry;
    setUpBubbleAndLabelGeometry (labelGeometry, bubbleGeometry, label);

    if (addAtStart && !addBubble (builder, *labelGeometry, *bubbleGeometry, geometry, toWorld, true))
        return false;

    if (addAtEnd && !addBubble (builder, *labelGeometry, *bubbleGeometry, geometry, toWorld, false))
        return false;

    return true;
    }

#pragma endregion Bubble Graphics

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
bool GeometryTests::AppendGridBubbleGraphics ( GeometryBuilderR builder, GridCurveCR curve ) const
    {
    DPoint3d originPoint;
    curve.GetCurve()->GetStartPoint (originPoint);

    Placement3d newPlacement (originPoint, curve.GetPlacement().GetAngles());

    GridLabelCPtr gridLabel = curve.GetNonElevationSurfaceGridLabel();
    if (gridLabel.IsNull())
        return false;

    Utf8String label = gridLabel->GetLabel().c_str();
    return Utf8String::IsNullOrEmpty (label.c_str()) || addBubbles (builder,
                                                                    *curve.GetCurve(),
                                                                    newPlacement.GetTransform(),
                                                                    label.c_str(),
                                                                    gridLabel->HasLabelAtStart(),
                                                                    gridLabel->HasLabelAtEnd());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
ElevationGridPtr GeometryTests::InsertElevationGrid ( DPoint3d origin,
                                                      double length,
                                                      double width,
                                                      double elevationIncrement,
                                                      int count,
                                                      Utf8CP gridName ) const
    {
    bvector <DPoint3d> baseShape = {
        { 0, 0, 0 },
        { length, 0, 0 },
        { length, width, 0 },
        { 0, width, 0 },
        { 0, 0, 0 }
    };
    bvector <CurveVectorPtr> floorPlaneCurves = bvector <CurveVectorPtr> (count);
    int gridIteration = 0;
    for (CurveVectorPtr& curveShape : floorPlaneCurves)
        {
        bvector <DPoint3d> thisShape = baseShape;
        std::transform (thisShape.begin(),
                        thisShape.end(),
                        thisShape.begin(),
                        [&] ( DPoint3d point ) ->DPoint3d
                            {
                            point.z = elevationIncrement * gridIteration;
                            return point;
                            });
        curveShape = CurveVector::CreateLinear (thisShape, CurveVector::BOUNDARY_TYPE_Outer);
        ++gridIteration;
        }

    ElevationGridPtr grid = ElevationGrid::CreateAndInsertWithSurfaces (ElevationGrid::CreateParams (GetModel(),
                                                                                                     GetDb().
                                                                                                     Elements().
                                                                                                     GetRootSubject()->
                                                                                                     GetElementId(),
                                                                                                     gridName),
                                                                        floorPlaneCurves);

    Placement3d gridPlacement = grid->GetPlacement();
    gridPlacement.SetOrigin (origin);

    grid->SetPlacement (gridPlacement);
    grid->Update();
    return grid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
OrthogonalGridPtr GeometryTests::InsertOrthogonalGrid ( DPoint3d origin,
                                                        double incrementX,
                                                        double incrementY,
                                                        double minExtentX,
                                                        double maxExtentX,
                                                        double minExtentY,
                                                        double maxExtentY,
                                                        double elevation,
                                                        int countX,
                                                        int countY,
                                                        Utf8CP gridName ) const
    {
    OrthogonalGrid::CreateParams orthogonalParams (GetModel(),
                                                   GetDb().Elements().GetRootSubject()->GetElementId(),
                                                   /*parent element*/
                                                   gridName,
                                                   incrementX,
                                                   /*defaultCoordIncX*/
                                                   incrementY,
                                                   /*defaultCoordIncY*/
                                                   minExtentX,
                                                   /*defaultStaExtX*/
                                                   maxExtentX,
                                                   /*defaultEndExtX*/
                                                   minExtentY,
                                                   /*defaultStaExtY*/
                                                   maxExtentY,
                                                   /*defaultEndExtY*/
                                                   -2 * BUILDING_TOLERANCE,
                                                   /*defaultStaElevation*/
                                                   elevation + 2 * BUILDING_TOLERANCE /*defaultEndElevation*/
                                                  );

    OrthogonalGridPtr grid = OrthogonalGrid::CreateAndInsertWithSurfaces (orthogonalParams, countX, countY);

    Placement3d gridPlacement = grid->GetPlacement();
    gridPlacement.SetOrigin (origin);

    grid->SetPlacement (gridPlacement);
    grid->Update();
    return grid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
RadialGridPtr GeometryTests::InsertRadialGrid ( DPoint3d origin,
                                                Angle angleIncrement,
                                                double radiusIncrement,
                                                Angle startAngle,
                                                Angle endAngle,
                                                double startRadius,
                                                double endRadius,
                                                double elevation,
                                                int radialCount,
                                                int circularCount,
                                                Utf8CP gridName ) const
    {
    RadialGrid::CreateParams createParams (GetModel(),
                                           GetDb().Elements().GetRootSubject()->GetElementId(),
                                           /*scope element*/
                                           gridName,
                                           /*name*/
                                           angleIncrement.Radians(),
                                           /*defaultAngleIncrement*/
                                           radiusIncrement,
                                           /*defaultRadiusIncrement*/
                                           startAngle.Radians(),
                                           /*defaultStartAngle*/
                                           endAngle.Radians(),
                                           /*defaultEndAngle*/
                                           startRadius,
                                           /*defaultStartRadius*/
                                           endRadius,
                                           /*defaultEndRadius*/
                                           -2 * BUILDING_TOLERANCE,
                                           /*defaultstaElevation*/
                                           elevation + 2 * BUILDING_TOLERANCE /*defaultendElevation*/
                                          );

    RadialGridPtr grid = RadialGrid::CreateAndInsertWithSurfaces (createParams, radialCount, circularCount);

    Placement3d gridPlacement = grid->GetPlacement();
    gridPlacement.SetOrigin (origin);

    grid->SetPlacement (gridPlacement);
    grid->Update();
    return grid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
bvector <GridCurvePtr> GeometryTests::InsertIntersectionCurves ( ElevationGridCR elevationGrid, GridCR otherGrid ) const
    {
    bvector <GridCurvePtr> curves;

    GridCurvesSetPtr curvesPortion = GridCurvesSet::Create (GetModel());
    curvesPortion->Insert();

    for (DgnElementId const& elevationSurfaceId : elevationGrid.MakeIterator().BuildIdList <DgnElementId>())
        {
        GridPlanarSurfaceCPtr floorGridSurface = GridPlanarSurface::Get (GetDb(), elevationSurfaceId);
        BeAssert (BentleyStatus::SUCCESS == otherGrid.IntersectGridSurface (floorGridSurface.get(), *curvesPortion));
        GetDb().SaveChanges();
        GetDb ().BriefcaseManager ().StartBulkOperation ();

        for (DgnElementId const& curveBundleId : floorGridSurface->
                                                 MakeGridCurveBundleIterator().BuildIdList <DgnElementId>())
            {
            GridCurveBundleCPtr curveBundle = GridCurveBundle::Get (GetDb(), curveBundleId);
            GridCurveCPtr gridCurveReadOnly = curveBundle->GetGridCurve();
            if (gridCurveReadOnly.IsNull())
                continue;

            curves.push_back (dynamic_cast <GridCurve*> (gridCurveReadOnly->CopyForEdit().get()));
            }
        }

    return curves;
    }

#pragma endregion GeometryTests Methods

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
static GridSurfaceCPtr getGridSurface ( GridCurveCR curve )
    {
    bvector <DgnElementId> intersectingSurfaces = curve.GetIntersectingSurfaceIds();
    for (DgnElementId const& surfaceId : intersectingSurfaces)
        {
        GridSurfaceCPtr surface = curve.GetDgnDb().Elements().Get <GridSurface> (surfaceId);
        if (nullptr == dynamic_cast <ElevationGridSurfaceCP> (surface.get()))
            return surface;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
bool GeometryTests::CreateGridLabel ( GridCurveCR curve, Utf8CP label, bool atStart, bool atEnd ) const
    {
    GridLabelPtr newLabel = GridLabel::Create (*getGridSurface (curve), label, atStart, atEnd);
    if (newLabel.IsNull())
        return false;

    DgnDbStatus status;
    newLabel->Insert (&status);
    return DgnDbStatus::Success == status;
    }

//---------------------------------------------------------------------------------------
// @betest                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
void GeometryTests::SetUp ()
    {
    GridsTestFixtureBase::SetUp ();
    // Copy and open a fresh DB
    BeFileName workingDbPath = copyWorkingDb (GetGivenProjectPath ());

    DgnDb::OpenParams openParams (Db::OpenMode::ReadWrite,
        DefaultTxn::Yes,
        SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade);
    DbResult openStatus;
    m_dbPtr = DgnDb::OpenDgnDb (&openStatus, workingDbPath, openParams);
    BeAssert (m_dbPtr.IsValid ());
    m_dbPtr->BriefcaseManager ().StartBulkOperation ();

    SetUpForGeometryPresentation ();
    }

//---------------------------------------------------------------------------------------
// @betest                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
void GeometryTests::TearDown ()
    {
    m_dbPtr->CloseDb ();
    GridsTestFixtureBase::TearDown ();
    }

//---------------------------------------------------------------------------------------
// @betest                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
TEST_F (GeometryTests, CreateGridsGeometry)
    {
    {
        // orthogonal grid without extents
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (0, 0, 5), 40, 40, 0, 1, "Elevation-O0");
        OrthogonalGridPtr orthogonal = InsertOrthogonalGrid (DPoint3d::From (0, 0, 0),
                                                             10,
                                                             5,
                                                             0,
                                                             15,
                                                             0,
                                                             30,
                                                             10,
                                                             4,
                                                             4,
                                                             "Orthogonal-Basic");
        bvector <GridCurvePtr> curves = InsertIntersectionCurves (*elevation, *orthogonal);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            ASSERT_TRUE (CreateGridLabel (*curve, Utf8PrintfString ("%d", curveNo++).c_str (), curveNo % 2 != 0, curveNo
 % 3 != 0));
            GetDb ().SaveChanges ();
            GetDb ().BriefcaseManager ().StartBulkOperation ();
            InsertPhysicalCurveElement (*curve);
            }
    }

    {
        // orthogonal grid with extents
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (35, -5, 5), 40, 40, 0, 1, "Elevation-O1");
        OrthogonalGridPtr orthogonal = InsertOrthogonalGrid (DPoint3d::From (40, 0, 0),
                                                             10,
                                                             5,
                                                             -1,
                                                             16,
                                                             -1,
                                                             31,
                                                             10,
                                                             4,
                                                             4,
                                                             "Orthogonal-Extended");
        bvector <GridCurvePtr> curves = InsertIntersectionCurves (*elevation, *orthogonal);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            ASSERT_TRUE (CreateGridLabel (*curve, Utf8PrintfString ("%d", curveNo++).c_str (), curveNo % 2 != 0, curveNo
 % 3 != 0));
            GetDb ().SaveChanges ();
            GetDb ().BriefcaseManager ().StartBulkOperation ();
            InsertPhysicalCurveElement (*curve);
            }
    }

    {
        // orthogonal grid altered individual extents
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (75, -5, 5), 40, 40, 0, 1, "Elevation-O2");
        OrthogonalGridPtr orthogonal = InsertOrthogonalGrid (DPoint3d::From (80, 0, 0),
                                                             10,
                                                             5,
                                                             0,
                                                             15,
                                                             0,
                                                             30,
                                                             10,
                                                             4,
                                                             4,
                                                             "Orthogonal-CustomExtended");
        for (DgnElementId axisId : orthogonal->MakeAxesIterator().BuildIdList <DgnElementId>())
            {
            GridAxisCPtr axis = GridAxis::Get (GetDb(), axisId);
            bvector <DgnElementId> surfacesIds = axis->MakeIterator().BuildIdList <DgnElementId>();
            bvector <PlanCartesianGridSurfacePtr> surfaces = bvector <PlanCartesianGridSurfacePtr> (4);
            std::transform (surfacesIds.begin(),
                            surfacesIds.end(),
                            surfaces.begin(),
                            [this] ( DgnElementId elementId ) ->PlanCartesianGridSurfacePtr
                                {
                                return PlanCartesianGridSurface::GetForEdit (GetDb(), elementId);
                                });

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
        bvector <GridCurvePtr> curves = InsertIntersectionCurves (*elevation, *orthogonal);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            ASSERT_TRUE (CreateGridLabel (*curve, Utf8PrintfString ("%d", curveNo++).c_str (), curveNo % 2 != 0, curveNo
 % 3 != 0));
            GetDb ().SaveChanges ();
            GetDb ().BriefcaseManager ().StartBulkOperation ();
            InsertPhysicalCurveElement (*curve);
            }
    }

    {
        // radial grid without extents
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (0, 40, 5), 40, 40, 0, 1, "Elevation-R0");
        RadialGridPtr radial = InsertRadialGrid (DPoint3d::From (0, 40, 0),
                                                 Angle::FromDegrees (30),
                                                 5,
                                                 Angle::FromDegrees (0),
                                                 Angle::FromDegrees (90),
                                                 0,
                                                 30,
                                                 10,
                                                 4,
                                                 6,
                                                 "Radial-Basic");
        bvector <GridCurvePtr> curves = InsertIntersectionCurves (*elevation, *radial);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            ASSERT_TRUE (CreateGridLabel (*curve, Utf8PrintfString ("%04d", curveNo++).c_str (), curveNo % 2 != 0,
 curveNo % 3 != 0));
            GetDb ().SaveChanges ();
            GetDb ().BriefcaseManager ().StartBulkOperation ();
            InsertPhysicalCurveElement (*curve);
            }
    }

    {
        // radial grid with extents
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (35, 35, 5), 40, 40, 0, 1, "Elevation-R1");
        RadialGridPtr radial = InsertRadialGrid (DPoint3d::From (40, 40, 0),
                                                 Angle::FromDegrees (30),
                                                 5,
                                                 Angle::FromDegrees (-10),
                                                 Angle::FromDegrees (100),
                                                 -5,
                                                 35,
                                                 10,
                                                 4,
                                                 6,
                                                 "Radial-Extended");
        bvector <GridCurvePtr> curves = InsertIntersectionCurves (*elevation, *radial);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            ASSERT_TRUE (CreateGridLabel (*curve, Utf8PrintfString ("%d", curveNo++).c_str (), curveNo % 2 != 0, curveNo
 % 3 != 0));
            GetDb ().SaveChanges ();
            GetDb ().BriefcaseManager ().StartBulkOperation ();
            InsertPhysicalCurveElement (*curve);
            }
    }

    {
        // radial grid altered individual extents
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (75, 35, 5), 40, 40, 0, 1, "Elevation-R2");
        RadialGridPtr radial = InsertRadialGrid (DPoint3d::From (80, 40, 0),
                                                 Angle::FromDegrees (30),
                                                 5,
                                                 Angle::FromDegrees (0),
                                                 Angle::FromDegrees (90),
                                                 0,
                                                 30,
                                                 10,
                                                 4,
                                                 6,
                                                 "Radial-CustomExtended");

        RadialAxisCPtr radialAxis = radial->GetRadialAxis();
        bvector <DgnElementId> radialSurfacesIds = radialAxis->MakeIterator().BuildIdList <DgnElementId>();
        bvector <PlanRadialGridSurfacePtr> radialSurfaces = bvector <PlanRadialGridSurfacePtr> (4);
        std::transform (radialSurfacesIds.begin(),
                        radialSurfacesIds.end(),
                        radialSurfaces.begin(),
                        [this] ( DgnElementId elementId ) ->PlanRadialGridSurfacePtr
                            {
                            return PlanRadialGridSurface::GetForEdit (GetDb(), elementId);
                            });
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
        bvector <DgnElementId> circularSurfacesIds = circularAxis->MakeIterator().BuildIdList <DgnElementId>();
        bvector <PlanCircumferentialGridSurfacePtr> circularSurfaces = bvector <PlanCircumferentialGridSurfacePtr> (6);
        std::transform (circularSurfacesIds.begin(),
                        circularSurfacesIds.end(),
                        circularSurfaces.begin(),
                        [this] ( DgnElementId elementId ) ->PlanCircumferentialGridSurfacePtr
                            {
                            return PlanCircumferentialGridSurface::GetForEdit (GetDb(), elementId);
                            });
        circularSurfaces[0]->SetStartAngle (Angle::DegreesToRadians (-10));
        circularSurfaces[0]->SetEndAngle (Angle::DegreesToRadians (100));
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

        bvector <GridCurvePtr> curves = InsertIntersectionCurves (*elevation, *radial);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            ASSERT_TRUE (CreateGridLabel (*curve, Utf8PrintfString ("%d", curveNo++).c_str (), curveNo % 2 != 0, curveNo
 % 3 != 0));
            GetDb ().SaveChanges ();
            GetDb ().BriefcaseManager ().StartBulkOperation ();
            InsertPhysicalCurveElement (*curve);
            }
    }

    {
        // spline grid
        ElevationGridPtr elevation = InsertElevationGrid (DPoint3d::From (0, 80, 5), 40, 40, 0, 1, "Elevation-S0");
        SketchGridPtr sketchGrid = SketchGrid::Create (GetModel(),
                                                       GetModel().GetModeledElementId(),
                                                       "Sketch Grid",
                                                       0.0,
                                                       10.0);
        sketchGrid->Insert();

        DgnModelCR defModel = BuildingUtils::GetGroupInformationModel (GetDb());
        GeneralGridAxisPtr gridAxis = GeneralGridAxis::CreateAndInsert (*sketchGrid);

        SketchLineGridSurface::CreateParams params (*sketchGrid->GetSurfacesModel().get(),
                                                    *gridAxis,
                                                    0.0,
                                                    10,
                                                    DPoint2d::From (0, 20),
                                                    DPoint2d::From (0, 70));
        GridPlanarSurfacePtr plane = SketchLineGridSurface::Create (params);
        plane->Insert();

        SketchArcGridSurface::CreateParams arcParams (*sketchGrid->GetSurfacesModel().get(),
                                                      *gridAxis,
                                                      0.0,
                                                      10,
                                                      GeometryUtils::CreateArc (10, Angle::Pi(), 0));
        GridArcSurfacePtr arc = SketchArcGridSurface::Create (arcParams);
        arc->Insert();

        ICurvePrimitivePtr splinePrimitive = GeometryUtils::CreateSplinePrimitive ({
            { 0, 0, 0 },
            { 10, 0, 0 },
            { 0, 10, 0 },
            { 10, 10, 0 },
            { 20, 0, 0 },
            { 0, 20, 0 },
            { 20, 20, 0 }
        });
        SketchSplineGridSurface::CreateParams splineParams (*sketchGrid->GetSurfacesModel().get(),
                                                            *gridAxis,
                                                            0.0,
                                                            10.0,
                                                            *splinePrimitive);
        GridSplineSurfacePtr spline = SketchSplineGridSurface::Create (splineParams);
        spline->Insert();

        Placement3d gridPlacement = sketchGrid->GetPlacement();
        gridPlacement.SetOrigin (DPoint3d::From (0, 80, 5));
        sketchGrid->SetPlacement (gridPlacement);
        sketchGrid->Update();

        GetDb ().SaveChanges ();
        GetDb ().BriefcaseManager ().StartBulkOperation ();

        bvector <GridCurvePtr> curves = InsertIntersectionCurves (*elevation, *sketchGrid);
        int curveNo = 0;
        for (GridCurvePtr curve : curves)
            {
            ASSERT_TRUE (CreateGridLabel (*curve, Utf8PrintfString ("%d", curveNo++).c_str (), curveNo % 2 != 0, curveNo
 % 3 != 0));
            GetDb ().SaveChanges ();
            GetDb ().BriefcaseManager ().StartBulkOperation ();
            InsertPhysicalCurveElement (*curve);
            }
    }

    GetDb().SaveChanges();
    }
