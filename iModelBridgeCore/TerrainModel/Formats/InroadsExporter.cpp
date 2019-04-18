/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

StatusInt InroadsExporter::Export(WCharCP filename, NamedDTM const& namedDtm)
    {
    BcDTMPtr transformedDTM;

    Transform transform;
    BcDTMP dtm = namedDtm.GetBcDTMP();

    bool hasTransform = !dtm->GetTransformation(transform);
    if (hasTransform)
        {
        DPoint3d fixedPoint;
        double scale;
        double aspectFix;
        RotMatrix axes;

        if (!transform.IsTranslateScaleRotateAroundZ(fixedPoint, axes, scale, aspectFix))
            {
            return ERROR;
            }
        else if (0 != aspectFix)
            {
            return ERROR;
            }

        BC_DTM_OBJ *dtmHandleP = nullptr;

        bcdtmObject_cloneDtmObject(dtm->GetTinHandle(), (BC_DTM_OBJ **)&dtmHandleP);

        // Create a new Digital TM instance
        transformedDTM = BcDTM::CreateFromDtmHandle(*dtmHandleP);

        transformedDTM->Transform(transform);
        dtm = transformedDTM.get();
        }
    else
        dtm->SetMemoryAccess(DTMAccessMode::Temporary);
    bcdtmFormatInroads_exportBclibDtmToInroadsDtmFile(dtm->GetTinHandle(), filename, namedDtm.GetName(), namedDtm.GetDescription());
    return SUCCESS;
    }

