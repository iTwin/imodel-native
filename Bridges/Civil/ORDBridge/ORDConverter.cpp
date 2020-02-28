/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ORDBridgeInternal.h>
#include <windows.h>
#include <VersionedDgnV8Api/GeomSerialization/GeomLibsFlatBufferApi.h>
#include "DynamicSchemaComparer.h"
#include "ScalableMeshWrapper.h"

#define DefaultDesignAlignmentsName     "Road/Rail Design Alignments"
#define DefaultTransportationSystemName "Transportation System"
#define QuantityTakeoffsSchemaName      "QuantityTakeoffsAspects"

BEGIN_ORDBRIDGE_NAMESPACE

struct ORDConverterExtensionRegistry
{
public:
    static bvector<ORDConverterExtension*> s_extensions;
}; // ORDConverterExtensionRegistry

bvector<ORDConverterExtension*> ORDConverterExtensionRegistry::s_extensions;

struct StopWatchCummulative
{
private:
    BentleyApi::StopWatch m_stopWatch;
    BentleyApi::BeDuration m_cummulative;
    size_t m_count;

    void StopAndAddElapsed()
        {
        m_stopWatch.Stop();
        m_cummulative += m_stopWatch.GetElapsed();
        m_count++;
        }

public:
    StopWatchCummulative(Utf8CP description) : m_stopWatch(description), m_count(0) {}

    void Start() { m_stopWatch.Start(); }
    void Stop() { StopAndAddElapsed(); }
    Utf8CP GetDescription() { return m_stopWatch.GetDescription(); }
    double GetTotalElapsedSeconds() { return m_cummulative.ToSeconds(); }
    size_t GetHitCount() { return m_count; }
    Utf8CP GetLogMessage() { return Utf8PrintfString("%s: %.2f - %d hits", GetDescription(), GetTotalElapsedSeconds(), GetHitCount()).c_str(); }
}; // StopWatchCummulative

struct StopWatchOnStack
{
private:
    StopWatchCummulative* m_stopWatch;

public:
    StopWatchOnStack(StopWatchCummulative& stopWatch) : m_stopWatch(&stopWatch) { m_stopWatch->Start(); }
    ~StopWatchOnStack() { m_stopWatch->Stop(); }
}; // StopWatchOnStack

StopWatchCummulative* s_preConvertStopWatch = new StopWatchCummulative("_PreConvertElement");
StopWatchCummulative* s_determineElemParamsStopWatch = new StopWatchCummulative("_DetermineElementParams");
StopWatchCummulative* s_processResultsStopWatch = new StopWatchCummulative("_ProcessResults");
StopWatchCummulative* s_convertProfilesStopWatch = new StopWatchCummulative("ConvertProfiles");
StopWatchCummulative* s_assignAlignmentAspectStopWatch = new StopWatchCummulative("AssignAlignmentAspect");
StopWatchCummulative* s_assignCorridorSurfaceAspectStopWatch = new StopWatchCummulative("AssignCorridorSurfaceAspect");
StopWatchCummulative* s_assignLinear3dAspectStopWatch = new StopWatchCummulative("AssignLinear3dAspect");
StopWatchCummulative* s_assignTemplateDropAspectStopWatch = new StopWatchCummulative("AssignTemplateDropAspect");
StopWatchCummulative* s_assignSuperelevationAspectStopWatch = new StopWatchCummulative("AssignSuperelevationAspect");
StopWatchCummulative* s_assignCorridorAspectStopWatch = new StopWatchCummulative("AssignCorridorAspect");

