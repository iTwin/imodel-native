/*--------------------------------------------------------------------------------------+
|
|  $Source: Domain/GeneratedInserts/GeneratedUniFormatInserts.cpp $
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
void ClassificationSystemsDomain::InsertUniFormatDefinitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr uniFormatSystem = TryAndGetSystem(db, "UniFormat");
    UniFormatClassDefinitionPtr subsection0UniFormat;
    subsection0UniFormat = InsertUniFormat(*uniFormatSystem, "0I", "INTRODUCTION", nullptr);
    UniFormatClassDefinitionPtr subsection1UniFormat;
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "10", "PROJECT DESCRIPTION", subsection0UniFormat.get());
    UniFormatClassDefinitionPtr subsection2UniFormat;
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "1010", "Project Summary", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "1020", "Project Program", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "1030", "Project Criteria", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "1040", "Existing Conditions", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "1050", "Owner\\u2019s Work", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "1090", "Funding", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "20", "OWNER DEVELOPMENT", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "2010", "Site Acquisition", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "2020", "Permits", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "2030", "Professional Services", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "2050", "Other Activities", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "2080", "Budget Project Contingencies", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "2090", "Budget Financing", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "30", "PROCUREMENT REQUIREMENTS", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "3010", "Project Delivery", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "3020", "Solicitation", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "3030", "Instructions for Procurement", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "3040", "Available Project Information", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "3050", "Procurement Forms and Supplements", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "40", "CONTRACTING REQUIREMENTS", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "4010", "Contracting Forms and Supplements", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "4020", "Project Forms", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "4030", "Conditions of the Contract", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "4040", "Revisions, Clarifications, and Modifications", subsection1UniFormat.get());
    subsection0UniFormat = InsertUniFormat(*uniFormatSystem, "A", "SUBSTRUCTURE", nullptr);
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "A10", "FOUNDATIONS", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A1010", "Standard Foundations", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A1020", "Special Foundations", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "A20", "SUBGRADE ENCLOSURES", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A2010", "Walls for Subgrade Enclosures", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "A40", "SLABS-ON-GRADE", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A4010", "Standard Slabs-on-Grade", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A4020", "Structural Slabs-on-Grade", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A4030", "Slab Trenches", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A4040", "Pits and Bases", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A4090", "Slab-On-Grade Supplementary Components", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "A60", "WATER AND GAS MITIGATION", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A6010", "Building Subdrainage", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A6020", "Off-Gassing Mitigation", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "A90", "SUBSTRUCTURE RELATED ACTIVITIES", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A9010", "Substructure Excavation", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A9020", "Construction Dewatering", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A9030", "Excavation Support", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "A9040", "Soil Treatment", subsection1UniFormat.get());
    subsection0UniFormat = InsertUniFormat(*uniFormatSystem, "B", "SHELL", nullptr);
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "B10", "SUPERSTRUCTURE", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B1010", "Floor Construction", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B1020", "Roof Construction", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B1080", "Stairs", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "B20", "EXTERIOR VERTICAL ENCLOSURES", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B2010", "Exterior Walls", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B2020", "Exterior Windows", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B2050", "Exterior Doors and Grilles", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B2070", "Exterior Louvers and Vents", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B2080", "Exterior Wall Appurtenances", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B2090", "Exterior Wall Specialties", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "B30", "EXTERIOR HORIZONTAL ENCLOSURES", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B3010", "Roofing", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B3020", "Roof Appurtenances", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B3040", "Traffic Bearing Horizontal Enclosures", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B3060", "Horizontal Openings", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "B3080", "Overhead Exterior Enclosures", subsection1UniFormat.get());
    subsection0UniFormat = InsertUniFormat(*uniFormatSystem, "C", "INTERIORS", nullptr);
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "C10", "INTERIOR CONSTRUCTION", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C1010", "Interior Partitions", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C1020", "Interior Windows", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C1030", "Interior Doors", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C1040", "Interior Grilles and Gates", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C1060", "Raised Floor Construction", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C1070", "Suspended Ceiling Construction", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C1090", "Interior Specialties", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "C20", "INTERIOR FINISHES", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C2010", "Wall Finishes", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C2020", "Interior Fabrications", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C2030", "Flooring", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C2040", "Stair Finishes", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C2050", "Ceiling Finishes", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "C2090", "Interior Finish Schedules", subsection1UniFormat.get());
    subsection0UniFormat = InsertUniFormat(*uniFormatSystem, "D", "SERVICES", nullptr);
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "D10", "CONVEYING", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D1010", "Vertical Conveying Systems", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D1030", "Horizontal Conveying", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D1050", "Material Handling", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D1080", "Operable Access Systems", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "D20", "PLUMBING", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D2010", "Domestic Water Distribution", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D2020", "Sanitary Drainage", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D2030", "Building Support Plumbing Systems", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D2050", "General Service Compressed-Air", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D2060", "Process Support Plumbing Systems", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "D30", "HEATING, VENTILATION, AND AIR CONDITIONING (HVAC)", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D3010", "Facility Fuel Systems", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D3020", "Heating Systems", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D3030", "Cooling Systems", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D3050", "Facility HVAC Distribution Systems", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D3060", "Ventilation", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D3070", "Special Purpose HVAC Systems", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "D40", "FIRE PROTECTION", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D4010", "Fire Suppression", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D4030", "Fire Protection Specialties", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "D50", "ELECTRICAL", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D5010", "Facility Power Generation", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D5020", "Electrical Service and Distribution", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D5030", "General Purpose Electrical Power", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D5040", "Lighting", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D5080", "Miscellaneous Electrical Systems", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "D60", "COMMUNICATIONS", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D6010", "Data Communications", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D6020", "Voice Communications", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D6030", "Audio-Video Communication", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D6060", "Distributed Communications and Monitoring", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D6090", "Communications Supplementary Components", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "D70", "ELECTRONIC SAFETY AND SECURITY", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D7010", "Access Control and Intrusion Detection", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D7030", "Electronic Surveillance", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D7050", "Detection and Alarm", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D7070", "Electronic Monitoring and Control", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D7090", "Electronic Safety and Security Supplementary Components", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "D80", "INTEGRATED AUTOMATION", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "D8010", "Integrated Automation Facility Controls", subsection1UniFormat.get());
    subsection0UniFormat = InsertUniFormat(*uniFormatSystem, "E", "EQUIPMENT AND FURNISHINGS", nullptr);
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "E10", "EQUIPMENT", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "E1010", "Vehicle and Pedestrian Equipment", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "E1030", "Commercial Equipment", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "E1040", "Institutional Equipment", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "E1060", "Residential Equipment", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "E1070", "Entertainment and Recreational Equipment", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "E1090", "Other Equipment", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "E20", "FURNISHINGS", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "E2010", "Fixed Furnishings", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "E2050", "Movable Furnishings", subsection1UniFormat.get());
    subsection0UniFormat = InsertUniFormat(*uniFormatSystem, "F", "SPECIAL CONSTRUCTION AND DEMOLITION", nullptr);
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "F10", "SPECIAL CONSTRUCTION", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "F1010", "Integrated Construction", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "F1020", "Special Structures", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "F1030", "Special Function Construction", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "F1050", "Special Facility Components", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "F1060", "Athletic and Recreational Special Construction", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "F1080", "Special Instrumentation", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "F20", "FACILITY REMEDIATION", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "F2010", "Hazardous Materials Remediation", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "F30", "DEMOLITION", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "F3010", "Structure Demolition", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "F3030", "Selective Demolition", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "F3050", "Structure Moving", subsection1UniFormat.get());
    subsection0UniFormat = InsertUniFormat(*uniFormatSystem, "G", "SITEWORK", nullptr);
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "G10", "SITE PREPARATION", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G1010", "Site Clearing", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G1020", "Site Elements Demolition", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G1030", "Site Element Relocations", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G1050", "Site Remediation", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G1070", "Site Earthwork", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "G20", "SITE IMPROVEMENTS", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G2010", "Roadways", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G2020", "Parking Lots", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G2030", "Pedestrian Plazas and Walkways", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G2040", "Airfields", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G2050", "Athletic, Recreational, and Playfield Areas", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G2060", "Site Development", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G2080", "Landscaping", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "G30", "LIQUID AND GAS SITE UTILITIES", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G3010", "Water Utilities", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G3020", "Sanitary Sewerage Utilities", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G3030", "Storm Drainage Utilities", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G3050", "Site Energy Distribution", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G3060", "Site Fuel Distribution", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G3090", "Liquid and Gas Site Utilities Supplementary Components", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "G40", "ELECTRICAL SITE IMPROVEMENTS", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G4010", "Site Electric Distribution Systems", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G4050", "Site Lighting", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "G50", "SITE COMMUNICATIONS", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G5010", "Site Communications Systems", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "G90", "MISCELLANEOUS SITE CONSTRUCTION", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "G9010", "Tunnels", subsection1UniFormat.get());
    subsection0UniFormat = InsertUniFormat(*uniFormatSystem, "Z", "GENERAL", nullptr);
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "Z10", "GENERAL REQUIREMENTS", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z1010", "Price and Payment Procedures", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z1020", "Administrative Requirements", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z1040", "Quality Requirements", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z1050", "Temporary Facilities and Controls", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z1060", "Product Requirements", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z1070", "Execution and Closeout Requirements", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z1090", "Life Cycle Activities", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "Z70", "TAXES, PERMITS, INSURANCE AND BONDS", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z7010", "Taxes", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z7030", "License Fees", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z7050", "Permit Costs", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z7070", "Bond Fees", subsection1UniFormat.get());
        subsection1UniFormat = InsertUniFormat(*uniFormatSystem, "Z90", "FEES AND CONTINGENCIES", subsection0UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z9010", "Overhead", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z9030", "Profit", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z9050", "Construction Contingencies", subsection1UniFormat.get());
            subsection2UniFormat = InsertUniFormat(*uniFormatSystem, "Z9090", "Financing Costs", subsection1UniFormat.get());
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
