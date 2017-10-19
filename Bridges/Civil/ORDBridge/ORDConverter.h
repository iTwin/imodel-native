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
private:
    void ConvertAlignments(Bentley::Cif::GeometryModel::SDK::GeometricModel const&, 
        Dgn::DgnDbSync::DgnV8::ConverterLibrary& converterLib,
        Dgn::iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId);
    void ConvertCorridors(Bentley::Cif::GeometryModel::SDK::GeometricModel const&, 
        Dgn::DgnDbSync::DgnV8::ConverterLibrary& converterLib,
        Dgn::iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId);

public:
    void ConvertORDData(BeFileNameCR dgnFileName, Dgn::SubjectCR subject, Dgn::iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId);
}; // ORDConverter

END_ORDBRIDGE_NAMESPACE