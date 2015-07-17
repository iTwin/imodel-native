/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/Lidar.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

public ref class LidarImporter : TerrainImporter
    {
    public: enum class Classification
        {
        Created = Bentley::TerrainModel::LidarImporter::Classification::Created,
        UnClassified = Bentley::TerrainModel::LidarImporter::Classification::UnClassified,
        Ground = Bentley::TerrainModel::LidarImporter::Classification::Ground,
        LowVegetation = Bentley::TerrainModel::LidarImporter::Classification::LowVegetation,
        MediumVegetation = Bentley::TerrainModel::LidarImporter::Classification::MediumVegetation,
        HighVegetation = Bentley::TerrainModel::LidarImporter::Classification::HighVegetation,
        Building = Bentley::TerrainModel::LidarImporter::Classification::Building,
        LowPoint = Bentley::TerrainModel::LidarImporter::Classification::LowPoint,
        Water = Bentley::TerrainModel::LidarImporter::Classification::Water,
        Rail = Bentley::TerrainModel::LidarImporter::Classification::Rail,
        RoadSurface = Bentley::TerrainModel::LidarImporter::Classification::RoadSurface,
        WireGuard = Bentley::TerrainModel::LidarImporter::Classification::WireGuard,
        WireConductor = Bentley::TerrainModel::LidarImporter::Classification::WireConductor,
        TransmissionTower = Bentley::TerrainModel::LidarImporter::Classification::TransmissionTower,
        WireStructureConnector = Bentley::TerrainModel::LidarImporter::Classification::WireStructureConnector,
        BridgeDeck = Bentley::TerrainModel::LidarImporter::Classification::BridgeDeck,
        HighNoise = Bentley::TerrainModel::LidarImporter::Classification::HighNoise,
        };

    ref class ClassificationInfo
        {
        public: property LidarImporter::Classification Classification;
        public: property UInt64 Count;
        };

    private: LidarImporterP m_importer;

    public: static LidarImporter^ Create (System::String^ filename);
    internal: LidarImporter (LidarImporterP importer);

    public: array<ClassificationInfo^>^ GetClassificationInfo ();
    public: Bentley::TerrainModelNET::DTM^ ImportTerrain (array<Classification>^ filter);
    };

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE