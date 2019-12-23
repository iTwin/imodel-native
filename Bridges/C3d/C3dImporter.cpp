/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    "C3dImporter.h"
#include    "C3dHelper.h"

BEGIN_C3D_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          06/19
+===============+===============+===============+===============+===============+======*/
struct C3dProtocolExtensions
{
public:
    OdStaticRxObject<AeccAlignmentExt>  m_aeccAlignmentExt;
    OdStaticRxObject<AeccCorridorExt>   m_aeccCorridorExt;
    C3dProtocolExtensions ()
        {
        try
            {
            AECCDbAlignment::rxInit ();
            AeccAlignmentExt::RxInit ();
            //m_aeccAlignmentExt = AeccAlignmentExt::CreateObject ();
            DwgRxClass::AddProtocolExtension (AECCDbAlignment::desc(), AeccAlignmentExt::Desc(), &m_aeccAlignmentExt);

            AECCDbCorridor::rxInit ();
            AeccCorridorExt::RxInit ();
            //m_aeccCorridorExt = AeccCorridorExt::CreateObject ();
            DwgRxClass::AddProtocolExtension (AECCDbCorridor::desc(), DwgProtocolExtension::Desc(), &m_aeccCorridorExt);
            }
        catch (OdError& error)
            {
            LOG.debugv ("Toolkit Exception: %ls", error.description().c_str());
            }
        }

