/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/Alignment.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPtr Alignment::Create(SpatialModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    DgnCategoryId categoryId;
    if (model.ToSpatialLocationModel() && DesignAlignments::QueryClassId(model.GetDgnDb()) == model.GetModeledElement()->GetElementClassId())
        categoryId = AlignmentCategory::GetAlignment(model.GetDgnDb());
    else
        categoryId = AlignmentCategory::GetLinear(model.GetDgnDb());

    AlignmentPtr retVal(new Alignment(*Create(model.GetDgnDb(), 
                                              SpatialLocationElement::CreateParams(
                                                  model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), categoryId))));
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
    if (Alignment::QueryClassId(element.GetDgnDb()) == element.GetElementClassId())
        return new Alignment(*dynamic_cast<SpatialLocationElementCP>(&element));

    // 2. If element is a Horizontal Alignment, return its Alignment.
    if (HorizontalAlignment::QueryClassId(element.GetDgnDb()) == element.GetElementClassId())
        {
        HorizontalAlignmentCPtr cPtr(new HorizontalAlignment(*dynamic_cast<SpatialLocationElementCP>(&element)));
        return cPtr->QueryAlignment();
        }

    // 3. If element is a Vertical Alignment, return its Alignment.
    if (VerticalAlignment::QueryClassId(element.GetDgnDb()) == element.GetElementClassId())
        {
        VerticalAlignmentCPtr cPtr(new VerticalAlignment(*dynamic_cast<GeometricElement2dCP>(&element)));
        return &cPtr->GetAlignment();
        }

    // 4. If element is 3d and it represents an Alignment, return it.
    if (element.ToGeometrySource3d())
        {
        auto stmtPtr = element.GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA("GraphicalElement3dRepresentsElement") " WHERE SourceECInstanceId=?;");
        BeAssert(stmtPtr.IsValid());

        stmtPtr->BindId(1, element.GetElementId());
        if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
            {
            auto alignmentCPtr = Alignment::Get(element.GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
            if (alignmentCPtr.IsValid())
                return alignmentCPtr;
            }
        }

    // 5. If element is a DrawingGraphic (2d) and it represents an Alignment, return it.
    if (element.ToDrawingGraphic())
        {
        auto stmtPtr = element.GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_DrawingGraphicRepresentsElement) " WHERE SourceECInstanceId=?;");
        BeAssert(stmtPtr.IsValid());

        stmtPtr->BindId(1, element.GetElementId());
        if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
            {
            auto alignmentCPtr = Alignment::Get(element.GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
            if (alignmentCPtr.IsValid())
                return alignmentCPtr;
            }
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
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentCPtr Alignment::GetHorizontal() const
    {
    return HorizontalAlignment::Get(GetDgnDb(), GetHorizontalId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignmentCPtr Alignment::GetMainVertical() const
    {
    return VerticalAlignment::Get(GetDgnDb(), GetMainVerticalId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void Alignment::_SetHorizontal(HorizontalAlignmentCR horizontal)
    {
    getP()->SetPropertyValue(BRRA_PROP_Alignment_Horizontal, horizontal.GetElementId());
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
/*DgnDbStatus Alignment::_OnDelete() const
    {
    DgnDbStatus retVal = DgnDbStatus::Success;
    auto horizontalCPtr = GetHorizontal();
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
    }*/

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
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void Alignment::SetMainVertical(VerticalAlignmentCP vertical)
    {
    if (vertical)
        getP()->SetPropertyValue(BRRA_PROP_Alignment_MainVertical, vertical->GetElementId());
    else
        {
        ECValue val;
        val.SetIsNull(true);

        getP()->SetPropertyValue(BRRA_PROP_Alignment_MainVertical, val);
        }
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
        relClassName = BIS_REL_DrawingGraphicRepresentsElement;
    else
        relClassName = "GraphicalElement3dRepresentsElement";

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != alignment.GetDgnDb().InsertLinkTableRelationship(insKey,
        *alignment.GetDgnDb().Schemas().GetClass(BIS_ECSCHEMA_NAME, relClassName)->GetRelationshipClassCP(),
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
        relClassName = BIS_REL_DrawingGraphicRepresentsElement;
    else
        relClassName = "GraphicalElement3dRepresentsElement";

    auto ecsql = Utf8PrintfString("SELECT ECInstanceId FROM %s.%s WHERE SourceECInstanceId = ? AND TargetECInstanceId = ?;", 
        BIS_ECSCHEMA_NAME, relClassName.c_str());
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
        horizAlignmPtr->m_editAlignment = this;
        if (horizAlignmPtr->Insert(stat).IsNull())
            return nullptr;

        if (alignmentPair.IsValidVertical())
            {
            auto verticalModelPtr = VerticalAlignmentModel::Create(VerticalAlignmentModel::CreateParams(GetDgnDb(), GetElementId()));

            if (DgnDbStatus::Success != verticalModelPtr->Insert())
                return nullptr;

            auto verticalAlignmPtr = VerticalAlignment::Create(*retVal, *alignmentPair.GetVerticalCurveVector());
            verticalAlignmPtr->GenerateElementGeom();
            verticalAlignmPtr->m_editAlignment = this;
            if (verticalAlignmPtr->InsertAsMainVertical(stat).IsNull())
                return nullptr;
            }

        GenerateAprox3dGeom();
        retVal = Update(stat);
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
        HorizontalAlignmentPtr horizAlignmPtr = new HorizontalAlignment(*GetHorizontal()->get()->MakeCopy<SpatialLocationElement>());
        horizAlignmPtr->SetGeometry(*alignmentPair.GetHorizontalCurveVector());

        if (DgnDbStatus::Success != horizAlignmPtr->GenerateElementGeom())
            return nullptr;

        if (horizAlignmPtr->Update(stat).IsNull())
            return nullptr;

        auto vertAlignmCPtr = GetMainVertical();

        // Updated geometry has a vertical
        if (alignmentPair.IsValidVertical())
            {
            // Main vertical exists... update it
            if (vertAlignmCPtr.IsValid())
                {
                VerticalAlignmentPtr vertAlignmPtr = new VerticalAlignment(*dynamic_cast<GeometricElement2dP>(vertAlignmCPtr->get()->CopyForEdit().get()));
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
                auto verticalModelPtr = VerticalAlignmentModel::Create(DgnModel::CreateParams(GetDgnDb(), 
                                                                                              VerticalAlignmentModel::QueryClassId(GetDgnDb()),
                    GetElementId()));

                if (DgnDbStatus::Success != verticalModelPtr->Insert())
                    return nullptr;

                auto verticalAlignmPtr = VerticalAlignment::Create(*retVal, *alignmentPair.GetVerticalCurveVector());
                if (DgnDbStatus::Success != verticalAlignmPtr->GenerateElementGeom())
                    return nullptr;

                if (verticalAlignmPtr->InsertAsMainVertical(stat).IsNull())
                    return nullptr;
                }
            }
        // Updated geometry doesn't have a vertical
        else
            {
            BeAssert(false);
            // Main vertical exists... delete it
            /*if (vertAlignmCPtr.IsValid())
                *stat = vertAlignmCPtr->Delete();*/
            }
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Alignment::GenerateAprox3dGeom(DgnSubCategoryId subCategoryId)
    {
    DPoint3d origin = { 0, 0, 0 };
    auto geomBuilderPtr = GeometryBuilder::Create(*get()->GetModel(), get()->GetCategoryId(), origin);
    if (subCategoryId.IsValid())
        geomBuilderPtr->Append(subCategoryId);

    if (!GetMainVerticalId().IsValid())
        {
        auto horizontalCPtr = GetHorizontal();
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

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(*getP()))
        return DgnDbStatus::NoGeometry;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DesignAlignments::CreateCodeBasic(SpatialModelCR model, Utf8StringCR codeVal)
    {
    return CodeSpec::CreateCode(BRRA_CODESPEC_DesignAlignments, model, codeVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DesignAlignments::QueryId(SpatialModelCR parentSpatialModel, Utf8StringCR codeVal)
    {
    ECSqlStatement stmt;
    stmt.Prepare(parentSpatialModel.GetDgnDb(),
        "SELECT ECInstanceId FROM " BRRA_SCHEMA(BRRA_CLASS_DesignAlignments) " WHERE Model.Id = ? AND CodeValue = ?;");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, parentSpatialModel.GetModelId());
    stmt.BindText(2, codeVal.c_str(), IECSqlBinder::MakeCopy::No);

    if (DbResult::BE_SQLITE_ROW == stmt.Step())
        return stmt.GetValueId<DgnElementId>(0);

    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DesignAlignmentsCPtr DesignAlignments::Insert(SpatialModelCR model, Utf8StringCR codeVal)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    if (QueryId(model, codeVal).IsValid())
        return nullptr;

    SpatialLocationElement::CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        AlignmentCategory::GetAlignment(model.GetDgnDb()));
    createParams.m_code = CreateCodeBasic(model, codeVal);

    auto newPtr = Create(model.GetDgnDb(), createParams);
    auto newCPtr = model.GetDgnDb().Elements().Insert<SpatialLocationElement>(*newPtr);
    if (newCPtr.IsNull())
        return nullptr;

    DesignAlignmentsCPtr retValCPtr = new DesignAlignments(*newCPtr);
    
    if (retValCPtr.IsNull())
        return nullptr;

    auto alignmentModelPtr = SpatialLocationModel::Create(*retValCPtr->get());
    if (DgnDbStatus::Success != alignmentModelPtr->Insert())
        return nullptr;

    auto horizontalPartitionCPtr = HorizontalAlignments::Insert(*alignmentModelPtr);
    if (horizontalPartitionCPtr.IsNull())
        return nullptr;

    auto horizontalBreakDownModelPtr = SpatialLocationModel::Create(*horizontalPartitionCPtr->get());
    horizontalBreakDownModelPtr->SetIsPlanProjection(true);
    if (DgnDbStatus::Success != horizontalBreakDownModelPtr->Insert())
        return nullptr;

    return retValCPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode HorizontalAlignments::CreateCode(SpatialModelCR spatialModel)
    {
    return CodeSpec::CreateCode(BRRA_CODESPEC_HorizontalAlignment, spatialModel, RoadRailAlignmentDomain::GetHorizontalAlignmentsCodeName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId HorizontalAlignments::QueryId(SpatialModelCR alignmentModel)
    {
    ECSqlStatement stmt;
    stmt.Prepare(alignmentModel.GetDgnDb(),
        "SELECT ECInstanceId FROM " BRRA_SCHEMA(BRRA_CLASS_HorizontalAlignments) " WHERE Model.Id = ?;");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, alignmentModel.GetModelId());

    if (DbResult::BE_SQLITE_ROW == stmt.Step())
        return stmt.GetValueId<DgnElementId>(0);

    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentsCPtr HorizontalAlignments::Insert(SpatialModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    if (QueryId(model).IsValid())
        return nullptr;

    DgnCategoryId categoryId;
    if (model.ToSpatialLocationModel() && DesignAlignments::QueryClassId(model.GetDgnDb()) == model.GetModeledElement()->GetElementClassId())
        categoryId = AlignmentCategory::GetAlignment(model.GetDgnDb());
    else
        categoryId = AlignmentCategory::GetLinear(model.GetDgnDb());

    SpatialLocationElement::CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), categoryId);
    createParams.m_code = CreateCode(model);

    auto newPtr = Create(model.GetDgnDb(), createParams);
    IBriefcaseManager::Request req;
    auto stat = newPtr->PopulateRequest(req, BeSQLite::DbOpcode::Insert);
    if (RepositoryStatus::Success == stat)
        Dgn::IBriefcaseManager::Response response = newPtr->GetDgnDb().BriefcaseManager().Acquire(req);

    auto newCPtr = model.GetDgnDb().Elements().Insert<SpatialLocationElement>(*newPtr);
    if (newCPtr.IsNull())
        return nullptr;

    return new HorizontalAlignments(*newCPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignment::HorizontalAlignment(SpatialLocationElement& element, AlignmentCR alignment, CurveVectorCR geometry):
    T_Super(element), m_alignmentId(alignment.GetElementId())
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

    auto horizontalAlignmentsCPtr = HorizontalAlignments::Query(*alignment.GetAlignmentModel());
    if (horizontalAlignmentsCPtr.IsNull())
        return nullptr;

    SpatialLocationElement::CreateParams createParams(alignment.GetDgnDb(), horizontalAlignmentsCPtr->GetSubModelId(),
        QueryClassId(alignment.GetDgnDb()), alignment.get()->GetCategoryId());
    return new HorizontalAlignment(*Create(alignment.GetDgnDb(), createParams), alignment, horizontalGeometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorCR HorizontalAlignment::GetGeometry() const
    {
    if (m_geometry.IsNull())
        {
        ECValue val;
        if (DgnDbStatus::Success != get()->GetPropertyValue(val, BRRA_PROP_HorizontalAlignment_HorizontalGeometry))
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

    getP()->SetPropertyValue(BRRA_PROP_HorizontalAlignment_HorizontalGeometry, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus HorizontalAlignment::GenerateElementGeom()
    {
    DPoint3d origin = { 0, 0, 0 };
    auto geomBuilderPtr = GeometryBuilder::Create(*get()->GetModel(), get()->GetCategoryId(), origin);
    if (!geomBuilderPtr->Append(GetGeometry(), GeometryBuilder::CoordSystem::World))
        return DgnDbStatus::NoGeometry;

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(*getP()))
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
    auto retVal = GetDgnDb().Elements().Insert<SpatialLocationElement>(*getP(), stat);
    if (retVal.IsNull())
        return nullptr;

    auto algPairPtr = AlignmentPair::Create(&GetGeometry(), nullptr);
    auto alignmentPtr = m_editAlignment.IsValid() ? m_editAlignment : Alignment::GetForEdit(GetDgnDb(), m_alignmentId);
    alignmentPtr->_SetHorizontal(*this);
    alignmentPtr->_SetLength(algPairPtr->LengthXY());
    if (m_editAlignment.IsNull()) alignmentPtr->Update();

    return new HorizontalAlignment(*retVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
HorizontalAlignmentCPtr HorizontalAlignment::Update(Dgn::DgnDbStatus* stat) 
    { 
    auto retVal = GetDgnDb().Elements().Update<SpatialLocationElement>(*getP(), stat);
    if (retVal.IsNull())
        return nullptr;

    auto algPairPtr = AlignmentPair::Create(&GetGeometry(), nullptr);
    auto alignmentPtr = m_editAlignment.IsValid() ? m_editAlignment : Alignment::GetForEdit(GetDgnDb(), QueryAlignment()->GetElementId());
    alignmentPtr->_SetLength(algPairPtr->LengthXY());
    if (m_editAlignment.IsNull()) alignmentPtr->Update();

    return new HorizontalAlignment(*retVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mindaugas.Butkus                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
/*void HorizontalAlignment::_CopyFrom(Dgn::DgnElementCR source, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(source, opts);

    m_geometry = nullptr;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignmentPtr VerticalAlignment::Create(AlignmentCR alignment, CurveVectorCR verticalGeometry)
    {
    GeometricElement2d::CreateParams createParams(alignment.GetDgnDb(), alignment.GetSubModelId(),
        QueryClassId(alignment.GetDgnDb()), AlignmentCategory::GetVertical(alignment.GetDgnDb()));
    return new VerticalAlignment(*Create(alignment.GetDgnDb(), createParams), verticalGeometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignment::VerticalAlignment(GeometricElement2d& element, CurveVectorCR geometry):
    T_Super(element)
    {
    SetGeometry(geometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr VerticalAlignment::GetGeometry() const
    {
    ECValue val;
    if (DgnDbStatus::Success != get()->GetPropertyValue(val, BRRA_PROP_VerticalAlignment_VerticalGeometry))
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

    getP()->SetPropertyValue(BRRA_PROP_VerticalAlignment_VerticalGeometry, val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus VerticalAlignment::GenerateElementGeom()
    {
    DPoint2d origin = { 0, 0 };
    auto geomBuilderPtr = GeometryBuilder::Create(*get()->GetModel(), get()->GetCategoryId(), origin);
    if (!geomBuilderPtr->Append(*GetGeometry(), GeometryBuilder::CoordSystem::World))
        return DgnDbStatus::NoGeometry;

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(*getP()))
        return DgnDbStatus::NoGeometry;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalAlignmentCPtr VerticalAlignment::InsertAsMainVertical(Dgn::DgnDbStatus* stat)
    {
    auto retValPtr = Insert(stat);
    
    if (retValPtr.IsNull())
        return nullptr;

    auto alignmentPtr = m_editAlignment.IsValid() ? m_editAlignment : Alignment::GetForEdit(GetDgnDb(), GetAlignment().GetElementId());
    alignmentPtr->SetMainVertical(this);
    if (m_editAlignment.IsNull()) alignmentPtr->Update();

    return retValPtr;
    }