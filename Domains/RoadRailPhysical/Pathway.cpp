/*--------------------------------------------------------------------------------------+
|
|     $Source: Pathway.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(PathwayElementHandler)
HANDLER_DEFINE_MEMBERS(RailwayHandler)
HANDLER_DEFINE_MEMBERS(RoadwayHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PathwayElement::SetAlignment(AlignmentCP alignment)
    {
    return SetPropertyValue("MainAlignment", (alignment) ? alignment->GetElementId() : DgnElementId(), Alignment::QueryClassId(GetDgnDb()));
    }

//=======================================================================================
//! Concrete implementation of a cascade algorithm targeting SegmentRanges.
//=======================================================================================
struct SegmentRangeCascadeAlgorithm : CascadeFromToLocationChangesAlgorithm
{
DEFINE_T_SUPER(CascadeFromToLocationChangesAlgorithm)

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
DgnDbStatus PathwayElement::_OnChildUpdate(DgnElementCR original, DgnElementCR replacement) const
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
void PathwayElement::_OnChildUpdated(DgnElementCR child) const
    {
    if (m_cascadeAlgorithmPtr.IsValid())
        _CommitCascadeChanges(*m_cascadeAlgorithmPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadwayPtr Roadway::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), RoadRailCategory::GetRoad(model.GetDgnDb()));

    return new Roadway(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Roadway::ValidateTravelwaySegment(DgnElementCR child) const
    {
    if (auto travelwaySegCP = dynamic_cast<RegularTravelwaySegmentCP>(&child))
        {
        auto travelwayDefId = travelwaySegCP->GetTravelwayDefinitionId();
        if (travelwayDefId.IsValid())
            {
            if (RoadTravelwayDefinition::Get(GetDgnDb(), travelwayDefId).IsNull())
                return DgnDbStatus::InvalidParent;
            }
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Roadway::_OnChildInsert(DgnElementCR child) const
    {
    DgnDbStatus retVal = T_Super::_OnChildInsert(child);

    if (DgnDbStatus::Success == retVal)
        retVal = ValidateTravelwaySegment(child);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Roadway::_OnChildUpdate(DgnElementCR original, DgnElementCR replacement) const
    {
    DgnDbStatus retVal = T_Super::_OnChildUpdate(original, replacement);

    if (DgnDbStatus::Success == retVal)
        retVal = ValidateTravelwaySegment(replacement);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailwayPtr Railway::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), RoadRailCategory::GetTrack(model.GetDgnDb()));

    return new Railway(createParams);
    }