    ~C3dProtocolExtensions ()
        {
        DwgRxClass::DeleteProtocolExtension (AECCDbAlignment::desc(), AeccAlignmentExt::Desc());
        AeccAlignmentExt::RxUnInit ();
        AECCDbAlignment::rxUninit ();

        DwgRxClass::DeleteProtocolExtension (AECCDbCorridor::desc(), AeccCorridorExt::Desc());
        AeccCorridorExt::RxUnInit ();
        AECCDbCorridor::rxUninit ();
        }

};  // C3dProtocolExtensions
static C3dProtocolExtensions*   s_c3dProtocolExtensions;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
C3dImporter::C3dImporter (DwgImporter::Options& options) : T_Super(options)
    {
    s_c3dProtocolExtensions = new C3dProtocolExtensions ();

    this->ParseC3dConfigurations ();

    m_c3dSchema = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
C3dImporter::~C3dImporter ()
    {
    if (s_c3dProtocolExtensions != nullptr)
        {
        delete s_c3dProtocolExtensions;
        s_c3dProtocolExtensions = nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dImporter::ParseC3dConfigurations ()
    {
#ifdef DEBUG
    m_c3dOptions.SetAlignedModelPrivate (false);
#endif

    BeXmlDomP   xmlDom = T_Super::m_config.GetDom ();
    if (nullptr == xmlDom)
        return;

    BeXmlDom::IterableNodeSet   nodes;
    xmlDom->SelectNodes (nodes, "/ConvertConfig/Options/OptionBool", nullptr);
    for (auto node : nodes)
        {
        Utf8String  str;
        bool        boolValue = false;
        if (BeXmlStatus::BEXML_Success == node->GetAttributeStringValue(str, "name") && BeXmlStatus::BEXML_Success == node->GetAttributeBooleanValue(boolValue, "value"))
            {
            if (str.EqualsI("SetAlignedModelPrivate"))
                m_c3dOptions.SetAlignedModelPrivate (boolValue);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr C3dImporter::GetAlignmentSubject ()
    {
    auto const& jobSubject = T_Super::GetJobSubject ();
    auto alignCode = Subject::CreateCode (jobSubject, CIVIL_ALIGNED_SUBJECT);
    auto alignedId = T_Super::GetDgnDb().Elements().QueryElementIdByCode (alignCode);
    if (!alignedId.IsValid())
        return  nullptr;
    return  T_Super::GetDgnDb().Elements().Get<Subject>(alignedId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String C3dImporter::_ComputeImportJobName (DwgDbBlockTableRecordCR modelspaceBlock) const
    {
    Utf8String  jobName = T_Super::GetOptions().GetBridgeJobName ();
    if (jobName.empty())
        jobName.assign ("Civil3d");
    return  jobName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus C3dImporter::OnBaseBridgeJobInitialized (DgnElementId jobId)
    {
    auto alignSubject = this->GetAlignmentSubject ();
    if (!alignSubject.IsValid())
        {
        auto const& jobSubject = T_Super::GetJobSubject ();
        auto alignCode = Subject::CreateCode (jobSubject, CIVIL_ALIGNED_SUBJECT);
        auto newSubject = Subject::Create (jobSubject, alignCode.GetValueUtf8CP());
        if (!newSubject.IsValid())
            {
            T_Super::ReportError (IssueCategory::Briefcase(), Issue::Message(), Utf8PrintfString("Failed to insert Alignment subject %s!", alignCode.GetValueUtf8CP()).c_str());
            return  BentleyStatus::BSIERROR;
            }

        Json::Value props(Json::nullValue);
        props["Perspective"] = "Aligned";
        newSubject->SetJsonProperties (Subject::json_Model(), props);
        alignSubject = dynamic_cast<SubjectCP>(newSubject->Insert().get());
        }
    if (!alignSubject.IsValid())
        {
        T_Super::ReportError (IssueCategory::Briefcase(), Issue::Message(), "Failed to get the Alignment subject!");
        return  BentleyStatus::BSIERROR;
        }

    RoadRailAlignment::RoadRailAlignmentDomain::OnSchemaImported (*alignSubject);
    RoadRailPhysical::RoadRailPhysicalDomain::OnSchemaImported (*alignSubject);

    RoadRailAlignment::RoadRailAlignmentDomain::SetUpDefinitionPartitions (*alignSubject);
    RailPhysical::RailPhysicalDomain::SetUpDefinitionPartition (*alignSubject);
    RoadPhysical::RoadPhysicalDomain::SetUpDefinitionPartition (*alignSubject);

    auto alignedPartition = RoadRailPhysical::PhysicalModelUtilities::CreateAndInsertPhysicalPartitionAndModel (*alignSubject, ALIGNMENTS_PARTITION_NAME);
    if (!alignedPartition.IsValid())
        {
        T_Super::ReportError (IssueCategory::Unknown(), Issue::CantCreateModel(), alignSubject->GetDisplayLabel().c_str());
        return  BentleyStatus::BSIERROR;
        }

    auto& db = T_Super::GetDgnDb ();
    iModelBridge::InsertElementHasLinksRelationship (db, alignedPartition->GetElementId(), T_Super::GetRepositoryLink(nullptr));

    auto roadNetwork = RoadPhysical::RoadNetwork::Insert (*alignedPartition->GetSubModel()->ToPhysicalModelP(), ROADNETWORK_MODEL_NAME);
    auto railNetwork = RailPhysical::RailNetwork::Insert (*alignedPartition->GetSubModel()->ToPhysicalModelP(), RAILNETWORK_MODEL_NAME);
    if (!roadNetwork.IsValid() || !railNetwork.IsValid())
        {
        T_Super::ReportError (IssueCategory::Unknown(), Issue::Message(), Utf8PrintfString("Error creating road/rail network in the Aligned model %s!", alignSubject->GetDisplayLabel().c_str()).c_str());
        return  BentleyStatus::BSIERROR;
        }

    m_roadNetworkModel = roadNetwork->GetNetworkModel ();
    m_railNetworkModel = railNetwork->GetNetworkModel ();

    auto designAlignments = RoadRailAlignment::DesignAlignments::Insert (*m_roadNetworkModel, DESIGNALIGNMENTS_NAME);
    if (!designAlignments.IsValid())
        {
        T_Super::ReportError (IssueCategory::Unknown(), Issue::Message(), Utf8PrintfString("Error inserting road network model %s!", alignSubject->GetDisplayLabel().c_str()).c_str());
        return  BentleyStatus::BSIERROR;
        }

    m_alignmentModel = designAlignments->GetAlignmentModel ();
    if (!m_alignmentModel.IsValid())
        return  BentleyStatus::BSIERROR;

    if (m_c3dOptions.IsAlignedModelPrivate())
        {
        alignedPartition->GetSubModel()->SetIsPrivate (true);
        alignedPartition->GetSubModel()->Update ();
        m_roadNetworkModel->SetIsPrivate (true);
        m_roadNetworkModel->Update ();
        m_railNetworkModel->SetIsPrivate (true);
        m_railNetworkModel->Update ();

        auto horizontalAlignments = RoadRailAlignment::HorizontalAlignments::Query (*m_roadNetworkModel);
        if (horizontalAlignments.IsValid())
            {
            horizontalAlignments->GetHorizontalModel()->SetIsPrivate (true);
            horizontalAlignments->GetHorizontalModel()->Update ();
            }

        horizontalAlignments = RoadRailAlignment::HorizontalAlignments::Query (*m_alignmentModel);
        if (horizontalAlignments.IsValid())
            {
            horizontalAlignments->GetHorizontalModel()->SetIsPrivate (true);
            horizontalAlignments->GetHorizontalModel()->Update();
            }

        m_alignmentModel->SetIsPrivate (true);
        m_alignmentModel->Update ();
        }

    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus C3dImporter::OnBaseBridgeJobFound (DgnElementId jobId)
    {
    auto alignSubject = this->GetAlignmentSubject ();
    if (!alignSubject.IsValid())
        {
        T_Super::ReportError (IssueCategory::Briefcase(), Issue::Message(), "Failed to get the Alignment subject!");
        return  BentleyStatus::BSIERROR;
        }

    m_roadNetworkModel = RoadRailPhysical::PhysicalModelUtilities::QueryRoadNetworkModel (*alignSubject, ALIGNMENTS_PARTITION_NAME, ROADNETWORK_MODEL_NAME);
    m_railNetworkModel = RoadRailPhysical::PhysicalModelUtilities::QueryRailNetworkModel (*alignSubject, ALIGNMENTS_PARTITION_NAME, RAILNETWORK_MODEL_NAME);

    if (!m_roadNetworkModel.IsValid() || !m_railNetworkModel.IsValid())
        {
        T_Super::ReportError (IssueCategory::Briefcase(), Issue::Message(), Utf8PrintfString("Error finding road/rail network under the Alignment subject %s!", alignSubject->GetDisplayLabel().c_str()).c_str());
        return  BentleyStatus::BSIERROR;
        }

    auto designAlignments = RoadRailAlignment::DesignAlignments::Query (*m_roadNetworkModel, DESIGNALIGNMENTS_NAME);
    if (!designAlignments.IsValid())
        {
        T_Super::ReportError (IssueCategory::Briefcase(), Issue::Message(), "Error finding the Alignments element %s!");
        return  BentleyStatus::BSIERROR;
        }

    auto m_alignmentModel = designAlignments->GetAlignmentModel ();
    if (!m_alignmentModel.IsValid())
        return  BentleyStatus::BSIERROR;

    return  BentleyStatus::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool    C3dImporter::_FilterEntity (ElementImportInputsR inputs) const
    {
    // vertical alignments are processed together with horizontal alignments
    if (inputs.GetEntity().isKindOf(AECCDbVAlignment::desc()))
        return  true;

    return  T_Super::_FilterEntity(inputs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   C3dImporter::_ImportEntity (ElementImportResultsR results, ElementImportInputsR inputs)
    {
    // override this only because so we can call it from C3D protocol extensions
    return T_Super::_ImportEntity (results, inputs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
void    C3dImporter::_SetChangeDetector (bool updating)
    {
    // take over the UpdaterChangeDetector
    if (updating)
        m_changeDetector.reset (new C3dUpdaterChangeDetector());
    else
        T_Super::_SetChangeDetector (updating);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
void    C3dUpdaterChangeDetector::_DeleteElement (DgnDbR db, DwgSourceAspects::ObjectAspectCR elementAspect)
    {
    // the base change detector has determined that the imported element needs to be deleted - delete the referenced civil element first
    auto civilElementId = C3dHelper::GetCivilReferenceElementId(elementAspect);
    if (civilElementId.IsValid())
        {
        LOG.tracev ("Delete referenced civil element %lld", civilElementId.GetValue());
        db.Elements().Delete (civilElementId);
        }
    // the base change detector proceeds to delete imported element
    T_Super::_DeleteElement (db, elementAspect);
    }

END_C3D_NAMESPACE
