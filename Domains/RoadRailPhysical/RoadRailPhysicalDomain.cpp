/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadRailPhysicalDomain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

DOMAIN_DEFINE_MEMBERS(RoadRailPhysicalDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailPhysicalDomain::RoadRailPhysicalDomain() : DgnDomain(BRRP_SCHEMA_NAME, "Bentley RoadRailPhysical Domain", 1)
    {    
    RegisterHandler(RoadRailCategoryModelHandler::GetHandler());

    RegisterHandler(TypicalSectionModelHandler::GetHandler());
    RegisterHandler(TypicalSectionPortionBreakDownModelHandler::GetHandler());

    RegisterHandler(TypicalSectionPortionElementHandler::GetHandler());
    RegisterHandler(TypicalSectionPortionHandler::GetHandler());

    RegisterHandler(TravelwayDefinitionModelHandler::GetHandler());
    RegisterHandler(TravelwayDefinitionElementHandler::GetHandler());
    RegisterHandler(RoadTravelwayDefinitionHandler::GetHandler());

    RegisterHandler(EndConditionDefinitionModelHandler::GetHandler());
    RegisterHandler(EndConditionDefinitionHandler::GetHandler());

    RegisterHandler(PathwayElementHandler::GetHandler());
    RegisterHandler(TravelwaySegmentElementHandler::GetHandler());
    RegisterHandler(RegularTravelwaySegmentHandler::GetHandler());
    RegisterHandler(TravelwayTransitionHandler::GetHandler());
    RegisterHandler(TravelwayIntersectionSegmentElementHandler::GetHandler());
    
    RegisterHandler(RoadwayStandardsModelHandler::GetHandler());

    RegisterHandler(RoadClassStandardsHandler::GetHandler());
    RegisterHandler(RoadClassDefinitionTableModelHandler::GetHandler());
    RegisterHandler(RoadClassDefinitionTableHandler::GetHandler());
    RegisterHandler(RoadClassDefinitionModelHandler::GetHandler());
    RegisterHandler(RoadClassDefinitionHandler::GetHandler());
    RegisterHandler(RoadClassHandler::GetHandler());

    RegisterHandler(RoadDesignSpeedStandardsHandler::GetHandler());
    RegisterHandler(RoadDesignSpeedDefinitionTableModelHandler::GetHandler());
    RegisterHandler(RoadDesignSpeedDefinitionTableHandler::GetHandler());
    RegisterHandler(RoadDesignSpeedDefinitionModelHandler::GetHandler());
    RegisterHandler(RoadDesignSpeedDefinitionHandler::GetHandler());
    RegisterHandler(RoadDesignSpeedHandler::GetHandler());
    
    RegisterHandler(RailwayHandler::GetHandler());
    RegisterHandler(RoadwayHandler::GetHandler());    
    RegisterHandler(RoadIntersectionLegElementHandler::GetHandler());    

    RegisterHandler(LinearlyLocatedStatusHandler::GetHandler());
    RegisterHandler(StatusAspectHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createPhysicalPartition(SubjectCR subject, Utf8CP physicalPartitionName)
    {
    DgnDbStatus status;

    auto physicalPartitionPtr = PhysicalPartition::Create(subject, physicalPartitionName);
    if (physicalPartitionPtr->Insert(&status).IsNull())
        return status;

    auto& physicalModelHandlerR = dgn_ModelHandler::Physical::GetHandler();
    auto physicalModelPtr = physicalModelHandlerR.Create(DgnModel::CreateParams(subject.GetDgnDb(), subject.GetDgnDb().Domains().GetClassId(physicalModelHandlerR),
        physicalPartitionPtr->GetElementId()));

    if (DgnDbStatus::Success != (status = physicalModelPtr->Insert()))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalModelPtr RoadRailPhysicalDomain::QueryPhysicalModel(Dgn::SubjectCR parentSubject, Utf8CP modelName)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = PhysicalPartition::CreateCode(parentSubject, modelName);
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    PhysicalPartitionCPtr partition = db.Elements().Get<PhysicalPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return dynamic_cast<PhysicalModelP>(partition->GetSubModel().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createTypicalSectionsPartition(SubjectCR subject)
    {
    DgnDbStatus status;

    auto typicalSectionDefsPartitionPtr = DefinitionPartition::Create(subject, "Typical Sections");
    if (typicalSectionDefsPartitionPtr->Insert(&status).IsNull())
        return status;

    auto typicalSectionDefModelPtr = TypicalSectionModel::Create(TypicalSectionModel::CreateParams(subject.GetDgnDb(), typicalSectionDefsPartitionPtr->GetElementId()));
    if (DgnDbStatus::Success != (status = typicalSectionDefModelPtr->Insert()))
        return status;

    auto travelwayDefPortionPtr = TypicalSectionPortion::Create(*typicalSectionDefModelPtr, "Travelway Definitions");
    if (travelwayDefPortionPtr->Insert(&status).IsNull())
        return status;

    auto travelwayDefModelPtr = TravelwayDefinitionModel::Create(TravelwayDefinitionModel::CreateParams(subject.GetDgnDb(), travelwayDefPortionPtr->GetElementId()));
    if (DgnDbStatus::Success != (status = travelwayDefModelPtr->Insert()))
        return status;

    auto endCondDefPortionPtr = TypicalSectionPortion::Create(*typicalSectionDefModelPtr, "End-Condition Definitions");
    if (endCondDefPortionPtr->Insert(&status).IsNull())
        return status;

    auto endCondDefModelPtr = EndConditionDefinitionModel::Create(EndConditionDefinitionModel::CreateParams(subject.GetDgnDb(), endCondDefPortionPtr->GetElementId()));
    if (DgnDbStatus::Success != (status = endCondDefModelPtr->Insert()))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createRoadClassModelHierarchy(RoadwayStandardsModelCR roadwayStandardsModel)
    {
    DgnDbStatus status;

    auto roadClassStandardsPtr = RoadClassStandards::Create(roadwayStandardsModel);

    RoadClassDefinitionTableModelPtr roadClassDefTableModelPtr;
    if (roadClassStandardsPtr->Insert(roadClassDefTableModelPtr, &status).IsNull())
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createRoadDesignSpeedModelHierarchy(RoadwayStandardsModelCR roadwayStandardsModel)
    {
    DgnDbStatus status;

    auto designSpeedStandardsPtr = RoadDesignSpeedStandards::Create(roadwayStandardsModel);

    RoadDesignSpeedDefinitionTableModelPtr designSpeedDefTableModelPtr;
    if (designSpeedStandardsPtr->Insert(designSpeedDefTableModelPtr, &status).IsNull())
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createRoadwayStandardsPartition(SubjectCR subject)
    {
    DgnDbStatus status;

    auto roadwayStandardsPartitionPtr = DefinitionPartition::Create(subject, "Roadway Standards");
    if (roadwayStandardsPartitionPtr->Insert(&status).IsNull())
        return status;

    auto& roadwayStandardsModelHandlerR = RoadwayStandardsModelHandler::GetHandler();
    auto roadwayStandardsModelPtr = roadwayStandardsModelHandlerR.Create(DgnModel::CreateParams(subject.GetDgnDb(), RoadwayStandardsModel::QueryClassId(subject.GetDgnDb()),
        roadwayStandardsPartitionPtr->GetElementId()));

    if (DgnDbStatus::Success != (status = roadwayStandardsModelPtr->Insert()))
        return status;

    if (DgnDbStatus::Success != (status = createRoadClassModelHierarchy(
        *dynamic_cast<RoadwayStandardsModelCP>(roadwayStandardsModelPtr.get()))))
        return status;

    if (DgnDbStatus::Success != (status = createRoadDesignSpeedModelHierarchy(
        *dynamic_cast<RoadwayStandardsModelCP>(roadwayStandardsModelPtr.get()))))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RoadRailPhysicalDomain::SetUpModelHierarchy(Dgn::SubjectCR subject, Utf8CP physicalPartitionName)
    {
    DgnDbStatus status;

    if (DgnDbStatus::Success != (status = createRoadwayStandardsPartition(subject)))

        return status;

    if (DgnDbStatus::Success != (status = createTypicalSectionsPartition(subject)))
        return status;

    if (DgnDbStatus::Success != (status = createPhysicalPartition(subject, physicalPartitionName)))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailPhysicalDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    RoadRailCategoryModel::SetUp(dgndb);

    DgnDbStatus status = SetUpModelHierarchy(*dgndb.Elements().GetRootSubject(), "Roads-Rail Physical");
    if (DgnDbStatus::Success != status)
        {
        BeAssert(false);
        }

    auto codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_RoadTravelway);
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_RoadClassDefinitionTable);
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_RoadClassDefinition);
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_RoadDesignSpeedDefinitionTable);
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }

    codeSpecPtr = CodeSpec::Create(dgndb, BRRP_CODESPEC_RoadDesignSpeedDefinition);
    BeAssert(codeSpecPtr.IsValid());
    if (codeSpecPtr.IsValid())
        {
        codeSpecPtr->Insert();
        BeAssert(codeSpecPtr->GetCodeSpecId().IsValid());
        }
    }
