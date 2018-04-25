/*--------------------------------------------------------------------------------------+
|
|  $Source: Domain/GeneratedInserts/GeneratedOmniClass34Inserts.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//===========================================================================================
// WARNING: This is an automatically generated code for building classification inserts 
// WARNING: To generate, call "bb -r BuildingShared -f BuildingShared -p CodeGenerators b -c"
//===========================================================================================


#include <ClassificationSystems/ClassificationSystemsApi.h>
#include <DgnClientFx/DgnClientApp.h>
#include <BuildingShared\BuildingSharedApi.h>


namespace BS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE
void ClassificationSystemsDomain::InsertOmniClass34Definitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr omniClassSystem = TryAndGetSystem(db, "OmniClass");
    OmniClassClassDefinitionPtr subsection0OmniClass;
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "34-11 00 00", "Management", nullptr);
    OmniClassClassDefinitionPtr subsection1OmniClass;
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-11 11 00", "Executive Management", subsection0OmniClass.get());
    OmniClassClassDefinitionPtr subsection2OmniClass;
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-11 11 11", "Chief Executive", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-11 11 21", "Vice President", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-11 11 31", "Chairperson", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-11 11 41", "Board Member", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-11 11 51", "Partner", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-11 21 00", "Middle-Management", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-11 21 11", "Supervisor", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-11 21 21", "Coordinator", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-11 21 31", "Trainer", subsection1OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "34-21 00 00", "Planning Roles", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-21 11 00", "Developer", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-21 14 00", "Owner", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-21 17 00", "Planner", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-21 21 00", "Cost Estimator", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-21 24 00", "Scheduler", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-21 27 00", "Contract Administrator", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "34-25 00 00", "Design Roles", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-25 11 00", "Space Designer", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-25 11 11", "Interior Designer", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-25 11 14", "Lighting Designer", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-25 11 17", "Space Planner", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-25 21 00", "Architect", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-25 31 00", "Engineer", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-25 41 00", "Specifier", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "34-31 00 00", "Procurement Roles", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-31 11 00", "Manufacturer", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-31 21 00", "Distributor", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-31 31 00", "Product Representative", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-31 41 00", "Buyer", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "34-35 00 00", "Execution Roles", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-35 11 00", "Surveyor", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-35 14 00", "Contractor", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-35 17 00", "Sub Contractor", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-35 21 00", "Tradesperson", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-35 21 11", "Equipment Operator", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-35 21 14", "Laborer", subsection1OmniClass.get());
    OmniClassClassDefinitionPtr subsection3OmniClass;
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "34-35 21 14 11", "Skilled Laborer", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "34-35 21 14 14", "Unskilled Laborer", subsection2OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-35 41 00", "Inspector", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-35 41 11", "Code Inspector", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-35 41 14", "Safety Inspector", subsection1OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "34-41 00 00", "Utilization Roles", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-41 11 00", "Facility Manager", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-41 21 00", "Facility Maintenance", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-41 21 11", "Facility Engineer", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-41 21 14", "Maintenance Manager", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-41 31 00", "Facility Services", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-41 31 11", "Janitor", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-41 31 14", "Window Washer", subsection1OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "34-55 00 00", "Support Roles", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-55 11 00", "Administrative Support Staff", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-55 11 11", "Administrative Assistant", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-55 11 14", "Receptionist", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-55 11 17", "Records Management Staff", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-55 11 21", "Intern", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-55 14 00", "Professional Support Staff", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-55 14 11", "Consultant", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-55 14 14", "Librarian", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-55 14 17", "Draftsperson", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-55 14 21", "Accountant", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-55 14 24", "Lawyer", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-55 99 00", "Other Support Roles", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "34-61 00 00", "Groups", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-61 11 00", "Team", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-61 21 00", "Board", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-61 31 00", "Committee", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-61 31 11", "Task Team", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-61 31 21", "Ad Hoc Committee", subsection1OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "34-65 00 00", "Organizations", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-65 11 00", "Business Organizations", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-65 11 11", "Corporation", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-65 11 14", "Partnership", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-65 11 17", "Sole Proprietorship", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-65 11 21", "Joint Venture", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "34-65 21 00", "Nonprofit Organizations", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-65 21 11", "Association", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-65 21 14", "Foundation", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "34-65 21 17", "Union", subsection1OmniClass.get());
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
