/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ElementHandle.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include "../DgnPlatform.h"
#include "ElementHandler.h"
#include "DgnDb.h"
#include "DgnModel.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct ITxn;
//=======================================================================================
//! A list of DgnElementDescr's.
// @bsiclass                                                    Keith.Bentley   02/14
//=======================================================================================
struct DgnElementPtrVec : bvector<DgnElementPtr>
{
const_iterator Find(DgnElementCR val) const
    {
    for (auto it=begin(); it!=end(); ++it)
        {
        if (it->get() == &val)
            return it;
        }
    return end(); // not found
    }
};

/*=================================================================================**//**
* @addtogroup ElemHandles
An ElementHandle is an indirect reference to an DgnElement's current state. It permits access to the element transparently, whether the underlying
element data is held in an DgnElementDescr or merely referenced through an DgnElement. An ElementHandle is the "this" pointer for all
methods of ElementHandler.

An ElementHandle provides readonly access to the underlying element. The subclass EditElementHandle adds methods for modifying elements.

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
protected:
    DgnElementPtr           m_persistent;
    mutable DgnElementPtr   m_writeable; 
    IElementStateP          m_state;

    DGNPLATFORM_EXPORT void Clone(ElementHandle const& from);

    void ClearWriteable() {m_writeable = nullptr;}
    void ClearPersistent() {m_persistent = nullptr;}
    void ClearElemState() {RELEASE_AND_CLEAR(m_state);}
    void ClearAll() {ClearWriteable(); ClearElemState(); ClearPersistent();}
    void Invalidate() {ClearAll();}
    void AssignDgnElement(DgnElementCP element)
        {
        m_persistent = (DgnElementP) element; // keep this order so element doesn't become garbage if element==m_writeable;
        m_writeable = nullptr;
        }

    DGNPLATFORM_EXPORT void AssignElemDescr(DgnElementP, bool isPersistent);
    DGNPLATFORM_EXPORT void Init(DgnElementCP element);
    DGNPLATFORM_EXPORT void Init(DgnElementCP elDscr, bool isPersistent);
    DGNPLATFORM_EXPORT void Init(DgnElementId, DgnDbR);

public:
    ElementHandle() {Init(nullptr);} //!< construct a blank, invalid, ElementHandle
    ~ElementHandle() {ClearAll();}

    //! Construct an ElementHandle from an DgnElementP and a DgnModelP.
    //! @param[in]  element       The DgnElementP of the element for this ElementHandle.
    explicit ElementHandle(DgnElementCP element) {Init(element);}

    //! Construct an ElementHandle from an DgnElementId and a DgnModel.
    //! @param[in] id The DgnElementId of the element for this ElementHandle.
    //! @param[in] model The DgnModel used to access the element.
    //! @remarks NOTE: test IsValid to determine whether the element was found.
    ElementHandle(DgnElementId id, DgnModelR model) {Init(id, model.GetDgnDb());}

    //! Construct an ElementHandle from an DgnElementId and a DgnFile
    //! @param[in] id The DgnElementId of the element for this ElementHandle.
    //! @param[in] db the DgnDb of the element.
    //! @remarks NOTE: test IsValid to determine whether the element was found.
    ElementHandle(DgnElementId id, DgnDbR db) {Init(id, db);}

    //! Construct an ElementHandle from a DgnElement.
    //! @param[in]  elDscr       DgnElementDescr to be referenced by this ElementHandle.
    //! @param[in]  isUnmodified if true and elDscr->GetDgnElement(), \a elDscr is considered to be as an exact image of the element in the model.
    //!                          When this is true, users of this ElementHandle may choose to use the DgnElement instead of \a elDscr.
    //!                          For example, the display code may choose to use a previously-cached presentation to draw the element. If you're not sure, pass false.
    ElementHandle(DgnElementCP elDscr, bool isUnmodified) {Init(elDscr, isUnmodified);}

    DGNPLATFORM_EXPORT ElementHandle(ElementHandleCR from); //!< Copy an ElementHandle

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
    //! Get the Handler for this ElementHandle. Every element must have a element handler. This method returns a reference to the ElementHandler
    //! for this element. If this ElementHandle has an DgnElementP, its handler is returned. Otherwise, the DgnElementDescr is used.
    //! @note It is illegal to call this method on an Invalid ElementHandle.
    DGNPLATFORM_EXPORT ElementHandlerR GetElementHandler() const;
/** @} */

    //! Get the DgnElementP for this ElementHandle.
    //! @return the DgnElementP, or nullptr.
    DgnElementCP GetPersistentElement() const {return m_persistent.get();}
    DgnElementCP GetDgnElement() const {return m_writeable.IsValid() ? m_writeable.get() : m_persistent.get();}
    DgnElementP GetEditElement() const {return m_writeable.get();}

    //! Get the DgnModelP for this ElementHandle.
    //! @return the DgnModelP. First checks persitent element, if present, and then resorts to using writeable element.
    //! @note This method will only return nullptr for an invalid ElementHandle.
    DGNPLATFORM_EXPORT DgnModelP GetDgnModelP() const;

    //! Get the DgnDb for this ElementHandle.
    //! @return the DgnDb.
    //! @note This method will only return nullptr for an invalid ElementHandle.
    DgnDbP GetDgnDb() const {DgnModelP dgnCache=GetDgnModelP(); return (nullptr == dgnCache) ? nullptr : &dgnCache->GetDgnDb();}

    GeometricElementCP GetGeometricElement() const {return GetDgnElement()->_ToGeometricElement();}
    PhysicalElementCP GetPhysicalElement() const {return GetDgnElement()->_ToPhysicalElement();}
    DrawingElementCP GetDrawingElement() const {return GetDgnElement()->_ToDrawingElement();}

    //! Determine whether this ElementHandle is currently valid.
    //! @return true if this ElementHandle references a valid element. If not, all other methods on this ElementHandle will either fail or crash.
    bool IsValid() const {return m_persistent.IsValid() || m_writeable.IsValid();}

    //! Determine whether this ElementHandle references an unmodified element in the cache.
    //! @return true if ElementHandle is unmodified representation of element in the cache.
    bool IsPersistent() const {return m_persistent.IsValid();}

    //! Peek to see whether this ElementHandle currently has an DgnElementDescr.
    //! @return the current DgnElementDescr or nullptr.
    DgnElementCP PeekElementDescrCP() const {return m_writeable.get();}
};

/*=================================================================================**//**
* A writeable "handle" to an DgnElement.
*
* EditElementHandle is used for methods that modify a DgnElement.
*
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  EditElementHandle : public ElementHandle
{
    friend struct ITxn;

private:
    EditElementHandleR operator= (EditElementHandleCR from); // illegal!!
    explicit EditElementHandle(ElementHandleCR from);        // illegal!!
    explicit EditElementHandle(EditElementHandleCR from);    // illegal!!

public:
    EditElementHandle() {} //!< construct a blank, invalid, EditElementHandle

    //! Construct an EditElementHandle from an DgnElementDescr.
    //! @see ElementHandle::ElementHandle
    EditElementHandle(DgnElementP element, bool isUnmodified) : ElementHandle(element, isUnmodified) {}

    //! Construct an EditElementHandle from an DgnElement.
    //! @see ElementHandle::ElementHandle
    EditElementHandle(DgnElementCP element) : ElementHandle(element) {}

    //! Construct an EditElementHandle from an DgnElementId and a DgnModel.
    //! @param[in]  id  The DgnElementId of the element for this ElementHandle.
    //! @param[in]  model The DgnModel used to access the element.
    //! @remarks NOTE: test IsValid to determine whether the element was found!
    EditElementHandle(DgnElementId id, DgnModelR model) : ElementHandle(id, model) {}

    //! Construct an EditElementHandle from an DgnElementId and a DgnDb.
    //! @param[in]  id  The DgnElementId of the element for this ElementHandle.
    //! @param[in]  project The DgnDb used to access the element.
    //! @remarks NOTE: test IsValid to determine whether the element was found!
    EditElementHandle(DgnElementId id, DgnDbR project) : ElementHandle(id, project) {}

    //! Construct an EditElementHandle from an ElementHandle.
    DGNPLATFORM_EXPORT EditElementHandle(ElementHandleCR from, bool duplicateDescr);

    //! @deprected Use a factory method on DgnElement sub-class
    DGNPLATFORM_EXPORT BentleyStatus CreateNewElement(DgnModelR model, DgnClassId elementClassId, DgnCategoryId category, Utf8CP code=nullptr);

    //! Invalidate the DgnElement for this EditElementHandle. This EditElementHandle will no longer be treated as an exact image of the element in the model.
    void SetNonPersistent() {ClearPersistent();}

    void SetIElementState(IElementStateP state) {ClearElemState(); m_state = state; if (m_state) m_state->AddRef();}
    IElementStateP GetIElementState() {return m_state;}

    //! @name Element Data
    //@{

    //! Get a writable DgnElementDescr from this EditElementHandle. If this EditElementHandle does not already have an DgnElementDescr, allocate one using
    //! the DgnElementP/DgnModel.
    //! @remarks Use #PeekElementDescrCP to see whether this EditElementHandle already has an DgnElementDescr.
    //! @remarks This method returns a pointer to the DgnElementDescr in the EditElementHandle. It is still owned by the EditElementHandle and \b must \b not
    //!          be freed by the caller.
    DGNPLATFORM_EXPORT DgnElementP GetElementDescrP();

    //! Extract and take ownership of the DgnElementDescr associated with this EditElementHandle.
    //! @return The DgnElementDescrP from the EditElementHandle. The DgnElementDescr is no longer associated with the EditElementHandle, and the
    //!         caller is responsible for freeing it.
    //! @remarks This method will fail if there is no "owned" DgnElementDescr already associated with this EditElementHandle.
    DGNPLATFORM_EXPORT DgnElementPtr ExtractElementDescr();

    //! Assign a new DgnElementDescr to this EditElementHandle. The existing DgnElementDescr (if present) is freed.
    //! @param[in]  elDscr       DgnElementDescr to be referenced by this ElementHandle.
    //! @param[in]  isUnmodified if true and elDscr->GetDgnElement(), \a elDscr is considered to be as an exact image of the element in the model.
    //!                          When this is true, users of this EditElementHandle may choose to use the DgnElement instead of \a elDscr.
    //!                          For example, the display code may choose to use a previously-cached presentation to draw the element. If you're not sure, pass false.
    void SetElementDescr(DgnElementP elDscr, bool isUnmodified=false) {AssignElemDescr(elDscr, isUnmodified);}

    //! Change the DgnElementP for this EditElementHandle. If there is currently an DgnElementDescr associated with
    //! this EditElementHandle, it is freed.
    //! @param[in]  element       New DgnElementP.
    //! @see SetDgnModel
    void SetDgnElement(DgnElementCP element) {AssignDgnElement(element);}

    //@}

    //! @name Read from model
    //@{
    DGNPLATFORM_EXPORT StatusInt FindById(DgnElementId elemID, DgnModelP dgnCache, bool allowDeleted = false);

    //@}

    //! @name Write changes to a Model
    //@{

    //! Delete this EditElementHandle from its model. The delete is via the current transaction (see ITxn::DeleteElement) and is always undoable.
    //! @return SUCCESS if the element was deleted from the model.
    //! @remarks After the element is successfully deleted from the model, this EditElementHandle becomes invalid (see #IsValid).
    DGNPLATFORM_EXPORT StatusInt DeleteFromModel();

    //! Replace the element referred to by the supplied DgnElementP with the modified DgnElementDescr held by this EditElementHandle.
    //! The replacement is via the current transaction (see ITxn::ReplaceElement) and is always undoable.
    //! @param[in] oldRef Element to replace.
    //! @return SUCCESS if the element was replaced in the model. This method will fail if there is no DgnElementDescr for this EditElementHandle.
    //! @remarks After the element is successfully replaced in the model, the DgnElementP of this EditElementHandle is updated with the (potentially new)
    //! DgnElementP of replaced element and the DgnElementDescr is freed.
    DGNPLATFORM_EXPORT StatusInt ReplaceInModel();
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
    //! Reset the iterator to the beginning of the set and return the first member.
    //! @param[out] elHandle    The ElementHandle for the first member.
    //! @return true if there was at least one member of the set and \a elHandle was set. If false, \a elHandle was not modified.
    virtual bool GetFirst(ElementHandleR elHandle) = 0;

    //! Get the next ElementHandle in the set.
    //! @param[out] elHandle    The ElementHandle for the first member.
    //! @return true if there were more members in the set and \a elHandle was set. If false, \a elHandle was not modified.
    virtual bool GetNext(ElementHandleR elHandle) = 0;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

