/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/Inroads.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "Inroads.h"

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

InroadsImporter^ InroadsImporter::Create (System::String^ filename)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (filename);
    InroadsImporterPtr importer = Bentley::TerrainModel::InroadsImporter::Create (p);

    if (importer.IsValid ())
        return gcnew InroadsImporter (importer.get());
    return nullptr;
    }

InroadsImporter::InroadsImporter (InroadsImporterP importer) : TerrainImporter (importer)
    {
    m_importer = importer;
    }

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE