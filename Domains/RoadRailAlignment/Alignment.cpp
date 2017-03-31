/*--------------------------------------------------------------------------------------+
|
|     $Source: Alignment.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

HANDLER_DEFINE_MEMBERS(AlignmentHandler)
HANDLER_DEFINE_MEMBERS(AlignmentHorizontalHandler)
HANDLER_DEFINE_MEMBERS(AlignmentVerticalHandler)

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
AlignmentHorizontalCPtr Alignment::QueryHorizontal() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BRRA_SCHEMA(BRRA_CLASS_AlignmentHorizontal) " WHERE Parent.Id = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return nullptr;

    return AlignmentHorizontal::Get(GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentVerticalCPtr Alignment::QueryMainVertical() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BRRA_SCHEMA(BRRA_REL_AlignmentRefersToMainVertical) " WHERE SourceECInstanceId = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    DgnElementId verticalId;
    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return nullptr;

    return AlignmentVertical::Get(GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Alignment::SetMainVertical(AlignmentCR alignment, AlignmentVerticalCR vertical)
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
DgnElementIdSet Alignment::QueryAlignmentVerticalIds() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT ECInstanceId FROM " BRRA_SCHEMA(BRRA_CLASS_AlignmentVertical) " WHERE Parent.Id = ?");
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
        BRRA_SCHEMA(BRRA_CLASS_AlignmentHorizontal) " horiz LEFT JOIN "
        BRRA_SCHEMA(BRRA_REL_AlignmentRefersToMainVertical) " mainVert ON horiz.Parent.Id = mainVert.SourceECInstanceId LEFT JOIN "
        BRRA_SCHEMA(BRRA_CLASS_AlignmentVertical) " vert ON mainVert.TargetECInstanceId = vert.ECInstanceId "
        "WHERE horiz.Parent.Id = ?;");
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
        auto horizAlignmPtr = AlignmentHorizontal::Create(*retVal, *alignmentPair.HorizontalCurveVector());
        if (horizAlignmPtr->Insert(stat).IsNull())
            return nullptr;

        if (alignmentPair.VerticalCurveVector().IsValid())
            {
            auto verticalAlignmPtr = AlignmentVertical::Create(*retVal, *alignmentPair.VerticalCurveVector());
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
        AlignmentHorizontalPtr horizAlignmPtr = dynamic_cast<AlignmentHorizontalP>(QueryHorizontal()->CopyForEdit().get());
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
                AlignmentVerticalPtr vertAlignmPtr = dynamic_cast<AlignmentVerticalP>(vertAlignmCPtr->CopyForEdit().get());
                vertAlignmPtr->SetGeometry(*alignmentPair.VerticalCurveVector());
                if (vertAlignmPtr->Update(stat).IsNull())
                    return nullptr;
                }
            // Main vertical doesn't exist... add it
            else
                {
                auto verticalAlignmPtr = AlignmentVertical::Create(*retVal, *alignmentPair.VerticalCurveVector());
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
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentHorizontal::AlignmentHorizontal(CreateParams const& params, CurveVectorR geometry):
    T_Super(params)
    {
    SetGeometry(geometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentHorizontalPtr AlignmentHorizontal::Create(AlignmentCR alignment, CurveVectorR horizontalGeometry)
    {
    if (!alignment.GetElementId().IsValid())
        return nullptr;

    CreateParams createParams(alignment.GetDgnDb(), alignment.GetModelId(), QueryClassId(alignment.GetDgnDb()), AlignmentCategory::Get(alignment.GetDgnDb()));
    createParams.SetParentId(alignment.GetElementId(), DgnClassId(alignment.GetDgnDb().Schemas().GetClassId(BRRA_SCHEMA_NAME, BRRA_REL_AlignmentOwnsHorizontal)));

    return new AlignmentHorizontal(createParams, horizontalGeometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorCR AlignmentHorizontal::GetGeometry() const
    {
    ECValue val;
    if (DgnDbStatus::Success != GetPropertyValue(val, BRRA_PROP_AlignmentHorizontal_HorizontalGeometry))
        {
        BeAssert(false);
        }

    BeAssert(val.IsIGeometry());

    return *val.GetIGeometry()->GetAsCurveVector();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentHorizontal::SetGeometry(CurveVectorR geometry)
    {
    CurveVectorPtr cvPtr = &geometry;

    ECValue val(PrimitiveType::PRIMITIVETYPE_IGeometry);
    val.SetIGeometry(*IGeometry::Create(cvPtr));

    SetPropertyValue(BRRA_PROP_AlignmentHorizontal_HorizontalGeometry, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentVerticalPtr AlignmentVertical::Create(AlignmentCR alignment, CurveVectorR verticalGeometry)
    {
    if (!alignment.GetElementId().IsValid())
        return nullptr;

    CreateParams createParams(alignment.GetDgnDb(), alignment.GetModelId(), QueryClassId(alignment.GetDgnDb()), AlignmentCategory::Get(alignment.GetDgnDb()));
    createParams.SetParentId(alignment.GetElementId(), DgnClassId(alignment.GetDgnDb().Schemas().GetClassId(BRRA_SCHEMA_NAME, BRRA_REL_AlignmentOwnsVerticals)));

    return new AlignmentVertical(createParams, verticalGeometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentVertical::AlignmentVertical(CreateParams const& params, CurveVectorR geometry):
    T_Super(params)
    {
    SetGeometry(geometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentVerticalCPtr AlignmentVertical::InsertAsMainVertical(Dgn::DgnDbStatus* stat)
    {
    auto retValPtr = Insert(stat);
    DgnDbStatus status = Alignment::SetMainVertical(GetAlignment(), *retValPtr);
    BeAssert(DgnDbStatus::Success == status);

    return retValPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorCR AlignmentVertical::GetGeometry() const
    {
    ECValue val;
    if (DgnDbStatus::Success != GetPropertyValue(val, BRRA_PROP_AlignmentVertical_VerticalGeometry))
        {
        BeAssert(false);
        }

    BeAssert(val.IsIGeometry());

    return *val.GetIGeometry()->GetAsCurveVector();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentVertical::SetGeometry(CurveVectorR geometry)
    {
    CurveVectorPtr cvPtr = &geometry;

    ECValue val(PrimitiveType::PRIMITIVETYPE_IGeometry);
    val.SetIGeometry(*IGeometry::Create(cvPtr));

    SetPropertyValue(BRRA_PROP_AlignmentVertical_VerticalGeometry, val);
    }