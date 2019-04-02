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


#include <ClassificationSystems/ClassificationSystemsApi.h>

#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>


namespace BS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

BEGIN_CLASSIFICATIONSYSTEMS_NAMESPACE
void ClassificationSystemsDomain::InsertOmniClass33Definitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr omniClassSystem = TryAndGetSystem(db, "OmniClass");
    ClassificationPtr subsection0OmniClass;
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Planning", "33-11 00 00", "", nullptr, nullptr);
    ClassificationPtr subsection1OmniClass;
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Regional Planning", "33-11 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Development Planning", "33-11 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Rural Planning", "33-11 31 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Urban Planning", "33-11 41 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Transportation Planning", "33-11 44 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Environmental Planning", "33-11 51 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Facility Conservation Planning", "33-11 61 00", "", nullptr, subsection0OmniClass.get());
    ClassificationPtr subsection2OmniClass;
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Historic Building Conservation Planning", "33-11 61 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Ancient Monument Conservation Planning", "33-11 61 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Archaeological Area Conservation Planning", "33-11 61 41", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Planning", "33-11 99 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Master Planning", "33-11 99 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Permitting", "33-11 99 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Risk Assessment", "33-11 99 17", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Design", "33-21 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Architecture", "33-21 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Residential Architecture", "33-21 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Commercial Architecture", "33-21 11 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Institutional Architecture", "33-21 11 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Industrial Architecture", "33-21 11 27", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Drafting", "33-21 19 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Landscape Architecture", "33-21 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Interior Design", "33-21 23 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Specifying", "33-21 25 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Graphic Design", "33-21 27 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Signage Graphic Design", "33-21 27 11", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Engineering", "33-21 31 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Civil Engineering", "33-21 31 11", "", nullptr, subsection1OmniClass.get());
    ClassificationPtr subsection3OmniClass;
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Geotechnical Engineering", "33-21 31 11 11", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Structural Engineering", "33-21 31 14", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Foundation Engineering", "33-21 31 14 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "High Rise Engineering", "33-21 31 14 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Long-span Structure Engineering", "33-21 31 14 31", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Tensile Structure Engineering", "33-21 31 14 41", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Pneumatic Structure Engineering", "33-21 31 14 51", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Hydraulic Structure Engineering", "33-21 31 14 54", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Mechanical Engineering", "33-21 31 17", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Plumbing Engineering", "33-21 31 17 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Fire Protection Engineering", "33-21 31 17 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Heating Ventilation and Air Conditioning Engineering", "33-21 31 17 31", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Refrigeration Engineering", "33-21 31 17 33", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Energy Monitoring and Controls Engineering", "33-21 31 17 34", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Hydrological Engineering", "33-21 31 17 37", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Electrical Engineering", "33-21 31 21", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "High Voltage Electrical Engineering", "33-21 31 21 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Medium Voltage Electrical Engineering", "33-21 31 21 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Low Voltage Electrical Engineering", "33-21 31 21 31", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Communications Engineering", "33-21 31 23", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Computer Network Engineering", "33-21 31 23 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Alarm and Detection Engineering", "33-21 31 23 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Audiovisual Engineering", "33-21 31 23 21", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Process Engineering", "33-21 31 24", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Piping Engineering", "33-21 31 24 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Wind Engineering", "33-21 31 24 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Co-Generation Engineering", "33-21 31 24 31", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Nuclear Engineering", "33-21 31 24 41", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Sanitary Engineering", "33-21 31 24 51", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Military Engineering", "33-21 31 27", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Other Engineering", "33-21 31 99", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Acoustical/Emanations Shielding Engineering", "33-21 31 99 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Antiterrorism/Physical Security Engineering", "33-21 31 99 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Value Engineering", "33-21 31 99 17", "", nullptr, subsection2OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Other Design", "33-21 99 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Fountain Design", "33-21 99 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Finish Hardware Design", "33-21 99 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Extraterrestrial Design Specialist", "33-21 99 15", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Health Services Design Specialist", "33-21 99 22", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Hospital Design Specialist", "33-21 99 22 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Nursing Home Design Specialist", "33-21 99 22 21", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Infrastructure Design Specialist", "33-21 99 24", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Irrigation Design Specialist", "33-21 99 25", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Laboratory Design Specialist", "33-21 99 26", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Lighting Design Specialist", "33-21 99 28", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Marina Design Specialist", "33-21 99 29", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Model Making Specialist", "33-21 99 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Solar Design Specialist", "33-21 99 39", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Transportation Design Specialist", "33-21 99 45", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Air Transportation Design Specialist", "33-21 99 45 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Roadway Transportation Design Specialist", "33-21 99 45 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Marine Transportation Design Specialist", "33-21 99 45 31", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Vertical Transportation Design Specialist", "33-21 99 45 41", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Parking/Traffic Specialist Design Specialist", "33-21 99 45 51", "", nullptr, subsection2OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Project Management", "33-25 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Cost Estimation", "33-25 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Proposal Preparation", "33-25 14 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Client Briefing", "33-25 17 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Scheduling", "33-25 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Contract Administration", "33-25 31 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Procurement", "33-25 41 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Manufacturing", "33-25 41 11", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Building Product Manufacturing", "33-25 41 11 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Process Manufacturing", "33-25 41 11 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Construction Equipment Manufacturing", "33-25 41 11 17", "", nullptr, subsection2OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Construction Product Sales", "33-25 41 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Construction Product Marketing", "33-25 41 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Construction Product Purchasing", "33-25 41 21", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Quality Assurance", "33-25 51 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Quality Control", "33-25 54 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Surveying", "33-31 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Building Surveying", "33-31 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Site Surveying", "33-31 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "GIS (Geographical Information System) Engineering", "33-31 31 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Construction", "33-41 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Contracting", "33-41 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "General Contracting", "33-41 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Subcontracting", "33-41 11 14", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Masonry", "33-41 11 14 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Carpentry", "33-41 11 14 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Iron Working", "33-41 11 14 17", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Plumbing Subcontracting", "33-41 11 14 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Fire Protection Subcontracting", "33-41 11 14 24", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Heating Ventilation and Air Conditioning Subcontracting", "33-41 11 14 27", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Refrigeration Subcontracting", "33-41 11 14 28", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Electrical Subcontracting", "33-41 11 14 37", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Energy Monitoring and Controls Subcontracting", "33-41 11 14 51", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Hydrological Subcontracting", "33-41 11 14 54", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Painting", "33-41 11 14 61", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Tiling", "33-41 11 14 64", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Plaster Subcontracting", "33-41 11 14 66", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Gypsum Board Subcontracting", "33-41 11 14 69", "", nullptr, subsection2OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Construction Management", "33-41 14 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Facility Use Disciplines", "33-55 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Real Estate", "33-55 14 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Real Estate Sales", "33-55 14 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Property Appraising", "33-55 14 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Leasing", "33-55 14 17", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Owner", "33-55 21 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Facility Operations", "33-55 24 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Facility Space Planning", "33-55 24 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Facility Management", "33-55 24 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Facility Maintenance", "33-55 24 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Facility Services", "33-55 24 21", "", nullptr, subsection1OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Plumbing Operation and Maintenance", "33-55 24 21 11", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Fire Protection Operation and Maintenance", "33-55 24 21 14", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Heating Ventilation and Air Conditioning Operation and Maintenance", "33-55 24 21 17", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Refrigeration Operation and Maintenance", "33-55 24 21 21", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Energy Monitoring and Controls Operation and Maintenance", "33-55 24 21 24", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Hydrological Operation and Maintenance", "33-55 24 21 27", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Lightning Protection Operation and Maintenance", "33-55 24 21 31", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Life Safety Operation and Maintenance", "33-55 24 21 34", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Radiation Protection Operation and Maintenance", "33-55 24 21 37", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Moisture Protection Operation and Maintenance", "33-55 24 21 41", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Indoor Air Quality Evaluation", "33-55 24 21 44", "", nullptr, subsection2OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Communications Operation and Maintenance", "33-55 24 21 47", "", nullptr, subsection2OmniClass.get());
    ClassificationPtr subsection4OmniClass;
                    subsection4OmniClass = InsertClassification(*omniClassSystem, "Telecommunications Operation and Maintenance", "33-55 24 21 47 11", "", nullptr, subsection3OmniClass.get());
                    subsection4OmniClass = InsertClassification(*omniClassSystem, "Information Technology Operation and Maintenance", "33-55 24 21 47 14", "", nullptr, subsection3OmniClass.get());
                subsection3OmniClass = InsertClassification(*omniClassSystem, "Facility Shielding Operation and Maintenance", "33-55 24 21 51", "", nullptr, subsection2OmniClass.get());
                    subsection4OmniClass = InsertClassification(*omniClassSystem, "Acoustic Shielding Operation and Maintenance", "33-55 24 21 51 11", "", nullptr, subsection3OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Facility Restoration", "33-55 36 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Concrete Restoration", "33-55 36 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Masonry Restoration", "33-55 36 21", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Parking Restoration", "33-55 36 31", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Historic Restoration", "33-55 36 41", "", nullptr, subsection1OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Support Disciplines", "33-81 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Legal", "33-81 11 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Code Specialist", "33-81 11 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Forensic Specialist", "33-81 11 14", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Environment", "33-81 13 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Environmental Impact", "33-81 13 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Hazardous Materials Abatement", "33-81 13 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Tree Preservation Specialist", "33-81 13 44", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Finance", "33-81 31 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Banking", "33-81 31 11", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Accounting", "33-81 31 14", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Insurance", "33-81 31 17", "", nullptr, subsection1OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Bonding", "33-81 31 21", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Human Resources", "33-81 34 00", "", nullptr, subsection0OmniClass.get());
    subsection0OmniClass = InsertClassification(*omniClassSystem, "Other Disciplines", "33-99 00 00", "", nullptr, nullptr);
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Science", "33-99 11 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Art", "33-99 13 00", "", nullptr, subsection0OmniClass.get());
            subsection2OmniClass = InsertClassification(*omniClassSystem, "Photography", "33-99 13 11", "", nullptr, subsection1OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Security", "33-99 41 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Public Relations", "33-99 45 00", "", nullptr, subsection0OmniClass.get());
        subsection1OmniClass = InsertClassification(*omniClassSystem, "Education", "33-99 61 00", "", nullptr, subsection0OmniClass.get());
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
