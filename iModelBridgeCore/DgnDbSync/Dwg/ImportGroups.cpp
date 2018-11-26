/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportGroups.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
GroupFactory::GroupFactory (DwgImporter& importer, DwgDbGroupCR group) : m_importer(importer), m_dwgGroup(group)
    {
    m_dwgGroup.GetAllEntityIds (m_dwgMemberIds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet GroupFactory::FindAllElements (DwgDbObjectIdCR objectId) const
    {
    DgnElementIdSet found;
    DwgDbBlockReferencePtr  blockInsert(objectId, DwgDbOpenMode::ForRead);
    if (blockInsert.OpenStatus() == DwgDbStatus::Success && blockInsert->IsXAttachment())
        {
        // the member is an xRef insert - find the target DgnModel and add all elements from the model:
        DgnModelP   model = nullptr;
        ResolvedModelMapping modelMap = m_importer.FindModel (objectId, DwgSyncInfo::ModelSourceType::XRefAttachment);
        if (modelMap.IsValid() && nullptr != (model = modelMap.GetModel()))
            found = model->MakeIterator().BuildIdSet<DgnElementId> ();
        return  found;
        }

    // the member is a single entity - query all elements (potentially across models) mapped from it:
    if (!m_importer.GetSyncInfo().FindElements(found, objectId))
        found.clear ();

    return  found;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    GroupFactory::SetGroupName (GenericGroupR genericGroup) const
    {
    Utf8String  name(m_dwgGroup.GetName().c_str());
    genericGroup.SetUserLabel (name.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
GenericGroupPtr GroupFactory::CreateAndInsert () const
    {
    GenericGroupPtr genericGroup;
    if (m_dwgMemberIds.empty())
        return  genericGroup;

    // get the model for generic groups:
    auto groupModel = m_importer.GetDgnDb().Models().Get<GenericGroupModel>(m_importer.GetGroupModelId());
    if (!groupModel.IsValid())
        return  genericGroup;

    genericGroup = GenericGroup::Create (*groupModel);
    if (!genericGroup.IsValid())
        return  genericGroup;

    // set the group name as the user label
    this->SetGroupName (*genericGroup);

    // insert GenericGroup to bim now, as AddMember needs source & target element IDs.
    if (genericGroup->Insert().IsNull())
        {
        Utf8String  name(m_dwgGroup.GetName().c_str());
        m_importer.ReportError (IssueCategory::UnexpectedData(), Issue::Message(), Utf8PrintfString("Failed inserting group %lls!", name.c_str()).c_str());
        genericGroup = nullptr;
        }

    return  genericGroup;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GroupFactory::Update (GenericGroupR genericGroup) const
    {
    // build a memeber removal list from existing memeber ID's of the group:
    auto existingMembers = ElementGroupsMembers::QueryMembers (genericGroup);

    // walk through DWG members
    for (auto objectId : m_dwgMemberIds)
        {
        // query all BIM elements mapped from this entity, potentially in multiple DgnModel's:
        DgnElementIdSet elementIds = this->FindAllElements (objectId);

        // add new members to GenericGroup, and remove existing members from the removal list:
        for (auto elementId : elementIds)
            {
            // if the element already exists in the group, no need to insert - remove it from the existing list:
            if (existingMembers.find(elementId) != existingMembers.end())
                {
                existingMembers.erase (elementId);
                continue;
                }
            // otherwise insert a new member into the group:
            auto member = m_importer.GetDgnDb().Elements().GetElement (elementId);
            if (member.IsValid())
                genericGroup.AddMember (*member);
            }
        }

    // the remaining entries in existing list are not seen in the new group - remove them:
    for (auto elementId : existingMembers)
        {
        auto member = m_importer.GetDgnDb().Elements().GetElement (elementId);
        if (member.IsValid())
            genericGroup.RemoveMember (*member);
        }

    // update the group name
    this->SetGroupName (genericGroup);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr   GroupFactory::Create () const
    {
    // currently GenericGroup requires pre-insertion for the source element ID to be valid for memebers:
    auto genericGroup = this->CreateAndInsert ();
    if (!genericGroup.IsValid())
        return  genericGroup;

    size_t  numExpected = m_dwgMemberIds.size ();
    size_t  numImported = 0;

    // walk through member entities:
    for (auto objectId : m_dwgMemberIds)
        {
        // query all BIM elements mapped from this entity, potentially in multiple DgnModel's:
        DgnElementIdSet elementIds = this->FindAllElements (objectId);

        // add them all in the same GenericGroup:
        for (auto elementId : elementIds)
            {
            auto member = m_importer.GetDgnDb().Elements().GetElement (elementId);
            if (member.IsValid())
                genericGroup->AddMember (*member);
            }
        numImported++;
        }

    if (numImported < numExpected)
        {
        Utf8PrintfString warn("%d memebers were created out of %d members", numExpected, numImported);
        m_importer.ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::UnexpectedData(), Issue::Message(), Utf8PrintfString("Group %ls", m_dwgGroup.GetName().c_str()).c_str(), warn.c_str());
        }

    return  genericGroup;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_OnUpdateGroup (DwgSyncInfo::Group const& oldProvenance, DwgDbGroupCR dwgGroup)
    {
    if (!oldProvenance.IsValid())
        return  BSIERROR;

    auto dwg = dwgGroup.GetDatabase ();
    if (!dwg.IsValid())
        return  BSIERROR;

    auto groupId = oldProvenance.GetDgnElementId ();
    auto& detector = this->_GetChangeDetector ();
    auto addMembers = this->_ShouldSyncGroupWithMembers ();

    DwgSyncInfo::Group  newProvenance(groupId, DwgSyncInfo::DwgFileId::GetFrom(*dwg), this->GetCurrentIdPolicy(), dwgGroup, addMembers);
    if (!newProvenance.IsValid())
        return  BSIERROR;

    // detect if the file and the group object has any change:
    if (detector._ShouldSkipFile(*this, *dwg.get()) || newProvenance.IsSame(oldProvenance))
        {
        // record this group as seen
        detector._OnGroupSeen (*this, groupId);
        return  BSISUCCESS;
        }

    // changes detected, make a copy from existing element for editing:
    DgnElementPtr newElement;
    DgnElementCPtr oldElement = this->GetDgnDb().Elements().GetElement (groupId);
    if (!oldElement.IsValid() || !(newElement = oldElement->CopyForEdit()).IsValid())
        return  BSIERROR;

    // update the group element in BIM
    auto status = this->_UpdateGroup (*newElement, dwgGroup);

    // update the sync info
    if (status == BSISUCCESS)
        {
        DgnDbStatus dbStat = DgnDbStatus::Success;
        if (!newElement->Update(&dbStat).IsValid() || dbStat != DgnDbStatus::Success)
            return  BSIERROR;
        
        status = this->GetSyncInfo().UpdateGroup (newProvenance);
        detector._OnGroupSeen (*this, groupId);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_UpdateGroup (DgnElementR dgnGroup, DwgDbGroupCR dwgGroup)
    {
    // the default implementation only supports GenericGroup
    auto genericGroup = dynamic_cast<GenericGroupP> (&dgnGroup);
    if (nullptr != genericGroup)
        {
        GroupFactory    factory (*this, dwgGroup);
        return factory.Update (*genericGroup);
        }
    return  BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr   DwgImporter::_ImportGroup (DwgDbGroupCR group)
    {
    GroupFactory    factory (*this, group);
    return factory.Create ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportGroups (DwgDbDatabaseCR dwg)
    {
    DwgDbDictionaryPtr  groups (dwg.GetGroupDictionaryId(), DwgDbOpenMode::ForRead);
    if (groups.OpenStatus() != DwgDbStatus::Success)
        {
        this->ReportError (IssueCategory::CorruptData(), Issue::CantOpenObject(), Utf8PrintfString("the group dictionary in %lls!", this->GetRootDwgFileName().c_str()).c_str());
        return  BSIERROR;
        }

    DwgDbDictionaryIteratorPtr  iter = groups->GetIterator ();
    if (!iter.IsValid() || !iter->IsValid())
        return  BentleyStatus::BSIERROR;

    auto& detector = this->_GetChangeDetector ();
    auto& syncInfo = this->GetSyncInfo ();

    // Check all entries of the dictionary:
    for (; !iter->Done(); iter->Next())
        {
        DwgDbGroupPtr   group(iter->GetObjectId(), DwgDbOpenMode::ForRead);
        if (group.OpenStatus() != DwgDbStatus::Success)
            {
            this->ReportError (IssueCategory::CorruptData(), Issue::CantOpenObject(), Utf8PrintfString("the group dictionary in %lls!", this->GetRootDwgFileName().c_str()).c_str());
            continue;
            }

        this->Progress ();

        if (this->IsUpdating())
            {
            DwgSyncInfo::Group  oldProvenance;
            if (this->GetSyncInfo().FindGroup(oldProvenance, group->GetObjectId()))
                {
                this->_OnUpdateGroup (oldProvenance, *group);
                continue;
                }
            // fall through to create a new group
            }

        auto dgnGroup = this->_ImportGroup (*group);

        if (dgnGroup.IsValid())
            {
            auto groupId = dgnGroup->GetElementId ();
            if (!groupId.IsValid())
                {
                BeAssert (false && "_ImportGroup should have inserted the group element!");
                auto inserted = dgnGroup->Insert ();
                groupId = inserted->GetElementId ();
                }
            // insert group into the sync info if not inserted by _ImportGroup, and record it as a seen group:
            DwgSyncInfo::Group  prov;
            if (!this->IsUpdating() || !syncInfo.FindGroup(prov, group->GetObjectId()))
                syncInfo.InsertGroup (groupId, *group);
            detector._OnGroupSeen (*this, groupId);
            }
        }

    return  BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportGroups ()
    {
    this->SetStepName (ProgressMessage::STEP_IMPORTING_GROUPS());

    // process root file first
    this->_ImportGroups (this->GetDwgDb());

    // process xRef files one at a time
    for (auto xref : m_loadedXrefFiles)
        this->_ImportGroups (xref.GetDatabase());

    return  BSISUCCESS;
    }
