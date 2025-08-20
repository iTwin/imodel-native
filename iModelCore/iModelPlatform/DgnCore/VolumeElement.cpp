/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/VolumeElement.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

#define VOLUME_DEFAULT_CATEGORY_NAME "VolumeCategory"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler
    {
    HANDLER_DEFINE_MEMBERS(VolumeElementHandler)
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
VolumeElement::VolumeElement(CreateParams const& params) : T_Super(params)
    {
    if (GetModel().IsValid() && params.m_shape.size() > 0)
       SetupGeomStream(params.m_origin, params.m_shape, params.m_height);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
// static
DgnCategoryId VolumeElement::GetDefaultCategoryId(DgnDbR db)
    {
    DefinitionModelR dictionary = db.GetDictionaryModel();
    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId(dictionary, VOLUME_DEFAULT_CATEGORY_NAME);
    if (categoryId.IsValid())
        return categoryId;

    DgnSubCategory::Appearance appearance;
    appearance.SetColor(ColorDef(0xF2, 0xB5, 0x0F)); // Yellow
    appearance.SetTransparency(0.75);
    appearance.SetDontSnap(true);
    appearance.SetDontLocate(true);

    SpatialCategory category(dictionary, VOLUME_DEFAULT_CATEGORY_NAME, DgnCategory::Rank::System);
    category.Insert(appearance);

    categoryId = category.GetCategoryId();
    BeAssert(categoryId.IsValid());
    return categoryId;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
void VolumeElement::SetupGeomStream(DPoint3dCR origin, bvector<DPoint2d> const& shape, double height)
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

    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, GetCategoryId(), origin, YawPitchRollAngles());
    builder->Append(*extrusionSolid);
    builder->Finish(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
VolumeElementCPtr VolumeElement::Get(DgnDbCR dgndb, Dgn::DgnElementId elementId)
    {
    DgnElementCPtr element = dgndb.Elements().GetElement(elementId);
    return VolumeElementCPtr(dynamic_cast<VolumeElement const*> (element.get()));
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
VolumeElementPtr VolumeElement::GetForEdit(DgnDbCR dgndb, Dgn::DgnElementId elementId)
    {
    return dgndb.Elements().GetForEdit<VolumeElement>(elementId);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
VolumeElementCPtr VolumeElement::Insert()
    {
    return GetDgnDb().Elements().Insert<VolumeElement>(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
VolumeElementCPtr VolumeElement::Update()
    {
    return GetDgnDb().Elements().UpdateAndGet<VolumeElement>(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
// static
DgnElementIdSet VolumeElement::QueryVolumes(DgnDbCR db)
    {
    DgnElementIdSet ids;

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_VolumeElement));
    if (stmt.IsValid())
        {
        while (BE_SQLITE_ROW == stmt->Step())
            ids.insert(stmt->GetValueId<DgnElementId>(0));
        }

    return ids;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
DgnElementId VolumeElement::QueryVolumeByLabel(DgnDbCR db, Utf8CP label)
    {
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_VolumeElement) " WHERE UserLabel=? LIMIT 1"); // find first if label not unique

    if (label)
        stmt->BindText(1, label, IECSqlBinder::MakeCopy::No);
    else
        stmt->BindNull(1);

    return (BE_SQLITE_ROW != stmt->Step()) ? DgnElementId() : stmt->GetValueId<DgnElementId>(0);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus VolumeElement::ExtractExtrusionDetail(DgnExtrusionDetail& extrusionDetail) const
    {
    GeometryCollection geomCollection(*this);

    GeometryCollection::const_iterator iter = geomCollection.begin();
    if (iter == geomCollection.end())
        {
        BeAssert(false && "Expected DgnExtrusion in the geometry source for VolumeElement-s");
        return ERROR;
        }

    iter.GetGeometryPtr()->GetAsISolidPrimitive()->TryGetDgnExtrusionDetail(extrusionDetail);
    extrusionDetail.TransformInPlace(iter.GetGeometryToWorld());

    BeAssert(++iter == geomCollection.end() && "Did not expect more than two entries in geometry source for VolumeElement-s");
    if (extrusionDetail.m_baseCurve->GetBoundaryType() != CurveVector::BOUNDARY_TYPE_Outer)
        {
        BeAssert(false && "Only extrusions of outer curves supported");
        return ERROR;
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus VolumeElement::ExtractGeomStream(bvector<DPoint3d>& shape, DVec3d& direction, double& height) const
    {
    DgnExtrusionDetail extrusionDetail;
    BentleyStatus status = ExtractExtrusionDetail(extrusionDetail);
    if (SUCCESS != status)
        return ERROR;

    height = direction.Normalize(extrusionDetail.m_extrusionVector);

    if (extrusionDetail.m_baseCurve.IsNull() || extrusionDetail.m_baseCurve->size() != 1)
        {
        BeAssert(false && "Empty or unexpected curve in the VolumeElement profile");
        return ERROR;
        }

    ICurvePrimitivePtr& curve = (*extrusionDetail.m_baseCurve)[0];

    if (!curve.IsValid() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString != curve->GetCurvePrimitiveType())
        {
        BeAssert(false && "Unhandled curve primitives in the VolumeElement profile");
        return ERROR;
        }

    bvector<DPoint3d> const* curvePoints = curve->GetLineStringCP();
    if (curvePoints == nullptr || curvePoints->size() == 0)
        {
        BeAssert(false && "Empty curve primitive in the VolumeElement profile");
        return ERROR;
        }

    shape.clear();
    shape.insert(shape.end(), curvePoints->begin(), curvePoints->end());

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus VolumeElement::SetClip(ViewContextR context) const
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
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    m_viewContext->PushClipPlanes(*m_clipPlaneSet);
#endif

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
void VolumeElement::ClearClip() const
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    if (m_viewContext)
        {
        m_viewContext->PopClip();
        m_viewContext = nullptr;
        }
#endif

    if (m_clipPlaneSet)
        {
        delete m_clipPlaneSet;
        m_clipPlaneSet = nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
ClipVectorPtr VolumeElement::CreateClipVector() const
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
BentleyStatus VolumeElement::GetRange(DRange3d& range) const
    {
    DgnExtrusionDetail extrusionDetail;
    BentleyStatus status = ExtractExtrusionDetail(extrusionDetail);
    if (SUCCESS != status)
        return ERROR;

    extrusionDetail.GetRange(range);
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
void VolumeElement::FindElements(DgnElementIdSet& elementIds, DgnDbR dgnDb, bool allowPartialOverlaps /*=true*/) const
    {
#if defined(TODO_FENCE_PARAMS)
    // This was always wacky - creating a viewport from the first spatial view in the file - assuming this is used anywhere, simplify implementation.
    DgnViewportPtr viewport = CreateNonVisibleViewport (dgnDb);
    FenceParams fence = CreateFence (viewport.get(), allowPartialOverlaps);

    // Prepare element query by range
    Statement stmt;
    Utf8CP sql = "SELECT ElementId FROM " DGN_VTABLE_SpatialIndex " WHERE MaxX > ? AND MinX < ?  AND MaxY > ? AND MinY < ? AND MaxZ > ? AND MinZ < ? AND ElementId != ?";
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
        stmt.BindId(7, GetElementId()); // Exclude the VolumeElement itself from the checks!!

    FindElements (elementIds, fence, stmt, dgnDb);
#endif
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
bool VolumeElement::ContainsElement(DgnElementCR element, bool allowPartialOverlaps /*=true*/) const
    {
#if defined(TODO_FENCE_PARAMS)
    // This was always wacky - creating a viewport from the first spatial view in the file - assuming this is used anywhere, simplify implementation.
    GeometrySourceCP geomSource = element.ToGeometrySource();
    if (nullptr == geomSource)
        return false;

    DgnViewportPtr viewport = CreateNonVisibleViewport(element.GetDgnDb());
    FenceParams fence = CreateFence (viewport.get(), allowPartialOverlaps);

    return fence.AcceptElement(*geomSource);
#else
    return false;
#endif
    }

END_BENTLEY_DGN_NAMESPACE
