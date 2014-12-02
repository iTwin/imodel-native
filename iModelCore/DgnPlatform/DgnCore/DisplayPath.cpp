/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DisplayPath.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayPath::DisplayPath ()
    {
    Empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    03/98
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayPath::DisplayPath (ElementRefP oDisp, DgnModelP model)
    {
    SetPath (oDisp, model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayPath::SetPath (ElementRefP newRef, DgnModelP newModel)
    {
    BeAssert (newRef);

    Empty();

    // set new root modelref
    SetRoot (newModel ? newModel : newRef->GetDgnModelP ());
    PushPathElem (newRef);
    }

/*---------------------------------------------------------------------------------**//**
* re-initialize a path to have no entries
* @bsimethod                                                    KeithBentley    01/98
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayPath::Empty ()
    {
    m_path.resize (0);
    m_root = NULL;

    m_cursorIndex     = -1;
    m_baseIndex       = -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    DisplayPath                                     KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
int DisplayPath::GetCount () const
    {
    return (int) m_path.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    DisplayPath                                     KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayPath::Contains (ElementRefP oObj) const
    {
    return  std::find (m_path.begin(), m_path.end(), oObj) != m_path.end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    DisplayPath                                     KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP DisplayPath::GetHitElem () const
    {
    return  GetCursorElem ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/98
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayPath::IsSubPathOf (DisplayPathCP containerPath) const
    {
    int     thisLength   = GetCount();
    int     containerLen = containerPath->GetCount();

    if (thisLength > containerLen)
        return  false;

    for (int i=0; i<thisLength; i++ )
        {
        if (GetPathElem (i) != containerPath->GetPathElem(i))
            return  false;
        }

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    DisplayPath                                     KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayPath::IsSamePath (DisplayPathCP oOtherPath, bool fullPath) const
    {
    if (this == oOtherPath)
        return  true;

    if (NULL == oOtherPath)
        return  false;

    int path1Length = fullPath ? GetCount() : GetCursorIndex()+1;
    int path2Length = fullPath ? oOtherPath->GetCount() : oOtherPath->GetCursorIndex()+1;

    // make sure they're the same length
    if (path1Length != path2Length)
        return  false;

    if (m_root != oOtherPath->m_root)
        return  false;

    // compare arrays for identical members
    if (fullPath)
        return m_path == oOtherPath->m_path;

    return std::equal (m_path.begin (), m_path.begin () + path1Length, oOtherPath->m_path.begin ());
    }

/*---------------------------------------------------------------------------------**//**
* set the path of this path to the same as another path.
* @bsimethod                                                    KeithBentley    05/98
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayPath::SetPath (DisplayPathCP fromPath)
    {
    m_path = *fromPath->GetArray();

    m_root          = fromPath->GetRoot();
    m_baseIndex     = fromPath->GetBaseIndex();
    m_cursorIndex   = fromPath->GetCursorIndex();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayPath::PushPathElem (ElementRefP oGeom)
    {
    //  set the cursor index to the last pushed entry
    m_cursorIndex = (int) m_path.size();

    m_path.push_back (oGeom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayPath::PopPathElem()
    {
    m_path.pop_back ();

    int count = (int) m_path.size ();

    if (count == m_cursorIndex)
        m_cursorIndex--;

    if (count == m_baseIndex)
        m_baseIndex--;
    }

/*---------------------------------------------------------------------------------**//**
* insert a new object at the head of an existing path. This is only necessary when we're
* creating a path backwards (from an object towards its parents).
* @bsimethod                                                    KeithBentley    03/98
+---------------+---------------+---------------+---------------+---------------+------*/
void    DisplayPath::InsertPathElemAtHead (ElementRefP oObj)
    {
    m_path.insert (m_path.begin(), oObj);
    m_cursorIndex++;
    }

/*---------------------------------------------------------------------------------**//**
* remove the current object at the head of the path. This is only necessary when we're
* creating a path backwards (from an object towards its parents).
* @bsimethod                                                    KeithBentley    03/98
+---------------+---------------+---------------+---------------+---------------+------*/
void    DisplayPath::RemovePathElemFromHead ()
    {
    m_path.erase (m_path.begin());
    m_cursorIndex--;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    DisplayPath                                     KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP     DisplayPath::GetPathElem (int index) const
    {
    if (index < 0)                              // ***NEEDS WORK: The old ObjectArray used to support -1 => END
        index = (int) m_path.size() - 1;

    if (index < 0 || index >= (int) m_path.size())
        return NULL;

    return  m_path.at(index).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    DisplayPath                                     KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void     DisplayPath::SetPathElem (ElementRefP oObj, int  index)
    {
    if (index < 0)
        index = (int) m_path.size() - 1;

    m_path.at (index) = oObj;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    DisplayPath                                     KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP     DisplayPath::GetCursorElem () const
    {
    return  GetPathElem (m_cursorIndex);
    }

/*---------------------------------------------------------------------------------**//**
* Get Element at head (beginning) of the display path
* @bsimethod                                                    RayBentley      06/00
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP     DisplayPath::GetHeadElem () const
    {
    if (m_path.empty ())
        return NULL;

    return m_path.front().get();
    }

/*---------------------------------------------------------------------------------**//**
* Get Element at tail (end) of the display path
* @bsimethod                                                    RayBentley      06/00
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP     DisplayPath::GetTailElem () const
    {
    if (m_path.empty ())
        return NULL;

    return m_path.back ().get();
    }

/*---------------------------------------------------------------------------------**//**
* Get the innermost element in the path that is NOT a non-model element. This is so
* that tools see a shared cell instance rather than a component of its definition.
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP     DisplayPath::GetComponentElem () const
    {
    if (m_path.empty ())
        return NULL;

    ElementRefP innerRef = m_path.front ().get();

    for (DgnElemRefArray::const_iterator curr = m_path.begin (); curr != m_path.end (); ++curr)
        {
        ElementRefP elemRef = curr->get();

        innerRef = elemRef;
        }

    return innerRef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    DisplayPath                                     KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void    DisplayPath::SetCursorIndex (int index)
    {
    if (index == -1)
        index = GetCount() -1;

    m_cursorIndex = index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHiliteState  DisplayPath::IsHilited () const
    {
    int     cursor = GetCursorIndex ();

    // to tell whether a path is hilited, we have to check every entry in the path up to the
    // cursor. If any of them are hilited, then all their children are hilited too.

    DgnElemRefArray::const_iterator elem = m_path.begin ();

    if (elem == m_path.end() || cursor < 0)
        return HILITED_None;

    for (int i=0; i <= cursor; i++, elem++)
        {
        ElementHiliteState  hiliteState = (*elem)->IsHilited();

        switch (hiliteState)
            {
            case HILITED_None:
                // Keep looking...need to find Normal/Bold/Dashed on a component!
                break;

            default:
                return hiliteState;
            }
        }

    return HILITED_None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayPath::SetHilited (ElementHiliteState newState) const
    {
    ElementRefP  cursorElem = GetCursorElem();

    // don't turn on/off hilite bit for elements in the selection set.
    if (cursorElem->IsInSelectionSet())
        return;

    // KLUDGE: Preserve any alternative hilite state (i.e. HILITED_Bold) already set on this element...
    if (HILITED_None == newState || HILITED_None == cursorElem->IsHilited())
        {
        cursorElem->SetHilited(newState);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void    DisplayPath::Dump (WCharCP label) const
    {
#if defined (NEEDS_WORK_DGNITEM)
    printf ("%ls", label);

    for (auto const& curr : m_path)
        printf (" -> TY:%d, ID:%llu", curr->GetLegacyType(), curr->GetElementId().GetValue());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP     DisplayPath::GetRoot () const
    {
    return m_root;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            DisplayPath::SetRoot (DgnModelP ref)
    {
    m_root = ref;
    }

/*---------------------------------------------------------------------------------**//**
* get the DgnModelP that is the root of this DisplayPath
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP     DisplayPath::GetEffectiveRoot () const
    {
#if defined (WIP_V10_TRANSIENTS)
    if (InTransientModel ())
        return TransientDgnModel::GetPtr();
#endif
    return GetRoot ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            DisplayPath::_DrawInVp (ViewportP vp, DgnDrawMode drawMode, DrawPurpose drawPurpose, bool* stopFlag) const
    {
    if (vp->IsActive())
        T_HOST.GetGraphicsAdmin()._DrawPathInVp (this, vp, drawMode, drawPurpose, stopFlag);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayPath::_GetInfoString (WStringR pathDescr, WCharCP delimiter) const
    {
    T_HOST.GetGraphicsAdmin()._GetInfoString (this, pathDescr, delimiter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08.05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DisplayPath::GetLocalToWorld (DMatrix4d& localToWorld, ViewportP viewport, int cursorIndex) const
    {
    // WIP_V10_NO_SHARED_CELLS - Remove with shared cells.
    StatusInt   status = SUCCESS;
    NullOutput  output;
    NullContext context (&output);

    context.Attach (viewport, DrawPurpose::NotSpecified);

    if (-1 == cursorIndex)
        cursorIndex = GetCount () - 1;

#if defined (NEEDS_WORK_DGNITEM)
    for (int i = 0; i < cursorIndex; i++)
        {
        ElementHandle eh (GetPathElem (i));
        IDisplayHandlerPathEntryExtension* extension = IDisplayHandlerPathEntryExtension::Cast (eh.GetHandler ());

        if (extension)
            extension->_PushDisplayEffects (eh, context);
        }
#endif

    context.GetCurrLocalToWorldTrans (localToWorld);

    context.Detach ();

    return status;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayPathPtr  DisplayPath::Create ()
    {
    return new DisplayPath ();
    }

