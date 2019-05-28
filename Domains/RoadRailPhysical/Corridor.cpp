/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadRailPhysicalDomain.h>
#include <RoadRailPhysical/Corridor.h>

HANDLER_DEFINE_MEMBERS(CorridorHandler)
HANDLER_DEFINE_MEMBERS(CorridorPortionElementHandler)
HANDLER_DEFINE_MEMBERS(CorridorSegmentHandler)
HANDLER_DEFINE_MEMBERS(PathwayDesignCriteriaHandler)
HANDLER_DEFINE_MEMBERS(PathwayElementHandler)
HANDLER_DEFINE_MEMBERS(RailwayHandler)
HANDLER_DEFINE_MEMBERS(RoadwayHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ILinearlyDesignedElement::SetDesignAlignment(AlignmentCP alignment)
    {
    auto& dgnElement = *const_cast<DgnElementP>(&_ILinearlyDesignedElementToDgnElement());
    if (alignment)
        return dgnElement.SetPropertyValue(BRRP_PROP_ILinearlyDesignedElement_DesignAlignment, alignment->GetElementId(), 
            dgnElement.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_ILinearlyDesignedElementAlongAlignment));
    else
        return dgnElement.SetPropertyValue(BRRP_PROP_ILinearlyDesignedElement_DesignAlignment, DgnElementId(), DgnClassId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
/*DgnElementIdSet IMainLinearElementSource::QueryLinearElementSourceIds(ILinearElementCR mainLinearElement, DgnClassId filterBaseClassId)
    {
    auto& elmCR = mainLinearElement.ToElement();

    Utf8String ecSql = "SELECT iMainLE.SourceECInstanceId FROM "
        BRRP_SCHEMA(BRRP_REL_ILinearElementSourceRefersToMainLinearElement) " iMainLE";

    if (filterBaseClassId.IsValid())
        {
        ecSql.append(", meta.ClassHasAllBaseClasses WHERE ");
        ecSql.append("iMainLE.TargetECInstanceId = ? AND ");
        ecSql.append("meta.ClassHasAllBaseClasses.SourceECInstanceId = iMainLE.SourceECClassId AND ");
        ecSql.append("meta.ClassHasAllBaseClasses.TargetECInstanceId = ?;");
        }
    else
        ecSql.append(" WHERE iMainLE.TargetECInstanceId = ?;");

    ECSqlStatement stmt;
    stmt.Prepare(elmCR.GetDgnDb(), ecSql.c_str());
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, elmCR.GetElementId());

    if (filterBaseClassId.IsValid())
        stmt.BindId(2, filterBaseClassId);

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.insert(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId Corridor::QueryCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_Corridor);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Corridor::CreateCode(RoadRailNetworkCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_Corridor, scope, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorCPtr Corridor::QueryByCode(RoadRailNetworkCR scope, Utf8StringCR code)
    {
    auto id = scope.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(scope, code));
    if (!id.IsValid())
        return nullptr;

    return Corridor::Get(scope.GetDgnDb(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Corridor::AddRepresentedBy(CorridorCR corridor, GeometrySourceCR representedBy)
    {
    if (!representedBy.ToElement()->GetElementId().IsValid() || !corridor.GetElementId().IsValid())
        return DgnDbStatus::BadElement;

    Utf8String relClassName;
    if (representedBy.Is2d())
        relClassName = BIS_REL_DrawingGraphicRepresentsElement;
    else
        relClassName = "GraphicalElement3dRepresentsElement";

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != corridor.GetDgnDb().InsertLinkTableRelationship(insKey,
        *corridor.GetDgnDb().Schemas().GetClass(BIS_ECSCHEMA_NAME, relClassName)->GetRelationshipClassCP(),
        ECInstanceId(representedBy.ToElement()->GetElementId().GetValue()), ECInstanceId(corridor.GetElementId().GetValue())))
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Corridor::QueryIsRepresentedBy(GeometrySourceCR geometrySource) const
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
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorPtr Corridor::Create(RoadRailNetworkCR network)
    {
    if (!network.GetElementId().IsValid() || !network.GetSubModelId().IsValid())
        return nullptr;

    CreateParams createParams(network.GetDgnDb(), network.GetSubModelId(), QueryClassId(network.GetDgnDb()),
        RoadRailCategory::GetCorridor(network.GetDgnDb()));

    return new Corridor(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorCPtr Corridor::Insert(Dgn::DgnDbStatus* status)
    {
    auto retValCPtr = GetDgnDb().Elements().Insert<Corridor>(*this, status);
    if (retValCPtr.IsNull())
        return nullptr;

    auto corridorModelPtr = PhysicalModel::Create(*retValCPtr);
    if (corridorModelPtr.IsValid())
        {
        if (DgnDbStatus::Success != corridorModelPtr->Insert())
            return nullptr;
        }

    return retValCPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId CorridorSegment::QueryId(CorridorCR corridor, Utf8StringCR codeVal)
    {
    auto stmtPtr = corridor.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT ECInstanceId FROM " BRRP_SCHEMA(BRRP_CLASS_CorridorSegment) " WHERE Model.Id = ? AND CodeValue = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, corridor.GetSubModelId());
    stmtPtr->BindText(2, codeVal.c_str(), IECSqlBinder::MakeCopy::No);

    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        return stmtPtr->GetValueId<DgnElementId>(0);

    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DgnElementId> CorridorSegment::QueryOrderedPathwayIds() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM "
        BRRP_SCHEMA(BRRP_CLASS_PathwayElement) " WHERE Model.Id = ? ORDER BY `Order`;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetSubModelId());

    bvector<DgnElementId> retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        retVal.push_back(stmtPtr->GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CorridorSegment::CreateCode(CorridorCR corridor, Utf8StringCR codeVal)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_CorridorSegment, corridor, codeVal);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorSegmentCPtr CorridorSegment::Insert(CorridorCR corridor, Utf8StringCR codeVal)
    {
    if (!corridor.GetElementId().IsValid())
        return nullptr;

    CreateParams createParams(corridor.GetDgnDb(), corridor.GetSubModelId(), QueryClassId(corridor.GetDgnDb()),
        RoadRailCategory::GetCorridor(corridor.GetDgnDb()));
    createParams.m_code = CreateCode(corridor, codeVal);

    CorridorSegmentPtr newPtr(new CorridorSegment(createParams));
    auto rangeCPtr = corridor.GetDgnDb().Elements().Insert<CorridorSegment>(*newPtr);
    if (rangeCPtr.IsNull())
        return nullptr;

    auto corridorSegmentModelPtr = PhysicalModel::Create(*rangeCPtr);
    if (corridorSegmentModelPtr.IsValid())
        {
        if (DgnDbStatus::Success != corridorSegmentModelPtr->Insert())
            return nullptr;
        }

    auto horizontalPartitionCPtr = HorizontalAlignments::Insert(*corridorSegmentModelPtr);
    if (horizontalPartitionCPtr.IsNull())
        return nullptr;

    auto horizontalBreakDownModelPtr = SpatialLocationModel::Create(*horizontalPartitionCPtr);
    if (DgnDbStatus::Success != horizontalBreakDownModelPtr->Insert())
        return nullptr;

    return rangeCPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId PathwayElement::QueryCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_Pathway);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode PathwayElement::CreateCode(PhysicalModelCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_Pathway, scope, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PathwayElementCPtr PathwayElement::QueryByCode(PhysicalModelCR model, Utf8StringCR code)
    {
    auto id = model.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(model, code));
    if (!id.IsValid())
        return nullptr;

    return PathwayElement::Get(model.GetDgnDb(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Roadway::Roadway(CreateParams const& params, PathwayElement::Order const& order) :
    T_Super(params)
    {
    SetOrder(order);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadwayPtr Roadway::Create(CorridorSegmentCR corridorSegment, PathwayElement::Order const& order)
    {
    if (!corridorSegment.GetElementId().IsValid() || !corridorSegment.GetSubModelId().IsValid())
        return nullptr;

    CreateParams createParams(corridorSegment.GetDgnDb(), corridorSegment.GetSubModelId(), QueryClassId(corridorSegment.GetDgnDb()),
        RoadRailCategory::GetRoadway(corridorSegment.GetDgnDb()));

    return new Roadway(createParams, order);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Railway::Railway(CreateParams const& params, PathwayElement::Order const& order) :
    T_Super(params)
    {
    SetOrder(order);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailwayPtr Railway::Create(CorridorSegmentCR corridorSegment, PathwayElement::Order const& order)
    {
    if (!corridorSegment.GetElementId().IsValid() || !corridorSegment.GetSubModelId().IsValid())
        return nullptr;

    CreateParams createParams(corridorSegment.GetDgnDb(), corridorSegment.GetSubModelId(), QueryClassId(corridorSegment.GetDgnDb()),
        RoadRailCategory::GetRailway(corridorSegment.GetDgnDb()));

    return new Railway(createParams, order);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode PathwayDesignCriteria::CreateCode(PathwayElementCR pathway)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_Pathway, pathway, RoadRailPhysicalDomain::GetPathwayDesignCriteriaCodeName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId PathwayDesignCriteria::QueryId(PathwayElementCR pathway)
    {
    auto stmtPtr = pathway.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT ECInstanceId FROM " BRRP_SCHEMA(BRRP_CLASS_PathwayDesignCriteria) " WHERE Parent.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, pathway.GetElementId());

    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        return stmtPtr->GetValueId<DgnElementId>(0);

    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Dgn::DgnElementId> PathwayDesignCriteria::QueryOrderedDesignSpeedIds() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT s.ECInstanceId FROM " 
        BRRP_SCHEMA(BRRP_CLASS_DesignSpeed) " s, " BLR_SCHEMA(BLR_CLASS_LinearlyReferencedFromToLocation) " f "
        "WHERE f.Element.Id = s.ECInstanceId AND s.Model.Id = ? ORDER BY f.FromPosition.DistanceAlongFromStart;");
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, GetSubModelId());

    bvector<Dgn::DgnElementId> retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.push_back(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PathwayDesignCriteriaCPtr PathwayDesignCriteria::Insert(PathwayElementCR pathway)
    {
    if (!pathway.GetElementId().IsValid())
        return nullptr;

    CreateParams createParams(pathway.GetDgnDb(), pathway.GetModelId(), QueryClassId(pathway.GetDgnDb()),
        RoadRailCategory::GetCorridor(pathway.GetDgnDb()));
    createParams.m_code = CreateCode(pathway);
    createParams.m_parentId = pathway.GetElementId();
    createParams.m_parentRelClassId = pathway.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayOwnsDesignCriteria);

    PathwayDesignCriteriaPtr newPtr(new PathwayDesignCriteria(createParams));
    auto designCriteriaCPtr = pathway.GetDgnDb().Elements().Insert<PathwayDesignCriteria>(*newPtr);
    if (designCriteriaCPtr.IsNull())
        return nullptr;

    auto designCriteriaModelPtr = SpatialLocationModel::Create(*designCriteriaCPtr);
    if (designCriteriaModelPtr.IsValid())
        {
        if (DgnDbStatus::Success != designCriteriaModelPtr->Insert())
            return nullptr;
        }

    return designCriteriaCPtr;
    }