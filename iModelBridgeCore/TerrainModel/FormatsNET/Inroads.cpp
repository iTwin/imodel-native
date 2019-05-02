/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Inroads.h"

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

InroadsImporter^ InroadsImporter::Create (System::String^ filename)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (filename);
    InroadsImporterPtr importer = BENTLEY_NAMESPACE_NAME::TerrainModel::InroadsImporter::Create (p);

    if (importer.IsValid ())
        return gcnew InroadsImporter (importer.get());
    return nullptr;
    }

InroadsImporter::InroadsImporter (InroadsImporterP importer) : TerrainImporter (importer)
    {
    m_importer = importer;
    }

InroadsExporter::InroadsExporter()
    {
    InroadsExporterPtr exporter = Bentley::TerrainModel::InroadsExporter::Create();
    m_exporter = exporter.get();
    SetTerrainExporter(m_exporter);
    }

void InroadsExporter::CreateDTM(System::String^ filename, NamedTerrain^ terrain)
    {
    pin_ptr<const wchar_t> uFilename = PtrToStringChars(filename);
    pin_ptr<const wchar_t> uName = PtrToStringChars(terrain->Name);
    pin_ptr<const wchar_t> uDescription = PtrToStringChars(terrain->Description);
    BcDTMP dtmP = (BcDTMP) terrain->Terrain->ExternalHandle.ToPointer();
    Bentley::TerrainModel::TerrainExporter::NamedDTM uTerrain(dtmP, uName, uDescription);
    m_exporter->Export(uFilename, uTerrain);
    }

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE