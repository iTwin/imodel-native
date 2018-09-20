/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterHandler.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RasterInternal.h>
#include <Raster/RasterHandler.h>
#include "RasterTileTree.h"

#define RASTER_MODEL_PROP_Clip "Clip"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_RASTER

HANDLER_DEFINE_MEMBERS(RasterModelHandler)

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



