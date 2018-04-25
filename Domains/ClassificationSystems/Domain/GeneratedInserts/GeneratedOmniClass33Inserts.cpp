/*--------------------------------------------------------------------------------------+
|
|  $Source: Domain/GeneratedInserts/GeneratedOmniClass33Inserts.cpp $
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
void ClassificationSystemsDomain::InsertOmniClass33Definitions(Dgn::DgnDbR db) const
    {
    ClassificationSystemCPtr omniClassSystem = TryAndGetSystem(db, "OmniClass");
    OmniClassClassDefinitionPtr subsection0OmniClass;
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "33-11 00 00", "Planning", nullptr);
    OmniClassClassDefinitionPtr subsection1OmniClass;
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-11 11 00", "Regional Planning", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-11 21 00", "Development Planning", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-11 31 00", "Rural Planning", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-11 41 00", "Urban Planning", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-11 44 00", "Transportation Planning", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-11 51 00", "Environmental Planning", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-11 61 00", "Facility Conservation Planning", subsection0OmniClass.get());
    OmniClassClassDefinitionPtr subsection2OmniClass;
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-11 61 21", "Historic Building Conservation Planning", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-11 61 31", "Ancient Monument Conservation Planning", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-11 61 41", "Archaeological Area Conservation Planning", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-11 99 00", "Other Planning", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-11 99 11", "Master Planning", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-11 99 14", "Permitting", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-11 99 17", "Risk Assessment", subsection1OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "33-21 00 00", "Design", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-21 11 00", "Architecture", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 11 11", "Residential Architecture", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 11 21", "Commercial Architecture", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 11 24", "Institutional Architecture", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 11 27", "Industrial Architecture", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-21 19 00", "Drafting", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-21 21 00", "Landscape Architecture", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-21 23 00", "Interior Design", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-21 25 00", "Specifying", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-21 27 00", "Graphic Design", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 27 11", "Signage Graphic Design", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 00", "Engineering", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 11", "Civil Engineering", subsection1OmniClass.get());
    OmniClassClassDefinitionPtr subsection3OmniClass;
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 11 11", "Geotechnical Engineering", subsection2OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 14", "Structural Engineering", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 14 11", "Foundation Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 14 21", "High Rise Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 14 31", "Long-span Structure Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 14 41", "Tensile Structure Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 14 51", "Pneumatic Structure Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 14 54", "Hydraulic Structure Engineering", subsection2OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 17", "Mechanical Engineering", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 17 11", "Plumbing Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 17 21", "Fire Protection Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 17 31", "Heating Ventilation and Air Conditioning Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 17 33", "Refrigeration Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 17 34", "Energy Monitoring and Controls Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 17 37", "Hydrological Engineering", subsection2OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 21", "Electrical Engineering", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 21 11", "High Voltage Electrical Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 21 21", "Medium Voltage Electrical Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 21 31", "Low Voltage Electrical Engineering", subsection2OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 23", "Communications Engineering", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 23 11", "Computer Network Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 23 14", "Alarm and Detection Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 23 21", "Audiovisual Engineering", subsection2OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 24", "Process Engineering", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 24 11", "Piping Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 24 21", "Wind Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 24 31", "Co-Generation Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 24 41", "Nuclear Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 24 51", "Sanitary Engineering", subsection2OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 27", "Military Engineering", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 99", "Other Engineering", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 99 11", "Acoustical/Emanations Shielding Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 99 14", "Antiterrorism/Physical Security Engineering", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 31 99 17", "Value Engineering", subsection2OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 00", "Other Design", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 11", "Fountain Design", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 14", "Finish Hardware Design", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 15", "Extraterrestrial Design Specialist", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 22", "Health Services Design Specialist", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 22 11", "Hospital Design Specialist", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 22 21", "Nursing Home Design Specialist", subsection2OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 24", "Infrastructure Design Specialist", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 25", "Irrigation Design Specialist", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 26", "Laboratory Design Specialist", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 28", "Lighting Design Specialist", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 29", "Marina Design Specialist", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 31", "Model Making Specialist", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 39", "Solar Design Specialist", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 45", "Transportation Design Specialist", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 45 11", "Air Transportation Design Specialist", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 45 21", "Roadway Transportation Design Specialist", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 45 31", "Marine Transportation Design Specialist", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 45 41", "Vertical Transportation Design Specialist", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-21 99 45 51", "Parking/Traffic Specialist Design Specialist", subsection2OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "33-25 00 00", "Project Management", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-25 11 00", "Cost Estimation", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-25 14 00", "Proposal Preparation", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-25 17 00", "Client Briefing", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-25 21 00", "Scheduling", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-25 31 00", "Contract Administration", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-25 41 00", "Procurement", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-25 41 11", "Manufacturing", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-25 41 11 11", "Building Product Manufacturing", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-25 41 11 14", "Process Manufacturing", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-25 41 11 17", "Construction Equipment Manufacturing", subsection2OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-25 41 14", "Construction Product Sales", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-25 41 17", "Construction Product Marketing", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-25 41 21", "Construction Product Purchasing", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-25 51 00", "Quality Assurance", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-25 54 00", "Quality Control", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "33-31 00 00", "Surveying", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-31 11 00", "Building Surveying", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-31 21 00", "Site Surveying", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-31 31 00", "GIS (Geographical Information System) Engineering", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "33-41 00 00", "Construction", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 00", "Contracting", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 11", "General Contracting", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14", "Subcontracting", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 11", "Masonry", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 14", "Carpentry", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 17", "Iron Working", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 21", "Plumbing Subcontracting", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 24", "Fire Protection Subcontracting", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 27", "Heating Ventilation and Air Conditioning Subcontracting", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 28", "Refrigeration Subcontracting", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 37", "Electrical Subcontracting", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 51", "Energy Monitoring and Controls Subcontracting", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 54", "Hydrological Subcontracting", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 61", "Painting", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 64", "Tiling", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 66", "Plaster Subcontracting", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-41 11 14 69", "Gypsum Board Subcontracting", subsection2OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-41 14 00", "Construction Management", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "33-55 00 00", "Facility Use Disciplines", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-55 14 00", "Real Estate", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-55 14 11", "Real Estate Sales", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-55 14 14", "Property Appraising", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-55 14 17", "Leasing", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-55 21 00", "Owner", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 00", "Facility Operations", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 11", "Facility Space Planning", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 14", "Facility Management", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 17", "Facility Maintenance", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21", "Facility Services", subsection1OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 11", "Plumbing Operation and Maintenance", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 14", "Fire Protection Operation and Maintenance", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 17", "Heating Ventilation and Air Conditioning Operation and Maintenance", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 21", "Refrigeration Operation and Maintenance", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 24", "Energy Monitoring and Controls Operation and Maintenance", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 27", "Hydrological Operation and Maintenance", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 31", "Lightning Protection Operation and Maintenance", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 34", "Life Safety Operation and Maintenance", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 37", "Radiation Protection Operation and Maintenance", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 41", "Moisture Protection Operation and Maintenance", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 44", "Indoor Air Quality Evaluation", subsection2OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 47", "Communications Operation and Maintenance", subsection2OmniClass.get());
    OmniClassClassDefinitionPtr subsection4OmniClass;
                    subsection4OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 47 11", "Telecommunications Operation and Maintenance", subsection3OmniClass.get());
                    subsection4OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 47 14", "Information Technology Operation and Maintenance", subsection3OmniClass.get());
                subsection3OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 51", "Facility Shielding Operation and Maintenance", subsection2OmniClass.get());
                    subsection4OmniClass = InsertOmniClass(*omniClassSystem, "33-55 24 21 51 11", "Acoustic Shielding Operation and Maintenance", subsection3OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-55 36 00", "Facility Restoration", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-55 36 11", "Concrete Restoration", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-55 36 21", "Masonry Restoration", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-55 36 31", "Parking Restoration", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-55 36 41", "Historic Restoration", subsection1OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "33-81 00 00", "Support Disciplines", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-81 11 00", "Legal", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-81 11 11", "Code Specialist", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-81 11 14", "Forensic Specialist", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-81 13 00", "Environment", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-81 13 11", "Environmental Impact", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-81 13 14", "Hazardous Materials Abatement", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-81 13 44", "Tree Preservation Specialist", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-81 31 00", "Finance", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-81 31 11", "Banking", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-81 31 14", "Accounting", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-81 31 17", "Insurance", subsection1OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-81 31 21", "Bonding", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-81 34 00", "Human Resources", subsection0OmniClass.get());
    subsection0OmniClass = InsertOmniClass(*omniClassSystem, "33-99 00 00", "Other Disciplines", nullptr);
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-99 11 00", "Science", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-99 13 00", "Art", subsection0OmniClass.get());
            subsection2OmniClass = InsertOmniClass(*omniClassSystem, "33-99 13 11", "Photography", subsection1OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-99 41 00", "Security", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-99 45 00", "Public Relations", subsection0OmniClass.get());
        subsection1OmniClass = InsertOmniClass(*omniClassSystem, "33-99 61 00", "Education", subsection0OmniClass.get());
    }

END_CLASSIFICATIONSYSTEMS_NAMESPACE
