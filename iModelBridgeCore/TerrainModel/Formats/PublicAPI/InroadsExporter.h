/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

/*__PUBLISH_SECTION_START__*/
#include "TerrainModel/Formats/InroadsImporter.h"
#include "TerrainModel/Formats/TerrainExporter.h"


TERRAINMODEL_TYPEDEFS(InroadsExporter)

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<InroadsExporter> InroadsExporterPtr;

struct InroadsExporter : TerrainExporter
    {

    /*__PUBLISH_SECTION_END__*/
    protected: InroadsExporter()
        {
        }

    /*__PUBLISH_SECTION_START__*/
    public: BENTLEYDTMFORMATS_EXPORT static InroadsExporterPtr Create();

    public: BENTLEYDTMFORMATS_EXPORT StatusInt Export(WCharCP filename, NamedDTM const& dtm);
    };

END_BENTLEY_TERRAINMODEL_NAMESPACE