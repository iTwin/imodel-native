/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/Alignment.h>

HANDLER_DEFINE_MEMBERS(AlignmentHandler)
HANDLER_DEFINE_MEMBERS(HorizontalAlignmentsHandler)
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

    AlignmentPtr retVal(new Alignment(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), AlignmentCategory::GetAlignment(model.GetDgnDb()))));
    retVal->SetStartValue(0.0);
    retVal->SetStartStation(0.0);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentCPtr Alignment::GetAssociated(DgnElementCR element)
    {
    // 1. If element is itself an alignment, return it.
    if (auto alignmentCP = dynamic_cast<AlignmentCP>(&element))
        return alignmentCP;

    // 2. If element is a Horizontal Alignment, return its Alignment.
    if (auto horizAlignmentCP = dynamic_cast<HorizontalAlignmentCP>(&element))
        return horizAlignmentCP->QueryAlignment();

    // 3. If element is a Vertical Alignment, return its Alignment.
    if (auto vertAlignmentCP = dynamic_cast<VerticalAlignmentCP>(&element))
        return &vertAlignmentCP->GetAlignment();

    // 4. If element is 3d and it represents an Alignment, return it.
    if (element.ToGeometrySource3d())
        {
        auto stmtPtr = element.GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BRRA_SCHEMA(BRRA_REL_GraphicalElement3dRepresentsAlignment) " WHERE SourceECInstanceId=?;");
        BeAssert(stmtPtr.IsValid());

        stmtPtr->BindId(1, element.GetElementId());
        if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
            return Alignment::Get(element.GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
        }

    // 5. If element is a DrawingGraphic (2d) and it represents an Alignment, return it.
    if (element.ToDrawingGraphic())
        {
        auto stmtPtr = element.GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BRRA_SCHEMA(BRRA_REL_DrawingGraphicRepresentsAlignment) " WHERE SourceECInstanceId=?;");
        BeAssert(stmtPtr.IsValid());

        stmtPtr->BindId(1, element.GetElementId());
        if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
            return Alignment::Get(element.GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
        }

    // 6. If element is an ILinearElementSource, return the first ILinearElement that it realizes as an Alignment.
    if (auto iLinearElementSource = dynamic_cast<ILinearElementSourceCP>(&element))
        {
        for (auto linearElementId : iLinearElementSource->QueryLinearElements())
            {
            auto alignmentPtr = Alignment::Get(element.GetDgnDb(), linearElementId);
            if (alignmentPtr.IsValid())
                return alignmentPtr;
            }
        }

    return nullptr;
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
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Alignment::AddRepresentedBy(AlignmentCR alignment, GeometrySourceCR representedBy)
    {
    if (!representedBy.ToElement()->GetElementId().IsValid() || !alignment.GetElementId().IsValid())
        return DgnDbStatus::BadElement;
    
    Utf8String relClassName;
    if (representedBy.Is2d())
        relClassName = BRRA_REL_DrawingGraphicRepresentsAlignment;
    else
        relClassName = BRRA_REL_GraphicalElement3dRepresentsAlignment;

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != alignment.GetDgnDb().InsertLinkTableRelationship(insKey,
        *alignment.GetDgnDb().Schemas().GetClass(BRRA_SCHEMA_NAME, relClassName)->GetRelationshipClassCP(),
        ECInstanceId(representedBy.ToElement()->GetElementId().GetValue()), ECInstanceId(alignment.GetElementId().GetValue())))
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Alignment::QueryIsRepresentedBy(GeometrySourceCR geometrySource) const
    {
    Utf8String relClassName;
    if (geometrySource.Is2d())
        relClassName = BRRA_REL_DrawingGraphicRepresentsAlignment;
    else
        relClassName = BRRA_REL_GraphicalElement3dRepresentsAlignment;

    auto ecsql = Utf8PrintfString("SELECT ECInstanceId FROM %s.%s WHERE SourceECInstanceId = ? AND TargetECInstanceId = ?;", 
        BRRA_SCHEMA_NAME, relClassName.c_str());
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement(ecsql.c_str());
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, geometrySource.ToElement()->GetElementId());
    stmtPtr->BindId(2, GetElementId());

    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        return true;

    return false;
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

    return AlignmentPair::Create(horizVectorPtr.get(), vertVectorPtr.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentCPtr Alignment::InsertWithMainPair(AlignmentPairCR alignmentPair, DgnDbStatus* stat)
    {
    if (nullptr == alignmentPair.GetHorizontalCurveVector())
        return nullptr;

    auto retVal = Insert(stat);
    if (retVal.IsValid())
        {
        auto horizAlignmPtr = HorizontalAlignment::Create(*this, *alignmentPair.GetHorizontalCurveVector());
        horizAlignmPtr->GenerateElementGeom();
        if (horizAlignmPtr->Insert(stat).IsNull())
            return nullptr;

        if (alignmentPair.IsValidVertical())
            {
            auto verticalModelPtr = VerticalAlignmentModel::Create(VerticalAlignmentModel::CreateParams(GetDgnDb(), GetElementId()));

            if (DgnDbStatus::Success != verticalModelPtr->Insert())
                return nullptr;

            auto verticalAlignmPtr = VerticalAlignment::Create(*verticalModelPtr, *alignmentPair.GetVerticalCurveVector());
            verticalAlignmPtr->GenerateElementGeom();
            if (verticalAlignmPtr->InsertAsMainVertical(stat).IsNull())
                return nullptr;
            }

        retVal = Update();
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentCPtr Alignment::UpdateWithMainPair(AlignmentPairCR alignmentPair, DgnDbStatus* stat)
    {
    if (!GetElementId().IsValid() || nullptr == alignmentPair.GetHorizontalCurveVector())
        return nullptr;

    auto retVal = Update(stat);
    if (retVal.IsValid())
        {
        HorizontalAlignmentPtr horizAlignmPtr = dynamic_cast<HorizontalAlignmentP>(QueryHorizontal()->CopyForEdit().get());
        horizAlignmPtr->SetGeometry(*alignmentPair.GetHorizontalCurveVector());

        if (DgnDbStatus::Success != horizAlignmPtr->GenerateElementGeom())
            return nullptr;

        if (horizAlignmPtr->Update(stat).IsNull())
            return nullptr;

        auto vertAlignmCPtr = QueryMainVertical();

        // Updated geometry has a vertical
        if (alignmentPair.IsValidVertical())
            {
            // Main vertical exists... update it
            if (vertAlignmCPtr.IsValid())
                {
                VerticalAlignmentPtr vertAlignmPtr = dynamic_cast<VerticalAlignmentP>(vertAlignmCPtr->CopyForEdit().get());
                vertAlignmPtr->SetGeometry(*alignmentPair.GetVerticalCurveVector());

                if (DgnDbStatus::Success != vertAlignmPtr->GenerateElementGeom())
                    return nullptr;

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

                auto verticalAlignmPtr = VerticalAlignment::Create(*verticalModelPtr, *alignmentPair.GetVerticalCurveVector());
                if (DgnDbStatus::Success != verticalAlignmPtr->GenerateElementGeom())
                    return nullptr;

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
    DPoint3d origin = { 0, 0, 0 };
    auto geomBuilderPtr = GeometryBuilder::Create(*GetModel(), GetCategoryId(), origin);

    if (!QueryMainVerticalId().IsValid())
        {
        auto horizontalCPtr = QueryHorizontal();
        if (horizontalCPtr.IsNull() || !geomBuilderPtr->Append(horizontalCPtr->GetGeometry(), GeometryBuilder::CoordSystem::World))
            return DgnDbStatus::NoGeometry;
        }
    else
        {
        auto mainPairPtr = QueryMainPair();
        if (mainPairPtr.IsNull())
            return DgnDbStatus::MissingId;

        bvector<DPoint3d> strokedPoints = mainPairPtr->GetStrokedAlignment();        

        if (!geomBuilderPtr->Append(*CurveVector::CreateLinear(strokedPoints), GeometryBuilder::CoordSystem::World))
            return DgnDbStatus::NoGeometry;
        }

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(*this))
        return DgnDbStatus::NoGeometry;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode HorizontalAlignments::CreateCode(SpatialLocationPartitionCR alignmentPartition, Utf8StringCR name)
    {
    return CodeSpec::CreateCode(BRRA_CODESPEC_HorizontalAlignment, alignmentPartition, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentsCPtr HorizontalAlignments::Insert(AlignmentModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    if (model.QueryHorizontalPartition().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), 
        AlignmentCategory::GetAlignment(model.GetDgnDb()));
    createParams.m_code = CreateCode(*dynamic_cast<SpatialLocationPartitionCP>(model.GetModeledElement().get()), "Horizontal Alignments");

    HorizontalAlignmentsPtr newPtr(new HorizontalAlignments(createParams));
    IBriefcaseManager::Request req;
    auto stat = newPtr->PopulateRequest(req, BeSQLite::DbOpcode::Insert);
    if (RepositoryStatus::Success == stat)
        Dgn::IBriefcaseManager::Response response = newPtr->GetDgnDb().BriefcaseManager().Acquire(req);
    return model.GetDgnDb().Elements().Insert<HorizontalAlignments>(*newPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignment::HorizontalAlignment(CreateParams const& params, AlignmentCR alignment, CurveVectorCR geometry):
    T_Super(params), m_alignmentId(alignment.GetElementId())
    {
    SetGeometry(geometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentPtr HorizontalAlignment::Create(AlignmentCR alignment, CurveVectorCR horizontalGeometry)
    {
    if (!alignment.GetElementId().IsValid())
        return nullptr;

    auto horizontalModelId = HorizontalAlignmentModel::QueryBreakDownModelId(*alignment.GetAlignmentModel());
    if (!horizontalModelId.IsValid())
        return nullptr;

    auto breakDownModelCPtr = HorizontalAlignmentModel::Get(alignment.GetDgnDb(), horizontalModelId);

    CreateParams createParams(alignment.GetDgnDb(), breakDownModelCPtr->GetModelId(),
        QueryClassId(alignment.GetDgnDb()), AlignmentCategory::GetAlignment(alignment.GetDgnDb()));
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
void HorizontalAlignment::SetGeometry(CurveVectorCR geometry)
    {
    m_geometry = nullptr;

    CurveVectorPtr cvPtr = geometry.Clone();

    ECValue val(PrimitiveType::PRIMITIVETYPE_IGeometry);
    val.SetIGeometry(*IGeometry::Create(cvPtr));

    SetPropertyValue(BRRA_PROP_HorizontalAlignment_HorizontalGeometry, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus HorizontalAlignment::GenerateElementGeom()
    {
    DPoint3d origin = { 0, 0, 0 };
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
VerticalAlignmentPtr VerticalAlignment::Create(VerticalAlignmentModelCR breakDownModel, CurveVectorCR verticalGeometry)
    {
    CreateParams createParams(breakDownModel.GetDgnDb(), breakDownModel.GetModelId(), 
        QueryClassId(breakDownModel.GetDgnDb()), AlignmentCategory::GetVertical(breakDownModel.GetAlignment()->GetDgnDb()));
    return new VerticalAlignment(createParams, verticalGeometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignment::VerticalAlignment(CreateParams const& params, CurveVectorCR geometry):
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
void VerticalAlignment::SetGeometry(CurveVectorCR geometry)
    {
    CurveVectorPtr cvPtr = geometry.Clone();

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