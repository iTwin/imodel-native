/*--------------------------------------------------------------------------------------+
|
|  $Source: Domain/GeneratedInserts/GeneratedOmniClass14Inserts.cpp $
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
void ClassificationSystemsDomain::InsertOmniClass14Definitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr omniClassSystem = TryAndGetSystem(db, "OmniClass");
    OmniClassClassDefinitionPtr subsection0OmniClass;
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "14-11 00 00", "Fully Enclosed Spaces", nullptr);
    OmniClassClassDefinitionPtr subsection1OmniClass;
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-11 11 00", "Rooms", subsection0OmniClass.get());
    OmniClassClassDefinitionPtr subsection2OmniClass;
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 11 11", "Room", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 11 14", "Lobby", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 11 17", "Hall", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 11 21", "Auditorium", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 11 24", "Anteroom", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 11 27", "Office", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 11 99", "Other Rooms", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-11 14 00", "Levels", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 14 11", "Level", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 14 14", "Floor", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 14 17", "Story", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 14 21", "Basement", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 14 24", "Attic", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 14 99", "Other Levels", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-11 17 00", "Atria", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 17 11", "Gallery", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 17 14", "Mall", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 17 17", "Atrium", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 17 21", "Enclosed Court", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 17 99", "Other Atria", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-11 21 00", "Shafts", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 21 11", "Stair Enclosure", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 21 14", "Elevator Shaft", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 21 17", "Mechanical Shaft", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 21 99", "Other Shafts", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-11 24 00", "Interstitial Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 24 11", "Plenum", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 24 14", "Cavity", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 24 17", "Interstitial Floor", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 24 99", "Other Interstitial Spaces", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-11 27 00", "Compartments", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 27 11", "Chamber", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 27 14", "Compartment", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-11 27 99", "Other Compartments", subsection1OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "14-14 00 00", "Partially-Enclosed Spaces", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-14 11 00", "Recessed Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 11 11", "Alcove", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 11 14", "Niche", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 11 17", "Exedra", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 11 99", "Other Recessed Spaces", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-14 14 00", "Transition Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 14 11", "Corridor", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 14 14", "Vestibule", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 14 17", "Nave", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 14 99", "Other Transition Spaces", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-14 17 00", "Raised Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 17 11", "Mezzanine", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 17 14", "Balcony", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 17 17", "Stage", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 17 21", "Platform", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 17 99", "Other Raised Spaces", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-14 21 00", "Lowered and Sunken Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 21 11", "Pit", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 21 14", "Pool", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-14 21 99", "Other Lowered and Sunken Spaces", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-14 99 00", "Other Partially-Enclosed Spaces", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "14-17 00 00", "Non-Enclosed Spaces", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-17 11 00", "Stations", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 11 11", "Work Station", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 11 14", "Cubicle", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 11 17", "Counter ", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 11 99", "Other Stations", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-17 14 00", "Grouped Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 14 11", "Seating Group", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 14 14", "Seating Row", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 14 17", "Seating Section", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 14 21", "Aisle", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 14 99", "Other Groups", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-17 17 00", "Zones", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 17 11", "Air Distribution Zone", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 17 14", "Queuing Zone", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 17 99", "Other Zones", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-17 21 00", "Areas", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 21 11", "Rink", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 21 14", "Ring", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 21 17", "Mat", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 21 21", "Court", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-17 21 99", "Other Areas", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-17 99 00", "Other Non-Enclosed Spaces", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "14-21 00 00", "Covered Spaces", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-21 11 00", "Attached Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-21 11 11", "Porch", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-21 11 14", "Porte Cochere", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-21 11 17", "Arcade", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-21 11 21", "Cloisters", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-21 11 24", "Breezeway", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-21 11 99", "Other Attached Spaces", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-21 14 00", "Free-Standing Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-21 14 11", "Gazebo", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-21 14 14", "Trellis", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-21 14 99", "Other Free-Standing Spaces", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-21 99 00", "Other Covered Spaces", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "14-24 00 00", "Uncovered Spaces", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-24 11 00", "Routes", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 11 11", "Path", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 11 16", "Trail", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 11 14", "Trail Head", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 11 17", "Street", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 11 24", "Alley", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 11 27", "Sidewalk", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 11 31", "Walkway", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 11 34", "Boardwalk", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 11 99", "Other Routes", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-24 14 00", "Outdoor Areas", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 14 11", "Terrace", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 14 14", "Patio", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 14 17", "Courtyard", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 14 21", "Light Well", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 14 27", "Farm Field", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 14 31", "Planting Bed", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 14 34", "Yard", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 14 37", "Roof Area", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 14 99", "Other Outdoor Areas", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-24 17 00", "Intersections and Nodes", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 17 11", "Crossroad", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 17 14", "Intersection", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 17 17", "Plaza", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-24 17 99", "Other Intersections and Nodes", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-24 99 00", "Other Uncovered Spaces", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "14-27 00 00", "Combined Spaces", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-27 11 00", "Integral Combined Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-27 11 11", "Suite", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-27 11 14", "Core", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-27 11 99", "Other Integral Combined Spaces", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-27 14 00", "Grouped Combined Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-27 14 11", "Wing", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-27 14 14", "Bay", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-27 14 99", "Other Grouped Combined Spaces", subsection1OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "14-31 00 00", "Space Designations to Facilitate Design and Construction", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-31 11 00", "Modules", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-31 11 11", "Planning Module", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-31 11 14", "Structural Module", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-31 11 17", "Proportional Module ", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-31 11 99", "Other Modules", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-31 14 00", "Clear Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-31 14 11", "Operational Clear Space", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-31 14 14", "Accessible Clear Space", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-31 14 17", "Service Space", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-31 14 99", "Other Clear Spaces", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-31 17 00", "Tolerance Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-31 17 11", "Planning Tolerance Space", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-31 17 14", "Construction Tolerance Space", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-31 17 99", "Other Tolerance Spaces", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-31 99 00", "Other Space Designations to Facilitate Design and Construction", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "14-34 00 00", "Topographical Spaces", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-34 11 00", "Sloped Topographical Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 11 11", "Valley", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 11 14", "Channel", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 11 17", "Plateau", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 11 21", "Knoll", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 11 24", "Ridge", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 11 27", "Summit", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 11 31", "Berm", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 11 34", "Swale", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 11 99", "Other Sloped Topographical Spaces", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-34 14 00", "Flat Topographical Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 14 11", "Plain", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 14 14", "Meadow", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 14 17", "Basin", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 14 99", "Other Flat Topographical Spaces", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-34 17 00", "Land and Water Topographical Transitions", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 17 11", "Peninsula", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 17 14", "Point", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 17 17", "Inlet", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 17 21", "Bay", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 17 24", "Shoreline", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 17 27", "Island", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 17 31", "Lagoon", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 17 99", "Other Land and Water Topographical Transitions", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-34 21 00", "Topographical Water Spaces", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 21 11", "Ocean", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 21 14", "Pond", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 21 17", "Lake", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 21 21", "Reservoir", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 21 24", "Creek", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 21 27", "River", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-34 21 99", "Other Topographical Water Spaces", subsection1OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "14-37 00 00", "Legal and Geopolitical Space Designations", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-37 11 00", "Property Limits", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 11 11", "Parcel", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 11 14", "Site", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 11 17", "Tract", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 11 99", "Other Property Limits", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-37 14 00", "Rights of Way", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 14 11", "Right-of-way Space", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 14 14", "Easement", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 14 17", "Air Rights Space", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 14 21", "Mineral Rights Space", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 14 99", "Other Rights of Way", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-37 17 00", "Blocks", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 17 11", "City Block", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 17 14", "Cul-de-sac", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 17 17", "Planned Unit Development", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 17 99", "Other Blocks", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-37 21 00", "Sub-City Limits", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 21 11", "District", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 21 14", "Neighborhood", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 21 17", "Campus", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 21 21", "Compound", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 21 24", "Base", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 21 99", "Other Sub-City Limits", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-37 24 00", "Municipal Limits", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 24 11", "Town", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 24 14", "City", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 24 99", "Other Municipal Limits", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "14-37 27 00", "Extra-Municipal Limits", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 27 11", "County", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 27 14", "Region", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 27 17", "State", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 27 21", "Province", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 27 24", "Nation", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "14-37 27 99", "Other Extra-Municipal Limits", subsection1OmniClass.get());
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
