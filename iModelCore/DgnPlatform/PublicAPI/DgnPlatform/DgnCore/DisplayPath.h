/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DisplayPath.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnPlatform.h"

//__PUBLISH_SECTION_END__
#include "DgnModel.h"
#include "IViewOutput.h"

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct DisplayPath : RefCountedBase
{
//__PUBLISH_SECTION_END__
private:

    typedef bvector<DgnElementPtr> DgnElemRefArray;

    DgnElemRefArray     m_path;
    DgnModelP        m_root;

    int                 m_cursorIndex;
    int                 m_baseIndex;

    const DgnElemRefArray*    GetArray () const {return &m_path;}

protected:
    DGNPLATFORM_EXPORT virtual void _GetInfoString (Utf8StringR pathDescr, Utf8CP delimiter) const;
    DGNPLATFORM_EXPORT virtual void _DrawInVp (DgnViewportP, DgnDrawMode drawMode, DrawPurpose drawPurpose, bool* stopFlag) const;

public:
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS 

    DGNPLATFORM_EXPORT DisplayPath ();
    DGNPLATFORM_EXPORT DisplayPath (DgnElementCP, DgnModelP model = NULL);

    void                    Empty ();

    void                    SetBaseIndex (int baseIndex) {m_baseIndex = baseIndex;}
    int                     GetBaseIndex () const {return m_baseIndex;}
    int                     GetCursorIndex () const {return m_cursorIndex;}
    void                    InitToViewBase (int viewNum);
    DGNVIEW_EXPORT void     DrawInView (IndexedViewSetR, int viewNum, DgnDrawMode drawMode, DrawPurpose drawPurpose) const;
    DGNVIEW_EXPORT void     DrawInViews (IndexedViewSetR, DgnDrawMode drawMode, DrawPurpose drawPurpose) const;
    DGNVIEW_EXPORT void     DrawInAllViews (IndexedViewSetR, DgnDrawMode drawMode, DrawPurpose drawPurpose) const;
    DGNVIEW_EXPORT void     Hilite (IndexedViewSetR, bool onOff) const;
    DGNVIEW_EXPORT bool     IsInSelectionSet () const;

    DGNPLATFORM_EXPORT virtual void Dump (WCharCP label) const;
    DGNPLATFORM_EXPORT void         GetInfoString (Utf8StringR pathDescr, Utf8CP delimiter) const;
    DGNPLATFORM_EXPORT virtual void SetHilited (DgnElement::Hilited) const;
    DGNPLATFORM_EXPORT DgnElement::Hilited IsHilited () const;

    DGNPLATFORM_EXPORT void         SetPathElem (DgnElementP oObj, int index);
    DGNPLATFORM_EXPORT void         RemovePathElemFromHead ();
    DGNPLATFORM_EXPORT void         InsertPathElemAtHead (DgnElementP oObj);

    DGNPLATFORM_EXPORT void         SetPath (DisplayPathCP fromPath);
    DGNPLATFORM_EXPORT void         SetPath (DgnElementCP newRef, DgnModelP newModel = NULL);

    DGNPLATFORM_EXPORT void         DrawInVp (DgnViewportP, DgnDrawMode drawMode, DrawPurpose drawPurpose, bool* stopFlag) const;

    DGNPLATFORM_EXPORT DgnElementP  GetHitElem () const;
    DGNPLATFORM_EXPORT DgnModelP    GetEffectiveRoot () const;
    DGNPLATFORM_EXPORT StatusInt    GetLocalToWorld (DMatrix4d& localToWorld, DgnViewportP viewport, int cursorIndex) const; // WIP_V10_NO_SHARED_CELLS - Remove with shared cells.

    DGNPLATFORM_EXPORT virtual bool IsSamePath (DisplayPathCP otherPath, bool fullPath) const;
    DGNPLATFORM_EXPORT bool         IsSubPathOf (DisplayPathCP containerPath) const;
    DGNPLATFORM_EXPORT bool         Contains (DgnElementP oObj) const;

    virtual DisplayPathType         GetPathType () const {return DisplayPathType::Display;}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    DGNPLATFORM_EXPORT DgnModelP GetRoot () const;
    DGNPLATFORM_EXPORT int          GetCount () const;
    DGNPLATFORM_EXPORT DgnElementP  GetPathElem (int index) const;
    DGNPLATFORM_EXPORT DgnElementP  GetHeadElem () const;
    DGNPLATFORM_EXPORT DgnElementP  GetTailElem () const;
    DGNPLATFORM_EXPORT DgnElementP  GetCursorElem () const;
    DGNPLATFORM_EXPORT DgnElementP  GetComponentElem () const;

    DGNPLATFORM_EXPORT void         SetRoot (DgnModelP ref);
    DGNPLATFORM_EXPORT void         PushPathElem (DgnElementCP elRef);
    DGNPLATFORM_EXPORT void         PopPathElem ();
    DGNPLATFORM_EXPORT void         SetCursorIndex (int index);

    DGNPLATFORM_EXPORT static DisplayPathPtr Create ();

}; // DisplayPath

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
