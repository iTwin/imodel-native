/*--------------------------------------------------------------------------------------+
|
|  $Source: Domain/GeneratedInserts/GeneratedOmniClass14Inserts.cpp $
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

void ClassificationSystemsDomain::InsertOmniClass14Definitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr omniClassSystem = TryAndGetSystem(db, "OmniClass");
    ClassificationTableCPtr omniClassTable = TryAndGetTable(*omniClassSystem , "Table 14 - Spaces by Form");

    ClassificationPtr subsection0OmniClass;
    subsection0OmniClass = InsertClassification(*omniClassTable, "Fully Enclosed Spaces", "14-11 00 00", "", nullptr, nullptr);
    ClassificationPtr subsection1OmniClass;
        subsection1OmniClass = InsertClassification(*omniClassTable, "Rooms", "14-11 11 00", "", nullptr, subsection0OmniClass.get());
    ClassificationPtr subsection2OmniClass;
            subsection2OmniClass = InsertClassification(*omniClassTable, "Room", "14-11 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Lobby", "14-11 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Hall", "14-11 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Auditorium", "14-11 11 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Anteroom", "14-11 11 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Office", "14-11 11 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Rooms", "14-11 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Levels", "14-11 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Level", "14-11 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Floor", "14-11 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Story", "14-11 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Basement", "14-11 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Attic", "14-11 14 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Levels", "14-11 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Atria", "14-11 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Gallery", "14-11 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Mall", "14-11 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Atrium", "14-11 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Enclosed Court", "14-11 17 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Atria", "14-11 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Shafts", "14-11 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Stair Enclosure", "14-11 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Elevator Shaft", "14-11 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Mechanical Shaft", "14-11 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Shafts", "14-11 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Interstitial Spaces", "14-11 24 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Plenum", "14-11 24 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Cavity", "14-11 24 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Interstitial Floor", "14-11 24 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Interstitial Spaces", "14-11 24 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Compartments", "14-11 27 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Chamber", "14-11 27 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Compartment", "14-11 27 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Compartments", "14-11 27 99", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Partially-Enclosed Spaces", "14-14 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Recessed Spaces", "14-14 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Alcove", "14-14 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Niche", "14-14 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Exedra", "14-14 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Recessed Spaces", "14-14 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Transition Spaces", "14-14 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Corridor", "14-14 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Vestibule", "14-14 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Nave", "14-14 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Transition Spaces", "14-14 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Raised Spaces", "14-14 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Mezzanine", "14-14 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Balcony", "14-14 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Stage", "14-14 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Platform", "14-14 17 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Raised Spaces", "14-14 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Lowered and Sunken Spaces", "14-14 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Pit", "14-14 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Pool", "14-14 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Lowered and Sunken Spaces", "14-14 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Partially-Enclosed Spaces", "14-14 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Non-Enclosed Spaces", "14-17 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Stations", "14-17 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Work Station", "14-17 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Cubicle", "14-17 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Counter ", "14-17 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Stations", "14-17 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Grouped Spaces", "14-17 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Seating Group", "14-17 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Seating Row", "14-17 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Seating Section", "14-17 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Aisle", "14-17 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Groups", "14-17 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Zones", "14-17 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Air Distribution Zone", "14-17 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Queuing Zone", "14-17 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Zones", "14-17 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Areas", "14-17 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Rink", "14-17 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Ring", "14-17 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Mat", "14-17 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Court", "14-17 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Areas", "14-17 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Non-Enclosed Spaces", "14-17 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Covered Spaces", "14-21 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Attached Spaces", "14-21 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Porch", "14-21 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Porte Cochere", "14-21 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Arcade", "14-21 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Cloisters", "14-21 11 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Breezeway", "14-21 11 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Attached Spaces", "14-21 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Free-Standing Spaces", "14-21 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Gazebo", "14-21 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Trellis", "14-21 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Free-Standing Spaces", "14-21 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Covered Spaces", "14-21 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Uncovered Spaces", "14-24 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Routes", "14-24 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Path", "14-24 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Trail", "14-24 11 16", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Trail Head", "14-24 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Street", "14-24 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Alley", "14-24 11 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Sidewalk", "14-24 11 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Walkway", "14-24 11 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Boardwalk", "14-24 11 34", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Routes", "14-24 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Outdoor Areas", "14-24 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Terrace", "14-24 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Patio", "14-24 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Courtyard", "14-24 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Light Well", "14-24 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Farm Field", "14-24 14 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Planting Bed", "14-24 14 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Yard", "14-24 14 34", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Roof Area", "14-24 14 37", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Outdoor Areas", "14-24 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Intersections and Nodes", "14-24 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Crossroad", "14-24 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Intersection", "14-24 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Plaza", "14-24 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Intersections and Nodes", "14-24 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Uncovered Spaces", "14-24 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Combined Spaces", "14-27 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Integral Combined Spaces", "14-27 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Suite", "14-27 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Core", "14-27 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Integral Combined Spaces", "14-27 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Grouped Combined Spaces", "14-27 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Wing", "14-27 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Bay", "14-27 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Grouped Combined Spaces", "14-27 14 99", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Space Designations to Facilitate Design and Construction", "14-31 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Modules", "14-31 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Planning Module", "14-31 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Structural Module", "14-31 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Proportional Module ", "14-31 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Modules", "14-31 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Clear Spaces", "14-31 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Operational Clear Space", "14-31 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Accessible Clear Space", "14-31 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Service Space", "14-31 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Clear Spaces", "14-31 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Tolerance Spaces", "14-31 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Planning Tolerance Space", "14-31 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Construction Tolerance Space", "14-31 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Tolerance Spaces", "14-31 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Space Designations to Facilitate Design and Construction", "14-31 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Topographical Spaces", "14-34 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Sloped Topographical Spaces", "14-34 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Valley", "14-34 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Channel", "14-34 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Plateau", "14-34 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Knoll", "14-34 11 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Ridge", "14-34 11 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Summit", "14-34 11 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Berm", "14-34 11 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Swale", "14-34 11 34", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Sloped Topographical Spaces", "14-34 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Flat Topographical Spaces", "14-34 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Plain", "14-34 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Meadow", "14-34 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Basin", "14-34 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Flat Topographical Spaces", "14-34 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Land and Water Topographical Transitions", "14-34 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Peninsula", "14-34 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Point", "14-34 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Inlet", "14-34 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Bay", "14-34 17 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Shoreline", "14-34 17 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Island", "14-34 17 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Lagoon", "14-34 17 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Land and Water Topographical Transitions", "14-34 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Topographical Water Spaces", "14-34 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Ocean", "14-34 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Pond", "14-34 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Lake", "14-34 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Reservoir", "14-34 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Creek", "14-34 21 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "River", "14-34 21 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Topographical Water Spaces", "14-34 21 99", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Legal and Geopolitical Space Designations", "14-37 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Property Limits", "14-37 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Parcel", "14-37 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Site", "14-37 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Tract", "14-37 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Property Limits", "14-37 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Rights of Way", "14-37 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Right-of-way Space", "14-37 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Easement", "14-37 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Air Rights Space", "14-37 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Mineral Rights Space", "14-37 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Rights of Way", "14-37 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Blocks", "14-37 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "City Block", "14-37 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Cul-de-sac", "14-37 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Planned Unit Development", "14-37 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Blocks", "14-37 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Sub-City Limits", "14-37 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "District", "14-37 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Neighborhood", "14-37 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Campus", "14-37 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Compound", "14-37 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Base", "14-37 21 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Sub-City Limits", "14-37 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Municipal Limits", "14-37 24 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Town", "14-37 24 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "City", "14-37 24 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Municipal Limits", "14-37 24 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Extra-Municipal Limits", "14-37 27 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "County", "14-37 27 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Region", "14-37 27 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "State", "14-37 27 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Province", "14-37 27 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Nation", "14-37 27 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Extra-Municipal Limits", "14-37 27 99", "", nullptr, subsection1OmniClass.get());

    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
