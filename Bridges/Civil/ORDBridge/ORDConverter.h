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
    DEFINE_T_SUPER(Dgn::DgnDbSync::DgnV8::RootModelConverter)

    friend struct ConvertORDElementXDomain;

protected:
    virtual void _OnConversionComplete() override;
    virtual bool _ShouldImportSchema(Utf8StringCR fullSchemaName, DgnV8ModelR v8Model) override;

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
    Params* m_ordParams;
    bmap<Bentley::ElementRefP, Dgn::DgnElementPtr> m_v8ToBimElmMap;

    typedef bpair<Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Alignment>, Bentley::ElementRefP> CifAlignmentV8RefPair;
    typedef bpair<Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Corridor>, Bentley::ElementRefP> CifCorridorV8RefPair;

    bvector<CifAlignmentV8RefPair> m_cifAlignments;
    bvector<CifCorridorV8RefPair> m_cifCorridors;

    void CreateRoadRailElements();

public:
    ORDConverter(Dgn::DgnDbSync::DgnV8::RootModelConverter::RootModelSpatialParams& params) : 
        Dgn::DgnDbSync::DgnV8::RootModelConverter(params)
        {}

    void SetORDParams(Params* ordParams) { m_ordParams = ordParams; }
}; // ORDConverter

struct ConvertORDElementXDomain : Dgn::DgnDbSync::DgnV8::XDomain
{
private:
    ORDConverter& m_converter;
    Dgn::DgnClassId m_spatialLocationClassId;
    bset<Bentley::ElementRefP> m_elementsSeen;    
    bset<Bentley::ElementRefP> m_alignmentV8RefSet;
    bset<Bentley::ElementRefP> m_corridorV8RefSet;

protected:
    virtual void _DetermineElementParams(Dgn::DgnClassId&, Dgn::DgnCode&, Dgn::DgnCategoryId&, DgnV8EhCR, Dgn::DgnDbSync::DgnV8::Converter&, ECObjectsV8::IECInstance const* primaryV8Instance, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&) override;
    virtual Result _PreConvertElement(DgnV8EhCR, Dgn::DgnDbSync::DgnV8::Converter&, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&) override;
    virtual void _ProcessResults(Dgn::DgnDbSync::DgnV8::ElementConversionResults&, DgnV8EhCR, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&, Dgn::DgnDbSync::DgnV8::Converter&) override;

public:
    ConvertORDElementXDomain(ORDConverter& converter);
}; // ConvertORDElementXDomain

END_ORDBRIDGE_NAMESPACE