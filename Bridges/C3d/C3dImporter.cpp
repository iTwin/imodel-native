/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "C3dInternal.h"

BEGIN_C3D_NAMESPACE

static C3dProtocolExtensions*   s_c3dProtocolExtensions;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
C3dImporter::C3dImporter (DwgImporter::Options& options) : T_Super(options)
    {
    // add C3D protocol extensions
    s_c3dProtocolExtensions = new C3dProtocolExtensions ();

    m_entitiesNeedProxyGeometry.clear ();

    this->ParseC3dConfigurations ();

    m_c3dSchema = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
C3dImporter::~C3dImporter ()
    {
    // remove C3D protocol extensions
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
#ifdef HIDE_ALIGNMENT_MODELS
    m_c3dOptions.SetAlignedModelPrivate (false);
#endif

    BeXmlDomP   xmlDom = T_Super::m_config.GetDom ();
    if (nullptr == xmlDom)
        return;

    BeXmlDom::IterableNodeSet   nodes;
    Utf8String  strValue;

    xmlDom->SelectNodes (nodes, "/ConvertConfig/UseProxyGeometry/Entity", nullptr);
    for (auto node : nodes)
        {
        if (BeXmlStatus::BEXML_Success == node->GetAttributeStringValue(strValue, "name") && !strValue.empty())
            m_entitiesNeedProxyGeometry.insert (strValue.ToUpper());
        }   

    xmlDom->SelectNodes (nodes, "/ConvertConfig/Options/OptionBool", nullptr);
    for (auto node : nodes)
        {
        bool        boolValue = false;
        if (BeXmlStatus::BEXML_Success == node->GetAttributeStringValue(strValue, "name") && BeXmlStatus::BEXML_Success == node->GetAttributeBooleanValue(boolValue, "value"))
            {
            if (strValue.EqualsI("SetAlignedModelPrivate"))
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

        horizontalAlignments = RoadRailAlignment::HorizontalAlignments::Query (*m_railNetworkModel);
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

    RoadRailAlignment::RoadRailAlignmentDomain::SetParentSubject (*alignSubject);

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
        {
        // however, if a VAlignment is not changed when updating, get and record existing elements mapped from it, so they won't be deleted
        if (this->IsUpdating())
            {
            IDwgChangeDetector::DetectionResults    detectionResults;
            DwgDbObjectP    object = DwgDbObject::Cast (inputs.GetEntityP());
            C3dImporter*    importer = const_cast<C3dImporter*> (this);
            if (object != nullptr && importer != nullptr)
                {
                auto& changeDetector = importer->GetChangeDetector ();
                if (!changeDetector._IsElementChanged(detectionResults, *importer, *object, inputs.GetModelMapping()))
                    {
                    auto existingElements = iModelExternalSourceAspect::GetSelectFromSameSource(this->GetDgnDb(), detectionResults.GetObjectAspect(), BeSQLite::EC::IECSqlBinder::MakeCopy::No);
                    while(BE_SQLITE_ROW == existingElements->Step())
                        changeDetector._OnElementSeen (*importer, existingElements->GetValueId<DgnElementId>(0));
                    }
                }
            }
        return  true;
        }

    return  T_Super::_FilterEntity(inputs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   C3dImporter::_ImportEntity (ElementImportResultsR results, ElementImportInputsR inputs)
    {
    /*-----------------------------------------------------------------------------------
    Below code is only needed as a workaround for some AECC entities which are known to be 
    incorrectly drawn by the Civil toolkit.  We filter these entities in or out on a case 
    by case.  After we have obtained an ODA fix for an entity, update the filter below.
    -----------------------------------------------------------------------------------*/
    auto sourceEntity = inputs.GetEntityP ();
    if (sourceEntity != nullptr && !m_entitiesNeedProxyGeometry.empty())
        {
        Utf8String  entityName(reinterpret_cast<WCharCP>(sourceEntity->GetDxfName().c_str()));

        auto found = m_entitiesNeedProxyGeometry.find (entityName.ToUpper());
        if (found != m_entitiesNeedProxyGeometry.end())
            {
            // convert the enabled object as a proxy entity:
            OdDbProxyEntityPtr  proxy = ::odEntityToProxy (*sourceEntity);
            if (!proxy.isNull())
                {
                // create geometrical element(s) from the proxy entity:
                ElementImportInputs proxyInputs(inputs.GetTargetModelR(), DwgDbEntity::Cast(proxy.detach()), inputs);
                // use the source entity name as user label
                proxyInputs.SetElementLabel (entityName);

                // import the new non-database resident entity
                auto status = T_Super::ImportNewEntity (results, proxyInputs, sourceEntity->GetOwnerId());

                // count as a database-resident entity
                if (status == BentleyStatus::BSISUCCESS)
                    m_entitiesImported++;
                return  status;
                }
            }
        }

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
