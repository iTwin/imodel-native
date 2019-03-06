/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDConverter.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
    AlignmentBim::AlignmentModelPtr m_bimDesignAlignmentModelPtr, m_bim3DLinearsAlignmentModelPtr;

    bool Is3DLinear(AlignmentCR alignment) const;

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

    AlignmentBim::AlignmentModelR GetDesignAlignmentModel() const { return *m_bimDesignAlignmentModelPtr; }
    AlignmentBim::AlignmentModelR Get3DLinearsAlignmentModel() const { return *m_bim3DLinearsAlignmentModelPtr; }
    DgnElementId ConvertAlignment(AlignmentCR cifAlignment, ORDConverter::Params& params, DgnCategoryId targetCategory = DgnCategoryId());
}; // ORDAlignmentsConverter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ORDAlignmentsConverter::ORDAlignmentsConverter(SubjectCR jobSubject)
    {
    m_bimDesignAlignmentModelPtr = AlignmentBim::AlignmentModel::Query(jobSubject, AlignmentBim::RoadRailAlignmentDomain::GetDesignPartitionName());
    m_bim3DLinearsAlignmentModelPtr = AlignmentBim::AlignmentModel::Query(jobSubject, AlignmentBim::RoadRailAlignmentDomain::Get3DLinearsPartitionName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ORDAlignmentsConverter::Is3DLinear(AlignmentCR alignment) const
    {
    auto linearEntity3dPtr = alignment.GetActiveLinearEntity3d();
    if (linearEntity3dPtr.IsNull())
        return false;

    if (WString::IsNullOrEmpty(linearEntity3dPtr->GetName().c_str()))
        return false;

    auto corridorPtr = linearEntity3dPtr->GetCorridor();
    return corridorPtr.IsValid();
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
    
    auto& bimAlignmentModelR = (Is3DLinear(cifAlignment)) ? Get3DLinearsAlignmentModel() : GetDesignAlignmentModel();

    DgnCode bimCode;
    
    // Identifying alignments appearing more than once in a single bridge-run.
    // For unnamed alignments, SyncId is used to differentiate them.
    // For named alignments, if a name (code) is already used, differentiate them with full file-name + modelname.
    if (Utf8String::IsNullOrEmpty(cifAlignmentName.c_str()))
        {        
        auto existingCPtr = bimAlignmentModelR.GetDgnDb().Elements().QueryElementByFederationGuid(bimGuid);
        if (existingCPtr.IsValid())
            return BentleyStatus::ERROR;
        cifAlignmentName = Utf8String(cifSyncId.c_str());
        }    

    Utf8String bimAlignmentName = cifAlignmentName;
    bimCode = AlignmentBim::RoadRailAlignmentDomain::CreateCode(bimAlignmentModelR, bimAlignmentName);

    int32_t suffix = 0;
    DgnElementId existingId;

    do
        {
        BeFileName fileName(cifAlignment.GetDgnModelP()->GetDgnFileP()->GetFileName().c_str());
        bimAlignmentName = Utf8String(fileName.GetFileNameAndExtension().c_str());
        bimAlignmentName += "\\";
        bimAlignmentName += Utf8String(cifAlignment.GetDgnModelP()->GetModelName());
        bimAlignmentName += "\\";
        bimAlignmentName += cifAlignmentName;

        if (suffix > 0)
            bimAlignmentName += Utf8PrintfString("-%d", suffix).c_str();

        bimCode = AlignmentBim::RoadRailAlignmentDomain::CreateCode(bimAlignmentModelR, bimAlignmentName);
        suffix++;

        existingId = bimAlignmentModelR.GetDgnDb().Elements().QueryElementIdByCode(bimCode);
        } while (existingId.IsValid());

    // Create Alignment
    auto bimAlignmentPtr = AlignmentBim::Alignment::Create(bimAlignmentModelR);
    if (targetCategoryId.IsValid())
        bimAlignmentPtr->SetCategoryId(targetCategoryId);
    
    if (bimCode.IsValid())
        bimAlignmentPtr->SetCode(bimCode);

    Utf8String userLabel(cifAlignment.GetName().c_str());
    bimAlignmentPtr->SetUserLabel(userLabel.c_str());
    
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

    bimHorizAlignmPtr->GenerateElementGeom();
    if (bimHorizAlignmPtr->Insert().IsNull())
        return BentleyStatus::ERROR;

    auto linearEntity3dPtr = cifAlignment.GetActiveLinearEntity3d();
    if (linearEntity3dPtr.IsValid())
        {
        linearGeomPtr = linearEntity3dPtr->GetLinearGeometry();
        if (linearGeomPtr.IsValid())
            {
            Bentley::CurveVectorPtr cifAlignment3dGeomPtr;
            linearGeomPtr->GetCurveVector(cifAlignment3dGeomPtr);

            CurveVectorPtr bimAlignment3dGeomPtr;
            if (BentleyStatus::SUCCESS != Marshal(bimAlignment3dGeomPtr, *cifAlignment3dGeomPtr))
                return BentleyStatus::ERROR;

            auto geomBuilder = GeometryBuilder::Create(*bimAlignmentPtr);
            if (!geomBuilder->Append(*bimAlignment3dGeomPtr, GeometryBuilder::CoordSystem::World))
                return BentleyStatus::ERROR;

            if (BentleyStatus::SUCCESS != geomBuilder->Finish(*bimAlignmentPtr))
                return BentleyStatus::ERROR;
            }
        }

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
        return BentleyStatus::ERROR;

    CurveVectorPtr bimVertGeometryPtr;
    if (BentleyStatus::SUCCESS != MarshalVertical(bimVertGeometryPtr, *vertGeometryPtr))
        return BentleyStatus::ERROR;
    
    auto verticalModelId = alignment.QueryVerticalAlignmentSubModelId();    
    if (!verticalModelId.IsValid())
        {
        auto verticalModelPtr = AlignmentBim::VerticalAlignmentModel::Create(
            AlignmentBim::VerticalAlignmentModel::CreateParams(GetDesignAlignmentModel().GetDgnDb(), alignment.GetElementId()));
        if (DgnDbStatus::Success != verticalModelPtr->Insert())
            return BentleyStatus::ERROR;

        verticalModelId = verticalModelPtr->GetModelId();
        }

    auto verticalModelCPtr = AlignmentBim::VerticalAlignmentModel::Get(alignment.GetDgnDb(), verticalModelId);;
    auto verticalAlignmPtr = AlignmentBim::VerticalAlignment::Create(*verticalModelCPtr, *bimVertGeometryPtr);

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
    auto alignmentCPtr = AlignmentBim::Alignment::Get(changeDetector.GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());
    AlignmentBim::HorizontalAlignmentPtr horizAlignmentPtr = 
        dynamic_cast<AlignmentBim::HorizontalAlignmentP>(alignmentCPtr->QueryHorizontal()->CopyForEdit().get());

    if (targetCategoryId.IsValid())
        horizAlignmentPtr->SetCategoryId(targetCategoryId);

    CurveVectorPtr bimHorizGeometryPtr;
    auto linearGeomPtr = cifAlignment.GetLinearGeometry();

    Bentley::CurveVectorPtr horizGeometryPtr;
    if (linearGeomPtr.IsValid())
        linearGeomPtr->GetCurveVector(horizGeometryPtr);

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
    ModelRefPinner modelPinner;

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

    if (!alignment.HasGeometry())
        {
        auto alignmentPtr = AlignmentBim::Alignment::GetForEdit(alignment.GetDgnDb(), alignment.GetElementId());
        if (DgnDbStatus::Success == alignmentPtr->GenerateAprox3dGeom())
            alignmentPtr->Update();
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
    ORDConverter& m_converter;
    RoadRailBim::DesignSpeedDefinitionCPtr m_defaultDesignSpeedDef;

private:
    ORDCorridorsConverter(ORDConverter& converterLib, TransformCR unitsScaleTransform);

    BentleyStatus Marshal(PolyfaceHeaderPtr& bimMesh, Bentley::PolyfaceHeaderCR v8Mesh);
    BentleyStatus CreateNewCorridor(CorridorCR cifCorridor,
        ORDConverter::Params& params, bool isRail, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change,
        RoadRailBim::CorridorCPtr& bimCorridorCPtr, DgnCategoryId targetCategoryId);
    BentleyStatus UpdateCorridor(CorridorCR cifCorridor,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change, DgnCategoryId targetCategoryId);
    BentleyStatus AssignCorridorGeomStream(CorridorCR cifCorridor, RoadRailBim::CorridorR corridor);

public:
    static ORDCorridorsConverterPtr Create(ORDConverter& converter, TransformCR unitsScaleTransform)
        {
        return new ORDCorridorsConverter(converter, unitsScaleTransform);
        }

    DgnElementId ConvertCorridor(CorridorCR cifCorridor, ORDConverter::Params& params, bool isRail, 
        DgnCategoryId targetCategoryId = DgnCategoryId());
}; // ORDCorridorsConverter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ORDCorridorsConverter::ORDCorridorsConverter(ORDConverter& converter, TransformCR unitsScaleTransform):
    m_converter(converter), m_unitsScaleTransform(unitsScaleTransform)
    {
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
BentleyStatus ORDCorridorsConverter::AssignCorridorGeomStream(CorridorCR cifCorridor, RoadRailBim::CorridorR corridor)
    {
    auto corridorSurfacesPtr = cifCorridor.GetCorridorSurfaces();
    if (corridorSurfacesPtr.IsNull())
        return BentleyStatus::SUCCESS;

    auto geomBuilderPtr = GeometryBuilder::Create(corridor);
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
                geomParams.SetCategoryId(corridor.GetCategoryId());

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

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(corridor))
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::CreateNewCorridor(
    CorridorCR cifCorridor, 
    ORDConverter::Params& params,
    bool isRail, 
    iModelBridgeSyncInfoFile::ChangeDetector::Results const& change, RoadRailBim::CorridorCPtr& bimCorridorCPtr, DgnCategoryId targetCategoryId)
    {
    auto corridorPtr = RoadRailBim::Corridor::Create(m_converter.GetPhysicalNetworkModel());
    if (targetCategoryId.IsValid())
        corridorPtr->SetCategoryId(targetCategoryId);

    ORDConverterUtils::AssignFederationGuid(*corridorPtr, cifCorridor.GetSyncId());

    if (!WString::IsNullOrEmpty(cifCorridor.GetName().c_str()))
        corridorPtr->SetUserLabel(Utf8String(cifCorridor.GetName().c_str()).c_str());
    
    AlignmentBim::AlignmentPtr bimMainAlignmentPtr;
    auto cifAlignmentPtr = cifCorridor.GetCorridorAlignment();
    if (cifAlignmentPtr.IsValid () && cifAlignmentPtr->IsFinalElement())
        {
        ORDAlignmentsConverter::CifAlignmentSourceItem alignmentItem(*cifAlignmentPtr);

        iModelExternalSourceAspect::ElementAndAspectId elementAndAspectId =
            iModelExternalSourceAspect::FindElementBySourceId(m_converter.GetDgnDb(), DgnElementId(params.fileScopeId), alignmentItem.Kind(), alignmentItem._GetId());
        if (elementAndAspectId.elementId.IsValid())
            {
            bimMainAlignmentPtr = AlignmentBim::Alignment::GetForEdit(m_converter.GetPhysicalNetworkModel().GetDgnDb(), elementAndAspectId.elementId);
            if (bimMainAlignmentPtr.IsValid())
                {
                if (bimMainAlignmentPtr->GetILinearElementSource().IsValid())
                    return BentleyStatus::ERROR; // Alignment already associated with another Corridor

                corridorPtr->SetMainLinearElement(bimMainAlignmentPtr.get());
                }
            }

        /*iModelBridgeSyncInfoFile::SourceIdentity sourceIdentity(params.fileScopeId, alignmentItem.Kind(), alignmentItem._GetId());
        auto iterator = params.syncInfo.MakeIteratorBySourceId(sourceIdentity);
        auto iterEntry = iterator.begin();
        if (iterEntry != iterator.end())
            {
            bimMainAlignmentPtr = AlignmentBim::Alignment::GetForEdit(m_converter.GetPhysicalNetworkModel().GetDgnDb(), iterEntry.GetDgnElementId());
            if (bimMainAlignmentPtr.IsValid())
                {
                if (bimMainAlignmentPtr->GetILinearElementSource().IsValid())
                    return BentleyStatus::ERROR; // Alignment already associated with another Corridor

                corridorPtr->SetMainLinearElement(bimMainAlignmentPtr.get());
                }
            }*/
        }

    if (BentleyStatus::SUCCESS != AssignCorridorGeomStream(cifCorridor, *corridorPtr))
        return BentleyStatus::ERROR;

    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = corridorPtr;
    if (BentleyStatus::SUCCESS != params.changeDetectorP->_UpdateBimAndSyncInfo(results, change))
        return BentleyStatus::ERROR;

    if (bimMainAlignmentPtr.IsValid())
        {
        bimMainAlignmentPtr->SetILinearElementSource(corridorPtr.get());
        bimMainAlignmentPtr->Update();
        }

    bimCorridorCPtr = RoadRailBim::Corridor::Get(corridorPtr->GetDgnDb(), corridorPtr->GetElementId());

    RoadRailBim::PathwayElementPtr pathwayPtr;
    if (isRail)
        pathwayPtr = RoadRailBim::Railway::Create(*bimCorridorCPtr);
    else
        pathwayPtr = RoadRailBim::Roadway::Create(*bimCorridorCPtr);

    pathwayPtr->SetMainLinearElement(bimMainAlignmentPtr.get());
    if (pathwayPtr->Insert(RoadRailBim::PathwayElement::Order::LeftMost).IsNull())
        return BentleyStatus::ERROR;

    if (m_defaultDesignSpeedDef.IsNull())
        {
        Dgn::DefinitionModelPtr standardsModelPtr;
        if (isRail)
            standardsModelPtr = RoadRailBim::RailwayStandardsModel::Query(*params.subjectCPtr);
        else
            standardsModelPtr = RoadRailBim::RoadwayStandardsModel::Query(*params.subjectCPtr);

        double msecSpeed = 0.0;
        Utf8String speedLabel;
        auto msecUnitCP = corridorPtr->GetDgnDb().Schemas().GetUnit("Units", "M_PER_SEC");
        if (params.rootModelUnitSystem == Dgn::UnitSystem::Metric)
            {
            auto kphUnitCP = corridorPtr->GetDgnDb().Schemas().GetUnit("Units", "KM_PER_HR");
            BENTLEY_NAMESPACE_NAME::Units::Quantity speedQty(100.0, *kphUnitCP); // 100 Km/h
            msecSpeed = speedQty.ConvertTo(msecUnitCP).GetMagnitude();
            speedLabel = "100 Km/h";
            }
        else
            { 
            auto mphUnitCP = corridorPtr->GetDgnDb().Schemas().GetUnit("Units", "MPH");
            BENTLEY_NAMESPACE_NAME::Units::Quantity speedQty(70.0, *mphUnitCP); // 70 MPH
            msecSpeed = speedQty.ConvertTo(msecUnitCP).GetMagnitude();
            speedLabel = "70 MPH";
            }        

        // Hard-coded for now - while the CIF SDK exposes an API for it
        auto designSpeedDefPtr = RoadRailBim::DesignSpeedDefinition::Create(*standardsModelPtr, msecSpeed); 
        designSpeedDefPtr->SetUserLabel(speedLabel.c_str());
        m_defaultDesignSpeedDef = designSpeedDefPtr->Insert();

        if (m_defaultDesignSpeedDef.IsNull())
            return BentleyStatus::ERROR;
        }

    if (bimMainAlignmentPtr.IsValid())
        {
        auto designSpeedPtr = 
            RoadRailBim::DesignSpeed::Create(RoadRailBim::DesignSpeed::CreateFromToParams(
                *pathwayPtr, *m_defaultDesignSpeedDef, 0, bimMainAlignmentPtr->GetLength()));
        if (designSpeedPtr->Insert().IsNull())
            return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::UpdateCorridor(CorridorCR cifCorridor,
    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change,
    DgnCategoryId targetCategoryId)
    {
    auto corridorPtr = RoadRailBim::Corridor::GetForEdit(changeDetector.GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());
    if (targetCategoryId.IsValid())
        corridorPtr->SetCategoryId(targetCategoryId);

    if (BentleyStatus::SUCCESS != AssignCorridorGeomStream(cifCorridor, *corridorPtr))
        return BentleyStatus::ERROR;

    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = corridorPtr;
    if (BentleyStatus::SUCCESS != changeDetector._UpdateBimAndSyncInfo(results, change))
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ORDCorridorsConverter::ConvertCorridor(CorridorCR cifCorridor, ORDConverter::Params& params, bool isRail, 
    DgnCategoryId targetCategoryId)
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
        if (BentleyStatus::SUCCESS != CreateNewCorridor(cifCorridor, params, isRail, change, bimCorridorCPtr, targetCategoryId))
            return DgnElementId();
        }
    else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Changed == change.GetChangeType())
        {
        if (BentleyStatus::SUCCESS != UpdateCorridor(cifCorridor, *params.changeDetectorP, change, targetCategoryId))
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

    AxisAlignedBox3d modelExtents = spatialModel.QueryModelRange();
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
ConvertORDElementXDomain::ConvertORDElementXDomain(ORDConverter& converter): m_converter(converter)
    {
    m_cifConsensusConnection = ConsensusConnection::Create(*m_converter.GetRootModelRefP());
    m_graphic3dClassId = converter.GetDgnDb().Schemas().GetClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_Graphic3d);    
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

            if (cifAlignmentPtr.IsValid() && cifCorridorPtr.IsValid())
                m_converter.m_cifGeneratedLinear3ds.push_back(cifLinearEntity3dPtr);
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
    if (v8mm.GetDgnModel().Is2dModel())
        return;

    if (!classId.IsValid())
        classId = m_graphic3dClassId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void assignORDFeatureAspect(Dgn::DgnElementR element, Cif::FeaturizedConsensusItemCR featurizedItem)
    {
    auto name = Utf8String(featurizedItem.GetName().c_str());
    auto featureDefPtr = featurizedItem.GetFeatureDefinition();

    if (!Utf8String::IsNullOrEmpty(name.c_str()) || featureDefPtr.IsValid())
        {
        if (auto featureAspectP = DgnV8ORDBim::FeatureAspect::GetP(element))
            {
            featureAspectP->SetName(name.c_str());

            if (featureDefPtr.IsValid())
                featureAspectP->SetDefinitionName(Utf8String(featureDefPtr->GetName().c_str()).c_str());
            }
        else
            {
            auto featureAspectPtr = DgnV8ORDBim::FeatureAspect::Create(name.c_str(), (featureDefPtr.IsValid()) ? Utf8String(featureDefPtr->GetName().c_str()).c_str() : nullptr);
            DgnV8ORDBim::FeatureAspect::Set(element, *featureAspectPtr);
            }
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
        templateName = Utf8String(templatePtr->GetName().c_str());

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
    if (auto corridorAspectP = DgnV8ORDBim::CorridorAspect::GetP(element))
        corridorAspectP->SetName(name.c_str());
    else
        {
        auto corridorAspectPtr = DgnV8ORDBim::CorridorAspect::Create(name.c_str());
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

    if (auto corridorSurfaceAspectP = DgnV8ORDBim::CorridorSurfaceAspect::GetP(element))
        {
        corridorSurfaceAspectP->SetIsTopMesh(isTopMesh);
        corridorSurfaceAspectP->SetIsBottomMesh(isBottomMesh);
        }
    else
        {
        auto corridorSurfaceAspectPtr = DgnV8ORDBim::CorridorSurfaceAspect::Create(isTopMesh, isBottomMesh, nullptr);
        DgnV8ORDBim::CorridorSurfaceAspect::Set(element, *corridorSurfaceAspectPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertORDElementXDomain::_ProcessResults(DgnDbSync::DgnV8::ElementConversionResults& elRes, DgnV8EhCR v8el, DgnDbSync::DgnV8::ResolvedModelMapping const& v8mm, DgnDbSync::DgnV8::Converter&)
    {
    if (m_converter.m_v8ToBimElmMap.end() != m_converter.m_v8ToBimElmMap.find(v8el.GetElementRef()))
        return;

    auto templateDropPtr = TemplateDrop::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
    if (templateDropPtr.IsValid())
        {
        assignORDFeatureAspect(*elRes.m_element, *templateDropPtr);
        assignTemplateDropAspect(*elRes.m_element, *templateDropPtr);        
        }
    else
        {
        auto superElevationPtr = SuperElevation::CreateFromElementHandle(v8el);
        if (superElevationPtr.IsValid())
            {
            assignStationRangeAspect(*elRes.m_element, superElevationPtr->GetStartDistance(), superElevationPtr->GetEndDistance());
            assignSuperelevationAspect(*elRes.m_element, *superElevationPtr);
            }
        else
            {
            auto cifCorridorPtr = Corridor::CreateFromElementHandle(v8el);
            if (cifCorridorPtr.IsValid())
                {
                assignCorridorAspect(*elRes.m_element, *cifCorridorPtr);
                }
            else
                {
                auto featurizedPtr = FeaturizedConsensusItem::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
                if (featurizedPtr.IsValid())
                    {
                    assignORDFeatureAspect(*elRes.m_element, *featurizedPtr);

                    if (v8mm.GetV8Model().Is3D())
                        {
                        if (auto cifCorridorSurfaceCP = dynamic_cast<CorridorSurfaceCP>(featurizedPtr.get()))
                            {
                            assignCorridorSurfaceAspect(*elRes.m_element, *cifCorridorSurfaceCP);
                            return;
                            }
                        }
                    }
                }
            }
        }

    if (m_alignmentV8RefSet.end() == m_alignmentV8RefSet.find(v8el.GetElementRef()) &&
        m_corridorV8RefSet.end() == m_corridorV8RefSet.find(v8el.GetElementRef()))
        return;

    m_converter.m_v8ToBimElmMap.insert({ v8el.GetElementRef(), elRes.m_element.get() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId convertToSpatialCategory(SubjectCR subject, DrawingCategoryCR drawingCategory)
    {
    auto configurationModelPtr = RoadRailAlignment::ConfigurationModel::Query(subject);
    SpatialCategory spatialCategory(*configurationModelPtr, drawingCategory.GetCategoryName());

    auto drawingSubCategoryCPtr = DgnSubCategory::Get(drawingCategory.GetDgnDb(), drawingCategory.GetDefaultSubCategoryId());

    spatialCategory.Insert(drawingSubCategoryCPtr->GetAppearance());
    return spatialCategory.GetCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId getSpatialCategoryToUse(SubjectCR subject, GeometrySourceCP bimGeomSourceCP, bmap<DgnCategoryId, DgnCategoryId>& drawingToSpatialCategoryMap)
    {
    DgnCategoryId categoryId = bimGeomSourceCP->GetCategoryId();
    if (bimGeomSourceCP->Is2d())
        {
        auto categoryMapIter = drawingToSpatialCategoryMap.find(categoryId);
        if (drawingToSpatialCategoryMap.end() == categoryMapIter)
            {
            DgnCategoryId drawingCategoryId = categoryId;
            categoryId = convertToSpatialCategory(subject, *DrawingCategory::Get(bimGeomSourceCP->GetSourceDgnDb(), drawingCategoryId));
            drawingToSpatialCategoryMap.insert({ drawingCategoryId, categoryId });
            }
        else
            categoryId = categoryMapIter->second;
        }

    return categoryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::CreateAlignments(bset<DgnCategoryId>& additionalCategoriesForSelector)
    {
    if (m_cifAlignments.empty())
        return;

    // There may be duplicate CifAlignments found via various V8 models. Some of them will have
    // the same ElementRef, but others will not, so relying on SyncId to tell whether two
    // CifAlignments are the same. Also, in some cases, some duplicate CifAlignments have
    // a blank name, so leaving the one with a name in front during sorting - that will be the
    // one processed later - the other duplicates will be ignored.
    std::sort(m_cifAlignments.begin(), m_cifAlignments.end(), [](CifAlignmentV8RefPair const& a, CifAlignmentV8RefPair const& b)
        {
        auto aSyncId = a.first->GetSyncId();        
        auto bSyncId = b.first->GetSyncId();
        auto equal = aSyncId.CompareTo(bSyncId);

        if (0 == equal)
            {
            auto aCifName = a.first->GetName();
            auto bCifName = b.first->GetName();
            equal = aCifName.CompareTo(bCifName);
            }

        return (0 == equal) ? true : (0 < equal);
        });

    Bentley::WString lastSyncId;
    Bentley::ElementRefP lastAlignmentRefP = nullptr;
    RoadRailAlignment::AlignmentCPtr bimAlignmentCPtr;
    bmap<DgnCategoryId, DgnCategoryId> drawingToSpatialCategoryMap;

    auto alignmentsConverterPtr = ORDAlignmentsConverter::Create(GetJobSubject());
    for (auto& cifAlignmentEntry : m_cifAlignments)
        {
        auto& cifAlignmentPtr = cifAlignmentEntry.first;
        if (lastAlignmentRefP == cifAlignmentPtr->GetElementHandle()->GetElementRef())
            continue;

        if (0 == lastSyncId.CompareTo(cifAlignmentPtr->GetSyncId()))
            continue;

        DgnCategoryId categoryId;
        DgnElementPtr bimElmPtr;

        auto v8Iter = m_v8ToBimElmMap.find(cifAlignmentEntry.second);
        if (v8Iter != m_v8ToBimElmMap.end())
            {
            bimElmPtr = v8Iter->second;
            if (auto bimGeomSourceCP = bimElmPtr->ToGeometrySource())
                {
                categoryId = getSpatialCategoryToUse(GetJobSubject(), bimGeomSourceCP, drawingToSpatialCategoryMap);

                if (additionalCategoriesForSelector.end() == additionalCategoriesForSelector.find(categoryId))
                    additionalCategoriesForSelector.insert(categoryId);
                }
            }

        auto bimAlignmentId = alignmentsConverterPtr->ConvertAlignment(*cifAlignmentPtr, *m_ordParams, categoryId);
        lastSyncId = cifAlignmentPtr->GetSyncId();
        lastAlignmentRefP = cifAlignmentPtr->GetElementHandle()->GetElementRef();
        bimAlignmentCPtr = RoadRailAlignment::Alignment::Get(GetDgnDb(), bimAlignmentId);

        if (bimAlignmentCPtr.IsValid() && bimElmPtr.IsValid())
            {
            if (auto bimGeomSourceCP = bimElmPtr->ToGeometrySource())
                {
                if (!bimAlignmentCPtr->QueryIsRepresentedBy(*bimGeomSourceCP))
                    RoadRailAlignment::Alignment::AddRepresentedBy(*bimAlignmentCPtr, *bimGeomSourceCP);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::CreatePathways(bset<DgnCategoryId>& additionalCategoriesForSelector)
    {
    if (m_cifCorridors.empty())
        return;

    std::sort(m_cifCorridors.begin(), m_cifCorridors.end(), [](CifCorridorV8RefPair const& a, CifCorridorV8RefPair const& b)
        { return a.first->GetElementHandle()->GetElementRef() > b.first->GetElementHandle()->GetElementRef(); });

    // The following query will get all element ids that have an "IsRail" property.
    auto& dgndb = GetDgnDb();
    BeSQLite::EC::ECSqlStatement stmt;
    stmt.Prepare(dgndb,
        "SELECT aspect.Element.Id, aspect.ECInstanceId, aspect.ECClassId FROM " BIS_SCHEMA(BIS_CLASS_ElementMultiAspect) " aspect, meta.ECPropertyDef propDef "
        "WHERE propDef.Name = 'IsRail' AND aspect.ECClassId = propDef.Class.Id");
    BeAssert(stmt.IsPrepared());

    bset<DgnElementId> railCorridorIds;
    while (BeSQLite::DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        auto elementId = stmt.GetValueId<DgnElementId>(0);
        auto dgnElementCPtr = dgndb.Elements().GetElement(elementId);
        auto ecInstanceId = stmt.GetValueId<BeSQLite::EC::ECInstanceId>(1);
        auto ecClassId = stmt.GetValueId<DgnClassId>(2);
        auto ecClass = dgndb.Schemas().GetClass(ecClassId);
        auto aspect = DgnElement::MultiAspect::GetAspect(*dgnElementCPtr, *ecClass, ecInstanceId);
        ECN::ECValue value;
        aspect->GetPropertyValue(value, "IsRail");

        if (!value.GetBoolean())
            continue;

        // elementId is a Rail corridor. Store elementId in a bset to be looked up later...
        railCorridorIds.insert(elementId);
        }


    Bentley::DgnPlatform::ModelId modelIdForConverter = 0;
    ORDCorridorsConverterPtr corridorsConverterPtr;

    Bentley::ElementRefP lastCorridorRefP = nullptr;
    RoadRailBim::CorridorCPtr corridorElmCPtr;
    bmap<DgnCategoryId, DgnCategoryId> drawingToSpatialCategoryMap;

    for (auto& cifCorridorEntry : m_cifCorridors)
        {
        auto& cifCorridorPtr = cifCorridorEntry.first;
        if (corridorsConverterPtr.IsNull() || modelIdForConverter != cifCorridorPtr->GetDgnModelP()->GetModelId())
            {
            corridorsConverterPtr = ORDCorridorsConverter::Create(*this, ComputeUnitsScaleTransform(*cifCorridorEntry.first->GetDgnModelP()));
            modelIdForConverter = cifCorridorPtr->GetDgnModelP()->GetModelId();
            }

        if (lastCorridorRefP == cifCorridorPtr->GetElementHandle()->GetElementRef())
            continue;

        DgnElementPtr bimElmPtr;
        DgnCategoryId categoryId;

        auto v8Iter = m_v8ToBimElmMap.find(cifCorridorEntry.second);
        if (v8Iter != m_v8ToBimElmMap.end())
            {
            bimElmPtr = v8Iter->second;
            if (auto bimGeomSourceCP = bimElmPtr->ToGeometrySource())
                {
                categoryId = getSpatialCategoryToUse(GetJobSubject(), bimGeomSourceCP, drawingToSpatialCategoryMap);

                if (additionalCategoriesForSelector.end() == additionalCategoriesForSelector.find(categoryId))
                    additionalCategoriesForSelector.insert(categoryId);
                }
            }

        bool isRail = bimElmPtr.IsValid() && railCorridorIds.find(bimElmPtr->GetElementId()) != railCorridorIds.end();
        auto bimCorridorId = corridorsConverterPtr->ConvertCorridor(*cifCorridorPtr, *m_ordParams, isRail, categoryId);
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
void setGeneratedAlignmentLabel(Dgn::DgnElementId alignmentId, RoadRailBim::PathwayElementCR pathway,
    RoadRailBim::TypicalSectionPointDefinitionCR typicalSectionPointDef)
    {
    auto pathwayAlignmentCPtr = pathway.GetMainLinearElementAs<RoadRailAlignment::Alignment>();

    Utf8String genAlgLabel;
    if (!Utf8String::IsNullOrEmpty(pathwayAlignmentCPtr->GetUserLabel()))
        {
        genAlgLabel = pathwayAlignmentCPtr->GetUserLabel();
        genAlgLabel.append("/");
        }

    genAlgLabel.append(typicalSectionPointDef.GetCode().GetValueUtf8CP());

    auto alignmentPtr = RoadRailAlignment::Alignment::GetForEdit(pathway.GetDgnDb(), alignmentId);
    alignmentPtr->SetUserLabel(genAlgLabel.c_str());
    alignmentPtr->Update();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::AssociateGeneratedAlignments()
    {
    auto roadwayStandardsModelPtr = RoadRailPhysical::RoadwayStandardsModel::Query(GetJobSubject());

    for (auto& cifGenLine3d : m_cifGeneratedLinear3ds)
        {
        if (WString::IsNullOrEmpty(cifGenLine3d->GetName().c_str()))
            continue;

        Utf8String pointCode(cifGenLine3d->GetName().c_str());
        auto pointDefCPtr = RoadRailPhysical::TypicalSectionPointDefinition::QueryByCode(*roadwayStandardsModelPtr, pointCode);
        if (pointDefCPtr.IsNull())
            pointDefCPtr = RoadRailPhysical::GenericTypicalSectionPointDefinition::CreateAndInsert(*roadwayStandardsModelPtr, pointCode.c_str(), pointCode.c_str());

        if (pointDefCPtr.IsNull())
            continue;

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

        RoadRailBim::CorridorCPtr corridorCPtr;
        auto cifCorridorPtr = cifGenLine3d->GetCorridor();

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
            auto pathwayId = corridorCPtr->QueryOrderedPathwayIds()[0];
            if (!pathwayId.IsValid())
                continue;

            auto pathwayCPtr = RoadRailBim::PathwayElement::Get(GetDgnDb(), pathwayId);
            if (pathwayCPtr.IsValid())
                {
                RoadRailBim::ILinearElementUtilities::SetRelatedCorridorPortion(*bimAlignmentCPtr, *pathwayCPtr, *pointDefCPtr);
                // TODO: enable after EAP
                /*if (Utf8String::IsNullOrEmpty(bimAlignmentCPtr->GetUserLabel()))
                    setGeneratedAlignmentLabel(bimAlignmentCPtr->GetElementId(), *roadwayCPtr, *pointDefCPtr);*/
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::CreateRoadRailElements()
    {
    bset<DgnCategoryId> additionalCategories;
    CreateAlignments(additionalCategories);
    CreatePathways(additionalCategories);
    AssociateGeneratedAlignments();

    auto designAlignmentModelPtr = AlignmentBim::AlignmentModel::Query(GetJobSubject(), AlignmentBim::RoadRailAlignmentDomain::GetDesignPartitionName());
    auto linearsAlignmentModelPtr = AlignmentBim::AlignmentModel::Query(GetJobSubject(), AlignmentBim::RoadRailAlignmentDomain::Get3DLinearsPartitionName());
    auto designHorizontalAlignmentModelId = AlignmentBim::HorizontalAlignmentModel::QueryBreakDownModelId(*designAlignmentModelPtr);
    auto designHorizAlignmentModelCPtr = AlignmentBim::HorizontalAlignmentModel::Get(GetDgnDb(), designHorizontalAlignmentModelId);
    auto linearsHorizontalAlignmentModelId = AlignmentBim::HorizontalAlignmentModel::QueryBreakDownModelId(*linearsAlignmentModelPtr);
    auto linearsHorizAlignmentModelCPtr = AlignmentBim::HorizontalAlignmentModel::Get(GetDgnDb(), linearsHorizontalAlignmentModelId);

    updateProjectExtents(*designHorizAlignmentModelCPtr, *m_ordParams, false);
    updateProjectExtents(*linearsHorizAlignmentModelCPtr, *m_ordParams, false);
    if (IsPhysicalNetworkModelSet())
        updateProjectExtents(GetPhysicalNetworkModel(), *m_ordParams, true);

    if (IsCreatingNewDgnDb() || !IsUpdating())
        {
        designAlignmentModelPtr->SetIsPrivate(false);
        designAlignmentModelPtr->Update();

        linearsAlignmentModelPtr->SetIsPrivate(false);
        linearsAlignmentModelPtr->Update();

        bvector<DgnCategoryId> additionalCategoriesForSelector;
        for (auto categoryId : additionalCategories)
            additionalCategoriesForSelector.push_back(categoryId);

        auto viewId = RoadRailBim::RoadRailPhysicalDomain::SetUpDefaultViews(GetJobSubject(), 
            GetPhysicalNetworkModel(), &additionalCategoriesForSelector);
        if (viewId.IsValid())
            {
            m_defaultViewId = viewId;

            auto viewDefCPtr = GetDgnDb().Elements().Get<SpatialViewDefinition>(viewId);
            auto modelSelectorPtr = GetDgnDb().Elements().GetForEdit<ModelSelector>(viewDefCPtr->GetModelSelectorId());
            modelSelectorPtr->AddModel(designAlignmentModelPtr->GetModelId());
            modelSelectorPtr->AddModel(linearsAlignmentModelPtr->GetModelId());
            modelSelectorPtr->Update();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::_OnConversionComplete()
    {
    if (m_isProcessing)
        CreateRoadRailElements();

    T_Super::_OnConversionComplete();
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
void ORDConverter::SetUpModelFormatters(Dgn::SubjectCR jobSubject)
    {    
    auto designAlignmentModelPtr = AlignmentBim::AlignmentModel::Query(jobSubject, AlignmentBim::RoadRailAlignmentDomain::GetDesignPartitionName());
    auto linearsAlignmentModelPtr = AlignmentBim::AlignmentModel::Query(jobSubject, AlignmentBim::RoadRailAlignmentDomain::Get3DLinearsPartitionName());

    DgnV8Api::ModelInfo const& v8ModelInfo = _GetModelInfo(*GetRootModelP());

    setUpModelFormatter(*designAlignmentModelPtr, v8ModelInfo);
    setUpModelFormatter(*linearsAlignmentModelPtr, v8ModelInfo);
    setUpModelFormatter(GetPhysicalNetworkModel(), v8ModelInfo);

    designAlignmentModelPtr->Update();
    linearsAlignmentModelPtr->Update();
    GetPhysicalNetworkModel().Update();
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

                        RoadRailBim::RoadRailPhysicalDomain::SetGeometricElementAsBoundingContentForSheet(
                            *dynamic_cast<GeometricElementCP>(bimClipElmCPtr.get()), 
                            *dynamic_cast<Sheet::ElementCP>(bimSheetElmCPtr.get()));
                        }
                    }
                }
            }
        }
    }

END_ORDBRIDGE_NAMESPACE