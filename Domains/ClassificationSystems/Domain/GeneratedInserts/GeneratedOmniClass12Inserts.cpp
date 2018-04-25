/*--------------------------------------------------------------------------------------+
|
|  $Source: Domain/GeneratedInserts/GeneratedOmniClass12Inserts.cpp $
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
void ClassificationSystemsDomain::InsertOmniClass12Definitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr omniClassSystem = TryAndGetSystem(db, "OmniClass");
    OmniClassClassDefinitionPtr subsection0OmniClass;
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "12-11 00 00", "Buildings", nullptr);
    OmniClassClassDefinitionPtr subsection1OmniClass;
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 00", "Low-Rise Buildings", subsection0OmniClass.get());
    OmniClassClassDefinitionPtr subsection2OmniClass;
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 14", "Low-Rise Centralized Buildings", subsection1OmniClass.get());
    OmniClassClassDefinitionPtr subsection3OmniClass;
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 14 11", "Pavilion", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 14 14", "Megaron", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 14 17", "Rotunda", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 14 21", "High Bay", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 14 24", "Bungalow", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 14 27", "Shack", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 14 51", "Arena", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 14 54", "Auditorium", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 14 99", "Other Low-Rise Centralized Buildings", subsection2OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 17", "Low-Rise Linear Buildings", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 17 11", "Arcade", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 17 14", "Mall", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 17 17", "Basilica", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 17 99", "Other Low-Rise Linear Buildings", subsection2OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 21", "Low-Rise Attached Buildings", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 21 11", "Infill Building", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 21 14", "Lean-to Building", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 21 17", "Building Addition", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 11 21 99", "Other Low-Rise Attached Buildings", subsection2OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-11 14 00", "Mid-Rise Buildings", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-11 14 11", "Mid-Rise Free-Standing Buildings", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 14 11 11", "Centralized Mid-Rise Free-Standing Building", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 14 11 14", "Centralized Mid-Rise Free-Standing Building with Court", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 14 11 17", "Linear Mid-Rise Free-Standing Building", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 14 11 99", "Other Mid-Rise Free-Standing Buildings", subsection2OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-11 14 14", "Mid-Rise Attached Buildings", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 14 14 11", "Infill Mid-Rise Building", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 14 14 14", "Building Wing", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 14 14 99", "Other Mid-Rise Attached Buildings", subsection2OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-11 17 00", "High-Rise Buildings", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-11 17 11", "High-Rise Free-Standing Buildings", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 17 11 11", "High-Rise Point Tower", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 17 11 14", "High-Rise Slab", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 17 11 99", "Other High-Rise Free-Standing Buildings", subsection2OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-11 17 14", "High-Rise Attached Buildings", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 17 14 11", "Attached High-Rise Sliver", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "12-11 17 14 99", "Other High-Rise Attached Buildings", subsection2OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-11 21 00", "Submerged Buildings", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-11 21 11", "Deeply Submerged Building", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-11 21 14", "Partially Submerged Building", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-11 21 99", "Other Submerged Buildings", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-11 24 00", "Mixed-Form Buildings", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-11 99 00", "Other Buildings", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "12-14 00 00", "Non-Building Structures", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-14 11 00", "Ornamental Structures", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 11 11", "Monument", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 11 14", "Sculpture", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 11 17", "Fountain", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 11 99", "Other Ornamental Structures", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-14 14 00", "Bridges", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 14 11", "Trabeated Bridge", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 14 14", "Arch Bridge", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 14 17", "Truss Bridge", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 14 21", "Cable-Stayed Bridge", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 14 24", "Suspension Bridge", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 14 99", "Other Bridges", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-14 17 00", "Platforms and Piers", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 17 11", "Platform", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 17 14", "Pier", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 17 17", "Viaduct", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 17 99", "Other Platforms and Piers", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-14 21 00", "Tanks", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 21 11", "Silo", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 21 14", "Crib", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 21 17", "Liquid Storage Tank", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 21 21", "Gas Storage Tank", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 21 99", "Other Tanks", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-14 24 00", "Towers", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 24 11", "Mast", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 24 14", "Derrick", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 24 17", "Gantry", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 24 21", "Chimney Stack", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 24 24", "Tower", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 24 99", "Other Towers", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-14 27 00", "Buried and Submerged Structures", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 27 11", "Subterranean Vertical Shaft", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 27 14", "Horizontal Tunnel", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 27 17", "Subterranean Duct Bank", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 27 99", "Other Buried and Submerged Structures", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-14 31 00", "Elevated Linear Structures", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 31 11", "Elevated Line", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 31 14", "Elevated Pipe Line", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-14 31 99", "Other Elevated Linear Structures", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-14 99 00", "Other Non-Building Structures", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "12-17 00 00", "Movable Structures", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-17 11 00", "Temporary Movable Structures", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 11 11", "Temporary Platform", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 11 12", "Temporary Staging ", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 11 14", "Temporary Building ", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 11 17", "Tent", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 11 99", "Other Temporary Movable Structures", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-17 14 00", "Moving Structures", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 14 11", "Track-Based Moving Structure", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 14 14", "Wheel-Based Moving Structure", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 14 17", "Cable-Based Moving Structure", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 14 99", "Other Moving Structures ", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-17 17 00", "Floating Structures", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 17 11", "Ships", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 17 14", "Barges", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 17 17", "Fixed-Location Floating Structure", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 17 99", "Other Floating Structures", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-17 21 00", "Orbiting Structures", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 21 11", "Space Station", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-17 21 99", "Other Orbiting Structures", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-17 99 00", "Other Movable Structures", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "12-21 00 00", "Land Forms", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-21 11 00", "Pavements and Tracks", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 11 11", "Pavement", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 11 14", "Tracks", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 11 99", "Other Pavements and Tracks", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-21 14 00", "Retaining Forms", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 14 11", "Retaining Wall", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 14 14", "Embankment", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 14 17", "Terrace", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 14 21", "Dam", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 14 99", "Other Retaining Forms", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-21 17 00", "Cuttings and Excavations", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 17 11", "Pit", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 17 14", "Trench", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 17 17", "Channel", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 17 99", "Other Cuttings/Excavations", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-21 21 00", "Plantings", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 21 11", "Planting Bed", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 21 14", "Garden", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 21 17", "Planting Field", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 21 21", "Orchard", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 21 24", "Forest", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-21 21 99", "Other Plantings", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-21 99 00", "Other Land Forms", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "12-24 00 00", "Water Forms", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-24 11 00", "Centralized Water Forms", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 11 11", "Pond", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 11 14", "Reservoir", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 11 17", "Lake", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 11 21", "Sea", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 11 24", "Ocean", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 11 99", "Other Centralized Water Forms", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-24 14 00", "Linear Water Forms", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 14 11", "Creek", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 14 14", "Aqueduct", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 14 17", "Canal", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 14 21", "Stream", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 14 24", "River", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 14 99", "Other Linear Water Forms", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-24 99 00", "Other Water Forms", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 17 11", "Bay", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-24 17 14", "Inlet", subsection1OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "12-27 00 00", "Construction Entity Groupings", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-27 17 00", "Campus", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 17 11", "Education Campus", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 17 14", "Compound", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 17 17", "Base", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 17 21", "Complex", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 17 24", "Airport", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 17 99", "Other Campus", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-27 21 00", "Districts", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 21 11", "City Block", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 21 14", "District", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 21 17", "Neighborhood", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 21 21", "Quarter", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 21 24", "Sector", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 21 27", "Reservation", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 21 99", "Other Districts", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-27 24 00", "Municipalities", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 24 11", "Hamlet", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 24 14", "Village", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 24 17", "Town", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 24 21", "Suburb", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 24 24", "City", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 24 27", "Metropolitan Area", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 24 99", "Other Municipalities", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-27 27 00", "Regions", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 27 11", "County", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 27 14", "Region", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 27 99", "Other Regions", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "12-27 31 00", "Territories", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 31 11", "Territory", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 31 14", "State", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 31 17", "Province", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 31 21", "Nation", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 31 24", "Theater", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "12-27 31 99", "Other Territories", subsection1OmniClass.get());
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
