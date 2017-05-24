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
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet Alignment::QueryReferingSpatialElements() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT SourceECInstanceId FROM " BRRA_SCHEMA(BRRA_REL_SpatialElementRefersToAlignment) " WHERE TargetECInstanceId = ?;");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, GetElementId());

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.insert(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentCPtr Alignment::QueryHorizontal() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BRRA_SCHEMA(BRRA_REL_AlignmentRefersToHorizontal) " WHERE SourceECInstanceId = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return nullptr;

    return HorizontalAlignment::Get(GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignmentCPtr Alignment::QueryMainVertical() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BRRA_SCHEMA(BRRA_REL_AlignmentRefersToMainVertical) " WHERE SourceECInstanceId = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    DgnElementId verticalId;
    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return nullptr;

    return VerticalAlignment::Get(GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Alignment::SetHorizontal(AlignmentCR alignment, HorizontalAlignmentCR vertical)
    {
    auto stmtDelPtr = alignment.GetDgnDb().GetPreparedECSqlStatement("SELECT ECClassId, ECInstanceId FROM " BRRA_SCHEMA(BRRA_REL_AlignmentRefersToHorizontal) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtDelPtr.IsValid());

    stmtDelPtr->BindId(1, alignment.GetElementId());
    if (DbResult::BE_SQLITE_ROW == stmtDelPtr->Step())
        {
        if (DbResult::BE_SQLITE_OK != alignment.GetDgnDb().DeleteNonNavigationRelationship(
            ECInstanceKey(stmtDelPtr->GetValueId<ECClassId>(0), stmtDelPtr->GetValueId<ECInstanceId>(1))))
            return DgnDbStatus::BadElement;
        }

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != alignment.GetDgnDb().InsertNonNavigationRelationship(insKey,
        *alignment.GetDgnDb().Schemas().GetClass(BRRA_SCHEMA_NAME, BRRA_REL_AlignmentRefersToHorizontal)->GetRelationshipClassCP(),
        ECInstanceId(alignment.GetElementId().GetValue()), ECInstanceId(vertical.GetElementId().GetValue())))
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Alignment::SetMainVertical(AlignmentCR alignment, VerticalAlignmentCR vertical)
    {
    auto stmtDelPtr = alignment.GetDgnDb().GetPreparedECSqlStatement("SELECT ECClassId, ECInstanceId FROM " BRRA_SCHEMA(BRRA_REL_AlignmentRefersToMainVertical) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtDelPtr.IsValid());

    stmtDelPtr->BindId(1, alignment.GetElementId());
    if (DbResult::BE_SQLITE_ROW == stmtDelPtr->Step())
        {
        if (DbResult::BE_SQLITE_OK != alignment.GetDgnDb().DeleteNonNavigationRelationship(
            ECInstanceKey(stmtDelPtr->GetValueId<ECClassId>(0), stmtDelPtr->GetValueId<ECInstanceId>(1))))
            return DgnDbStatus::BadElement;
        }
     
    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != alignment.GetDgnDb().InsertNonNavigationRelationship(insKey,
        *alignment.GetDgnDb().Schemas().GetClass(BRRA_SCHEMA_NAME, BRRA_REL_AlignmentRefersToMainVertical)->GetRelationshipClassCP(),
        ECInstanceId(alignment.GetElementId().GetValue()), ECInstanceId(vertical.GetElementId().GetValue())))
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
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

        if (alignmentPair.VerticalCurveVector().IsValid())
            {
            auto verticalModelPtr = VerticalAlignmentModel::Create(VerticalAlignmentModel::CreateParams(GetDgnDb(), GetElementId()));

            if (DgnDbStatus::Success != verticalModelPtr->Insert())
                return nullptr;

            auto verticalAlignmPtr = VerticalAlignment::Create(*verticalModelPtr, *alignmentPair.VerticalCurveVector());
            if (verticalAlignmPtr->InsertAsMainVertical(stat).IsNull())
                return nullptr;
            }
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentCPtr Alignment::UpdateWithMainPair(AlignmentPairCR alignmentPair, DgnDbStatus* stat)
    {
    if (alignmentPair.HorizontalCurveVector().IsNull())
        return nullptr;

    auto retVal = Update(stat);
    if (retVal.IsValid())
        {
        HorizontalAlignmentPtr horizAlignmPtr = dynamic_cast<HorizontalAlignmentP>(QueryHorizontal()->CopyForEdit().get());
        horizAlignmPtr->SetGeometry(*alignmentPair.HorizontalCurveVector());
        if (horizAlignmPtr->Update(stat).IsNull())
            return nullptr;

        auto vertAlignmCPtr = QueryMainVertical();

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
                    retVal->GetElementId()));

                if (DgnDbStatus::Success != verticalModelPtr->Insert())
                    return nullptr;

                auto verticalAlignmPtr = VerticalAlignment::Create(*verticalModelPtr, *alignmentPair.VerticalCurveVector());
                if (verticalAlignmPtr->InsertAsMainVertical(stat).IsNull())
                    return nullptr;
                }
            }
        // Updated geometry doesn't have a vertical
        else
            {
            // Main vertical exists... delete it
            if (vertAlignmCPtr.IsValid())
                *stat = vertAlignmCPtr->Delete();
            }
        }

    return retVal;
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
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentCPtr HorizontalAlignment::Insert(DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<HorizontalAlignment>(*this, stat);
    if (retVal.IsValid() && m_alignmentId.IsValid())
        Alignment::SetHorizontal(*Alignment::Get(GetDgnDb(), m_alignmentId), *retVal);

    return retVal;
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
VerticalAlignmentCPtr VerticalAlignment::InsertAsMainVertical(Dgn::DgnDbStatus* stat)
    {
    auto retValPtr = Insert(stat);
    DgnDbStatus status = Alignment::SetMainVertical(GetAlignment(), *retValPtr);
    BeAssert(DgnDbStatus::Success == status);

    return retValPtr;
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