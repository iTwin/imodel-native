/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DisplayPath.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnPlatform.h"

//__PUBLISH_SECTION_END__
#include "DgnModel.h"
#include "IViewOutput.h"

#define DESCRDELIMIT_CommaSpace     L", "
#define DESCRDELIMIT_Newline        L"\n"
#define DESCRDELIMIT_StdSingleLine  DESCRDELIMIT_CommaSpace
#define DESCRDELIMIT_StdMultiLine   DESCRDELIMIT_Newline

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct DisplayPath : RefCountedBase
{
//__PUBLISH_SECTION_END__
private:

    typedef bvector<ElementRefPtr> DgnElemRefArray;

    DgnElemRefArray     m_path;
    DgnModelP        m_root;

    int                 m_cursorIndex;
    int                 m_baseIndex;

    const DgnElemRefArray*    GetArray () const {return &m_path;}

protected:
    DGNPLATFORM_EXPORT virtual void _GetInfoString (WStringR pathDescr, WCharCP delimiter) const;
    DGNPLATFORM_EXPORT virtual void _DrawInVp (ViewportP, DgnDrawMode drawMode, DrawPurpose drawPurpose, bool* stopFlag) const;

public:

    // OPERATOR_NEW_KLUDGE
    void * operator new(size_t size) { return bentleyAllocator_allocateRefCounted (size); }
    void operator delete(void *rawMemory, size_t size) { bentleyAllocator_deleteRefCounted (rawMemory, size); }
    void * operator new [](size_t size) { return bentleyAllocator_allocateArrayRefCounted (size); }
    void operator delete [] (void *rawMemory, size_t size) { bentleyAllocator_deleteArrayRefCounted (rawMemory, size); }

    DGNPLATFORM_EXPORT DisplayPath ();
    DGNPLATFORM_EXPORT DisplayPath (ElementRefP, DgnModelP model = NULL);

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
    DGNPLATFORM_EXPORT void         GetInfoString (WStringR pathDescr, WCharCP delimiter) const;
    DGNPLATFORM_EXPORT virtual void SetHilited (ElementHiliteState) const;
    DGNPLATFORM_EXPORT ElementHiliteState IsHilited () const;

    DGNPLATFORM_EXPORT void         SetPathElem (ElementRefP oObj, int index);
    DGNPLATFORM_EXPORT void         RemovePathElemFromHead ();
    DGNPLATFORM_EXPORT void         InsertPathElemAtHead (ElementRefP oObj);

    DGNPLATFORM_EXPORT void         SetPath (DisplayPathCP fromPath);
    DGNPLATFORM_EXPORT void         SetPath (ElementRefP newRef, DgnModelP newModel = NULL);

    DGNPLATFORM_EXPORT void         DrawInVp (ViewportP, DgnDrawMode drawMode, DrawPurpose drawPurpose, bool* stopFlag) const;

    DGNPLATFORM_EXPORT ElementRefP  GetHitElem () const;
    DGNPLATFORM_EXPORT DgnModelP    GetEffectiveRoot () const;
    DGNPLATFORM_EXPORT StatusInt    GetLocalToWorld (DMatrix4d& localToWorld, ViewportP viewport, int cursorIndex) const; // WIP_V10_NO_SHARED_CELLS - Remove with shared cells.

    DGNPLATFORM_EXPORT virtual bool IsSamePath (DisplayPathCP otherPath, bool fullPath) const;
    DGNPLATFORM_EXPORT bool         IsSubPathOf (DisplayPathCP containerPath) const;
    DGNPLATFORM_EXPORT bool         Contains (ElementRefP oObj) const;

    virtual DisplayPathType         GetPathType () const {return DisplayPathType::Display;}

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

    DGNPLATFORM_EXPORT DgnModelP GetRoot () const;
    DGNPLATFORM_EXPORT int          GetCount () const;
    DGNPLATFORM_EXPORT ElementRefP  GetPathElem (int index) const;
    DGNPLATFORM_EXPORT ElementRefP  GetHeadElem () const;
    DGNPLATFORM_EXPORT ElementRefP  GetTailElem () const;
    DGNPLATFORM_EXPORT ElementRefP  GetCursorElem () const;
    DGNPLATFORM_EXPORT ElementRefP  GetComponentElem () const;

    DGNPLATFORM_EXPORT void         SetRoot (DgnModelP ref);
    DGNPLATFORM_EXPORT void         PushPathElem (ElementRefP elRef);
    DGNPLATFORM_EXPORT void         PopPathElem ();
    DGNPLATFORM_EXPORT void         SetCursorIndex (int index);

    DGNPLATFORM_EXPORT static DisplayPathPtr Create ();

}; // DisplayPath

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
