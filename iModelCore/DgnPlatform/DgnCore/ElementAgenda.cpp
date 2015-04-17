/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementAgenda.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   08/03
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAgenda::ElementAgenda ()
    {
    SetSource (ModifyElementSource::Unknown);

    m_hilitedState = AgendaHilitedState::Unknown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   08/03
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAgenda::~ElementAgenda ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
EditElementHandleP ElementAgenda::Insert (DgnElementCP elRef, bool atHead)
    {
    if (NULL == elRef)
        return NULL;

    m_hilitedState = AgendaHilitedState::Unknown; // NOTE: Insert invalidates hilite state!

    ElemAgendaEntry agendum (elRef);

    iterator location = insert (atHead ? begin() : end(), agendum);

    EditElementHandle& newItem = *location;
    return &newItem;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/04
+---------------+---------------+---------------+---------------+---------------+------*/
EditElementHandleP ElementAgenda::InsertElemDescr(DgnElementP edP, bool atHead)
    {
    if (NULL == edP)
        return NULL;

    m_hilitedState = AgendaHilitedState::Unknown; // NOTE: Insert invalidates hilite state!

    // Create new empty agendum...
    ElemAgendaEntry blank;
    EditElementHandleP newAgendum = &*insert (atHead ? begin() : end(), blank);

    newAgendum->SetElementDescr (edP, false);
    return newAgendum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
EditElementHandleP ElementAgenda::Insert (EditElementHandleR eeh, bool atHead)
    {
    if (NULL != eeh.PeekElementDescrCP ())
        return InsertElemDescr (eeh.ExtractElementDescr().get(), atHead);

    DgnElementCP elRef = eeh.GetDgnElement ();
    if (NULL == elRef)
        return NULL;

    return Insert (elRef, atHead);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAgenda::Insert (bset<DgnElementCP>& elSet)
    {
    EditElementHandleCP curr = GetFirst();
    EditElementHandleCP end  = curr + GetCount();

    // first make sure there are no entries in the elSet that are already in this Agenda
    for (; curr < end ; curr++)
        {
        DgnElementCP test = curr->GetDgnElement();

        if ((NULL != test) && (elSet.end() != elSet.find (test)))
            elSet.erase (test);
        }

    // add all entries in the set to the agenda (no need to test for uniqueness)
    for (auto iter = elSet.begin(); iter != elSet.end(); iter++)
        Insert (*iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
EditElementHandleCP ElementAgenda::Find (DgnElementCP elRef, size_t startIndex, size_t endIndex) const
    {
    if (IsEmpty())
        return  NULL;

    if (endIndex > GetCount())
        endIndex = GetCount();

    if (endIndex <= startIndex)
        return  NULL;

    EditElementHandleCP curr = GetFirst() + startIndex;
    EditElementHandleCP end  = GetFirst() + endIndex;

    for (; curr < end ; curr++)
        {
        if (curr->GetDgnElement() == elRef)
            return curr;
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
EditElementHandleP ElementAgenda::InsertPath (DisplayPathCP path, bool doGroups, bool groupLockState, bool allowLocked)
    {
    DgnElementP     elRef    = path->GetHitElem();

    EditElementHandleP entry = Insert (elRef);
    if (NULL == entry)
        return NULL;

    if (!doGroups)
        return entry;

    // NOTE: Insert of group members can invalidate entry...
    size_t entryIndex = GetCount()-1;

#if defined (NEEDS_WORK_DGNITEM)
    AddMembersOfAssembly (*elRef->GetDgnDb(), elRef->GetAssemblyId());
#endif

#ifdef DGN_IMPORTER_REORG_WIP
    // get the element header to check the graphic group number
    Header const* hdr = &entry->GetElementCP()->hdr;
    AddMembersOfGroups (elRef, modelRef, NULL, groupLockState, allowLocked, (groupLockState && hdr->IsGraphic()) ? hdr->dhdr.grphgrp : 0);
#endif

    return (EditElementHandleP) (GetFirst () + entryIndex);
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAgenda::AddMembersOfAssembly (DgnDbR project, ElementId assemblyId)
    {
    if (!assemblyId.IsValid())
        return;

    for (auto it : project.MakeAssemblyIterator(assemblyId))
        {
        PersistentDgnElementPtr el = it.GetDgnElement();
        BeAssert (el.IsValid());
        Insert (el.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* find all the members of named groups/graphic groups.
* @bsimethod                                                    Brien.Bastings  02/04
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAgenda::AddMembersOfGroups (DgnElementP elRef, DgnModelP modelRef, DgnModelP baseDgnModel, bool groupLock, bool allowLocked, int ggNum)
    {
    if (NULL == modelRef)
        return;

        BeAssert (false && "WIP_GROUPS");
    }

/*---------------------------------------------------------------------------------**//**
* find named group members that want to be selected along with supplied element.
* @bsimethod                                                    Brien.Bastings  04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ElementAgenda::AddGroupSelectionMembers (DgnModelP baseDgnModel)
    {
    // Add named grouped members for groups with "select" flag...
    if (!baseDgnModel)
        return;

        BeAssert (false && "WIP_GROUPS");
    }
#endif

