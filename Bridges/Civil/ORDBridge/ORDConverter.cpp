/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDConverter.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ORDBridgeInternal.h>
#include <windows.h>
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
    Dgn::SpatialLocationModelPtr m_bimDesignAlignmentModelPtr;
    Dgn::PhysicalModelPtr m_bimNetworkModelPtr;
    ORDConverter& m_ordConverter;

    bool IsDesignAlignment(AlignmentCR alignment) const;
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
        AlignmentBim::AlignmentCPtr& bimAlignment);
    BentleyStatus UpdateBimAlignment(AlignmentCR cifAlignment,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change);
    BentleyStatus UpdateBimVerticalAlignment(ProfileCR,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change);
    BentleyStatus ConvertProfiles(AlignmentCR cifAlignment, AlignmentBim::AlignmentCR alignment, ORDConverter::Params& params);

public:
    static ORDAlignmentsConverterPtr Create(ORDConverter& converter)
        {
        return new ORDAlignmentsConverter(converter);
        }

    Dgn::SpatialLocationModelR GetDesignAlignmentModel() const { return *m_bimDesignAlignmentModelPtr; }
    Dgn::PhysicalModelR GetRoadNetworkModel() const { return *m_bimNetworkModelPtr; }
    DgnElementId ConvertAlignment(AlignmentCR cifAlignment, ORDConverter::Params& params);
}; // ORDAlignmentsConverter

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

private:
    ORDCorridorsConverter(ORDConverter& converter, TransformCR unitsScaleTransform);

    BentleyStatus Marshal(PolyfaceHeaderPtr& bimMesh, Bentley::PolyfaceHeaderCR v8Mesh);
    BentleyStatus CreateNewCorridor(CorridorCR cifCorridor,
        ORDConverter::Params& params, bool isRail, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change,
        RoadRailBim::CorridorCPtr& bimCorridorCPtr);
    BentleyStatus UpdateCorridor(CorridorCR cifCorridor,
        iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change);
    BentleyStatus AssignCorridorGeomStream(CorridorCR cifCorridor, RoadRailBim::CorridorR corridor);
    RoadRailBim::DesignSpeedDefinitionCPtr ORDCorridorsConverter::GetOrInsertDesignSpeedDefinition(
        Dgn::DefinitionModelCR standardsModel, double speedInMPerSec, RoadRailBim::DesignSpeedDefinition::UnitSystem unitSystem);    

