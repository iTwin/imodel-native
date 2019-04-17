/*--------------------------------------------------------------------------------------+
|
|  $Source: Domain/GeneratedInserts/GeneratedOmniClass33Inserts.cpp $
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

void GeneratedInserts::InsertOmniClass33Definitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr omniClassSystem = TryAndGetSystem(db, "OmniClass", "2006-03-28");
    ClassificationTableCPtr omniClassTable = TryAndGetTable(*omniClassSystem , "Table 33 - Disciplines");

    ClassificationPtr subsection0OmniClass;
    subsection0OmniClass = InsertClassification(*omniClassTable, "Planning", "33-11 00 00", "", nullptr, nullptr);
    ClassificationPtr subsection1OmniClass;
        subsection1OmniClass = InsertClassification(*omniClassTable, "Regional Planning", "33-11 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Development Planning", "33-11 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Rural Planning", "33-11 31 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Urban Planning", "33-11 41 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Transportation Planning", "33-11 44 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Environmental Planning", "33-11 51 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Facility Conservation Planning", "33-11 61 00", "", nullptr, subsection0OmniClass.get());
    ClassificationPtr subsection2OmniClass;
            subsection2OmniClass = InsertClassification(*omniClassTable, "Historic Building Conservation Planning", "33-11 61 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Ancient Monument Conservation Planning", "33-11 61 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Archaeological Area Conservation Planning", "33-11 61 41", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Planning", "33-11 99 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Master Planning", "33-11 99 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Permitting", "33-11 99 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Risk Assessment", "33-11 99 17", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Design", "33-21 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Architecture", "33-21 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Residential Architecture", "33-21 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Commercial Architecture", "33-21 11 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Institutional Architecture", "33-21 11 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Industrial Architecture", "33-21 11 27", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Drafting", "33-21 19 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Landscape Architecture", "33-21 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Interior Design", "33-21 23 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Specifying", "33-21 25 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Graphic Design", "33-21 27 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Signage Graphic Design", "33-21 27 11", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Engineering", "33-21 31 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Civil Engineering", "33-21 31 11", "", nullptr, subsection1OmniClass.get());
    ClassificationPtr subsection3OmniClass;
                subsection3OmniClass = InsertClassification(*omniClassTable, "Geotechnical Engineering", "33-21 31 11 11", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Structural Engineering", "33-21 31 14", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Foundation Engineering", "33-21 31 14 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "High Rise Engineering", "33-21 31 14 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Long-span Structure Engineering", "33-21 31 14 31", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Tensile Structure Engineering", "33-21 31 14 41", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Pneumatic Structure Engineering", "33-21 31 14 51", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Hydraulic Structure Engineering", "33-21 31 14 54", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Mechanical Engineering", "33-21 31 17", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Plumbing Engineering", "33-21 31 17 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Fire Protection Engineering", "33-21 31 17 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Heating Ventilation and Air Conditioning Engineering", "33-21 31 17 31", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Refrigeration Engineering", "33-21 31 17 33", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Energy Monitoring and Controls Engineering", "33-21 31 17 34", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Hydrological Engineering", "33-21 31 17 37", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Electrical Engineering", "33-21 31 21", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "High Voltage Electrical Engineering", "33-21 31 21 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Medium Voltage Electrical Engineering", "33-21 31 21 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Low Voltage Electrical Engineering", "33-21 31 21 31", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Communications Engineering", "33-21 31 23", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Computer Network Engineering", "33-21 31 23 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Alarm and Detection Engineering", "33-21 31 23 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Audiovisual Engineering", "33-21 31 23 21", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Process Engineering", "33-21 31 24", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Piping Engineering", "33-21 31 24 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Wind Engineering", "33-21 31 24 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Co-Generation Engineering", "33-21 31 24 31", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Nuclear Engineering", "33-21 31 24 41", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Sanitary Engineering", "33-21 31 24 51", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Military Engineering", "33-21 31 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Other Engineering", "33-21 31 99", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Acoustical/Emanations Shielding Engineering", "33-21 31 99 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Antiterrorism/Physical Security Engineering", "33-21 31 99 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Value Engineering", "33-21 31 99 17", "", nullptr, subsection2OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Other Design", "33-21 99 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Fountain Design", "33-21 99 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Finish Hardware Design", "33-21 99 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Extraterrestrial Design Specialist", "33-21 99 15", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Health Services Design Specialist", "33-21 99 22", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Hospital Design Specialist", "33-21 99 22 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Nursing Home Design Specialist", "33-21 99 22 21", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Infrastructure Design Specialist", "33-21 99 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Irrigation Design Specialist", "33-21 99 25", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Laboratory Design Specialist", "33-21 99 26", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Lighting Design Specialist", "33-21 99 28", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Marina Design Specialist", "33-21 99 29", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Model Making Specialist", "33-21 99 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Solar Design Specialist", "33-21 99 39", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Transportation Design Specialist", "33-21 99 45", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Air Transportation Design Specialist", "33-21 99 45 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Roadway Transportation Design Specialist", "33-21 99 45 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Marine Transportation Design Specialist", "33-21 99 45 31", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Vertical Transportation Design Specialist", "33-21 99 45 41", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Parking/Traffic Specialist Design Specialist", "33-21 99 45 51", "", nullptr, subsection2OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Project Management", "33-25 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Cost Estimation", "33-25 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Proposal Preparation", "33-25 14 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Client Briefing", "33-25 17 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Scheduling", "33-25 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Contract Administration", "33-25 31 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Procurement", "33-25 41 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Manufacturing", "33-25 41 11", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Building Product Manufacturing", "33-25 41 11 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Process Manufacturing", "33-25 41 11 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Construction Equipment Manufacturing", "33-25 41 11 17", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Construction Product Sales", "33-25 41 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Construction Product Marketing", "33-25 41 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Construction Product Purchasing", "33-25 41 21", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Quality Assurance", "33-25 51 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Quality Control", "33-25 54 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Surveying", "33-31 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Building Surveying", "33-31 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Site Surveying", "33-31 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "GIS (Geographical Information System) Engineering", "33-31 31 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Construction", "33-41 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Contracting", "33-41 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "General Contracting", "33-41 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Subcontracting", "33-41 11 14", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Masonry", "33-41 11 14 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Carpentry", "33-41 11 14 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Iron Working", "33-41 11 14 17", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Plumbing Subcontracting", "33-41 11 14 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Fire Protection Subcontracting", "33-41 11 14 24", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Heating Ventilation and Air Conditioning Subcontracting", "33-41 11 14 27", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Refrigeration Subcontracting", "33-41 11 14 28", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Electrical Subcontracting", "33-41 11 14 37", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Energy Monitoring and Controls Subcontracting", "33-41 11 14 51", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Hydrological Subcontracting", "33-41 11 14 54", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Painting", "33-41 11 14 61", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Tiling", "33-41 11 14 64", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Plaster Subcontracting", "33-41 11 14 66", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Gypsum Board Subcontracting", "33-41 11 14 69", "", nullptr, subsection2OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Construction Management", "33-41 14 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Facility Use Disciplines", "33-55 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Real Estate", "33-55 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Real Estate Sales", "33-55 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Property Appraising", "33-55 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Leasing", "33-55 14 17", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Owner", "33-55 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Facility Operations", "33-55 24 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Facility Space Planning", "33-55 24 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Facility Management", "33-55 24 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Facility Maintenance", "33-55 24 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Facility Services", "33-55 24 21", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Plumbing Operation and Maintenance", "33-55 24 21 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Fire Protection Operation and Maintenance", "33-55 24 21 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Heating Ventilation and Air Conditioning Operation and Maintenance", "33-55 24 21 17", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Refrigeration Operation and Maintenance", "33-55 24 21 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Energy Monitoring and Controls Operation and Maintenance", "33-55 24 21 24", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Hydrological Operation and Maintenance", "33-55 24 21 27", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Lightning Protection Operation and Maintenance", "33-55 24 21 31", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Life Safety Operation and Maintenance", "33-55 24 21 34", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Radiation Protection Operation and Maintenance", "33-55 24 21 37", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Moisture Protection Operation and Maintenance", "33-55 24 21 41", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Indoor Air Quality Evaluation", "33-55 24 21 44", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Communications Operation and Maintenance", "33-55 24 21 47", "", nullptr, subsection2OmniClass.get());
    ClassificationPtr subsection4OmniClass;
                    subsection4OmniClass = InsertClassification(*omniClassTable, "Telecommunications Operation and Maintenance", "33-55 24 21 47 11", "", nullptr, subsection3OmniClass.get());
                    subsection4OmniClass = InsertClassification(*omniClassTable, "Information Technology Operation and Maintenance", "33-55 24 21 47 14", "", nullptr, subsection3OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassTable, "Facility Shielding Operation and Maintenance", "33-55 24 21 51", "", nullptr, subsection2OmniClass.get());
                    subsection4OmniClass = InsertClassification(*omniClassTable, "Acoustic Shielding Operation and Maintenance", "33-55 24 21 51 11", "", nullptr, subsection3OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Facility Restoration", "33-55 36 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Concrete Restoration", "33-55 36 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Masonry Restoration", "33-55 36 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Parking Restoration", "33-55 36 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Historic Restoration", "33-55 36 41", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Support Disciplines", "33-81 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Legal", "33-81 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Code Specialist", "33-81 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Forensic Specialist", "33-81 11 14", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Environment", "33-81 13 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Environmental Impact", "33-81 13 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Hazardous Materials Abatement", "33-81 13 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Tree Preservation Specialist", "33-81 13 44", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Finance", "33-81 31 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Banking", "33-81 31 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Accounting", "33-81 31 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Insurance", "33-81 31 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Bonding", "33-81 31 21", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Human Resources", "33-81 34 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassTable, "Other Disciplines", "33-99 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassTable, "Science", "33-99 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Art", "33-99 13 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassTable, "Photography", "33-99 13 11", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Security", "33-99 41 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Public Relations", "33-99 45 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassTable, "Education", "33-99 61 00", "", nullptr, subsection0OmniClass.get());

    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
