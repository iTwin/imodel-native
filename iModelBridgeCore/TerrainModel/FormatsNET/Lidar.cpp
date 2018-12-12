/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/Lidar.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Lidar.h"

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

LidarImporter^ LidarImporter::Create (System::String^ filename)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (filename);
    LidarImporterPtr importer = BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Create (p);

    if (importer.IsValid ())
        return gcnew LidarImporter (importer.get());
    return nullptr;
    }

LidarImporter::LidarImporter (LidarImporterP importer) : TerrainImporter (importer)
    {
    m_importer = importer;
    }

array<LidarImporter::ClassificationInfo^>^ LidarImporter::GetClassificationInfo ()
    {
    bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::ClassificationInfo> info;
    info = m_importer->GetClassificationInfo ();
    array<ClassificationInfo^>^ mInfo = gcnew array<ClassificationInfo^> ((int)info.size ());

    for (int i = 0; i < (int)info.size (); i++)
        {
        ClassificationInfo^ cInfo = gcnew ClassificationInfo ();
        cInfo->Classification = (Classification)info[i].classification;
        cInfo->Count = info[i].count;
        mInfo[i] = cInfo;
        }
    return mInfo;
    }

BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ LidarImporter::ImportTerrain (array<Classification>^ filter)
    {
    bvector<BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification> uFilter;

    for each (Classification classification in filter)
        {
        uFilter.push_back ((BENTLEY_NAMESPACE_NAME::TerrainModel::LidarImporter::Classification)classification);
        }
    BcDTMPtr dtm = m_importer->ImportTerrain (uFilter);
    return dtm.IsValid() ? BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM::FromHandle ((System::IntPtr)dtm.get()) : nullptr;
    }
END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE