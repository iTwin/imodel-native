/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementAgenda.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

typedef struct IDataObject    GuiDataObject;

#include <Bentley/bvector.h>
#include <Bentley/bset.h>
#include "IViewOutput.h"
#include "ElementHandle.h"

#if !defined (DOCUMENTATION_GENERATOR)
DGNPLATFORM_TYPEDEFS (IModifyElement)
DGNPLATFORM_TYPEDEFS (IRedrawOperation)
DGNPLATFORM_TYPEDEFS (IRedrawAbort)
#endif

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! this gets passed as an argument to the element function
enum class ModifyElementSource
{
    Selected         = 0,    //!< The element is selected by the user.
    SelectionSet     = 1,    //!< The element is processed because it is in the selection set.
    Fence            = 2,    //!< The element is processed because it is passes the fence criteria.
    WorkingSet       = 3,    //!< The element is processed because it is is in the working set.
    GraphicGroup     = 4,    //!< The element is processed because it belongs to the graphic group of the selected element.
    NamedGroup       = 5,    //!< The element is processed because it belongs to a named group that the selected element also belongs.
    DragSelection    = 6,    //!< The element is selected by the user by drag selection or multi-selection using ctrl.
    Unknown          = 10,   //!< The source for the element is unknown - not caused by a modification command.
};

#if defined (NOT_NOW_WIP_REMOVE_ELEMENTHANDLE)

struct  IElementAgendaEvents;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
template <class T_ElemHandle> class ElemHandleArray : public bvector<T_ElemHandle>
{
public:
    //! Now just a wrapper around vector->empty().  Returns true if the ElemHandleArray contains no elements.
    bool                    IsEmpty ()          const   {return this->empty();}
    //! Now just a wrapper around vector->size().  Returns the number of elements in the ElemHandleArray. 
    size_t                  GetCount ()         const   {return this->size();}
    //! Return the first elemement in the ElemHandleArray or NULL if it is empty. 
    T_ElemHandle const*     GetFirst ()         const   {return IsEmpty() ? NULL : &this->front();}
    //! Return the last elemement in the ElemHandleArray or NULL if it is empty. 
    T_ElemHandle const*     GetLast ()          const   {return IsEmpty() ? NULL : &this->back();}
    //! Return the ith elemement in the ElemHandleArray or NULL if it is empty. 
    T_ElemHandle const*     GetEntry (size_t i) const   {return (i >= GetCount()) ? NULL : &this->front()+i;}
    //! Return an editable pointer the first elemement in the ElemHandleArray or NULL if it is empty. 
    T_ElemHandle*           GetFirstP ()                {return IsEmpty() ? NULL : &this->front();}
    //! Return an editable pointer the last elemement in the ElemHandleArray or NULL if it is empty. 
    T_ElemHandle*           GetLastP ()                 {return IsEmpty() ? NULL : &this->back();}
    //! Return an editable pointer the ith elemement in the ElemHandleArray or NULL if it is empty. 
    T_ElemHandle*           GetEntryP (size_t i)        {return (i >= GetCount()) ? NULL : &this->front()+i;}
    //! Remove all elements from the array.  Call delete on the elements if andFree is specified.
    void                    Empty (bool andFree=true)   {if (andFree) this->clear(); else this->resize (0);}
    DEFINE_T_SUPER(bvector<T_ElemHandle>);
    typedef typename T_Super::iterator iterator;

    //! Remove all elements from the array that are invalid elements.  See ElementHandle::IsValid().
    void DropInvalidEntries ()
        {
        if (IsEmpty())
            return;

        iterator curr = this->begin();   // first entry
        iterator endIt = this->end();    // one past the last entry
        iterator nextValid = curr; // where next valid entry should go

        for (; curr != endIt; ++curr)
            {
            if (curr->IsValid ())
                {
                if (nextValid != curr)
                    *nextValid = *curr;
                ++nextValid;
                }
            }

        if (nextValid != endIt)
            this->erase (nextValid, endIt);
        }
};

