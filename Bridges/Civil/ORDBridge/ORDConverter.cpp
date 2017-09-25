/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDConverter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    Dgn::DgnDbSync::DgnV8::ConverterLibrary& m_converterLib;
    AlignmentBim::AlignmentModelPtr m_bimAlignmentModelPtr;

private:
    ORDAlignmentsConverter(DgnDbSync::DgnV8::ConverterLibrary& converterLib);

    BentleyStatus Marshal(CurveVectorPtr& bimCurveVector, Bentley::CurveVectorCR v8CurveVector);
    BentleyStatus MarshalVertical(CurveVectorPtr& bimCurveVector, Bentley::CurveVectorCR v8CurveVector);
    BentleyStatus CreateNewBimVerticalAlignment(ProfileCR, AlignmentBim::AlignmentCR alignment,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change,
        AlignmentBim::VerticalAlignmentCPtr& verticalAlignment);
    BentleyStatus CreateNewBimAlignment(AlignmentCR cifAlignment, 
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change, 
        AlignmentBim::AlignmentCPtr& bimAlignment);
    BentleyStatus UpdateBimAlignment(AlignmentCR cifAlignment,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change);
    BentleyStatus UpdateBimVerticalAlignment(ProfileCR,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change);
    BentleyStatus ConvertProfiles(AlignmentCR cifAlignment, AlignmentBim::AlignmentCR alignment, iModelBridgeSyncInfoFile::ChangeDetector& changeDetector);

