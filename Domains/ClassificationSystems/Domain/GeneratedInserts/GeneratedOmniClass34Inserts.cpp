/*--------------------------------------------------------------------------------------+
|
|  $Source: Domain/GeneratedInserts/GeneratedOmniClass34Inserts.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//===========================================================================================
// WARNING: This is an automatically generated code for building classification inserts 
// WARNING: To generate, call "bb -r BuildingShared -f BuildingShared -p CodeGenerators b -c"
//===========================================================================================


#include <ClassificationSystems/ClassificationSystemsApi.h>

#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>


namespace BS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE
void ClassificationSystemsDomain::InsertOmniClass34Definitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr omniClassSystem = TryAndGetSystem(db, "OmniClass");
    ClassificationPtr subsection0OmniClass;
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Management", "34-11 00 00", "", nullptr, nullptr);
    ClassificationPtr subsection1OmniClass;
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Executive Management", "34-11 11 00", "", nullptr, subsection0OmniClass.get());
    ClassificationPtr subsection2OmniClass;
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Chief Executive", "34-11 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Vice President", "34-11 11 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Chairperson", "34-11 11 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Board Member", "34-11 11 41", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Partner", "34-11 11 51", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Middle-Management", "34-11 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Supervisor", "34-11 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Coordinator", "34-11 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Trainer", "34-11 21 31", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Planning Roles", "34-21 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Developer", "34-21 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Owner", "34-21 14 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Planner", "34-21 17 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Cost Estimator", "34-21 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Scheduler", "34-21 24 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Contract Administrator", "34-21 27 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Design Roles", "34-25 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Space Designer", "34-25 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Interior Designer", "34-25 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Lighting Designer", "34-25 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Space Planner", "34-25 11 17", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Architect", "34-25 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Engineer", "34-25 31 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Specifier", "34-25 41 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Procurement Roles", "34-31 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Manufacturer", "34-31 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Distributor", "34-31 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Product Representative", "34-31 31 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Buyer", "34-31 41 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Execution Roles", "34-35 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Surveyor", "34-35 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Contractor", "34-35 14 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Sub Contractor", "34-35 17 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Tradesperson", "34-35 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Equipment Operator", "34-35 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Laborer", "34-35 21 14", "", nullptr, subsection1OmniClass.get());
    ClassificationPtr subsection3OmniClass;
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Skilled Laborer", "34-35 21 14 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Unskilled Laborer", "34-35 21 14 14", "", nullptr, subsection2OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Inspector", "34-35 41 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Code Inspector", "34-35 41 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Safety Inspector", "34-35 41 14", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Utilization Roles", "34-41 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Facility Manager", "34-41 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Facility Maintenance", "34-41 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Facility Engineer", "34-41 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Maintenance Manager", "34-41 21 14", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Facility Services", "34-41 31 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Janitor", "34-41 31 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Window Washer", "34-41 31 14", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Support Roles", "34-55 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Administrative Support Staff", "34-55 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Administrative Assistant", "34-55 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Receptionist", "34-55 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Records Management Staff", "34-55 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Intern", "34-55 11 21", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Professional Support Staff", "34-55 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Consultant", "34-55 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Librarian", "34-55 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Draftsperson", "34-55 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Accountant", "34-55 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Lawyer", "34-55 14 24", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Support Roles", "34-55 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Groups", "34-61 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Team", "34-61 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Board", "34-61 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Committee", "34-61 31 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Task Team", "34-61 31 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Ad Hoc Committee", "34-61 31 21", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Organizations", "34-65 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Business Organizations", "34-65 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Corporation", "34-65 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Partnership", "34-65 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Sole Proprietorship", "34-65 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Joint Venture", "34-65 11 21", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Nonprofit Organizations", "34-65 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Association", "34-65 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Foundation", "34-65 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Union", "34-65 21 17", "", nullptr, subsection1OmniClass.get());
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
