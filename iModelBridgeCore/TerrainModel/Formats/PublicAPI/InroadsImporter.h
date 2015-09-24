/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/PublicAPI/InroadsImporter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

/*__PUBLISH_SECTION_START__*/
#include <TerrainModel/Formats/Formats.h>
#include <TerrainModel/Formats/TerrainImporter.h>
#include <Bentley/WString.h>
#include <list>
#include <TerrainModel/Core/IDTM.h>
#include <TerrainModel/Core/bcDTMClass.h>

TERRAINMODEL_TYPEDEFS (InroadsImporter)

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<InroadsImporter> InroadsImporterPtr;

struct InroadsImporter : SingleTerrainImporter
    {
    /*__PUBLISH_SECTION_END__*/
    private: InroadsImporter (WCharCP filename);

    protected: virtual ImportedTerrain _ImportTerrain (WCharCP name) const override;
    protected: virtual WCharCP _GetFileUnitString () const override;
    protected: virtual FileUnit _GetFileUnit () const override;
    /*__PUBLISH_SECTION_START__*/
    public: BENTLEYDTMFORMATS_EXPORT static bool IsFileSupported (WCharCP filename);
    public: BENTLEYDTMFORMATS_EXPORT static InroadsImporterPtr Create (WCharCP filename);
    };

END_BENTLEY_TERRAINMODEL_NAMESPACE
    