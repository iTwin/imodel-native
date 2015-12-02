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

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE_EC 
USING_NAMESPACE_BENTLEY_EC

#define VOLUME_DEFAULT_CATEGORY_NAME "VolumeCategory"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
    {
    HANDLER_DEFINE_MEMBERS(NamedVolumeHandler)
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   11/15
//+---------------+---------------+---------------+---------------+---------------+-----
NamedVolume::NamedVolume(CreateParams const& params) : T_Super(params) 
    {
    if (GetModel().IsValid() && params.m_shape.size() > 0)
       SetupGeomStream(params.m_origin, params.m_shape, params.m_height);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   11/15
//+---------------+---------------+---------------+---------------+---------------+-----
// static
DgnCategoryId NamedVolume::GetDefaultCategoryId(DgnDbR db)
    {
    DgnCategoryId categoryId = DgnCategory::QueryCategoryId(VOLUME_DEFAULT_CATEGORY_NAME, db);
    if (categoryId.IsValid())
        return categoryId;

    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef(0xF2, 0xB5, 0x0F)); // Yellow
    appearance.SetTransparency(0.75);
    appearance.SetDontSnap(true);
    appearance.SetDontLocate(true);

    DgnCategory category(DgnCategory::CreateParams(db, VOLUME_DEFAULT_CATEGORY_NAME, DgnCategory::Scope::Physical, DgnCategory::Rank::System));
    category.Insert(appearance);
    
    categoryId = category.GetCategoryId();
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   11/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::SetupGeomStream(DPoint3dCR origin, bvector<DPoint2d> const& shape, double height)
    {
    bvector<DPoint3d> shape3d;
    for (int ii = 0; ii < (int) shape.size(); ii++)
        shape3d.push_back(DPoint3d::From(shape[ii]));
    CurveVectorPtr baseCurve = CurveVector::CreateLinear(&shape3d[0], (int) shape3d.size(), CurveVector::BOUNDARY_TYPE_Outer, true);

    DVec3d extrusionVector;
    extrusionVector.Init(0, 0, height);
    
    DgnExtrusionDetail extrusionDetail(baseCurve, extrusionVector, true);
    ISolidPrimitivePtr extrusionSolid = ISolidPrimitive::CreateDgnExtrusion(extrusionDetail);

    DgnModelPtr model = GetModel();
    BeAssert(model.IsValid());

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*model, GetCategoryId(), origin, YawPitchRollAngles());
    builder->Append(*extrusionSolid);
    builder->SetGeomStreamAndPlacement(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   11/15
//+---------------+---------------+---------------+---------------+---------------+-----
NamedVolumeCPtr NamedVolume::Get(DgnDbCR dgndb, Dgn::DgnElementId elementId)
    {
    DgnElementCPtr element = dgndb.Elements().GetElement(elementId);
    return NamedVolumeCPtr(dynamic_cast<NamedVolume const*> (element.get()));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   11/15
//+---------------+---------------+---------------+---------------+---------------+-----
NamedVolumePtr NamedVolume::GetForEdit(DgnDbCR dgndb, Dgn::DgnElementId elementId)
    {
    return dgndb.Elements().GetForEdit<NamedVolume>(elementId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   11/15
//+---------------+---------------+---------------+---------------+---------------+-----
NamedVolumeCPtr NamedVolume::Insert()
    {
    return GetDgnDb().Elements().Insert<NamedVolume>(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   11/15
//+---------------+---------------+---------------+---------------+---------------+-----
NamedVolumeCPtr NamedVolume::Update()
    {
    return GetDgnDb().Elements().Update<NamedVolume>(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   11/15
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus NamedVolume::ExtractExtrusionDetail(DgnExtrusionDetail& extrusionDetail) const
    {
    ElementGeometryCollection geomCollection(*this);

    ElementGeometryCollection::const_iterator iter = geomCollection.begin();
    if (iter == geomCollection.end())
        {
        BeAssert(false && "Expected DgnExtrusion in the geometry source for NamedVolume-s");
        return ERROR;
        }

    (*iter)->GetAsISolidPrimitive()->TryGetDgnExtrusionDetail(extrusionDetail);
    extrusionDetail.TransformInPlace(geomCollection.GetGeometryToWorld());
    
    BeAssert(++iter == geomCollection.end() && "Did not expect more than two entries in geometry source for NamedVolume-s");
    if (extrusionDetail.m_baseCurve->GetBoundaryType() != CurveVector::BOUNDARY_TYPE_Outer)
        {
        BeAssert(false && "Only extrusions of outer curves supported");
        return ERROR;
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   11/15
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus NamedVolume::ExtractGeomStream(bvector<DPoint3d>& shape, DVec3d& direction, double& height) const
    {
    DgnExtrusionDetail extrusionDetail;
    BentleyStatus status = ExtractExtrusionDetail(extrusionDetail);
    if (SUCCESS != status)
        return ERROR;

    height = direction.Normalize(extrusionDetail.m_extrusionVector);

    if (extrusionDetail.m_baseCurve.IsNull() || extrusionDetail.m_baseCurve->size() != 1)
        {
        BeAssert(false && "Empty or unexpected curve in the NamedVolume profile");
        return ERROR;
        }

    ICurvePrimitivePtr& curve = (*extrusionDetail.m_baseCurve)[0];

    if (!curve.IsValid() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString != curve->GetCurvePrimitiveType())
        {
        BeAssert(false && "Unhandled curve primitives in the NamedVolume profile");
        return ERROR;
        }

    bvector<DPoint3d> const* curvePoints = curve->GetLineStringCP();
    if (curvePoints == nullptr || curvePoints->size() == 0)
        {
        BeAssert(false && "Empty curve primitive in the NamedVolume profile");
        return ERROR;
        }

    shape.clear();
    shape.insert(shape.end(), curvePoints->begin(), curvePoints->end());

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus NamedVolume::SetClip(ViewContextR context) const
    {
    bvector<DPoint3d> shape;
    DVec3d direction;
    double height;
    BentleyStatus status = ExtractGeomStream(shape, direction, height);
    if (SUCCESS != status)
        return status;

    // Sweep the shape to form clip planes
    ClipPlaneSet tmpClipPlaneSet = ClipPlaneSet::FromSweptPolygon(&shape[0], shape.size(), &direction);

    // Add top and bottom clip planes
    DVec3d topNormal;
    topNormal.Negate(direction);
    DPoint3d topPoint;
    topPoint.SumOf(shape[0], direction, height);

    DPlane3d topPlane, bottomPlane;
    bottomPlane.InitFromOriginAndNormal(shape[0], direction);
    topPlane.InitFromOriginAndNormal(topPoint, topNormal);

    ClipPlane topClipPlane(topPlane);
    ClipPlane bottomClipPlane(bottomPlane);

    for (ConvexClipPlaneSet& subSet : tmpClipPlaneSet)
        {
        subSet.push_back(topClipPlane);
        subSet.push_back(bottomClipPlane);
        }

    ClearClip();

    m_clipPlaneSet = new ClipPlaneSet();
    m_clipPlaneSet->insert(m_clipPlaneSet->end(), tmpClipPlaneSet.begin(), tmpClipPlaneSet.end());

    m_viewContext = &context;
    m_viewContext->PushClipPlanes(*m_clipPlaneSet);

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::ClearClip() const
    {
    if (m_viewContext)
        {
        m_viewContext->PopTransformClip();
        m_viewContext = nullptr;
        }

    if (m_clipPlaneSet)
        {
        delete m_clipPlaneSet;
        m_clipPlaneSet = nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
ClipVectorPtr NamedVolume::CreateClipVector() const
    {
    DgnExtrusionDetail extrusionDetail;
    BentleyStatus status = ExtractExtrusionDetail(extrusionDetail);
    if (SUCCESS != status)
        return nullptr;

    double zLow = 0.0;
    double zHigh = extrusionDetail.m_extrusionVector.Magnitude();
    return ClipVector::CreateFromCurveVector(*(extrusionDetail.m_baseCurve), 0.1, 0.4, &zLow, &zHigh);
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
unique_ptr<FenceParams> NamedVolume::CreateFence(DgnViewportP viewport, bool allowPartialOverlaps) const
    {
    unique_ptr<FenceParams> fence = std::unique_ptr<FenceParams> (FenceParams::Create ());
    
    ClipVectorPtr clipVector = this->CreateClipVector();
    if (clipVector.IsNull())
        return nullptr;
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
    // TODO: Is it cool to assume the first view found can be used to create a CameraViewController?
    auto viewIter = ViewDefinition::MakeIterator(project);
    BeAssert(viewIter.begin() != viewIter.end());
    if (viewIter.begin() == viewIter.end())
        return nullptr;

    DgnViewId viewId = (*viewIter.begin()).GetId(); 
    CameraViewControllerP viewController = new CameraViewController(project, viewId);
    viewController->Load();
    viewController->SetCameraOn (false); // Has to be done after Load()!!
    return std::unique_ptr<NonVisibleViewport> (new NonVisibleViewport (*viewController));
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   02/15
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus NamedVolume::GetRange(DRange3d& range) const
    {
    DgnExtrusionDetail extrusionDetail;
    BentleyStatus status = ExtractExtrusionDetail(extrusionDetail);
    if (SUCCESS != status)
        return ERROR;

    extrusionDetail.GetRange(range);
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::FindElements(DgnElementIdSet& elementIds, FenceParamsR fence, Statement& stmt, DgnDbR dgnDb) const
    {
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

        elementIds.insert (id);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::FindElements(DgnElementIdSet& elementIds, DgnDbR dgnDb, bool allowPartialOverlaps /*=true*/) const
    {
    unique_ptr<DgnViewport> viewport = CreateNonVisibleViewport (dgnDb);
    unique_ptr<FenceParams> fence = this->CreateFence (viewport.get(), allowPartialOverlaps);
    
    // Prepare element query by range
    Statement stmt;
    Utf8CP sql = "SELECT ElementId FROM " DGN_VTABLE_RTree3d " WHERE MaxX > ? AND MinX < ?  AND MaxY > ? AND MinY < ? AND MaxZ > ? AND MinZ < ? AND ElementId != ?";
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

    DgnElementId elementId = GetElementId();
    if (elementId.IsValid())
        stmt.BindId(7, GetElementId()); // Exclude the NamedVolume itself from the checks!!

    this->FindElements (elementIds, *fence, stmt, dgnDb);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::FindElements(DgnElementIdSet& elementIds, DgnViewportR viewport, bool allowPartialOverlaps /*=true*/) const
    {
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
    Utf8CP sql = "SELECT r.ElementId FROM " DGN_VTABLE_RTree3d " AS r, " DGN_TABLE(DGN_CLASSNAME_Element) " AS e, " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " AS g " \
        " WHERE r.MaxX > ? AND r.MinX < ?  AND r.MaxY > ? AND r.MinY < ? AND r.MaxZ > ? AND r.MinZ < ?" \
        " AND e.Id=r.ElementId AND g.ElementId=r.ElementId AND e.Id != ?" \
        " AND InVirtualSet (?,e.ModelId,g.CategoryId)";
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

    DgnElementId elementId = GetElementId();
    if (elementId.IsValid())
        stmt.BindId(7, GetElementId()); // Exclude the NamedVolume itself from the checks!!

    stmt.BindVirtualSet (8, *viewController);

    this->FindElements (elementIds, *fence, stmt, dgnDb);

    // Turn camera back on
    viewController->SetCameraOn (wasCameraOn);
    viewport.SynchWithViewController (false);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
bool NamedVolume::ContainsElement(DgnElementCR element, bool allowPartialOverlaps /*=true*/) const
    {
    GeometrySourceCP geomSource = element.ToGeometrySource();
    if (nullptr == geomSource)
        return false;

    unique_ptr<DgnViewport> viewport = CreateNonVisibleViewport(element.GetDgnDb());
    unique_ptr<FenceParams> fence = this->CreateFence (viewport.get(), allowPartialOverlaps);

    return fence->AcceptElement(*geomSource);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   02/15
//+---------------+---------------+---------------+---------------+---------------+-----
void NamedVolume::Fit (DgnViewport& viewport, double const* aspectRatio /*=nullptr*/, ViewController::MarginPercent const* margin /*=nullptr*/) const
    {
    DRange3d volumeRange;
    this->GetRange (volumeRange);

    viewport.GetViewControllerR().LookAtVolume(volumeRange, aspectRatio, margin);
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
