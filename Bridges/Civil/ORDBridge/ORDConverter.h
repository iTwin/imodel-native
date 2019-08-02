/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDConverter.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
    friend struct ORDAlignmentsConverter;
    friend struct ORDCorridorsConverter;

protected:
    virtual void _OnConversionComplete() override;
    virtual bool _ShouldImportSchema(Utf8StringCR fullSchemaName, DgnV8ModelR v8Model) override;
    virtual void _OnSheetsConvertViewAttachment(Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const& v8SheetModelMapping, DgnAttachmentR v8DgnAttachment) override;    

public:
    struct Params
        {
        Params(Dgn::iModelBridge::Params const& iModelBridgeParams, Dgn::SubjectCR subject, Dgn::iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, 
            Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId, Dgn::UnitSystem rootModelUnitSystem, Dgn::iModelBridgeSyncInfoFile& syncInfo) :
            iModelBridgeParamsCP(&iModelBridgeParams), subjectCPtr(&subject), changeDetectorP(&changeDetector), fileScopeId(fileScopeId), 
            spatialDataTransformHasChanged(false), rootModelUnitSystem(rootModelUnitSystem), syncInfo(syncInfo), domainModelsPrivate(true)
            {}

        Dgn::iModelBridge::Params const* iModelBridgeParamsCP;
        Dgn::SubjectCPtr subjectCPtr;
        Dgn::iModelBridgeSyncInfoFile::ChangeDetector* changeDetectorP;
        Dgn::iModelBridgeSyncInfoFile::ROWID fileScopeId;
        bool spatialDataTransformHasChanged;
        bool isCreatingNewDgnDb;
        bool isUpdating;
        Dgn::UnitSystem rootModelUnitSystem;
        Dgn::iModelBridgeSyncInfoFile& syncInfo;
        bool domainModelsPrivate;
        };

private:
    Params* m_ordParams;
    RoadRailPhysical::RoadRailNetworkCPtr m_roadRailNetworkCPtr;
    bmap<Bentley::ElementRefP, Dgn::DgnElementPtr> m_v8ToBimElmMap;

    bvector<Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Corridor>> m_cifCorridors;
    bvector<Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Alignment>> m_cifAlignments;    
    bvector<Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::LinearEntity3d>> m_cifGeneratedLinear3ds;
    bool m_isProcessing;

    void CreateRoadRailElements();
    void CreateAlignments();
    void CreatePathways();
    void SetCorridorDesignAlignments();
    void AssociateGeneratedAlignments();

public:
    ORDConverter(Dgn::DgnDbSync::DgnV8::RootModelConverter::RootModelSpatialParams& params) : 
        Dgn::DgnDbSync::DgnV8::RootModelConverter(params), m_isProcessing(false)
        {
        // When EC Content is skipped, item types don't get converted.  By default, the framework has this 
        // m_skipECContent set to true.  Let's change that for us to default to false so we default to getting
        // item types converted.
        m_skipECContent = false;
        this->SetKeepHostAlive(true);
        }

    virtual ~ORDConverter()
        {
        if (m_dgndb.IsValid())
            m_dgndb = nullptr;
        }

    void SetORDParams(Params* ordParams) { m_ordParams = ordParams; }
    void SetIsProcessing(bool newVal) { m_isProcessing = newVal; }

    RoadRailPhysical::RoadRailNetworkCP GetRoadRailNetwork() const { return m_roadRailNetworkCPtr.get(); }
    void SetRoadRailNetwork(RoadRailPhysical::RoadRailNetworkCR network) { m_roadRailNetworkCPtr = &network; }
    void SetUpModelFormatters();
    Dgn::UnitSystem GetRootModelUnitSystem();
    BentleyStatus AddDynamicSchema();
}; // ORDConverter

struct ConvertORDElementXDomain : Dgn::DgnDbSync::DgnV8::XDomain
{
private:
    ORDConverter& m_converter;
    Dgn::DgnClassId m_graphic3dClassId;
    bset<Bentley::ElementRefP> m_elementsSeen;    
    bset<Bentley::ElementRefP> m_alignmentV8RefSet;
    bset<Bentley::ElementRefP> m_corridorV8RefSet;
    Bentley::Cif::ConsensusConnectionPtr m_cifConsensusConnection;

    typedef bool (ConvertORDElementXDomain::*AspectAssignmentFunc)(Dgn::DgnElementR, DgnV8EhCR) const;
    bvector<AspectAssignmentFunc> m_aspectAssignFuncs;

    bool AssignLinearQuantityAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const;
    bool AssignTemplateDropAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const;
    bool AssignSuperelevationAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const;
    bool AssignCorridorAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const;
    bool AssignCorridorSurfaceAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const;
    bool AssignFeatureAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const;

protected:
    virtual void _DetermineElementParams(Dgn::DgnClassId&, Dgn::DgnCode&, Dgn::DgnCategoryId&, DgnV8EhCR, Dgn::DgnDbSync::DgnV8::Converter&, ECObjectsV8::IECInstance const* primaryV8Instance, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&) override;
    virtual Result _PreConvertElement(DgnV8EhCR, Dgn::DgnDbSync::DgnV8::Converter&, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&) override;
    virtual void _ProcessResults(Dgn::DgnDbSync::DgnV8::ElementConversionResults&, DgnV8EhCR, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const&, Dgn::DgnDbSync::DgnV8::Converter&) override;
    virtual void _OnFinishConversion(Dgn::DgnDbSync::DgnV8::Converter&) override {};

public:
    ConvertORDElementXDomain(ORDConverter& converter);
}; // ConvertORDElementXDomain

END_ORDBRIDGE_NAMESPACE