//! Interface to notfy applications of ElementAgenda activity.
struct     IElementAgendaEvents
{
    virtual bool DoModifyAgendaEntries (ElementAgendaP agenda, AgendaOperation opType, AgendaModify modify) = 0;
    virtual void OnPreModifyAgenda (ElementAgendaCP agenda, AgendaOperation opType, AgendaModify modify, bool isGroupOperation) = 0;
    virtual void OnPostModifyAgenda (ElementAgendaCP agenda, AgendaOperation opType, AgendaModify modify, bool isGroupOperation) = 0;

#if !defined (DOCUMENTATION_GENERATOR)
    virtual void DoAddDeferredClipboardFormats (ElementAgendaP, AgendaOperation opType, AgendaModify modify, GuiDataObject*) = 0;
    virtual bool Dummy1 (void*) {return false;}
#endif
};

//=======================================================================================
//! @ref EditElementHandle residing in an ElementAgenda
// @bsiclass                                                    Keith.Bentley   08/13
//=======================================================================================
struct ElemAgendaEntry : EditElementHandle
{
public:
    ElemAgendaEntry() {}
    ElemAgendaEntry (DgnElementCP elRef)  : EditElementHandle ((DgnElementP)elRef) {}
    ElemAgendaEntry (ElementHandleCR from, bool duplicateDescr) : EditElementHandle (from, duplicateDescr) {}
    ElemAgendaEntry (ElemAgendaEntry const& from) : EditElementHandle(from,false) {}

    ElemAgendaEntry& operator= (ElemAgendaEntry const& from)
        {
        if (this == &from)
            return *this;

        Clone(from);
        return *this;
        }
};

enum class AgendaHilitedState
{
    Unknown  = 0,    //!< this agenda is in an indeterminate state wrt hilite.
    Yes      = 1,    //!< all of the entries in this agenda were hilited by a call to ElementAgenda::Hilite
    No       = 2,    //!< all of the entries in this agenda were unhilited by a call to ElementAgenda::ClearHilite
};


typedef ElemHandleArray<ElemAgendaEntry>      T_AgendumVector;
//=======================================================================================
//!  A bvector of EditElementHandle entries to be used for operating on groups of elements. Typically, ElementAgendas are created
//!  by the DgnPlatform framework from sources that work on multiple elements such as the SelectionSet and Fences. However, as a general-
//!  purpose bvector of EditElemHandles, ElementAgendas can be created by applications for other purposes as well. ElementAgendas are themselves
//!  reference counted.
//!
//!  <h3>Collections and Iterators</h3>
//!    ElementAgenda is a bvector of EditElementHandle. For example:
//! \code
//! ElementAgenda ag;
//! ag.Insert (ref1, model);
//! ag.Insert (ref2, model);
//! ...
//!
//! for each (ElementHandleCR eh in ag)
//!     {
//!     printf ("%llu\n", eh.GetElementId());
//!     }
//! \endcode
//!
//!  \ingroup ElemHandleCollections
//  @bsiclass
//=======================================================================================
struct ElementAgenda : RefCounted <T_AgendumVector>
{
private:
    ModifyElementSource m_source;
    AgendaHilitedState  m_hilitedState;

//__PUBLISH_SECTION_END__
    DGNVIEW_EXPORT void Restart ();
    DGNVIEW_EXPORT void SetEntriesHiliteFlag (ElementHiliteState hiliteState, bool checkSS);
    DGNVIEW_EXPORT void ModifySubAgenda (EditElementHandleP first, EditElementHandleP last, IModifyElementP op, bool redraw, ElementAgendaP reselectAgenda);

//__PUBLISH_SECTION_START__
public:

//=======================================================================================
//! @ingroup ElemHandleCollections
//! Iterator for ElementAgenda entries.
// @bsiclass
//=======================================================================================
    class       Iterator : public IElementSet
        {
    private:
        EditElementHandleCP m_first, m_curr, m_last;

        bool GetCurrent (ElementHandle& val)
            {
            if ((NULL==m_curr) || (m_curr > m_last))
                return false;

            val = *m_curr;
            return  true;
            }

    public:
        Iterator (EditElementHandleCP first, EditElementHandleCP last)
            {
            m_curr = m_first = first;
            m_last = last;
            }

        virtual bool GetFirst (ElementHandle& val) override
            {
            m_curr = m_first;
            return GetCurrent (val);
            }
        virtual bool GetNext (ElementHandle& val) override
            {
            m_curr++;
            return GetCurrent (val);
            }
        };

    //! Construct a new, empty, ElementAgenda.
    DGNPLATFORM_EXPORT ElementAgenda();
    DGNPLATFORM_EXPORT virtual ~ElementAgenda();

    //! Draw all of the entries in this ElementAgenda in a single DgnViewport. Technically, this is a "redraw" operation.
    //! @param[in]      viewport        The viewport in which to draw the agenda
    //! @param[in]      drawMode        The drawmode for the draw. Generally only DgnDrawMode::Normal is useful.
    //! @param[in]      drawPurpose     The purpose argument to be passed to all ElementHandler to describe why this draw operation is
    //!                                     taking place.   Be as specific as possible, but if no values seem correct, use DrawPurpose::NotSpecified
    //! @param[in]      redrawOp        This object is notified at various times during the redraw operation. Can be NULL.
    //! @param[in]      clip            A ClipVector that clips the elements within the viewport. Can be NULL.
    //! @param[in]      abort           An object that is called periodically during the redraw operation to determine whether the operation is taking
    //!                                     too long and should be aborted.
    DGNVIEW_EXPORT void Draw (DgnViewportP viewport, DgnDrawMode drawMode, DrawPurpose drawPurpose, IRedrawOperationP redrawOp, ClipVectorCP clip, IRedrawAbortP abort);

    //! Draw all of the entries in this ElementAgenda in all visible Viewports. Technically, this is a "redraw" operation.
    //! @param[in]      drawMode        The drawmode for the draw. Generally only DgnDrawMode::Normal is useful.
    //! @param[in]      drawPurpose     The purpose argument to be passed to all ElementHandler to describe why this draw operation is
    //!                                     taking place.   Be as specific as possible, but if no values seem correct, use DrawPurpose::NotSpecified
    //! @param[in]      redrawOp        This object is notified at various times during the redraw operation. Can be NULL.
    //! @param[in]      clip            A ClipVector that clips the elements within the viewport. Can be NULL.
    //! @param[in]      abort           An object that is called periodically during the redraw operation to determine whether the operation is taking
    //!                                     too long and should be aborted.
    DGNVIEW_EXPORT void DrawInAllViews (DgnDrawMode drawMode, DrawPurpose drawPurpose, IRedrawOperationP redrawOp, ClipVectorCP clip, IRedrawAbortP abort);

    //! Empty this ElementAgenda. Does not free memory associated with the agenda. First calls #ClearHilite.
    DGNVIEW_EXPORT void Clear ();

    //! Set the capacity for this ElementAgenda. If you expect to add a large number of entries to the agenda, and if you know how many
    //! there will be, then calling this method first can improve performance.
    //! @param[in]      nEntries        Set the array hold at least this number of entries.
    void SetCapacity (int nEntries) {reserve (nEntries);}

    //! Get the source for this ElementAgenda, if applicable. The "source" is merely an indication of what the collection of elements
    //! in this agenda means. Also, if the source is ModifyElementSource::SelectionSet, the Selection
    //! Set is kept current with changes to the agenda.
    ModifyElementSource GetSource () const {return m_source;}

    //! Set the source for this ElementAgenda.
    //! @see #GetSource
    void SetSource (ModifyElementSource val) {m_source = val;}

    //! Attempt to find an DgnElementP in this ElementAgenda.
    //! @param[in]     elRef            The DgnElementP to find
    //! @param[in]     startIndex       The index of the first entry to be considered for the Find operation.
    //! @param[in]     endIndex         The index of the last entry to be considered for the Find operation.
    //! @return A pointer to the EditElementHandle holding the element, or NULL if not found.
    DGNPLATFORM_EXPORT EditElementHandleCP Find(DgnElementCP elRef, size_t startIndex = 0, size_t endIndex = -1) const;
//__PUBLISH_SECTION_END__

    //! Insert an DgnElementDescr into this ElementAgenda.
    //! @param[in]     elDscr           An DgnElementDescr that is to be inserted into this agenda.
    //! @param[in]     atHead           If true, put this in at the head (front) of the agenda, otherwise put it at the end. Passing false is more efficient.
    //! @return A pointer to the EditElementHandle within the ElementAgenda holding \c elDscr.
    //! @note After this call, \c elDscr is owned by this ElementAgenda and should NOT be freed by the caller!
    //! @note For performance reasons, no attempt is made to enforce uniqueness of the entries (i.e. this method will allow duplicate
    //! entries for the same element, even though that's generally undesirable.) If you're unsure whether the entry you're adding
    //! may already be in the agenda, call #Find. Also, see Insert(bset<DgnElementP>&) for more efficient technique to enforce uniqueness for large sets.
    DGNPLATFORM_EXPORT EditElementHandleP InsertElemDescr (DgnElementP elDscr, bool atHead=false);
//__PUBLISH_SECTION_START__

