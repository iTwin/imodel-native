/*--------------------------------------------------------------------------------------+
|
|     $Source: Alignment.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

HANDLER_DEFINE_MEMBERS(AlignmentHandler)
HANDLER_DEFINE_MEMBERS(HorizontalAlignmentsPortionHandler)
HANDLER_DEFINE_MEMBERS(HorizontalAlignmentHandler)
HANDLER_DEFINE_MEMBERS(VerticalAlignmentHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPtr Alignment::Create(AlignmentModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    return new Alignment(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), AlignmentCategory::Get(model.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double Alignment::_GetLength() const
    {
    return GetHorizontal()->GetGeometry().Length();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet Alignment::QueryVerticalAlignmentIds() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT va.ECInstanceId FROM " 
        BRRA_SCHEMA(BRRA_CLASS_VerticalAlignment) " va, " BIS_SCHEMA(BIS_CLASS_Model) " m "
        "WHERE m.ECInstanceId = va.Model.Id AND m.ModeledElement.Id = ?");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, GetElementId());

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.insert(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr Alignment::QueryMainPair() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT horiz.HorizontalGeometry, vert.VerticalGeometry FROM "
        BRRA_SCHEMA(BRRA_REL_AlignmentRefersToHorizontal) " arh INNER JOIN "
        BRRA_SCHEMA(BRRA_CLASS_HorizontalAlignment) " horiz ON horiz.ECInstanceId = arh.TargetECInstanceId LEFT JOIN "
        BRRA_SCHEMA(BRRA_REL_AlignmentRefersToMainVertical) " mainVert ON arh.SourceECInstanceId = mainVert.SourceECInstanceId LEFT JOIN "
        BRRA_SCHEMA(BRRA_CLASS_VerticalAlignment) " vert ON mainVert.TargetECInstanceId = vert.ECInstanceId "
        "WHERE arh.SourceECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return nullptr;

    auto horizVectorPtr = stmtPtr->GetValueGeometry(0)->GetAsCurveVector();

    CurveVectorPtr vertVectorPtr;
    if (!stmtPtr->IsValueNull(1))
        vertVectorPtr = stmtPtr->GetValueGeometry(1)->GetAsCurveVector();

    return AlignmentPair::Create(*horizVectorPtr, vertVectorPtr.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentCPtr Alignment::InsertWithMainPair(AlignmentPairCR alignmentPair, DgnDbStatus* stat)
    {
    if (alignmentPair.HorizontalCurveVector().IsNull())
        return nullptr;

    auto retVal = Insert(stat);
    if (retVal.IsValid())
        {
        auto horizAlignmPtr = HorizontalAlignment::Create(*this, *alignmentPair.HorizontalCurveVector());
        if (horizAlignmPtr->Insert(stat).IsNull())
            return nullptr;

        SetHorizontal(*horizAlignmPtr);

        if (alignmentPair.VerticalCurveVector().IsValid())
            {
            auto verticalModelPtr = VerticalAlignmentModel::Create(VerticalAlignmentModel::CreateParams(GetDgnDb(), GetElementId()));

            if (DgnDbStatus::Success != verticalModelPtr->Insert())
                return nullptr;

            auto verticalAlignmPtr = VerticalAlignment::Create(*verticalModelPtr, *alignmentPair.VerticalCurveVector());
            if (verticalAlignmPtr->Insert(stat).IsNull())
                return nullptr;

            SetMainVertical(*verticalAlignmPtr);
            }

        retVal = Update(stat);
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentCPtr Alignment::UpdateWithMainPair(AlignmentPairCR alignmentPair, DgnDbStatus* stat)
    {
    if (!GetElementId().IsValid())
        return nullptr;

    if (alignmentPair.HorizontalCurveVector().IsNull())
        return nullptr;

    HorizontalAlignmentPtr horizAlignmPtr = dynamic_cast<HorizontalAlignmentP>(GetHorizontal()->CopyForEdit().get());
    horizAlignmPtr->SetGeometry(*alignmentPair.HorizontalCurveVector());
    if (horizAlignmPtr->Update(stat).IsNull())
        return nullptr;

    auto vertAlignmCPtr = GetMainVertical();

    // Updated geometry has a vertical
    if (alignmentPair.VerticalCurveVector().IsValid())
        {
        // Main vertical exists... update it
        if (vertAlignmCPtr.IsValid())
            {
            VerticalAlignmentPtr vertAlignmPtr = dynamic_cast<VerticalAlignmentP>(vertAlignmCPtr->CopyForEdit().get());
            vertAlignmPtr->SetGeometry(*alignmentPair.VerticalCurveVector());
            if (vertAlignmPtr->Update(stat).IsNull())
                return nullptr;
            }
        // Main vertical doesn't exist... add it
        else
            {
            // TODO: check if vertical model breakdown exists.. if it does, reuse it
            auto verticalModelPtr = VerticalAlignmentModel::Create(DgnModel::CreateParams(GetDgnDb(), VerticalAlignmentModel::QueryClassId(GetDgnDb()),
                GetElementId()));

            if (DgnDbStatus::Success != verticalModelPtr->Insert())
                return nullptr;

            auto verticalAlignmPtr = VerticalAlignment::Create(*verticalModelPtr, *alignmentPair.VerticalCurveVector());
            if (verticalAlignmPtr->Insert(stat).IsNull())
                return nullptr;

            SetMainVertical(*verticalAlignmPtr);
            }
        }
    // Updated geometry doesn't have a vertical
    else
        {
        // Main vertical exists... delete it
        if (vertAlignmCPtr.IsValid())
            *stat = vertAlignmCPtr->Delete();
        }

    return Update(stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Alignment::GenerateAprox3dGeom()
    {
    auto horizAlignmentCPtr = HorizontalAlignment::Get(GetDgnDb(), GetHorizontalId());
    if (horizAlignmentCPtr.IsNull())
        return DgnDbStatus::MissingId;

    auto& horizGeometryCR = horizAlignmentCPtr->GetGeometry();
    DPoint3d origin = { 0, 0, 0 };
    auto geomBuilderPtr = GeometryBuilder::Create(*GetModel(), GetCategoryId(), origin);
    if (!geomBuilderPtr->Append(horizGeometryCR, GeometryBuilder::CoordSystem::World))
        return DgnDbStatus::NoGeometry;

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(*this))
        return DgnDbStatus::NoGeometry;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentCPtr Alignment::GetHorizontal() const 
    { 
    return HorizontalAlignment::Get(GetDgnDb(), GetHorizontalId()); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignmentCPtr Alignment::GetMainVertical() const 
    { 
    return VerticalAlignment::Get(GetDgnDb(), GetMainVerticalId()); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Alignment::SetHorizontal(HorizontalAlignmentCR horizontal)
    { 
    return SetPropertyValue("Horizontal", ECValue(horizontal.GetElementId())); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Alignment::SetMainVertical(VerticalAlignmentCR vertical)
    { 
    if (vertical.GetModel()->GetModeledElementId() != GetElementId())
        return DgnDbStatus::InvalidId;

    return SetPropertyValue("MainVertical", ECValue(vertical.GetElementId())); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentsPortionCPtr HorizontalAlignmentsPortion::InsertPortion(AlignmentModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    if (model.QueryHorizontalPartition().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), AlignmentCategory::Get(model.GetDgnDb()));
    HorizontalAlignmentsPortionPtr newPortionPtr(new HorizontalAlignmentsPortion(createParams));
    return model.GetDgnDb().Elements().Insert<HorizontalAlignmentsPortion>(*newPortionPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignment::HorizontalAlignment(CreateParams const& params, AlignmentCR alignment, CurveVectorR geometry):
    T_Super(params), m_alignmentId(alignment.GetElementId())
    {
    SetGeometry(geometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentPtr HorizontalAlignment::Create(AlignmentCR alignment, CurveVectorR horizontalGeometry)
    {
    if (!alignment.GetElementId().IsValid())
        return nullptr;

    auto horizontalModelId = HorizontalAlignmentModel::QueryBreakDownModelId(*alignment.GetAlignmentModel());
    if (!horizontalModelId.IsValid())
        return nullptr;

    auto breakDownModelCPtr = HorizontalAlignmentModel::Get(alignment.GetDgnDb(), horizontalModelId);

    CreateParams createParams(alignment.GetDgnDb(), breakDownModelCPtr->GetModelId(),
        QueryClassId(alignment.GetDgnDb()), AlignmentCategory::Get(alignment.GetDgnDb()));
    return new HorizontalAlignment(createParams, alignment, horizontalGeometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorCR HorizontalAlignment::GetGeometry() const
    {
    if (m_geometry.IsNull())
        {
        ECValue val;
        if (DgnDbStatus::Success != GetPropertyValue(val, BRRA_PROP_HorizontalAlignment_HorizontalGeometry))
            {
            BeAssert(false);
            }

        BeAssert(val.IsIGeometry());
        m_geometry = val.GetIGeometry()->GetAsCurveVector();
        }

    return *m_geometry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HorizontalAlignment::SetGeometry(CurveVectorR geometry)
    {
    m_geometry = nullptr;

    CurveVectorPtr cvPtr = &geometry;

    ECValue val(PrimitiveType::PRIMITIVETYPE_IGeometry);
    val.SetIGeometry(*IGeometry::Create(cvPtr));

    SetPropertyValue(BRRA_PROP_HorizontalAlignment_HorizontalGeometry, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus HorizontalAlignment::GenerateElementGeom()
    {
    DPoint2d origin = { 0, 0 };
    auto geomBuilderPtr = GeometryBuilder::Create(*GetModel(), GetCategoryId(), origin);
    if (!geomBuilderPtr->Append(GetGeometry(), GeometryBuilder::CoordSystem::World))
        return DgnDbStatus::NoGeometry;

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(*this))
        return DgnDbStatus::NoGeometry;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignmentPtr VerticalAlignment::Create(VerticalAlignmentModelCR breakDownModel, CurveVectorR verticalGeometry)
    {
    CreateParams createParams(breakDownModel.GetDgnDb(), breakDownModel.GetModelId(), 
        QueryClassId(breakDownModel.GetDgnDb()), AlignmentCategory::Get(breakDownModel.GetDgnDb()));
    return new VerticalAlignment(createParams, verticalGeometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignment::VerticalAlignment(CreateParams const& params, CurveVectorR geometry):
    T_Super(params)
    {
    SetGeometry(geometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr VerticalAlignment::GetGeometry() const
    {
    ECValue val;
    if (DgnDbStatus::Success != GetPropertyValue(val, BRRA_PROP_VerticalAlignment_VerticalGeometry))
        {
        BeAssert(false);
        }

    BeAssert(val.IsIGeometry());

    return val.GetIGeometry()->GetAsCurveVector();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void VerticalAlignment::SetGeometry(CurveVectorR geometry)
    {
    CurveVectorPtr cvPtr = &geometry;

    ECValue val(PrimitiveType::PRIMITIVETYPE_IGeometry);
    val.SetIGeometry(*IGeometry::Create(cvPtr));

    SetPropertyValue(BRRA_PROP_VerticalAlignment_VerticalGeometry, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus VerticalAlignment::GenerateElementGeom()
    {
    DPoint2d origin = { 0, 0 };
    auto geomBuilderPtr = GeometryBuilder::Create(*GetModel(), GetCategoryId(), origin);
    if (!geomBuilderPtr->Append(*GetGeometry(), GeometryBuilder::CoordSystem::World))
        return DgnDbStatus::NoGeometry;

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(*this))
        return DgnDbStatus::NoGeometry;

    return DgnDbStatus::Success;
    }