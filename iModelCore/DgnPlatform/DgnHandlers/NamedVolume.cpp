/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/NamedVolume.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/NamedVolume.h>
#include    <DgnPlatform/QueryView.h>

#define DGNEC_SCHEMA_NAME L"dgn"
#define DGNEC_SCHEMA_MAJOR_VERSION 1
#define DGNEC_SCHEMA_MINOR_VERSION 2

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE_EC 
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::SetShape (DPoint2dCP shapePoints, size_t shapeNumPoints)
    {
    m_shape.resize (shapeNumPoints);
    memcpy (&m_shape.front(), shapePoints, shapeNumPoints * sizeof(DPoint2d));

    if (!m_shape[shapeNumPoints -1].IsEqual (m_shape[0]))
        m_shape.push_back (m_shape[0]);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::BindForInsertOrUpdate (ECSqlStatement& statement) const
    {
    statement.BindText (1, m_name.c_str(), IECSqlBinder::MakeCopy::No);
    statement.BindPoint3D (2, m_origin);
    statement.BindDouble (3, m_height);
    IECSqlArrayBinder& arrayBinder = statement.BindArray (4, (uint32_t) m_shape.size());
    for (DPoint2dCR point : m_shape)
        {
        arrayBinder.AddArrayElement().BindPoint2D (point);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
StatusInt NamedVolume::Insert (DgnDbR project)
    {
    PRECONDITION (!m_ecKey.IsValid() && "Call update instead", ERROR);

    ECSqlStatement statement;
    Utf8CP ecSql = "INSERT INTO dgn.NamedVolume (Name, Origin, Height, Shape) VALUES(?,?,?,?)";
    ECSqlStatus ecSqlStatus = statement.Prepare (project, ecSql);
    if (!EXPECTED_CONDITION (ecSqlStatus.IsSuccess()))
        return ERROR;

    this->BindForInsertOrUpdate (statement);
    DbResult stepStatus = statement.Step (m_ecKey);
    return (stepStatus == BE_SQLITE_DONE) ? SUCCESS : ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
// static 
unique_ptr<NamedVolume> NamedVolume::Read (Utf8StringCR name, DgnDbR project)
    {
    ECSqlStatement statement;
    Utf8PrintfString ecSql ("SELECT Origin, Height, Shape FROM ONLY dgn.NamedVolume WHERE Name = '%s'", name.c_str());
    ECSqlStatus status = statement.Prepare (project, ecSql.c_str());
    if (!EXPECTED_CONDITION (status.IsSuccess()))
        return nullptr;

    DbResult stepStatus = statement.Step();
    if (!EXPECTED_CONDITION (stepStatus == BE_SQLITE_ROW))
        return nullptr;

    DPoint3d origin = statement.GetValuePoint3D (0);
    double height = statement.GetValueDouble (1);
    IECSqlArrayValue const& shapeValue = statement.GetValueArray (2);
    bvector<DPoint2d> shape;
    for (IECSqlValue const* pointValue : shapeValue)
        {
        shape.push_back (pointValue->GetPoint2D());
        }

    unique_ptr<NamedVolume> volume = std::unique_ptr<NamedVolume> (new NamedVolume (name, origin, (DPoint2dCP) &shape[0], shape.size(), height));
    return volume;
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
// static 
bool NamedVolume::Exists (Utf8StringCR name, DgnDbR project)
    {
    ECSqlStatement statement;
    Utf8PrintfString ecSql ("SELECT * FROM ONLY dgn.NamedVolume WHERE Name = '%s'", name.c_str());
    ECSqlStatus status = statement.Prepare (project, ecSql.c_str());
    if (!EXPECTED_CONDITION (status.IsSuccess()))
        return false;

    DbResult stepStatus = statement.Step();
    return (stepStatus == BE_SQLITE_ROW);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
StatusInt NamedVolume::Update (DgnDbR project)
    {
    PRECONDITION (m_ecKey.IsValid() && "Call insert instead", ERROR);

    ECSqlStatement statement;
    Utf8CP ecSql = "UPDATE ONLY dgn.NamedVolume SET Name=?, Origin=?, Height=?, Shape=? WHERE ECInstanceId=?";
    ECSqlStatus ecSqlStatus = statement.Prepare (project, ecSql);
    if (!EXPECTED_CONDITION (ecSqlStatus.IsSuccess()))
        return ERROR;

    this->BindForInsertOrUpdate (statement);
    statement.BindId (5, m_ecKey.GetECInstanceId());

    DbResult stepStatus = statement.Step ();
    return (stepStatus == BE_SQLITE_DONE) ? SUCCESS : ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
// static 
StatusInt NamedVolume::Delete (Utf8StringCR name, DgnDbR project)
    {
    ECSqlStatement statement;
    Utf8CP ecSql = "DELETE FROM ONLY dgn.NamedVolume WHERE NamedVolume.Name=?";
    ECSqlStatus ecSqlStatus = statement.Prepare (project, ecSql);
    if (!EXPECTED_CONDITION (ecSqlStatus.IsSuccess()))
        return ERROR;

    statement.BindText (1, name.c_str(), IECSqlBinder::MakeCopy::No);
    
    DbResult stepStatus = statement.Step ();
    return (stepStatus == BE_SQLITE_DONE) ? SUCCESS : ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::SetClip (DgnViewport& viewport) const
    {
    DPoint3d origin = this->GetOrigin();

    DVec3d direction;
    direction.Init (0, 0, 1.0);

    // Transform the local shape points to the Project Csys
    bvector<DPoint3d> shape;
    this->Get3dShape (shape);

    // Sweep the shape to form clip planes
    ClipPlaneSet tmpClipPlaneSet = ClipPlaneSet::FromSweptPolygon (&shape[0], shape.size(), &direction);

    // Add top and bottom clip planes
    DVec3d topNormal;
    topNormal.Negate(direction);
    DPoint3d topOrigin;
    topOrigin.SumOf (origin, direction, this->GetHeight());

    DPlane3d topPlane, bottomPlane;
    bottomPlane.InitFromOriginAndNormal (origin, direction);
    topPlane.InitFromOriginAndNormal (topOrigin, topNormal);

    ClipPlane topClipPlane (topPlane);
    ClipPlane bottomClipPlane (bottomPlane);

    for (ConvexClipPlaneSet& subSet : tmpClipPlaneSet)
        {
        subSet.push_back (topClipPlane);
        subSet.push_back (bottomClipPlane);
        }

    // Setup the view controller
    ClipPlaneSet* sweptClipPlaneSet = new ClipPlaneSet();
    sweptClipPlaneSet->insert (sweptClipPlaneSet->end(), tmpClipPlaneSet.begin(), tmpClipPlaneSet.end());
#ifdef WIP_NAMED_VOLUME
    PhysicalViewControllerP vi = (PhysicalViewControllerP) viewport.GetViewControllerP()->ToPhysicalViewController();
    vi->SetClipPlanes (sweptClipPlaneSet); // PhysicalViewController takes over memorymanagement of ClipPlaneSet
#endif
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
// static
void NamedVolume::ClearClip (DgnViewport& viewport)
    {
#ifdef WIP_NAMED_VOLUME
    PhysicalViewControllerP vi = (PhysicalViewControllerP) viewport.GetViewControllerP()->ToPhysicalViewController();
    vi->SetClipPlanes (nullptr);
#endif
    }

#define WEIGHT_Thin 1
#define WEIGHT_Bold 3
#define STYLE_Solid 0

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
// static
void NamedVolume::DrawFace (DPoint3dCP points, size_t numPoints, uint32_t color, ViewContextR context )
    {
    DgnViewportP viewport = context.GetViewport ();
    BeAssert (viewport != nullptr);
    IViewDrawR vDraw = context.GetIViewDraw();

    viewport->SetSymbologyRgb (ColorDef(color), ColorDef(color), WEIGHT_Thin, STYLE_Solid);
    vDraw.DrawShape3d ((int) numPoints, points, true, nullptr);

    viewport->SetSymbologyRgb (ColorDef(color), ColorDef(color), WEIGHT_Bold, STYLE_Solid);
    vDraw.DrawLineString3d ((int) numPoints, points, nullptr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::Draw (uint32_t color, ViewContextR context) const
    {
    DgnViewportP viewport = context.GetViewport ();
    if (!EXPECTED_CONDITION (viewport != nullptr))
        return;

    viewport->SetSymbologyRgb (ColorDef(color), ColorDef(color), WEIGHT_Thin, STYLE_Solid);

    bvector<DPoint3d> bottomFace;
    this->Get3dShape (bottomFace);

    DVec3d direction;
    direction.Init (0, 0, 1.0);

    bvector<DPoint3d> topFace;
    for (int ii=0; ii < (int) bottomFace.size(); ii++)
        {
        DPoint3dR bottomPoint = bottomFace[ii];

        DPoint3d topPoint;
        topPoint.SumOf (bottomPoint, direction, this->GetHeight());
        topFace.push_back (topPoint);

        if (ii > 0)
            {
            DPoint3dR previousBottomPoint = bottomFace[ii-1];
            DPoint3dR previousTopPoint = topFace[ii-1];
            DPoint3d sideFace[5] = {previousBottomPoint, bottomPoint, topPoint, previousTopPoint, previousBottomPoint};
            DrawFace (sideFace, 5, color, context);
            }
        }

    DrawFace  (&topFace[0], topFace.size(), color, context);
    DrawFace  (&bottomFace[0], bottomFace.size(),color, context);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::Get3dShape (bvector<DPoint3d>& shape) const
    {
    DPoint3d origin = this->GetOrigin();
    bvector<DPoint2d> localShape = this->GetShape();
    for (int ii=0; ii < (int) localShape.size(); ii++)
        {
        DPoint3d point = DPoint3d::FromSumOf (origin, DPoint3d::From (localShape[ii]));
        shape.push_back (point);
        }
    }
     
//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
ClipVectorPtr NamedVolume::CreateClipVector() const
    {
    bvector<DPoint3d> shape;
    this->Get3dShape (shape);

    CurveVectorPtr curveVector = CurveVector::CreateLinear (&shape[0], (int) shape.size(), CurveVector::BOUNDARY_TYPE_Outer, true);
    double zLow = 0.0;
    double zHigh = this->GetHeight();
    return ClipVector::CreateFromCurveVector (*curveVector, 0.1, 0.4, &zLow, &zHigh);
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
unique_ptr<FenceParams> NamedVolume::CreateFence(DgnViewportP viewport, bool allowPartialOverlaps) const
    {
    unique_ptr<FenceParams> fence = std::unique_ptr<FenceParams> (FenceParams::Create ());
    
    ClipVectorPtr clipVector = this->CreateClipVector();
    fence->SetClip (*clipVector);
    fence->SetClipMode (FenceClipMode::None);
    fence->SetOverlapMode (allowPartialOverlaps);
    fence->SetViewParams (viewport);
    
    // Note: Setting view params from the viewport sets up the transform from the view, which is
    // not what we want since our clips are in the project's (world) coordinate system
    Transform identity;
    identity.InitIdentity();
    fence->SetTransform (identity);

    return std::move (fence);
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
// static
unique_ptr<DgnViewport> NamedVolume::CreateNonVisibleViewport (DgnDbR project) 
    {
    // TODO: Is there a way to avoid specifying a view??
    // TODO: Is it cool to assume the first view found can be used to create a PhysicalViewController?
    auto viewIter = ViewDefinition::MakeIterator(project);
    BeAssert(viewIter.begin() != viewIter.end());
    if (viewIter.begin() == viewIter.end())
        return nullptr;

    DgnViewId viewId = (*viewIter.begin()).GetId(); 
    PhysicalViewControllerP viewController = new PhysicalViewController (project, viewId);
    viewController->Load();
#ifdef WIP_NAMED_VOLUME
    viewController->SetCameraOn (false); // Has to be done after Load()!!
#endif
    return std::unique_ptr<NonVisibleViewport> (new NonVisibleViewport (*viewController));
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   02/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::GetRange(DRange3d& range) const
    {
    bvector<DPoint3d> bottomShape;
    this->Get3dShape (bottomShape);

    DVec3d direction;
    direction.Init (0, 0, 1.0);

    bvector<DPoint3d> topShape;
    for (const DPoint3d& bottomPoint : bottomShape)
        {
        DPoint3d topPoint;
        topPoint.SumOf (bottomPoint, direction, this->GetHeight());
        topShape.push_back (topPoint);
        }

    range.InitFrom (&bottomShape[0], (int) bottomShape.size());
    range.Extend (&topShape[0], (int) topShape.size());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::FindElements 
(
DgnElementIdSet* elementIds,
FenceParamsR fence,
Statement& stmt,
DgnDbR dgnDb
) const
    {
    if (nullptr == elementIds)
        return;

    DbResult result;
    while ((result = stmt.Step()) == BE_SQLITE_ROW)
        {
        DgnElementId id = stmt.GetValueId<DgnElementId> (0);
        DgnElementCPtr element = dgnDb.Elements().GetElement(id);

        if (!element.IsValid())
            continue;
        
        GeometrySourceCP geomElement = element->ToGeometrySource();

        if (nullptr == geomElement || !fence.AcceptElement (*geomElement))
            continue;

        elementIds->insert (id);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::FindElements 
(
DgnElementIdSet* elementIds,
DgnDbR dgnDb,
bool allowPartialOverlaps /*=true*/
) const
    {
    if (nullptr == elementIds)
        return;

    unique_ptr<DgnViewport> viewport = CreateNonVisibleViewport (dgnDb);
    unique_ptr<FenceParams> fence = this->CreateFence (viewport.get(), allowPartialOverlaps);
    
    // Prepare element query by range
    Statement stmt;
#ifdef WIP_NAMED_VOLUME
    Utf8CP sql = "SELECT ElementId FROM " DGNELEMENT_VTABLE_3dRTree " WHERE xmax > ? AND xmin < ?  AND ymax > ? AND ymin < ? AND zmax > ? AND zmin < ?";
#else
    Utf8CP sql = "";
#endif
    DbResult result = stmt.Prepare (dgnDb, sql);
    BeAssert (result == BE_SQLITE_OK);

    DRange3d volumeRange;
    this->GetRange (volumeRange);
    stmt.BindDouble (1, volumeRange.low.x);
    stmt.BindDouble (2, volumeRange.high.x);
    stmt.BindDouble (3, volumeRange.low.y);
    stmt.BindDouble (4, volumeRange.high.y);
    stmt.BindDouble (5, volumeRange.low.z);
    stmt.BindDouble (6, volumeRange.high.z);

    this->FindElements (elementIds, *fence, stmt, dgnDb);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::FindElements 
(
DgnElementIdSet* elementIds,
DgnViewportR viewport,
bool allowPartialOverlaps /*=true*/
) const
    {
    if (nullptr == elementIds)
        return;

    QueryViewControllerP viewController = dynamic_cast<QueryViewControllerP> (&viewport.GetViewControllerR());
    BeAssert (viewController != nullptr);
    DgnDbR dgnDb = viewController->GetDgnDb();
    
    // Turn camera off
    // TODO: Seems like turning the camera off and back on shouldn't be necessary, but the results seem to be affected - needs investigation .
    bool wasCameraOn = viewController->IsCameraOn();
    viewController->SetCameraOn (false); 
    viewport.SynchWithViewController (false); 
    
    unique_ptr<FenceParams> fence = this->CreateFence (&viewport, allowPartialOverlaps);
    
    // Prepare element query by range, and by what's visible in the view
    Statement stmt;
#ifdef WIP_NAMED_VOLUME
    Utf8CP sql = "SELECT a.ElementId FROM " DGNELEMENT_VTABLE_3dRTree " AS a, " DGNELEMENT_TABLE_Data " AS b" \
        " WHERE a.xmax > ? AND a.xmin < ?  AND a.ymax > ? AND a.ymin < ? AND a.zmax > ? AND a.zmin < ?" \
        " AND a.ElementId=b.ElementId" \
        " AND InVirtualSet (?, b.ModelId, b.Level)";
#else
    Utf8CP sql = "";
#endif
    DbResult result = stmt.Prepare (dgnDb, sql);
    BeAssert (result == BE_SQLITE_OK);

    DRange3d volumeRange;
    this->GetRange (volumeRange);
    DRange3d viewRange = viewport.GetFrustum (DgnCoordSystem::World, false).ToRange();
    DRange3d queryRange; //  Query range is the intersection of the range of the volume, and range of the current view
    queryRange.IntersectionOf (volumeRange, viewRange);
    stmt.BindDouble (1, queryRange.low.x);
    stmt.BindDouble (2, queryRange.high.x);
    stmt.BindDouble (3, queryRange.low.y);
    stmt.BindDouble (4, queryRange.high.y);
    stmt.BindDouble (5, queryRange.low.z);
    stmt.BindDouble (6, queryRange.high.z);

    stmt.BindVirtualSet (7, *viewController);

    this->FindElements (elementIds, *fence, stmt, dgnDb);

    // Turn camera back on
    viewController->SetCameraOn (wasCameraOn);
    viewport.SynchWithViewController (false);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
bool NamedVolume::ContainsElement
(
DgnElementR element, 
bool allowPartialOverlaps /*=true*/
) const
    {
    GeometrySourceCP geomElement = element.ToGeometrySource();

    if (nullptr == geomElement)
        return false;

    DgnDbR dgnDb = element.GetDgnDb();

    unique_ptr<DgnViewport> viewport = CreateNonVisibleViewport (dgnDb);
    unique_ptr<FenceParams> fence = this->CreateFence (viewport.get(), allowPartialOverlaps);

    return fence->AcceptElement (*geomElement);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   02/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::Fit (DgnViewport& viewport, double const* aspectRatio /*=nullptr*/, ViewController::MarginPercent const* margin /*=nullptr*/) const
    {
    DRange3d volumeRange;
    this->GetRange (volumeRange);
#ifdef WIP_NAMED_VOLUME
    viewport.GetViewControllerP()->LookAtVolume (volumeRange, aspectRatio, margin);
#endif
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
