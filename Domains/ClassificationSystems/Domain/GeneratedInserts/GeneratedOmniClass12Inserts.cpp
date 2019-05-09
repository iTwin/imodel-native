/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

void GeneratedInserts::InsertOmniClass12Definitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr omniClassSystem = TryAndGetSystem(db, "OmniClass", "2006-03-28");
    ClassificationTableCPtr omniClassTable = TryAndGetTable(*omniClassSystem , "Table 12 - Construction Entities by Form");

    ClassificationPtr subsection0OmniClass;
    subsection0OmniClass = InsertClassification(*omniClassTable, "Buildings", "12-11 00 00", "", nullptr, nullptr);
    ClassificationPtr subsection1OmniClass;
        subsection1OmniClass = InsertClassification(*omniClassTable, "Low-Rise Buildings", "12-11 11 00", "", nullptr, subsection0OmniClass.get());
    ClassificationPtr subsection2OmniClass;
            subsection2OmniClass = InsertClassification(*omniClassTable, "Low-Rise Centralized Buildings", "12-11 11 14", "", nullptr, subsection1OmniClass.get());
    ClassificationPtr subsection3OmniClass;
                subsection3OmniClass = InsertClassification(*omniClassTable, "Pavilion", "12-11 11 14 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Megaron", "12-11 11 14 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Rotunda", "12-11 11 14 17", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "High Bay", "12-11 11 14 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Bungalow", "12-11 11 14 24", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Shack", "12-11 11 14 27", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Arena", "12-11 11 14 51", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Auditorium", "12-11 11 14 54", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Other Low-Rise Centralized Buildings", "12-11 11 14 99", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Low-Rise Linear Buildings", "12-11 11 17", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Arcade", "12-11 11 17 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Mall", "12-11 11 17 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Basilica", "12-11 11 17 17", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Other Low-Rise Linear Buildings", "12-11 11 17 99", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Low-Rise Attached Buildings", "12-11 11 21", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Infill Building", "12-11 11 21 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Lean-to Building", "12-11 11 21 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Building Addition", "12-11 11 21 17", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Other Low-Rise Attached Buildings", "12-11 11 21 99", "", nullptr, subsection2OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Mid-Rise Buildings", "12-11 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Mid-Rise Free-Standing Buildings", "12-11 14 11", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Centralized Mid-Rise Free-Standing Building", "12-11 14 11 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Centralized Mid-Rise Free-Standing Building with Court", "12-11 14 11 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Linear Mid-Rise Free-Standing Building", "12-11 14 11 17", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Other Mid-Rise Free-Standing Buildings", "12-11 14 11 99", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Mid-Rise Attached Buildings", "12-11 14 14", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Infill Mid-Rise Building", "12-11 14 14 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Building Wing", "12-11 14 14 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Other Mid-Rise Attached Buildings", "12-11 14 14 99", "", nullptr, subsection2OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "High-Rise Buildings", "12-11 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "High-Rise Free-Standing Buildings", "12-11 17 11", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "High-Rise Point Tower", "12-11 17 11 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "High-Rise Slab", "12-11 17 11 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Other High-Rise Free-Standing Buildings", "12-11 17 11 99", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "High-Rise Attached Buildings", "12-11 17 14", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Attached High-Rise Sliver", "12-11 17 14 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Other High-Rise Attached Buildings", "12-11 17 14 99", "", nullptr, subsection2OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Submerged Buildings", "12-11 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Deeply Submerged Building", "12-11 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Partially Submerged Building", "12-11 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Submerged Buildings", "12-11 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Mixed-Form Buildings", "12-11 24 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Buildings", "12-11 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Non-Building Structures", "12-14 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Ornamental Structures", "12-14 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Monument", "12-14 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Sculpture", "12-14 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Fountain", "12-14 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Ornamental Structures", "12-14 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Bridges", "12-14 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Trabeated Bridge", "12-14 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Arch Bridge", "12-14 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Truss Bridge", "12-14 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Cable-Stayed Bridge", "12-14 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Suspension Bridge", "12-14 14 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Bridges", "12-14 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Platforms and Piers", "12-14 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Platform", "12-14 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Pier", "12-14 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Viaduct", "12-14 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Platforms and Piers", "12-14 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Tanks", "12-14 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Silo", "12-14 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Crib", "12-14 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Liquid Storage Tank", "12-14 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Gas Storage Tank", "12-14 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Tanks", "12-14 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Towers", "12-14 24 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Mast", "12-14 24 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Derrick", "12-14 24 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Gantry", "12-14 24 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Chimney Stack", "12-14 24 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Tower", "12-14 24 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Towers", "12-14 24 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Buried and Submerged Structures", "12-14 27 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Subterranean Vertical Shaft", "12-14 27 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Horizontal Tunnel", "12-14 27 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Subterranean Duct Bank", "12-14 27 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Buried and Submerged Structures", "12-14 27 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Elevated Linear Structures", "12-14 31 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Elevated Line", "12-14 31 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Elevated Pipe Line", "12-14 31 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Elevated Linear Structures", "12-14 31 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Non-Building Structures", "12-14 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Movable Structures", "12-17 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Temporary Movable Structures", "12-17 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Temporary Platform", "12-17 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Temporary Staging ", "12-17 11 12", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Temporary Building ", "12-17 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Tent", "12-17 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Temporary Movable Structures", "12-17 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Moving Structures", "12-17 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Track-Based Moving Structure", "12-17 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Wheel-Based Moving Structure", "12-17 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Cable-Based Moving Structure", "12-17 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Moving Structures ", "12-17 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Floating Structures", "12-17 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Ships", "12-17 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Barges", "12-17 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Fixed-Location Floating Structure", "12-17 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Floating Structures", "12-17 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Orbiting Structures", "12-17 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Space Station", "12-17 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Orbiting Structures", "12-17 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Movable Structures", "12-17 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Land Forms", "12-21 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Pavements and Tracks", "12-21 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Pavement", "12-21 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Tracks", "12-21 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Pavements and Tracks", "12-21 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Retaining Forms", "12-21 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Retaining Wall", "12-21 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Embankment", "12-21 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Terrace", "12-21 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Dam", "12-21 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Retaining Forms", "12-21 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Cuttings and Excavations", "12-21 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Pit", "12-21 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Trench", "12-21 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Channel", "12-21 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Cuttings/Excavations", "12-21 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Plantings", "12-21 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Planting Bed", "12-21 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Garden", "12-21 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Planting Field", "12-21 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Orchard", "12-21 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Forest", "12-21 21 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Plantings", "12-21 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Land Forms", "12-21 99 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Water Forms", "12-24 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Centralized Water Forms", "12-24 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Pond", "12-24 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Reservoir", "12-24 11 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Lake", "12-24 11 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Sea", "12-24 11 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Ocean", "12-24 11 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Centralized Water Forms", "12-24 11 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Linear Water Forms", "12-24 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Creek", "12-24 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Aqueduct", "12-24 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Canal", "12-24 14 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Stream", "12-24 14 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "River", "12-24 14 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Linear Water Forms", "12-24 14 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Water Forms", "12-24 99 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Bay", "12-24 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Inlet", "12-24 17 14", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Construction Entity Groupings", "12-27 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Campus", "12-27 17 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Education Campus", "12-27 17 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Compound", "12-27 17 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Base", "12-27 17 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Complex", "12-27 17 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Airport", "12-27 17 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Campus", "12-27 17 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Districts", "12-27 21 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "City Block", "12-27 21 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "District", "12-27 21 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Neighborhood", "12-27 21 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Quarter", "12-27 21 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Sector", "12-27 21 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Reservation", "12-27 21 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Districts", "12-27 21 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Municipalities", "12-27 24 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Hamlet", "12-27 24 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Village", "12-27 24 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Town", "12-27 24 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Suburb", "12-27 24 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "City", "12-27 24 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Metropolitan Area", "12-27 24 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Municipalities", "12-27 24 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Regions", "12-27 27 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "County", "12-27 27 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Region", "12-27 27 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Regions", "12-27 27 99", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Territories", "12-27 31 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Territory", "12-27 31 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "State", "12-27 31 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Province", "12-27 31 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Nation", "12-27 31 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Theater", "12-27 31 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Territories", "12-27 31 99", "", nullptr, subsection1OmniClass.get());

    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
