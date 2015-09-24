/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/LidarImporter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/WString.h>
#include <list>
#include <TerrainModel/Formats/Lidar.h>
#include <TerrainModel/Formats/LidarImporter.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

DTMStatusInt bcdtmFormatLidar_getLasFileFeatureTypes (WCharCP lasFileNameP, bvector<LidarImporter::ClassificationInfo>& classificationInfo);
DTMStatusInt bcdtmFormatLidar_importLasFileFeaturesDtmObject
(
BC_DTM_OBJ *dtmP,                  // Pointer To DTM Object
WCharCP lasFileNameP,             // LAS File Name
const bvector<LidarImporter::Classification>* importFeatures,
uint64_t& totalNumLidarPoints,     // Number Of Lidar Points Imported For Each Feature
bvector<long>* numLidarPoints              // Number Of Lidar Points Imported For Each Feature
);
DTMStatusInt bcdtmFormatLidar_getGCS (WCharCP lasFileNameP, BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr& gcs);

LidarImporter::LidarImporter (WCharCP filename) : SingleTerrainImporter (filename)
        {
        m_lasFeatureCountsValid = false;
        m_gcsValid = false;
        }

bool LidarImporter::IsFileSupported (WCharCP filename)
    {
    if (BeFileName::GetExtension (filename).CompareToI (L"las") == 0)
        {
        BeFile file;

        if (file.Open (filename, BeFileAccess::Read) == BeFileStatus::Success)
            {
            char header[4];
            uint32_t bytesRead = 0;
            if (file.Read (header, &bytesRead, 4) == BeFileStatus::Success && bytesRead == 4)
                {
                file.Close ();
                if (header[0] == 'L' && header[1] == 'A' && header[2] == 'S' && header[3] == 'F')
                    return true;
                }
            file.Close ();
            }
        }
        return false;
    }

BENTLEYDTMFORMATS_EXPORT LidarImporterPtr LidarImporter::Create (WCharCP filename)
    {
    if (LidarImporter::IsFileSupported (filename))
        {
        return new LidarImporter (filename);
        }
    return nullptr;
    }

BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr LidarImporter::_GetGCS () const
    {
    if (!m_gcsValid)
        {
        m_gcsValid = true;
        if (bcdtmFormatLidar_getGCS (m_fileName.GetWCharCP (), m_gcs) != DTM_SUCCESS)
            m_gcs = nullptr;
        }
    return m_gcs;
    }

WCharCP LidarImporter::_GetFileUnitString () const
    {
    BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr gcs = _GetGCS ();
    if (gcs.IsValid ())
        {
        T_WStringVector* unitNames = BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::GetUnitNames ();
        return (*unitNames)[gcs->GetUnitCode()].GetWCharCP();
        }
    return TerrainImporter::_GetFileUnitString();
    }


FileUnit LidarImporter::_GetFileUnit () const
    {
    BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr gcs = _GetGCS ();
    if (gcs.IsValid())
        {
        return (FileUnit)gcs->GetUnitCode ();
        }
    return FileUnit::Unknown;
    }

ImportedTerrain LidarImporter::_ImportTerrain (WCharCP name) const
    {
    if (name == m_name)
        {
        BcDTMPtr dtm = BcDTM::Create ();

        uint64_t totalNumLidarPoints;
        if (bcdtmFormatLidar_importLasFileFeaturesDtmObject (dtm->GetTinHandle (), m_fileName.GetWCharCP (), nullptr, totalNumLidarPoints, nullptr) == DTM_SUCCESS)
            return ImportedTerrain (dtm.get (), m_name.GetWCharCP (), nullptr, true);
        }
    return ImportedTerrain (nullptr, name, nullptr, false);
    }

BcDTMPtr LidarImporter::ImportTerrain (const bvector<LidarImporter::Classification> classificationFilter) const
    {
    BcDTMPtr dtm = BcDTM::Create ();

    uint64_t totalNumLidarPoints;
    if (bcdtmFormatLidar_importLasFileFeaturesDtmObject (dtm->GetTinHandle (), m_fileName.GetWCharCP (), &classificationFilter, totalNumLidarPoints, nullptr) == DTM_SUCCESS)
        return dtm;
    return nullptr;
    }

bvector<LidarImporter::ClassificationInfo> const& LidarImporter::GetClassificationInfo () const
    {
    if (!m_lasFeatureCountsValid)
        {
        m_lasFeatureCountsValid = true;
        bcdtmFormatLidar_getLasFileFeatureTypes (m_fileName.GetWCharCP (), m_classificationInfo);
        }
    return m_classificationInfo;
    }
 