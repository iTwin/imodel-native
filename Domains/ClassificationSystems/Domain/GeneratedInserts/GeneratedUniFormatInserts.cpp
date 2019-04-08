/*--------------------------------------------------------------------------------------+
|
|  $Source: Domain/GeneratedInserts/GeneratedUniFormatInserts.cpp $
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
void ClassificationSystemsDomain::InsertUniFormatDefinitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr uniFormatSystem = TryAndGetSystem(db, "UniFormat");
    ClassificationPtr subsection0UniFormat;
    subsection0UniFormat = InsertClassification(*uniFormatSystem, "INTRODUCTION", "0I", "", nullptr, nullptr);
    ClassificationPtr subsection1UniFormat;
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "PROJECT DESCRIPTION", "10", "", nullptr, subsection0UniFormat.get());
    ClassificationPtr subsection2UniFormat;
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Project Summary", "1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Project Program", "1020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Project Criteria", "1030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Existing Conditions", "1040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Owner\\u2019s Work", "1050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Funding", "1090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "OWNER DEVELOPMENT", "20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Site Acquisition", "2010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Permits", "2020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Professional Services", "2030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Other Activities", "2050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Budget Project Contingencies", "2080", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Budget Financing", "2090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "PROCUREMENT REQUIREMENTS", "30", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Project Delivery", "3010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Solicitation", "3020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Instructions for Procurement", "3030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Available Project Information", "3040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Procurement Forms and Supplements", "3050", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "CONTRACTING REQUIREMENTS", "40", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Contracting Forms and Supplements", "4010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Project Forms", "4020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Conditions of the Contract", "4030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Revisions, Clarifications, and Modifications", "4040", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatSystem, "SUBSTRUCTURE", "A", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "FOUNDATIONS", "A10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Standard Foundations", "A1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Special Foundations", "A1020", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "SUBGRADE ENCLOSURES", "A20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Walls for Subgrade Enclosures", "A2010", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "SLABS-ON-GRADE", "A40", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Standard Slabs-on-Grade", "A4010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Structural Slabs-on-Grade", "A4020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Slab Trenches", "A4030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Pits and Bases", "A4040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Slab-On-Grade Supplementary Components", "A4090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "WATER AND GAS MITIGATION", "A60", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Building Subdrainage", "A6010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Off-Gassing Mitigation", "A6020", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "SUBSTRUCTURE RELATED ACTIVITIES", "A90", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Substructure Excavation", "A9010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Construction Dewatering", "A9020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Excavation Support", "A9030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Soil Treatment", "A9040", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatSystem, "SHELL", "B", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "SUPERSTRUCTURE", "B10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Floor Construction", "B1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Roof Construction", "B1020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Stairs", "B1080", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "EXTERIOR VERTICAL ENCLOSURES", "B20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Exterior Walls", "B2010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Exterior Windows", "B2020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Exterior Doors and Grilles", "B2050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Exterior Louvers and Vents", "B2070", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Exterior Wall Appurtenances", "B2080", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Exterior Wall Specialties", "B2090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "EXTERIOR HORIZONTAL ENCLOSURES", "B30", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Roofing", "B3010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Roof Appurtenances", "B3020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Traffic Bearing Horizontal Enclosures", "B3040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Horizontal Openings", "B3060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Overhead Exterior Enclosures", "B3080", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatSystem, "INTERIORS", "C", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "INTERIOR CONSTRUCTION", "C10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Interior Partitions", "C1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Interior Windows", "C1020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Interior Doors", "C1030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Interior Grilles and Gates", "C1040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Raised Floor Construction", "C1060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Suspended Ceiling Construction", "C1070", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Interior Specialties", "C1090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "INTERIOR FINISHES", "C20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Wall Finishes", "C2010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Interior Fabrications", "C2020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Flooring", "C2030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Stair Finishes", "C2040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Ceiling Finishes", "C2050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Interior Finish Schedules", "C2090", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatSystem, "SERVICES", "D", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "CONVEYING", "D10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Vertical Conveying Systems", "D1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Horizontal Conveying", "D1030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Material Handling", "D1050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Operable Access Systems", "D1080", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "PLUMBING", "D20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Domestic Water Distribution", "D2010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Sanitary Drainage", "D2020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Building Support Plumbing Systems", "D2030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "General Service Compressed-Air", "D2050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Process Support Plumbing Systems", "D2060", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "HEATING, VENTILATION, AND AIR CONDITIONING (HVAC)", "D30", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Facility Fuel Systems", "D3010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Heating Systems", "D3020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Cooling Systems", "D3030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Facility HVAC Distribution Systems", "D3050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Ventilation", "D3060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Special Purpose HVAC Systems", "D3070", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "FIRE PROTECTION", "D40", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Fire Suppression", "D4010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Fire Protection Specialties", "D4030", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "ELECTRICAL", "D50", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Facility Power Generation", "D5010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Electrical Service and Distribution", "D5020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "General Purpose Electrical Power", "D5030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Lighting", "D5040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Miscellaneous Electrical Systems", "D5080", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "COMMUNICATIONS", "D60", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Data Communications", "D6010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Voice Communications", "D6020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Audio-Video Communication", "D6030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Distributed Communications and Monitoring", "D6060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Communications Supplementary Components", "D6090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "ELECTRONIC SAFETY AND SECURITY", "D70", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Access Control and Intrusion Detection", "D7010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Electronic Surveillance", "D7030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Detection and Alarm", "D7050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Electronic Monitoring and Control", "D7070", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Electronic Safety and Security Supplementary Components", "D7090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "INTEGRATED AUTOMATION", "D80", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Integrated Automation Facility Controls", "D8010", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatSystem, "EQUIPMENT AND FURNISHINGS", "E", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "EQUIPMENT", "E10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Vehicle and Pedestrian Equipment", "E1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Commercial Equipment", "E1030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Institutional Equipment", "E1040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Residential Equipment", "E1060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Entertainment and Recreational Equipment", "E1070", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Other Equipment", "E1090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "FURNISHINGS", "E20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Fixed Furnishings", "E2010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Movable Furnishings", "E2050", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatSystem, "SPECIAL CONSTRUCTION AND DEMOLITION", "F", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "SPECIAL CONSTRUCTION", "F10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Integrated Construction", "F1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Special Structures", "F1020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Special Function Construction", "F1030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Special Facility Components", "F1050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Athletic and Recreational Special Construction", "F1060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Special Instrumentation", "F1080", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "FACILITY REMEDIATION", "F20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Hazardous Materials Remediation", "F2010", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "DEMOLITION", "F30", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Structure Demolition", "F3010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Selective Demolition", "F3030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Structure Moving", "F3050", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatSystem, "SITEWORK", "G", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "SITE PREPARATION", "G10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Site Clearing", "G1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Site Elements Demolition", "G1020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Site Element Relocations", "G1030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Site Remediation", "G1050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Site Earthwork", "G1070", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "SITE IMPROVEMENTS", "G20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Roadways", "G2010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Parking Lots", "G2020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Pedestrian Plazas and Walkways", "G2030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Airfields", "G2040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Athletic, Recreational, and Playfield Areas", "G2050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Site Development", "G2060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Landscaping", "G2080", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "LIQUID AND GAS SITE UTILITIES", "G30", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Water Utilities", "G3010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Sanitary Sewerage Utilities", "G3020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Storm Drainage Utilities", "G3030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Site Energy Distribution", "G3050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Site Fuel Distribution", "G3060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Liquid and Gas Site Utilities Supplementary Components", "G3090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "ELECTRICAL SITE IMPROVEMENTS", "G40", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Site Electric Distribution Systems", "G4010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Site Lighting", "G4050", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "SITE COMMUNICATIONS", "G50", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Site Communications Systems", "G5010", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "MISCELLANEOUS SITE CONSTRUCTION", "G90", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Tunnels", "G9010", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatSystem, "GENERAL", "Z", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "GENERAL REQUIREMENTS", "Z10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Price and Payment Procedures", "Z1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Administrative Requirements", "Z1020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Quality Requirements", "Z1040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Temporary Facilities and Controls", "Z1050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Product Requirements", "Z1060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Execution and Closeout Requirements", "Z1070", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Life Cycle Activities", "Z1090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "TAXES, PERMITS, INSURANCE AND BONDS", "Z70", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Taxes", "Z7010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "License Fees", "Z7030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Permit Costs", "Z7050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Bond Fees", "Z7070", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatSystem, "FEES AND CONTINGENCIES", "Z90", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Overhead", "Z9010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Profit", "Z9030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Construction Contingencies", "Z9050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatSystem, "Financing Costs", "Z9090", "", nullptr, subsection1UniFormat.get());
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
