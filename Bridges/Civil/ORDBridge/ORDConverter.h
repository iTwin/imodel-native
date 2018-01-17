/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDConverter.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
        Params(Dgn::iModelBridge::Params const& iModelBridgeParams, Dgn::SubjectCR subject, Dgn::iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId) :
            iModelBridgeParamsCP(&iModelBridgeParams), subjectCPtr(&subject), changeDetectorP(&changeDetector), fileScopeId(fileScopeId), spatialDataTransformHasChanged(false)
            {}

        Dgn::iModelBridge::Params const* iModelBridgeParamsCP;
        Dgn::SubjectCPtr subjectCPtr;
        Dgn::iModelBridgeSyncInfoFile::ChangeDetector* changeDetectorP;
        Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId;
        bool spatialDataTransformHasChanged;
        bool isCreatingNewDgnDb;
        };

private:
    void ConvertAlignments(Bentley::Cif::GeometryModel::SDK::GeometricModel const&, Dgn::DgnDbSync::DgnV8::ConverterLibrary& converterLib, Params& params);
    void ConvertCorridors(Bentley::Cif::GeometryModel::SDK::GeometricModel const&, Dgn::DgnDbSync::DgnV8::ConverterLibrary& converterLib, Params& params);

public:
    void ConvertORDData(Params& params);
}; // ORDConverter

struct ORDV8Converter : Dgn::DgnDbSync::DgnV8::RootModelConverter
{
protected:
    virtual bool _ShouldImportSchema(Utf8StringCR fullSchemaName, DgnV8ModelR v8Model) override;
    virtual Dgn::DgnModelId _MapModelIntoProject(DgnV8ModelR v8Model, Utf8CP, DgnV8Api::DgnAttachment const* attachment) override;

public:
    ORDV8Converter(Dgn::DgnDbSync::DgnV8::RootModelConverter::RootModelSpatialParams& params) : 
        Dgn::DgnDbSync::DgnV8::RootModelConverter(params)
        {}
}; // ORDV8Converter

struct ConvertORDElementExtension : Dgn::DgnDbSync::DgnV8::ConvertToDgnDbElementExtension
    {
    static void Register();
    virtual Result _PreConvertElement(DgnV8EhCR, Dgn::DgnDbSync::DgnV8::Converter&, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&) override;
    };

END_ORDBRIDGE_NAMESPACE