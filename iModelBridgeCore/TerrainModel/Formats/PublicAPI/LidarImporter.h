/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/PublicAPI/LidarImporter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

/*__PUBLISH_SECTION_START__*/
#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Formats/TerrainImporter.h>
#include <Bentley/WString.h>
#include <list>
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/bcDTMClass.h>

TERRAINMODEL_TYPEDEFS (LidarImporter)

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<LidarImporter> LidarImporterPtr;

struct LidarImporter : SingleTerrainImporter
    {
    public: enum class Classification : unsigned char
        {
        Created = 0,
        UnClassified = 1,
        Ground = 2,
        LowVegetation = 3,
        MediumVegetation = 4,
        HighVegetation = 5,
        Building = 6,
        LowPoint = 7,
        Water = 9,
        Rail = 10,
        RoadSurface = 11,
        WireGuard = 13,
        WireConductor= 14,
        TransmissionTower = 15,
        WireStructureConnector = 16,
        BridgeDeck = 17,
        HighNoise = 18,
        };

    public: struct ClassificationInfo
        {
        Classification classification;
        uint64_t count;
        };

    /*__PUBLISH_SECTION_END__*/
    private: mutable bool m_lasFeatureCountsValid;
    private: mutable bvector<ClassificationInfo> m_classificationInfo;
    private: mutable BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr m_gcs;
    private: mutable bool m_gcsValid;
    private: LidarImporter (WCharCP filename);

    protected: virtual ImportedTerrain _ImportTerrain (WCharCP name) const override;
    protected: virtual BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr _GetGCS () const override;
    protected: virtual WCharCP LidarImporter::_GetFileUnitString () const override;
    protected: virtual FileUnit _GetFileUnit () const override;
    /*__PUBLISH_SECTION_START__*/
    public: BENTLEYDTMFORMATS_EXPORT static bool IsFileSupported (WCharCP filename);
    public: BENTLEYDTMFORMATS_EXPORT static LidarImporterPtr Create (WCharCP filename);

// Lidar specific
    public: BENTLEYDTMFORMATS_EXPORT bvector<ClassificationInfo> const& GetClassificationInfo () const;
    public: BENTLEYDTMFORMATS_EXPORT BcDTMPtr ImportTerrain (const bvector<Classification> classificationFilter) const;
    };

END_BENTLEY_TERRAINMODEL_NAMESPACE