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
    RegisterHandler(CrossSectionDefinitionModelHandler::GetHandler());
    RegisterHandler(CrossSectionPortionBreakDownModelHandler::GetHandler());
    /*RegisterHandler(CrossSectionElementHandler::GetHandler());
    RegisterHandler(RoadCrossSectionHandler::GetHandler());*/

    RegisterHandler(CrossSectionPortionDefinitionElementHandler::GetHandler());
    RegisterHandler(CrossSectionPortionDefinitionHandler::GetHandler());

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

    RegisterHandler(StatusAspectHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createPhysicalPartition(DgnDbR db)
    {
    DgnDbStatus status;

    auto physicalPartitionPtr = PhysicalPartition::Create(*db.Elements().GetRootSubject(), "Physical");
    if (physicalPartitionPtr->Insert(&status).IsNull())
        return status;

    auto& physicalModelHandlerR = dgn_ModelHandler::Physical::GetHandler();
    auto physicalModelPtr = physicalModelHandlerR.Create(DgnModel::CreateParams(db, db.Domains().GetClassId(physicalModelHandlerR),
        physicalPartitionPtr->GetElementId()));

    if (DgnDbStatus::Success != (status = physicalModelPtr->Insert()))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus createCrossSectionsPartition(DgnDbR db)
    {
    DgnDbStatus status;

    auto crossSectionsPartitionPtr = DefinitionPartition::Create(*db.Elements().GetRootSubject(), "CrossSections");
    if (crossSectionsPartitionPtr->Insert(&status).IsNull())
        return status;

    auto crossSectionDefModelPtr = CrossSectionDefinitionModel::Create(CrossSectionDefinitionModel::CreateParams(db, crossSectionsPartitionPtr->GetElementId()));
    if (DgnDbStatus::Success != (status = crossSectionDefModelPtr->Insert()))
        return status;

    auto travelwayDefPortionPtr = CrossSectionPortionDefinition::Create(*crossSectionDefModelPtr, "Travelway Definitions");
    if (travelwayDefPortionPtr->Insert(&status).IsNull())
        return status;

    auto travelwayDefModelPtr = TravelwayDefinitionModel::Create(TravelwayDefinitionModel::CreateParams(db, travelwayDefPortionPtr->GetElementId()));
    if (DgnDbStatus::Success != (status = travelwayDefModelPtr->Insert()))
        return status;

    auto endCondDefPortionPtr = CrossSectionPortionDefinition::Create(*crossSectionDefModelPtr, "End-Condition Definitions");
    if (endCondDefPortionPtr->Insert(&status).IsNull())
        return status;

    auto endCondDefModelPtr = EndConditionDefinitionModel::Create(EndConditionDefinitionModel::CreateParams(db, endCondDefPortionPtr->GetElementId()));
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
DgnDbStatus createRoadwayStandardsPartition(DgnDbR db)
    {
    DgnDbStatus status;

    auto roadwayStandardsPartitionPtr = DefinitionPartition::Create(*db.Elements().GetRootSubject(), "Roadway Standards");
    if (roadwayStandardsPartitionPtr->Insert(&status).IsNull())
        return status;

    auto& roadwayStandardsModelHandlerR = RoadwayStandardsModelHandler::GetHandler();
    auto roadwayStandardsModelPtr = roadwayStandardsModelHandlerR.Create(DgnModel::CreateParams(db, RoadwayStandardsModel::QueryClassId(db),
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
DgnDbStatus RoadRailPhysicalDomain::SetUpModelHierarchy(Dgn::DgnDbR db)
    {
    DgnDbStatus status;

    if (DgnDbStatus::Success != (status = createRoadwayStandardsPartition(db)))
        return status;

    if (DgnDbStatus::Success != (status = createCrossSectionsPartition(db)))
        return status;

    if (DgnDbStatus::Success != (status = createPhysicalPartition(db)))
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadRailPhysicalDomain::_OnSchemaImported(DgnDbR dgndb) const
    {
    RoadRailCategory::InsertDomainCategories(dgndb);

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
