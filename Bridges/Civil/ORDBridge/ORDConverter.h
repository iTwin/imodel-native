/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDConverter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "ORDBridgeInternal.h"

BEGIN_ORDBRIDGE_NAMESPACE

struct ORDConverter
{
public:
    struct Params
        {
        Params(BeFileNameCR dgnFileName, Dgn::SubjectCR subject, Dgn::iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId) :
            dgnFileName(dgnFileName), subjectCPtr(&subject), changeDetectorP(&changeDetector), fileScopeId(fileScopeId), spatialDataTransformHasChanged(false)
            {}

        BeFileName dgnFileName;
        Dgn::SubjectCPtr subjectCPtr;
        Dgn::iModelBridgeSyncInfoFile::ChangeDetector* changeDetectorP;
        Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId;
        bool spatialDataTransformHasChanged;
        };

private:
    void ConvertAlignments(Bentley::Cif::GeometryModel::SDK::GeometricModel const&, Dgn::DgnDbSync::DgnV8::ConverterLibrary& converterLib, Params& params);
    void ConvertCorridors(Bentley::Cif::GeometryModel::SDK::GeometricModel const&, Dgn::DgnDbSync::DgnV8::ConverterLibrary& converterLib, Params& params);

public:
    void ConvertORDData(Params& params);
}; // ORDConverter

END_ORDBRIDGE_NAMESPACE