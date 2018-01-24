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

struct ORDConverter : Dgn::DgnDbSync::DgnV8::RootModelConverter
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

    ORDConverter(Dgn::DgnDbSync::DgnV8::RootModelConverter::RootModelSpatialParams& params) : 
        Dgn::DgnDbSync::DgnV8::RootModelConverter(params)
        {}
}; // ORDConverter

struct ConvertORDElementXDomain : Dgn::DgnDbSync::DgnV8::XDomain
{
private:
    ORDConverter::Params& m_params;
    ORDConverter& m_converter;
    Dgn::DgnClassId m_spatialLocationClassId;
    bset<Bentley::DgnPlatform::ElementId> m_elementsSeen;
    bmap<Bentley::DgnPlatform::ElementId, bpair<Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Alignment>, Dgn::DgnElementPtr>> m_alignmentsMap;
    bmap<Bentley::DgnPlatform::ElementId, bpair<Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Corridor>, Dgn::DgnElementPtr>> m_corridorsMap;

protected:
    virtual void _DetermineElementParams(Dgn::DgnClassId&, Dgn::DgnCode&, Dgn::DgnCategoryId&, DgnV8EhCR, Dgn::DgnDbSync::DgnV8::Converter&, ECObjectsV8::IECInstance const* primaryV8Instance, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&) override;
    virtual Result _PreConvertElement(DgnV8EhCR, Dgn::DgnDbSync::DgnV8::Converter&, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&) override;
    virtual void _ProcessResults(Dgn::DgnDbSync::DgnV8::ElementConversionResults&, DgnV8EhCR, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&, Dgn::DgnDbSync::DgnV8::Converter&) override;

public:
    ConvertORDElementXDomain(ORDConverter& converter, ORDConverter::Params& params);

    void CreateRoadRailElements();
}; // ConvertORDElementXDomain

END_ORDBRIDGE_NAMESPACE