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
USING_NAMESPACE_DGNDBSYNC_DWG

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
GenericGroupPtr GroupFactory::CreateAndInsert ()
    {
    GenericGroupPtr genericGroup;
    if (m_dwgGroup.GetAllEntityIds(m_memberIds) == 0)
        return  genericGroup;

    // get the model for generic groups:
    auto groupModel = m_importer.GetDgnDb().Models().Get<GenericGroupModel>(m_importer.GetGroupModelId());
    if (!groupModel.IsValid())
        return  genericGroup;

    genericGroup = GenericGroup::Create (*groupModel);
    if (!genericGroup.IsValid())
        return  genericGroup;

    // set the group name as the user label
    Utf8String  name(m_dwgGroup.GetName().c_str());
    genericGroup->SetUserLabel (name.c_str());

    // insert GenericGroup to bim now, as AddMember needs source & target element IDs.
    if (genericGroup->Insert().IsNull())
        {
        m_importer.ReportError (IssueCategory::UnexpectedData(), Issue::Message(), Utf8PrintfString("Failed inserting group %lls!", name.c_str()).c_str());
        genericGroup = nullptr;
        }

    return  genericGroup;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GroupFactory::CreateGroup ()
    {
    auto genericGroup = this->CreateAndInsert ();
    if (!genericGroup.IsValid())
        return  BSIERROR;

    size_t  numExpected = m_memberIds.size ();
    size_t  numImported = 0;

    // walk through member entities:
    for (auto objectId : m_memberIds)
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
        Utf8PrintfString warn("Memebers expected: %d, created: %d", numExpected, numImported);
        m_importer.ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::UnexpectedData(), Issue::Message(), Utf8PrintfString("Group %ls", m_dwgGroup.GetName().c_str()).c_str(), warn.c_str());
        }

    return  BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportGroup (DwgDbGroupCR group)
    {
    BentleyStatus   status = BSIERROR;
    GroupFactory    factory (*this, group);

    if (this->IsUpdating())
        {
        // WIP
        return  status;
        }

    status = factory.CreateGroup ();

    return  status;
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

    DwgDbDictionaryIterator iter = groups->GetIterator ();
    if (!iter.IsValid())
        return  BentleyStatus::BSIERROR;

    // Check all entries of the dictionary:
    for (; !iter.Done(); iter.Next())
        {
        DwgDbGroupPtr   group(iter.GetObjectId(), DwgDbOpenMode::ForRead);
        if (group.OpenStatus() != DwgDbStatus::Success)
            {
            this->ReportError (IssueCategory::CorruptData(), Issue::CantOpenObject(), Utf8PrintfString("the group dictionary in %lls!", this->GetRootDwgFileName().c_str()).c_str());
            continue;
            }

        this->_ImportGroup (*group);
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
