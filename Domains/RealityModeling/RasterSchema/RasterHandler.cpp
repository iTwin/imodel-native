/*-------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <RasterInternal.h>
#include <Raster/RasterHandler.h>
#include "RasterTileTree.h"
#include "WmsSource.h"

#define RASTER_MODEL_PROP_Clip "Clip"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RASTER

HANDLER_DEFINE_MEMBERS(RasterModelHandler)

BEGIN_UNNAMED_NAMESPACE
static const double halfMillimeter() { return .5 * DgnUnits::OneMillimeter(); }
static void fixRange(double& low, double& high) { if (low == high) { low -= halfMillimeter(); high += halfMillimeter(); } }
END_UNNAMED_NAMESPACE

//----------------------------------------------------------------------------------------
// @bsiclass                                                      Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
struct RasterBorderGeometrySource : public GeometrySource3d, RefCountedBase
    {
    RasterBorderGeometrySource(DPoint3dCP corners, RasterModel& model);

    virtual DgnDbR _GetSourceDgnDb() const override { return m_dgnDb; }
    virtual DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    virtual Placement3dCR _GetPlacement() const override { return m_placement; }
    virtual GeometryStreamCR _GetGeometryStream() const override { return m_geom; }

    virtual DgnElementCP _ToElement() const override { return nullptr; }
    virtual GeometrySource3dCP _GetAsGeometrySource3d() const override { return this; }
    virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { return DgnDbStatus::Success; }
    virtual DgnDbStatus _SetPlacement(Placement3dCR placement) override { return DgnDbStatus::Success; }

    DgnDbR                      m_dgnDb;
    DgnCategoryId               m_categoryId;
    Placement3d                 m_placement;
    GeometryStream              m_geom;
    };

/*---------------------------------------------------------------------------------**//**
* Hack: RasterHandler should be selecting a specific category rather than a random one.
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnCategoryId getDefaultCategoryId(DgnDbR db)
    {
    ElementIterator iterator = SpatialCategory::MakeIterator(db);
    if (iterator.begin() == iterator.end())
        return DgnCategoryId();

    return (*iterator.begin()).GetId<DgnCategoryId>();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
RasterBorderGeometrySource::RasterBorderGeometrySource(DPoint3dCP pCorners, RasterModel& model)
    :m_dgnDb(model.GetDgnDb()),
    m_categoryId(getDefaultCategoryId(model.GetDgnDb()))
    {
    GeometryBuilderPtr builder = GeometryBuilder::Create(*this);
    Render::GeometryParams geomParams;
    geomParams.SetCategoryId(GetCategoryId());
    geomParams.SetLineColor(ColorDef::LightGrey());
    geomParams.SetWeight(2);
    builder->Append(geomParams);

    DPoint3d box[5];
    box[0] = pCorners[0];
    box[1] = pCorners[1];
    box[2] = pCorners[3];
    box[3] = pCorners[2];
    box[4] = box[0];
    builder->Append(*ICurvePrimitive::CreateLineString(box, 5));

    builder->Finish(*this);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
RasterClip::RasterClip() {};
RasterClip::~RasterClip() {}
CurveVectorCP RasterClip::GetBoundaryCP() const {return m_pBoundary.get();}
RasterClip::MaskVector const& RasterClip::GetMasks() const { return m_masks; }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
void RasterClip::Clear()
    {
    m_pBoundary = nullptr;
    m_masks.clear();
    m_clipVector = nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
StatusInt RasterClip::SetBoundary(CurveVectorP pBoundary)
    {
    if (nullptr == pBoundary)
        {
        m_pBoundary = nullptr;
        return SUCCESS;
        }

    if (CurveVector::BOUNDARY_TYPE_Outer != pBoundary->GetBoundaryType()) 
        return ERROR;

    m_pBoundary = pBoundary;
    m_clipVector = nullptr;
    
    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
StatusInt RasterClip::SetMasks(RasterClip::MaskVector const& masks)
    {
    for (auto const& mask : masks)
        {
        if (CurveVector::BOUNDARY_TYPE_Inner != mask->GetBoundaryType())
            return ERROR;
        }

    m_masks = masks;

    m_clipVector = nullptr;

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
StatusInt RasterClip::AddMask(CurveVectorR curve)
    {
    if (CurveVector::BOUNDARY_TYPE_Inner != curve.GetBoundaryType())
        return ERROR;

    m_masks.push_back(&curve);

    m_clipVector = nullptr;
    return SUCCESS;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
void RasterClip::ToBlob(bvector<uint8_t>& blob, DgnDbR dgndb) const
    {
    if (IsEmpty())
        {
        blob.clear();
        return;
        }

    // Create a parity region that will hold boundary and masks.
    CurveVectorPtr clips = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);

    if (HasBoundary())
        {
        // We make sure of that during set but just in case.
        BeAssert(CurveVector::BOUNDARY_TYPE_Outer == GetBoundaryCP()->GetBoundaryType());
        if (CurveVector::BOUNDARY_TYPE_Outer == GetBoundaryCP()->GetBoundaryType())
            clips->Add(m_pBoundary);
        }

    for (auto& mask : GetMasks())
        {
        // We make sure of that during set but just in case.
        BeAssert(CurveVector::BOUNDARY_TYPE_Inner == mask->GetBoundaryType());
        if (CurveVector::BOUNDARY_TYPE_Inner == mask->GetBoundaryType())
            clips->Add(mask);
        }

    BentleyGeometryFlatBuffer::GeometryToBytes(*clips, blob);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
void RasterClip::FromBlob(void const* blob, size_t size)
    {
    Clear();

    if (blob == nullptr || size == 0)
        return;

    CurveVectorPtr clips = BentleyGeometryFlatBuffer::BytesToCurveVector(static_cast<Byte const*>(blob));

    if (clips.IsNull())
        return;

    BeAssert(clips->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_ParityRegion);

    for (auto& loop : *clips)
        {
        CurveVectorPtr loopCurves = loop->GetChildCurveVectorP();
        if (loopCurves.IsNull())
            continue;

        BeAssert(loopCurves->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Outer || loopCurves->GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Inner);

        if (CurveVector::BOUNDARY_TYPE_Outer == loopCurves->GetBoundaryType())
            {
            BeAssert(!HasBoundary()); // only one is allowed.
            SetBoundary(loopCurves.get());
            }
        else if (CurveVector::BOUNDARY_TYPE_Inner == loopCurves->GetBoundaryType())
            {
            AddMask(*loopCurves);
            }
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
void RasterClip::InitFrom(RasterClip const& other)
    {
    Clear();

    if (other.HasBoundary())
        SetBoundary(other.GetBoundaryCP()->Clone().get());

    for (auto const& mask : other.GetMasks())
        {
        AddMask(*mask->Clone());
        }
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  8/2016
//----------------------------------------------------------------------------------------
ClipVectorCP RasterClip::GetClipVector() const
    {
    if (m_clipVector.IsValid())
        return m_clipVector.get();

    m_clipVector = nullptr;
         
    if (HasBoundary())
        {
        ClipPrimitivePtr boundary = ClipPrimitive::CreateFromBoundaryCurveVector(*GetBoundaryCP(), 0.1, 0.4, nullptr, nullptr, nullptr);
        m_clipVector = ClipVector::CreateFromPrimitive(boundary.get());
        }

    for (auto const& mask : GetMasks())
        {
        ClipPrimitivePtr clipMask = ClipPrimitive::CreateFromBoundaryCurveVector(*mask, 0.1, 0.4, nullptr, nullptr, nullptr);
        if (m_clipVector.IsValid())
            m_clipVector->push_back(clipMask);
        else
            m_clipVector = ClipVector::CreateFromPrimitive(clipMask.get());
        }

    return m_clipVector.get();
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterModel::RasterModel(CreateParams const& params) : T_Super (params)
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterModel::~RasterModel()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
AxisAlignedBox3d RasterModel::_QueryModelRange() const
    {
    auto rasterRoot = Load(nullptr);
    if (nullptr == rasterRoot)
        return AxisAlignedBox3d();

    ElementAlignedBox3d range = rasterRoot->ComputeRange();
    if (!range.IsValid())
        return AxisAlignedBox3d();

    Frustum box(range);
    box.Multiply(rasterRoot->GetLocation());

    AxisAlignedBox3d aaRange;
    aaRange.Extend(box.m_pts, 8);

    // low and high are not allowed to be equal
    fixRange(aaRange.low.x, aaRange.high.x);
    fixRange(aaRange.low.y, aaRange.high.y);
    fixRange(aaRange.low.z, aaRange.high.z);

    return aaRange;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void RasterModel::_OnFitView(Dgn::FitContextR context) 
    {
    auto rasterRoot = Load(nullptr);
    if (nullptr == rasterRoot)
        return;

    ElementAlignedBox3d rangeWorld = rasterRoot->ComputeRange();
    context.ExtendFitRange(rangeWorld, rasterRoot->GetLocation());
    }

#if 0 // This is how we pick a raster. We do not require this feature for now so disable it.
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void RasterModel::_DrawModel(Dgn::ViewContextR context)
    {
    if (context.GetDrawPurpose() == DrawPurpose::Pick)
        {
        auto rasterRoot = Load(nullptr);
        if (nullptr == rasterRoot)
            {
            RefCountedPtr<RasterBorderGeometrySource> pSource(new RasterBorderGeometrySource(rasterRoot.GetCorners(), *this));
            RefCountedPtr<RasterBorderGeometrySource::ElemTopology> pTopology = RasterBorderGeometrySource::ElemTopology::Create(*pSource);

            context.SetElemTopology(pTopology.get());
            context.VisitGeometry(*pSource);
            context.SetElemTopology(nullptr);
            }
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
RasterRootP RasterModel::Load(Dgn::Render::SystemP system) const
    {
    auto root = const_cast<RasterModel&>(*this).GetTileTree(system);
    return static_cast<RasterRootP>(root);
    }

#if defined(WIP_TILETREE_PUBLISH)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::TileTree::RootCPtr RasterModel::_GetPublishingTileTree(Dgn::Render::SystemP renderSys) const
    { 
    _Load(renderSys);

    return m_root.get();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::ClipVectorPtr RasterModel::_GetPublishingClip () const
    {
    ClipVectorCP    clip = GetClip().GetClipVector();

    return (nullptr == clip) ? nullptr : clip->Clone(nullptr);
    }
#endif

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
void RasterModel::ComputeDepthTransformation(TransformR transfo, ViewContextR context) const
    {
    auto rasterRoot = static_cast<RasterRootP>(m_root.get());
    BeAssert(nullptr != rasterRoot);

    transfo.InitIdentity();
    return;

#if 0 // NOT_NOW: setting a depth transfo may cause the raster to disappear because it is being clipped away by the view frustum.
    if (0.0 == GetDepthBias() || context.GetViewport() == nullptr || !IsParallelToGround())
        {
        transfo.InitIdentity();
        return;
        }
    
    static double s_depthFactor = 1.0;

    DVec3d viewZ;
    context.GetViewport()->GetRotMatrix().GetRow(viewZ, 2);

    DVec3d trans;
    trans.ScaleToLength(viewZ, GetDepthBias()*s_depthFactor);

    if (!context.IsCameraOn())
        {
        transfo.InitFrom(trans);
        return;
        }

    auto const& cam = context.GetViewport()->GetCamera();
    
    ElementAlignedBox3d box = rasterRoot->GetRootTile()->GetRange();

    DPoint3d lowerLeft = box.low;
    DPoint3d lowerRight = DPoint3d::From(box.high.x, box.low.y, box.low.z);
    DPoint3d topLeft = DPoint3d::From(box.low.x, box.high.y, box.low.z);    

    // Push raster corners toward the back of the viewport. 
    DPoint3d lowerLeftBias = DPoint3d::FromSumOf(lowerLeft, trans);
    DPoint3d lowerRightBias = DPoint3d::FromSumOf(lowerRight, trans);
    DPoint3d topLeftBias = DPoint3d::FromSumOf(topLeft, trans);    
        
    DPlane3d biasPlane = DPlane3d::From3Points(lowerLeftBias, lowerRightBias, topLeftBias);
    
    // Camera eye to corners rays.
    DRay3d lowerLeftRay = DRay3d::FromOriginAndTarget(cam.GetEyePoint(), lowerLeft);
    DRay3d lowerRightRay = DRay3d::FromOriginAndTarget(cam.GetEyePoint(), lowerRight);
    DRay3d topLeftRay = DRay3d::FromOriginAndTarget(cam.GetEyePoint(), topLeft);
    
    // Project corners to the bias plane in the eye-corners direction.
    double intParam;
    DPoint3d newCorners[3];
    if (!lowerLeftRay.Intersect(newCorners[0], intParam, biasPlane) ||
        !lowerRightRay.Intersect(newCorners[1], intParam, biasPlane) ||
        !topLeftRay.Intersect(newCorners[2], intParam, biasPlane))
        {
        transfo.InitIdentity();
        return;
        }

    Transform transA;
    transA.InitFromPlaneOf3Points(lowerLeft, lowerRight, topLeft);
    Transform transAInverse;
    transAInverse.InverseOf(transA);

    Transform transB;
    transB.InitFromPlaneOf3Points(newCorners[0], newCorners[1], newCorners[2]);
      
    transfo.InitProduct(transB, transAInverse);
#endif
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
RasterClipCR RasterModel::GetClip() const
    {
    return m_clips;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
void RasterModel::SetClip(RasterClipCR clip)
    {
    m_clips = clip;
    return;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
void RasterModel::SetClip(void const* blob, size_t size)
    {
    m_clips.FromBlob(blob, size);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
Utf8String RasterModel::GetDescription() const
    {
    RefCountedCPtr<RepositoryLink> pLink = ILinkElementBase<RepositoryLink>::Get(GetDgnDb(), GetModeledElementId());
    if (!pLink.IsValid())
        return "";

    return pLink->GetDescription();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
void RasterModel::_BindWriteParams(BeSQLite::EC::ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    // Shoud have been added by RasterModelHandler::_GetClassParams() if not make sure the handler is registered.
    BeAssert(stmt.GetParameterIndex(RASTER_MODEL_PROP_Clip) != -1); 

    bvector<uint8_t> clipData;
    m_clips.ToBlob(clipData, GetDgnDb());
    
    if(clipData.empty())
        stmt.BindNull(stmt.GetParameterIndex(RASTER_MODEL_PROP_Clip));
    else
        stmt.BindBlob(stmt.GetParameterIndex(RASTER_MODEL_PROP_Clip), clipData.data(), (int) clipData.size(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
DgnDbStatus RasterModel::_ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParamsCR params)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(statement, params);
    if (DgnDbStatus::Success != status)
        return status;

    // Shoud have been added by RasterModelHandler::_GetClassParams() if not make sure the handler is registred.
    BeAssert(params.GetSelectIndex(RASTER_MODEL_PROP_Clip) != -1);

    int blobSize = 0;
    void const* pBlob = statement.GetValueBlob(params.GetSelectIndex(RASTER_MODEL_PROP_Clip), &blobSize);

    m_clips.FromBlob(pBlob, blobSize);

    return DgnDbStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
void RasterModel::_InitFrom(DgnModelCR other)
    {
    T_Super::_InitFrom(other);
    RasterModel const* otherModel = dynamic_cast<RasterModel const*>(&other);
    if (nullptr != otherModel)
        m_clips.InitFrom(otherModel->m_clips);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
void RasterModelHandler::_GetClassParams(ECSqlClassParamsR params)
    {
    T_Super::_GetClassParams(params);
    params.Add(RASTER_MODEL_PROP_Clip, ECSqlClassParams::StatementType::All);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
void RasterModel::_OnSaveJsonProperties()
    {
    SetJsonProperties(json_depthBias(), m_depthBias);
    T_Super::_OnSaveJsonProperties();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2016
//----------------------------------------------------------------------------------------
void RasterModel::_OnLoadedJsonProperties()
    {
    m_depthBias = GetJsonProperties(json_depthBias()).asDouble();
    T_Super::_OnLoadedJsonProperties();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2016
//----------------------------------------------------------------------------------------
bool RasterModel::IsParallelToGround() const {return _IsParallelToGround();}



