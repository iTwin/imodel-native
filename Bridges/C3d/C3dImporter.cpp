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
    m_entitiesForPostImport.clear ();

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
    // create & add Aligned json property to the Civil domain subject
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

    // add AppGraphics json property into the base bridge's hierarchy subject for Civil tools
    auto hierarchySubject =T_Super::GetJobHierarchySubject ();
    if (hierarchySubject.IsValid())
        {
        SubjectPtr  subjectEdit = T_Super::GetDgnDb().Elements().GetForEdit<Subject>(hierarchySubject->GetElementId());
        if (subjectEdit.IsValid())
            {
            auto json = subjectEdit->GetJsonProperties (Subject::json_Model());
            json["Perspective"] = "AppGraphics";
            subjectEdit->SetJsonProperties (Subject::json_Model(), json);
            subjectEdit->SetUserLabel (CIVIL3D_DEFAULT_SUBJECT);
            subjectEdit->Update ();
            }
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

    auto roadrailPhysical = alignedPartition->GetSubModel()->ToPhysicalModelP ();
    if (roadrailPhysical == nullptr)
        return  BentleyStatus::BSIERROR;

    auto roadNetwork = RoadPhysical::RoadNetwork::Insert (*roadrailPhysical, ROADNETWORK_MODEL_NAME);
    auto railNetwork = RailPhysical::RailNetwork::Insert (*roadrailPhysical, RAILNETWORK_MODEL_NAME);
    if (!roadNetwork.IsValid() || !railNetwork.IsValid())
        {
        T_Super::ReportError (IssueCategory::Unknown(), Issue::Message(), Utf8PrintfString("Error creating road/rail network in the Aligned model %s!", alignSubject->GetDisplayLabel().c_str()).c_str());
        return  BentleyStatus::BSIERROR;
        }

    m_roadNetworkModel = roadNetwork->GetNetworkModel ();
    m_railNetworkModel = railNetwork->GetNetworkModel ();
    if (!m_roadNetworkModel.IsValid() || !m_railNetworkModel.IsValid())
        return  BentleyStatus::BSIERROR;

    // set the unit formatter of Civil domain models from the C3D model
    GeometricModelP c3dModel = nullptr;
    auto rootModel = T_Super::GetRootModel().GetModel ();
    if (rootModel != nullptr)
        c3dModel = rootModel->ToGeometricModelP ();

    auto civilModel = m_roadNetworkModel->ToGeometricModelP ();
    if (civilModel != nullptr && c3dModel != nullptr)
        civilModel->GetFormatterR() = c3dModel->GetFormatter ();

    auto designAlignments = RoadRailAlignment::DesignAlignments::Insert (*m_roadNetworkModel, DESIGNALIGNMENTS_NAME);
    if (!designAlignments.IsValid())
        {
        T_Super::ReportError (IssueCategory::Unknown(), Issue::Message(), Utf8PrintfString("Error inserting road network model %s!", alignSubject->GetDisplayLabel().c_str()).c_str());
        return  BentleyStatus::BSIERROR;
        }

    m_alignmentModel = designAlignments->GetAlignmentModel ();
    if (!m_alignmentModel.IsValid())
        return  BentleyStatus::BSIERROR;

    if ((civilModel = m_alignmentModel->ToGeometricModelP()) != nullptr && c3dModel != nullptr)
        civilModel->GetFormatterR() = c3dModel->GetFormatter ();

    if (m_c3dOptions.IsAlignedModelPrivate())
        {
        roadrailPhysical->SetIsPrivate (true);
        roadrailPhysical->Update ();
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
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
void    C3dImporter::_PostProcessViewports ()
    {
    T_Super::_PostProcessViewports ();

    // if we have turned on road or rail network models, this is the time to add it to the modelspace view:
    bool showRoads = (m_roadNetworkModel.IsValid() && !m_roadNetworkModel->IsPrivate());
    bool showRails = (m_railNetworkModel.IsValid() && !m_railNetworkModel->IsPrivate());
    if (!showRoads && !showRails)
        return;

    for (auto aspect : DwgSourceAspects::ViewAspectIterator(*m_dgndb, this->GetRepositoryLink(m_dwgdb.get())))
        {
        if (aspect.GetSourceType() == DwgSourceAspects::ViewAspect::SourceType::ModelSpaceViewport)
            {
            auto view = ViewDefinition::Get (*m_dgndb, aspect.GetViewId());
            if (view.IsValid())
                {
                auto editView = view->LoadViewController ();
                if (editView.IsValid())
                    {
                    auto modelspaceView = editView->ToSpatialViewP ();
                    if (modelspaceView != nullptr)
                        {
                        auto& modelSelector = modelspaceView->GetSpatialViewDefinition().GetModelSelector ();
                        if (showRoads)
                            modelSelector.AddModel (m_roadNetworkModel->GetModelId());
                        if (showRails)
                            modelSelector.AddModel (m_railNetworkModel->GetModelId());
                        modelSelector.Update ();
                        break;
                        }
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
void C3dImporter::SetRenderableDefaultView ()
    {
    // set the modelspace viewport to a renderable visual style, and isometric view, to obtain mesh or Brep's
    static DVec3d   s_isoviewNormal = DVec3d::From (-0.577, -0.577, 0.577);
    DwgDbViewportTableRecordPtr vport(T_Super::GetDwgDb().GetActiveModelspaceViewportId(), DwgDbOpenMode::ForWrite);
    if (vport.OpenStatus() == DwgDbStatus::Success)
        {
        DwgDbDictionaryPtr  dictionary(T_Super::GetDwgDb().GetVisualStyleDictionaryId(), DwgDbOpenMode::ForRead);
        if (dictionary.OpenStatus() == DwgDbStatus::Success)
            {
            auto iter = dictionary->GetIterator ();
            if (iter.IsValid())
                {
                for (; !iter->Done(); iter->Next())
                    {
                    DwgDbVisualStylePtr visualStyle(iter->GetObjectId(), DwgDbOpenMode::ForRead);
                    if (visualStyle.OpenStatus() == DwgDbStatus::Success)
                        {
                        auto renderMode = DwgHelper::GetRenderModeFromVisualStyle (*visualStyle);
                        if (renderMode != RenderMode::Wireframe)
                            {
                            // first non-wireframe visual style found:
                            vport->SetVisualStyle (visualStyle->GetObjectId());
                            break;
                            }
                        }
                    }
                }
            vport->SetViewDirection (s_isoviewNormal);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/20
+---------------+---------------+---------------+---------------+---------------+------*/
bool C3dImporter::_AllowEntityMaterialOverrides (DwgDbEntityCR entity) const
    {
    // a workaround ODA's removing material from TIN - CIVIL-1203:
    if (entity.isKindOf(AECCDbSurface::desc()))
        return  false;
    return  T_Super::_AllowEntityMaterialOverrides(entity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr C3dImporter::_CreateElement (DgnElement::CreateParams& params, ElementImportInputsR inputs, size_t elementIndex)
    {
    // create a DgnElement that opts in for special handlings
    if (inputs.GetEntity().isKindOf(AECCDbCorridor::desc()))
        {
        auto corridorExt = static_cast<AeccCorridorExt*>(AeccCorridorExt::Cast(inputs.GetEntity().QueryX(AeccCorridorExt::Desc())));
        if (nullptr != corridorExt)
            return  corridorExt->CreateElement(params, inputs, elementIndex);
        }

    // fallback to creating elements that are either based on AeccBase or PhysicalObject
    return T_Super::_CreateElement(params, inputs, elementIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/20
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d    C3dImporter::_GetElementPlacementPoint (DwgDbEntityCR entity)
    {
    AECCDbGraph const* graph = AECCDbGraph::cast (&entity);
    if (graph != nullptr)
        {
        auto origin = graph->GetOrigin ();
        return  DPoint3d::From(origin.x, origin.y, origin.z);
        }

    AECCDbStructure const* structure = AECCDbStructure::cast (&entity);
    if (structure != nullptr)
        {
        auto origin = structure->GetLocation ();
        return  DPoint3d::From(origin.x, origin.y, origin.z);
        }

    AECCDbAlignment const* alignment = AECCDbAlignment::cast (&entity);
    if (alignment != nullptr)
        {
        auto ecs = alignment->getEcs ();
        auto point2d = alignment->GetBeginPointByIndex (0);
        OdGePoint3d point3d(point2d.x, point2d.y, 0);
        point3d.transformBy (ecs);
        return  DPoint3d::From(point3d.x, point3d.y, point3d.z);
        }

    return  T_Super::_GetElementPlacementPoint(entity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool    C3dImporter::_FilterEntity (ElementImportInputsR inputs)
    {
    // vertical alignments are processed together with horizontal alignments
    auto const& entity = inputs.GetEntity ();
    if (entity.isKindOf(AECCDbVAlignment::desc()))
        {
        // however, if a VAlignment is not changed when updating, get and record existing elements mapped from it, so they won't be deleted
        if (this->IsUpdating())
            {
            IDwgChangeDetector::DetectionResults    detectionResults;
            DwgDbObjectP    object = DwgDbObject::Cast (inputs.GetEntityP());
            if (object != nullptr)
                {
                auto& changeDetector = this->GetChangeDetector ();
                if (!changeDetector._IsElementChanged(detectionResults, *this, *object, inputs.GetModelMapping()))
                    {
                    auto existingElements = iModelExternalSourceAspect::GetSelectFromSameSource(this->GetDgnDb(), detectionResults.GetObjectAspect());
                    while(BE_SQLITE_ROW == existingElements->Step())
                        changeDetector._OnElementSeen (*this, existingElements->GetValueId<DgnElementId>(0));
                    }
                }
            }
        return  true;
        }
    else if (entity.isKindOf(AECCDbCorridor::desc()))
        {
        // AeccCorridors shall be post imported
        this->RegisterEntityForPostImport (entity.GetObjectId());
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
* @bsimethod                                                    Don.Fu          02/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus C3dImporter::_ProcessDetectionResults (IDwgChangeDetector::DetectionResultsR detected, ElementImportResultsR results, ElementImportInputsR inputs)
    {
    // process imported results by default
    auto status = T_Super::_ProcessDetectionResults (detected, results, inputs);

    // post process results as needed
    if (status == BentleyStatus::BSISUCCESS)
        {
        DwgDbEntityCR   entity = inputs.GetEntity ();
        if (entity.isKindOf(AECCDbAlignment::desc()))
            {
            // post-add relationships for the civil alignment vs the aecc element
            DgnElementId    alignmentId = C3dHelper::GetCivilReferenceElementId (results.GetExistingElement());
            if (alignmentId.IsValid())
                {
                auto&   db = T_Super::GetDgnDb ();
                // the header element
                AeccAlignmentExt::UpdateElementRepresentedBy (db, alignmentId, results.GetExistingElementId());
                // the child elements
                for (auto& child : results.m_childElements)
                    {
                    alignmentId = C3dHelper::GetCivilReferenceElementId (child.GetExistingElement());
                    if (alignmentId.IsValid())
                        AeccAlignmentExt::UpdateElementRepresentedBy (db, alignmentId, child.GetExistingElementId());
                    }
                }
            }
        else if (entity.isKindOf(AECCDbCorridor::desc()))
            {
            // post add or update relationships and other corridor dependents
            auto corridorExt = static_cast<AeccCorridorExt*>(AeccCorridorExt::Cast(inputs.GetEntity().QueryX(AeccCorridorExt::Desc())));
            if (nullptr != corridorExt)
                return  corridorExt->UpdateElement (results);
            }
        }
    return  status;
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