public:
    static ORDAlignmentsConverterPtr Create(DgnDbSync::DgnV8::ConverterLibrary& converterLib)
        {
        return new ORDAlignmentsConverter(converterLib);
        }

    AlignmentBim::AlignmentModelR GetAlignmentModel() const { return *m_bimAlignmentModelPtr; }
    BentleyStatus ConvertAlignment(AlignmentCR cifAlignment, iModelBridgeSyncInfoFile::ChangeDetector& changeDetector);
}; // ORDAlignmentsConverter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ORDAlignmentsConverter::ORDAlignmentsConverter(DgnDbSync::DgnV8::ConverterLibrary& converterLib):
    m_converterLib(converterLib)
    {
    m_bimAlignmentModelPtr = AlignmentBim::AlignmentModel::Query(m_converterLib.GetJobSubject(), ORDBRIDGE_AlignmentModelName);    
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
    AlignmentBim::AlignmentCPtr& bimAlignment)
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
    if (bimCode.IsValid())
        bimAlignmentPtr->SetCode(bimCode);
    
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
    if (bimCode.IsValid())
        bimHorizAlignmPtr->SetCode(AlignmentBim::RoadRailAlignmentDomain::CreateCode(*bimHorizAlignmPtr->GetModel(), bimCode.GetValueUtf8()));

    bimHorizAlignmPtr->GenerateElementGeom();
    if (bimHorizAlignmPtr->Insert().IsNull())
        return BentleyStatus::ERROR;

    auto linearEntity3dPtr = cifAlignment.GetActiveLinearEntity3d();
    if (linearEntity3dPtr.IsValid())
        {
        auto cifAlignment3dGeomPtr = linearEntity3dPtr->GetGeometry();

        CurveVectorPtr bimAlignment3dGeomPtr;
        if (BentleyStatus::SUCCESS != Marshal(bimAlignment3dGeomPtr, *cifAlignment3dGeomPtr))
            return BentleyStatus::ERROR;

        auto geomBuilder = GeometryBuilder::Create(*bimAlignmentPtr);
        if (!geomBuilder->Append(*bimAlignment3dGeomPtr, GeometryBuilder::CoordSystem::World))
            return BentleyStatus::ERROR;

        if (BentleyStatus::SUCCESS != geomBuilder->Finish(*bimAlignmentPtr))
            return BentleyStatus::ERROR;
        }
    else
        bimAlignmentPtr->GenerateAprox3dGeom();

    if (bimAlignmentPtr->Update().IsNull())
        return BentleyStatus::ERROR;

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
    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change)
    {
    auto horizGeometryPtr = cifAlignment.GetGeometry();

    auto alignmentCPtr = AlignmentBim::Alignment::Get(changeDetector.GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());
    AlignmentBim::HorizontalAlignmentPtr horizAlignmentPtr = 
        dynamic_cast<AlignmentBim::HorizontalAlignmentP>(alignmentCPtr->QueryHorizontal()->CopyForEdit().get());

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
BentleyStatus ORDAlignmentsConverter::ConvertProfiles(AlignmentCR cifAlignment, AlignmentBim::AlignmentCR alignment, iModelBridgeSyncInfoFile::ChangeDetector& changeDetector)
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
        auto profileChange = changeDetector._DetectChange(iModelBridgeSyncInfoFile::ROWID(), profileItem.Kind(), profileItem);

        AlignmentBim::VerticalAlignmentCPtr verticalAlignmCPtr;
        if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged == profileChange.GetChangeType())
            {
            changeDetector._OnElementSeen(profileChange.GetSyncInfoRecord().GetDgnElementId());
            }
        else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New == profileChange.GetChangeType())
            {
            CreateNewBimVerticalAlignment(*cifProfilePtr, alignment, changeDetector, profileChange, verticalAlignmCPtr);
            }
        else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Changed == profileChange.GetChangeType())
            {
            if (BentleyStatus::SUCCESS != UpdateBimVerticalAlignment(*cifProfilePtr, changeDetector, profileChange))
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
BentleyStatus ORDAlignmentsConverter::ConvertAlignment(AlignmentCR cifAlignment, iModelBridgeSyncInfoFile::ChangeDetector& changeDetector)
    {
    CifAlignmentSourceItem sourceItem(cifAlignment);

    // only convert an Alignment if it is new or has changed in the source
    auto change = changeDetector._DetectChange(iModelBridgeSyncInfoFile::ROWID(), sourceItem.Kind(), sourceItem);

    AlignmentBim::AlignmentCPtr alignmentCPtr;
    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New == change.GetChangeType())
        {
        if (BentleyStatus::SUCCESS != CreateNewBimAlignment(cifAlignment, changeDetector, change, alignmentCPtr))
            return BentleyStatus::ERROR;
        }
    else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged == change.GetChangeType())
        {
        changeDetector._OnElementSeen(change.GetSyncInfoRecord().GetDgnElementId());
        }
    else if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Changed == change.GetChangeType())
        {
        if (BentleyStatus::SUCCESS != UpdateBimAlignment(cifAlignment, changeDetector, change))
            return BentleyStatus::ERROR;
        }

    if (alignmentCPtr.IsNull())
        alignmentCPtr = AlignmentBim::Alignment::Get(changeDetector.GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());

    return ConvertProfiles(cifAlignment, *alignmentCPtr, changeDetector);
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
    Dgn::DgnDbSync::DgnV8::ConverterLibrary& m_converterLib;
    Dgn::PhysicalModelPtr m_bimPhysicalModelPtr;

private:
    ORDCorridorsConverter(DgnDbSync::DgnV8::ConverterLibrary& converterLib, TransformCR unitsScaleTransform);

    BentleyStatus Marshal(PolyfaceHeaderPtr& bimMesh, Bentley::PolyfaceHeaderCR v8Mesh);
    BentleyStatus CreateNewRoadway(CorridorCR cifCorridor,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change);
    BentleyStatus UpdateRoadway(CorridorCR cifCorridor,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change);
    BentleyStatus AssignRoadwayGeomStream(CorridorCR cifCorridor, RoadRailBim::RoadwayR roadway);

