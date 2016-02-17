/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/MX.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "MX.h"

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

MXFilImporter^ MXFilImporter::Create (System::String^ filename)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (filename);
    MXFilImporterPtr importer = Bentley::TerrainModel::MXFilImporter::Create (p);
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

MXFilExporter::MXExportError MXFilExporter::Export(System::String^ filename, System::String^ modelName, System::String^ stringName, Bentley::TerrainModelNET::DTM^ dtm, bool allowOverwrite)
    {
    pin_ptr<const wchar_t> nFilename = PtrToStringChars(filename);
    pin_ptr<const wchar_t> nModelName = PtrToStringChars(modelName);
    pin_ptr<const wchar_t> nStringName = PtrToStringChars(stringName);
    BcDTMP nDtm = (BcDTMP)dtm->ExternalHandle.ToPointer();

    return (MXExportError)m_exporter->Export(nFilename, nModelName, nStringName, nDtm, allowOverwrite);
    }

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE