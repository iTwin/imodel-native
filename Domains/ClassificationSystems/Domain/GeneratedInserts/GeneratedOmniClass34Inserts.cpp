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


#include "PublicApi/GeneratedInsertsApi.h"
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>


namespace BS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE

void GeneratedInserts::InsertOmniClass34Definitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr omniClassSystem = TryAndGetSystem(db, "OmniClass", "2006-03-28");
    ClassificationTableCPtr omniClassTable = TryAndGetTable(*omniClassSystem , "Table 34 - Oraganizational Roles");

    ClassificationPtr subsection0OmniClass;
    subsection0OmniClass = InsertClassification(*omniClassTable, "Management", "34-11 00 00", "", nullptr, nullptr);
    ClassificationPtr subsection1OmniClass;
        subsection1OmniClass = InsertClassification(*omniClassTable, "Executive Management", "34-11 11 00", "", nullptr, subsection0OmniClass.get());
    ClassificationPtr subsection2OmniClass;
            subsection2OmniClass = InsertClassification(*omniClassTable, "Chief Executive", "34-11 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Vice President", "34-11 11 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Chairperson", "34-11 11 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Board Member", "34-11 11 41", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Partner", "34-11 11 51", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Middle-Management", "34-11 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Supervisor", "34-11 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Coordinator", "34-11 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Trainer", "34-11 21 31", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Planning Roles", "34-21 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Developer", "34-21 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Owner", "34-21 14 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Planner", "34-21 17 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Cost Estimator", "34-21 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Scheduler", "34-21 24 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Contract Administrator", "34-21 27 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Design Roles", "34-25 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Space Designer", "34-25 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Interior Designer", "34-25 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Lighting Designer", "34-25 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Space Planner", "34-25 11 17", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Architect", "34-25 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Engineer", "34-25 31 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Specifier", "34-25 41 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Procurement Roles", "34-31 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Manufacturer", "34-31 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Distributor", "34-31 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Product Representative", "34-31 31 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Buyer", "34-31 41 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Execution Roles", "34-35 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Surveyor", "34-35 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Contractor", "34-35 14 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Sub Contractor", "34-35 17 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Tradesperson", "34-35 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Equipment Operator", "34-35 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Laborer", "34-35 21 14", "", nullptr, subsection1OmniClass.get());
    ClassificationPtr subsection3OmniClass;
                subsection3OmniClass = InsertClassification(*omniClassTable, "Skilled Laborer", "34-35 21 14 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Unskilled Laborer", "34-35 21 14 14", "", nullptr, subsection2OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Inspector", "34-35 41 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Code Inspector", "34-35 41 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Safety Inspector", "34-35 41 14", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Utilization Roles", "34-41 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Facility Manager", "34-41 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Facility Maintenance", "34-41 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Facility Engineer", "34-41 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Maintenance Manager", "34-41 21 14", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Facility Services", "34-41 31 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Janitor", "34-41 31 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Window Washer", "34-41 31 14", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Support Roles", "34-55 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Administrative Support Staff", "34-55 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Administrative Assistant", "34-55 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Receptionist", "34-55 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Records Management Staff", "34-55 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Intern", "34-55 11 21", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Professional Support Staff", "34-55 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Consultant", "34-55 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Librarian", "34-55 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Draftsperson", "34-55 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Accountant", "34-55 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Lawyer", "34-55 14 24", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Support Roles", "34-55 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Groups", "34-61 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Team", "34-61 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Board", "34-61 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Committee", "34-61 31 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Task Team", "34-61 31 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Ad Hoc Committee", "34-61 31 21", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Organizations", "34-65 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Business Organizations", "34-65 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Corporation", "34-65 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Partnership", "34-65 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Sole Proprietorship", "34-65 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Joint Venture", "34-65 11 21", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Nonprofit Organizations", "34-65 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Association", "34-65 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Foundation", "34-65 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Union", "34-65 21 17", "", nullptr, subsection1OmniClass.get());

    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
