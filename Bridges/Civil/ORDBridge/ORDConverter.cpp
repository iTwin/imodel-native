/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDConverter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ORDBridgeInternal.h>
#include <VersionedDgnV8Api/GeomSerialization/GeomLibsFlatBufferApi.h>

BEGIN_ORDBRIDGE_NAMESPACE

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
    static void AssignFeatureDefinition(DgnElementR bimElement, Bentley::WStringCR featureDefinitionName);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverterUtils::AssignFeatureDefinition(DgnElementR bimElement, Bentley::WStringCR featureDefinitionName)
    {
    ECN::AdHocJsonValue userProps = bimElement.GetUserProperties("OpenRoadsDesigner");
    userProps.SetValueText("FeatureDefinitionName", Utf8String(featureDefinitionName.c_str()).c_str());
    bimElement.SetUserProperties("OpenRoadsDesigner", userProps);
    }

DEFINE_POINTER_SUFFIX_TYPEDEFS(ORDAlignmentsConverter)
DEFINE_REF_COUNTED_PTR(ORDAlignmentsConverter)

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ORDAlignmentsConverter: RefCountedBase
{
    struct CifAlignmentSourceItem : ConsensusSourceItem
    {
    DEFINE_T_SUPER(ConsensusSourceItem)

    public:
        CifAlignmentSourceItem(AlignmentCR alignment): T_Super(alignment) {}

        static Utf8CP Kind() { return "Alignment"; }

    protected:
        Utf8String _GetHash() override
            {
            auto alignmentCP = dynamic_cast<AlignmentCP>(m_entity);
            auto horizCurveVectorPtr = alignmentCP->GetGeometry();

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
        CifProfileSourceItem(ProfileCR profile): T_Super(profile) {}

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
    AlignmentBim::AlignmentModelPtr m_bimAlignmentModelPtr;

private:
    ORDAlignmentsConverter(SubjectCR jobSubject);
    ORDAlignmentsConverter(DgnDbSync::DgnV8::ConverterLibrary& converterLib);

    BentleyStatus Marshal(CurveVectorPtr& bimCurveVector, Bentley::CurveVectorCR v8CurveVector);
    BentleyStatus MarshalVertical(CurveVectorPtr& bimCurveVector, Bentley::CurveVectorCR v8CurveVector);
    BentleyStatus CreateNewBimVerticalAlignment(ProfileCR, AlignmentBim::AlignmentCR alignment,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change,
        AlignmentBim::VerticalAlignmentCPtr& verticalAlignment);
    BentleyStatus CreateNewBimAlignment(AlignmentCR cifAlignment, 
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change, 
        AlignmentBim::AlignmentCPtr& bimAlignment, DgnCategoryId targetCategoryId);
    BentleyStatus UpdateBimAlignment(AlignmentCR cifAlignment,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change, DgnCategoryId targetCategoryId);
    BentleyStatus UpdateBimVerticalAlignment(ProfileCR,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change);
    BentleyStatus ConvertProfiles(AlignmentCR cifAlignment, AlignmentBim::AlignmentCR alignment, ORDConverter::Params& params);

public:
    static ORDAlignmentsConverterPtr Create(SubjectCR jobSubject)
        {
        return new ORDAlignmentsConverter(jobSubject);
        }

    AlignmentBim::AlignmentModelR GetAlignmentModel() const { return *m_bimAlignmentModelPtr; }
    DgnElementId ConvertAlignment(AlignmentCR cifAlignment, ORDConverter::Params& params, DgnCategoryId targetCategory = DgnCategoryId());
}; // ORDAlignmentsConverter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ORDAlignmentsConverter::ORDAlignmentsConverter(SubjectCR jobSubject)
    {
    m_bimAlignmentModelPtr = AlignmentBim::AlignmentModel::Query(jobSubject, ORDBRIDGE_AlignmentModelName);
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

    Transform flipAxes = Transform::FromOriginAndVectors(DPoint3d::FromZero(), DVec3d::UnitX(), DVec3d::UnitZ(), DVec3d::UnitY());
    bimCurveVectorXZPtr->TransformInPlace(flipAxes);

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDAlignmentsConverter::CreateNewBimAlignment(AlignmentCR cifAlignment, 
    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change,
    AlignmentBim::AlignmentCPtr& bimAlignment, DgnCategoryId targetCategoryId)
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
    
    DgnCode bimCode;
    
    // Identifying alignments appearing more than once in a single bridge-run.
    // For unnamed alignments, SyncId is used to differentiate them.
    // For named alignments, if a name (code) is already used, differentiate them with full file-name + modelname.
    if (Utf8String::IsNullOrEmpty(cifAlignmentName.c_str()))
        {        
        auto existingCPtr = m_bimAlignmentModelPtr->GetDgnDb().Elements().QueryElementByFederationGuid(bimGuid);
        if (existingCPtr.IsValid())
            return BentleyStatus::ERROR;
        }
    else
        {
        Utf8String bimAlignmentName = cifAlignmentName;
        bimCode = AlignmentBim::RoadRailAlignmentDomain::CreateCode(*m_bimAlignmentModelPtr, bimAlignmentName);

        int32_t suffix = 0;
        auto existingId = m_bimAlignmentModelPtr->GetDgnDb().Elements().QueryElementIdByCode(bimCode);
        while (existingId.IsValid())
            {
            auto existingAlgCPtr = AlignmentBim::Alignment::Get(m_bimAlignmentModelPtr->GetDgnDb(), existingId);

            if (existingAlgCPtr.IsValid() && existingAlgCPtr->GetFederationGuid() == bimGuid)
                return BentleyStatus::ERROR;
            else
                {
                BeFileName fileName(cifAlignment.GetDgnModelP()->GetDgnFileP()->GetFileName().c_str());
                bimAlignmentName = Utf8String(fileName.GetFileNameAndExtension().c_str());
                bimAlignmentName += "\\";
                bimAlignmentName += Utf8String(cifAlignment.GetDgnModelP()->GetModelName());
                bimAlignmentName += "\\";
                bimAlignmentName += cifAlignmentName;

                if (suffix > 0)
                    bimAlignmentName += Utf8PrintfString("-%d", suffix).c_str();

                bimCode = AlignmentBim::RoadRailAlignmentDomain::CreateCode(*m_bimAlignmentModelPtr, bimAlignmentName);
                suffix++;
                }

            existingId = m_bimAlignmentModelPtr->GetDgnDb().Elements().QueryElementIdByCode(bimCode);
            }
        }    

    // Create Alignment
    auto bimAlignmentPtr = AlignmentBim::Alignment::Create(*m_bimAlignmentModelPtr);
    if (targetCategoryId.IsValid())
        bimAlignmentPtr->SetCategoryId(targetCategoryId);
    
    if (bimCode.IsValid())
        bimAlignmentPtr->SetCode(bimCode);

    // By default, set the user label to name of the Alignment.  This corresponds to the "Feature Name" property in ORD.
    Utf8String userLabel(cifAlignment.GetName().c_str());

    // Handling the user label if it's unset in ORD is still not well defined.  
    // We're following the guidelines here: http://builds.bentley.com/prgbuilds/AzureBuilds/BISDocs/latest/public/introduction-to-bis/imodel-bridges/
    // but the final decision for how we auto-create user labels has not been made yet.
    if (Utf8String::IsNullOrEmpty(userLabel.c_str()))
        {
        if (bimCode.IsValid())
            {
            userLabel = bimCode.GetValueUtf8();
            }
        else
            {
            userLabel += Utf8String(cifAlignment.GetClassName());
            }
        }
    
    bimAlignmentPtr->SetUserLabel(userLabel.c_str());
    
    Bentley::WString featureDefName;
    if (cifAlignment.GetFeatureDefinition().IsValid())
        {
        featureDefName = cifAlignment.GetFeatureDefinition()->GetName();
        if (!Bentley::WString::IsNullOrEmpty(featureDefName.c_str()))
            ORDConverterUtils::AssignFeatureDefinition(*bimAlignmentPtr, featureDefName);
        }

    ORDConverterUtils::AssignFederationGuid(*bimAlignmentPtr, cifSyncId);

    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = bimAlignmentPtr;
    if (BentleyStatus::SUCCESS != changeDetector._UpdateBimAndSyncInfo(results, change)) 
        return BentleyStatus::ERROR;

    bimAlignment = bimAlignmentPtr.get();

    CurveVectorPtr bimHorizGeometryPtr;
    if (BentleyStatus::SUCCESS != Marshal(bimHorizGeometryPtr, *horizGeometryPtr))
        return BentleyStatus::ERROR;

    // Create Horizontal Alignment
    auto bimHorizAlignmPtr = AlignmentBim::HorizontalAlignment::Create(*bimAlignmentPtr, *bimHorizGeometryPtr);
    if (targetCategoryId.IsValid())
        bimHorizAlignmPtr->SetCategoryId(targetCategoryId);

    if (bimCode.IsValid())
        bimHorizAlignmPtr->SetCode(AlignmentBim::RoadRailAlignmentDomain::CreateCode(*bimHorizAlignmPtr->GetModel(), bimCode.GetValueUtf8()));

    if (!Bentley::WString::IsNullOrEmpty(featureDefName.c_str()))
        ORDConverterUtils::AssignFeatureDefinition(*bimHorizAlignmPtr, featureDefName);

    bimHorizAlignmPtr->GenerateElementGeom();
    if (bimHorizAlignmPtr->Insert().IsNull())
        return BentleyStatus::ERROR;

    auto cifStationingPtr = cifAlignment.GetStationing();
    if (cifStationingPtr.IsValid())
        {
        bimAlignmentPtr->SetStartStation(cifStationingPtr->GetStartStation());
        bimAlignmentPtr->SetStartValue(cifStationingPtr->GetDistanceAlong());
        }

    if (bimAlignmentPtr->Update().IsNull())
        return BentleyStatus::ERROR;

    if (cifStationingPtr.IsValid())
        {
        auto stationEquationsPtr = cifStationingPtr->GetStationEquations();
        while (stationEquationsPtr.IsValid() && stationEquationsPtr->MoveNext())
            {
            auto stationEqPtr = stationEquationsPtr->GetCurrent();
            if (stationEqPtr.IsNull())
                continue;

            auto bimStationPtr = AlignmentBim::AlignmentStation::Create(*bimAlignmentPtr, stationEqPtr->GetDistanceAlong(), stationEqPtr->GetEquivalentStation());
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
        return BentleyStatus::ERROR;

    CurveVectorPtr bimVertGeometryPtr;
    if (BentleyStatus::SUCCESS != MarshalVertical(bimVertGeometryPtr, *vertGeometryPtr))
        return BentleyStatus::ERROR;
    
    auto verticalModelId = alignment.QueryVerticalAlignmentSubModelId();    
    if (!verticalModelId.IsValid())
        {
        auto verticalModelPtr = AlignmentBim::VerticalAlignmentModel::Create(
            AlignmentBim::VerticalAlignmentModel::CreateParams(m_bimAlignmentModelPtr->GetDgnDb(), alignment.GetElementId()));
        if (DgnDbStatus::Success != verticalModelPtr->Insert())
            return BentleyStatus::ERROR;

        verticalModelId = verticalModelPtr->GetModelId();
        }

    auto verticalModelCPtr = AlignmentBim::VerticalAlignmentModel::Get(alignment.GetDgnDb(), verticalModelId);;
    auto verticalAlignmPtr = AlignmentBim::VerticalAlignment::Create(*verticalModelCPtr, *bimVertGeometryPtr);

    if (cifProfile.GetFeatureDefinition().IsValid())
        {
        auto featureDefName = cifProfile.GetFeatureDefinition()->GetName();
        if (!Bentley::WString::IsNullOrEmpty(featureDefName.c_str()))
            ORDConverterUtils::AssignFeatureDefinition(*verticalAlignmPtr, featureDefName);
        }

    verticalAlignmPtr->GenerateElementGeom();

    iModelBridgeSyncInfoFile::ConversionResults verticalResults;
    verticalResults.m_element = verticalAlignmPtr;
    if (BentleyStatus::SUCCESS != changeDetector._UpdateBimAndSyncInfo(verticalResults, change))
        return BentleyStatus::ERROR;

    verticalAlignment = verticalAlignmPtr.get();

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDAlignmentsConverter::UpdateBimAlignment(AlignmentCR cifAlignment,
    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change, DgnCategoryId targetCategoryId)
    {
    auto horizGeometryPtr = cifAlignment.GetGeometry();

    auto alignmentCPtr = AlignmentBim::Alignment::Get(changeDetector.GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());
    AlignmentBim::HorizontalAlignmentPtr horizAlignmentPtr = 
        dynamic_cast<AlignmentBim::HorizontalAlignmentP>(alignmentCPtr->QueryHorizontal()->CopyForEdit().get());

    if (targetCategoryId.IsValid())
        horizAlignmentPtr->SetCategoryId(targetCategoryId);

    CurveVectorPtr bimHorizGeometryPtr;
    if (BentleyStatus::SUCCESS != Marshal(bimHorizGeometryPtr, *horizGeometryPtr))
        return BentleyStatus::ERROR;

    horizAlignmentPtr->SetGeometry(*bimHorizGeometryPtr);
    horizAlignmentPtr->GenerateElementGeom();
    if (horizAlignmentPtr->Update().IsNull())
        return BentleyStatus::ERROR;

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
        return BentleyStatus::ERROR;

    CurveVectorPtr bimVertGeometryPtr;
    if (BentleyStatus::SUCCESS != MarshalVertical(bimVertGeometryPtr, *vertGeometryPtr))
        return BentleyStatus::ERROR;

    auto verticalAlignmentPtr = AlignmentBim::VerticalAlignment::GetForEdit(changeDetector.GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());
    verticalAlignmentPtr->SetGeometry(*bimVertGeometryPtr);
    if (verticalAlignmentPtr->Update().IsNull())
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDAlignmentsConverter::ConvertProfiles(AlignmentCR cifAlignment, AlignmentBim::AlignmentCR alignment, ORDConverter::Params& params)
    {
    Bentley::WString activeProfileId;
    auto activeProfilePtr = cifAlignment.GetActiveProfile();
    if (activeProfilePtr.IsValid())
        activeProfileId = activeProfilePtr->GetSyncId();

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
                AlignmentBim::Alignment::SetMainVertical(alignment, *verticalAlignmCPtr);
            }
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ORDAlignmentsConverter::ConvertAlignment(AlignmentCR cifAlignment, ORDConverter::Params& params, DgnCategoryId targetCategoryId)
    {
    CifAlignmentSourceItem sourceItem(cifAlignment);

    // only convert an Alignment if it is new or has changed in the source
    auto change = params.changeDetectorP->_DetectChange(params.fileScopeId, sourceItem.Kind(), sourceItem, nullptr, params.spatialDataTransformHasChanged);

    AlignmentBim::AlignmentCPtr alignmentCPtr;
    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New == change.GetChangeType())
        {
        if (BentleyStatus::SUCCESS != CreateNewBimAlignment(cifAlignment, *params.changeDetectorP, change, alignmentCPtr, targetCategoryId))
            return DgnElementId();
        }
    else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged == change.GetChangeType())
        {
        params.changeDetectorP->_OnElementSeen(change.GetSyncInfoRecord().GetDgnElementId());
        }
    else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Changed == change.GetChangeType())
        {
        if (BentleyStatus::SUCCESS != UpdateBimAlignment(cifAlignment, *params.changeDetectorP, change, targetCategoryId))
            return DgnElementId();
        }

    if (alignmentCPtr.IsNull())
        alignmentCPtr = AlignmentBim::Alignment::Get(params.changeDetectorP->GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());

    ConvertProfiles(cifAlignment, *alignmentCPtr, params);

    return alignmentCPtr->GetElementId();
    }

DEFINE_POINTER_SUFFIX_TYPEDEFS(ORDCorridorsConverter)
DEFINE_REF_COUNTED_PTR(ORDCorridorsConverter)

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ORDCorridorsConverter: RefCountedBase
{
    struct CifCorridorSourceItem : ConsensusSourceItem
    {
    DEFINE_T_SUPER(ConsensusSourceItem)

    public:
        CifCorridorSourceItem(CorridorCR corridor): T_Super(corridor) {}

        static Utf8CP Kind() { return "Corridor"; }

    protected:
        Utf8String _GetHash() override
            {
            /*m_sha1(xml);
            return m_sha1.GetHashString();*/
            return "";
            }
    }; // CifCorridorSourceItem

private:
    Transform m_unitsScaleTransform;
    Dgn::DgnDbSync::DgnV8::Converter& m_converter;
    Dgn::PhysicalModelPtr m_bimPhysicalModelPtr;

private:
    ORDCorridorsConverter(DgnDbSync::DgnV8::Converter& converterLib, TransformCR unitsScaleTransform);

    BentleyStatus Marshal(PolyfaceHeaderPtr& bimMesh, Bentley::PolyfaceHeaderCR v8Mesh);
    BentleyStatus CreateNewRoadway(CorridorCR cifCorridor,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ROWID fileScopeId, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change,
        RoadRailBim::RoadwayCPtr& bimRoadwayCPtr);
    BentleyStatus UpdateRoadway(CorridorCR cifCorridor,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change);
    BentleyStatus AssignRoadwayGeomStream(CorridorCR cifCorridor, RoadRailBim::RoadwayR roadway);

public:
    static ORDCorridorsConverterPtr Create(DgnDbSync::DgnV8::Converter& converter, TransformCR unitsScaleTransform)
        {
        return new ORDCorridorsConverter(converter, unitsScaleTransform);
        }

    Dgn::PhysicalModelR GetPhysicalModel() const { return *m_bimPhysicalModelPtr; }
    DgnElementId ConvertCorridor(CorridorCR cifCorridor, ORDConverter::Params& params);
}; // ORDCorridorsConverter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ORDCorridorsConverter::ORDCorridorsConverter(DgnDbSync::DgnV8::Converter& converter, TransformCR unitsScaleTransform):
    m_converter(converter), m_unitsScaleTransform(unitsScaleTransform)
    {
    m_bimPhysicalModelPtr = RoadRailBim::RoadRailPhysicalDomain::QueryPhysicalModel(m_converter.GetJobSubject(), ORDBRIDGE_PhysicalModelName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::Marshal(PolyfaceHeaderPtr& bimMeshPtr, Bentley::PolyfaceHeaderCR v8CurveVector)
    {
    DgnDbSync::DgnV8::Converter::ConvertPolyface(bimMeshPtr, v8CurveVector);
    bimMeshPtr->Transform(m_unitsScaleTransform);

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::AssignRoadwayGeomStream(CorridorCR cifCorridor, RoadRailBim::RoadwayR roadway)
    {
    auto corridorSurfacesPtr = cifCorridor.GetCorridorSurfaces();
    if (corridorSurfacesPtr.IsNull())
        return BentleyStatus::SUCCESS;

    auto geomBuilderPtr = GeometryBuilder::Create(roadway);
    while (corridorSurfacesPtr.IsValid() && corridorSurfacesPtr->MoveNext())
        {
        auto corridorSurfacePtr = corridorSurfacesPtr->GetCurrent();
        if (corridorSurfacePtr.IsValid())
            {
            auto v8PolyfaceHeaderPtr = corridorSurfacePtr->GetMesh();
            if (v8PolyfaceHeaderPtr.IsValid())
                {
                PolyfaceHeaderPtr bimPolyfaceHeaderPtr;
                if (BentleyStatus::SUCCESS != Marshal(bimPolyfaceHeaderPtr, *v8PolyfaceHeaderPtr))
                    return BentleyStatus::ERROR;

                ElemDisplayParams v8DispParams;
                auto v8CorridorElmHandleCP = corridorSurfacePtr->GetElementHandle();
                corridorSurfacePtr->GetElementHandle()->GetDisplayHandler()->GetElemDisplayParams(*v8CorridorElmHandleCP, v8DispParams, true);

                Render::GeometryParams geomParams;
                geomParams.SetCategoryId(roadway.GetCategoryId());

                if (auto pMaterial = v8DispParams.GetMaterial())
                    {
                    geomParams.SetMaterialId(m_converter.GetRemappedMaterial(pMaterial));
                    if (!geomBuilderPtr->Append(geomParams, GeometryBuilder::CoordSystem::World))
                        return BentleyStatus::ERROR;
                    }

                if (!geomBuilderPtr->Append(*bimPolyfaceHeaderPtr, GeometryBuilder::CoordSystem::World))
                    return BentleyStatus::ERROR;
                }
            }
        }

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(roadway))
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::CreateNewRoadway(
    CorridorCR cifCorridor, 
    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, 
    iModelBridgeSyncInfoFile::ROWID fileScopeId,
    iModelBridgeSyncInfoFile::ChangeDetector::Results const& change, RoadRailBim::RoadwayCPtr& bimRoadwayCPtr)
    {
    auto roadwayPtr = RoadRailBim::Roadway::Create(*m_bimPhysicalModelPtr);

    ORDConverterUtils::AssignFederationGuid(*roadwayPtr, cifCorridor.GetSyncId());

    if (!WString::IsNullOrEmpty(cifCorridor.GetName().c_str()))
        roadwayPtr->SetUserLabel(Utf8String(cifCorridor.GetName().c_str()).c_str());
    
    AlignmentBim::AlignmentPtr bimMainAlignmentPtr;
    auto cifAlignmentPtr = cifCorridor.GetCorridorAlignment();
    if (cifAlignmentPtr.IsValid () && cifAlignmentPtr->IsFinalElement())
        {
        ORDAlignmentsConverter::CifAlignmentSourceItem alignmentItem(*cifAlignmentPtr);
        iModelBridgeSyncInfoFile::SourceIdentity sourceIdentity(fileScopeId, alignmentItem.Kind(), alignmentItem._GetId());
        auto iterator = changeDetector.GetSyncInfo().MakeIteratorBySourceId(sourceIdentity);
        auto iterEntry = iterator.begin();
        if (iterEntry != iterator.end())
            {
            bimMainAlignmentPtr = AlignmentBim::Alignment::GetForEdit(m_bimPhysicalModelPtr->GetDgnDb(), iterEntry.GetDgnElementId());
            if (bimMainAlignmentPtr.IsValid())
                {
                if (bimMainAlignmentPtr->GetILinearElementSource().IsValid())
                    return BentleyStatus::ERROR; // Alignment already associated with another Roadway

                roadwayPtr->SetMainAlignment(bimMainAlignmentPtr.get());
                }
            }
        }

    if (BentleyStatus::SUCCESS != AssignRoadwayGeomStream(cifCorridor, *roadwayPtr))
        return BentleyStatus::ERROR;

    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = roadwayPtr;
    if (BentleyStatus::SUCCESS != changeDetector._UpdateBimAndSyncInfo(results, change))
        return BentleyStatus::ERROR;

    if (bimMainAlignmentPtr.IsValid())
        {
        bimMainAlignmentPtr->SetILinearElementSource(roadwayPtr.get());
        bimMainAlignmentPtr->Update();
        }

    bimRoadwayCPtr = RoadRailBim::Roadway::Get(roadwayPtr->GetDgnDb(), roadwayPtr->GetElementId());

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::UpdateRoadway(CorridorCR cifCorridor,
    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change)
    {
    auto roadwayPtr = RoadRailBim::Roadway::GetForEdit(changeDetector.GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());

    if (BentleyStatus::SUCCESS != AssignRoadwayGeomStream(cifCorridor, *roadwayPtr))
        return BentleyStatus::ERROR;

    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = roadwayPtr;
    if (BentleyStatus::SUCCESS != changeDetector._UpdateBimAndSyncInfo(results, change))
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ORDCorridorsConverter::ConvertCorridor(CorridorCR cifCorridor, ORDConverter::Params& params)
    {
    CifCorridorSourceItem sourceItem(cifCorridor);

    RoadRailBim::RoadwayCPtr bimRoadwayCPtr;

    // only convert a Corridor if it is new or has changed in the source
    auto change = params.changeDetectorP->_DetectChange(params.fileScopeId, sourceItem.Kind(), sourceItem, nullptr, params.spatialDataTransformHasChanged);
    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged == change.GetChangeType())
        {
        params.changeDetectorP->_OnElementSeen(change.GetSyncInfoRecord().GetDgnElementId());
        }
    else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New == change.GetChangeType())
        {
        if (BentleyStatus::SUCCESS != CreateNewRoadway(cifCorridor, *params.changeDetectorP, params.fileScopeId, change, bimRoadwayCPtr))
            return DgnElementId();
        }
    else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Changed == change.GetChangeType())
        {
        if (BentleyStatus::SUCCESS != UpdateRoadway(cifCorridor, *params.changeDetectorP, change))
            return DgnElementId();
        }

    if (bimRoadwayCPtr.IsNull())
        bimRoadwayCPtr = RoadRailBim::Roadway::Get(params.changeDetectorP->GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());

    return bimRoadwayCPtr->GetElementId();
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

    AxisAlignedBox3d modelExtents = spatialModel.QueryModelRange();
    if (modelExtents.IsEmpty())
        return;

    modelExtents.Extend(0.5);

    if (params.isCreatingNewDgnDb && !forceExtendExtents)
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
ConvertORDElementXDomain::ConvertORDElementXDomain(ORDConverter& converter): m_converter(converter)
    {
    m_spatialLocationClassId = converter.GetDgnDb().Schemas().GetClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_SpatialLocation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ConvertORDElementXDomain::Result ConvertORDElementXDomain::_PreConvertElement(DgnV8EhCR v8el, Dgn::DgnDbSync::DgnV8::Converter&, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const& v8mm)
    {
    if (m_elementsSeen.end() != m_elementsSeen.find(v8el.GetElementRef()))
        return Result::Proceed;

    m_elementsSeen.insert(v8el.GetElementRef());

    auto cifAlignmentPtr = Alignment::CreateFromElementHandle(v8el);
    Bentley::Cif::CorridorPtr cifCorridorPtr;
    if (!cifAlignmentPtr.IsValid())
        {
        auto cifLinearEntity3dPtr = LinearEntity3d::CreateFromElementHandle(v8el);
        if (cifLinearEntity3dPtr.IsValid())
            {
            cifAlignmentPtr = cifLinearEntity3dPtr->GetAlignment();
            cifCorridorPtr = cifLinearEntity3dPtr->GetCorridor();
            }
        }

    if (!cifAlignmentPtr.IsValid())
        {
        if (cifCorridorPtr.IsNull())
            cifCorridorPtr = Corridor::CreateFromElementHandle(v8el);

        if (cifCorridorPtr.IsValid())
            {
            m_corridorV8RefSet.insert(v8el.GetElementRef());
            m_converter.m_cifCorridors.push_back({ cifCorridorPtr, v8el.GetElementRef() });
            }

        return Result::Proceed;
        }

    if (!cifAlignmentPtr->IsFinalElement())
        return Result::SkipElement;

    m_alignmentV8RefSet.insert(v8el.GetElementRef());
    m_converter.m_cifAlignments.push_back({cifAlignmentPtr, v8el.GetElementRef()});

    return Result::Proceed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertORDElementXDomain::_DetermineElementParams(DgnClassId& classId, DgnCode& code, DgnCategoryId& categoryId, DgnV8EhCR v8el, DgnDbSync::DgnV8::Converter& converter, ECObjectsV8::IECInstance const* primaryV8Instance, DgnDbSync::DgnV8::ResolvedModelMapping const& v8mm)
    {
    if (!m_spatialLocationClassId.IsValid() || !v8el.GetDgnModelP()->Is3D())
        return;

    auto alignmentIter = m_alignmentV8RefSet.find(v8el.GetElementRef());
    if (alignmentIter != m_alignmentV8RefSet.end())
        classId = m_spatialLocationClassId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertORDElementXDomain::_ProcessResults(DgnDbSync::DgnV8::ElementConversionResults& elRes, DgnV8EhCR v8el, DgnDbSync::DgnV8::ResolvedModelMapping const&, DgnDbSync::DgnV8::Converter&)
    {
    if (m_converter.m_v8ToBimElmMap.end() != m_converter.m_v8ToBimElmMap.find(v8el.GetElementRef()))
        return;

    if (m_alignmentV8RefSet.end() == m_alignmentV8RefSet.find(v8el.GetElementRef()) &&
        m_corridorV8RefSet.end() == m_corridorV8RefSet.find(v8el.GetElementRef()))
        return;

    m_converter.m_v8ToBimElmMap.insert({ v8el.GetElementRef(), elRes.m_element.get() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId convertToSpatialCategory(DrawingCategoryCR drawingCategory, DgnDbR dgnDb)
    {
    auto domainCategoryModelPtr = RoadRailAlignment::AlignmentCategoryModel::GetDomainModel(dgnDb);
    SpatialCategory spatialCategory(*domainCategoryModelPtr, drawingCategory.GetCategoryName());

    auto drawingSubCategoryCPtr = DgnSubCategory::Get(dgnDb, drawingCategory.GetDefaultSubCategoryId());

    spatialCategory.Insert(drawingSubCategoryCPtr->GetAppearance());
    return spatialCategory.GetCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::CreateRoadRailElements()
    {
    //bmap<DgnCategoryId, DgnCategoryId> drawingToSpatialCategoryMap;

    bset<DgnCategoryId> categoryIdMap;
    if (!m_cifAlignments.empty())
        {
        std::sort(m_cifAlignments.begin(), m_cifAlignments.end(), [](CifAlignmentV8RefPair const& a, CifAlignmentV8RefPair const& b)
            { return a.first->GetElementHandle()->GetElementRef() > b.first->GetElementHandle()->GetElementRef(); });

        Bentley::ElementRefP lastAlignmentRefP = nullptr;
        RoadRailAlignment::AlignmentCPtr bimAlignmentCPtr;
        auto alignmentsConverterPtr = ORDAlignmentsConverter::Create(GetJobSubject());
        for (auto& cifAlignmentEntry : m_cifAlignments)
            {
            auto& cifAlignmentPtr = cifAlignmentEntry.first;
            if (lastAlignmentRefP != cifAlignmentPtr->GetElementHandle()->GetElementRef())
                {
                auto bimAlignmentId = alignmentsConverterPtr->ConvertAlignment(*cifAlignmentPtr, *m_ordParams);
                lastAlignmentRefP = cifAlignmentPtr->GetElementHandle()->GetElementRef();
                bimAlignmentCPtr = RoadRailAlignment::Alignment::Get(GetDgnDb(), bimAlignmentId);
                }

            if (bimAlignmentCPtr.IsValid())
                {
                auto v8Iter = m_v8ToBimElmMap.find(cifAlignmentEntry.second);
                auto& bimElmPtr = v8Iter->second;
                RoadRailAlignment::Alignment::AddRepresentedBy(*bimAlignmentCPtr, *bimElmPtr);
                }
            }

        /*DgnCategoryId categoryId;
        if (alignmentEntry.second.second->ToGeometrySource3d())
            categoryId = alignmentEntry.second.second->ToGeometrySource3d()->GetCategoryId();
        else
            {
                categoryId = alignmentEntry.second.second->ToGeometrySource2d()->GetCategoryId();
                auto categoryMapIter = drawingToSpatialCategoryMap.find(categoryId);
                if (drawingToSpatialCategoryMap.end() == categoryMapIter)
                    {
                    DgnCategoryId drawingCategoryId = categoryId;
                    categoryId = convertToSpatialCategory(*DrawingCategory::Get(GetDgnDb(), drawingCategoryId), GetDgnDb());
                    drawingToSpatialCategoryMap.insert({ drawingCategoryId, categoryId });
                    }
                else
                    categoryId = categoryMapIter->second;
                }

            if (categoryIdMap.end() == categoryIdMap.find(categoryId))
                categoryIdMap.insert(categoryId);*/
        }

    if (!m_cifCorridors.empty())
        {
        std::sort(m_cifCorridors.begin(), m_cifCorridors.end(), [](CifCorridorV8RefPair const& a, CifCorridorV8RefPair const& b)
            { return a.first->GetElementHandle()->GetElementRef() > b.first->GetElementHandle()->GetElementRef(); });

        Bentley::DgnPlatform::ModelId modelIdForConverter = 0;
        ORDCorridorsConverterPtr corridorsConverterPtr;

        Bentley::ElementRefP lastCorridorRefP = nullptr;
        RoadRailBim::PathwayElementCPtr pathwayElmCPtr;
        for (auto& cifCorridorEntry : m_cifCorridors)
            {
            auto& cifCorridorPtr = cifCorridorEntry.first;
            if (corridorsConverterPtr.IsNull() || modelIdForConverter != cifCorridorPtr->GetDgnModelP()->GetModelId())
                {
                corridorsConverterPtr = ORDCorridorsConverter::Create(*this, ComputeUnitsScaleTransform(*cifCorridorEntry.first->GetDgnModelP()));
                modelIdForConverter = cifCorridorPtr->GetDgnModelP()->GetModelId();
                }

            if (lastCorridorRefP != cifCorridorPtr->GetElementHandle()->GetElementRef())
                {
                auto bimRoadwayId = corridorsConverterPtr->ConvertCorridor(*cifCorridorPtr, *m_ordParams);
                lastCorridorRefP = cifCorridorPtr->GetElementHandle()->GetElementRef();
                pathwayElmCPtr = RoadRailPhysical::PathwayElement::Get(GetDgnDb(), bimRoadwayId);
                }

            if (pathwayElmCPtr.IsValid())
                {
                auto v8Iter = m_v8ToBimElmMap.find(cifCorridorEntry.second);
                auto& bimElmPtr = v8Iter->second;
                RoadRailPhysical::PathwayElement::AddRepresentedBy(*pathwayElmCPtr, *bimElmPtr);
                }
            }
        }

    auto alignmentModelPtr = AlignmentBim::AlignmentModel::Query(GetJobSubject(), ORDBRIDGE_AlignmentModelName);
    auto horizontalAlignmentModelId = AlignmentBim::HorizontalAlignmentModel::QueryBreakDownModelId(*alignmentModelPtr);
    auto horizAlignmentModelCPtr = AlignmentBim::HorizontalAlignmentModel::Get(GetDgnDb(), horizontalAlignmentModelId);
    auto physicalModelPtr = RoadRailBim::RoadRailPhysicalDomain::QueryPhysicalModel(GetJobSubject(), ORDBRIDGE_PhysicalModelName);

    updateProjectExtents(*horizAlignmentModelCPtr, *m_ordParams, false);
    updateProjectExtents(*physicalModelPtr, *m_ordParams, true);

    if (IsCreatingNewDgnDb())
        {
        bvector<DgnCategoryId> additionalCategories;
        for (auto categoryId : categoryIdMap)
            additionalCategories.push_back(categoryId);

        auto viewId = RoadRailBim::RoadRailPhysicalDomain::SetUpDefaultViews(GetJobSubject(), ORDBRIDGE_AlignmentModelName, ORDBRIDGE_PhysicalModelName, &additionalCategories);
        if (viewId.IsValid())
            m_defaultViewId = viewId;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::_OnConversionComplete()
    {
    CreateRoadRailElements();

    T_Super::_OnConversionComplete();
    }

END_ORDBRIDGE_NAMESPACE