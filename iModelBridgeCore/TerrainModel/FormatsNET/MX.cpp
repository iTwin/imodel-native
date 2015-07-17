/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/MX.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE