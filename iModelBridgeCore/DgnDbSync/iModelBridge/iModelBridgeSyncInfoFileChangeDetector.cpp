/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/iModelBridgeSyncInfoFileChangeDetector.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/iModelBridgeSyncInfoFile.h>
#include <Logging/bentleylogging.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING

#undef LOG
#define LOG (*LoggingManager::GetLogger(L"iModelBridge"))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus iModelBridgeSyncInfoFile::ChangeDetector::InsertResultsIntoBIM(ConversionResults& conversionResults)
    {
    if (!conversionResults.m_element.IsValid())
        return DgnDbStatus::Success;

    GetLocksAndCodes(*conversionResults.m_element);

    DgnDbStatus stat;

    auto result = GetDgnDb().Elements().Insert(*conversionResults.m_element, &stat);

    if (DgnDbStatus::Success != stat)
        {
        BeAssert((DgnDbStatus::LockNotHeld != stat) && "Failed to get or retain necessary locks");
        BeAssert(false);
        //ReportIssue(IssueSeverity::Error, IssueCategory::Unsupported(), Issue::ConvertFailure(), IssueReporter::FmtElement(*conversionResults.m_element).c_str());
        return stat;
        }

    _OnElementSeen(conversionResults.m_element->GetElementId());

    conversionResults.m_element = result->CopyForEdit(); // Note that we don't plan to modify the result after this. We just
                                              // want the output to reflect the outcome. Since, we have a non-const
                                              // pointer, we have to make a copy.
    //if (result.IsValid() && LOG_IS_SEVERITY_ENABLED(LOG_TRACE))
    //    LOG.tracev("Insert %s", m_issueReporter.FmtElement(*result).c_str());

    for (ConversionResults& child : conversionResults.m_childElements)
        {
        if (!child.m_element.IsValid())
            continue;
        child.m_element->SetParentId(conversionResults.m_element->GetElementId(), GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));
        auto status = InsertResultsIntoBIM(child);
        if (DgnDbStatus::Success != status)
            return status;
        }
        
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr iModelBridgeSyncInfoFile::ChangeDetector::MakeCopyForUpdate(DgnElementCR newEl, DgnElementCR originalEl)
    {
    // *** This copying is necessary only because originalEl has the ElementId and newEl does (or may) not.
    //      We can't set the elementid. We can only copy it.
    BeAssert(originalEl.GetElementId().IsValid());

    DgnElementPtr writeEl = originalEl.CopyForEdit();   // writeEl will have originalEl's ElementId
    writeEl->CopyFrom(newEl);                            // writeEl now gets newEl's data (other than ElementId)
    writeEl->CopyAppDataFrom(newEl);                     // writeEl also gets newEl's appdata

    // The Code may have changed. (Note: _CopyFrom zeroes out the original Code, so we have to assign it even if unchanged.)
    DgnCode code = originalEl.GetCode();
    DgnCode newCode = newEl.GetCode();
    if (newCode.IsValid() && newCode != code)
        code = newCode;
    writeEl->SetCode(code);

    return writeEl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus iModelBridgeSyncInfoFile::ChangeDetector::GetLocksAndCodes(DgnElementR el)
    {
    if (GetDgnDb().BriefcaseManager().IsBulkOperation())
        return DgnDbStatus::Success;

    // Request locks and codes explicity this is happening in a phase such as _OpenSource that is called before we go into bulk insert mode.
    IBriefcaseManager::Request req;
    el.PopulateRequest(req, (el.GetElementId().IsValid())? BeSQLite::DbOpcode::Update: BeSQLite::DbOpcode::Insert);
    if (RepositoryStatus::Success != GetDgnDb().BriefcaseManager().Acquire(req).Result())
        {
        BeAssert(false);
        return DgnDbStatus::LockNotHeld;
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus iModelBridgeSyncInfoFile::ChangeDetector::UpdateResultsInBIMForOneElement(ConversionResults& conversionResults, DgnElementId existingElementId)
    {
    if (!conversionResults.m_element.IsValid() || !existingElementId.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadArg;
        }

    _OnElementSeen(existingElementId);

    DgnElementCPtr el = GetDgnDb().Elements().GetElement(existingElementId);

    if (!el.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadArg;
        }

    if (el->GetElementClassId() != conversionResults.m_element->GetElementClassId())
        {
        //ReportIssueV(IssueSeverity::Error, IssueCategory::Unsupported(), Issue::UpdateDoesNotChangeClass(), nullptr, 
        //    m_issueReporter.FmtElement(*el).c_str(), conversionResults.m_element->GetElementClass()->GetECSqlName().c_str());
        }

    DgnElementPtr writeEl = MakeCopyForUpdate(*conversionResults.m_element, *el);

    GetLocksAndCodes(*writeEl);

    DgnDbStatus stat; 
    DgnElementCPtr result = GetDgnDb().Elements().Update(*writeEl, &stat); 
    if (!result.IsValid())
        return stat;

    conversionResults.m_element = result->CopyForEdit();// Note that we don't plan to modify the result after this. We just
                                                        // want the output to reflect the outcome. Since, we have a non-const
                                                        // pointer, we have to make a copy.

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus iModelBridgeSyncInfoFile::ChangeDetector::UpdateResultsInBIMForChildren(ConversionResults& parentConversionResults)
    {
    if (parentConversionResults.m_childElements.empty())
        return DgnDbStatus::Success;

    if (!parentConversionResults.m_element.IsValid() || !parentConversionResults.m_element->GetElementId().IsValid())
        {
        BeAssert(false && "input should be persistent parent element");
        return DgnDbStatus::BadArg;
        }
    DgnElementId parentId = parentConversionResults.m_element->GetElementId();

    // Make sure the parentid property is set on all of the children.
    for (auto& newChild : parentConversionResults.m_childElements)
        newChild.m_element->SetParentId(parentId, GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));

    // While we could just delete all existing children and insert all new ones, we try to do better.
    // If we can figure out how the new children map to existing children, we can update them. 

    // Note that in the update logic below we don't delete existing children that were not mapped. 
    // Instead, we just refrain from calling the change detector's _OnElementSeen method on unmatched child elements. 
    // That will allow the updater in its final phase to infer that they should be deleted.

    // The best way is if an extension sets up the DgnElementId of the child elements in parentConversionResults. 
    auto const& firstChild = parentConversionResults.m_childElements.front();
    if (firstChild.m_element.IsValid() && firstChild.m_element->GetElementId().IsValid())
        {
        auto existingChildIdSet = parentConversionResults.m_element->QueryChildren();
        for (auto& childRes : parentConversionResults.m_childElements)
            {
            if (!childRes.m_element.IsValid())
                continue;
            auto existingChildElementId = childRes.m_element->GetElementId();
            auto iFound = existingChildIdSet.find(existingChildElementId);
            if (iFound != existingChildIdSet.end())
                {
                UpdateResultsInBIM(childRes, existingChildElementId);
                // *** WIP_CONVERTER - bail out if any child update fails?
                }
            }
        return DgnDbStatus::Success;
        }

    // If we have to guess, we just try to match them up 1:1 in sequence. 
    bvector<DgnElementId> existingChildIds;
    CachedStatementPtr stmt = GetDgnDb().Elements().GetStatement("SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ParentId=?");
    stmt->BindId(1, parentId);
    while (BE_SQLITE_ROW == stmt->Step())
        existingChildIds.push_back(stmt->GetValueId<DgnElementId>(0));

    size_t count = std::min(existingChildIds.size(), parentConversionResults.m_childElements.size());
    size_t i = 0;
    for ( ; i<count; ++i)
        {
        UpdateResultsInBIM(parentConversionResults.m_childElements.at(i), existingChildIds.at(i));
        // *** WIP_CONVERTER - bail out if any child update fails?
        }

    for ( ; i < parentConversionResults.m_childElements.size(); ++i)
        {
        InsertResultsIntoBIM(parentConversionResults.m_childElements.at(i));
        // *** WIP_CONVERTER - bail out if any child update fails?
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus iModelBridgeSyncInfoFile::ChangeDetector::UpdateResultsInBIM(ConversionResults& conversionResults, 
                                                                         DgnElementId existingElementId)
    {
    auto status = UpdateResultsInBIMForOneElement(conversionResults, existingElementId);
    if (DgnDbStatus::Success != status)
        {
        BeAssert((DgnDbStatus::LockNotHeld != status) && "Failed to get or retain necessary locks");
        return status;
        }
    return UpdateResultsInBIMForChildren(conversionResults);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus iModelBridgeSyncInfoFile::ChangeDetector::_UpdateBimAndSyncInfo(ConversionResults& conversionResults, 
                                                                              ChangeDetector::Results const& changeDetectorResults)
    {
    if (ChangeDetector::ChangeType::Unchanged == changeDetectorResults.GetChangeType())
        {
        _OnElementSeen(changeDetectorResults.GetSyncInfoRecord().GetDgnElementId());
        conversionResults.m_syncInfoRecord = changeDetectorResults.GetSyncInfoRecord();
        return BentleyStatus::SUCCESS;
        }

    if (conversionResults.m_element.IsValid())
        {
        DgnDbStatus status;
        
        DgnElementId eid;
        if (changeDetectorResults.GetSyncInfoRecord().IsValid())
            eid = changeDetectorResults.GetSyncInfoRecord().GetDgnElementId();

        if (!eid.IsValid())
            {
            // The element is not recorded in syncinfo, and so this would normally be handled as an insert.
            // But double-check. It could be that the element already exists in the BIM. (That happens in hybrid
            // bridges that use multiple converters at once. One converter will insert an element, such as a 
            // RepositoryLink, and then another converter within the same bridge will try to do the same.)
            // If so, this is really an update.
            if (conversionResults.m_element->GetElementId().IsValid() && GetDgnDb().Elements().GetElement(conversionResults.m_element->GetElementId()).IsValid())
                eid = conversionResults.m_element->GetElementId();
            }

        if (!eid.IsValid())
            status = InsertResultsIntoBIM(conversionResults);
        else
            status = UpdateResultsInBIM(conversionResults, eid);

        if (DgnDbStatus::Success != status)
            return BentleyStatus::ERROR;
        }

    BentleyStatus sistatus = m_si.WriteResults(changeDetectorResults.GetSyncInfoRecord().GetROWID(), conversionResults, 
                             changeDetectorResults.GetSourceIdentity(), changeDetectorResults.GetCurrentState(), *this);

    if (BSISUCCESS != sistatus)
        return sistatus;

    return iModelBridge::SaveChangesToConserveMemory(GetDgnDb());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
iModelBridgeSyncInfoFile::ChangeDetector::Results iModelBridgeSyncInfoFile::ChangeDetector::_DetectChange(ROWID scope, Utf8CP kind, ISourceItem& item, T_Filter* filter, bool forceChange)
    {
    Results res;

    auto sid = SourceIdentity(scope, kind, item._GetId());

    if (sid.GetId().empty())
        {
        // --------------------------------
        // Look up by hash.
        // --------------------------------
        SourceState currentState(item._GetLastModifiedTime(), item._GetHash());

        auto byhash = m_si.MakeIteratorByHash(sid.GetScopeROWID(), sid.GetKind(), currentState.GetHash());
        auto i = byhash.begin();
        if (i == byhash.end())
            return Results(sid, currentState);    // It's new

        Record rec = i.GetRecord();
        if ((nullptr != filter) && !(*filter)(rec, m_si))
            return Results(sid, currentState);    // We have it, but the filter rejected it. We have to treat it as new

        // We found it because its content hash matches a stored record. So, we report that this item is the same as this record.
        return Results(forceChange? ChangeType::Changed: ChangeType::Unchanged, rec, currentState);
        }
        
    // --------------------------------
    // Look up by source id
    // --------------------------------
    auto byid = m_si.MakeIteratorBySourceId(sid);

    for (auto i = byid.begin(); i != byid.end(); ++i)
        {
        Record rec = i.GetRecord();
        if ((nullptr != filter) && !(*filter)(rec, m_si))
            continue;

        double lmt = item._GetLastModifiedTime();
        if (!forceChange && (0 != lmt) && (rec.GetSourceState().GetLastModifiedTime() == lmt))
            return Results(ChangeType::Unchanged, rec, SourceState(lmt,""));

        SourceState currentState(lmt, item._GetHash());
        ChangeType ch = (rec.GetSourceState().GetHash() == currentState.GetHash())? ChangeType::Unchanged: ChangeType::Changed;
        if (forceChange)
            ch = ChangeType::Changed;
        return Results(ch, rec, currentState);
        }
    
    SourceState currentState(item._GetLastModifiedTime(), item._GetHash());
    return Results(sid, currentState); // it's new
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Bentley.Systems
//---------------------------------------------------------------------------------------
iModelBridgeSyncInfoFile::ChangeDetector::Results iModelBridgeSyncInfoFile::InitialConversionChangeDetector::_DetectChange(ROWID scope, Utf8CP kind, ISourceItem& item, T_Filter* filter, bool)
    {
    Results res;
    auto sid = SourceIdentity(scope, kind, item._GetId());
    SourceState currentState(item._GetLastModifiedTime(), item._GetHash());
    return Results(sid, currentState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSyncInfoFile::ChangeDetector::_DeleteElementsNotSeenInScopes(bvector<ROWID> const& onlyInScopes)
    {
    // *** NB: This alogorithm *infers* that an element was deleted in the source repository if we did not see it during the conversion.
    //          This inference is valid only if we know that we saw all items in the source and that they were all added to m_elementsSeen or m_scopesSkipped.

    // iterate over all of records of the previous conversion.
    auto iter = m_si.MakeIterator();
    for (auto elementInSyncInfo=iter.begin(); elementInSyncInfo!=iter.end(); ++elementInSyncInfo)
        {
        if (std::find(onlyInScopes.begin(), onlyInScopes.end(), elementInSyncInfo.GetSourceIdentity().GetScopeROWID()) == onlyInScopes.end())
            continue;   // consider only items in the specified scopes

        auto previousConversion = elementInSyncInfo.GetRecord();
        if (!previousConversion.GetDgnElementId().IsValid())
            continue;   // ignore discards and records created for organization purposes

        if (m_elementsSeen.Contains(previousConversion.GetDgnElementId())) // if update encountered at least one V8 element that was mapped to this BIM element,
            continue;   // keep this BIM element alive

        if (m_scopesSkipped.find(previousConversion.GetSourceIdentity().GetScopeROWID()) != m_scopesSkipped.end()) // if we skipped this whole scope in the source (e.g., because it was unchanged),
            continue;   // we don't expect any element from it to be in m_elementsSeen. Keep them all alive.

        if (m_si.IsSourceItemMappedToAnElementThatWasSeen(elementInSyncInfo, m_elementsSeen)) // if update mapped this one source item to multiple elements and if we did see one of those elements,
            continue;   // infer that this is a child of an assembly, and the assembly parent is in m_elementsSeen (probably unchanged).

        // We did not encounter the source item that was mapped to this BIM element. We infer that the item
        // was deleted. Therefore, the update to the BIM is to delete the corresponding BIM element.
        LOG.tracev("Delete element %lld", previousConversion.GetDgnElementId().GetValue());

        _OnItemDelete(previousConversion);
        GetDgnDb().Elements().Delete(previousConversion.GetDgnElementId());
        m_si.DeleteAllItemsMappedToElement(previousConversion.GetDgnElementId());

        //if (_WantProvenanceInBim())
        //    DgnV8ElementProvenance::Delete(previouslyConvertedElementId, converter.GetDgnDb());

        _OnItemConverted(previousConversion, ChangeOperation::Delete);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSyncInfoFile::ChangeDetector::_OnItemConverted(Record const& rec, ChangeOperation changeOperation)
    {
    ++m_elementsConverted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Ramanujam.Raman                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSyncInfoFile::ChangeDetector::_OnItemDelete(Record const& rec)
    {
    // *** TBD
    // m_linkConverter->RemoveLinksOnElement(elementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelBridgeSyncInfoFile::ChangeDetector::_OnScopeSkipped(ROWID srid)
    {
    if (0 == srid) 
        return;

    // *** TBD: Write a recursive CTE to find all children of srid and add them too.
    m_scopesSkipped.insert(srid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/17
+---------------+---------------+---------------+---------------+---------------+------*/
iModelBridgeSyncInfoFile::ChangeDetector::ChangeDetector(iModelBridgeSyncInfoFile& si) 
    : m_si(si) 
    {
    }
