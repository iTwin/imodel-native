/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/Inroads.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

public ref class InroadsImporter : TerrainImporter
    {
    private: InroadsImporterP m_importer;

    public: static InroadsImporter^ Create (System::String^ filename);
    internal: InroadsImporter (InroadsImporterP importer);
    };

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE