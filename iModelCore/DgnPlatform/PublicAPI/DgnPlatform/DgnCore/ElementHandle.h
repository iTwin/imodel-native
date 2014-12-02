/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementHandle.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "../DgnPlatform.h"
#include "Handler.h"
#include "DgnFile.h"
#include "MSElementDescr.h"
#include "DgnModel.h"
#include "XAttributeIter.h"
#include "XAttributeChange.h"

DGNPLATFORM_TYPEDEFS (ElementLinkageIterator)
DGNPLATFORM_TYPEDEFS (ConstElementLinkageIterator)
//__PUBLISH_SECTION_END__
DGNPLATFORM_TYPEDEFS (XAttributeChangeIter)
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct ITxn;
class  Handler;

// =======================================================================================
//! An element handler can use these flags to specify what DgnPlatform can do with the element when the handler is not available.
// =======================================================================================
enum            MissingHandlerPermissions
{
    MISSING_HANDLER_PERMISSION_None             = 0x0000,       //!< Disallow all operations on the element when handler is not available.
    MISSING_HANDLER_PERMISSION_Delete           = (1<<0),       //!< OK to delete the element
    MISSING_HANDLER_PERMISSION_Copy             = (1<<1),       //!< OK to create a copy of the element (Note: Element Handler XAttribute is also copied)
//__PUBLISH_SECTION_END__
    MISSING_HANDLER_PERMISSION_UNUSED__         = (1<<2),
//__PUBLISH_SECTION_START__
    MISSING_HANDLER_PERMISSION_Draw             = (1<<3),       //!< OK to display the element according to its element type. Also includes permission to snap to the element or to measure it.
    MISSING_HANDLER_PERMISSION_Manipulators     = (1<<4),       //!< OK to display and possibly use default manipulators, according to its DgnPlatform element type
    MISSING_HANDLER_PERMISSION_Move             = (1<<5),       //!< OK to move (translate) the element
    MISSING_HANDLER_PERMISSION_Transform        = (1<<6),       //!< OK to transform (mirror, rotate, scale, stretch) the element in arbitrarily complicated ways
    MISSING_HANDLER_PERMISSION_CurveEdit        = (1<<7),       //!< OK to break, add vertices to, or extend this curve (1-D curves only)
//__PUBLISH_SECTION_END__
    MISSING_HANDLER_PERMISSION_ChangeMerge      = (1<<8),       //!< OK to replicate changes made to this element from file to file (Note: MISSING_HANDLER_PERMISSION_Delete, MISSING_HANDLER_PERMISSION_Copy, MISSING_HANDLER_PERMISSION_Transform, etc. are not checked during change-merging)
//__PUBLISH_SECTION_START__
    MISSING_HANDLER_PERMISSION_ChangeAttrib     = (1<<9),       //!< OK to change the element attributes (symbology, transparency, ...)
    MISSING_HANDLER_PERMISSION_All_             = 0xffffffff,   //!< Mask of all defined default flags.
};

//__PUBLISH_SECTION_END__

/*=================================================================================**//**
* Navigates the set of scheduled changes to XAttributes on an EditElementHandle.

    An XAttribute change set will never contain more than one change to the same XAttribute.

    In order to see all changes to attributes with a given handler, you can iterate
    the entire change set, looking for handlerId. Or, you can call Search and then SearchNext,
    specifying a handlerId.

    It does not make sense to call ToNext after calling Search or SearchNext.
    Likewise, it makes no sense to call SearchNext after calling Search with
    a different handlerid. That is because item order in an XAttribute change set is
    undefined. Therefore, it makes not sense to start an iteration or search from
    some point in the midst of the set.
+===============+===============+===============+===============+===============+======*/
struct XAttributeChangeIter
{
private:
    bset<XAttributeChange>::const_iterator m_changeSetIter;
    XAttributeChangeSetP m_changeSet;

    DGNPLATFORM_EXPORT void ToFirst ();
    DGNPLATFORM_EXPORT void Init (MSElementDescrCP);

public:
    DGNPLATFORM_EXPORT XAttributeChangeIter (XAttributeChangeSetP);

    //! Get Change set
    XAttributeChangeSetP GetXAttributeChangeSet () const {return m_changeSet;}

public:
    //! Get a reference to the scheduled change, if any, to the specified XAttribute.
    explicit DGNPLATFORM_EXPORT XAttributeChangeIter (ElementHandleCR, XAttributeHandlerId=XAttributeHandlerId(0,0), UInt32 xAttrId=XAttributeHandle::MATCH_ANY_ID);

    //!  Test if the search or navigation operation that yielded this iterator found
    //!  a scheduled change or failed.
    DGNPLATFORM_EXPORT bool IsValid () const;

    //!  Test if iterator is positioned at the beginning
    DGNPLATFORM_EXPORT bool IsBeginning () const;

    //!  Navigate to the next scheduled change (of any XAttribute).
    //! @return false if this iterator has reached the end of the scheduled changes.
    DGNPLATFORM_EXPORT bool ToNext ();

    //! Find change to a specific XAttribute or an instance of a change to an XAttribute with a specified handler.
    //! @remarks If XattrId is not specified or is MATCH_ANY_ID or if current position is not at the
    //! beginning, this does a brute-force linear search, starting from the current position.
    DGNPLATFORM_EXPORT bool Search (XAttributeHandlerIdCR, UInt32 xAttrId=XAttributeHandle::MATCH_ANY_ID);

    //! Find another instance of a change to an XAttributes with a specified handler.
    //! @remarks This does a brute-force linear search, starting from the next position after the current iterator position.
    bool SearchNext (XAttributeHandlerIdCR h, UInt32 i=XAttributeHandle::MATCH_ANY_ID) {ToNext(); return Search(h,i);}

public:
    //!  Get access to the Change record referenced by this iterator
    DGNPLATFORM_EXPORT XAttributeChangeCR operator *() const;

    //!  Get access to the Change record referenced by this iterator
    XAttributeChangeCP operator->() const {return (&**this);}

    DGNPLATFORM_EXPORT bool operator==(XAttributeChangeIter const&) const;
    DGNPLATFORM_EXPORT bool operator!=(XAttributeChangeIter const&) const;

    DGNPLATFORM_EXPORT XAttributeChangeIter& operator++();
};

//__PUBLISH_SECTION_START__

/*=================================================================================**//**
* @addtogroup ElemHandles
An ElementHandle is an indirect reference to an element's current state. It permits access to the element transparently, whether the underlying
element data is held in an MSElementDescr or merely referenced through an ElementRef. An ElementHandle is the "this" pointer for all
methods of \ref ElementHandler.

An ElementHandle provides readonly access to the underlying element. The subclass EditElementHandle adds methods that allow modification of the
underlying element.

It should be obvious that for time critical bulk operations that merely read but don't change existing elements, access through an ElementRef is
more efficient and preferable to MSElementDescr. An ElementRef contains a direct pointer to the element data in memory,
whereas an MSElementDescr is a alloced/freed copy of the data. However, if you already have an MSElementDescr, or if the element is being
modified or created in memory via an MSElementDescr, then it is not possible to use an ElementRef. ElementHandle provides the mechanism to abstract
access to an element, using the ElementRef where possible and reading the MSElementDescr only when necessary.
<h3>ElementHandle Lifecycle</h3>
ElementHandle are lightweight objects and are usually stack-based. ElementHandle can be constructed from either an ElementRef or via
an existing MSElementDescr. When constructing an ElementHandle from an existing MSElementDescr (which should be avoided when dealing with persistent
elements in favor of using the ElementRef constructor), you must indicate whether the ElementHandle is to "own" the MSElementDescr or not.
If an ElementHandle holds an "owning" reference to an MSElementDescr, its destructor releases the reference and frees the MSElementDescr. Whenever 
an ElementHandle reads an MSElementDescr on behalf of a caller, it saves the MSElementDescr as an owning reference. In this manner, ElemHandles can 
be used much like a "smart pointer" for MSElementDescr's.
+===============+===============+===============+===============+===============+======*/

#if !defined (DOCUMENTATION_GENERATOR)
//=======================================================================================
//! Interface implemented to hold additional information about an element held by an ElementHandle.
//=======================================================================================
struct     IElementState : public Bentley::IRefCounted {};
#endif

//=======================================================================================
//! A readonly "handle" to an element.
//! @ingroup ElemHandles
//=======================================================================================
struct ElementHandle
{
//__PUBLISH_SECTION_END__
    friend class Handler;

protected:
//__PUBLISH_SECTION_START__

#if !defined (DOCUMENTATION_GENERATOR)
protected:
    ElementRefPtr               m_elmRef;
    mutable MSElementDescrPtr   m_dscr; // NOTE: When both m_dscr and m_elmRef are valid, it is assumed that MSElementDescr reflects the persistent element state.
    IElementStateP              m_state;

    DGNPLATFORM_EXPORT void Clone (ElementHandle const& from);
    void ClearElementDescr() {m_dscr = NULL;}
    void ClearElementRef() {m_elmRef = NULL;}
    void ClearElemState()
        {
        if (m_state)
            {
            m_state->Release();
            m_state = NULL;
            }
        }

    void ClearAll() {ClearElementDescr(); ClearElemState(); ClearElementRef();}
    void AssignElementRef (ElementRefP elRef)
        {
        ClearElementDescr();
        ClearElementRef();
        m_elmRef       = elRef;
        }

    void Invalidate () {ClearAll();}
    DGNPLATFORM_EXPORT void AssignElemDescr (MSElementDescrP, bool isPersistent);
    DGNPLATFORM_EXPORT DgnElementCP GetUnstableMSElementCP () const;

    DGNPLATFORM_EXPORT void Init (ElementRefP elRef);
    DGNPLATFORM_EXPORT void Init (MSElementDescrCP elDscr, bool isPersistent);
    DGNPLATFORM_EXPORT void Init (ElementId, DgnProjectR);
#endif

//__PUBLISH_SECTION_END__
public:
    DRange3dCP CheckIndexRange () const;
    DGNPLATFORM_EXPORT DRange3dCP GetIndexRange () const;

    //! Get the XAttributeChangeSet, if any, associated with this element in memory
    DGNPLATFORM_EXPORT XAttributeChangeSetP QueryXAttributeChangeSet () const;

    //! Get the XAttributeChangeSet associated with this element in memory or create an empty one if necessary.
    DGNPLATFORM_EXPORT XAttributeChangeSetP GetXAttributeChangeSet () const;

//__PUBLISH_SECTION_START__
public:
    ElementHandle () {Init (NULL);} //!< construct a blank, invalid, ElementHandle
    ~ElementHandle () {ClearAll();}

    //! Construct an ElementHandle from an ElementRefP and a DgnModelP.
    //! @param[in]  elRef       The ElementRefP of the element for this ElementHandle.
    explicit ElementHandle (ElementRefP elRef) {Init (elRef);}

    //! Construct an ElementHandle from an ElementId and a DgnModel.
    //! @param[in] id The ElementId of the element for this ElementHandle.
    //! @param[in] model The DgnModel used to access the element.
    //! @remarks NOTE: test IsValid to determine whether the element was found.
    ElementHandle (ElementId id, DgnModelR model) {Init(id, model.GetDgnProject());}

    //! Construct an ElementHandle from an ElementId and a DgnFile
    //! @param[in] id The ElementId of the element for this ElementHandle.
    //! @param[in] project the DgnProject of the element.
    //! @remarks NOTE: test IsValid to determine whether the element was found.
    ElementHandle (ElementId id, DgnProjectR project) {Init(id, project);}

    //! Construct an ElementHandle from an MSElementDescr.
    //! @param[in]  elDscr       MSElementDescr to be referenced by this ElementHandle.
    //! @param[in]  isUnmodified if true and elDscr->GetElementRef(), \a elDscr is considered to be as an exact image of the element in the model.
    //!                          When this is true, users of this ElementHandle may choose to use the ElementRef instead of \a elDscr. 
    //!                          For example, the display code may choose to use a previously-cached presentation to draw the element. If you're not sure, pass false.
    ElementHandle (MSElementDescrCP elDscr, bool isUnmodified=false) {Init (elDscr, isUnmodified);}

    //! Construct an ElementHandle from an DgnElement and a DgnModel. This technique is rarely useful or necessary. 
    //! This constructor merely allocates an MSElementDescr from the element.
    //! @param[in]  el          Element from which the MSElementDescr for this ElementHandle should be created.
    //! @param[in]  model       DgnModel for element.
    //! @remarks NOTE: This means that the ElementHandle points to a \b copy of the input element.
    DGNPLATFORM_EXPORT ElementHandle (DgnElementCR el, DgnModelR model);

    DGNPLATFORM_EXPORT ElementHandle (ElementHandleCR from); //!< Copy an ElementHandle
#if !defined (DOCUMENTATION_GENERATOR)
    ElementHandleR operator= (ElementHandleCR from)
        {
        if (this == &from)
            return *this;

        Clone(from);
        return *this;
        }
#endif

/** @name Handler Queries */
/** @{ */
    //! Get the Handler for this ElementHandle. Every element must have a handler. This method returns a reference to the Handler
    //! for this element. If this ElementHandle has an ElementRefP, its handler is returned. Otherwise, the MSElementDescr is used.
    //! @note If you attempt to get the Handler for an Invalid ElementHandle, it will return a "NullHandler" that does nothing.
    DGNPLATFORM_EXPORT Handler& GetHandler () const;

    //! A shortcut method to get a DisplayHandler pointer for the Handler of this ElementHandle, or NULL if the element's Handler
    //! does not derive from DisplayHandler.
    DisplayHandlerP GetDisplayHandler () const {return GetHandler().GetDisplayHandler();}

    //! A shortcut method to get the ITextQuery interface on the Handler for this ElementHandle, or NULL if the element's Handler
    //! does not implement that interface.
    DGNPLATFORM_EXPORT ITextQueryCP GetITextQuery () const;

    //! A shortcut method to get the ITextEdit interface on the Handler for this ElementHandle, or NULL if the element's Handler
    //! does not implement that interface.
    DGNPLATFORM_EXPORT ITextEditP GetITextEdit () const;
/** @} */

/** @name Element Data Queries */
/** @{ */
    //! Get the ElementRefP for this ElementHandle.
    //! @return the ElementRefP, or NULL.
    ElementRefP GetElementRef () const {return m_elmRef.get();}

    //! Get the DgnModelP for this ElementHandle.
    //! @return the DgnModelP. First checks ElementRefP, if present, and then resorts to using MSElementDescr.
    //! @note This method will only return NULL for an invalid ElementHandle.
    DGNPLATFORM_EXPORT DgnModelP GetDgnModelP () const;

    //! Get the DgnProject for this ElementHandle.
    //! @return the DgnProject.
    //! @note This method will only return NULL for an invalid ElementHandle.
    DgnProjectP GetDgnProject() const {DgnModelP dgnCache = GetDgnModelP (); return (NULL == dgnCache) ? NULL : &dgnCache->GetDgnProject();}

    //! Get a const pointer to the DgnElement associated with this ElementHandle.
    //! @return a const pointer the DgnElement associated with this ElementHandle.
    //! @remarks The pointer returned by this method \em must be treated as const. It is \em never valid or legal to cast away the
    //!          const-ness of this pointer. Doing so will result in either an access violation or a corrupted cache.
    DgnElementCP GetElementCP () const {return m_dscr.IsValid() ? &m_dscr->Element() : GetUnstableMSElementCP();}

    //! Determine whether this ElementHandle is currently valid.
    //! @return true if this ElementHandle references a valid element. If not, all other methods on this ElementHandle will either fail or crash.
    bool IsValid () const {return m_elmRef.IsValid() || m_dscr.IsValid();}

    //! Determine whether this ElementHandle references an unmodified element in the cache.
    //! @return true if ElementHandle is unmodified representation of element in the cache.
    bool IsPersistent () const {return NULL != GetElementRef ();}

    //! Get a copy of the Elm_hdr of the element referenced by this ElementHandle.
    void GetElementHeader (DgnElementHeaderR hdr) const {hdr = *GetElementCP();}

    //! Peek to see whether this ElementHandle currently has an MSElementDescr.
    //! @return the current MSElementDescr or NULL.
    MSElementDescrCP PeekElementDescrCP () const {return m_dscr.get();}

    //! Get an MSElementDescrCP from this ElementHandle. If this ElementHandle does not already have an MSElementDescr, allocate one using the ElementRef.
    //! @remarks This call should \em only be used where necessary, and should be avoided in readonly, time critical cases. In particular
    //! DisplayHandler::Draw methods should avoid this call since the alloc/free overhead can be substantial.
    //! @remarks Use #PeekElementDescrCP to see whether this ElementHandle already has an MSElementDescr.
    //! @remarks This method returns a pointer to the MSElementDescr in the ElementHandle. It is still owned by the ElementHandle and \b must \b not be released by the caller.
    DGNPLATFORM_EXPORT MSElementDescrCP GetElementDescrCP () const;
/** @} */

//__PUBLISH_SECTION_END__
    //!  Query if there are any scheduled changes for this element descriptor.
    DGNPLATFORM_EXPORT bool AnyXAttributeChanges () const;

// =======================================================================================
//! Iterate over XAttributes on an element. This iterator presents a view of the
//! element's XAttributes comprised of scheduled XAttribute changes and persistent XAttributes. By
//! contrast, ElementXAttributeIter reports only persistent XAttributes, and XAttributeChangeIter reports
//! only XAttribute changes.
//!
//! During iteration, an XAttribute change takes precedence over the persistent version, as follows:
//!  - deleted - report as not found
//!  - added - return new XAttribute
//!  - replaced - return modified XAttribute
//!  - historical version - return historical version of XAttribute
//!  - historical version did not exist - report as not found
//!
//! A given XAttribute is reported only once, whether from the change set or from the persistent set.
//! @ingroup XAttributes
// =======================================================================================
class XAttributeIter
    {
        private:
            enum State {STATE_StartChanges, STATE_InChanges, STATE_InPersistent, STATE_NotFound};
            UInt32                      m_id;
            XAttributeChangeIter        m_ci;
            XAttributeCollection        m_collection;
            XAttributeCollection::Entry m_entry;
            State                       m_state;

            friend struct ElementHandle;
            void SetEnd () {m_state = STATE_NotFound;}

        public:
            //! Construct an ElemHandleXAttributeIter for the persistent XAttributes and XAttribute changes on the specified element handle.
            DGNPLATFORM_EXPORT explicit XAttributeIter (ElementHandleCR eh, XAttributeHandlerId handlerId=XAttributeHandlerId(0,0), UInt32 id=XAttributeHandle::MATCH_ANY_ID);

            //! Move to the next matching XAttribute of this element.
            //! @return true if the iterator is valid, false if there were no more XAttributes.
            //! @remarks This method will always move to the next XAttribute on the current element <i>regardless of its XAttributeHandlerId.</i>
            //!          It does \b not apply any filtering, even if this iterator was constructed with a specific XAttributeHanderId. To find the
            //!          next XAttribute of a specific type, use #SearchNext.
            DGNPLATFORM_EXPORT bool ToNext ();

            //! Determine whether this XAttributeHandle is valid or not.
            //! @return true if this XAttributeHandle is valid. If this method returns false, all other methods on this XAttributeHandle
            //! will fail or crash.
            DGNPLATFORM_EXPORT bool IsValid () const;

            DGNPLATFORM_EXPORT XAttributeIter& operator++ ();
            DGNPLATFORM_EXPORT bool operator== (XAttributeIter const&) const;
            DGNPLATFORM_EXPORT bool operator!= (XAttributeIter const& r) const {return !(*this == r);}

            //! Get the XAttributeId of the XAttribute this XAttributeHandle references.
            DGNPLATFORM_EXPORT UInt32 GetId () const;

            //! Get the XAttributeHandlerId of the XAttribute this XAttributeHandle references.
            DGNPLATFORM_EXPORT XAttributeHandlerId GetHandlerId () const;

            //! Get the number of bytes of data in the XAttribute this XAttributeHandle references.
            DGNPLATFORM_EXPORT UInt32 GetSize () const;

            //! Get a \c const pointer to the data for the XAttribute this XAttributeIter references. This pointer is a direct pointer into
            //! the element cache. It will remain valid while this XAttributeIter is valid and referencing this XAttribute,
            //! but will \b not be valid thereafter. Therefore, do not save this pointer outside of methods that work directly with this XAttributeIter.
            //! @remarks It is \em never valid or legal to cast away the const-ness of this pointer. Doing so will result in a crash, corrupted
            //!          data in the element cache, corrupted files, and many other bad things. <b>DO NOT DO THAT!!!</b> If you are unsure whether
            //!          your code is safe, allocate a buffer and copy the XAttribute data there before using it.
            DGNPLATFORM_EXPORT void const* PeekData () const;

            //! If this identifies a persistent XAttribute, this function will return the ElementXAttributeIter.
            //! If this identifies an XAttribute change or if this is invalid, this function will return NULL;
            DGNPLATFORM_EXPORT XAttributeCollection::Entry const* GetElementXAttributeIter () const;

    }; // XAttributeIter

    //! Get an XAttributeIter for this handle
    DGNPLATFORM_EXPORT XAttributeIter GetXAttributeIter () const;

    //! Get an iterator over the element's XAttributes and scheduled XAttribute Changes
    DGNPLATFORM_EXPORT XAttributeIter BeginXAttributes () const;

    //! Get an iterator that marks the end of the element's XAttributes and scheduled XAttribute Changes
    DGNPLATFORM_EXPORT XAttributeIter EndXAttributes () const;

//__PUBLISH_SECTION_START__
/** @name Element Linkage Queries */
/** @{ */
    //! Get an iterator over the element's user data linkages
    //! @private
    //! @param rl   Linkage ID filter value identifies the linkages of interest. Defaults to all linkages.
    DGNPLATFORM_EXPORT ConstElementLinkageIterator BeginElementLinkages (UInt16 rl = 0) const;

    //! Get an iterator that marks the end of the element's user data linkages
    //! @private
    DGNPLATFORM_EXPORT ConstElementLinkageIterator EndElementLinkages () const;
/** @} */

    //! Get the element ID of the element contained in the ElementHandle.
    DGNPLATFORM_EXPORT ElementId GetElementId () const;
};

/*=================================================================================**//**
* A writeable "handle" to an DgnElement.
*
* EditElementHandle is the ideal means of passing an element to functions, when the functions might modify the element.
* EditElementHandle is also useful for creating and adding new elements and for deleting existing elements.
*
* <h4>EditElementHandle, MSElementDescrP, and ElementRefP</h4>
*
* Like an ElementHandle, an EditElementHandle can hold either an ElementRef or an MSElementDescr in order to represent an element.
* Unlike ElementHandle, an EditElementHandle can be used to modify the element data.
*<p>
* EditElementHandle is often set up to take ownership of an underlying MSElementDescr. This feature allows EditElementHandle,
* like ElementHandle, to serve as a smart pointer. As a read/write handle, EditElementHandle is also well suited for cases
* where the underlying MSElementDescr pointer must be reallocated. Use of EditElementHandle makes for cleaner, simpler, more reliable
* code and is preferable to MSElementDescrH.
*
* <h4>Common Use</h4>
*
* Some common workflows involving an EditElementHandle are:
*
* \li Create New Element
*<p>
* Creating a new element involves setting up an element descriptor in memory and then writing it to a cache.
* The handle ends up holding the new ElementRefP that represents the newly created element.
*   \code
    // Create the element data
    MSElementDescrP elementDescriptor = ...

    //  Set up handle to a new, non-persistent element descriptor
    //  Note that the handle takes ownership of the descriptor.
    EditElementHandle eh (elementDescriptor, true, true);

    //  Append the element to a cache.
    //  Note: AddToModel assigns an ElementRefP to the element.
    //        It switches the handle's internal representation to use the ElementRefP and frees the original descriptor.
    if (SUCCESS != eh.AddToModel ())
        error!
    else
        {
        BeAssert (eh.GetElementRef () != NULL);
        BeAssert (eh.IsPersistent());
        BeAssert (eh.PeekElementDescrCP() == NULL);    // The original descriptor has been discarded and freed!
        }
    \endcode
*
* \li Modify Existing Element
*<p>
* The modification scenario starts with an existing, unmodified element. The modification logic will normally
* work on an element descriptor. The handle then shifts to holding the modified descriptor. After writing the modified
* descriptor back to its cache, then handle switches back to representing an existing, unmodified element.
*   \code
    // Get a reference to the element
    ElementRefP elementRef = ...

    //  Set up handle to an existing, persistent element
    EditElementHandle eh (elementRef);
    elementRef = NULL;

    //  If you really want to get the ElementRefP, get it from the descriptor:
    // eh.GetElementDescrCP()->GetElementRef()

    //  Modify the element's data in some way, e.g., by calling a Handler method
    eh.GetHandler().SomeModificationMethod (eh);
    ...

    //  Write the changes
    //  Note: ReplaceInModel may possibly move the ElementRefP if the element changes in size.
    //        ReplaceInModel switches the handle's internal representation to use the (updated) ElementRefP and frees the original descriptor.
    if (eh.ReplaceInModel (oldRef) != SUCCESS)
        error!
    else
        {
        BeAssert (eh.GetElementRef () != NULL);
        BeAssert (eh.IsPersistent());
        // NB: eh.GetElementRef() may not equal the original elementRef!
        }
    \endcode
* <p>
* \li Use Legacy Code to Modify an Element Descriptor
*<p>
* Legacy code often works with element descriptors. This shows how to use the ExtractElementDescr method in order
* to hand off a descriptor and manage its lifecycle.
*   \code
    //  Set up handle to an existing, persistent element
    EditElementHandle eh (elementRef);

    //  Call legacy code, which will operate on an element descriptor.
    //  In this case the legacy code may reallocate the descriptor.
    eh.GetElementDescrP ();                            // Cause the handle to create and hold a descriptor.
    MSElementDescrP ed = eh.ExtractElementDescr ();    // take ownership of descriptor away from eh
    legacyFunction (&ed, ...);                         // legacy code modifies the naked descriptor
    eh.SetElementDescr (ed, false);              // return ownership of descriptor to eh
    ed = NULL;

    //  Write the changes
    if (eh.ReplaceInModel (oldRef) != SUCCESS)
        error!
    else
        {
        BeAssert (eh.GetElementRef () != NULL);
        BeAssert (eh.IsPersistent());
        }
    \endcode
*<p>
*
* <h4>The Persistent and Unmodified State</h4>
*
* Applications that work with legacy code may have to understand and even manage a handle's internal state.
* When following the common use scenarios above, an application can use an EditElementHandle without worrying
* too much about whether the handle contains an ElementRefP or an MSElementDescrP or whether the descriptor's
* elementRef field is valid or not. The EditElementHandle's state logic can manage this data in most cases.
* If the application follows a different workflow, however, it might have to control this information in the
* handle.
*<p>
* An EditElementHandle can represent an element in three possible states:
* -# <em>Persistent and unmodified.</em>
* DgnPlatform will assume that the EditElementHandle's ElementRefP represents the correct state of the element's data.
* The GetElementRef method will return non-null only in this state. The IsPersistent method will return true only in this state.
* Most of the time, the handle will contain only an ElementRefP in this state. In rare situations involving legacy code, the handle
* may contain both an ElementRefP and a MSElementDescr.
* -# <em>Persistent and modified.</em>
* DgnPlatform will assume that the EditElementHandle's ElementRefP does not reflect the element's current state.
* GetElementRef will return NULL and IsPersistent will return false in this state, even if the descriptor has
* its GetElementRef() field set up. This is to remind you to look in the descriptor for the element's data.
* -# <em>Non-persistent.</em>
* The EditElementHandle contains only an element descriptor.
*<p>
* Applications can control an EditElementHandle's persistent status by means of the \a isUnmodified argument to the
* element descriptor constructor and the SetElementDescr method. Normally, \a isUnmodified is set to false. That is, if
* you are setting up an EditElementHandle from a descriptor, you are probably creating a new element or have
* modified its data. If you want to represent a persistent, unmodified element, the preferred method is to specify
* an ElementRefP. When using legacy code, however, an application might possibly start by reading a persistent
* element into an element descriptor. If the application then wants to switch over to using an EditElementHandle, it
* can pass isUnmodified = true. That will cause the EditElementHandle to adopt the descriptor's ElementRefP and also
* to hold the descriptor pointer. The following example shows how to create a persistent and unmodified handle
* that also acts as a smart pointer.
*
    \code
    EditElementHandle eh (elementDescriptor, true);
    eh.GetElementDescrP ();    // already has a descriptor, so no need to read it
    eh.GetElementRef ();       // also has an ElementRefP

    // eh will free elementDescriptor
    \endcode
*<p>
*
* <h4>Scheduling XAttribute Changes</h4>
*
* You can use EditElementHandle to add, replace, or delete an element's XAttributes. Unlike XAttrIO, EditElementHandle
* \em schedules XAttribute changes. Scheduled changes are written to the cache when you rewrite the handle's
* underlying MSElementDescr (e.g., by using the AddToModel or ReplaceInModel methods).
*<p>
* The XAttributeChangeIter class can be used to inspect scheduled changes.
*<p>
* The ElementHandle::XAttributeIter class can be used to iterate XAtttributes on an element, taking scheduled changes into account.
\ingroup ElemHandles
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  EditElementHandle : public ElementHandle
{
//__PUBLISH_SECTION_END__
    friend struct ITxn;
    friend struct ElementLinkageIterator;
//__PUBLISH_SECTION_START__

private:
    EditElementHandleR operator= (EditElementHandleCR from); // illegal!!
    explicit EditElementHandle (ElementHandleCR from);        // illegal!!
    explicit EditElementHandle (EditElementHandleCR from);    // illegal!!

public:
    EditElementHandle() {} //!< construct a blank, invalid, EditElementHandle

    //! Construct an EditElementHandle from an MSElementDescr.
    //! @see ElementHandle::ElementHandle
    EditElementHandle (MSElementDescrP descr, bool isUnmodified=false) : ElementHandle (descr, isUnmodified) {}

    //! Construct an EditElementHandle from an ElementRef.
    //! @see ElementHandle::ElementHandle
    EditElementHandle (ElementRefP elRef) : ElementHandle (elRef) {}

    //! Construct an EditElementHandle from an DgnElement and a DgnModel.
    //! @see ElementHandle::ElementHandle
    EditElementHandle (DgnElementCR el, DgnModelR model) : ElementHandle (el, model) {}

    //! Construct an EditElementHandle from an ElementId and a DgnModel.
    //! @param[in]  id  The ElementId of the element for this ElementHandle.
    //! @param[in]  model The DgnModel used to access the element.
    //! @remarks NOTE: test IsValid to determine whether the element was found!
    EditElementHandle (ElementId id, DgnModelR model) : ElementHandle (id, model) {}

    //! Construct an EditElementHandle from an ElementId and a DgnProject.
    //! @param[in]  id  The ElementId of the element for this ElementHandle.
    //! @param[in]  project The DgnProject used to access the element.
    //! @remarks NOTE: test IsValid to determine whether the element was found!
    EditElementHandle (ElementId id, DgnProjectR project) : ElementHandle (id, project) {}

    //! Construct an EditElementHandle from an ElementHandle.
    DGNPLATFORM_EXPORT EditElementHandle (ElementHandleCR from, bool duplicateDescr);

    //! Duplicate the element descriptor and its scheduled XAttribute changes.
    DGNPLATFORM_EXPORT void Duplicate (ElementHandleCR);

    //! Change the DgnModel for this EditElementHandle.
    //! @param model IN new DgnModel
    DGNPLATFORM_EXPORT void SetDgnModel (DgnModelR model);

    //! Invalidate the ElementRef for this EditElementHandle. This EditElementHandle will no longer be treated as an exact image of the element in the model.
    void SetNonPersistent () {ClearElementRef();}

    //! Mark this EditElementHandle as invalid. If there is an MSElementDescr associated with this EditElementHandle, it is freed.
    void Invalidate () {ElementHandle::Invalidate ();}

    void SetIElementState (IElementStateP state) {ClearElemState (); m_state = state; if (m_state) m_state->AddRef ();}
    IElementStateP GetIElementState () {return m_state;}

    //! Change the Handler for this Element. Generally, this method is used when elements are first created. Otherwise, it can be very
    //! tricky to change the type of an element.
    DGNPLATFORM_EXPORT void ChangeElementHandler(HandlerR);

    //! @name Element Data
    //@{

    //! Get a writable MSElementDescr from this EditElementHandle. If this EditElementHandle does not already have an MSElementDescr, allocate one using
    //! the ElementRefP/DgnModel.
    //! @remarks Use #PeekElementDescrCP to see whether this EditElementHandle already has an MSElementDescr.
    //! @remarks This method returns a pointer to the MSElementDescr in the EditElementHandle. It is still owned by the EditElementHandle and \b must \b not
    //!          be freed by the caller.
    MSElementDescrP GetElementDescrP () {return (MSElementDescrP) ElementHandle::GetElementDescrCP();}

    //! Get a pointer to a writeable DgnElement from this EditElementHandle.
    //! @remarks The element will be the "el" member of the MSElementDescr for this EditElementHandle. Therefore, it is \em not valid to modify the
    //!          size of the element through this pointer. To do that, use #ReplaceElement.
    //! @remarks If this EditElementHandle does not already have an MSElementDescr, one will be allocated by this call.
    DgnElementP GetElementP () {GetElementDescrP(); return m_dscr.IsValid() ? &m_dscr->ElementR() : NULL;}

    //! Replace the element associated with this EditElementHandle with a new element.
    //! @param[in]  el The new element. The element is copied from this buffer and allocated in a new MSElementDescr.
    DGNPLATFORM_EXPORT StatusInt ReplaceElement (DgnElementCP el);

    //! Replace the MSElementDescr associated with the EditElementHandle with a new MSElementDescr.
    //! @param[in]  elDscr      The new MSElementDescr to be associated with the EditElementHandle. After this call, \a elDscr becomes owned by this EditElementHandle.
    //! @return SUCCESS if the MSElementDescr was successfully replaced by \a elDscr, ERROR otherwise.
    //! @remarks This method will fail if the MSElementDescr is not "owned" by this EditElementHandle.
    //! @remarks If the DgnModel in \a elDscr is NULL, the value from the existing MSElementDescr is copied into \a elDscr.
    DGNPLATFORM_EXPORT StatusInt ReplaceElementDescr (MSElementDescrP elDscr);

    //! Extract and take ownership of the MSElementDescr associated with this EditElementHandle.
    //! @return The MSElementDescrP from the EditElementHandle. The MSElementDescr is no longer associated with the EditElementHandle, and the
    //!         caller is responsible for freeing it.
    //! @remarks This method will fail if there is no "owned" MSElementDescr already associated with this EditElementHandle.
    DGNPLATFORM_EXPORT MSElementDescrPtr ExtractElementDescr();

    //! Assign a new MSElementDescr to this EditElementHandle. The existing MSElementDescr (if present) is freed.
    //! @param[in]  elDscr       MSElementDescr to be referenced by this ElementHandle.
    //! @param[in]  isUnmodified if true and elDscr->GetElementRef(), \a elDscr is considered to be as an exact image of the element in the model.
    //!                          When this is true, users of this EditElementHandle may choose to use the ElementRef instead of \a elDscr. 
    //!                          For example, the display code may choose to use a previously-cached presentation to draw the element. If you're not sure, pass false.
    void SetElementDescr (MSElementDescrP elDscr, bool isUnmodified=false) {AssignElemDescr (elDscr, isUnmodified);}

    //! Change the ElementRefP for this EditElementHandle. If there is currently an MSElementDescr associated with
    //! this EditElementHandle, it is freed.
    //! @param[in]  elRef       New ElementRefP.
    //! @see SetDgnModel
    void SetElementRef (ElementRefP elRef) {AssignElementRef (elRef);}

    //@}

    //! @name XAttribute Changes
    //@{

    /**  Schedule the specified XAttribute to be written to the MSElementDescr.
         @remarks This function makes a copy of \em data
         @param[in] h           XAttributeHandler ID
         @param[in] xAttrId     XAttribute ID
         @param[in] dataSize    Number of bytes of data in XAttribute data
         @param[in] data        XAttribute data
         @return non-zero error status if the change cannot be scheduled. Normally, this would only happen in an out-of-memory situation.
         @remarks EditElementHandle does not distinguish between adding a new XAttribute and replacing
         the value of an existing XAttribute. In both cases, you call ScheduleWriteXAttribute. If you
         have an ElementRefP, you can use the XAttributeHandle class to test if an XAttribute exists or not.
         \code
         // Example of how to add an element with XAttributes to a cache
         EditElementHandle eh;
         eh.SetElementDescr ( ... );
         ...
         eh.ScheduleWriteXAttribute (myXaHandlerId, xaIndex, nbytes, xaBytes);
         ...
         eh.AddToModel ();   // adds the element and the XAttribute to the active model cache
         \endcode
         @remarks If the XAttribute was previously scheduled for write or delete, this request to write the specified
         data supercedes the previous request and schedules the specified data for write instead.
    */
    DGNPLATFORM_EXPORT StatusInt ScheduleWriteXAttribute (XAttributeHandlerIdCR h, UInt32 xAttrId, size_t dataSize, void const* data);

    //!  Cancel out the effects of a prior call to ScheduleWriteXAttribute
    //!  @param[in] h           XAttributeHandler ID
    //!  @param[in] xAttrId     XAttribute ID
    //!  @return non-zero       error status if the XAttribute was not scheduled for write
    DGNPLATFORM_EXPORT StatusInt CancelWriteXAttribute (XAttributeHandlerIdCR h, UInt32 xAttrId);

    //!  Schedule removal of the specified XAttribute from the MSElementDescr.
    //!  @param[in] h           XAttributeHandler ID
    //!  @param[in] xAttrId     XAttribute ID
    //!  @return non-zero error status if the change cannot be scheduled. Normally, this would only happen in an out-of-memory situation.
    //!  @remarks If the XAttribute was previously scheduled for write, this request to delete the XAttribute
    //!  cancels the previous write request and schedules the XAttribute to be deleted instead.
    //!  @remarks If the XAttribute was previously scheduled for delete, this request effectively does nothing;
    //!  the XAttribute will remain scheduled for deletion.
    DGNPLATFORM_EXPORT StatusInt ScheduleDeleteXAttribute (XAttributeHandlerIdCR h, UInt32 xAttrId);

    //!  Cancel out the effects of a prior call to ScheduleDeleteXAttribute
    //!  @param[in] h           XAttributeHandler ID
    //!  @param[in] xAttrId     XAttribute ID
    //!  @return non-zero error status if the XAttribute was not scheduled for deletion
    DGNPLATFORM_EXPORT StatusInt CancelDeleteXAttribute (XAttributeHandlerIdCR h, UInt32 xAttrId);
    //@}

    //! @name Read from model
    //@{
    DGNPLATFORM_EXPORT StatusInt FindById (ElementId elemID, DgnModelP dgnCache, bool allowDeleted = false);

    //@}

    //! @name Write changes to a Model
    //@{

    //! Add this EditElementHandle to the associated model. The addition is via the current transaction (see ITxn::AddElement) and is always undoable.
    //! @return SUCCESS if the element was added to its model. This method will fail if there is no MSElementDescr for this
    //!     EditElementHandle, or if the MSElementDescr is not owned by this EditElementHandle, or if this EditElementHandle is not associated with a model.
    //! @remarks This method is just a wrapper for: return ITxnManager::GetCurrentTxn().AddElement (*this);
    //! @remarks After the element is successfully added to the model, the ElementRefP of this EditElementHandle is set and the MSElementDescr is freed.
    DGNPLATFORM_EXPORT StatusInt AddToModel ();

    //! Delete this EditElementHandle from its model. The delete is via the current transaction (see ITxn::DeleteElement) and is always undoable.
    //! @return SUCCESS if the element was deleted from the model.
    //! @remarks After the element is successfully deleted from the model, this EditElementHandle becomes invalid (see #IsValid).
    DGNPLATFORM_EXPORT StatusInt DeleteFromModel ();

    //! Replace the element referred to by the supplied ElementRefP with the modified MSElementDescr held by this EditElementHandle.
    //! The replacement is via the current transaction (see ITxn::ReplaceElement) and is always undoable.
    //! @param[in] oldRef Element to replace.
    //! @return SUCCESS if the element was replaced in the model. This method will fail if there is no MSElementDescr for this EditElementHandle.
    //! @remarks After the element is successfully replaced in the model, the ElementRefP of this EditElementHandle is updated with the (potentially new)
    //! ElementRefP of replaced element and the MSElementDescr is freed.
    DGNPLATFORM_EXPORT StatusInt ReplaceInModel (ElementRefP oldRef);
    //@}

    //! @name Element Linkages
    //@{

    //! Get an iterator over the element's user data linkages
    //! @param rl   Linkage ID filter value identifies the linkages of interest. Defaults to all linkages.
    //! @private
    DGNPLATFORM_EXPORT ElementLinkageIterator BeginElementLinkages (UInt16 rl = 0);

    //! Get an iterator that marks the end of the element's user data linkages
    //! @private
    DGNPLATFORM_EXPORT ElementLinkageIterator EndElementLinkages ();

    //! Replace an element data linkage on this element
    //! @param it                   The linkage that is to be replaced
    //! @param newLinkageHeader     new linkage header (copied).
    //! @param newLinkageData       new linkage data (copied).
    //! @return non-zero error status if the maximum element size would be exceeded
    //! @remarks You must call LinkageUtil::SetWords on the new linkage header in order to record the total size of the new linkage
    //! (header and data) before passing it into this function.
    //! @remarks \a newLinkageData is copied to the location immediately following the new linkage header on the element. No alignment padding is inserted between the two.
    //! @private
    DGNPLATFORM_EXPORT StatusInt ReplaceElementLinkage (ElementLinkageIteratorR it, LinkageHeader const& newLinkageHeader, void const* newLinkageData);

    //! Remove an element data linkage from this element.
    //! When this function is called during the iterator, the resulting iterator will point to the next element, and should not be incremented.  For example:
    /**
    \code
       for (ElementLinkageIterator li = element.BeginElementLinkages(); li != element.EndElementLinkages(); )  // Incremented below
           {
           if (NeedsToBeDeleted)
              element.RemoveElementLinkage (li);
           else
              ++li;
           }
    \endcode
    */
    //! @param it  The linkage that is to be removed
    DGNPLATFORM_EXPORT StatusInt RemoveElementLinkage (ElementLinkageIteratorR it);

    //! Add a new element data linkage to this element.
    //! @param[out] newLinkageIt Points to new linkage on the element
    //! @param newLinkageHeader  new linkage header (copied).
    //! @param newLinkageData new linkage data (copied).
    //! @param wh   where the new linkage should be inserted into the element's linkage data area. Pass EndElementLinkages() to append.
    //!             Pass BeginElementLinkages() to insert before all existing linkages.
    //!             Otherwise, pass an iterator that identifies an existing linkage, and this function will insert the new linkage before it.
    //! @param rl   Linkage ID filter value to assign to \a newLinkageIt. Defaults to all linkages.
    //! @remarks You must call LinkageUtil::SetWords on the new linkage header in order to record the total size of the new linkage
    //! (header and data) before passing it into this function.
    //! @remarks \a newLinkageData is copied to the location immediately following the new linkage header on the element. No alignment padding is inserted between the two.
    //! @private
    DGNPLATFORM_EXPORT StatusInt InsertElementLinkage (ElementLinkageIterator* newLinkageIt, LinkageHeaderCR newLinkageHeader, void const* newLinkageData, ElementLinkageIterator& wh, UInt16 rl = 0);

    //! Appends a new element data linkage to the end of this element.
    //! @param[out] newLinkageIt        Points to new linkage on the element
    //! @param      newLinkageHeader    new linkage header (copied).
    //! @param      newLinkageData      new linkage data (copied).
    //! @remarks You must call LinkageUtil::SetWords on the new linkage header in order to record the total size of the new linkage (header and data) before passing it into this function.
    //! @remarks \a newLinkageData is copied to the location immediately following the new linkage header on the element. No alignment padding is inserted between the two.
    //! @private
    DGNPLATFORM_EXPORT StatusInt AppendElementLinkage (ElementLinkageIterator* newLinkageIt, LinkageHeaderCR newLinkageHeader, void const* newLinkageData);
    //@}
};

