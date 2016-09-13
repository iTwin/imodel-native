/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterSchemaInternal.h>
#include <RasterSchema/RasterHandler.h>
#include "RasterSource.h"
#include "RasterTileTree.h"
#include "WmsSource.h"

#define RASTER_MODEL_PROP_Clip "Clip"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RASTERSCHEMA

HANDLER_DEFINE_MEMBERS(RasterModelHandler)

//----------------------------------------------------------------------------------------
// @bsiclass                                                      Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
struct RasterBorderGeometrySource : public GeometrySource3d, RefCountedBase
    {
    struct ElemTopology : RefCounted<IElemTopology>
        {
        protected:
            RefCountedPtr<RasterBorderGeometrySource> m_source;

            virtual IElemTopologyP _Clone() const override { return new ElemTopology(*this); }
            virtual bool _IsEqual(IElemTopologyCR rhs) const override { return _ToGeometrySource() == rhs._ToGeometrySource(); }
            virtual GeometrySourceCP _ToGeometrySource() const override { return m_source.get(); }
            virtual IEditManipulatorPtr _GetTransientManipulator(HitDetailCR hit) const { return nullptr; /*TODO*/ }

            ElemTopology(RasterBorderGeometrySource& source) { m_source = &source; }
            ElemTopology(ElemTopology const& from) { m_source = from.m_source; }

        public:

            static RefCountedPtr<ElemTopology> Create(RasterBorderGeometrySource& source) { return new ElemTopology(source); }

        }; // ElemTopology

    RasterBorderGeometrySource(DPoint3dCP corners, RasterModel& model);

    virtual DgnDbR _GetSourceDgnDb() const override { return m_dgnDb; }
    virtual DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    virtual Placement3dCR _GetPlacement() const override { return m_placement; }
    virtual GeometryStreamCR _GetGeometryStream() const override { return m_geom; }
    virtual Render::GraphicSet& _Graphics() const override { return m_graphics; };
    virtual DgnElement::Hilited _IsHilited() const override { return m_hilited; }
    virtual void _GetInfoString(HitDetailCR, Utf8StringR descr, Utf8CP delimiter) const override { descr = m_infoString; }

    virtual DgnElementCP _ToElement() const override { return nullptr; }
    virtual GeometrySource3dCP _ToGeometrySource3d() const override { return this; }
    virtual DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { return DgnDbStatus::Success; }
    virtual DgnDbStatus _SetPlacement(Placement3dCR placement) override { return DgnDbStatus::Success; }
    virtual void _SetHilited(DgnElement::Hilited newState) const override { m_hilited = newState; }

    DgnDbR                      m_dgnDb;
    DgnCategoryId               m_categoryId;
    Placement3d                 m_placement;
    GeometryStream              m_geom;
    Utf8String                  m_infoString;
    mutable Render::GraphicSet  m_graphics;
    mutable DgnElement::Hilited m_hilited;
    };


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
RasterBorderGeometrySource::RasterBorderGeometrySource(DPoint3dCP pCorners, RasterModel& model)
    :m_dgnDb(model.GetDgnDb()),
    m_categoryId(DgnCategory::QueryFirstCategoryId(model.GetDgnDb())),
    m_hilited(DgnElement::Hilited::None),
    m_infoString(model.GetUserLabel())
    {
    if (m_infoString.empty())
        m_infoString = model.GetCode().GetValueCP();

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

    //&&MM validate. The Z-value?
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
        //&&MM validate. The Z-value?
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
    //&&MM validate. The Z-value?
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
void RasterClip::FromBlob(void const* blob, size_t size, DgnDbR dgndb)
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
    //m_loadStatus = LoadRasterStatus::Unloaded;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                       Eric.Paquet     4/2015
//----------------------------------------------------------------------------------------
RasterModel::~RasterModel()
    {
    // Wait for tasks that we may have queued. 
    //&&MM bogus in WaitForIdle it will deadlock if task queue is not empty.
    BeFolly::IOThreadPool::GetPool().WaitForIdle();

    m_root = nullptr;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  8/2016
//----------------------------------------------------------------------------------------
DMatrix4dCR  RasterModel::GetSourceToWorld() const { return _GetSourceToWorld(); }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  2/2016
//----------------------------------------------------------------------------------------
void RasterModel::_DropGraphicsForViewport(DgnViewportCR viewport)
    {
    //&&MM todo
//     if (m_rasterTreeP.IsValid())
//         m_rasterTreeP->DropGraphicsForViewport(viewport);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void RasterModel::_OnFitView(Dgn::FitContextR context) 
    {
    _Load(nullptr);
    if (!m_root.IsValid())
        return;

    ElementAlignedBox3d rangeWorld = m_root->ComputeRange();
    context.ExtendFitRange(rangeWorld, m_root->GetLocation());
    }

#if 0 // This is how we pick a raster. We do not require this feature for now so disable it.
//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void RasterModel::_DrawModel(Dgn::ViewContextR context)
    {
    if (context.GetDrawPurpose() == DrawPurpose::Pick)
        {
        _Load(nullptr);
        if (m_root.IsValid())
            {
            RefCountedPtr<RasterBorderGeometrySource> pSource(new RasterBorderGeometrySource(m_root.GetCorners(), *this));
            RefCountedPtr<RasterBorderGeometrySource::ElemTopology> pTopology = RasterBorderGeometrySource::ElemTopology::Create(*pSource);

            context.SetElemTopology(pTopology.get());
            context.VisitGeometry(*pSource);
            context.SetElemTopology(nullptr);
            }
        }
    }
#endif

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2016
//----------------------------------------------------------------------------------------
void RasterModel::_AddTerrainGraphics(TerrainContextR context) const
    {
    _Load(&context.GetTargetR().GetSystem());

    if (!m_root.IsValid() || !m_root->GetRootTile().IsValid())
        {
        BeAssert(false);
        return;
        }

    auto now = std::chrono::steady_clock::now();
    TileTree::DrawArgs args(context, m_root->GetLocation(), now, now - m_root->GetExpirationTime());
    m_root->Draw(args);

    DEBUG_PRINTF("Map draw %d graphics, %d total, %d missing ", args.m_graphics.m_entries.size(), m_root->GetRootTile()->CountTiles(), args.m_missing.size());

    args.DrawGraphics(context);

    if (!args.m_missing.empty())
        {
        args.RequestMissingTiles(*m_root);
        context.GetViewport()->ScheduleTerrainProgressiveTask(*new RasterProgressive(*m_root, args.m_missing));
        }
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
DgnDbStatus RasterModel::BindInsertAndUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    // Shoud have been added by RasterModelHandler::_GetClassParams() if not make sure the handler is registred.
    BeAssert(statement.GetParameterIndex(RASTER_MODEL_PROP_Clip) != -1); 

    bvector<uint8_t> clipData;
    m_clips.ToBlob(clipData, GetDgnDb());
    
    if(clipData.empty())
        statement.BindNull(statement.GetParameterIndex(RASTER_MODEL_PROP_Clip));
    else
        statement.BindBinary(statement.GetParameterIndex(RASTER_MODEL_PROP_Clip), clipData.data(), (int) clipData.size(), BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);

    return DgnDbStatus::Success;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
DgnDbStatus RasterModel::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    T_Super::_BindInsertParams(statement);
    return BindInsertAndUpdateParams(statement);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  7/2016
//----------------------------------------------------------------------------------------
DgnDbStatus RasterModel::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    T_Super::_BindUpdateParams(statement);
    return BindInsertAndUpdateParams(statement);
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
    void const* pBlob = statement.GetValueBinary(params.GetSelectIndex(RASTER_MODEL_PROP_Clip), &blobSize);

    m_clips.FromBlob(pBlob, blobSize, GetDgnDb());

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
