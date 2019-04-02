/*--------------------------------------------------------------------------------------+
|
|  $Source: Domain/GeneratedInserts/GeneratedOmniClass12Inserts.cpp $
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
void ClassificationSystemsDomain::InsertOmniClass12Definitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr omniClassSystem = TryAndGetSystem(db, "OmniClass");
    ClassificationPtr subsection0OmniClass;
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Buildings", "12-11 00 00", "", nullptr, nullptr);
    ClassificationPtr subsection1OmniClass;
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Low-Rise Buildings", "12-11 11 00", "", nullptr, subsection0OmniClass.get());
    ClassificationPtr subsection2OmniClass;
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Low-Rise Centralized Buildings", "12-11 11 14", "", nullptr, subsection1OmniClass.get());
    ClassificationPtr subsection3OmniClass;
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Pavilion", "12-11 11 14 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Megaron", "12-11 11 14 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Rotunda", "12-11 11 14 17", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "High Bay", "12-11 11 14 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Bungalow", "12-11 11 14 24", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Shack", "12-11 11 14 27", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Arena", "12-11 11 14 51", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Auditorium", "12-11 11 14 54", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Other Low-Rise Centralized Buildings", "12-11 11 14 99", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Low-Rise Linear Buildings", "12-11 11 17", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Arcade", "12-11 11 17 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Mall", "12-11 11 17 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Basilica", "12-11 11 17 17", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Other Low-Rise Linear Buildings", "12-11 11 17 99", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Low-Rise Attached Buildings", "12-11 11 21", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Infill Building", "12-11 11 21 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Lean-to Building", "12-11 11 21 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Building Addition", "12-11 11 21 17", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Other Low-Rise Attached Buildings", "12-11 11 21 99", "", nullptr, subsection2OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Mid-Rise Buildings", "12-11 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Mid-Rise Free-Standing Buildings", "12-11 14 11", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Centralized Mid-Rise Free-Standing Building", "12-11 14 11 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Centralized Mid-Rise Free-Standing Building with Court", "12-11 14 11 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Linear Mid-Rise Free-Standing Building", "12-11 14 11 17", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Other Mid-Rise Free-Standing Buildings", "12-11 14 11 99", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Mid-Rise Attached Buildings", "12-11 14 14", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Infill Mid-Rise Building", "12-11 14 14 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Building Wing", "12-11 14 14 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Other Mid-Rise Attached Buildings", "12-11 14 14 99", "", nullptr, subsection2OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "High-Rise Buildings", "12-11 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "High-Rise Free-Standing Buildings", "12-11 17 11", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "High-Rise Point Tower", "12-11 17 11 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "High-Rise Slab", "12-11 17 11 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Other High-Rise Free-Standing Buildings", "12-11 17 11 99", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "High-Rise Attached Buildings", "12-11 17 14", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Attached High-Rise Sliver", "12-11 17 14 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Other High-Rise Attached Buildings", "12-11 17 14 99", "", nullptr, subsection2OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Submerged Buildings", "12-11 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Deeply Submerged Building", "12-11 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Partially Submerged Building", "12-11 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Submerged Buildings", "12-11 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Mixed-Form Buildings", "12-11 24 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Buildings", "12-11 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Non-Building Structures", "12-14 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Ornamental Structures", "12-14 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Monument", "12-14 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Sculpture", "12-14 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Fountain", "12-14 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Ornamental Structures", "12-14 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Bridges", "12-14 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Trabeated Bridge", "12-14 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Arch Bridge", "12-14 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Truss Bridge", "12-14 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Cable-Stayed Bridge", "12-14 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Suspension Bridge", "12-14 14 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Bridges", "12-14 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Platforms and Piers", "12-14 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Platform", "12-14 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Pier", "12-14 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Viaduct", "12-14 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Platforms and Piers", "12-14 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Tanks", "12-14 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Silo", "12-14 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Crib", "12-14 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Liquid Storage Tank", "12-14 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Gas Storage Tank", "12-14 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Tanks", "12-14 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Towers", "12-14 24 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Mast", "12-14 24 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Derrick", "12-14 24 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Gantry", "12-14 24 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Chimney Stack", "12-14 24 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Tower", "12-14 24 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Towers", "12-14 24 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Buried and Submerged Structures", "12-14 27 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Subterranean Vertical Shaft", "12-14 27 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Horizontal Tunnel", "12-14 27 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Subterranean Duct Bank", "12-14 27 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Buried and Submerged Structures", "12-14 27 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Elevated Linear Structures", "12-14 31 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Elevated Line", "12-14 31 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Elevated Pipe Line", "12-14 31 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Elevated Linear Structures", "12-14 31 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Non-Building Structures", "12-14 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Movable Structures", "12-17 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Temporary Movable Structures", "12-17 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Temporary Platform", "12-17 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Temporary Staging ", "12-17 11 12", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Temporary Building ", "12-17 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Tent", "12-17 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Temporary Movable Structures", "12-17 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Moving Structures", "12-17 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Track-Based Moving Structure", "12-17 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Wheel-Based Moving Structure", "12-17 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Cable-Based Moving Structure", "12-17 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Moving Structures ", "12-17 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Floating Structures", "12-17 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Ships", "12-17 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Barges", "12-17 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Fixed-Location Floating Structure", "12-17 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Floating Structures", "12-17 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Orbiting Structures", "12-17 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Space Station", "12-17 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Orbiting Structures", "12-17 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Movable Structures", "12-17 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Land Forms", "12-21 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Pavements and Tracks", "12-21 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Pavement", "12-21 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Tracks", "12-21 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Pavements and Tracks", "12-21 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Retaining Forms", "12-21 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Retaining Wall", "12-21 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Embankment", "12-21 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Terrace", "12-21 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Dam", "12-21 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Retaining Forms", "12-21 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Cuttings and Excavations", "12-21 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Pit", "12-21 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Trench", "12-21 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Channel", "12-21 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Cuttings/Excavations", "12-21 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Plantings", "12-21 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Planting Bed", "12-21 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Garden", "12-21 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Planting Field", "12-21 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Orchard", "12-21 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Forest", "12-21 21 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Plantings", "12-21 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Land Forms", "12-21 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Water Forms", "12-24 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Centralized Water Forms", "12-24 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Pond", "12-24 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Reservoir", "12-24 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Lake", "12-24 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Sea", "12-24 11 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Ocean", "12-24 11 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Centralized Water Forms", "12-24 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Linear Water Forms", "12-24 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Creek", "12-24 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Aqueduct", "12-24 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Canal", "12-24 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Stream", "12-24 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "River", "12-24 14 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Linear Water Forms", "12-24 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Water Forms", "12-24 99 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Bay", "12-24 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Inlet", "12-24 17 14", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Construction Entity Groupings", "12-27 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Campus", "12-27 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Education Campus", "12-27 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Compound", "12-27 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Base", "12-27 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Complex", "12-27 17 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Airport", "12-27 17 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Campus", "12-27 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Districts", "12-27 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "City Block", "12-27 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "District", "12-27 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Neighborhood", "12-27 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Quarter", "12-27 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Sector", "12-27 21 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Reservation", "12-27 21 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Districts", "12-27 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Municipalities", "12-27 24 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Hamlet", "12-27 24 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Village", "12-27 24 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Town", "12-27 24 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Suburb", "12-27 24 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "City", "12-27 24 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Metropolitan Area", "12-27 24 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Municipalities", "12-27 24 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Regions", "12-27 27 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "County", "12-27 27 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Region", "12-27 27 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Regions", "12-27 27 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Territories", "12-27 31 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Territory", "12-27 31 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "State", "12-27 31 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Province", "12-27 31 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Nation", "12-27 31 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Theater", "12-27 31 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Territories", "12-27 31 99", "", nullptr, subsection1OmniClass.get());
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
