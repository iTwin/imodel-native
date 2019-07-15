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

void GeneratedInserts::InsertUniFormatDefinitions(Dgn::DgnDbR db, Dgn::DgnModelCR model) const
    {
    ClassificationSystemCPtr uniFormatSystem = TryAndGetSystem(db, model, "UniFormat", "2010");
    ClassificationTableCPtr uniFormatTable = TryAndGetTable(*uniFormatSystem , "2010 Edition UniFormat - Levels One through Three");

    ClassificationPtr subsection0UniFormat;
    subsection0UniFormat = InsertClassification(*uniFormatTable, "INTRODUCTION", "0I", "", nullptr, nullptr);
    ClassificationPtr subsection1UniFormat;
        subsection1UniFormat = InsertClassification(*uniFormatTable, "PROJECT DESCRIPTION", "10", "", nullptr, subsection0UniFormat.get());
    ClassificationPtr subsection2UniFormat;
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Project Summary", "1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Project Program", "1020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Project Criteria", "1030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Existing Conditions", "1040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Owner\\u2019s Work", "1050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Funding", "1090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "OWNER DEVELOPMENT", "20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Site Acquisition", "2010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Permits", "2020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Professional Services", "2030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Other Activities", "2050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Budget Project Contingencies", "2080", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Budget Financing", "2090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "PROCUREMENT REQUIREMENTS", "30", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Project Delivery", "3010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Solicitation", "3020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Instructions for Procurement", "3030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Available Project Information", "3040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Procurement Forms and Supplements", "3050", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "CONTRACTING REQUIREMENTS", "40", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Contracting Forms and Supplements", "4010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Project Forms", "4020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Conditions of the Contract", "4030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Revisions, Clarifications, and Modifications", "4040", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatTable, "SUBSTRUCTURE", "A", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatTable, "FOUNDATIONS", "A10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Standard Foundations", "A1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Special Foundations", "A1020", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "SUBGRADE ENCLOSURES", "A20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Walls for Subgrade Enclosures", "A2010", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "SLABS-ON-GRADE", "A40", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Standard Slabs-on-Grade", "A4010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Structural Slabs-on-Grade", "A4020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Slab Trenches", "A4030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Pits and Bases", "A4040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Slab-On-Grade Supplementary Components", "A4090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "WATER AND GAS MITIGATION", "A60", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Building Subdrainage", "A6010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Off-Gassing Mitigation", "A6020", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "SUBSTRUCTURE RELATED ACTIVITIES", "A90", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Substructure Excavation", "A9010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Construction Dewatering", "A9020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Excavation Support", "A9030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Soil Treatment", "A9040", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatTable, "SHELL", "B", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatTable, "SUPERSTRUCTURE", "B10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Floor Construction", "B1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Roof Construction", "B1020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Stairs", "B1080", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "EXTERIOR VERTICAL ENCLOSURES", "B20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Exterior Walls", "B2010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Exterior Windows", "B2020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Exterior Doors and Grilles", "B2050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Exterior Louvers and Vents", "B2070", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Exterior Wall Appurtenances", "B2080", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Exterior Wall Specialties", "B2090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "EXTERIOR HORIZONTAL ENCLOSURES", "B30", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Roofing", "B3010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Roof Appurtenances", "B3020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Traffic Bearing Horizontal Enclosures", "B3040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Horizontal Openings", "B3060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Overhead Exterior Enclosures", "B3080", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatTable, "INTERIORS", "C", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatTable, "INTERIOR CONSTRUCTION", "C10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Interior Partitions", "C1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Interior Windows", "C1020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Interior Doors", "C1030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Interior Grilles and Gates", "C1040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Raised Floor Construction", "C1060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Suspended Ceiling Construction", "C1070", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Interior Specialties", "C1090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "INTERIOR FINISHES", "C20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Wall Finishes", "C2010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Interior Fabrications", "C2020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Flooring", "C2030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Stair Finishes", "C2040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Ceiling Finishes", "C2050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Interior Finish Schedules", "C2090", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatTable, "SERVICES", "D", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatTable, "CONVEYING", "D10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Vertical Conveying Systems", "D1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Horizontal Conveying", "D1030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Material Handling", "D1050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Operable Access Systems", "D1080", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "PLUMBING", "D20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Domestic Water Distribution", "D2010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Sanitary Drainage", "D2020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Building Support Plumbing Systems", "D2030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "General Service Compressed-Air", "D2050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Process Support Plumbing Systems", "D2060", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "HEATING, VENTILATION, AND AIR CONDITIONING (HVAC)", "D30", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Facility Fuel Systems", "D3010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Heating Systems", "D3020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Cooling Systems", "D3030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Facility HVAC Distribution Systems", "D3050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Ventilation", "D3060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Special Purpose HVAC Systems", "D3070", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "FIRE PROTECTION", "D40", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Fire Suppression", "D4010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Fire Protection Specialties", "D4030", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "ELECTRICAL", "D50", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Facility Power Generation", "D5010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Electrical Service and Distribution", "D5020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "General Purpose Electrical Power", "D5030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Lighting", "D5040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Miscellaneous Electrical Systems", "D5080", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "COMMUNICATIONS", "D60", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Data Communications", "D6010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Voice Communications", "D6020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Audio-Video Communication", "D6030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Distributed Communications and Monitoring", "D6060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Communications Supplementary Components", "D6090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "ELECTRONIC SAFETY AND SECURITY", "D70", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Access Control and Intrusion Detection", "D7010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Electronic Surveillance", "D7030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Detection and Alarm", "D7050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Electronic Monitoring and Control", "D7070", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Electronic Safety and Security Supplementary Components", "D7090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "INTEGRATED AUTOMATION", "D80", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Integrated Automation Facility Controls", "D8010", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatTable, "EQUIPMENT AND FURNISHINGS", "E", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatTable, "EQUIPMENT", "E10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Vehicle and Pedestrian Equipment", "E1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Commercial Equipment", "E1030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Institutional Equipment", "E1040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Residential Equipment", "E1060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Entertainment and Recreational Equipment", "E1070", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Other Equipment", "E1090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "FURNISHINGS", "E20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Fixed Furnishings", "E2010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Movable Furnishings", "E2050", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatTable, "SPECIAL CONSTRUCTION AND DEMOLITION", "F", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatTable, "SPECIAL CONSTRUCTION", "F10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Integrated Construction", "F1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Special Structures", "F1020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Special Function Construction", "F1030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Special Facility Components", "F1050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Athletic and Recreational Special Construction", "F1060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Special Instrumentation", "F1080", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "FACILITY REMEDIATION", "F20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Hazardous Materials Remediation", "F2010", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "DEMOLITION", "F30", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Structure Demolition", "F3010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Selective Demolition", "F3030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Structure Moving", "F3050", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatTable, "SITEWORK", "G", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatTable, "SITE PREPARATION", "G10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Site Clearing", "G1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Site Elements Demolition", "G1020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Site Element Relocations", "G1030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Site Remediation", "G1050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Site Earthwork", "G1070", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "SITE IMPROVEMENTS", "G20", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Roadways", "G2010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Parking Lots", "G2020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Pedestrian Plazas and Walkways", "G2030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Airfields", "G2040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Athletic, Recreational, and Playfield Areas", "G2050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Site Development", "G2060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Landscaping", "G2080", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "LIQUID AND GAS SITE UTILITIES", "G30", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Water Utilities", "G3010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Sanitary Sewerage Utilities", "G3020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Storm Drainage Utilities", "G3030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Site Energy Distribution", "G3050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Site Fuel Distribution", "G3060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Liquid and Gas Site Utilities Supplementary Components", "G3090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "ELECTRICAL SITE IMPROVEMENTS", "G40", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Site Electric Distribution Systems", "G4010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Site Lighting", "G4050", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "SITE COMMUNICATIONS", "G50", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Site Communications Systems", "G5010", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "MISCELLANEOUS SITE CONSTRUCTION", "G90", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Tunnels", "G9010", "", nullptr, subsection1UniFormat.get());
    subsection0UniFormat = InsertClassification(*uniFormatTable, "GENERAL", "Z", "", nullptr, nullptr);
        subsection1UniFormat = InsertClassification(*uniFormatTable, "GENERAL REQUIREMENTS", "Z10", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Price and Payment Procedures", "Z1010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Administrative Requirements", "Z1020", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Quality Requirements", "Z1040", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Temporary Facilities and Controls", "Z1050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Product Requirements", "Z1060", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Execution and Closeout Requirements", "Z1070", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Life Cycle Activities", "Z1090", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "TAXES, PERMITS, INSURANCE AND BONDS", "Z70", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Taxes", "Z7010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "License Fees", "Z7030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Permit Costs", "Z7050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Bond Fees", "Z7070", "", nullptr, subsection1UniFormat.get());
        subsection1UniFormat = InsertClassification(*uniFormatTable, "FEES AND CONTINGENCIES", "Z90", "", nullptr, subsection0UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Overhead", "Z9010", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Profit", "Z9030", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Construction Contingencies", "Z9050", "", nullptr, subsection1UniFormat.get());
            subsection2UniFormat = InsertClassification(*uniFormatTable, "Financing Costs", "Z9090", "", nullptr, subsection1UniFormat.get());

    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