public:
    static ORDCorridorsConverterPtr Create(DgnDbSync::DgnV8::ConverterLibrary& converterLib, TransformCR unitsScaleTransform)
        {
        return new ORDCorridorsConverter(converterLib, unitsScaleTransform);
        }

    Dgn::PhysicalModelR GetPhysicalModel() const { return *m_bimPhysicalModelPtr; }
    BentleyStatus ConvertCorridor(CorridorCR cifCorridor, iModelBridgeSyncInfoFile::ChangeDetector& changeDetector);
}; // ORDCorridorsConverter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ORDCorridorsConverter::ORDCorridorsConverter(DgnDbSync::DgnV8::ConverterLibrary& converterLib, TransformCR unitsScaleTransform):
    m_converterLib(converterLib), m_unitsScaleTransform(unitsScaleTransform)
    {
    m_bimPhysicalModelPtr = RoadRailBim::RoadRailPhysicalDomain::QueryPhysicalModel(m_converterLib.GetJobSubject(), ORDBRIDGE_PhysicalModelName);
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
                    geomParams.SetMaterialId(m_converterLib.GetRemappedMaterial(pMaterial));
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
    iModelBridgeSyncInfoFile::ChangeDetector::Results const& change)
    {
    auto roadwayPtr = RoadRailBim::Roadway::Create(*m_bimPhysicalModelPtr);

    ORDConverterUtils::AssignFederationGuid(*roadwayPtr, cifCorridor.GetSyncId());
    RoadRailBim::StatusAspect::Set(*roadwayPtr, *RoadRailBim::StatusAspect::Create(RoadRailBim::StatusAspect::Status::Proposed));

    if (!WString::IsNullOrEmpty(cifCorridor.GetName().c_str()))
        roadwayPtr->SetUserLabel(Utf8String(cifCorridor.GetName().c_str()).c_str());
    
    auto cifAlignmentPtr = cifCorridor.GetCorridorAlignment();
    if (cifAlignmentPtr.IsValid () && cifAlignmentPtr->IsFinalElement())
        {
        ORDAlignmentsConverter::CifAlignmentSourceItem alignmentItem(*cifAlignmentPtr);
        iModelBridgeSyncInfoFile::SourceIdentity sourceIdentity(iModelBridgeSyncInfoFile::ROWID(), alignmentItem.Kind(), alignmentItem._GetId());
        auto iterator = changeDetector.GetSyncInfo().MakeIteratorBySourceId(sourceIdentity);
        auto iterEntry = iterator.begin();
        if (iterEntry != iterator.end())
            {
            auto bimAlignmentCPtr = AlignmentBim::Alignment::Get(m_bimPhysicalModelPtr->GetDgnDb(), iterEntry.GetDgnElementId());
            if (bimAlignmentCPtr.IsValid())
                roadwayPtr->SetAlignment(bimAlignmentCPtr.get());
            }
        }

    if (BentleyStatus::SUCCESS != AssignRoadwayGeomStream(cifCorridor, *roadwayPtr))
        return BentleyStatus::ERROR;

    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = roadwayPtr;
    if (BentleyStatus::SUCCESS != changeDetector._UpdateBimAndSyncInfo(results, change))
        return BentleyStatus::ERROR;

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
BentleyStatus ORDCorridorsConverter::ConvertCorridor(CorridorCR cifCorridor, iModelBridgeSyncInfoFile::ChangeDetector& changeDetector)
    {
    CifCorridorSourceItem sourceItem(cifCorridor);

    // only convert a Corridor if it is new or has changed in the source
    auto change = changeDetector._DetectChange(iModelBridgeSyncInfoFile::ROWID(), sourceItem.Kind(), sourceItem);
    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Unchanged == change.GetChangeType())
        {
        changeDetector._OnElementSeen(change.GetSyncInfoRecord().GetDgnElementId());
        return BentleyStatus::SUCCESS;
        }

    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New == change.GetChangeType())
        return CreateNewRoadway(cifCorridor, changeDetector, change);

    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::Changed == change.GetChangeType())
        return UpdateRoadway(cifCorridor, changeDetector, change);

    return BentleyStatus::ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::ConvertAlignments(GeometryModel::SDK::GeometricModel const& geomModel, DgnDbSync::DgnV8::ConverterLibrary& converterLib,
    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector)
    {    
    ORDAlignmentsConverterPtr alignmentsConvPtr;

    auto alignmentsPtr = geomModel.GetAlignments();
    while (alignmentsPtr.IsValid() && alignmentsPtr->MoveNext())
        {
        auto cifAlignmentPtr = alignmentsPtr->GetCurrent();
        if (!cifAlignmentPtr->IsFinalElement())
            continue;

        if (alignmentsConvPtr.IsNull())
            alignmentsConvPtr = ORDAlignmentsConverter::Create(converterLib);

        alignmentsConvPtr->ConvertAlignment(*cifAlignmentPtr, changeDetector);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::ConvertCorridors(GeometryModel::SDK::GeometricModel const& geomModel, DgnDbSync::DgnV8::ConverterLibrary& converterLib,
    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector)
    {
    ORDCorridorsConverterPtr corridorsConvPtr;

    auto corridorsPtr = geomModel.GetCorridors();
    while (corridorsPtr.IsValid() && corridorsPtr->MoveNext())
        {
        auto corridorPtr = corridorsPtr->GetCurrent();
        if (!corridorPtr.IsValid())
            continue;

        if (corridorsConvPtr.IsNull())
            {
            auto dgnModelP = corridorPtr->GetDgnModelP();
            corridorsConvPtr = ORDCorridorsConverter::Create(converterLib, converterLib.ComputeUnitsScaleTransform(*dgnModelP));
            }

        corridorsConvPtr->ConvertCorridor(*corridorPtr, changeDetector);
        }
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::ConvertORDData(BeFileNameCR dgnFileName, SubjectCR subject, iModelBridgeSyncInfoFile::ChangeDetector& changeDetector)
    {
    DgnDbSync::DgnV8::RootModelConverter::RootModelSpatialParams converterLibraryParams;    
    DgnDbSync::DgnV8::ConverterLibrary converterLib(subject.GetDgnDb(), converterLibraryParams);
    converterLib.SetJobSubject(subject);
    converterLib.SetSpatialParentSubject(subject);
    
    DgnV8Api::DgnFileStatus v8Status;
    auto dgnFilePtr = converterLib.OpenDgnV8File(v8Status, dgnFileName);
    if (dgnFilePtr.IsNull())
        return;

    bset<DgnFileP> dgnFileSet;
    converterLibraryParams.AddDrawingOrSheetFile(dgnFileName);    
    dgnFileSet.insert(dgnFilePtr.get());

    converterLib.SetRootV8File(dgnFilePtr.get());
    auto rootSpatialModelP = loadDgnModel(*dgnFilePtr, converterLib.GetRootModelId());
    if (!rootSpatialModelP)
        return;

    auto rootCIFModelP = loadDgnModel(*dgnFilePtr, dgnFilePtr->GetDefaultModelId());
    if (!rootCIFModelP)
        return;

    auto cifConnPtr = ConsensusConnection::Create(*rootCIFModelP);
    auto cifModelPtr = ConsensusModel::Create(*cifConnPtr);
    if (cifModelPtr.IsValid())
        {        
        converterLib.ComputeCoordinateSystemTransform(*rootSpatialModelP);
        converterLib.ConvertModelMaterials(*rootSpatialModelP);

        // Mapping the root-model to the Physical Model on the BIM side.
        // That assumes the root-model is spatial (3D).
        converterLib.RecordModelMapping(*rootSpatialModelP,
            *RoadRailBim::RoadRailPhysicalDomain::GetDomain().QueryPhysicalModel(subject, ORDBRIDGE_PhysicalModelName));

        auto geomModelsPtr = cifModelPtr->GetActiveGeometricModels();
        while (geomModelsPtr.IsValid() && geomModelsPtr->MoveNext())
            {
            auto geomModelPtr = geomModelsPtr->GetCurrent();
            if (geomModelPtr.IsValid())
                {
                auto currentDgnFileP = geomModelPtr->GetDgnModelP()->GetDgnFileP();
                if (dgnFileSet.end() == dgnFileSet.find(currentDgnFileP))
                    {
                    converterLibraryParams.AddDrawingOrSheetFile(BeFileName(currentDgnFileP->GetFileName().c_str()));
                    dgnFileSet.insert(currentDgnFileP);
                    }

                ConvertAlignments(*geomModelPtr, converterLib, changeDetector);
                ConvertCorridors(*geomModelPtr, converterLib, changeDetector);
                }
            }

        converterLib.ConvertAllDrawingsAndSheets();
        converterLib.RemoveUnusedMaterials();
        }
    }

END_ORDBRIDGE_NAMESPACE