struct ORDDynamicSchemaGenerator
{
private:
    Dgn::DgnDbPtr m_dgnDbPtr;
    ECN::ECSchemaPtr m_dynamicSchema;
    ECN::ECSchemaPtr m_qtoSchema;
    ECN::ECClassCP m_graphicalElement3dClassCP;

public:
    static Utf8String FeatureNameToClassName(Utf8StringCR featureName);
    static Utf8String GetTargetSchemaName() { return "CivilDesignerProductsDynamic"; }
    static Utf8String GetQTOVolumeClassName() { return "QTO_VolumeAspect"; }
    static Utf8String GetQTOSurfaceAreaClassName() { return "QTO_SurfaceAreaAspect"; }
    static Utf8String GetQTOMaterialClassName() { return "QTO_MaterialAspect"; }
    static Utf8String GetQTOSideAreasClassName() { return "QTO_SideAreasAspect"; }

private:
    void Initialize();
    BentleyStatus SetupSchema(BeFileNameCR assetsDir);
    BentleyStatus AddQuantityTakeOffAspectClasses();
    void AddClass(Utf8StringCR name);
    void AddClasses(ConsensusConnectionR cifConn);
    bool RequireSchemaUpdate();
    BentleyStatus Finish();

public:
    BentleyStatus Generate(Dgn::DgnDbR dgnDb, BeFileNameCR assetsDir, ConsensusConnectionR cifConn);
}; // ORDDynamicSchemaGenerator

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ORDDynamicSchemaGenerator::FeatureNameToClassName(Utf8StringCR featureName)
    {
    return ECN::ECNameValidation::EncodeToValidName(featureName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDDynamicSchemaGenerator::Initialize()
    {
    m_graphicalElement3dClassCP = m_dgnDbPtr->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_GraphicalElement3d);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDDynamicSchemaGenerator::SetupSchema(BeFileNameCR assetsDir)
    {
    if (ECN::ECObjectsStatus::Success != ECN::ECSchema::CreateSchema(m_dynamicSchema, GetTargetSchemaName(), "orddgndyn", 1, 0, 0))
        {
        BeAssert(false);
        ORDBRIDGE_LOGE("Error creating dynamic schema.");
        return BentleyStatus::ERROR;
        }

    ECN::ECSchemaPtr coreSchema = ECN::CoreCustomAttributeHelper::GetSchema();
    if (coreSchema.IsValid())
        {
        ECN::ECObjectsStatus stat = m_dynamicSchema->AddReferencedSchema(*coreSchema);
        if (ECN::ECObjectsStatus::Success != stat && ECN::ECObjectsStatus::NamedItemAlreadyExists != stat)
            {
            ORDBRIDGE_LOGE("Error adding a reference to the core custom attributes schema.");
            }
        else
            {
            ECN::IECInstancePtr dynamicInstance = ECN::CoreCustomAttributeHelper::CreateCustomAttributeInstance("DynamicSchema");
            if (dynamicInstance.IsValid())
                m_dynamicSchema->SetCustomAttribute(*dynamicInstance);
            }
        }

    ECN::ECSchemaCP graphicalElement3dBaseClass = m_dgnDbPtr->Schemas().GetSchema(BIS_ECSCHEMA_NAME);
    ECN::ECSchemaP nonConstBase = const_cast<ECN::ECSchemaP>(graphicalElement3dBaseClass);
    m_dynamicSchema->AddReferencedSchema(*nonConstBase);

    auto schemaReadContextPtr = ECN::ECSchemaReadContext::CreateContext(false, true);

    BeFileName ecdbDir = assetsDir;
    ecdbDir.AppendToPath(L"ECSchemas");
    ecdbDir.AppendToPath(L"ECDb");

    BeFileName dgnDir = assetsDir;
    dgnDir.AppendToPath(L"ECSchemas");
    dgnDir.AppendToPath(L"Dgn");

    BeFileName domainDir = assetsDir;
    domainDir.AppendToPath(L"ECSchemas");
    domainDir.AppendToPath(L"Domain");

    schemaReadContextPtr->AddSchemaPath(ecdbDir);
    schemaReadContextPtr->AddSchemaPath(dgnDir);
    schemaReadContextPtr->AddSchemaPath(domainDir);
    schemaReadContextPtr->SetSkipValidation(true);

    ECN::SchemaKey qtoSchemaKey(QuantityTakeoffsSchemaName, 1, 0);
    m_qtoSchema = schemaReadContextPtr->LocateSchema(qtoSchemaKey, ECN::SchemaMatchType::Latest);
    if (m_qtoSchema.IsNull())
        {
        ORDBRIDGE_LOGE("Failed to locate QTO schema.");
        return BentleyStatus::ERROR;
        }

    m_dynamicSchema->AddReferencedSchema(*m_qtoSchema);

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDDynamicSchemaGenerator::AddQuantityTakeOffAspectClasses()
    {
    auto hiddenClassCACP = ECN::CoreCustomAttributeHelper::GetClass("HiddenClass");

    ECN::IECInstancePtr hiddenClassInstance;
    ECN::StandaloneECEnablerPtr hiddenClassEnabler;
    if (hiddenClassCACP)
        {
        hiddenClassEnabler = hiddenClassCACP->GetDefaultStandaloneEnabler();

        if (hiddenClassEnabler.IsValid())
            {
            hiddenClassInstance = hiddenClassEnabler->CreateInstance();
            hiddenClassInstance->SetValue("Show", ECN::ECValue(false));
            }
        }

    auto hiddenPropertyCACP = ECN::CoreCustomAttributeHelper::GetClass("HiddenProperty");

    ECN::IECInstancePtr hiddenPropertyInstance;
    ECN::StandaloneECEnablerPtr hiddenPropertyEnabler;
    if (hiddenPropertyCACP)
        {
        hiddenPropertyEnabler = hiddenPropertyCACP->GetDefaultStandaloneEnabler();

        if (hiddenPropertyEnabler.IsValid())
            {
            hiddenPropertyInstance = hiddenPropertyEnabler->CreateInstance();
            hiddenPropertyInstance->SetValue("Show", ECN::ECValue(false));
            }
        }

    // Create MaterialAspect subclass and hide it
    ECN::ECEntityClassP materialAspectClassP;
    if (ECN::ECObjectsStatus::Success != m_dynamicSchema->CreateEntityClass(materialAspectClassP, GetQTOMaterialClassName()))
        return BentleyStatus::ERROR;

    auto baseMaterialAspectClassCP = m_qtoSchema->GetClassCP("MaterialAspect");
    materialAspectClassP->AddBaseClass(*baseMaterialAspectClassCP);

    if (hiddenClassInstance.IsValid())
        materialAspectClassP->SetCustomAttribute(*hiddenClassInstance);

    ECN::PrimitiveECPropertyP ecPropertyP;
    if (ECN::ECObjectsStatus::Success != materialAspectClassP->CreatePrimitiveProperty(ecPropertyP, "Material", ECN::PrimitiveType::PRIMITIVETYPE_String))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    if (ECN::ECObjectsStatus::Success != materialAspectClassP->CreatePrimitiveProperty(ecPropertyP, "MaterialDensity", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    if (ECN::ECObjectsStatus::Success != materialAspectClassP->CreatePrimitiveProperty(ecPropertyP, "Weight", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    // Create VolumeAspect subclass and hide it
    ECN::ECEntityClassP volumeAspectClassP;
    if (ECN::ECObjectsStatus::Success != m_dynamicSchema->CreateEntityClass(volumeAspectClassP, GetQTOVolumeClassName()))
        return BentleyStatus::ERROR;

    auto baseVolumeAspectClassCP = m_qtoSchema->GetClassCP("VolumeAspect");
    volumeAspectClassP->AddBaseClass(*baseVolumeAspectClassCP);

    if (hiddenClassInstance.IsValid())
        volumeAspectClassP->SetCustomAttribute(*hiddenClassInstance);

    if (ECN::ECObjectsStatus::Success != volumeAspectClassP->CreatePrimitiveProperty(ecPropertyP, "GrossVolume", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    if (ECN::ECObjectsStatus::Success != volumeAspectClassP->CreatePrimitiveProperty(ecPropertyP, "NetVolume", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    // Create SurfaceAreaAspect subclass and hide it
    ECN::ECEntityClassP surfaceAreaAspectClassP;
    if (ECN::ECObjectsStatus::Success != m_dynamicSchema->CreateEntityClass(surfaceAreaAspectClassP, GetQTOSurfaceAreaClassName()))
        return BentleyStatus::ERROR;

    auto baseSurfaceAreaAspectClassCP = m_qtoSchema->GetClassCP("SurfaceAreaAspect");
    surfaceAreaAspectClassP->AddBaseClass(*baseSurfaceAreaAspectClassCP);

    if (hiddenClassInstance.IsValid())
        surfaceAreaAspectClassP->SetCustomAttribute(*hiddenClassInstance);

    if (ECN::ECObjectsStatus::Success != surfaceAreaAspectClassP->CreatePrimitiveProperty(ecPropertyP, "GrossSurfaceArea", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    if (ECN::ECObjectsStatus::Success != surfaceAreaAspectClassP->CreatePrimitiveProperty(ecPropertyP, "NetSurfaceArea", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    // Create SideAreasAspect subclass and hide it
    ECN::ECEntityClassP sideAreasAspectClassP;
    if (ECN::ECObjectsStatus::Success != m_dynamicSchema->CreateEntityClass(sideAreasAspectClassP, GetQTOSideAreasClassName()))
        return BentleyStatus::ERROR;

    auto baseSideAreasAspectClassCP = m_qtoSchema->GetClassCP("SideAreasAspect");
    sideAreasAspectClassP->AddBaseClass(*baseSideAreasAspectClassCP);

    if (hiddenClassInstance.IsValid())
        sideAreasAspectClassP->SetCustomAttribute(*hiddenClassInstance);

    if (ECN::ECObjectsStatus::Success != sideAreasAspectClassP->CreatePrimitiveProperty(ecPropertyP, "BottomGrossArea", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    if (ECN::ECObjectsStatus::Success != sideAreasAspectClassP->CreatePrimitiveProperty(ecPropertyP, "BottomNetArea", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    if (ECN::ECObjectsStatus::Success != sideAreasAspectClassP->CreatePrimitiveProperty(ecPropertyP, "LeftSideGrossArea", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    if (ECN::ECObjectsStatus::Success != sideAreasAspectClassP->CreatePrimitiveProperty(ecPropertyP, "LeftSideNetArea", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    if (ECN::ECObjectsStatus::Success != sideAreasAspectClassP->CreatePrimitiveProperty(ecPropertyP, "RightSideGrossArea", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    if (ECN::ECObjectsStatus::Success != sideAreasAspectClassP->CreatePrimitiveProperty(ecPropertyP, "RightSideNetArea", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    if (ECN::ECObjectsStatus::Success != sideAreasAspectClassP->CreatePrimitiveProperty(ecPropertyP, "TopGrossArea", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    if (ECN::ECObjectsStatus::Success != sideAreasAspectClassP->CreatePrimitiveProperty(ecPropertyP, "TopNetArea", ECN::PrimitiveType::PRIMITIVETYPE_Double))
        return BentleyStatus::ERROR;

    if (hiddenPropertyInstance.IsValid())
        ecPropertyP->SetCustomAttribute(*hiddenPropertyInstance);

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDDynamicSchemaGenerator::AddClass(Utf8StringCR name)
    {
    Utf8String className = FeatureNameToClassName(name);

    if (m_dynamicSchema->GetClassCP(className.c_str()))
        return;

    ECN::ECEntityClassP newClassP;
    if (ECN::ECObjectsStatus::Success != m_dynamicSchema->CreateEntityClass(newClassP, className))
        ORDBRIDGE_LOG.warning(Utf8PrintfString("Error creating dynamic class for FeatureDef '%s' as '%s'.", name.c_str(), className.c_str()).c_str());

    newClassP->AddBaseClass(*m_graphicalElement3dClassCP);
    newClassP->SetDisplayLabel(name);
    newClassP->SetClassModifier(ECN::ECClassModifier::Sealed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDDynamicSchemaGenerator::AddClasses(ConsensusConnectionR cifConn)
    {
    auto cifModelPtr = ConsensusModel::Create(cifConn);
    if (cifModelPtr.IsNull())
        return;

    auto geomModelsPtr = cifModelPtr->GetActiveGeometricModels();
    while (geomModelsPtr.IsValid() && geomModelsPtr->MoveNext())
        {
        auto featureDefsPtr = geomModelsPtr->GetCurrent()->GetFeatureDefintions();
        while (featureDefsPtr.IsValid() && featureDefsPtr->MoveNext())
            {
            auto featureDefPtr = featureDefsPtr->GetCurrent();
            if (featureDefPtr.IsNull())
                continue;

            auto featureDefName = featureDefPtr->GetName();
            if (WString::IsNullOrEmpty(featureDefName.c_str()))
                continue;

            AddClass(Utf8String(featureDefName.c_str()));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ORDDynamicSchemaGenerator::RequireSchemaUpdate()
    {
    auto existingSchemaCP = m_dgnDbPtr->Schemas().GetSchema(GetTargetSchemaName());
    if (!existingSchemaCP)
        return true;

    return DynamicSchemaComparer::RequireSchemaUpdate(*m_dynamicSchema, *existingSchemaCP, *m_graphicalElement3dClassCP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDDynamicSchemaGenerator::Finish()
    {
    if (!RequireSchemaUpdate())
        return BentleyStatus::SUCCESS;

    bvector<ECN::ECSchemaCP> schemas;
    schemas.push_back(m_dynamicSchema.get());

    auto status = m_dgnDbPtr->ImportSchemas(schemas);
    if (status != Dgn::SchemaStatus::Success)
        {
        ORDBRIDGE_LOGE("Failed to import dynamic schemas.");
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDDynamicSchemaGenerator::Generate(DgnDbR dgnDb, BeFileNameCR assetsDir, ConsensusConnectionR cifConn)
    {
    m_dgnDbPtr = &dgnDb;

    Initialize();
    BentleyStatus status = SetupSchema(assetsDir);
    if (status != BentleyStatus::SUCCESS)
        return status;

    if (BentleyStatus::SUCCESS != (status = AddQuantityTakeOffAspectClasses()))
        {
        ORDBRIDGE_LOGE("Failed to add QTO classes.");
        return status;
        }

    AddClasses(cifConn);

    return Finish();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverterExtension::Register(ORDConverterExtension& ext)
    {
    ORDConverterExtensionRegistry::s_extensions.push_back(&ext);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverterExtension::UnRegister(ORDConverterExtension& ext)
    {
    auto iter = std::find(ORDConverterExtensionRegistry::s_extensions.begin(), ORDConverterExtensionRegistry::s_extensions.end(), &ext);
    if (iter != ORDConverterExtensionRegistry::s_extensions.end())
        ORDConverterExtensionRegistry::s_extensions.erase(iter);
    }

void ORDConverterExtension::UnRegisterAll()
    {
    std::for_each(ORDConverterExtensionRegistry::s_extensions.begin(), ORDConverterExtensionRegistry::s_extensions.end(),
        [](ORDConverterExtension* pExtension) { ORDConverterExtension::UnRegister(*pExtension); });
    }


struct ConsensusSourceItem : iModelBridgeSyncInfoFile::ISourceItem
    {
    protected:
        ConsensusEntityCP m_entity;
        SHA1 m_sha1;

    protected:
        ConsensusSourceItem(ConsensusEntityCR entity) : m_entity(&entity) {}

    public:
        Utf8String _GetId() override { return Utf8String(m_entity->GetSyncId().c_str()); }
        double _GetLastModifiedTime() override { return m_entity->GetElementHandle()->GetElementRef()->GetLastModified(); }
    }; // ConsensusSourceItem

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
struct ORDConverterUtils
    {
    public:
        static BeSQLite::BeGuid CifSyncIdToBeGuid(Bentley::WStringCR cifSyncId);
        static bool AssignFederationGuid(DgnElementR bimElement, Bentley::WStringCR cifSyncId);
    }; // ORDConverterUtils

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Diego.Diaz                      09/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::BeGuid ORDConverterUtils::CifSyncIdToBeGuid(Bentley::WStringCR cifSyncId)
    {
    BeSQLite::BeGuid federationGuid;

    if (!WString::IsNullOrEmpty(cifSyncId.c_str()))
        {
        Utf8String syncId;
        if (cifSyncId.StartsWith(L"{"))
            syncId = Utf8String(cifSyncId.substr(1, cifSyncId.size() - 2).c_str());
        else
            syncId = Utf8String(cifSyncId.c_str());

        federationGuid.FromString(syncId.c_str());
        }

    return federationGuid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ORDConverterUtils::AssignFederationGuid(DgnElementR bimElement, Bentley::WStringCR cifSyncId)
    {
    BeSQLite::BeGuid federationGuid = CifSyncIdToBeGuid(cifSyncId);

    if (federationGuid.IsValid())
        {
        bimElement.SetFederationGuid(federationGuid);
        return true;
        }

    return false;
    }

DEFINE_POINTER_SUFFIX_TYPEDEFS(ORDAlignmentsConverter)
DEFINE_REF_COUNTED_PTR(ORDAlignmentsConverter)

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ORDAlignmentsConverter : RefCountedBase
    {
    struct CifAlignmentSourceItem : ConsensusSourceItem
        {
        DEFINE_T_SUPER(ConsensusSourceItem)

        public:
            CifAlignmentSourceItem(AlignmentCR alignment) : T_Super(alignment) {}

            static Utf8CP Kind() { return "Alignment"; }

        protected:
            Utf8String _GetHash() override
                {
                auto alignmentCP = dynamic_cast<AlignmentCP>(m_entity);
                auto linearGeomPtr = alignmentCP->GetLinearGeometry();

                Bentley::CurveVectorPtr horizCurveVectorPtr;
                if (linearGeomPtr.IsValid())
                    linearGeomPtr->GetCurveVector(horizCurveVectorPtr);

                Bentley::bvector<Byte> buffer;
                Bentley::BentleyGeometryFlatBuffer::GeometryToBytes(*horizCurveVectorPtr, buffer);

                if (buffer.empty())
                    return nullptr;

                m_sha1.Add(buffer.begin(), buffer.size());
                return m_sha1.GetHashString();
                }
        }; // CifAlignmentSourceItem

    struct CifProfileSourceItem : ConsensusSourceItem
        {
        DEFINE_T_SUPER(ConsensusSourceItem)

        public:
            CifProfileSourceItem(ProfileCR profile) : T_Super(profile) {}

            static Utf8CP Kind() { return "VerticalAlignment"; }

        protected:
            Utf8String _GetHash() override
                {
                auto profileCP = dynamic_cast<ProfileCP>(m_entity);

                Bentley::CurveVectorPtr curveVectorPtr;
                if (SUCCESS != profileCP->GetVLinearGeometry()->GetCurveVector(curveVectorPtr))
                    return nullptr;

                Bentley::bvector<Byte> buffer;
                Bentley::BentleyGeometryFlatBuffer::GeometryToBytes(*curveVectorPtr, buffer);

                if (buffer.empty())
                    return nullptr;

                m_sha1.Add(buffer.begin(), buffer.size());
                return m_sha1.GetHashString();
                }
        }; // CifProfileSourceItem

    private:
        Dgn::SpatialLocationModelPtr m_bimDesignAlignmentModelPtr;
        Dgn::PhysicalModelPtr m_bimNetworkModelPtr;
        ORDConverter& m_ordConverter;

        SpatialModelP Get3DLinearsAlignmentModel(AlignmentCR alignment, ORDConverter::Params& params) const;

    private:
        ORDAlignmentsConverter(ORDConverter& converter);

        void AssignGeomStream(ConsensusItemCR consensusItem, Dgn::GeometricElement3dR targetElm);
        void AssociateRepresentElements(AlignmentCR cifAlignment, AlignmentBim::AlignmentCR bimAlignment);
        BentleyStatus Marshal(CurveVectorPtr& bimCurveVector, Bentley::CurveVectorCR v8CurveVector);
        BentleyStatus MarshalVertical(CurveVectorPtr& bimCurveVector, Bentley::CurveVectorCR v8CurveVector);
        BentleyStatus CreateNewBimVerticalAlignment(ProfileCR, AlignmentBim::AlignmentCR alignment,
                                                    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change,
                                                    AlignmentBim::VerticalAlignmentCPtr& verticalAlignment);
        BentleyStatus CreateNewBimAlignment(AlignmentCR cifAlignment,
                                            ORDConverter::Params& params, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change,
                                            AlignmentBim::AlignmentCPtr& bimAlignment, bool isKnownToBeDesignedAlignment);
        BentleyStatus UpdateBimAlignment(AlignmentCR cifAlignment,
                                         iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change);
        BentleyStatus UpdateBimVerticalAlignment(ProfileCR,
                                                 iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change);
        BentleyStatus ConvertProfiles(AlignmentCR cifAlignment, AlignmentBim::AlignmentCR alignment, ORDConverter::Params& params);

    public:
    static bool IsDesignAlignment(Utf8StringCR featureDefName);
    static bool IsDesignAlignment(AlignmentCR alignment);

        static ORDAlignmentsConverterPtr Create(ORDConverter& converter)
            {
            return new ORDAlignmentsConverter(converter);
            }

        Dgn::SpatialLocationModelR GetDesignAlignmentModel() const { return *m_bimDesignAlignmentModelPtr; }
        Dgn::PhysicalModelR GetRoadNetworkModel() const { return *m_bimNetworkModelPtr; }
        DgnElementId ConvertAlignment(AlignmentCR cifAlignment, ORDConverter::Params& params, bool isKnownToBeDesignedAlignment);
    }; // ORDAlignmentsConverter

DEFINE_POINTER_SUFFIX_TYPEDEFS(ORDCorridorsConverter)
DEFINE_REF_COUNTED_PTR(ORDCorridorsConverter)

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ORDCorridorsConverter : RefCountedBase
    {
    struct CifCorridorSourceItem : ConsensusSourceItem
        {
        DEFINE_T_SUPER(ConsensusSourceItem)

        public:
            CifCorridorSourceItem(CorridorCR corridor) : T_Super(corridor) {}

            static Utf8CP Kind() { return "Corridor"; }

        protected:
            Utf8String _GetHash() override
                {
                auto corridorCP = dynamic_cast<CorridorCP>(m_entity);
                auto surfacesPtr = corridorCP->GetCorridorSurfaces();

                Bentley::bvector<Byte> buffer;
                while (surfacesPtr.IsValid() && surfacesPtr->MoveNext())
                    {
                    auto surfacePtr = surfacesPtr->GetCurrent();
                    if (surfacePtr.IsValid())
                        {
                        auto meshPtr = surfacePtr->GetMesh();
                        if (meshPtr.IsValid())
                            {
                            buffer.clear();
                            Bentley::BentleyGeometryFlatBuffer::GeometryToBytes(*meshPtr, buffer);
                            m_sha1.Add(buffer.begin(), buffer.size());
                            }
                        }
                    }

                return m_sha1.GetHashString();
                }
        }; // CifCorridorSourceItem

    private:
        Transform m_unitsScaleTransform;
        ORDConverter& m_converter;
        ECN::ECUnitCP m_kphUnitCP, m_mphUnitCP, m_msecUnitCP;

        enum class NetworkType { Road, Rail, Undetermined };

    private:
        ORDCorridorsConverter(ORDConverter& converter, TransformCR unitsScaleTransform);

        BentleyStatus Marshal(PolyfaceHeaderPtr& bimMesh, Bentley::PolyfaceHeaderCR v8Mesh);
        BentleyStatus CreateNewCorridor(CorridorCR cifCorridor,
                                        ORDConverter::Params& params, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change,
                                        RoadRailBim::CorridorCPtr& bimCorridorCPtr);
        BentleyStatus UpdateCorridor(CorridorCR cifCorridor,
                                     iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change);
        BentleyStatus CreateCorridorComponents(CorridorCR cifCorridor, RoadRailBim::CorridorPortionElementCR corridorPortion);
        BentleyStatus UpdateCorridorComponents(CorridorCR cifCorridor, RoadRailBim::CorridorPortionElementCR corridorPortion);
        RoadRailBim::DesignSpeedDefinitionCPtr ORDCorridorsConverter::GetOrInsertDesignSpeedDefinition(
            Dgn::DefinitionModelCR standardsModel, double speedInMPerSec, RoadRailBim::DesignSpeedDefinition::UnitSystem unitSystem);

    public:
        static ORDCorridorsConverterPtr Create(ORDConverter& converter, TransformCR unitsScaleTransform)
            {
            return new ORDCorridorsConverter(converter, unitsScaleTransform);
            }

        DgnElementId ConvertCorridor(CorridorCR cifCorridor, ORDConverter::Params& params, DgnCategoryId targetCategoryId = DgnCategoryId());

        void ConvertSpeedTable(SpeedTableCP speedTable, int32_t cifDesignSpeedIdx, RoadRailBim::PathwayElementCR pathway, Dgn::DefinitionModelCR standardsModel,
                               RoadRailBim::DesignSpeedDefinition::UnitSystem unitSystem, ORDConverter::Params const& params);
    }; // ORDCorridorsConverter

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Diego.Diaz                      01/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
ORDAlignmentsConverter::ORDAlignmentsConverter(ORDConverter& converter) : m_ordConverter(converter)
    {
    m_bimNetworkModelPtr = converter.GetRoadNetwork()->GetNetworkModel();
    auto designAlignmentsCPtr = AlignmentBim::DesignAlignments::Query(*m_bimNetworkModelPtr, DefaultDesignAlignmentsName);
    m_bimDesignAlignmentModelPtr = designAlignmentsCPtr->GetAlignmentModel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ORDAlignmentsConverter::IsDesignAlignment(Utf8StringCR featureDefName)
    {
    auto iterSlash = featureDefName.find('\\');
    if (iterSlash != featureDefName.npos)
        {
        auto featureDefType = featureDefName.substr(0, iterSlash);

        // TODO: this technique won't work on localized versions of ORD.
        // CIF SDK to expose an API for this check.
        return (0 == strcmp(featureDefType.c_str(), "Alignment"));
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ORDAlignmentsConverter::IsDesignAlignment(AlignmentCR alignment)
    {
    auto linear3dPtr = alignment.GetActiveLinearEntity3d();
    if (linear3dPtr.IsValid())
        {
        if (linear3dPtr->GetCorridor().IsValid())
            return false;
        }

    auto featureDefPtr = alignment.GetFeatureDefinition();
    if (featureDefPtr.IsNull())
        {
        Cif::FeaturizedConsensusItemPtr representationOf = alignment.GetRepresentationOf();
        if (representationOf.IsValid())
            featureDefPtr = representationOf->GetFeatureDefinition();
        }

    if (featureDefPtr.IsValid())
        {
        auto featureDefName = featureDefPtr->GetName();
        return IsDesignAlignment(Utf8String(featureDefName.c_str()));
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialModelP ORDAlignmentsConverter::Get3DLinearsAlignmentModel(AlignmentCR alignment, ORDConverter::Params& params) const
    {
    auto linear3dPtr = alignment.GetActiveLinearEntity3d();

    if (linear3dPtr.IsValid())
        {
        auto corridorPtr = linear3dPtr->GetCorridor();
        if (corridorPtr.IsValid())
            {
            ORDCorridorsConverter::CifCorridorSourceItem corridorItem(*corridorPtr);

            iModelExternalSourceAspect::ElementAndAspectId elementAndAspectId =
                iModelExternalSourceAspect::FindElementBySourceId(m_ordConverter.GetDgnDb(), DgnElementId(params.fileScopeId), corridorItem.Kind(), corridorItem._GetId());
            if (elementAndAspectId.elementId.IsValid())
                {
                auto bimCorridorCPtr = RoadRailBim::Corridor::Get(m_ordConverter.GetDgnDb(), elementAndAspectId.elementId);
                auto transportationNetworkCPtr = RoadRailBim::TransportationSystem::Query(*bimCorridorCPtr, DefaultTransportationSystemName);
                return transportationNetworkCPtr->GetTransportationSystemModel().get();
                }
            }
        }

    return m_bimNetworkModelPtr.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDAlignmentsConverter::Marshal(CurveVectorPtr& bimCurveVectorPtr, Bentley::CurveVectorCR v8CurveVector)
    {
    // v8CurveVector assumed to be already in meters - as returned by the CIF SDK
    DgnDbSync::DgnV8::Converter::ConvertCurveVector(bimCurveVectorPtr, v8CurveVector, nullptr);

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDAlignmentsConverter::MarshalVertical(CurveVectorPtr& bimCurveVectorXZPtr, Bentley::CurveVectorCR v8CurveVector)
    {
    // v8CurveVector assumed to be already in meters - as returned by the CIF SDK
    DgnDbSync::DgnV8::Converter::ConvertCurveVector(bimCurveVectorXZPtr, v8CurveVector, nullptr);

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDAlignmentsConverter::AssociateRepresentElements(AlignmentCR cifAlignment, AlignmentBim::AlignmentCR bimAlignment)
    {
    auto v8Iter = m_ordConverter.m_v8ToBimElmMap.find(cifAlignment.GetElementHandle()->GetElementRef());
    if (v8Iter != m_ordConverter.m_v8ToBimElmMap.end())
        {
        auto bimAlgElmPtr = v8Iter->second;
        if (auto bimGeomSourceCP = bimAlgElmPtr->ToGeometrySource())
            {
            if (!bimAlignment.QueryIsRepresentedBy(*bimGeomSourceCP))
                {
                RoadRailAlignment::Alignment::AddRepresentedBy(bimAlignment, *bimGeomSourceCP);

                if (!Utf8String::IsNullOrEmpty(bimAlignment.GetUserLabel()))
                    {
                    auto ptr = bimAlignment.GetDgnDb().Elements().GetForEdit<DgnElement>(bimAlgElmPtr->GetElementId());
                    ptr->SetUserLabel(bimAlignment.GetUserLabel());
                    ptr->Update();
                    }
                }
            }
        }

    auto linear3dPtr = cifAlignment.GetActiveLinearEntity3d();
    if (linear3dPtr.IsNull())
        return;

    v8Iter = m_ordConverter.m_v8ToBimElmMap.find(linear3dPtr->GetElementHandle()->GetElementRef());
    if (v8Iter != m_ordConverter.m_v8ToBimElmMap.end())
        {
        auto bimLin3dElmPtr = v8Iter->second;
        if (auto bimGeomSourceCP = bimLin3dElmPtr->ToGeometrySource())
            {
            if (!bimAlignment.QueryIsRepresentedBy(*bimGeomSourceCP))
                RoadRailAlignment::Alignment::AddRepresentedBy(bimAlignment, *bimGeomSourceCP);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnSubCategoryId getOrInsertSubCategory(Dgn::DgnDbR dgnDb, bmap<DgnSubCategoryId, DgnSubCategoryId>& subCategoryMap,
                                             Dgn::DgnCategoryId const& sourceCategoryId, Dgn::DgnSubCategoryId const& sourceSubCatId,
                                             Dgn::DgnCategoryId const& targetCategoryId)
    {
    Dgn::DgnSubCategoryId targetSubCatId;
    auto subCatIter = subCategoryMap.find(sourceSubCatId);
    if (subCategoryMap.end() == subCatIter)
        {
        auto sourceCategoryCPtr = DgnCategory::Get(dgnDb, sourceCategoryId);
        auto targetCategoryCPtr = DgnCategory::Get(dgnDb, targetCategoryId);
        targetSubCatId = DgnSubCategory::QuerySubCategoryId(dgnDb,
                                                            DgnSubCategory::CreateCode(*targetCategoryCPtr, sourceCategoryCPtr->GetCategoryName()));
        if (!targetSubCatId.IsValid())
            {
            auto sourceSubCatCPtr = DgnSubCategory::Get(dgnDb, sourceSubCatId);

            DgnSubCategory newTargetSubCat(DgnSubCategory::CreateParams(dgnDb,
                                                                        targetCategoryId, sourceCategoryCPtr->GetCategoryName(), sourceSubCatCPtr->GetAppearance()));
            newTargetSubCat.Insert();
            targetSubCatId = newTargetSubCat.GetSubCategoryId();
            }

        BeAssert(targetSubCatId.IsValid());
        subCategoryMap.insert({sourceSubCatId, targetSubCatId});
        }
    else
        targetSubCatId = subCatIter->second;

    return targetSubCatId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDAlignmentsConverter::AssignGeomStream(ConsensusItemCR consensusItem, Dgn::GeometricElement3dR targetElm)
    {
    auto v8Iter = m_ordConverter.m_v8ToBimElmMap.find(consensusItem.GetElementHandle()->GetElementRef());

    DgnElementPtr bimConvertedElementPtr;
    if (v8Iter != m_ordConverter.m_v8ToBimElmMap.end())
        bimConvertedElementPtr = v8Iter->second;
    else
        return;

    auto bimTargetCategoryId = targetElm.GetCategoryId();
    bmap<DgnSubCategoryId, DgnSubCategoryId> subCategoryMap;
    auto bimAlignmentCatCPtr = SpatialCategory::Get(m_ordConverter.GetDgnDb(), bimTargetCategoryId);
    auto bimGeomSourceCP = bimConvertedElementPtr->ToGeometrySource();
    if (!bimGeomSourceCP)
        return;

    GeometryCollection geomColl(*bimGeomSourceCP);
    if (geomColl.begin() == geomColl.end())
        return;

    DPoint3d origin;
    YawPitchRollAngles angles;
    auto bimConvertedGeomSourceCP = bimConvertedElementPtr->ToGeometrySource();
    if (bimConvertedGeomSourceCP->Is2d())
        {
        auto& placement2d = bimConvertedGeomSourceCP->GetAsGeometrySource2d()->GetPlacement();
        auto origin2d = placement2d.GetOrigin();
        origin = {origin2d.x, origin2d.y, 0};
        angles.SetYaw(placement2d.GetAngle());
        }
    else
        {
        auto& placement3d = bimConvertedGeomSourceCP->GetAsGeometrySource3d()->GetPlacement();
        origin = placement3d.GetOrigin();
        angles = placement3d.GetAngles();
        }

    auto geomBuilderPtr = GeometryBuilder::Create(*targetElm.GetModel(), bimTargetCategoryId, origin, angles);
    for (auto& iter : geomColl)
        {
        auto geomParams = iter.GetGeometryParams(); // Intentionally copied

        auto subCatId = geomParams.GetSubCategoryId();
        DgnSubCategoryId targetSubCatId = getOrInsertSubCategory(m_ordConverter.GetDgnDb(), subCategoryMap,
                                                                 geomParams.GetCategoryId(), subCatId, bimAlignmentCatCPtr->GetCategoryId());
        if (targetSubCatId.IsValid())
            {
            geomParams.SetCategoryId(bimTargetCategoryId, false);
            geomParams.SetSubCategoryId(targetSubCatId, false);
            if (!geomBuilderPtr->Append(geomParams))
                BeAssert(false);
            }

        if (!geomBuilderPtr->Append(*iter.GetGeometryPtr()))
            BeAssert(false);
        }

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(targetElm))
        BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDAlignmentsConverter::CreateNewBimAlignment(AlignmentCR cifAlignment,
    ORDConverter::Params& params, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change,
    AlignmentBim::AlignmentCPtr& bimAlignment, bool isKnownToBeDesignedAlignment)
    {
    auto linearGeomPtr = cifAlignment.GetLinearGeometry();
    if (linearGeomPtr.IsNull())
        return BentleyStatus::SUCCESS;

    Bentley::CurveVectorPtr horizGeometryPtr;
    linearGeomPtr->GetCurveVector(horizGeometryPtr);
    if (horizGeometryPtr.IsNull())
        return BentleyStatus::SUCCESS;

    Utf8String cifAlignmentName(cifAlignment.GetName().c_str());
    Bentley::WString cifSyncId = cifAlignment.GetSyncId();
    auto bimGuid = ORDConverterUtils::CifSyncIdToBeGuid(cifSyncId);

    Dgn::SpatialModelPtr bimAlignmentModelPtr;
    if (isKnownToBeDesignedAlignment || IsDesignAlignment(cifAlignment))
        bimAlignmentModelPtr = &GetDesignAlignmentModel();
    else
        bimAlignmentModelPtr = Get3DLinearsAlignmentModel(cifAlignment, params);

    DgnCode bimCode;

    if (Utf8String::IsNullOrEmpty(cifAlignmentName.c_str()))
        {
        auto featureName = cifAlignment.GetFeatureName();
        auto featureDefPtr = cifAlignment.GetFeatureDefinition();
        if (featureDefPtr.IsNull())
            {
            auto representationOfPtr = cifAlignment.GetRepresentationOf();
            if (representationOfPtr.IsValid())
                {
                featureDefPtr = representationOfPtr->GetFeatureDefinition();
                featureName = representationOfPtr->GetFeatureName();
                }
            }

        if (!WString::IsNullOrEmpty(featureName.c_str()))
            cifAlignmentName = Utf8String(featureName.c_str());
        else if (featureDefPtr.IsValid() &&
            !WString::IsNullOrEmpty(featureDefPtr->GetName().c_str()))
            cifAlignmentName = Utf8String(featureDefPtr->GetName().c_str());
        else
            cifAlignmentName = Utf8String(cifSyncId.c_str());
        }

    uint32_t algSuffix = 0;
    Utf8String bimAlignmentName = cifAlignmentName;
    bimCode = AlignmentBim::RoadRailAlignmentDomain::CreateCode(*bimAlignmentModelPtr, bimAlignmentName);

    // Addressing alignments with the same name
    do
        {
        if (!bimAlignmentModelPtr->GetDgnDb().Elements().QueryElementIdByCode(bimCode).IsValid())
            break;

        Utf8String bimAlignmentNameDup = Utf8PrintfString("%s_%d", bimAlignmentName.c_str(), ++algSuffix);
        bimCode = AlignmentBim::RoadRailAlignmentDomain::CreateCode(*bimAlignmentModelPtr, bimAlignmentNameDup);
        } while (true);

        // Create Alignment
        auto bimAlignmentPtr = AlignmentBim::Alignment::Create(*bimAlignmentModelPtr);

        if (bimCode.IsValid())
            bimAlignmentPtr->SetCode(bimCode);

        Utf8String userLabel(cifAlignment.GetName().c_str());
        bimAlignmentPtr->SetUserLabel(userLabel.c_str());

        ORDConverterUtils::AssignFederationGuid(*bimAlignmentPtr->getP(), cifSyncId);

        DgnCategoryId alignmentCatId;
        auto cif3dLinearPtr = cifAlignment.GetActiveLinearEntity3d();
        if (cif3dLinearPtr.IsValid())
            AssignGeomStream(*cif3dLinearPtr, *bimAlignmentPtr->getP());

        auto cifStationingPtr = cifAlignment.GetStationing();
        if (cifStationingPtr.IsValid())
            {
            bimAlignmentPtr->SetStartStation(cifStationingPtr->GetStartStation());
            bimAlignmentPtr->SetStartValue(cifStationingPtr->GetDistanceAlong());
            }

        iModelBridgeSyncInfoFile::ConversionResults results;
        results.m_element = bimAlignmentPtr->getP();
        if (BentleyStatus::SUCCESS != params.changeDetectorP->_UpdateBimAndSyncInfo(results, change))
        {
        ORDBRIDGE_LOGE("CreateNewBimAlignment '%s' - failed update and sync.", userLabel.c_str());
            return BentleyStatus::ERROR;
        }

        CurveVectorPtr bimHorizGeometryPtr;
        if (BentleyStatus::SUCCESS != Marshal(bimHorizGeometryPtr, *horizGeometryPtr))
        {
        ORDBRIDGE_LOGE("CreateNewBimAlignment '%s' - failed to marshal curve vector.", userLabel.c_str());
            return BentleyStatus::ERROR;
        }

        // Create Horizontal Alignment
        auto bimHorizAlignmPtr = AlignmentBim::HorizontalAlignment::Create(*bimAlignmentPtr, *bimHorizGeometryPtr);

    if (bimHorizAlignmPtr.IsValid())
        {
        if (bimCode.IsValid())
            bimHorizAlignmPtr->SetCode(AlignmentBim::RoadRailAlignmentDomain::CreateCode(*bimHorizAlignmPtr->GetModel(), bimCode.GetValueUtf8()));

        AssignGeomStream(cifAlignment, *bimHorizAlignmPtr->getP());

        if (bimHorizAlignmPtr->Insert().IsNull())
            {
            ORDBRIDGE_LOGE("CreateNewBimAlignment '%s' - failed to insert horizontal alignment.", userLabel.c_str());
            return BentleyStatus::ERROR;
            }
        }

        bimAlignment = AlignmentBim::Alignment::Get(m_ordConverter.GetDgnDb(), bimAlignmentPtr->GetElementId());

        AssociateRepresentElements(cifAlignment, *bimAlignmentPtr);

        if (cifStationingPtr.IsValid())
            {
            auto stationEquationsPtr = cifStationingPtr->GetStationEquations();
            while (stationEquationsPtr.IsValid() && stationEquationsPtr->MoveNext())
                {
                auto stationEqPtr = stationEquationsPtr->GetCurrent();
                if (stationEqPtr.IsNull())
                    continue;

                auto bimStationPtr = AlignmentBim::AlignmentStation::Create(
                    AlignmentBim::AlignmentStation::CreateAtParams(*bimAlignmentPtr, stationEqPtr->GetDistanceAlong(), stationEqPtr->GetEquivalentStation()));
                if (!Bentley::WString::IsNullOrEmpty(stationEqPtr->GetName().c_str()))
                    bimStationPtr->SetUserLabel(Utf8String(stationEqPtr->GetName().c_str()).c_str());

                bimStationPtr->Insert();
                }
            }

        return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDAlignmentsConverter::CreateNewBimVerticalAlignment(ProfileCR cifProfile, AlignmentBim::AlignmentCR alignment,
                                                                    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change,
                                                                    AlignmentBim::VerticalAlignmentCPtr& verticalAlignment)
    {
    auto vLinearGeomPtr = cifProfile.GetVLinearGeometry();

    Bentley::CurveVectorPtr vertGeometryPtr;
    if (SUCCESS != vLinearGeomPtr->GetCurveVector(vertGeometryPtr))
        {
        ORDBRIDGE_LOGE("CreateNewBimVerticalAlignment '%s' - failed to get curve vector.", Utf8String(cifProfile.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    CurveVectorPtr bimVertGeometryPtr;
    if (BentleyStatus::SUCCESS != MarshalVertical(bimVertGeometryPtr, *vertGeometryPtr))
        {
        ORDBRIDGE_LOGE("CreateNewBimVerticalAlignment '%s' - failed to marshal curve vector.", Utf8String(cifProfile.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    auto verticalModelId = alignment.QueryVerticalAlignmentSubModelId();
    if (!verticalModelId.IsValid())
        {
        auto verticalModelPtr = AlignmentBim::VerticalAlignmentModel::Create(
            AlignmentBim::VerticalAlignmentModel::CreateParams(GetDesignAlignmentModel().GetDgnDb(), alignment.GetElementId()));
        if (DgnDbStatus::Success != verticalModelPtr->Insert())
            {
            ORDBRIDGE_LOGE("CreateNewBimVerticalAlignment '%s' - failed to insert vertical model.", Utf8String(cifProfile.GetName().c_str()).c_str());
            return BentleyStatus::ERROR;
            }

        verticalModelId = verticalModelPtr->GetModelId();
        }

    auto verticalAlignmPtr = AlignmentBim::VerticalAlignment::Create(alignment, *bimVertGeometryPtr);

    verticalAlignmPtr->GenerateElementGeom();

    iModelBridgeSyncInfoFile::ConversionResults verticalResults;
    verticalResults.m_element = verticalAlignmPtr->getP();
    if (BentleyStatus::SUCCESS != changeDetector._UpdateBimAndSyncInfo(verticalResults, change))
        {
        ORDBRIDGE_LOGE("CreateNewBimVerticalAlignment '%s' - failed to update and sync.", Utf8String(cifProfile.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    verticalAlignment = verticalAlignmPtr.get();

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDAlignmentsConverter::UpdateBimAlignment(AlignmentCR cifAlignment,
                                                         iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change)
    {
    auto alignmentCPtr = AlignmentBim::Alignment::Get(changeDetector.GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());
    auto horizAlignmentPtr = AlignmentBim::HorizontalAlignment::GetForEdit(changeDetector.GetDgnDb(), alignmentCPtr->GetHorizontal()->GetElementId());

    CurveVectorPtr bimHorizGeometryPtr;
    auto linearGeomPtr = cifAlignment.GetLinearGeometry();

    Bentley::CurveVectorPtr horizGeometryPtr;
    if (linearGeomPtr.IsValid())
        linearGeomPtr->GetCurveVector(horizGeometryPtr);

    if (BentleyStatus::SUCCESS != Marshal(bimHorizGeometryPtr, *horizGeometryPtr))
        {
        ORDBRIDGE_LOGE("UpdateBimAlignment '%s' - failed to marshal curve vector.", Utf8String(cifAlignment.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    horizAlignmentPtr->SetGeometry(*bimHorizGeometryPtr);
    horizAlignmentPtr->GenerateElementGeom();
    if (horizAlignmentPtr->Update().IsNull())
        {
        ORDBRIDGE_LOGE("UpdateBimAlignment '%s' - failed to update.", Utf8String(cifAlignment.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    auto alignmentPtr = AlignmentBim::Alignment::GetForEdit(alignmentCPtr->GetDgnDb(), alignmentCPtr->GetElementId());
    alignmentPtr->GenerateAprox3dGeom();
    alignmentPtr->Update();

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDAlignmentsConverter::UpdateBimVerticalAlignment(ProfileCR cifProfile,
                                                                 iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change)
    {
    auto vLinearGeomPtr = cifProfile.GetVLinearGeometry();

    Bentley::CurveVectorPtr vertGeometryPtr;
    if (SUCCESS != vLinearGeomPtr->GetCurveVector(vertGeometryPtr))
        {
        ORDBRIDGE_LOGE("UpdateBimVerticalAlignment '%s' - failed to get curve vector.", Utf8String(cifProfile.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    CurveVectorPtr bimVertGeometryPtr;
    if (BentleyStatus::SUCCESS != MarshalVertical(bimVertGeometryPtr, *vertGeometryPtr))
        {
        ORDBRIDGE_LOGE("UpdateBimVerticalAlignment '%s' - failed to marshal curve vector.", Utf8String(cifProfile.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    auto verticalAlignmentPtr = AlignmentBim::VerticalAlignment::GetForEdit(changeDetector.GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());
    verticalAlignmentPtr->SetGeometry(*bimVertGeometryPtr);
    if (verticalAlignmentPtr->Update().IsNull())
        {
        ORDBRIDGE_LOGE("UpdateBimVerticalAlignment '%s' - failed to update.", Utf8String(cifProfile.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDAlignmentsConverter::ConvertProfiles(AlignmentCR cifAlignment, AlignmentBim::AlignmentCR alignment, ORDConverter::Params& params)
    {
    StopWatchOnStack stopWatch(*s_convertProfilesStopWatch);

    m_ordConverter.GetProgressMeter().SetCurrentTaskName(
        ORDConverter::ORDBridgeProgressMessage::GetString(ORDConverter::ORDBridgeProgressMessage::TASK_PROCESS_PROFILE()).c_str());

    ModelRefPinner modelPinner;

    Bentley::WString activeProfileId;
    auto activeProfilePtr = cifAlignment.GetActiveProfile();
    if (activeProfilePtr.IsValid())
        activeProfileId = activeProfilePtr->GetSyncId();

    AlignmentBim::AlignmentPtr alignmentPtr;
    auto cifProfilesPtr = cifAlignment.GetProfiles();
    while (cifProfilesPtr.IsValid() && cifProfilesPtr->MoveNext())
        {
        auto cifProfilePtr = cifProfilesPtr->GetCurrent();
        if (!cifProfilePtr->IsFinalElement())
            continue;

        CifProfileSourceItem profileItem(*cifProfilePtr);
        auto profileChange = params.changeDetectorP->_DetectChange(params.fileScopeId, profileItem.Kind(), profileItem);

        AlignmentBim::VerticalAlignmentCPtr verticalAlignmCPtr;
        if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged == profileChange.GetChangeType())
            {
            params.changeDetectorP->_OnElementSeen(profileChange.GetSyncInfoRecord().GetDgnElementId());
            }
        else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New == profileChange.GetChangeType())
            {
            CreateNewBimVerticalAlignment(*cifProfilePtr, alignment, *params.changeDetectorP, profileChange, verticalAlignmCPtr);
            }
        else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Changed == profileChange.GetChangeType())
            {
            if (BentleyStatus::SUCCESS != UpdateBimVerticalAlignment(*cifProfilePtr, *params.changeDetectorP, profileChange))
                return BentleyStatus::ERROR;
            }

        if (!Bentley::WString::IsNullOrEmpty(activeProfileId.c_str()) && cifProfilePtr->GetSyncId().Equals(activeProfileId))
            {
            if (verticalAlignmCPtr.IsNull())
                verticalAlignmCPtr = AlignmentBim::VerticalAlignment::Get(alignment.GetDgnDb(), profileChange.GetSyncInfoRecord().GetDgnElementId());

            if (verticalAlignmCPtr.IsValid())
                {
                if (alignmentPtr.IsNull())
                    alignmentPtr = AlignmentBim::Alignment::GetForEdit(alignment.GetDgnDb(), alignment.GetElementId());

                alignmentPtr->SetMainVertical(verticalAlignmCPtr.get());
                }
            }
        }

    if (alignmentPtr.IsValid())
        alignmentPtr->Update();

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ORDAlignmentsConverter::ConvertAlignment(AlignmentCR cifAlignment, ORDConverter::Params& params, bool isKnownToBeDesignedAlignment)
    {
    CifAlignmentSourceItem sourceItem(cifAlignment);

    // only convert an Alignment if it is new or has changed in the source
    auto change = params.changeDetectorP->_DetectChange(params.fileScopeId, sourceItem.Kind(), sourceItem, nullptr, params.spatialDataTransformHasChanged);

    AlignmentBim::AlignmentCPtr alignmentCPtr;
    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New == change.GetChangeType())
        {
        if (BentleyStatus::SUCCESS != CreateNewBimAlignment(cifAlignment, params, change, alignmentCPtr, isKnownToBeDesignedAlignment))
            return DgnElementId();
        }
    else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged == change.GetChangeType())
        {
        params.changeDetectorP->_OnElementSeen(change.GetSyncInfoRecord().GetDgnElementId());
        }
    else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Changed == change.GetChangeType())
        {
        if (BentleyStatus::SUCCESS != UpdateBimAlignment(cifAlignment, *params.changeDetectorP, change))
            return DgnElementId();
        }

    if (alignmentCPtr.IsNull())
        alignmentCPtr = AlignmentBim::Alignment::Get(params.changeDetectorP->GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());

    // Only process profiles for CIF design alignments - not 3dLinears. Bad performance issue in CIF.
    if (alignmentCPtr->GetCategoryId() == AlignmentBim::AlignmentCategory::GetAlignment(alignmentCPtr->GetDgnDb()))
        ConvertProfiles(cifAlignment, *alignmentCPtr, params);

    return alignmentCPtr->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ORDCorridorsConverter::ORDCorridorsConverter(ORDConverter& converter, TransformCR unitsScaleTransform) :
    m_converter(converter), m_unitsScaleTransform(unitsScaleTransform)
    {
    m_msecUnitCP = converter.GetDgnDb().Schemas().GetUnit("Units", "M_PER_SEC");
    m_kphUnitCP = converter.GetDgnDb().Schemas().GetUnit("Units", "KM_PER_HR");
    m_mphUnitCP = converter.GetDgnDb().Schemas().GetUnit("Units", "MPH");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::Marshal(PolyfaceHeaderPtr& bimMeshPtr, Bentley::PolyfaceHeaderCR v8Mesh)
    {
    DgnDbSync::DgnV8::Converter::ConvertPolyface(bimMeshPtr, v8Mesh);
    bimMeshPtr->Transform(m_unitsScaleTransform);

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailBim::DesignSpeedDefinitionCPtr ORDCorridorsConverter::GetOrInsertDesignSpeedDefinition(
    Dgn::DefinitionModelCR standardsModel, double speedInMPerSec, RoadRailBim::DesignSpeedDefinition::UnitSystem unitSystem)
    {
    BENTLEY_NAMESPACE_NAME::Units::Quantity speedQty(speedInMPerSec, *m_msecUnitCP);

    double speedInCodeUnits;
    if (unitSystem == RoadRailBim::DesignSpeedDefinition::UnitSystem::SI)
        speedInCodeUnits = speedQty.ConvertTo(m_kphUnitCP).GetMagnitude();
    else
        speedInCodeUnits = speedQty.ConvertTo(m_mphUnitCP).GetMagnitude();

    speedInCodeUnits = round(speedInCodeUnits);

    auto retValCPtr = RoadRailBim::DesignSpeedDefinition::QueryByCode(standardsModel, speedInCodeUnits, unitSystem);
    if (retValCPtr.IsValid())
        return retValCPtr;

    auto retValPtr = RoadRailBim::DesignSpeedDefinition::Create(standardsModel, speedInCodeUnits, unitSystem);
    return retValPtr->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDCorridorsConverter::ConvertSpeedTable(SpeedTableCP speedTable, int32_t cifDesignSpeedIdx, RoadRailBim::PathwayElementCR pathway, Dgn::DefinitionModelCR standardsModel,
                                              RoadRailBim::DesignSpeedDefinition::UnitSystem unitSystem, ORDConverter::Params const& params)
    {
    bvector<bpair<double, bpair<double, double>>> stationStartEndSpeeds;

    if (speedTable)
        {
        auto speedSectionsPtr = speedTable->GetSpeedSections();
        while (speedSectionsPtr->MoveNext())
            {
            auto speedSectionPtr = speedSectionsPtr->GetCurrent();
            double startStation = speedSectionPtr->GetStation();
            auto designSpeedsPtr = speedSectionPtr->GetDesignSpeeds();
            if (designSpeedsPtr.IsValid())
                {
                int32_t i = 0;
                while (designSpeedsPtr->MoveNext() && i++ < cifDesignSpeedIdx);

                auto designSpeedPtr = designSpeedsPtr->GetCurrent();
                auto startSpeed = designSpeedPtr->GetStart();
                auto endSpeed = designSpeedPtr->GetEnd();

                stationStartEndSpeeds.push_back({startStation, { startSpeed, endSpeed }});
                }
            }
        }

    auto alignmentCPtr = AlignmentBim::Alignment::Get(pathway.GetDgnDb(), pathway.GetMainAlignmentId());
    double lastStartStation = alignmentCPtr->GetLength();

    if (stationStartEndSpeeds.empty())
        {
        BENTLEY_NAMESPACE_NAME::Units::Quantity speedQtyKPH(100.0, *m_kphUnitCP);
        BENTLEY_NAMESPACE_NAME::Units::Quantity speedQtyMPH(60.0, *m_mphUnitCP);

        double speedQtyMPSEC = (unitSystem == RoadRailBim::DesignSpeedDefinition::UnitSystem::SI) ?
            speedQtyKPH.ConvertTo(m_msecUnitCP).GetMagnitude() : speedQtyMPH.ConvertTo(m_msecUnitCP).GetMagnitude();
        stationStartEndSpeeds.push_back({0.0, { speedQtyMPSEC, speedQtyMPSEC }});
        }

    auto designCriteriaCPtr = RoadRailBim::PathwayDesignCriteria::Query(pathway);
    if (designCriteriaCPtr.IsNull())
        designCriteriaCPtr = RoadRailBim::PathwayDesignCriteria::Insert(pathway);

    if (params.domainModelsPrivate)
        {
        if (!designCriteriaCPtr->GetDesignCriteriaModel()->IsPrivate())
            {
            designCriteriaCPtr->GetDesignCriteriaModel()->SetIsPrivate(params.domainModelsPrivate);
            designCriteriaCPtr->GetDesignCriteriaModel()->Update();
            }
        }

    auto existingSpeeds = designCriteriaCPtr->QueryOrderedDesignSpeedIds();
    if (existingSpeeds.size() == stationStartEndSpeeds.size())
        return; // TODO - compare existing vs. new speeds -> insert/update/delete accordingly

    for (auto rIter = stationStartEndSpeeds.rbegin(); rIter != stationStartEndSpeeds.rend(); rIter++)
        {
        auto& stationStartEndSpeed = *rIter;

        auto designSpeedPtr = RoadRailBim::DesignSpeed::Create(
            RoadRailBim::DesignSpeed::CreateFromToParams(*designCriteriaCPtr, *alignmentCPtr,
                                                         *GetOrInsertDesignSpeedDefinition(standardsModel, stationStartEndSpeed.second.first, unitSystem),
                                                         *GetOrInsertDesignSpeedDefinition(standardsModel, stationStartEndSpeed.second.second, unitSystem),
                                                         stationStartEndSpeed.first, lastStartStation));
        designSpeedPtr->SetAttributedElement(pathway.get());
        designSpeedPtr->Insert();

        lastStartStation = stationStartEndSpeed.first;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnElementCPtr createCorridorComponent(CorridorSurfaceCR corridorSurface,
                                            ECN::StandaloneECEnablerR instanceEnabler,
                                            RoadRailBim::CorridorPortionElementCR corridorPortion,
                                            Dgn::DgnCategoryId const& categoryId,
                                            Dgn::DgnCodeCR code,
                                            Dgn::DgnElementCR graphicalElement,
                                            bmap<DgnSubCategoryId, DgnSubCategoryId>& subCategoryMap)
    {
    auto ecInstancePtr = instanceEnabler.CreateInstance();
    ecInstancePtr->SetValue("Model", ECN::ECValue(corridorPortion.GetModelId()));
    ecInstancePtr->SetValue("Category", ECN::ECValue(categoryId));
    ecInstancePtr->SetValue("CodeSpec", ECN::ECValue(code.GetCodeSpecId()));
    ecInstancePtr->SetValue("CodeScope", ECN::ECValue(code.GetScopeElementId(corridorPortion.GetDgnDb())));

    auto corridorComponentElmPtr = corridorPortion.GetDgnDb().Elements().CreateElement(*ecInstancePtr);
    BeAssert(corridorComponentElmPtr.IsValid());

    corridorComponentElmPtr->SetParentId(corridorPortion.GetElementId(),
                                         corridorPortion.GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));

    auto featureDefPtr = corridorSurface.GetFeatureDefinition();
    if (featureDefPtr.IsNull())
        {
        Cif::FeaturizedConsensusItemPtr representationOf = corridorSurface.GetRepresentationOf();
        if (representationOf.IsValid())
            featureDefPtr = representationOf->GetFeatureDefinition();
        }

    if (featureDefPtr.IsValid())
        corridorComponentElmPtr->SetUserLabel(Utf8String(featureDefPtr->GetName().c_str()).c_str());

    auto geomBuilderPtr = GeometryBuilder::Create(*corridorPortion.GetModel(), categoryId,
                                                  graphicalElement.ToGeometrySource3d()->GetPlacement().GetOrigin(),
                                                  graphicalElement.ToGeometrySource3d()->GetPlacement().GetAngles());

    for (auto geomIter : GeometryCollection(*graphicalElement.ToGeometrySource()))
        {
        auto geomParams = geomIter.GetGeometryParams(); // Intentionally copied

        auto subCatId = geomParams.GetSubCategoryId();
        DgnSubCategoryId targetSubCatId = getOrInsertSubCategory(graphicalElement.GetDgnDb(), subCategoryMap,
                                                                 geomParams.GetCategoryId(), subCatId, categoryId);
        geomParams.SetCategoryId(categoryId, false);
        geomParams.SetSubCategoryId(targetSubCatId, false);

        geomBuilderPtr->Append(geomParams);

        auto geomPtr = geomIter.GetGeometryPtr();
        if (geomPtr.IsValid())
            geomBuilderPtr->Append(*geomPtr);
        }

    geomBuilderPtr->Finish(*corridorComponentElmPtr->ToGeometrySourceP());

    auto corridorComponentElmCPtr = corridorComponentElmPtr->Insert();
    if (corridorComponentElmCPtr.IsNull())
        return nullptr;

    BeSQLite::EC::ECInstanceKey insKey;
    corridorPortion.GetDgnDb().InsertLinkTableRelationship(insKey,
                                                            *corridorPortion.GetDgnDb().Schemas().GetClass(BIS_ECSCHEMA_NAME, "GraphicalElement3dRepresentsElement")->GetRelationshipClassCP(),
                                                            BeSQLite::EC::ECInstanceId(graphicalElement.GetElementId().GetValue()),
                                                            BeSQLite::EC::ECInstanceId(corridorComponentElmCPtr->GetElementId().GetValue()));

    auto startStationAsWStr = corridorSurface.GetStartStation();
    auto endStationAsWStr = corridorSurface.GetEndStation();

    WCharP startStationNextCharP, endStationNextCharP;
    double startDistAlong = wcstod(startStationAsWStr.c_str(), &startStationNextCharP);
    double endDistAlong = wcstod(endStationAsWStr.c_str(), &endStationNextCharP);

    auto alignmentCPtr = AlignmentBim::Alignment::Get(corridorPortion.GetDgnDb(), corridorPortion.GetMainAlignmentId());

    auto linearLocationPtr = LinearReferencing::LinearLocation::Create(*corridorComponentElmCPtr, categoryId,
        LinearReferencing::LinearLocation::ILinearlyLocatedSingleFromTo::CreateFromToParams(
            *alignmentCPtr,
            LinearReferencing::DistanceExpression(startDistAlong),
            LinearReferencing::DistanceExpression(endDistAlong)));
    linearLocationPtr->Insert();

    return corridorComponentElmCPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnElementCPtr updateCorridorComponent(CorridorSurfaceCR corridorSurface,
                                            Dgn::DgnElementId const& corridorComponentId,
                                            RoadRailBim::CorridorPortionElementCR corridorPortion,
                                            Dgn::DgnCategoryId const& categoryId,
                                            Dgn::DgnElementCR graphicalElement,
                                            bmap<DgnSubCategoryId, DgnSubCategoryId>& subCategoryMap)
    {
    auto corridorComponentElmPtr = graphicalElement.GetDgnDb().Elements().GetForEdit<Dgn::DgnElement>(corridorComponentId);

    auto featureDefPtr = corridorSurface.GetFeatureDefinition();
    if (featureDefPtr.IsNull())
        {
        Cif::FeaturizedConsensusItemPtr representationOf = corridorSurface.GetRepresentationOf();
        if (representationOf.IsValid())
            featureDefPtr = representationOf->GetFeatureDefinition();
        }

    if (featureDefPtr.IsValid())
        corridorComponentElmPtr->SetUserLabel(Utf8String(featureDefPtr->GetName().c_str()).c_str());

    auto geomBuilderPtr = GeometryBuilder::Create(*corridorPortion.GetModel(), categoryId,
                                                  graphicalElement.ToGeometrySource3d()->GetPlacement().GetOrigin(),
                                                  graphicalElement.ToGeometrySource3d()->GetPlacement().GetAngles());

    for (auto geomIter : GeometryCollection(*graphicalElement.ToGeometrySource()))
        {
        auto geomParams = geomIter.GetGeometryParams(); // Intentionally copied

        auto subCatId = geomParams.GetSubCategoryId();
        DgnSubCategoryId targetSubCatId = getOrInsertSubCategory(graphicalElement.GetDgnDb(), subCategoryMap,
                                                                 geomParams.GetCategoryId(), subCatId, categoryId);
        geomParams.SetCategoryId(categoryId, false);
        geomParams.SetSubCategoryId(targetSubCatId, false);

        geomBuilderPtr->Append(geomParams);

        auto geomPtr = geomIter.GetGeometryPtr();
        if (geomPtr.IsValid())
            geomBuilderPtr->Append(*geomPtr);
        }

    geomBuilderPtr->Finish(*corridorComponentElmPtr->ToGeometrySourceP());

    auto corridorComponentElmCPtr = corridorComponentElmPtr->Update();
    if (corridorComponentElmCPtr.IsNull())
        return nullptr;

    auto startStationAsWStr = corridorSurface.GetStartStation();
    auto endStationAsWStr = corridorSurface.GetEndStation();

    WCharP startStationNextCharP, endStationNextCharP;
    double startDistAlong = wcstod(startStationAsWStr.c_str(), &startStationNextCharP);
    double endDistAlong = wcstod(endStationAsWStr.c_str(), &endStationNextCharP);
    auto alignmentCPtr = AlignmentBim::Alignment::Get(corridorPortion.GetDgnDb(), corridorPortion.GetMainAlignmentId());

    auto linearLocationId = LinearReferencing::LinearLocation::Query(*corridorComponentElmCPtr);

    if (linearLocationId.IsValid())
        {
        auto linearLocationPtr = LinearReferencing::LinearLocation::GetForEdit(corridorPortion.GetDgnDb(), linearLocationId);

        if (corridorPortion.GetMainAlignmentId() != linearLocationPtr->GetLinearElementId())
            linearLocationPtr->SetLinearElement(alignmentCPtr.get());

        linearLocationPtr->SetFromDistanceAlongFromStart(startDistAlong);
        linearLocationPtr->SetToDistanceAlongFromStart(endDistAlong);
        linearLocationPtr->Update();
        }
    else
        {
        auto linearLocationPtr = LinearReferencing::LinearLocation::Create(*corridorComponentElmCPtr, categoryId,
            LinearReferencing::LinearLocation::ILinearlyLocatedSingleFromTo::CreateFromToParams(
                *alignmentCPtr,
                LinearReferencing::DistanceExpression(startDistAlong),
                LinearReferencing::DistanceExpression(endDistAlong)));
        linearLocationPtr->Insert();
        }


    return corridorComponentElmCPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::CreateCorridorComponents(CorridorCR cifCorridor, RoadRailBim::CorridorPortionElementCR corridorPortion)
    {
    auto genPhysicalObjClassCP = corridorPortion.GetDgnDb().Schemas().GetClass("Generic", "PhysicalObject");
    auto genPhysicalObjEnablerP = genPhysicalObjClassCP->GetDefaultStandaloneEnabler();
    DgnCode emptyCode = DgnCode::CreateEmpty();

    Dgn::DgnCategoryId categoryId;
    if (auto pathwayCP = corridorPortion.ToPathway())
        {
        if (pathwayCP->ToRailway())
            categoryId = RoadRailBim::RoadRailCategory::GetRailway(corridorPortion.GetDgnDb());
        else
            categoryId = RoadRailBim::RoadRailCategory::GetRoadway(corridorPortion.GetDgnDb());
        }
    else
        categoryId = RoadRailBim::RoadRailCategory::GetCorridor(corridorPortion.GetDgnDb());

    bmap<DgnSubCategoryId, DgnSubCategoryId> subCategoryMap;

    auto cifCorridorElmRefP = cifCorridor.GetElementHandle()->GetElementRef();
    for (auto& corridorSurfacePtr : m_converter.m_cifCorridorSurfaces)
        {
        if (corridorSurfacePtr->GetCorridor().IsNull())
            continue;

        if (corridorSurfacePtr->GetCorridor()->GetElementHandle()->GetElementRef() != cifCorridorElmRefP)
            continue;

        auto iter = m_converter.m_v8ToBimElmMap.find(corridorSurfacePtr->GetElementHandle()->GetElementRef());
        if (iter == m_converter.m_v8ToBimElmMap.end())
            continue;

        // TODO: create code from corridorSurface's GetName, if available.
        // Maybe its CodeSpec is defined by the CifCommon-level for generic PhysicalObjects.
        createCorridorComponent(*corridorSurfacePtr, *genPhysicalObjEnablerP, corridorPortion,
                                categoryId, emptyCode, *iter->second, subCategoryMap);
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::UpdateCorridorComponents(CorridorCR cifCorridor, RoadRailBim::CorridorPortionElementCR corridorPortion)
    {
    auto genPhysicalObjClassCP = corridorPortion.GetDgnDb().Schemas().GetClass("Generic", "PhysicalObject");
    auto genPhysicalObjEnablerP = genPhysicalObjClassCP->GetDefaultStandaloneEnabler();
    DgnCode emptyCode = DgnCode::CreateEmpty();

    Dgn::DgnCategoryId categoryId;
    if (auto pathwayCP = corridorPortion.ToPathway())
        {
        if (pathwayCP->ToRailway())
            categoryId = RoadRailBim::RoadRailCategory::GetRailway(corridorPortion.GetDgnDb());
        else
            categoryId = RoadRailBim::RoadRailCategory::GetRoadway(corridorPortion.GetDgnDb());
        }
    else
        categoryId = RoadRailBim::RoadRailCategory::GetCorridor(corridorPortion.GetDgnDb());

    BeSQLite::EC::ECSqlStatement stmt;
    stmt.Prepare(corridorPortion.GetDgnDb(), "SELECT TargetECInstanceId FROM " BIS_SCHEMA("GraphicalElement3dRepresentsElement")
                 " WHERE SourceECInstanceId = ?;");
    BeAssert(stmt.IsPrepared());

    bmap<DgnSubCategoryId, DgnSubCategoryId> subCategoryMap;

    auto cifCorridorElmRefP = cifCorridor.GetElementHandle()->GetElementRef();
    for (auto& corridorSurfacePtr : m_converter.m_cifCorridorSurfaces)
        {
        if (corridorSurfacePtr->GetCorridor().IsNull())
            continue;

        if (corridorSurfacePtr->GetCorridor()->GetElementHandle()->GetElementRef() != cifCorridorElmRefP)
            continue;

        auto iter = m_converter.m_v8ToBimElmMap.find(corridorSurfacePtr->GetElementHandle()->GetElementRef());
        if (iter == m_converter.m_v8ToBimElmMap.end())
            continue;

        stmt.BindId(1, iter->second->GetElementId());
        if (BeSQLite::DbResult::BE_SQLITE_ROW == stmt.Step())
            updateCorridorComponent(*corridorSurfacePtr, stmt.GetValueId<Dgn::DgnElementId>(0), corridorPortion,
                                    categoryId, *iter->second, subCategoryMap);
        else
            createCorridorComponent(*corridorSurfacePtr, *genPhysicalObjEnablerP, corridorPortion,
                                    categoryId, emptyCode, *iter->second, subCategoryMap);
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::CreateNewCorridor(
    CorridorCR cifCorridor,
    ORDConverter::Params& params,
    iModelBridgeSyncInfoFile::ChangeDetector::Results const& change, RoadRailBim::CorridorCPtr& bimCorridorCPtr)
    {
    auto cifAlignmentPtr = cifCorridor.GetCorridorAlignment();
    if (cifAlignmentPtr.IsNull())
        {
        ORDBRIDGE_LOGE("CreateNewCorridor '%s' - CIF Corridor with no Alignment.", Utf8String(cifCorridor.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    auto cifAlgElmRefP = cifAlignmentPtr->GetElementHandle()->GetElementRef();
    auto algToBimIter = m_converter.m_cifAlignmentToBimID.find(cifAlgElmRefP);
    if (algToBimIter == m_converter.m_cifAlignmentToBimID.end())
        {
        ORDBRIDGE_LOGE("CreateNewCorridor '%s' - BIM Alignment not found.", Utf8String(cifCorridor.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    auto bimAlignmentCPtr = AlignmentBim::Alignment::Get(m_converter.GetDgnDb(), algToBimIter->second);

    NetworkType networkType;
    auto cifCantsPtr = cifAlignmentPtr->GetCants();
    auto cifSuperSectionsPtr = cifAlignmentPtr->GetSuperElevationSections();

    if (cifCantsPtr.IsValid() && cifCantsPtr->MoveNext())
        networkType = NetworkType::Rail;
    else if (cifSuperSectionsPtr.IsValid() && cifSuperSectionsPtr->MoveNext())
        networkType = NetworkType::Road;
    else
        networkType = NetworkType::Undetermined;

    RoadRailBim::TransportationNetworkCP network;
    if (networkType == NetworkType::Rail)
        network = m_converter.GetRailNetwork();
    else
        network = m_converter.GetRoadNetwork();

    // TODO: Assuming BIM corridor start = 0, end = alignment's length. Those are not exposed through CIF SDK.
    auto corridorPtr = RoadRailBim::Corridor::Create(*network, RoadRailBim::Corridor::CreateFromToParams(*bimAlignmentCPtr,
                                                                                               DistanceExpression(0),
                                                                                               DistanceExpression(bimAlignmentCPtr->GetLength())));

    ORDConverterUtils::AssignFederationGuid(*corridorPtr->getP(), cifCorridor.GetSyncId());

    if (!WString::IsNullOrEmpty(cifCorridor.GetName().c_str()))
        {
        Utf8String corridorName(cifCorridor.GetName().c_str());
        corridorPtr->SetUserLabel(corridorName.c_str());

        auto bimCode = RoadRailBim::Corridor::CreateCode(*m_converter.GetRoadNetwork(), corridorName);

        uint32_t corSuffix = 0;
        // Addressing corridors with the same name
        do
            {
            if (!m_converter.GetDgnDb().Elements().QueryElementIdByCode(bimCode).IsValid())
                break;

            Utf8String bimCorridorNameDup = Utf8PrintfString("%s_%d", corridorName.c_str(), ++corSuffix);
            bimCode = RoadRailBim::Corridor::CreateCode(*m_converter.GetRoadNetwork(), bimCorridorNameDup);
            } while (true);

        corridorPtr->SetCode(bimCode);
        }

    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = corridorPtr->getP();
    if (BentleyStatus::SUCCESS != params.changeDetectorP->_UpdateBimAndSyncInfo(results, change))
        {
        ORDBRIDGE_LOGE("CreateNewCorridor '%s' - failed update and sync.", Utf8String(cifCorridor.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    corridorPtr->InsertLinearElementRelationship();

    bimCorridorCPtr = RoadRailBim::Corridor::Get(corridorPtr->GetDgnDb(), corridorPtr->GetElementId());

    auto bimAlignmentPtr = AlignmentBim::Alignment::GetForEdit(bimAlignmentCPtr->GetDgnDb(), bimAlignmentCPtr->GetElementId());
    bimAlignmentPtr->SetSource(bimCorridorCPtr.get());
    bimAlignmentCPtr = bimAlignmentPtr->Update();

    auto corridorModelPtr = PhysicalModel::Create(*bimCorridorCPtr->get());
    corridorModelPtr->SetIsPrivate(params.domainModelsPrivate);

    if (DgnDbStatus::Success != corridorModelPtr->Insert())
        {
        ORDBRIDGE_LOGE("CreateNewCorridor '%s' - failed PhysicalModel insert.", Utf8String(cifCorridor.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    auto transportationSystemCPtr = RoadRailBim::TransportationSystem::Insert(*bimCorridorCPtr, DefaultTransportationSystemName);
    if (params.domainModelsPrivate)
        {
        transportationSystemCPtr->GetTransportationSystemModel()->SetIsPrivate(params.domainModelsPrivate);
        transportationSystemCPtr->GetTransportationSystemModel()->Update();

        auto horizontalAlignmentsCPtr = AlignmentBim::HorizontalAlignments::Query(*transportationSystemCPtr->GetTransportationSystemModel());
        horizontalAlignmentsCPtr->GetHorizontalModel()->SetIsPrivate(params.domainModelsPrivate);
        horizontalAlignmentsCPtr->GetHorizontalModel()->Update();
        }

    RoadRailBim::CorridorPortionElementCPtr corridorPortionCPtr;
    if (networkType == NetworkType::Rail)
        {
        auto railwayPtr = RailBim::Railway::Create(*transportationSystemCPtr, bimAlignmentCPtr.get());
        railwayPtr->SetMainAlignment(bimAlignmentCPtr.get());
        corridorPortionCPtr = railwayPtr->Insert();
        }
    else if (networkType == NetworkType::Road)
        {
        auto roadwayPtr = RoadBim::Roadway::Create(*transportationSystemCPtr, bimAlignmentCPtr.get());
        roadwayPtr->SetMainAlignment(bimAlignmentCPtr.get());
        corridorPortionCPtr = roadwayPtr->Insert();
        }
    else
        {
        auto undeterminedPtr = RoadRailBim::UndeterminedCorridorPortion::Create(*transportationSystemCPtr, bimAlignmentCPtr.get());
        undeterminedPtr->SetMainAlignment(bimAlignmentCPtr.get());
        corridorPortionCPtr = undeterminedPtr->Insert();
        }

    if (corridorPortionCPtr.IsNull())
        {
        ORDBRIDGE_LOGE("CreateNewCorridor '%s' - failed pathway insert.", Utf8String(cifCorridor.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    if (auto pathwayCP = corridorPortionCPtr->ToPathway())
        {
        SpeedTablePtr speedTablePtr = cifAlignmentPtr->GetSpeedTable();
        if (speedTablePtr.IsValid())
            {
            Dgn::DefinitionModelCPtr standardsModelCPtr;
            if (networkType == NetworkType::Rail)
                standardsModelCPtr = RailBim::RailwayStandardsModelUtilities::Query(m_converter.GetJobSubject());
            else
                standardsModelCPtr = RoadBim::RoadwayStandardsModelUtilities::Query(m_converter.GetJobSubject());

            ConvertSpeedTable(speedTablePtr.get(), 0, *pathwayCP, *standardsModelCPtr,
                (params.rootModelUnitSystem == Dgn::UnitSystem::Metric) ?
                              RoadRailBim::DesignSpeedDefinition::UnitSystem::SI : RoadRailBim::DesignSpeedDefinition::UnitSystem::Imperial,
                              *m_converter.m_ordParams);
            }
        }

    return CreateCorridorComponents(cifCorridor, *corridorPortionCPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::UpdateCorridor(CorridorCR cifCorridor,
                                                    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change)
    {
    auto corridorPtr = RoadRailBim::Corridor::GetForEdit(changeDetector.GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());
    auto transportSystemCPtr = RoadRailBim::TransportationSystem::Query(*corridorPtr, DefaultTransportationSystemName);
    auto corridorPortionIds = transportSystemCPtr->QueryCorridorPortionIds();
    if (corridorPortionIds.size() != 1)
        return BentleyStatus::ERROR;

    auto corridorPortionCPtr = RoadRailBim::CorridorPortionElement::Get(changeDetector.GetDgnDb(), *corridorPortionIds.begin());

    UpdateCorridorComponents(cifCorridor, *corridorPortionCPtr);

    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = corridorPtr->getP();
    if (BentleyStatus::SUCCESS != changeDetector._UpdateBimAndSyncInfo(results, change))
        {
        ORDBRIDGE_LOGE("UpdateCorridor '%s' - failed update and sync.", Utf8String(cifCorridor.GetName().c_str()).c_str());
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ORDCorridorsConverter::ConvertCorridor(CorridorCR cifCorridor, ORDConverter::Params& params, DgnCategoryId targetCategoryId)
    {
    CifCorridorSourceItem sourceItem(cifCorridor);

    RoadRailBim::CorridorCPtr bimCorridorCPtr;

    // only convert a Corridor if it is new or has changed in the source
    auto change = params.changeDetectorP->_DetectChange(params.fileScopeId, sourceItem.Kind(), sourceItem, nullptr, params.spatialDataTransformHasChanged);
    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged == change.GetChangeType())
        {
        params.changeDetectorP->_OnElementSeen(change.GetSyncInfoRecord().GetDgnElementId());
        }
    else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New == change.GetChangeType())
        {
        if (BentleyStatus::SUCCESS != CreateNewCorridor(cifCorridor, params, change, bimCorridorCPtr))
            return DgnElementId();
        }
    else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Changed == change.GetChangeType())
        {
        if (BentleyStatus::SUCCESS != UpdateCorridor(cifCorridor, *params.changeDetectorP, change))
            return DgnElementId();
        }

    if (bimCorridorCPtr.IsNull())
        bimCorridorCPtr = RoadRailBim::Corridor::Get(params.changeDetectorP->GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());

    return bimCorridorCPtr->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::DgnModelP loadDgnModel(Bentley::DgnFileR dgnFile, ModelId rootModelId)
    {
    // The Converter's initialization step sets SetProcessingDisabled = true, but
    // CIF needs backpointers to be created while "processingAffected" in order to
    // be able to find some data (e.g. Alignment's names). Setting this to false
    // temporarily while DependencyManager().ProcessAffected() is called by
    // LoadRootModelById.
    DependencyManager::SetProcessingDisabled(false);

    StatusInt status;
    auto modelP = dgnFile.LoadRootModelById(&status, rootModelId, true, true, true);

    DependencyManager::SetProcessingDisabled(true);

    return modelP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
void updateProjectExtents(SpatialModelCR spatialModel, ORDConverter::Params& params, bool forceExtendExtents)
    {
    DgnDbR db = spatialModel.GetDgnDb();

    AxisAlignedBox3d modelExtents = spatialModel.QueryElementsRange();
    if (modelExtents.IsEmpty())
        return;

    modelExtents.Extend(0.5);

    if ((params.isCreatingNewDgnDb || !params.isUpdating) && !forceExtendExtents)
        {
        db.GeoLocation().SetProjectExtents(modelExtents);
        return;
        }

    // Make sure the project extents include the elements in the spatialModel, plus a margin
    AxisAlignedBox3d currentProjectExtents = db.GeoLocation().GetProjectExtents();
    if (!modelExtents.IsContained(currentProjectExtents))
        {
        currentProjectExtents.Extend(modelExtents);
        db.GeoLocation().SetProjectExtents(currentProjectExtents);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ConvertORDElementXDomain::ConvertORDElementXDomain(ORDConverter& converter) : m_converter(converter)
    {
    m_cifConsensusConnection = ConsensusConnection::Create(*m_converter.GetRootModelRefP());

    m_aspectAssignFuncs.push_back(&ConvertORDElementXDomain::AssignAlignmentAspect);
    m_aspectAssignFuncs.push_back(&ConvertORDElementXDomain::AssignLinear3dAspect);
    m_aspectAssignFuncs.push_back(&ConvertORDElementXDomain::AssignCorridorSurfaceAspect);
    m_aspectAssignFuncs.push_back(&ConvertORDElementXDomain::AssignCorridorAspect);
    m_aspectAssignFuncs.push_back(&ConvertORDElementXDomain::AssignTemplateDropAspect);
    m_aspectAssignFuncs.push_back(&ConvertORDElementXDomain::AssignSuperelevationAspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ConvertORDElementXDomain::Result ConvertORDElementXDomain::_PreConvertElement(DgnV8EhCR v8el, Dgn::DgnDbSync::DgnV8::Converter& conv, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const& v8mm)
    {
    StopWatchOnStack stopWatch(*s_preConvertStopWatch);

    //ORDExtensions will return Result::SkipElement if the element doesn't belong to their domain
    for (auto pExt : ORDConverterExtensionRegistry::s_extensions)
        {
        pExt->PreConvertElement(v8el, *static_cast<ORDConverter*>(&conv), v8mm);
        }

    if (m_elementsSeen.end() != m_elementsSeen.find(v8el.GetElementRef()))
        return Result::Proceed;

    m_elementsSeen.insert(v8el.GetElementRef());

    auto cifAlignmentPtr = Alignment::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
    Bentley::Cif::CorridorPtr cifCorridorPtr;
    if (!cifAlignmentPtr.IsValid())
        {
        auto cifLinearEntity3dPtr = LinearEntity3d::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
        if (cifLinearEntity3dPtr.IsValid())
            {
            cifAlignmentPtr = cifLinearEntity3dPtr->GetAlignment();
            cifCorridorPtr = cifLinearEntity3dPtr->GetCorridor();
            if (cifAlignmentPtr.IsValid())
                m_converter.m_cifGeneratedLinear3ds.push_back(cifLinearEntity3dPtr);
            }
        else
            {
            cifCorridorPtr = Corridor::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
            }
        }

    if (cifCorridorPtr.IsValid())
        {
        auto elmRefP = cifCorridorPtr->GetElementHandle()->GetElementRef();
        if (m_corridorV8RefSet.end() == m_corridorV8RefSet.find(elmRefP))
            {
            m_corridorV8RefSet.insert(elmRefP);
            m_converter.m_cifCorridors.push_back(cifCorridorPtr);
            }
        }

    if (cifAlignmentPtr.IsValid())
        {
        if (cifAlignmentPtr->IsFinalElement())
            {
            auto elmRefP = cifAlignmentPtr->GetElementHandle()->GetElementRef();
            if (m_alignmentV8RefSet.end() == m_alignmentV8RefSet.find(elmRefP))
                {
                m_alignmentV8RefSet.insert(elmRefP);
                m_converter.m_cifAlignments.push_back(cifAlignmentPtr);
                }
            }
        else
            {
            return Result::SkipElement;
            }
        }

    return Result::Proceed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvertORDElementXDomain::_GetBasisTransform(Bentley::Transform& transform, DgnV8EhCR v8el, Dgn::DgnDbSync::DgnV8::Converter&)
    {
    // From Sam Wilson:
    // Brien and I found a problem that causes project extents to be extended unduly.
    // It happens when the converter tries to guess at the local coordinate system or basis for long complex chains
    // that wind through space. Sometimes it computes an LCS that is rotated. In the ORD case, we clearly want an LCS
    // that is aligned with global x, y, z. We are thinking about various rules of thumb that might be used to improve
    // the converter�s guesses. But it would be simpler and more reliable for ORDBridge to register handler extensions
    // that supply a sensible basis transform for these problem elements.
    // ORDBridge should override the _GetBasisTransform method on its XDomain. That should return a transform with a
    // rotation matrix component that is the identity matrix. The translation part should ideally be a point in the chain,
    // but 0,0 shouldn't cause further problems.
    if (FeaturizedConsensusItem::CreateFromElementHandle(*m_cifConsensusConnection, v8el).IsNull())
        return false;

    auto displayHandlerP = v8el.GetDisplayHandler();
    if (!displayHandlerP)
        return false;

    Bentley::DPoint3d origin;
    displayHandlerP->GetSnapOrigin(v8el, origin);

    transform = Bentley::Transform::FromIdentity();
    transform.SetTranslation(origin);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertORDElementXDomain::_DetermineElementParams(DgnClassId& classId, DgnCode& code, DgnCategoryId& categoryId, DgnV8EhCR v8el, DgnDbSync::DgnV8::Converter& converter, ECObjectsV8::IECInstance const* primaryV8Instance, DgnDbSync::DgnV8::ResolvedModelMapping const& v8mm)
    {
    StopWatchOnStack stopWatch(*s_determineElemParamsStopWatch);
    m_currentFeatureDefName.clear();
    m_currentFeatureName.clear();
    m_currentFeatureDescription.clear();

    if (v8mm.GetDgnModel().Is2dModel())
        return;

    if (!classId.IsValid())
        {
        auto featurizedPtr = FeaturizedConsensusItem::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
        if (featurizedPtr.IsValid())
            {
            auto featureName = featurizedPtr->GetName();
            auto featureDefPtr = featurizedPtr->GetFeatureDefinition();
            auto featureDescription = featureDefPtr.IsValid() ? featureDefPtr->GetDescription() : featurizedPtr->GetFeatureDescription();

            if (featureDefPtr.IsNull())
                {
                Cif::FeaturizedConsensusItemPtr representationOf = featurizedPtr->GetRepresentationOf();
                if (representationOf.IsValid())
                    {
                    featureName = representationOf->GetName();
                    featureDefPtr = representationOf->GetFeatureDefinition();
                    featureDescription = featureDefPtr.IsValid() ? featureDefPtr->GetDescription() : representationOf->GetFeatureDescription();
                    }
                }

            if (featureDefPtr.IsValid())
                {
                m_currentFeatureName = Utf8String(featureName.c_str());
                m_currentFeatureDescription = Utf8String(featureDescription.c_str());
                auto featureDefName = featureDefPtr->GetName();

                if (!WString::IsNullOrEmpty(featureDefName.c_str()))
                    {
                    m_currentFeatureDefName = Utf8String(featureDefName.c_str());
                    auto className = ORDDynamicSchemaGenerator::FeatureNameToClassName(m_currentFeatureDefName.c_str());
                    auto dynamicClassId = converter.GetDgnDb().Schemas().GetClassId(ORDDynamicSchemaGenerator::GetTargetSchemaName(), className);
                    if (dynamicClassId.IsValid())
                        {
                        classId = dynamicClassId;
                        }
                    }
                }
            }

        /* TODO: Delete existing element if its classId needs to change
        if (m_converter.IsUpdating())
            {
            iModelExternalSourceAspect::ElementAndAspectId elementAndAspectId =
                iModelExternalSourceAspect::FindElementBySourceId(m_converter.GetDgnDb(),
                    DgnElementId(m_converter.m_ordParams->fileScopeId), "Element",
                    Utf8PrintfString("%d", v8el.GetElementId()).c_str());
            if (elementAndAspectId.elementId.IsValid())
                {
                auto existingElementId = elementAndAspectId.elementId;
                auto existingElmCPtr = m_converter.GetDgnDb().Elements().GetElement(existingElementId);
                if (classId != existingElmCPtr->GetElementClassId())
                    {
                    existingElmCPtr->Delete();
                    ORDBRIDGE_LOGI("Deleting elementId %lld because its classId changed.", existingElementId.GetValue());
                    }
                }
            }*/
        }

    for (auto pExt : ORDConverterExtensionRegistry::s_extensions)
        pExt->DetermineElementParams(classId, code, categoryId, v8el, *static_cast<ORDConverter*>(&converter), primaryV8Instance, v8mm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void assignORDFeatureAspect(Dgn::DgnElementR element, Utf8StringCR featureName, Utf8StringCR featureDefName, Utf8StringCR featureDescription)
    {
    if (auto featureAspectP = DgnV8ORDBim::FeatureAspect::GetP(element))
        {
        featureAspectP->SetName(featureName.c_str());
        featureAspectP->SetDefinitionName(featureDefName.c_str());
        featureAspectP->SetDescription(featureDescription.c_str());
        }
    else
        {
        auto featureAspectPtr = DgnV8ORDBim::FeatureAspect::Create(featureName.c_str(), featureDefName.c_str(), featureDescription.c_str());
        DgnV8ORDBim::FeatureAspect::Set(element, *featureAspectPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void assignORDAlignmentAspect(Dgn::DgnElementR element, Cif::AlignmentCR alignment, Utf8StringCR featureDefName)
    {
    Utf8String activeProfileName;

    if (ORDAlignmentsConverter::IsDesignAlignment(featureDefName))
        {
        ModelRefPinner modelPinner;

        auto profilePtr = alignment.GetActiveProfile();
        if (profilePtr.IsValid())
            activeProfileName = Utf8String(profilePtr->GetName().c_str());
        }

    auto linearGeomPtr = alignment.GetLinearGeometry();

    Bentley::CurveVectorPtr horizCurveVectorPtr;
    if (linearGeomPtr.IsValid())
        linearGeomPtr->GetCurveVector(horizCurveVectorPtr);

    Bentley::DPoint3d startPoint, endPoint;
    startPoint.Zero(); endPoint.Zero();

    if (horizCurveVectorPtr.IsValid())
        horizCurveVectorPtr->GetStartEnd(startPoint, endPoint);

    if (auto alignmentAspectP = DgnV8ORDBim::AlignmentAspect::GetP(element))
        {
        alignmentAspectP->SetActiveProfileName(activeProfileName.c_str());
        alignmentAspectP->SetStartPoint({startPoint.x, startPoint.y});
        alignmentAspectP->SetEndPoint({endPoint.x, endPoint.y});
        }
    else
        {
        auto alignmentAspectPtr = DgnV8ORDBim::AlignmentAspect::Create({startPoint.x, startPoint.y}, {endPoint.x, endPoint.y}, activeProfileName.c_str());
        DgnV8ORDBim::AlignmentAspect::Set(element, *alignmentAspectPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void assignTemplateDropAspect(Dgn::DgnElementR element, Cif::TemplateDropCR templateDrop)
    {
    auto templatePtr = templateDrop.GetTemplate();
    auto description = Utf8String(templateDrop.GetDescription().c_str());

    Utf8String templateName;
    if (templatePtr.IsValid())
        templateName = Utf8String(templatePtr->GetTemplatePath().c_str());

    if (auto templateDropAspectP = DgnV8ORDBim::TemplateDropAspect::GetP(element))
        {
        templateDropAspectP->SetInterval(templateDrop.GetInterval());
        templateDropAspectP->SetDescription(description.c_str());

        if (templatePtr.IsValid())
            templateDropAspectP->SetTemplateName(templateName.c_str());
        }
    else
        {
        auto templateDropAspectPtr = DgnV8ORDBim::TemplateDropAspect::Create(templateDrop.GetInterval(), templateName.c_str(), description.c_str());
        DgnV8ORDBim::TemplateDropAspect::Set(element, *templateDropAspectPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void assignCorridorAspect(Dgn::DgnElementR element, CorridorCR corridor)
    {
    auto name = Utf8String(corridor.GetName().c_str());
    auto corridorAlignmentPtr = corridor.GetCorridorAlignment();

    Utf8String activeProfileName, horizontalName;
    if (corridorAlignmentPtr.IsValid())
        {
        horizontalName = Utf8String(corridorAlignmentPtr->GetName().c_str());

        ModelRefPinner modelPinner;

        auto profilePtr = corridorAlignmentPtr->GetActiveProfile();
        if (profilePtr.IsValid())
            activeProfileName = Utf8String(profilePtr->GetName().c_str());
        }

    if (auto corridorAspectP = DgnV8ORDBim::CorridorAspect::GetP(element))
        {
        corridorAspectP->SetName(name.c_str());
        corridorAspectP->SetHorizontalName(horizontalName.c_str());
        corridorAspectP->SetActiveProfileName(activeProfileName.c_str());
        }
    else
        {
        auto corridorAspectPtr = DgnV8ORDBim::CorridorAspect::Create(name.c_str(), horizontalName.c_str(), activeProfileName.c_str());
        DgnV8ORDBim::CorridorAspect::Set(element, *corridorAspectPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void assignStationRangeAspect(Dgn::DgnElementR element, double startStation, double endStation)
    {
    if (auto stationRangeAspectP = DgnV8ORDBim::StationRangeAspect::GetP(element))
        {
        stationRangeAspectP->SetStartStation(startStation);
        stationRangeAspectP->SetEndStation(endStation);
        }
    else
        {
        auto stationRangeAspectPtr = DgnV8ORDBim::StationRangeAspect::Create(startStation, endStation);
        DgnV8ORDBim::StationRangeAspect::Set(element, *stationRangeAspectPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void assignSuperelevationAspect(Dgn::DgnElementR element, Cif::SuperElevationCR cifSuperelevation)
    {
    auto name = Utf8String(cifSuperelevation.GetName().c_str());
    if (auto superelevationAspectP = DgnV8ORDBim::SuperelevationAspect::GetP(element))
        {
        superelevationAspectP->SetName(name.c_str());
        superelevationAspectP->SetNormalCrossSlope(cifSuperelevation.GetNormalCrossSlope());
        }
    else
        {
        auto superelevationAspectPtr = DgnV8ORDBim::SuperelevationAspect::Create(name.c_str(), cifSuperelevation.GetNormalCrossSlope());
        DgnV8ORDBim::SuperelevationAspect::Set(element, *superelevationAspectPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void assignCorridorSurfaceAspect(Dgn::DgnElementR element, Cif::CorridorSurfaceCR cifCorridorSurface)
    {
    bool isTopMesh = cifCorridorSurface.IsTopMesh();
    bool isBottomMesh = cifCorridorSurface.IsBottomMesh();
    auto description = Utf8String(cifCorridorSurface.GetDescription().c_str());

    Utf8String corridorName, horizontalName, profileName;
    auto corridorPtr = cifCorridorSurface.GetCorridor();

    if (corridorPtr.IsValid())
        {
        corridorName = Utf8String(corridorPtr->GetName().c_str());
        auto alignmentPtr = corridorPtr->GetCorridorAlignment();

        if (alignmentPtr.IsValid())
            {
            ModelRefPinner modelPinner;

            horizontalName = Utf8String(alignmentPtr->GetName().c_str());
            auto profilePtr = alignmentPtr->GetActiveProfile();
            if (profilePtr.IsValid())
                profileName = Utf8String(profilePtr->GetName().c_str());
            }
        }
    else
        {
        auto name = Utf8String(cifCorridorSurface.GetName().c_str());
        ORDBRIDGE_LOGW(Utf8PrintfString("CIF CorridorSurface '%s' '%s' - related corridor not found.", name.c_str(), description.c_str()).c_str());
        }

    if (auto corridorSurfaceAspectP = DgnV8ORDBim::CorridorSurfaceAspect::GetP(element))
        {
        corridorSurfaceAspectP->SetIsTopMesh(isTopMesh);
        corridorSurfaceAspectP->SetIsBottomMesh(isBottomMesh);
        corridorSurfaceAspectP->SetDescription(description.c_str());
        corridorSurfaceAspectP->SetCorridorName(corridorName.c_str());
        corridorSurfaceAspectP->SetHorizontalName(horizontalName.c_str());
        corridorSurfaceAspectP->SetProfileName(profileName.c_str());
        }
    else
        {
        auto corridorSurfaceAspectPtr = DgnV8ORDBim::CorridorSurfaceAspect::Create(isTopMesh, isBottomMesh, description.c_str(),
            corridorName.c_str(), horizontalName.c_str(), profileName.c_str());
        DgnV8ORDBim::CorridorSurfaceAspect::Set(element, *corridorSurfaceAspectPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void assignStationRangeAspect(Dgn::DgnElementR element, Cif::CorridorSurfaceCR cifCorridorSurface)
    {
    auto startStationAsWStr = cifCorridorSurface.GetStartStation();
    auto endStationAsWStr = cifCorridorSurface.GetEndStation();

    WCharP startStationNextCharP, endStationNextCharP;
    double startStation = wcstod(startStationAsWStr.c_str(), &startStationNextCharP);
    double endStation = wcstod(endStationAsWStr.c_str(), &endStationNextCharP);

    if (startStationAsWStr.c_str() != startStationNextCharP && endStationAsWStr.c_str() != endStationNextCharP)
        {
        if (auto stationRangeAspectP = DgnV8ORDBim::StationRangeAspect::GetP(element))
            {
            stationRangeAspectP->SetStartStation(startStation);
            stationRangeAspectP->SetEndStation(endStation);
            }
        else
            {
            auto stationRangeAspectPtr = DgnV8ORDBim::StationRangeAspect::Create(startStation, endStation);
            DgnV8ORDBim::StationRangeAspect::Set(element, *stationRangeAspectPtr);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void assignQuantityAspect(Dgn::DgnElementR element, Cif::AlignmentCR cifAlignment)
    {
    double length = cifAlignment.GetLinearGeometry()->GetLength2d();

    if (auto linearQuantityAspectP = DgnV8ORDBim::LinearQuantityAspect::GetP(element))
        {
        linearQuantityAspectP->SetLength(length);
        }
    else
        {
        auto linearQuantityAspectPtr = DgnV8ORDBim::LinearQuantityAspect::Create(length);
        DgnV8ORDBim::LinearQuantityAspect::Set(element, *linearQuantityAspectPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void assignQuantityAspect(Dgn::DgnElementR element, Cif::Linear3dConsensusItemCR cifLinear3d)
    {
    double length = cifLinear3d.GetLinearGeometry()->GetLength2d();

    if (auto linearQuantityAspectP = DgnV8ORDBim::LinearQuantityAspect::GetP(element))
        {
        linearQuantityAspectP->SetLength(length);
        }
    else
        {
        auto linearQuantityAspectPtr = DgnV8ORDBim::LinearQuantityAspect::Create(length);
        DgnV8ORDBim::LinearQuantityAspect::Set(element, *linearQuantityAspectPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void assignQuantityAspect(Dgn::DgnElementR element, Cif::CorridorSurfaceCR cifCorridorSurface)
    {
    ValidatedDouble volume, surfaceArea;

    double polyfaceVolume = 0.0;
    Bentley::DPoint3d centroid;
    Bentley::RotMatrix axes;
    Bentley::DVec3d momentXYZ;

    auto polyfaceHeaderPtr = cifCorridorSurface.GetMesh();
    if (polyfaceHeaderPtr->ComputePrincipalMomentsAllowMissingSideFacets(polyfaceVolume, centroid, axes, momentXYZ, true, 1e-3))
        volume = fabs(polyfaceVolume);

    size_t numPositive, numPerpendicular, numNegative;
    double forwardProjectedSum, reverseProjectedSum, forwardAbsoluteSum, reverseAbsoluteSum, perpendicularAbsoluteSum;

    double sumAreas = polyfaceHeaderPtr->SumDirectedAreas(
        Bentley::DVec3d::UnitZ(), numPositive, numPerpendicular, numNegative,
        forwardProjectedSum, reverseProjectedSum, forwardAbsoluteSum, reverseAbsoluteSum, perpendicularAbsoluteSum);
    if (fabs(sumAreas) > DBL_EPSILON)
        {
        if (numPositive > 0)
            surfaceArea = forwardAbsoluteSum;
        else
            surfaceArea = reverseAbsoluteSum;
        }

    if (!volume.IsValid() && !surfaceArea.IsValid())
        return;

    double uorsPerMeter = cifCorridorSurface.GetDgnModelP()->GetModelInfo().GetUorPerMeter();
    double squaredUorsPerMeter = uorsPerMeter * uorsPerMeter;
    double cubicUorsPerMeter = squaredUorsPerMeter * uorsPerMeter;

    double volumeVal = volume.IsValid() ? volume.Value() / cubicUorsPerMeter : NAN;
    double topSurfaceAreaVal = surfaceArea.IsValid() ? surfaceArea.Value() / squaredUorsPerMeter : NAN;
    if (auto volumetricQuantityAspectP = DgnV8ORDBim::VolumetricQuantityAspect::GetP(element))
        {
        volumetricQuantityAspectP->SetSlopedArea(topSurfaceAreaVal);
        volumetricQuantityAspectP->SetVolume(volumeVal);
        }
    else
        {
        auto volumetricQuantityAspectPtr = DgnV8ORDBim::VolumetricQuantityAspect::Create(volumeVal, topSurfaceAreaVal);
        DgnV8ORDBim::VolumetricQuantityAspect::Set(element, *volumetricQuantityAspectPtr);
        }

    if (volume.IsValid())
        {
        auto qtoVolumeClassCP = element.GetDgnDb().Schemas().GetClass(ORDDynamicSchemaGenerator::GetTargetSchemaName(), ORDDynamicSchemaGenerator::GetQTOVolumeClassName());

        if (auto qtoVolumeInstanceP = Dgn::DgnElement::GenericUniqueAspect::GetAspectP(element, *qtoVolumeClassCP))
            {
            qtoVolumeInstanceP->SetValue("NetVolume", ECN::ECValue(volumeVal));
            qtoVolumeInstanceP->SetValue("GrossVolume", ECN::ECValue(volumeVal));
            }
        else
            {
            auto enablerP = qtoVolumeClassCP->GetDefaultStandaloneEnabler();
            auto instancePtr = enablerP->CreateInstance();
            instancePtr->SetValue("NetVolume", ECN::ECValue(volumeVal));
            instancePtr->SetValue("GrossVolume", ECN::ECValue(volumeVal));
            Dgn::DgnElement::GenericUniqueAspect::SetAspect(element, *instancePtr);
            }
        }

    if (surfaceArea.IsValid())
        {
        auto qtoSurfaceAreaClassCP = element.GetDgnDb().Schemas().GetClass(ORDDynamicSchemaGenerator::GetTargetSchemaName(), ORDDynamicSchemaGenerator::GetQTOSurfaceAreaClassName());

        if (auto qtoSurfaceAreaInstanceP = Dgn::DgnElement::GenericUniqueAspect::GetAspectP(element, *qtoSurfaceAreaClassCP))
            {
            qtoSurfaceAreaInstanceP->SetValue("GrossSurfaceArea", ECN::ECValue(topSurfaceAreaVal));
            qtoSurfaceAreaInstanceP->SetValue("NetSurfaceArea", ECN::ECValue(topSurfaceAreaVal));
            }
        else
            {
            auto enablerP = qtoSurfaceAreaClassCP->GetDefaultStandaloneEnabler();
            auto instancePtr = enablerP->CreateInstance();
            instancePtr->SetValue("GrossSurfaceArea", ECN::ECValue(topSurfaceAreaVal));
            instancePtr->SetValue("NetSurfaceArea", ECN::ECValue(topSurfaceAreaVal));
            Dgn::DgnElement::GenericUniqueAspect::SetAspect(element, *instancePtr);
            }

        auto qtoSideAreasClassCP = element.GetDgnDb().Schemas().GetClass(ORDDynamicSchemaGenerator::GetTargetSchemaName(), ORDDynamicSchemaGenerator::GetQTOSideAreasClassName());

        if (auto qtoSideAreasInstanceP = Dgn::DgnElement::GenericUniqueAspect::GetAspectP(element, *qtoSideAreasClassCP))
            {
            qtoSideAreasInstanceP->SetValue("TopGrossArea", ECN::ECValue(topSurfaceAreaVal));
            qtoSideAreasInstanceP->SetValue("TopNetArea", ECN::ECValue(topSurfaceAreaVal));
            }
        else
            {
            auto enablerP = qtoSideAreasClassCP->GetDefaultStandaloneEnabler();
            auto instancePtr = enablerP->CreateInstance();
            instancePtr->SetValue("TopGrossArea", ECN::ECValue(topSurfaceAreaVal));
            instancePtr->SetValue("TopNetArea", ECN::ECValue(topSurfaceAreaVal));
            Dgn::DgnElement::GenericUniqueAspect::SetAspect(element, *instancePtr);
            }
        }

    auto featureDefPtr = cifCorridorSurface.GetFeatureDefinition();
    if (featureDefPtr.IsValid())
        {
        auto featureDefName = featureDefPtr->GetName();
        if (!WString::IsNullOrEmpty(featureDefName.c_str()))
            {
            auto qtoMaterialClassCP = element.GetDgnDb().Schemas().GetClass(ORDDynamicSchemaGenerator::GetTargetSchemaName(), ORDDynamicSchemaGenerator::GetQTOMaterialClassName());

            if (auto qtoMaterialInstanceP = Dgn::DgnElement::GenericUniqueAspect::GetAspectP(element, *qtoMaterialClassCP))
                qtoMaterialInstanceP->SetValue("Material", ECN::ECValue(Utf8String(featureDefName.c_str()).c_str()));
            else
                {
                auto enablerP = qtoMaterialClassCP->GetDefaultStandaloneEnabler();
                auto instancePtr = enablerP->CreateInstance();
                instancePtr->SetValue("Material", ECN::ECValue(Utf8String(featureDefName.c_str()).c_str()));
                Dgn::DgnElement::GenericUniqueAspect::SetAspect(element, *instancePtr);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvertORDElementXDomain::AssignLinear3dAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const
    {
    StopWatchOnStack stopWatch(*s_assignLinear3dAspectStopWatch);

    if (v8el.GetDgnModelP()->Is3d())
        {
        auto cifConsensusItemPtr = Linear3dConsensusItem::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
        if (auto cifLinear3dCP = dynamic_cast<Linear3dConsensusItemCP>(cifConsensusItemPtr.get()))
            {
            assignQuantityAspect(element, *cifLinear3dCP);
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvertORDElementXDomain::AssignTemplateDropAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const
    {
    StopWatchOnStack stopWatch(*s_assignTemplateDropAspectStopWatch);

    auto templateDropPtr = TemplateDrop::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
    if (templateDropPtr.IsValid())
        {
        assignTemplateDropAspect(element, *templateDropPtr);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvertORDElementXDomain::AssignSuperelevationAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const
    {
    StopWatchOnStack stopWatch(*s_assignSuperelevationAspectStopWatch);

    auto superElevationPtr = SuperElevation::CreateFromElementHandle(v8el);
    if (superElevationPtr.IsValid())
        {
        // The following check is due to an null pointer error in CIF when calling GetEndDistance().
        // This check is a band-aid fix for now to avoid getting into trouble.
        if (superElevationPtr->GetParentSection()->GetAlignment().IsValid())
            {
            assignStationRangeAspect(element, superElevationPtr->GetStartDistance(), superElevationPtr->GetEndDistance());
            assignSuperelevationAspect(element, *superElevationPtr);
            return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvertORDElementXDomain::AssignCorridorAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const
    {
    StopWatchOnStack stopWatch(*s_assignCorridorAspectStopWatch);

    auto cifCorridorPtr = Corridor::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
    if (cifCorridorPtr.IsValid())
        {
        assignCorridorAspect(element, *cifCorridorPtr);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvertORDElementXDomain::AssignFeatureAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const
    {
    if (!Utf8String::IsNullOrEmpty(m_currentFeatureDefName.c_str()) ||
        !Utf8String::IsNullOrEmpty(m_currentFeatureName.c_str()))
        {
        assignORDFeatureAspect(element, m_currentFeatureName, m_currentFeatureDefName, m_currentFeatureDescription);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvertORDElementXDomain::AssignAlignmentAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const
    {
    StopWatchOnStack stopWatch(*s_assignAlignmentAspectStopWatch);

    auto alignmentPtr = Alignment::CreateFromElementHandle(v8el);
    if (alignmentPtr.IsValid())
        {
        assignORDAlignmentAspect(element, *alignmentPtr, m_currentFeatureDefName);
        assignQuantityAspect(element, *alignmentPtr);

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void insertClippingElement(CorridorSurfaceCR corridorSurface, DgnDbR dgnDb,
                           DgnModelId const& clippingsModelId, Dgn::DgnCategoryId const& categoryId)
    {
    auto v8MeshPtr = corridorSurface.GetMesh();
    if (v8MeshPtr.IsNull())
        return;

    PolyfaceHeaderPtr bimMeshPtr;
    DgnDbSync::DgnV8::Converter::ConvertPolyface(bimMeshPtr, *v8MeshPtr);
    if (bimMeshPtr.IsNull())
        return;

    bvector<bvector<ptrdiff_t>> readIndices;
    if (!bimMeshPtr->PartitionReadIndicesByNormal(DVec3d::UnitZ(), readIndices))
        return;

    bvector<PolyfaceHeaderPtr> subMeshes;
    if (!bimMeshPtr->CopyPartitions(readIndices, subMeshes) || subMeshes.empty())
        return;

    PolyfaceHeaderPtr subMeshTopPtr = subMeshes[0];
    if (subMeshTopPtr->GetPointCount() == 0)
        return;

    Transform projectToXYTransf;
    projectToXYTransf.InitFromProjectionToPlane(DPoint3d(), DVec3d::UnitZ());

    auto dgnModelP = corridorSurface.GetDgnModelP();
    auto& v8OriginCR = dgnModelP->GetModelInfo().GetGlobalOrigin();
    double toMetersFactor = 1.0 / DgnV8Api::ModelInfo::GetUorPerMeter(dgnModelP->GetModelInfoCP());
    auto toMetersTransform = Transform::FromFixedPointAndScaleFactors(DPoint3d::From(0.0, 0.0, 0.0), toMetersFactor, toMetersFactor, 1.0);
    toMetersTransform.SetTranslation({v8OriginCR.x, v8OriginCR.y, v8OriginCR.z});

    subMeshTopPtr->Transform(toMetersTransform);
    subMeshTopPtr->Transform(projectToXYTransf);

    auto genSpatialLocClassCP = dgnDb.Schemas().GetClass("Generic", "SpatialLocation");
    auto genSpatialLocEnablerP = genSpatialLocClassCP->GetDefaultStandaloneEnabler();
    DgnCode emptyCode = DgnCode::CreateEmpty();

    auto ecInstancePtr = genSpatialLocEnablerP->CreateInstance();
    ecInstancePtr->SetValue("Model", ECN::ECValue(clippingsModelId));
    ecInstancePtr->SetValue("Category", ECN::ECValue(categoryId));
    ecInstancePtr->SetValue("CodeSpec", ECN::ECValue(emptyCode.GetCodeSpecId()));
    ecInstancePtr->SetValue("CodeScope", ECN::ECValue(emptyCode.GetScopeElementId(dgnDb)));

    auto clippingElmPtr = dgnDb.Elements().CreateElement(*ecInstancePtr);
    BeAssert(clippingElmPtr.IsValid());

    auto geomBuilderPtr = GeometryBuilder::Create(*clippingElmPtr->GetModel(), categoryId, DPoint3d::From(0, 0, 0));
    BeAssert(geomBuilderPtr.IsValid());

    Dgn::Render::GeometryParams geomParams(categoryId);
    geomParams.SetLineColor(Dgn::ColorDef::Black());
    geomParams.SetFillColor(Dgn::ColorDef::Black());
    if (!geomBuilderPtr->Append(geomParams))
        {
        BeAssert(false);
        }

    if (!geomBuilderPtr->Append(*subMeshTopPtr))
        {
        BeAssert(false);
        }

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(*clippingElmPtr->ToGeometrySourceP()))
        {
        BeAssert(false);
        }

    if (clippingElmPtr->Insert().IsNull())
        {
        BeAssert(false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvertORDElementXDomain::AssignCorridorSurfaceAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const
    {
    StopWatchOnStack stopWatch(*s_assignCorridorSurfaceAspectStopWatch);

    if (v8el.GetDgnModelP()->Is3d())
        {
        auto featurizedPtr = FeaturizedConsensusItem::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
        if (featurizedPtr.IsValid())
            {
            if (auto cifCorridorSurfaceP = dynamic_cast<CorridorSurfaceP>(featurizedPtr.get()))
                {
                m_converter.m_cifCorridorSurfaces.push_back(
                    Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::CorridorSurface>(cifCorridorSurfaceP));

                assignCorridorSurfaceAspect(element, *cifCorridorSurfaceP);
                assignStationRangeAspect(element, *cifCorridorSurfaceP);
                assignQuantityAspect(element, *cifCorridorSurfaceP);

                insertClippingElement(*cifCorridorSurfaceP, m_converter.GetDgnDb(), m_converter.m_clippingsModelId,
                                      element.ToGeometrySource()->GetCategoryId());
                return true;
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertORDElementXDomain::_ProcessResults(DgnDbSync::DgnV8::ElementConversionResults& elRes, DgnV8EhCR v8el, DgnDbSync::DgnV8::ResolvedModelMapping const& v8mm, DgnDbSync::DgnV8::Converter&)
    {
    StopWatchOnStack stopWatch(*s_processResultsStopWatch);
    if (m_converter.m_v8ToBimElmMap.end() != m_converter.m_v8ToBimElmMap.find(v8el.GetElementRef()))
        return;

    AssignFeatureAspect(*elRes.m_element, v8el);

    for (auto& func : m_aspectAssignFuncs)
        {
        if ((this->*func)(*elRes.m_element, v8el))
            break;
        }

    m_converter.m_v8ToBimElmMap.insert({v8el.GetElementRef(), elRes.m_element.get()});

    auto bimModelId = v8mm.GetDgnModel().GetModelId();
    if (v8mm.GetV8Model().Is3D())
        {
        auto iter = m_converter.m_3dModels.find(bimModelId);
        if (iter == m_converter.m_3dModels.end())
            m_converter.m_3dModels.insert(bimModelId);
        }
    else
        {
        auto iter = m_converter.m_planViewModels.find(bimModelId);
        if (iter == m_converter.m_planViewModels.end())
            m_converter.m_planViewModels.insert(bimModelId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::DgnPlatform::ModelId ORDConverter::_GetRootModelId()
    {
    auto rootModelId = T_Super::_GetRootModelId();
    auto rootDgnFileP = GetRootV8File();
    ModelRefPinner modelPinner;

    DgnV8Api::DgnFileStatus openStatus;
    if (auto rootModelRefP = rootDgnFileP->LoadRootModelById((Bentley::StatusInt*)&openStatus, rootModelId, /*fillCache*/true, /*loadRefs*/true, GetParams().GetProcessAffected()))
        {
        if (auto planModelRefP = GeometryModelDgnECDataBinder::GetInstance().GetPlanModelFromModel(rootModelRefP))
            {
            rootModelId = planModelRefP->GetModelId();
            ORDBRIDGE_LOGI("CIF found Plan Model '%s' - using it as root-model.", Utf8String(planModelRefP->GetModelNameCP()).c_str());
            }
        else
            {
            ORDBRIDGE_LOGW("CIF couldn't find any Plan Model. Continuing with specified root-model.");
            }
        }
    else
        {
        ORDBRIDGE_LOGE("Specified root-model could not be loaded.");
        }

    return rootModelId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::CreateDesignAlignments()
    {
    if (m_cifCorridors.empty())
        return;

    Bentley::ElementRefP lastCorridorRefP = nullptr;

    auto alignmentsConverterPtr = ORDAlignmentsConverter::Create(*this);
    for (auto& cifCorridorPtr : m_cifCorridors)
        {
        if (lastCorridorRefP == cifCorridorPtr->GetElementHandle()->GetElementRef())
            continue;

        lastCorridorRefP = cifCorridorPtr->GetElementHandle()->GetElementRef();

        auto cifAlignmentPtr = cifCorridorPtr->GetCorridorAlignment();
        if (cifAlignmentPtr.IsNull())
            continue;

        auto cifAlgElmRefP = cifAlignmentPtr->GetElementHandle()->GetElementRef();
        if (m_cifAlignmentToBimID.find(cifAlgElmRefP) != m_cifAlignmentToBimID.end())
            continue;

        auto bimAlignmentId = alignmentsConverterPtr->ConvertAlignment(*cifAlignmentPtr, *m_ordParams, true);
        m_cifAlignmentToBimID.insert({cifAlignmentPtr->GetElementHandle()->GetElementRef(), bimAlignmentId});
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::Create3dLinears()
    {
    if (m_cifAlignments.empty())
        return;

    // There may be duplicate CifAlignments found via various V8 models. Some of them will have
    // the same ElementRef, but others will not, so relying on SyncId to tell whether two
    // CifAlignments are the same. Also, in some cases, some duplicate CifAlignments have
    // a blank name, so leaving the one with a name in front during sorting - that will be the
    // one processed later - the other duplicates will be ignored.
    std::sort(m_cifAlignments.begin(), m_cifAlignments.end(),
              [] (Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Alignment> const& a, Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Alignment> const& b)
              {
              auto aSyncId = a->GetSyncId();
              auto bSyncId = b->GetSyncId();
              auto equal = aSyncId.CompareTo(bSyncId);

              if (0 == equal)
                  {
                  auto aCifName = a->GetName();
                  auto bCifName = b->GetName();
                  equal = aCifName.CompareTo(bCifName);
                  }

              return (0 == equal) ? true : (0 < equal);
              });

    Bentley::WString lastSyncId;
    Bentley::ElementRefP lastAlignmentRefP = nullptr;
    RoadRailAlignment::AlignmentCPtr bimAlignmentCPtr;
    bmap<DgnCategoryId, DgnCategoryId> drawingToSpatialCategoryMap;

    auto alignmentsConverterPtr = ORDAlignmentsConverter::Create(*this);
    for (auto& cifAlignmentPtr : m_cifAlignments)
        {
        auto cifAlgElmRefP = cifAlignmentPtr->GetElementHandle()->GetElementRef();
        if (lastAlignmentRefP == cifAlgElmRefP)
            continue;

        if (0 == lastSyncId.CompareTo(cifAlignmentPtr->GetSyncId()))
            continue;

        lastAlignmentRefP = cifAlgElmRefP;
        if (m_cifAlignmentToBimID.find(cifAlgElmRefP) != m_cifAlignmentToBimID.end())
            continue;

        auto bimAlignmentId = alignmentsConverterPtr->ConvertAlignment(*cifAlignmentPtr, *m_ordParams, false);
        lastSyncId = cifAlignmentPtr->GetSyncId();

        m_cifAlignmentToBimID.insert({cifAlgElmRefP, bimAlignmentId});
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::CreatePathways()
    {
    if (m_cifCorridors.empty())
        return;

    std::sort(m_cifCorridors.begin(), m_cifCorridors.end(),
              [] (Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Corridor> const& a, Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Corridor> const& b)
              {
              return a->GetElementHandle()->GetElementRef() > b->GetElementHandle()->GetElementRef();
              });

    Bentley::DgnPlatform::ModelId modelIdForConverter = 0;
    ORDCorridorsConverterPtr corridorsConverterPtr;

    Bentley::ElementRefP lastCorridorRefP = nullptr;
    RoadRailBim::CorridorCPtr corridorElmCPtr;
    bmap<DgnCategoryId, DgnCategoryId> drawingToSpatialCategoryMap;

    for (auto& cifCorridorPtr : m_cifCorridors)
        {
        if (corridorsConverterPtr.IsNull() || modelIdForConverter != cifCorridorPtr->GetDgnModelP()->GetModelId())
            {
            corridorsConverterPtr = ORDCorridorsConverter::Create(*this, ComputeUnitsScaleTransform(*cifCorridorPtr->GetDgnModelP()));
            modelIdForConverter = cifCorridorPtr->GetDgnModelP()->GetModelId();
            }

        if (lastCorridorRefP == cifCorridorPtr->GetElementHandle()->GetElementRef())
            continue;

        DgnElementPtr bimElmPtr;
        auto v8Iter = m_v8ToBimElmMap.find(cifCorridorPtr->GetElementHandle()->GetElementRef());
        if (v8Iter != m_v8ToBimElmMap.end())
            bimElmPtr = v8Iter->second;

        auto bimCorridorId = corridorsConverterPtr->ConvertCorridor(*cifCorridorPtr, *m_ordParams);
        lastCorridorRefP = cifCorridorPtr->GetElementHandle()->GetElementRef();
        corridorElmCPtr = RoadRailPhysical::Corridor::Get(GetDgnDb(), bimCorridorId);

        if (corridorElmCPtr.IsValid() && bimElmPtr.IsValid())
            {
            if (auto bimGeomSourceCP = bimElmPtr->ToGeometrySource())
                {
                if (!corridorElmCPtr->QueryIsRepresentedBy(*bimGeomSourceCP))
                    RoadRailPhysical::Corridor::AddRepresentedBy(*corridorElmCPtr, *bimGeomSourceCP);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::AssociateGeneratedAlignments()
    {
    for (auto& cifGenLine3d : m_cifGeneratedLinear3ds)
        {
        AlignmentBim::AlignmentCPtr bimAlignmentCPtr;
        auto cifAlignmentPtr = cifGenLine3d->GetAlignment();

        ORDAlignmentsConverter::CifAlignmentSourceItem alignmentItem(*cifAlignmentPtr);
        iModelExternalSourceAspect::ElementAndAspectId elementAndAspectId =
            iModelExternalSourceAspect::FindElementBySourceId(GetDgnDb(), DgnElementId(m_ordParams->fileScopeId), alignmentItem.Kind(), alignmentItem._GetId());
        if (elementAndAspectId.elementId.IsValid())
            {
            bimAlignmentCPtr = AlignmentBim::Alignment::Get(GetDgnDb(), elementAndAspectId.elementId);
            if (bimAlignmentCPtr.IsNull())
                continue;
            }

        if (bimAlignmentCPtr.IsNull() || bimAlignmentCPtr->GetSource().IsValid())
            continue;

        RoadRailBim::CorridorCPtr corridorCPtr;
        auto cifCorridorPtr = cifGenLine3d->GetCorridor();
        if (cifCorridorPtr.IsNull())
            continue;

        ORDCorridorsConverter::CifCorridorSourceItem corridorItem(*cifCorridorPtr);

        elementAndAspectId =
            iModelExternalSourceAspect::FindElementBySourceId(GetDgnDb(), DgnElementId(m_ordParams->fileScopeId), corridorItem.Kind(), corridorItem._GetId());
        if (elementAndAspectId.aspectId.IsValid() && elementAndAspectId.elementId.IsValid())
            {
            corridorCPtr = RoadRailBim::Corridor::Get(GetDgnDb(), elementAndAspectId.elementId);
            if (corridorCPtr.IsNull())
                continue;
            }

        if (corridorCPtr.IsValid() && bimAlignmentCPtr.IsValid())
            {
            auto corridorSegmentCPtr = RoadRailBim::TransportationSystem::Query(*corridorCPtr, DefaultTransportationSystemName);
            if (corridorSegmentCPtr.IsNull())
                continue;

            auto pathwayIds = corridorSegmentCPtr->QueryPathwayIds();
            if (pathwayIds.empty())
                continue;

            auto pathwayId = *pathwayIds.begin();
            if (!pathwayId.IsValid())
                continue;

            auto pathwayCPtr = RoadRailBim::PathwayElement::Get(GetDgnDb(), pathwayId);
            if (pathwayCPtr.IsValid())
                {
                auto bimAlignmentPtr = AlignmentBim::Alignment::GetForEdit(bimAlignmentCPtr->GetDgnDb(), bimAlignmentCPtr->GetElementId());
                bimAlignmentPtr->getP()->SetParentId(pathwayCPtr->GetElementId(),
                                             pathwayCPtr->GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_CorridorPortionOwnsAlignments));
                bimAlignmentPtr->SetSource(pathwayCPtr.get());
                bimAlignmentPtr->Update();
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::CreateRoadRailElements()
    {
    BentleyApi::StopWatch stopWatch("CreateRoadRailElements", true);
    GetProgressMeter().SetCurrentStepName(ORDBridgeProgressMessage::GetString(ORDBridgeProgressMessage::STEP_ALIGN_DATA()).c_str());

    GetProgressMeter().SetCurrentTaskName(ORDBridgeProgressMessage::GetString(ORDBridgeProgressMessage::TASK_CREATE_DESIGNALIGNMENTS()).c_str());
    CreateDesignAlignments();

    GetProgressMeter().SetCurrentTaskName(ORDBridgeProgressMessage::GetString(ORDBridgeProgressMessage::TASK_CREATE_CORRIDORS()).c_str());
    CreatePathways();

    GetProgressMeter().SetCurrentTaskName(ORDBridgeProgressMessage::GetString(ORDBridgeProgressMessage::TASK_CREATE_3DLINEARS()).c_str());
    Create3dLinears();

    GetProgressMeter().SetCurrentTaskName(ORDBridgeProgressMessage::GetString(ORDBridgeProgressMessage::TASK_ASSOCIATE_3DLINEARS()).c_str());
    AssociateGeneratedAlignments();

    auto roadNetworkModelPtr = GetRoadNetwork()->GetNetworkModel();
    auto designAlignmentsCPtr = AlignmentBim::DesignAlignments::Query(*roadNetworkModelPtr, DefaultDesignAlignmentsName);
    auto designAlignmentModelPtr = designAlignmentsCPtr->GetAlignmentModel();
    auto designHorizontalAlignmentsCPtr = AlignmentBim::HorizontalAlignments::Query(*designAlignmentModelPtr);
    auto designHorizAlignmentModelPtr = designHorizontalAlignmentsCPtr->GetHorizontalModel();

    updateProjectExtents(*designHorizAlignmentModelPtr, *m_ordParams, false);
    updateProjectExtents(*roadNetworkModelPtr, *m_ordParams, true);

    stopWatch.Stop();
    ORDBRIDGEPERF_LOGI("%s", s_preConvertStopWatch->GetLogMessage());
    ORDBRIDGEPERF_LOGI("%s", s_determineElemParamsStopWatch->GetLogMessage());
    ORDBRIDGEPERF_LOGI("%s", s_assignAlignmentAspectStopWatch->GetLogMessage());
    ORDBRIDGEPERF_LOGI("%s", s_assignCorridorAspectStopWatch->GetLogMessage());
    ORDBRIDGEPERF_LOGI("%s", s_assignCorridorSurfaceAspectStopWatch->GetLogMessage());
    ORDBRIDGEPERF_LOGI("%s", s_assignLinear3dAspectStopWatch->GetLogMessage());
    ORDBRIDGEPERF_LOGI("%s", s_assignSuperelevationAspectStopWatch->GetLogMessage());
    ORDBRIDGEPERF_LOGI("%s", s_assignTemplateDropAspectStopWatch->GetLogMessage());
    ORDBRIDGEPERF_LOGI("%s", s_processResultsStopWatch->GetLogMessage());
    ORDBRIDGEPERF_LOGI("%s", s_convertProfilesStopWatch->GetLogMessage());
    ORDBRIDGEPERF_LOGI("%s: %.2f", stopWatch.GetDescription(), stopWatch.GetElapsedSeconds());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
CategorySelectorPtr createDefaultSpatialCategorySelector(DefinitionModelR model)
    {
    Utf8String selectorName = "ORDBridge";
    auto selectorId = model.GetDgnDb().Elements().QueryElementIdByCode(CategorySelector::CreateCode(model, selectorName));
    auto selectorPtr = model.GetDgnDb().Elements().GetForEdit<CategorySelector>(selectorId);
    if (selectorPtr.IsValid())
        return selectorPtr;

    selectorPtr = new CategorySelector(model, selectorName);
    auto iter = SpatialCategory::MakeIterator(model.GetDgnDb());
    for (auto const& category : iter)
        selectorPtr->AddCategory(DgnCategoryId(category.GetElementId().GetValue()));

    return selectorPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
DisplayStyle3dPtr createDefaultDisplayStyle3d(DefinitionModelR model)
    {
    Utf8String styleName = "ORDBridge";
    auto styleId = model.GetDgnDb().Elements().QueryElementIdByCode(DisplayStyle3d::CreateCode(model, styleName));
    auto stylePtr = model.GetDgnDb().Elements().GetForEdit<DisplayStyle3d>(styleId);
    if (stylePtr.IsValid())
        return stylePtr;

    stylePtr = new DisplayStyle3d(model, styleName);
    stylePtr->SetBackgroundColor(ColorDef::Black());
    stylePtr->SetSkyBoxEnabled(false);
    stylePtr->SetGroundPlaneEnabled(false);

    Render::ViewFlags viewFlags = stylePtr->GetViewFlags();
    viewFlags.SetRenderMode(Render::RenderMode::SmoothShade);
    viewFlags.SetMonochrome(false);
    viewFlags.SetShowFill(true);
    viewFlags.SetShowMaterials(true);
    viewFlags.SetShowPatterns(true);
    viewFlags.SetShowShadows(true);
    stylePtr->SetViewFlags(viewFlags);

    return stylePtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   BentleySystems
//---------------------------------------------------------------------------------------
ModelSelectorPtr createDefaultModelSelector(DefinitionModelR definitionModel, Utf8StringCR name, bset<DgnModelId> const& modelIds)
    {
    ModelSelectorPtr modelSelectorPtr = new ModelSelector(definitionModel, name);

    for (auto modelId : modelIds)
        modelSelectorPtr->AddModel(modelId);

    return modelSelectorPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
DgnViewId create3dView(DefinitionModelR model, Utf8StringCR viewName,
    CategorySelectorR categorySelector, ModelSelectorR modelSelector, DisplayStyle3dR displayStyle)
    {
    DgnDbR db = model.GetDgnDb();

    SpatialViewDefinition view(model, viewName, categorySelector, displayStyle, modelSelector);

    DgnViewId viewId;
    DgnViewId existingViewId = ViewDefinition::QueryViewId(db, view.GetCode());
    if (existingViewId.IsValid())
        viewId = existingViewId;
    else
        {
        view.SetStandardViewRotation(Dgn::StandardView::Top);
        view.LookAtVolume(db.GeoLocation().GetProjectExtents());

        if (!view.Insert().IsValid())
            return viewId;

        viewId = view.GetViewId();
        }

    if (!viewId.IsValid())
        return viewId;

    return viewId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ORDConverter::GetAlignedSubjectName() const
    {
    return Utf8PrintfString("%s <Aligned>", GetDgnDb().Elements().GetRootSubject()->GetCode().GetValueUtf8CP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr ORDConverter::GetAlignedSubject() const
    {
    auto alignedSubjectId = GetDgnDb().Elements().QueryElementIdByCode(Subject::CreateCode(GetJobSubject(), GetAlignedSubjectName()));
    return GetDgnDb().Elements().Get<Subject>(alignedSubjectId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::CreateDefaultSavedViews()
    {
    auto alignedSubjectCPtr = GetAlignedSubject();
    auto configurationModelPtr = AlignmentBim::RoadRailAlignmentDomain::QueryConfigurationModel(*alignedSubjectCPtr);
    auto displayStyle3dPtr = createDefaultDisplayStyle3d(*configurationModelPtr);
    auto categorySelectorPtr = createDefaultSpatialCategorySelector(*configurationModelPtr);

    if (!m_planViewModels.empty())
        {
        auto planViewModelSelectorPtr = createDefaultModelSelector(*configurationModelPtr, "ORDBridge-PlanView", m_planViewModels);
        auto viewName = ORDBridgeElementCodes::GetString(ORDBridgeElementCodes::VIEW_PLANVIEW_MODELS());
        create3dView(*configurationModelPtr, viewName, *categorySelectorPtr, *planViewModelSelectorPtr, *displayStyle3dPtr);
        }

    if (!m_3dModels.empty())
        {
        auto threeDModelSelectorPtr = createDefaultModelSelector(*configurationModelPtr, "ORDBridge-3D", m_3dModels);
        auto viewName = ORDBridgeElementCodes::GetString(ORDBridgeElementCodes::VIEW_3D_MODELS());
        create3dView(*configurationModelPtr, viewName, *categorySelectorPtr, *threeDModelSelectorPtr, *displayStyle3dPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::_OnConversionComplete()
    {
    if (m_isProcessing)
        {
        CreateRoadRailElements();

        for (auto pExt : ORDConverterExtensionRegistry::s_extensions)
            pExt->OnConversionComplete(*this);

        if (!IsUpdating())
            {
            ScalableMeshWrapper::AddTerrainClassifiers(GetDgnDb(), m_clippingsModelId);
            CreateDefaultSavedViews();
            }
        }

    T_Super::_OnConversionComplete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::_ConvertModels()
    {
    T_Super::_ConvertModels();
    //load backpointers
    Bentley::Cif::SDK::PersistentPathBackPointerBuilder::GetBuilder().BuildBackPointers();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ORDConverter::_ShouldImportSchema(Utf8StringCR fullSchemaName, DgnV8ModelR v8Model)
    {
    if (fullSchemaName.Contains("Bentley_Civil") || fullSchemaName.StartsWith("Civil."))
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
static Dgn::UnitDefinition fromV8(DgnV8Api::UnitDefinition const& v8Def)
    {
    return Dgn::UnitDefinition(Dgn::UnitBase(v8Def.GetBase()), Dgn::UnitSystem(v8Def.GetSystem()), v8Def.GetNumerator(), v8Def.GetDenominator(), Utf8String(v8Def.GetLabelCP()).c_str());
    }

Dgn::DgnElementId ORDConverter::GetBimElementFor(Bentley::ElementRefP elementRefP) const
    {
    auto iterator = m_v8ToBimElmMap.find(elementRefP);
    if (iterator == m_v8ToBimElmMap.end())
        {
        return Dgn::DgnElementId();
        }
    return iterator->second->GetElementId();
    }

Dgn::DgnViewId ORDConverter::GetDefaultViewId()
    {
    return m_defaultViewId;
    }

void ORDConverter::InsertIntoV8ToBimElmMap(Bentley::ElementRefP v8ElementRefP, Dgn::DgnElementPtr bimElementPtr)
    {
    m_v8ToBimElmMap.Insert(v8ElementRefP, bimElementPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void setUpModelFormatter(Dgn::GeometricModelR geometricModel, DgnV8Api::ModelInfo const& v8ModelInfo)
    {
    auto& displayInfo = geometricModel.GetFormatterR();

    displayInfo.SetUnits(fromV8(v8ModelInfo.GetMasterUnit()), fromV8(v8ModelInfo.GetSubUnit()));
    displayInfo.SetRoundoffUnit(v8ModelInfo.GetRoundoffUnit(), v8ModelInfo.GetRoundoffRatio());
    displayInfo.SetLinearUnitMode(Dgn::DgnUnitFormat(v8ModelInfo.GetLinearUnitMode()));
    displayInfo.SetLinearPrecision(PrecisionFormat(v8ModelInfo.GetLinearPrecision()));
    displayInfo.SetAngularMode(Dgn::AngleMode(v8ModelInfo.GetAngularMode()));
    displayInfo.SetAngularPrecision(Dgn::AnglePrecision(v8ModelInfo.GetAngularPrecision()));
    displayInfo.SetDirectionMode(Dgn::DirectionMode(v8ModelInfo.GetDirectionMode()));
    displayInfo.SetDirectionClockwise(v8ModelInfo.GetDirectionClockwise());
    displayInfo.SetDirectionBaseDir(v8ModelInfo.GetDirectionBaseDir());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    diego.diaz                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::UnitSystem ORDConverter::GetRootModelUnitSystem()
    {
    DgnV8Api::ModelInfo const& v8ModelInfo = _GetModelInfo(*GetRootModelP());
    return (Dgn::UnitSystem)v8ModelInfo.GetMasterUnit().GetSystem();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::SetUpModelFormatters()
    {
    auto roadNetworkModelPtr = GetRoadNetwork()->GetNetworkModel();
    auto designAlignmentsCPtr = AlignmentBim::DesignAlignments::Query(*roadNetworkModelPtr, DefaultDesignAlignmentsName);
    auto designAlignmentModelPtr = designAlignmentsCPtr->GetAlignmentModel();

    DgnV8Api::ModelInfo const& v8ModelInfo = _GetModelInfo(*GetRootModelP());

    setUpModelFormatter(*designAlignmentModelPtr, v8ModelInfo);
    setUpModelFormatter(*roadNetworkModelPtr, v8ModelInfo);

    designAlignmentModelPtr->Update();
    roadNetworkModelPtr->Update();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::_OnSheetsConvertViewAttachment(Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const& v8SheetModelMapping, DgnAttachmentR v8DgnAttachment)
    {
    if (v8DgnAttachment.IsAssociatedToNamedView())
        {
        EditElementHandle namedViewEeh;
        if (v8DgnAttachment.GetNamedViewElement(namedViewEeh))
            {
            auto namedViewPtr = DgnV8Api::NamedView::Create(namedViewEeh);

            EditElementHandle clipEeh;
            if (SUCCESS == namedViewPtr->GetClipElement(clipEeh))
                {
                DgnElementId bimClipElmId;
                if (TryFindElement(bimClipElmId, clipEeh))
                    {
                    auto bimClipElmCPtr = v8SheetModelMapping.GetDgnModel().GetDgnDb().Elements().GetElement(bimClipElmId);
                    if (bimClipElmCPtr->IsGeometricElement())
                        {
                        auto bimSheetElmCPtr = v8SheetModelMapping.GetDgnModel().GetModeledElement();

                        DgnV8ORDBim::DgnV8OpenRoadsDesignerDomain::SetGeometricElementAsBoundingContentForSheet(
                            *dynamic_cast<GeometricElementCP>(bimClipElmCPtr.get()),
                            *dynamic_cast<Sheet::ElementCP>(bimSheetElmCPtr.get()));
                        }
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDConverter::AddDynamicSchema()
    {
    auto cifConsensusConnectionPtr = ConsensusConnection::Create(*GetRootModelRefP());

    ORDDynamicSchemaGenerator dynSchemaGen;
    return dynSchemaGen.Generate(GetDgnDb(), GetParams().GetAssetsDir(), *cifConsensusConnectionPtr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ORDConverter::GetExtensionCount() const
    {
    return ORDConverterExtensionRegistry::s_extensions.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDConverter::AddExtensionSchema(bool& hasMoreChanges)
    {
    if (!m_iterValid) {
        m_makeSchemaChangeExtIter = ORDConverterExtensionRegistry::s_extensions.begin();
        m_iterValid = true;
    }

    BeAssert(m_makeSchemaChangeExtIter != ORDConverterExtensionRegistry::s_extensions.end());

    BentleyStatus retVal = (*m_makeSchemaChangeExtIter)->MakeSchemaChanges();
    hasMoreChanges = (++m_makeSchemaChangeExtIter != ORDConverterExtensionRegistry::s_extensions.end());
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDConverter::MakeRoadRailSchemaChanges()
    {
    auto schemaReadContextPtr = ECN::ECSchemaReadContext::CreateContext(false, true);

    auto assetsDir = GetParams().GetAssetsDir();
    BeFileName ecdbDir = assetsDir;
    ecdbDir.AppendToPath(L"ECSchemas");
    ecdbDir.AppendToPath(L"ECDb");

    BeFileName dgnDir = assetsDir;
    dgnDir.AppendToPath(L"ECSchemas");
    dgnDir.AppendToPath(L"Dgn");

    BeFileName domainDir = assetsDir;
    domainDir.AppendToPath(L"ECSchemas");
    domainDir.AppendToPath(L"Domain");

    schemaReadContextPtr->AddSchemaPath(ecdbDir);
    schemaReadContextPtr->AddSchemaPath(dgnDir);
    schemaReadContextPtr->AddSchemaPath(domainDir);

    BeFileName lrSchemaPath = assetsDir;
    lrSchemaPath.AppendToPath(LinearReferencingDomain::GetSchemaRelativePath());

    ECN::ECSchemaPtr lrSchemaPtr;
    if (ECN::SchemaReadStatus::Success != ECN::ECSchema::ReadFromXmlFile(lrSchemaPtr, lrSchemaPath.c_str(), *schemaReadContextPtr))
        return BentleyStatus::ERROR;

    BeFileName rraSchemaPath = assetsDir;
    rraSchemaPath.AppendToPath(RoadRailAlignment::RoadRailAlignmentDomain::GetSchemaRelativePath());

    ECN::ECSchemaPtr rraSchemaPtr;
    if (ECN::SchemaReadStatus::Success != ECN::ECSchema::ReadFromXmlFile(rraSchemaPtr, rraSchemaPath.c_str(), *schemaReadContextPtr))
        return BentleyStatus::ERROR;

    BeFileName rrpSchemaPath = assetsDir;
    rrpSchemaPath.AppendToPath(RoadRailPhysical::RoadRailPhysicalDomain::GetSchemaRelativePath());

    ECN::ECSchemaPtr rrpSchemaPtr;
    if (ECN::SchemaReadStatus::Success != ECN::ECSchema::ReadFromXmlFile(rrpSchemaPtr, rrpSchemaPath.c_str(), *schemaReadContextPtr))
        return BentleyStatus::ERROR;

    BeFileName rdpSchemaPath = assetsDir;
    rdpSchemaPath.AppendToPath(RoadPhysical::RoadPhysicalDomain::GetSchemaRelativePath());

    ECN::ECSchemaPtr rdpSchemaPtr;
    if (ECN::SchemaReadStatus::Success != ECN::ECSchema::ReadFromXmlFile(rdpSchemaPtr, rdpSchemaPath.c_str(), *schemaReadContextPtr))
        return BentleyStatus::ERROR;

    BeFileName rlpSchemaPath = assetsDir;
    rlpSchemaPath.AppendToPath(RailPhysical::RailPhysicalDomain::GetSchemaRelativePath());

    ECN::ECSchemaPtr rlpSchemaPtr;
    if (ECN::SchemaReadStatus::Success != ECN::ECSchema::ReadFromXmlFile(rlpSchemaPtr, rlpSchemaPath.c_str(), *schemaReadContextPtr))
        return BentleyStatus::ERROR;

    BeFileName bspSchemaPath = assetsDir;
    bspSchemaPath.AppendToPath(BridgeStructuralPhysical::BridgeStructuralPhysicalDomain::GetSchemaRelativePath());

    ECN::ECSchemaPtr bspSchemaPtr;
    if (ECN::SchemaReadStatus::Success != ECN::ECSchema::ReadFromXmlFile(bspSchemaPtr, bspSchemaPath.c_str(), *schemaReadContextPtr))
        return BentleyStatus::ERROR;

    bvector<ECN::ECSchemaCP> schemas;
    schemas.push_back(lrSchemaPtr.get());
    schemas.push_back(rraSchemaPtr.get());
    schemas.push_back(rrpSchemaPtr.get());
    schemas.push_back(rdpSchemaPtr.get());
    schemas.push_back(rlpSchemaPtr.get());
    schemas.push_back(bspSchemaPtr.get());

    if (SchemaStatus::Success != GetDgnDb().ImportSchemas(schemas))
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId ORDConverter::_MapModelIntoProject(DgnV8ModelR v8Model, Bentley::Utf8CP newName, DgnV8Api::DgnAttachment const* attachment)
    {
    bool isPlanarModel = false;
    auto modelClassId = _ComputeModelType(v8Model);
    if (GetDgnDb().Schemas().GetClass(modelClassId)->Is(GetDgnDb().Schemas().GetClass(BIS_ECSCHEMA_NAME, "GraphicalModel3d")))
        {
        if (!v8Model.Is3D())
            isPlanarModel = true;
        }

    auto newModelId = CreateModelFromV8Model(v8Model, newName, modelClassId, attachment);
    if (isPlanarModel)
        {
        auto newModelPtr = GetDgnDb().Models().Get<Dgn::GeometricModel3d>(newModelId);
        newModelPtr->SetIsPlanProjection(true);
        newModelPtr->Update();
        }

    return newModelId;
    }

END_ORDBRIDGE_NAMESPACE