    //! Insert an element into this ElementAgenda.
    //! @param[in]      eeh             The EditElementHandle that is to be inserted into this agenda; if it has a descriptor, it will be extracted and used; otherwise an element ref will be used. If neither are present, nothing is added.
    //! @param[in]      atHead          If true, put this in at the head (front) of the agenda, otherwise put it at the end. Passing false is more efficient.
    //! @return A pointer to the EditElementHandle within the ElementAgenda holding \c elDscr.
    //! @note If the EditElementHandle you provide has a descriptor, this will extract and then own the descriptor; you can thus not use the EditElementHandle; use this method's return value instead.
    //! @note For performance reasons, no attempt is made to enforce uniqueness of the entries (i.e. this method will allow duplicate entries for the same element, even though that's generally undesirable.) If you're unsure whether the entry you're adding may already be in the agenda, call #Find. Also, see Insert(bset<DgnElementP>&) for more efficient technique to enforce uniqueness for large sets.
    DGNPLATFORM_EXPORT EditElementHandleP Insert (EditElementHandleR eeh, bool atHead = false);

    //! Insert an DgnElementP into this ElementAgenda.
    //! @param[in]      elRef           The DgnElementP to insert
    //! @param[in]      atHead          If true, put this in at the head (front) of the agenda, otherwise put it at the end. Passing false is more efficient.
    //! @return A pointer to the EditElementHandle within the ElementAgenda holding the newly inserted entry.
    //! @note For performance reasons, no attempt is made to enforce uniqueness of the entries (i.e. this method will allow duplicate
    //! entries for the same element, even though that's generally undesirable.) 
    DGNPLATFORM_EXPORT EditElementHandleP Insert (DgnElementCP elRef, bool atHead=false);

    //! Insert an DgnElementP from a DisplayPath into this ElementAgenda. Optionally, add all of the other members of
    //! graphic groups or named groups containing the element.
    //! @param[in]      path            The path to insert into this ElementAgenda. The DgnElementP is taken from the HitElem and the DgnModel is taken
    //!                                 from the PathRoot.
    //! @param[in]      doGroups        If true add all other members of graphic or named groups containing the element.
    //! @param[in]      allowLocked     If \c doGroups is true, allow members of groups that are locked to be inserted into the ElementAgenda.
    //!                                     Generally ElementAgendas are used for performing modification and locked elements are ineligible.
    //! @return A pointer to the EditElementHandle within the ElementAgenda holding the newly inserted entry. In the case where \c doGroups is
    //! true and more than one element is added to this agenda, the return is the entry for the original element from \c path.
    //! @note For performance reasons, no attempt is made to enforce uniqueness of the entries (i.e. this method will allow duplicate
    //! entries for the same element, even though that's generally undesirable.) If you're unsure whether the entry you're adding
    //! may already be in the agenda, call #Find. Also, see Insert(bset<DgnElementP>&) for more efficient technique to enforce uniqueness for large sets.
    DGNVIEW_EXPORT EditElementHandleP InsertPath (DisplayPathCP path, bool doGroups, bool allowLocked);

    //! Insert an DgnElementP from a DisplayPath into this ElementAgenda. Optionally, add all of the other members of
    //! graphic groups or named groups containing the element.
    //! @param[in]      path            The path to insert into this ElementAgenda. The DgnElementP is taken from the HitElem and the DgnModel is taken
    //!                                     from the PathRoot.
    //! @param[in]      doGroups        If true add all other members of graphic or named groups containing the element.
    //! @param[in]      groupLockState  Interpret group traversal rules according to this state of group lock. See NamedGroup class.
    //! @param[in]      allowLocked     If \c doGroups is true, allow members of groups that are locked to be inserted into the ElementAgenda.
    //!                                     Generally ElementAgendas are used for performing modification and locked elements are ineligible.
    //! @return A pointer to the EditElementHandle within the ElementAgenda holding the newly inserted entry. In the case where \c doGroups is
    //! true and more than one element is added to this agenda, the return is the entry for the original element from \c path.
    //! @note For performance reasons, no attempt is made to enforce uniqueness of the entries (i.e. this method will allow duplicate
    //! entries for the same element, even though that's generally undesirable.) If you're unsure whether the entry you're adding
    //! may already be in the agenda, call #Find. Also, see Insert(bset<DgnElementP>&) for more efficient technique to enforce uniqueness for large sets.
    DGNPLATFORM_EXPORT EditElementHandleP InsertPath (DisplayPathCP path, bool doGroups, bool groupLockState, bool allowLocked);

