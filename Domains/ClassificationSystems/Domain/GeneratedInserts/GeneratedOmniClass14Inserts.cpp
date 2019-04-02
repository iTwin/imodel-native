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
    ClassificationPtr subsection0OmniClass;
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Fully Enclosed Spaces", "14-11 00 00", "", nullptr, nullptr);
    ClassificationPtr subsection1OmniClass;
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Rooms", "14-11 11 00", "", nullptr, subsection0OmniClass.get());
    ClassificationPtr subsection2OmniClass;
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Room", "14-11 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Lobby", "14-11 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Hall", "14-11 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Auditorium", "14-11 11 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Anteroom", "14-11 11 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Office", "14-11 11 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Rooms", "14-11 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Levels", "14-11 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Level", "14-11 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Floor", "14-11 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Story", "14-11 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Basement", "14-11 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Attic", "14-11 14 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Levels", "14-11 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Atria", "14-11 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Gallery", "14-11 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Mall", "14-11 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Atrium", "14-11 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Enclosed Court", "14-11 17 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Atria", "14-11 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Shafts", "14-11 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Stair Enclosure", "14-11 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Elevator Shaft", "14-11 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Mechanical Shaft", "14-11 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Shafts", "14-11 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Interstitial Spaces", "14-11 24 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Plenum", "14-11 24 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Cavity", "14-11 24 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Interstitial Floor", "14-11 24 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Interstitial Spaces", "14-11 24 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Compartments", "14-11 27 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Chamber", "14-11 27 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Compartment", "14-11 27 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Compartments", "14-11 27 99", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Partially-Enclosed Spaces", "14-14 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Recessed Spaces", "14-14 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Alcove", "14-14 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Niche", "14-14 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Exedra", "14-14 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Recessed Spaces", "14-14 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Transition Spaces", "14-14 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Corridor", "14-14 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Vestibule", "14-14 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Nave", "14-14 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Transition Spaces", "14-14 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Raised Spaces", "14-14 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Mezzanine", "14-14 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Balcony", "14-14 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Stage", "14-14 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Platform", "14-14 17 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Raised Spaces", "14-14 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Lowered and Sunken Spaces", "14-14 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Pit", "14-14 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Pool", "14-14 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Lowered and Sunken Spaces", "14-14 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Partially-Enclosed Spaces", "14-14 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Non-Enclosed Spaces", "14-17 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Stations", "14-17 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Work Station", "14-17 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Cubicle", "14-17 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Counter ", "14-17 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Stations", "14-17 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Grouped Spaces", "14-17 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Seating Group", "14-17 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Seating Row", "14-17 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Seating Section", "14-17 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Aisle", "14-17 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Groups", "14-17 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Zones", "14-17 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Air Distribution Zone", "14-17 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Queuing Zone", "14-17 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Zones", "14-17 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Areas", "14-17 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Rink", "14-17 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Ring", "14-17 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Mat", "14-17 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Court", "14-17 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Areas", "14-17 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Non-Enclosed Spaces", "14-17 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Covered Spaces", "14-21 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Attached Spaces", "14-21 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Porch", "14-21 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Porte Cochere", "14-21 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Arcade", "14-21 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Cloisters", "14-21 11 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Breezeway", "14-21 11 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Attached Spaces", "14-21 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Free-Standing Spaces", "14-21 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Gazebo", "14-21 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Trellis", "14-21 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Free-Standing Spaces", "14-21 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Covered Spaces", "14-21 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Uncovered Spaces", "14-24 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Routes", "14-24 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Path", "14-24 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Trail", "14-24 11 16", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Trail Head", "14-24 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Street", "14-24 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Alley", "14-24 11 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Sidewalk", "14-24 11 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Walkway", "14-24 11 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Boardwalk", "14-24 11 34", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Routes", "14-24 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Outdoor Areas", "14-24 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Terrace", "14-24 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Patio", "14-24 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Courtyard", "14-24 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Light Well", "14-24 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Farm Field", "14-24 14 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Planting Bed", "14-24 14 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Yard", "14-24 14 34", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Roof Area", "14-24 14 37", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Outdoor Areas", "14-24 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Intersections and Nodes", "14-24 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Crossroad", "14-24 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Intersection", "14-24 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Plaza", "14-24 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Intersections and Nodes", "14-24 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Uncovered Spaces", "14-24 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Combined Spaces", "14-27 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Integral Combined Spaces", "14-27 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Suite", "14-27 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Core", "14-27 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Integral Combined Spaces", "14-27 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Grouped Combined Spaces", "14-27 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Wing", "14-27 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Bay", "14-27 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Grouped Combined Spaces", "14-27 14 99", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Space Designations to Facilitate Design and Construction", "14-31 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Modules", "14-31 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Planning Module", "14-31 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Structural Module", "14-31 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Proportional Module ", "14-31 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Modules", "14-31 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Clear Spaces", "14-31 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Operational Clear Space", "14-31 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Accessible Clear Space", "14-31 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Service Space", "14-31 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Clear Spaces", "14-31 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Tolerance Spaces", "14-31 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Planning Tolerance Space", "14-31 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Construction Tolerance Space", "14-31 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Tolerance Spaces", "14-31 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Space Designations to Facilitate Design and Construction", "14-31 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Topographical Spaces", "14-34 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Sloped Topographical Spaces", "14-34 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Valley", "14-34 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Channel", "14-34 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Plateau", "14-34 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Knoll", "14-34 11 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Ridge", "14-34 11 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Summit", "14-34 11 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Berm", "14-34 11 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Swale", "14-34 11 34", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Sloped Topographical Spaces", "14-34 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Flat Topographical Spaces", "14-34 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Plain", "14-34 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Meadow", "14-34 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Basin", "14-34 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Flat Topographical Spaces", "14-34 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Land and Water Topographical Transitions", "14-34 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Peninsula", "14-34 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Point", "14-34 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Inlet", "14-34 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Bay", "14-34 17 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Shoreline", "14-34 17 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Island", "14-34 17 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Lagoon", "14-34 17 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Land and Water Topographical Transitions", "14-34 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Topographical Water Spaces", "14-34 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Ocean", "14-34 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Pond", "14-34 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Lake", "14-34 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Reservoir", "14-34 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Creek", "14-34 21 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "River", "14-34 21 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Topographical Water Spaces", "14-34 21 99", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Legal and Geopolitical Space Designations", "14-37 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Property Limits", "14-37 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Parcel", "14-37 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Site", "14-37 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Tract", "14-37 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Property Limits", "14-37 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Rights of Way", "14-37 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Right-of-way Space", "14-37 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Easement", "14-37 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Air Rights Space", "14-37 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Mineral Rights Space", "14-37 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Rights of Way", "14-37 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Blocks", "14-37 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "City Block", "14-37 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Cul-de-sac", "14-37 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Planned Unit Development", "14-37 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Blocks", "14-37 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Sub-City Limits", "14-37 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "District", "14-37 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Neighborhood", "14-37 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Campus", "14-37 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Compound", "14-37 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Base", "14-37 21 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Sub-City Limits", "14-37 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Municipal Limits", "14-37 24 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Town", "14-37 24 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "City", "14-37 24 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Municipal Limits", "14-37 24 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Extra-Municipal Limits", "14-37 27 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "County", "14-37 27 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Region", "14-37 27 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "State", "14-37 27 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Province", "14-37 27 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Nation", "14-37 27 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Extra-Municipal Limits", "14-37 27 99", "", nullptr, subsection1OmniClass.get());
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
