/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/Lidar.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

public ref class LidarImporter : TerrainImporter
    {
    public: enum class Classification
        {
        Created = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::Created,
        UnClassified = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::UnClassified,
        Ground = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::Ground,
        LowVegetation = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::LowVegetation,
        MediumVegetation = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::MediumVegetation,
        HighVegetation = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::HighVegetation,
        Building = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::Building,
        LowPoint = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::LowPoint,
        Water = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::Water,
        Rail = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::Rail,
        RoadSurface = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::RoadSurface,
        WireGuard = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::WireGuard,
        WireConductor = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::WireConductor,
        TransmissionTower = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::TransmissionTower,
        WireStructureConnector = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::WireStructureConnector,
        BridgeDeck = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::BridgeDeck,
        HighNoise = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification::HighNoise,
        };

    ref class ClassificationInfo
        {
        public: property LidarImporter::Classification Classification;
        public: property uint64_t Count;
        };

    private: LidarImporterP m_importer;

    public: static LidarImporter^ Create (System::String^ filename);
    internal: LidarImporter (LidarImporterP importer);

    public: array<ClassificationInfo^>^ GetClassificationInfo ();
    public: BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ ImportTerrain (array<Classification>^ filter);
    };

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE