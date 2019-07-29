/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "MX.h"

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

MXFilImporter^ MXFilImporter::Create (System::String^ filename)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (filename);
    MXFilImporterPtr importer = BENTLEY_NAMESPACE_NAME::TerrainModel::MXFilImporter::Create (p);
    if (importer.IsValid ())
        return gcnew MXFilImporter (importer.get ());
    return nullptr;
    }

MXFilImporter::MXFilImporter (MXFilImporterP importer) : TerrainImporter (importer)
    {
    m_importer = importer;
    }

MXFilExporter::MXFilExporter()
    {
    MXFilExporterPtr exporter = Bentley::TerrainModel::MXFilExporter::Create();
    m_exporter = exporter.get();
    SetTerrainExporter(m_exporter);
    }

MXFilExporter::MXExportError MXFilExporter::Export(System::String^ filename, System::String^ modelName, System::String^ stringName, NamedTerrain^ dtm, bool allowOverwrite)
    {
    pin_ptr<const wchar_t> nFilename = PtrToStringChars(filename);
    pin_ptr<const wchar_t> nModelName = PtrToStringChars(modelName);
    pin_ptr<const wchar_t> nStringName = PtrToStringChars(stringName);

    pin_ptr<const wchar_t> uName = PtrToStringChars(dtm->Name);
    pin_ptr<const wchar_t> uDescription = PtrToStringChars(dtm->Description);
    BcDTMP dtmP = (BcDTMP)dtm->Terrain->ExternalHandle.ToPointer();
    Bentley::TerrainModel::LandXMLExporter::NamedDTM uTerrain(dtmP, uName, uDescription);

    return (MXExportError)m_exporter->Export(nFilename, nModelName, nStringName, uTerrain, allowOverwrite);
    }

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE