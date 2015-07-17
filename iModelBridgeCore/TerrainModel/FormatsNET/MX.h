/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/MX.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

public ref class MXFilImporter : TerrainImporter
    {
    MXFilImporterP m_importer;

    public: static MXFilImporter^ Create (System::String^ filename);
    internal: MXFilImporter (MXFilImporterP importer);
    };

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE