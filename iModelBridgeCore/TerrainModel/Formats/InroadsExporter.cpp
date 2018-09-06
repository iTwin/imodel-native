/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/InroadsExporter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/WString.h>
#include <list>
#include <TerrainModel/Formats/InroadsExporter.h>
#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>
#include <Geom/GeomApi.h>
#include "TriangulationPreserver.h"
#include <TerrainModel/Formats/inroads.h>
#include "InroadsTM/InroadsTM.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL

BENTLEYDTMFORMATS_EXPORT InroadsExporterPtr InroadsExporter::Create()
    {
    return new InroadsExporter();
    }

StatusInt InroadsExporter::Export(WCharCP filename, NamedDTM const& dtm)
    {
    dtm.GetBcDTMP()->SetMemoryAccess(DTMAccessMode::Temporary);
    bcdtmFormatInroads_exportBclibDtmToInroadsDtmFile(dtm.GetBcDTMP()->GetTinHandle(), filename, dtm.GetName(), dtm.GetDescription());
    return SUCCESS;
    }