/*=================================================================================**//**
* This interface provides a transparent way to provide access to a collection of ElemHandles. It is typically
* implemented by an iterator class returned by the set, but can be implemented by any application to supply other types
* of ElementHandle collections.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct     IElementSet
{
    //! Reset the iterator to the begining of the set and return the first member.
    //! @param[out] elHandle    The ElementHandle for the first member.
    //! @return true if there was at least one member of the set and \a elHandle was set. If false, \a elHandle was not modified.
    virtual bool GetFirst (ElementHandleR elHandle) = 0;

    //! Get the next ElementHandle in the set.
    //! @param[out] elHandle    The ElementHandle for the first member.
    //! @return true if there were more members in the set and \a elHandle was set. If false, \a elHandle was not modified.
    virtual bool GetNext (ElementHandleR elHandle) = 0;
};

// =======================================================================================
//! Iterate Linkages on an element
//! @private
// @bsiclass                                                      Sam.Wilson      11/2007
// =======================================================================================
struct          ConstElementLinkageIterator
{
#if !defined (DOCUMENTATION_GENERATOR)
protected:
    friend struct      ElementHandle;
    LinkageHeader const* m_thisLinkage;
    LinkageHeader const* m_end;
    UInt16               m_requestedLinkage;
#endif

//__PUBLISH_SECTION_END__

    void SetEnd (DgnElementCP);
    bool ToNextFromValid ();
    bool IsRequestedLinkage () const;

    enum IntialPos {POS_ATBEGIN=0, POS_ATEND=1};

    ConstElementLinkageIterator (ElementHandleCR eh, IntialPos, UInt16 rl);
    ConstElementLinkageIterator (ElementHandleCR eh, byte*, UInt16 rl);

//__PUBLISH_SECTION_START__
public:
    ConstElementLinkageIterator () {m_thisLinkage=m_end=0; m_requestedLinkage=0;}

    DGNPLATFORM_EXPORT bool IsValid () const;
    DGNPLATFORM_EXPORT bool ToNext ();

    ConstElementLinkageIterator& operator++ () {ToNext(); return *this;}
    bool operator== (ConstElementLinkageIterator const& rhs) const {return m_thisLinkage == rhs.m_thisLinkage;}
    bool operator!= (ConstElementLinkageIterator const& rhs) const {return !(m_thisLinkage == rhs.m_thisLinkage);}

    LinkageHeader const& operator* () const {return *m_thisLinkage;}
    LinkageHeader const* operator-> () const {return m_thisLinkage;}

    //! Returns a read-only pointer to the data immediately following the linkage header. (void*)(linkageheader + 1)
    //! @remarks The returned data pointer will be 4-byte aligned. If the data contains doubles, this will be sub-optimal.
    void const* GetData () const {return m_thisLinkage + 1;}
    LinkageHeader const* GetLinkage () const {return m_thisLinkage;}

}; // ConstElementLinkageIterator

// =======================================================================================
//! Iterate Linkages on an element
//! It is not safe to delete linkages in this iterator.
//! @private
// @bsiclass                                                      Sam.Wilson      11/2007
// =======================================================================================
struct          ElementLinkageIterator : ConstElementLinkageIterator
{
//__PUBLISH_SECTION_END__
protected:
    friend struct      EditElementHandle;
    ElementLinkageIterator (EditElementHandleR eh, byte*, UInt16 rl);
    ElementLinkageIterator (EditElementHandleR eh, IntialPos, UInt16 rl);
    StatusInt ReplaceLinkage (EditElementHandleR, LinkageHeader const& l, void const* d);
    StatusInt RemoveLinkage (EditElementHandleR);
    StatusInt CheckElementContainsThisLinkage (EditElementHandleR);

//__PUBLISH_SECTION_START__
public:
    ElementLinkageIterator () : ConstElementLinkageIterator () {;}

    ElementLinkageIterator& operator++() {ToNext(); return *this;}
    bool operator==(ElementLinkageIterator const& rhs) const {return m_thisLinkage == rhs.m_thisLinkage;}
    bool operator!=(ElementLinkageIterator const& rhs) const {return !(m_thisLinkage == rhs.m_thisLinkage);}

    LinkageHeader& operator*() {return const_cast<LinkageHeader&>(*m_thisLinkage);}
    LinkageHeader* operator->() {return const_cast<LinkageHeader*>(m_thisLinkage);}

    //! Returns a read-write pointer to the data immediately following the linkage header. (void*)(linkageheader + 1)
    //! @remarks Be careful not to overflow the buffer when writing through this pointer!
    //! @remarks The returned data pointer will be 4-byte aligned. If the data contains doubles, this will be sub-optimal.
    void* GetDataRW () {return const_cast<void*>(GetData());}

};

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