    //! Add a set of elements to this agenda. This call guarantees uniqueness of the entries in the ElementAgenda (presuming the existing
    //! entries are unique before this call.)
    //! @param[in,out] elemSet  On input, the set of unique DgnElementP entries to add to this ElementAgenda. On output, the set of entries actually added to this agenda.
    //! @remarks This function modifies \c elemSet by removing entries that are already in this agenda.
    //! @note If this ElementAgenda is not empty before this call, all of the existing entries are compared for uniqueness against
    //! the entries in \c elemSet so that the resultant ElementAgenda is also a unique set. For example, if you were to call this method twice
    //! with the same elemSet, the second call would do nothing.
    DGNPLATFORM_EXPORT void Insert(bset<DgnElementCP>& elemSet);

    //! Perform a modification operation on all of the entries in this ElementAgenda.
    //! @param[in]      modifyOp        The operation to be performed on the agenda. The implementation determine what happens to the elements in the ElementAgenda.
    //!                                     Must not be NULL.
    //! @param[in]      redraw          If true, redraw the result of the operation immediately. Otherwise it will appear at the end of the transaction. If
    //!                                     the operation has a visible effect it is best to pass true for this so the user is presented with feedback when processing large groups.
    //!                                     The tradeoff is that redrawing immediately can make the total time to complete the operation longer.
    //! @return SUCCESS if this ElementAgenda was not empty and \c modifyOp was called.
    //! @note If \c modifyOp changes the elements as they are processed, this ElementAgenda can, and most likely will, be different after
    //! the call. For example, if \c modifyOp deletes elements, they are removed from the agenda.
    DGNVIEW_EXPORT StatusInt ModifyAgenda (IModifyElementP modifyOp, bool redraw);

    //! Populate this ElementAgenda from a set of elements. The current contents of this agenda are cleared before it is loaded from the set.
    //! @param[in]      elementSet      The collection of elements that are to be held by this agenda after this call.
    //! @param[in]      source          The value to be returned by #GetSource after this call.
    //! @return SUCCESS if at least one element was added to the agenda from \c elementSet.
    DGNVIEW_EXPORT StatusInt BuildFromElementSet (IElementSetP elementSet, ModifyElementSource source);

    //! Populate this ElementAgenda from the elements visible in a DgnViewport.
    //! @param[in] viewPort all elements visible in this viewPort are considered eligible for the agenda.
    //! @return SUCCESS if the resultant ElementAgenda has at least one entry.
    DGNPLATFORM_EXPORT void BuildFromViewport (DgnViewportR viewPort);

    //! Mark all entries in this agenda as being hilited and then redraw the agenda so that fact is visible to the user. The agenda itself
    //! also holds a flag indicating whether its entries are all in the hilite state so that calls to #ClearHilite can reverse that.
    //! @note Any calls to one of the Insert methods clears to hilite flag on the ElementAgenda, so that #ClearHilite will not do anything.
    DGNVIEW_EXPORT void Hilite ();

    //! If entries in agenda are drawn in hilite by a previous call to #Hilite, they are unhilited.
    DGNVIEW_EXPORT void ClearHilite ();

}; // ElementAgenda

/*=================================================================================**//**
* Interface for an agent that can display elements
* @bsiclass                                     Sam.Wilson                      04/2010
+===============+===============+===============+===============+===============+======*/
struct          IDrawElementAgenda
{
private:
    virtual void _DrawElementAgenda (ElementAgendaR agenda, DgnDrawMode drawMode, DrawPurpose purpose) = 0;

public:
    //! Callback invoked to display changed elements
    DGNPLATFORM_EXPORT void DrawElementAgenda (ElementAgendaR agenda, DgnDrawMode drawMode, DrawPurpose purpose);
}; // IDrawElementAgenda

#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
