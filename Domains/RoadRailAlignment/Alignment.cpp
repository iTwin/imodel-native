/*--------------------------------------------------------------------------------------+
|
|     $Source: Alignment.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/Alignment.h>

HANDLER_DEFINE_MEMBERS(AlignmentHandler)
HANDLER_DEFINE_MEMBERS(HorizontalAlignmentsPortionHandler)
HANDLER_DEFINE_MEMBERS(HorizontalAlignmentHandler)
HANDLER_DEFINE_MEMBERS(VerticalAlignmentHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Alignment::Alignment(CreateParams const& params) : 
    T_Super(params) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPtr Alignment::Create(AlignmentModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    AlignmentPtr retVal(new Alignment(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), AlignmentCategory::Get(model.GetDgnDb()))));
    retVal->SetStartValue(0.0);
    retVal->SetStartStation(0.0);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double Alignment::_GetLength() const
    {
    return QueryHorizontal()->GetGeometry().Length();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d Alignment::_ToDPoint3d(DistanceExpressionCR distanceExpression) const
    {
    auto mainPairPtr = QueryMainPair();

    DPoint3d retVal;
    if (distanceExpression.GetLateralOffsetFromILinearElement().IsValid())
        {
        retVal = mainPairPtr->GetPointAtAndOffset(
            distanceExpression.GetDistanceAlongFromStart(), 
            distanceExpression.GetLateralOffsetFromILinearElement().Value());
        retVal.z = mainPairPtr->GetVerticalElevationAt(distanceExpression.GetDistanceAlongFromStart());
        }
    else
        {
        retVal = mainPairPtr->GetPointAtWithZ(distanceExpression.GetDistanceAlongFromStart());
        }

    if (distanceExpression.GetVerticalOffsetFromILinearElement().IsValid())
        retVal.z += distanceExpression.GetVerticalOffsetFromILinearElement().Value();

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DistanceExpression Alignment::_ToDistanceExpression(DPoint3dCR point) const
    {
    auto mainPairPtr = QueryMainPair();

    double horizOffset;
    double distanceFromStart = mainPairPtr->HorizontalDistanceAlongFromStart(point, &horizOffset);
    double vertElev = mainPairPtr->GetVerticalElevationAt(distanceFromStart);
    double vertOffset = (point.z - vertElev);

    return DistanceExpression(distanceFromStart, horizOffset, vertOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Alignment::_OnDelete() const
    {
    DgnDbStatus retVal = DgnDbStatus::Success;
    auto horizontalCPtr = QueryHorizontal();
    if (horizontalCPtr.IsValid())
        {
        if (DgnDbStatus::Success != (retVal = horizontalCPtr->Delete()))
            return retVal;
        }

    auto subModelId = QueryVerticalAlignmentSubModelId();
    if (subModelId.IsValid())
        {
        auto subModelPtr = VerticalAlignmentModel::GetForEdit(GetDgnDb(), subModelId);
        if (DgnDbStatus::Success != (retVal = subModelPtr->Delete()))
            return retVal;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId Alignment::QueryVerticalAlignmentSubModelId() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT ECInstanceId FROM " BRRA_SCHEMA(BRRA_CLASS_VerticalAlignmentModel) " WHERE ModeledElement.Id = ?");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, GetElementId());
    if (DbResult::BE_SQLITE_ROW != stmt.Step())
        return DgnModelId();

    return stmt.GetValueId<DgnModelId>(0);
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
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId Alignment::QueryMainVerticalId() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BRRA_SCHEMA(BRRA_REL_AlignmentRefersToMainVertical) " WHERE SourceECInstanceId = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    DgnElementId verticalId;
    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnElementId();

    return stmtPtr->GetValueId<DgnElementId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignmentCPtr Alignment::QueryMainVertical() const
    { 
    return VerticalAlignment::Get(GetDgnDb(), QueryMainVerticalId()); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Alignment::DistanceAlongStationPair> Alignment::QueryOrderedStations() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT atLocation.AtPosition.DistanceAlongFromStart, station.Station FROM " 
        BRRA_SCHEMA(BRRA_CLASS_AlignmentStation) " station, " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedAtLocation) " atLocation "
        "WHERE station.ECInstanceId = atLocation.Element.Id AND station.Parent.Id = ?");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, GetElementId());

    bvector<DistanceAlongStationPair> retVal;

    double lastDistanceAlong = 0;
    double lastStation = GetStartStation();
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        double distanceAlong = stmt.GetValueDouble(0);
        if (retVal.empty() && fabs(distanceAlong) > DBL_EPSILON)
            retVal.push_back({ 0, lastStation });

        double station = stmt.GetValueDouble(1);
        retVal.push_back({ distanceAlong, station });
        lastDistanceAlong = distanceAlong;
        lastStation = station;
        }

    if (retVal.empty())
        retVal.push_back({ 0, lastStation });

    double length = GetLength();
    if (fabs(lastDistanceAlong - length) > DBL_EPSILON)
        retVal.push_back({ length, lastStation + (length - lastDistanceAlong) });

    return retVal;
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
        if (DbResult::BE_SQLITE_OK != alignment.GetDgnDb().DeleteLinkTableRelationship(
            ECInstanceKey(stmtDelPtr->GetValueId<ECClassId>(0), stmtDelPtr->GetValueId<ECInstanceId>(1))))
            return DgnDbStatus::BadElement;
        }

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != alignment.GetDgnDb().InsertLinkTableRelationship(insKey,
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
        if (DbResult::BE_SQLITE_OK != alignment.GetDgnDb().DeleteLinkTableRelationship(
            ECInstanceKey(stmtDelPtr->GetValueId<ECClassId>(0), stmtDelPtr->GetValueId<ECInstanceId>(1))))
            return DgnDbStatus::BadElement;
        }
     
    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != alignment.GetDgnDb().InsertLinkTableRelationship(insKey,
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
    auto retVal = Insert(stat);
    if (retVal.IsValid())
        {
        auto horizAlignmPtr = HorizontalAlignment::Create(*this, alignmentPair.GetHorizontalCurveVector());
        horizAlignmPtr->GenerateElementGeom();
        if (horizAlignmPtr->Insert(stat).IsNull())
            return nullptr;

        if (alignmentPair.VerticalCurveVector().IsValid())
            {
            auto verticalModelPtr = VerticalAlignmentModel::Create(VerticalAlignmentModel::CreateParams(GetDgnDb(), GetElementId()));

            if (DgnDbStatus::Success != verticalModelPtr->Insert())
                return nullptr;

            auto verticalAlignmPtr = VerticalAlignment::Create(*verticalModelPtr, *alignmentPair.VerticalCurveVector());
            verticalAlignmPtr->GenerateElementGeom();
            if (verticalAlignmPtr->InsertAsMainVertical(stat).IsNull())
                return nullptr;
            }

        GenerateAprox3dGeom();
        retVal = Update();
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
                    GetElementId()));

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
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Alignment::GenerateAprox3dGeom()
    {
    auto mainPairPtr = QueryMainPair();
    if (mainPairPtr.IsNull())
        return DgnDbStatus::MissingId;

    DPoint3d origin = { 0, 0, 0 };
    auto geomBuilderPtr = GeometryBuilder::Create(*GetModel(), GetCategoryId(), origin);

    if (mainPairPtr->VerticalCurveVector().IsNull())
        {
        if (!geomBuilderPtr->Append(*mainPairPtr->HorizontalCurveVector(), GeometryBuilder::CoordSystem::World))
            return DgnDbStatus::NoGeometry;
        }
    else
        {
        bvector<DPoint3d> strokedPoints = mainPairPtr->GetStrokedAlignment();        

        if (!geomBuilderPtr->Append(*CurveVector::CreateLinear(strokedPoints), GeometryBuilder::CoordSystem::World))
            return DgnDbStatus::NoGeometry;
        }

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(*this))
        return DgnDbStatus::NoGeometry;

    return DgnDbStatus::Success;
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
        QueryClassId(alignment.GetDgnDb()), AlignmentCategory::GetHorizontal(alignment.GetDgnDb()));
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
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentCPtr HorizontalAlignment::QueryAlignment() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT SourceECInstanceId FROM " BRRA_SCHEMA(BRRA_REL_AlignmentRefersToHorizontal)
        " WHERE TargetECInstanceId = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());
    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return nullptr;

    return Alignment::Get(GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
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
* @bsimethod                                    Mindaugas.Butkus                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HorizontalAlignment::_CopyFrom(Dgn::DgnElementCR source)
    {
    T_Super::_CopyFrom(source);

    m_geometry = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignmentPtr VerticalAlignment::Create(VerticalAlignmentModelCR breakDownModel, CurveVectorR verticalGeometry)
    {
    CreateParams createParams(breakDownModel.GetDgnDb(), breakDownModel.GetModelId(), 
        QueryClassId(breakDownModel.GetDgnDb()), AlignmentCategory::GetVertical(breakDownModel.GetDgnDb()));
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignmentCPtr VerticalAlignment::InsertAsMainVertical(Dgn::DgnDbStatus* stat)
    {
    auto retValPtr = Insert(stat);
    DgnDbStatus status = Alignment::SetMainVertical(GetAlignment(), *retValPtr);
    BeAssert(DgnDbStatus::Success == status);

    return retValPtr;
    }