/*--------------------------------------------------------------------------------------+
|
|     $Source: SegmentRange.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(SegmentRangeElementHandler)
HANDLER_DEFINE_MEMBERS(RailRangeHandler)
HANDLER_DEFINE_MEMBERS(RoadRangeHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId SegmentRangeElement::QueryAlignmentId() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement(
        "SELECT TargetECInstanceId FROM " BRRP_SCHEMA(BRRP_REL_SegmentRangeRefersToAlignment) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnElementId();

    return stmtPtr->GetValueId<DgnElementId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SegmentRangeElement::SetAlignment(SegmentRangeElementCR roadRange, AlignmentCP alignment)
    {
    if (!roadRange.GetElementId().IsValid() || (alignment && !alignment->GetElementId().IsValid()))
        return DgnDbStatus::BadArg;

    auto stmtDelPtr = roadRange.GetDgnDb().GetPreparedECSqlStatement("SELECT ECClassId, ECInstanceId FROM " BRRP_SCHEMA(BRRP_REL_SegmentRangeRefersToAlignment) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtDelPtr.IsValid());

    stmtDelPtr->BindId(1, roadRange.GetElementId());
    if (DbResult::BE_SQLITE_ROW == stmtDelPtr->Step())
        {
        if (DbResult::BE_SQLITE_OK != roadRange.GetDgnDb().DeleteNonNavigationRelationship(
            ECInstanceKey(stmtDelPtr->GetValueId<ECClassId>(0), stmtDelPtr->GetValueId<ECInstanceId>(1))))
            return DgnDbStatus::BadElement;
        }

    if (alignment)
        {
        ECInstanceKey insKey;
        if (DbResult::BE_SQLITE_OK != roadRange.GetDgnDb().InsertNonNavigationRelationship(insKey,
            *roadRange.GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_REL_SegmentRangeRefersToAlignment)->GetRelationshipClassCP(),
            ECInstanceId(roadRange.GetElementId().GetValue()), ECInstanceId(alignment->GetElementId().GetValue())))
            return DgnDbStatus::BadElement;
        }
    
    return DgnDbStatus::Success;
    }

//=======================================================================================
//! Concrete implementation of a cascade algorithm targeting SegmentRanges.
//=======================================================================================
struct SegmentRangeCascadeAlgorithm : BridgeCascadeAlgorithm
{
DEFINE_T_SUPER(BridgeCascadeAlgorithm)

protected:
    SegmentRangeCascadeAlgorithm(ILinearlyLocatedCR original, ILinearlyLocatedCR replacement, CascadeLocationChangesAction action): 
        T_Super(original, replacement, action) {}

public:
    static RefCountedPtr<SegmentRangeCascadeAlgorithm> Create(ILinearlyLocatedCR original, ILinearlyLocatedCR replacement, CascadeLocationChangesAction action)
        { return new SegmentRangeCascadeAlgorithm(original, replacement, action); }
}; // SegmentRangeCascadeAlgorithm

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SegmentRangeElement::_OnChildUpdate(DgnElementCR original, DgnElementCR replacement) const
    {
    DgnDbStatus status = T_Super::_OnChildUpdate(original, replacement);
    if (DgnDbStatus::Success != status)
        return status;

    auto originalLinearlyLocatedCP = dynamic_cast<ILinearlyLocatedCP>(&original);
    if (!originalLinearlyLocatedCP)
        return status;

    auto replacementLinearlyLocatedCP = dynamic_cast<ILinearlyLocatedCP>(&replacement);
    if (!replacementLinearlyLocatedCP || 
        CascadeLocationChangesAction::None == replacementLinearlyLocatedCP->GetCascadeLocationChangesActionFlag())
        return status;
       
    m_cascadeAlgorithmPtr = SegmentRangeCascadeAlgorithm::Create(
        *originalLinearlyLocatedCP, *replacementLinearlyLocatedCP, replacementLinearlyLocatedCP->GetCascadeLocationChangesActionFlag());
    return _PrepareCascadeChanges(*m_cascadeAlgorithmPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void SegmentRangeElement::_OnChildUpdated(DgnElementCR child) const
    {
    if (m_cascadeAlgorithmPtr.IsValid())
        _CommitCascadeChanges(*m_cascadeAlgorithmPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRangePtr RoadRange::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), RoadRailCategory::GetRoad(model.GetDgnDb()));

    return new RoadRange(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRangeCPtr RoadRange::InsertWithAlignment(AlignmentCR alignment, DgnDbStatus* status)
    {
    auto retVal = Insert(status);
    if (retVal.IsValid())
        {
        DgnDbStatus localStatus = SetAlignment(*retVal, &alignment);
        if (status)
            *status = localStatus;
        }
    
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailRangePtr RailRange::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), RoadRailCategory::GetTrack(model.GetDgnDb()));

    return new RailRange(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailRangeCPtr RailRange::InsertWithAlignment(AlignmentCR alignment, DgnDbStatus* status)
    {
    auto retVal = Insert(status);
    if (retVal.IsValid())
        *status = SetAlignment(*retVal, &alignment);
    
    return retVal;
    }