public:
    static ORDCorridorsConverterPtr Create(ORDConverter& converter, TransformCR unitsScaleTransform)
        {
        return new ORDCorridorsConverter(converter, unitsScaleTransform);
        }

    DgnElementId ConvertCorridor(CorridorCR cifCorridor, ORDConverter::Params& params, bool isRail, 
        DgnCategoryId targetCategoryId = DgnCategoryId());

    void ConvertSpeedTable(SpeedTableCP speedTable, int32_t cifDesignSpeedIdx, RoadRailBim::PathwayElementCR pathway, Dgn::DefinitionModelCR standardsModel,
        RoadRailBim::DesignSpeedDefinition::UnitSystem unitSystem, ORDConverter::Params const& params);
}; // ORDCorridorsConverter

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ORDAlignmentsConverter::ORDAlignmentsConverter(ORDConverter& converter): m_ordConverter(converter)
    {
    m_bimNetworkModelPtr = converter.GetRoadRailNetwork()->GetNetworkModel();
    auto designAlignmentsCPtr = AlignmentBim::DesignAlignments::Query(*m_bimNetworkModelPtr);
    m_bimDesignAlignmentModelPtr = designAlignmentsCPtr->GetAlignmentModel();    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ORDAlignmentsConverter::IsDesignAlignment(AlignmentCR alignment) const
    {
    auto linear3dPtr = alignment.GetActiveLinearEntity3d();
    if (linear3dPtr.IsValid())
        {
        if (linear3dPtr->GetCorridor().IsValid())
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialModelP ORDAlignmentsConverter::Get3DLinearsAlignmentModel(AlignmentCR alignment, ORDConverter::Params& params) const
    {
    auto linear3dPtr = alignment.GetActiveLinearEntity3d();
    auto corridorPtr = linear3dPtr->GetCorridor();

    ORDCorridorsConverter::CifCorridorSourceItem corridorItem(*corridorPtr);

    iModelExternalSourceAspect::ElementAndAspectId elementAndAspectId =
        iModelExternalSourceAspect::FindElementBySourceId(m_ordConverter.GetDgnDb(), DgnElementId(params.fileScopeId), corridorItem.Kind(), corridorItem._GetId());
    if (elementAndAspectId.elementId.IsValid())
        {
        auto bimCorridorCPtr = RoadRailBim::Corridor::Get(m_ordConverter.GetDgnDb(), elementAndAspectId.elementId);
        auto corridorPortionsCPtr = RoadRailBim::CorridorPortions::Query(*bimCorridorCPtr);
        return corridorPortionsCPtr->GetCorridorPortionsModel().get();
        }

    BeAssert(false);
    return m_bimDesignAlignmentModelPtr.get();
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
                RoadRailAlignment::Alignment::AddRepresentedBy(bimAlignment, *bimGeomSourceCP);
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
        origin = { origin2d.x, origin2d.y, 0 };
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
        auto subCatIter = subCategoryMap.find(subCatId);

        DgnSubCategoryId targetSubCatId;
        if (subCategoryMap.end() != subCatIter)
            targetSubCatId = subCatIter->second;
        else
            {
            auto bimConvertedCatCPtr = DgnCategory::Get(m_ordConverter.GetDgnDb(), geomParams.GetCategoryId());
            targetSubCatId = DgnSubCategory::QuerySubCategoryId(m_ordConverter.GetDgnDb(),
                DgnSubCategory::CreateCode(*bimAlignmentCatCPtr, bimConvertedCatCPtr->GetCategoryName()));
            if (!targetSubCatId.IsValid())
                {
                auto bimConvertedSubCatCPtr = DgnSubCategory::Get(m_ordConverter.GetDgnDb(), subCatId);

                DgnSubCategory newAlignmentSubCat(DgnSubCategory::CreateParams(m_ordConverter.GetDgnDb(),
                    bimTargetCategoryId, bimConvertedCatCPtr->GetCategoryName(), bimConvertedSubCatCPtr->GetAppearance()));
                newAlignmentSubCat.Insert();
                targetSubCatId = newAlignmentSubCat.GetSubCategoryId();

                if (targetSubCatId.IsValid())
                    subCategoryMap.insert({ subCatId, targetSubCatId });
                }
            }

        if (targetSubCatId.IsValid())
            {
            geomParams.SetCategoryId(bimTargetCategoryId);
            geomParams.SetSubCategoryId(targetSubCatId);
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
    
    Dgn::SpatialModelPtr bimAlignmentModelPtr;
    if (IsDesignAlignment(cifAlignment))
        bimAlignmentModelPtr = &GetDesignAlignmentModel();
    else
        bimAlignmentModelPtr = Get3DLinearsAlignmentModel(cifAlignment, params);

    DgnCode bimCode;
    
    if (Utf8String::IsNullOrEmpty(cifAlignmentName.c_str()))
        {        
        if (!WString::IsNullOrEmpty(cifAlignment.GetFeatureName().c_str()))
            cifAlignmentName = Utf8String(cifAlignment.GetFeatureName().c_str());
        else if (cifAlignment.GetFeatureDefinition().IsValid() &&
            !WString::IsNullOrEmpty(cifAlignment.GetFeatureDefinition()->GetName().c_str()))
            cifAlignmentName = Utf8String(cifAlignment.GetFeatureDefinition()->GetName().c_str());
        else
            cifAlignmentName = Utf8String(cifSyncId.c_str());
        }    

    Utf8String bimAlignmentName = cifAlignmentName;
    bimCode = AlignmentBim::RoadRailAlignmentDomain::CreateCode(*bimAlignmentModelPtr, bimAlignmentName);

    // Create Alignment
    auto bimAlignmentPtr = AlignmentBim::Alignment::Create(*bimAlignmentModelPtr);
    
    if (bimCode.IsValid())
        bimAlignmentPtr->SetCode(bimCode);

    Utf8String userLabel(cifAlignment.GetName().c_str());
    bimAlignmentPtr->SetUserLabel(userLabel.c_str());
    
    ORDConverterUtils::AssignFederationGuid(*bimAlignmentPtr, cifSyncId);

    DgnCategoryId alignmentCatId;
    auto cif3dLinearPtr = cifAlignment.GetActiveLinearEntity3d();
    if (cif3dLinearPtr.IsValid())
        AssignGeomStream(*cif3dLinearPtr, *bimAlignmentPtr);

    auto cifStationingPtr = cifAlignment.GetStationing();
    if (cifStationingPtr.IsValid())
        {
        bimAlignmentPtr->SetStartStation(cifStationingPtr->GetStartStation());
        bimAlignmentPtr->SetStartValue(cifStationingPtr->GetDistanceAlong());
        }

    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = bimAlignmentPtr;
    if (BentleyStatus::SUCCESS != params.changeDetectorP->_UpdateBimAndSyncInfo(results, change)) 
        return BentleyStatus::ERROR;

    CurveVectorPtr bimHorizGeometryPtr;
    if (BentleyStatus::SUCCESS != Marshal(bimHorizGeometryPtr, *horizGeometryPtr))
        return BentleyStatus::ERROR;

    // Create Horizontal Alignment
    auto bimHorizAlignmPtr = AlignmentBim::HorizontalAlignment::Create(*bimAlignmentPtr, *bimHorizGeometryPtr);

    if (bimCode.IsValid())
        bimHorizAlignmPtr->SetCode(AlignmentBim::RoadRailAlignmentDomain::CreateCode(*bimHorizAlignmPtr->GetModel(), bimCode.GetValueUtf8()));

    AssignGeomStream(cifAlignment, *bimHorizAlignmPtr);

    if (bimHorizAlignmPtr->Insert().IsNull())
        return BentleyStatus::ERROR;

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

    auto verticalAlignmPtr = AlignmentBim::VerticalAlignment::Create(alignment, *bimVertGeometryPtr);

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
    auto alignmentCPtr = AlignmentBim::Alignment::Get(changeDetector.GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());
    AlignmentBim::HorizontalAlignmentPtr horizAlignmentPtr = 
        dynamic_cast<AlignmentBim::HorizontalAlignmentP>(alignmentCPtr->GetHorizontal()->CopyForEdit().get());

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
DgnElementId ORDAlignmentsConverter::ConvertAlignment(AlignmentCR cifAlignment, ORDConverter::Params& params)
    {
    CifAlignmentSourceItem sourceItem(cifAlignment);

    // only convert an Alignment if it is new or has changed in the source
    auto change = params.changeDetectorP->_DetectChange(params.fileScopeId, sourceItem.Kind(), sourceItem, nullptr, params.spatialDataTransformHasChanged);

    AlignmentBim::AlignmentCPtr alignmentCPtr;
    if (iModelBridgeSyncInfoFile::ChangeDetector::ChangeType::New == change.GetChangeType())
        {
        if (BentleyStatus::SUCCESS != CreateNewBimAlignment(cifAlignment, params, change, alignmentCPtr))
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

    ConvertProfiles(cifAlignment, *alignmentCPtr, params);

    return alignmentCPtr->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ORDCorridorsConverter::ORDCorridorsConverter(ORDConverter& converter, TransformCR unitsScaleTransform):
    m_converter(converter), m_unitsScaleTransform(unitsScaleTransform)
    {
    m_msecUnitCP = converter.GetDgnDb().Schemas().GetUnit("Units", "M_PER_SEC");
    m_kphUnitCP = converter.GetDgnDb().Schemas().GetUnit("Units", "KM_PER_HR");
    m_mphUnitCP = converter.GetDgnDb().Schemas().GetUnit("Units", "MPH");
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

    auto corridorCategoryCPtr = DgnCategory::Get(m_converter.GetDgnDb(), corridor.GetCategoryId());

    bmap<DgnSubCategoryId, DgnSubCategoryId> subCategoryMap;
    auto geomBuilderPtr = GeometryBuilder::Create(corridor);
    while (corridorSurfacesPtr.IsValid() && corridorSurfacesPtr->MoveNext())
        {
        auto corridorSurfacePtr = corridorSurfacesPtr->GetCurrent();
        if (corridorSurfacePtr.IsNull())
            continue;

        auto iter = m_converter.m_v8ToBimElmMap.find(corridorSurfacePtr->GetElementHandle()->GetElementRef());
        if (iter == m_converter.m_v8ToBimElmMap.end())
            continue;

        auto bimConvertedElmPtr = iter->second;
        for (auto geomIter : GeometryCollection(*bimConvertedElmPtr->ToGeometrySource()))
            {
            auto geomParams = geomIter.GetGeometryParams(); // Intentionally copied

            auto subCatId = geomParams.GetSubCategoryId();
            auto subCatIter = subCategoryMap.find(subCatId);

            DgnSubCategoryId targetSubCatId;
            if (subCategoryMap.end() != subCatIter)
                targetSubCatId = subCatIter->second;
            else
                {
                auto bimConvertedCatCPtr = DgnCategory::Get(m_converter.GetDgnDb(), geomParams.GetCategoryId());
                targetSubCatId = DgnSubCategory::QuerySubCategoryId(m_converter.GetDgnDb(),
                    DgnSubCategory::CreateCode(*corridorCategoryCPtr, bimConvertedCatCPtr->GetCategoryName()));
                if (!targetSubCatId.IsValid())
                    {
                    auto bimConvertedSubCatCPtr = DgnSubCategory::Get(m_converter.GetDgnDb(), subCatId);

                    DgnSubCategory newCorridorSubCat(DgnSubCategory::CreateParams(m_converter.GetDgnDb(),
                        corridorCategoryCPtr->GetCategoryId(), bimConvertedCatCPtr->GetCategoryName(), bimConvertedSubCatCPtr->GetAppearance()));
                    newCorridorSubCat.Insert();
                    targetSubCatId = newCorridorSubCat.GetSubCategoryId();

                    if (targetSubCatId.IsValid())
                        subCategoryMap.insert({ subCatId, targetSubCatId });
                    }
                }

            if (targetSubCatId.IsValid())
                {
                geomParams.SetCategoryId(corridorCategoryCPtr->GetCategoryId());
                geomParams.SetSubCategoryId(targetSubCatId);
                if (!geomBuilderPtr->Append(geomParams, GeometryBuilder::CoordSystem::World))
                    BeAssert(false);
                }

            auto geomPtr = geomIter.GetGeometryPtr();
            geomPtr->TransformInPlace(geomIter.GetGeometryToWorld());
            if (!geomBuilderPtr->Append(*geomPtr, GeometryBuilder::CoordSystem::World))
                BeAssert(false);
            }
        }

    if (BentleyStatus::SUCCESS != geomBuilderPtr->Finish(corridor))
        return BentleyStatus::ERROR;

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

                stationStartEndSpeeds.push_back({ startStation, { startSpeed, endSpeed } });
                }
            }
        }

    auto alignmentCPtr = AlignmentBim::Alignment::Get(pathway.GetDgnDb(), pathway.GetDesignAlignmentId());
    double lastStartStation = alignmentCPtr->GetLength();

    if (stationStartEndSpeeds.empty())
        {
        BENTLEY_NAMESPACE_NAME::Units::Quantity speedQtyKPH(100.0, *m_kphUnitCP);
        BENTLEY_NAMESPACE_NAME::Units::Quantity speedQtyMPH(60.0, *m_mphUnitCP);

        double speedQtyMPSEC = (unitSystem == RoadRailBim::DesignSpeedDefinition::UnitSystem::SI) ? 
            speedQtyKPH.ConvertTo(m_msecUnitCP).GetMagnitude() : speedQtyMPH.ConvertTo(m_msecUnitCP).GetMagnitude();
        stationStartEndSpeeds.push_back({ 0.0, { speedQtyMPSEC, speedQtyMPSEC } });
        }

    auto designCriteriaCPtr = RoadRailBim::PathwayDesignCriteria::Query(pathway);
    if (designCriteriaCPtr.IsNull())
        designCriteriaCPtr = RoadRailBim::PathwayDesignCriteria::Insert(pathway);

    if (params.domainModelsPrivate)
        {
        if (!designCriteriaCPtr->GetDesignCriteriaModel()->IsPrivate())
            {
            designCriteriaCPtr->GetDesignCriteriaModel()->SetIsPrivate(true);
            designCriteriaCPtr->GetDesignCriteriaModel()->Update();
            }
        }

    auto existingSpeeds = designCriteriaCPtr->QueryOrderedDesignSpeedIds();
    if (existingSpeeds.size() == stationStartEndSpeeds.size())
        return; // TODO - compare existing vs. new speeds -> insert/update/delete

    for (auto rIter = stationStartEndSpeeds.rbegin(); rIter != stationStartEndSpeeds.rend(); rIter++)
        {
        auto& stationStartEndSpeed = *rIter;

        auto designSpeedPtr = RoadRailBim::DesignSpeed::Create(
            RoadRailBim::DesignSpeed::CreateFromToParams(*designCriteriaCPtr, *alignmentCPtr,
                *GetOrInsertDesignSpeedDefinition(standardsModel, stationStartEndSpeed.second.first, unitSystem),
                *GetOrInsertDesignSpeedDefinition(standardsModel, stationStartEndSpeed.second.second, unitSystem),
                stationStartEndSpeed.first, lastStartStation));

        designSpeedPtr->Insert();

        lastStartStation = stationStartEndSpeed.first;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::CreateNewCorridor(
    CorridorCR cifCorridor, 
    ORDConverter::Params& params,
    bool isRail, 
    iModelBridgeSyncInfoFile::ChangeDetector::Results const& change, RoadRailBim::CorridorCPtr& bimCorridorCPtr)
    {
    auto corridorPtr = RoadRailBim::Corridor::Create(*m_converter.GetRoadRailNetwork());

    ORDConverterUtils::AssignFederationGuid(*corridorPtr, cifCorridor.GetSyncId());

    if (!WString::IsNullOrEmpty(cifCorridor.GetName().c_str()))
        {
        Utf8String corridorName(cifCorridor.GetName().c_str());
        corridorPtr->SetUserLabel(corridorName.c_str());
        corridorPtr->SetCode(RoadRailBim::Corridor::CreateCode(*m_converter.GetRoadRailNetwork(), corridorName));
        }
    
    AssignCorridorGeomStream(cifCorridor, *corridorPtr);

    iModelBridgeSyncInfoFile::ConversionResults results;
    results.m_element = corridorPtr;
    if (BentleyStatus::SUCCESS != params.changeDetectorP->_UpdateBimAndSyncInfo(results, change))
        return BentleyStatus::ERROR;

    bimCorridorCPtr = RoadRailBim::Corridor::Get(corridorPtr->GetDgnDb(), corridorPtr->GetElementId());
    auto corridorModelPtr = PhysicalModel::Create(*bimCorridorCPtr);
    corridorModelPtr->SetIsPrivate(params.domainModelsPrivate);

    if (DgnDbStatus::Success != corridorModelPtr->Insert())
        return BentleyStatus::ERROR;

    auto corridorPortionsCPtr = RoadRailBim::CorridorPortions::Insert(*bimCorridorCPtr);
    if (params.domainModelsPrivate)
        {
        corridorPortionsCPtr->GetCorridorPortionsModel()->SetIsPrivate(true);
        corridorPortionsCPtr->GetCorridorPortionsModel()->Update();

        auto horizontalAlignmentsCPtr = AlignmentBim::HorizontalAlignments::Query(*corridorPortionsCPtr->GetCorridorPortionsModel());
        horizontalAlignmentsCPtr->GetHorizontalModel()->SetIsPrivate(true);
        horizontalAlignmentsCPtr->GetHorizontalModel()->Update();
        }

    RoadRailBim::PathwayElementPtr pathwayPtr;
    if (isRail)
        pathwayPtr = RoadRailBim::Railway::Create(*corridorPortionsCPtr, RoadRailBim::PathwayElement::Order::LeftMost);
    else
        pathwayPtr = RoadRailBim::Roadway::Create(*corridorPortionsCPtr, RoadRailBim::PathwayElement::Order::LeftMost);

    if (pathwayPtr->Insert().IsNull())
        return BentleyStatus::ERROR;    

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ORDCorridorsConverter::UpdateCorridor(CorridorCR cifCorridor,
    iModelBridgeSyncInfoFile::ChangeDetector& changeDetector, iModelBridgeSyncInfoFile::ChangeDetector::Results const& change)
    {
    auto corridorPtr = RoadRailBim::Corridor::GetForEdit(changeDetector.GetDgnDb(), change.GetSyncInfoRecord().GetDgnElementId());

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
        if (BentleyStatus::SUCCESS != CreateNewCorridor(cifCorridor, params, isRail, change, bimCorridorCPtr))
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

    m_aspectAssignFuncs.push_back(&ConvertORDElementXDomain::AssignCorridorSurfaceAspect);
    m_aspectAssignFuncs.push_back(&ConvertORDElementXDomain::AssignLinearQuantityAspect);
    m_aspectAssignFuncs.push_back(&ConvertORDElementXDomain::AssignCorridorAspect);
    m_aspectAssignFuncs.push_back(&ConvertORDElementXDomain::AssignTemplateDropAspect);
    m_aspectAssignFuncs.push_back(&ConvertORDElementXDomain::AssignSuperelevationAspect);
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
            if (cifAlignmentPtr.IsValid())
                m_converter.m_cifGeneratedLinear3ds.push_back(cifLinearEntity3dPtr);
            }
        else
            {
            cifCorridorPtr = Corridor::CreateFromElementHandle(v8el);
            }
        }

    if (cifAlignmentPtr.IsValid())
        {
        auto elmRefP = cifAlignmentPtr->GetElementHandle()->GetElementRef();
        if (m_alignmentV8RefSet.end() == m_alignmentV8RefSet.find(elmRefP))
            {
            m_alignmentV8RefSet.insert(elmRefP);
            m_converter.m_cifAlignments.push_back(cifAlignmentPtr);
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
    auto description = Utf8String(cifCorridorSurface.GetDescription().c_str());

    if (auto corridorSurfaceAspectP = DgnV8ORDBim::CorridorSurfaceAspect::GetP(element))
        {
        corridorSurfaceAspectP->SetIsTopMesh(isTopMesh);
        corridorSurfaceAspectP->SetIsBottomMesh(isBottomMesh);
        corridorSurfaceAspectP->SetDescription(description.c_str());
        }
    else
        {
        auto corridorSurfaceAspectPtr = DgnV8ORDBim::CorridorSurfaceAspect::Create(isTopMesh, isBottomMesh, description.c_str());
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
    double surfaceAreaVal = surfaceArea.IsValid() ? surfaceArea.Value() / squaredUorsPerMeter : NAN;
    if (auto volumetricQuantityAspectP = DgnV8ORDBim::VolumetricQuantityAspect::GetP(element))
        {
        volumetricQuantityAspectP->SetSurfaceArea(surfaceAreaVal);
        volumetricQuantityAspectP->SetVolume(volumeVal);
        }
    else
        {
        auto volumetricQuantityAspectPtr = DgnV8ORDBim::VolumetricQuantityAspect::Create(volumeVal, surfaceAreaVal);
        DgnV8ORDBim::VolumetricQuantityAspect::Set(element, *volumetricQuantityAspectPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvertORDElementXDomain::AssignLinearQuantityAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const
    {
    auto cifConsensusItemPtr = Linear3dConsensusItem::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
    if (auto cifLinear3dCP = dynamic_cast<Linear3dConsensusItemCP>(cifConsensusItemPtr.get()))
        {
        assignORDFeatureAspect(element, *cifConsensusItemPtr);
        assignQuantityAspect(element, *cifLinear3dCP);
        return true;
        }
    else
        {
        auto cifAlignmentPtr = Alignment::CreateFromElementHandle(v8el);
        if (cifAlignmentPtr.IsValid())
            {
            assignORDFeatureAspect(element, *cifAlignmentPtr);
            assignQuantityAspect(element, *cifAlignmentPtr);
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
    auto templateDropPtr = TemplateDrop::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
    if (templateDropPtr.IsValid())
        {
        // TemplateDrops no longer seem to have access to a feature definition in the CIF SDK.  Leaving 
        // this line commented out until we get confirmation that this wasn't a mistake from the ORD team.
        //assignORDFeatureAspect(element, *templateDropPtr);
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
    auto superElevationPtr = SuperElevation::CreateFromElementHandle(v8el);
    if (superElevationPtr.IsValid())
        {
        // The following check is due to an null pointer error in CIF when calling GetEndDistance().
        // This check is a band-aid fix for now to avoid getting into trouble.
        if (superElevationPtr->GetParentSection()->GetAlignment() != nullptr)
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
    auto cifCorridorPtr = Corridor::CreateFromElementHandle(v8el);
    if (cifCorridorPtr.IsValid())
        {
        assignCorridorAspect(element, *cifCorridorPtr);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvertORDElementXDomain::AssignCorridorSurfaceAspect(Dgn::DgnElementR element, DgnV8EhCR v8el) const
    {
    auto featurizedPtr = FeaturizedConsensusItem::CreateFromElementHandle(*m_cifConsensusConnection, v8el);
    if (featurizedPtr.IsValid())
        {
        if (element.GetModel()->Is3d())
            {
            if (auto cifCorridorSurfaceCP = dynamic_cast<CorridorSurfaceCP>(featurizedPtr.get()))
                {
                assignORDFeatureAspect(element, *featurizedPtr);
                assignCorridorSurfaceAspect(element, *cifCorridorSurfaceCP);
                assignStationRangeAspect(element, *cifCorridorSurfaceCP);
                assignQuantityAspect(element, *cifCorridorSurfaceCP);
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
    if (m_converter.m_v8ToBimElmMap.end() != m_converter.m_v8ToBimElmMap.find(v8el.GetElementRef()))
        return;

    for (auto& func : m_aspectAssignFuncs)
        {
        if ((this->*func)(*elRes.m_element, v8el))
            break;
        }

    m_converter.m_v8ToBimElmMap.insert({ v8el.GetElementRef(), elRes.m_element.get() });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId convertToSpatialCategory(SubjectCR subject, DrawingCategoryCR drawingCategory)
    {
    auto configurationModelPtr = AlignmentBim::RoadRailAlignmentDomain::QueryConfigurationModel(subject);
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
void ORDConverter::CreateAlignments()
    {
    if (m_cifAlignments.empty())
        return;

    // There may be duplicate CifAlignments found via various V8 models. Some of them will have
    // the same ElementRef, but others will not, so relying on SyncId to tell whether two
    // CifAlignments are the same. Also, in some cases, some duplicate CifAlignments have
    // a blank name, so leaving the one with a name in front during sorting - that will be the
    // one processed later - the other duplicates will be ignored.
    std::sort(m_cifAlignments.begin(), m_cifAlignments.end(), 
        [](Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Alignment> const& a, Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Alignment> const& b)
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
        if (lastAlignmentRefP == cifAlignmentPtr->GetElementHandle()->GetElementRef())
            continue;

        if (0 == lastSyncId.CompareTo(cifAlignmentPtr->GetSyncId()))
            continue;

        auto bimAlignmentId = alignmentsConverterPtr->ConvertAlignment(*cifAlignmentPtr, *m_ordParams);
        lastSyncId = cifAlignmentPtr->GetSyncId();
        lastAlignmentRefP = cifAlignmentPtr->GetElementHandle()->GetElementRef();
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
        [](Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Corridor> const& a, Bentley::RefCountedPtr<Bentley::Cif::GeometryModel::SDK::Corridor> const& b)
        { return a->GetElementHandle()->GetElementRef() > b->GetElementHandle()->GetElementRef(); });

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

        bool isRail = bimElmPtr.IsValid() && railCorridorIds.find(bimElmPtr->GetElementId()) != railCorridorIds.end();
        auto bimCorridorId = corridorsConverterPtr->ConvertCorridor(*cifCorridorPtr, *m_ordParams, isRail);
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
* @bsimethod                                    Diego.Diaz                      05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::SetCorridorDesignAlignments()
    {
    if (m_cifCorridors.empty())
        return;

    ORDCorridorsConverterPtr corridorConverterPtr;
    for (auto& cifCorridorPtr : m_cifCorridors)
        {
        ORDCorridorsConverter::CifCorridorSourceItem corridorItem(*cifCorridorPtr);

        RoadRailBim::CorridorCPtr corridorCPtr;
        auto elementAndAspectId =
            iModelExternalSourceAspect::FindElementBySourceId(GetDgnDb(), DgnElementId(m_ordParams->fileScopeId), corridorItem.Kind(), corridorItem._GetId());
        if (elementAndAspectId.aspectId.IsValid() && elementAndAspectId.elementId.IsValid())
            {
            corridorCPtr = RoadRailBim::Corridor::Get(GetDgnDb(), elementAndAspectId.elementId);
            if (corridorCPtr.IsNull())
                continue;
            }

        AlignmentBim::AlignmentPtr bimMainAlignmentPtr;
        auto cifAlignmentPtr = cifCorridorPtr->GetCorridorAlignment();
        if (cifAlignmentPtr.IsValid())
            {
            ORDAlignmentsConverter::CifAlignmentSourceItem alignmentItem(*cifAlignmentPtr);

            iModelExternalSourceAspect::ElementAndAspectId elementAndAspectId =
                iModelExternalSourceAspect::FindElementBySourceId(GetDgnDb(), DgnElementId(m_ordParams->fileScopeId), alignmentItem.Kind(), alignmentItem._GetId());
            if (elementAndAspectId.elementId.IsValid())
                {
                bimMainAlignmentPtr = AlignmentBim::Alignment::GetForEdit(GetDgnDb(), elementAndAspectId.elementId);
                if (bimMainAlignmentPtr.IsValid())
                    {
                    BeAssert(!bimMainAlignmentPtr->GetSource().IsValid()); // Alignment already associated with another Corridor
                    bimMainAlignmentPtr->SetSource(corridorCPtr.get());
                    bimMainAlignmentPtr->Update();

                    auto corridorPtr = corridorCPtr->MakeCopy<RoadRailBim::Corridor>();
                    corridorPtr->SetDesignAlignment(bimMainAlignmentPtr.get());
                    corridorPtr->Update();

                    auto corridorPortionsCPtr = RoadRailBim::CorridorPortions::Query(*corridorPtr);
                    for (auto pathwayId : corridorPortionsCPtr->QueryOrderedPathwayIds())
                        {
                        auto pathwayPtr = RoadRailBim::PathwayElement::GetForEdit(GetDgnDb(), pathwayId);
                        pathwayPtr->SetDesignAlignment(bimMainAlignmentPtr.get());
                        pathwayPtr->Update();

                        SpeedTablePtr speedTablePtr;
                        //SpeedTablePtr speedTablePtr = corridorAlgPtr->GetActiveSpeedTable();

                        Dgn::DefinitionModelCPtr standardsModelCPtr;
                        if (pathwayPtr->ToRailway())
                            standardsModelCPtr = RoadRailBim::RailwayStandardsModelUtilities::Query(GetJobSubject());
                        else
                            standardsModelCPtr = RoadRailBim::RoadwayStandardsModelUtilities::Query(GetJobSubject());

                        if (corridorConverterPtr.IsNull())
                            corridorConverterPtr = ORDCorridorsConverter::Create(*this, ComputeUnitsScaleTransform(*cifCorridorPtr->GetDgnModelP()));

                        corridorConverterPtr->ConvertSpeedTable(speedTablePtr.get(), 0, *pathwayPtr, *standardsModelCPtr,
                            (m_ordParams->rootModelUnitSystem == Dgn::UnitSystem::Metric) ?
                            RoadRailBim::DesignSpeedDefinition::UnitSystem::SI : RoadRailBim::DesignSpeedDefinition::UnitSystem::Imperial,
                            *m_ordParams);
                        }
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ORDConverter::AssociateGeneratedAlignments()
    {
    auto roadwayStandardsModelPtr = RoadRailBim::RoadwayStandardsModelUtilities::Query(GetJobSubject());

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
            auto corridorPortionsCPtr = RoadRailBim::CorridorPortions::Query(*corridorCPtr);
            if (corridorPortionsCPtr.IsNull())
                continue;

                auto pathwayIds = corridorPortionsCPtr->QueryOrderedPathwayIds();
                if (pathwayIds.empty())
                    continue;

            auto pathwayId = pathwayIds.front();
            if (!pathwayId.IsValid())
                continue;

            auto pathwayCPtr = RoadRailBim::PathwayElement::Get(GetDgnDb(), pathwayId);
            if (pathwayCPtr.IsValid())
                {
                auto bimAlignmentPtr = bimAlignmentCPtr->MakeCopy<AlignmentBim::Alignment>();
                bimAlignmentPtr->SetParentId(pathwayCPtr->GetElementId(), 
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
    CreatePathways();
    CreateAlignments();
    SetCorridorDesignAlignments();
    AssociateGeneratedAlignments();

    auto roadRailNetworkModelPtr = GetRoadRailNetwork()->GetNetworkModel();
    auto designAlignmentsCPtr = AlignmentBim::DesignAlignments::Query(*roadRailNetworkModelPtr);
    auto designAlignmentModelPtr = designAlignmentsCPtr->GetAlignmentModel();
    auto designHorizontalAlignmentsCPtr = AlignmentBim::HorizontalAlignments::Query(*designAlignmentModelPtr);
    auto designHorizAlignmentModelPtr = designHorizontalAlignmentsCPtr->GetHorizontalModel();

    updateProjectExtents(*designHorizAlignmentModelPtr, *m_ordParams, false);
    updateProjectExtents(*roadRailNetworkModelPtr, *m_ordParams, true);
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
void ORDConverter::SetUpModelFormatters()
    {    
    auto roadRailNetworkModelPtr = GetRoadRailNetwork()->GetNetworkModel();
    auto designAlignmentsCPtr = AlignmentBim::DesignAlignments::Query(*roadRailNetworkModelPtr);
    auto designAlignmentModelPtr = designAlignmentsCPtr->GetAlignmentModel();

    DgnV8Api::ModelInfo const& v8ModelInfo = _GetModelInfo(*GetRootModelP());

    setUpModelFormatter(*designAlignmentModelPtr, v8ModelInfo);
    setUpModelFormatter(*roadRailNetworkModelPtr, v8ModelInfo);

    designAlignmentModelPtr->Update();
    roadRailNetworkModelPtr->Update();
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

END_ORDBRIDGE_NAMESPACE