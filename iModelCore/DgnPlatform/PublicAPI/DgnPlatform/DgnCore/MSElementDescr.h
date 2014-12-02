/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/MSElementDescr.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include    "DgnElements.h"

DGNPLATFORM_TYPEDEFS (XAttributeChangeSet)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! A list of MSElementDescr's.
// @bsiclass                                                    Keith.Bentley   02/14
//=======================================================================================
struct MSElementDescrVec : bvector<MSElementDescrPtr>
{
const_iterator Find(MSElementDescrCR val) const
    {
    for (auto it=begin(); it!=end(); ++it)
        {
        if (it->get() == &val)
            return it;
        }
    return end(); // not found
    }
};

//=======================================================================================
//! An MSElementDescr holds a copy of a MicroStation element. MSElementDescr's are necessary and
//! intended for creating new elements and making changes to existing elements.
//! MSElementDescr's can also hold an XAttributeChangeSet.
//! @note For readonly access to existing elements, see the ElementHandle class.
//=======================================================================================
struct MSElementDescr : RefCountedBase, NonCopyableClass
{
private:
    size_t               m_allocBytes;
    mutable XAttributeChangeSet* m_attributes;
    DgnModelP            m_dgnModel;
    ElementRefP          m_elementRef;
    DgnItemId            m_itemId;
    HandlerP             m_handler;
    DgnElementP          m_el;

    void Init(DgnElementCR element, ElementRefP elRef, DgnModelR model);
    ~MSElementDescr();

public:
    //! Ctor for MSElementDescr from an DgnElement.
    //! @param[in]  element     The existing element.
    //! @param[in]  model       The value for the DgnModel member of the new MSElementDescr.
    DGNPLATFORM_EXPORT MSElementDescr(DgnElementCR element, DgnModelR model);

    //! Ctor for MSElementDescr from an ElementRef. Element data is loaded from ElementRef.
    //! @param[in]  elemRef The existing ElementRef
    DGNPLATFORM_EXPORT MSElementDescr(ElementRefR elemRef);

    //! Allocate a new MSElementDescr holding a copy of the supplied DgnElement.
    //! @param[in]  element     The existing element.
    //! @param[in]  model       The value for the DgnModel member of the new MSElementDescr.
    //! @return An MSElementDescrPtr to the new MSElementDescr.
    static MSElementDescrPtr Allocate(DgnElementCR element, DgnModelR model) {return new MSElementDescr(element, model);}

    //! Allocates and returns a copy of this MSElementDescr.
    //! @param[in] copyScheduledXaChanges pass true to copy any scheduled XAttribute change to the new copy.
    //! @param[in] loadPersistentXasAsChanges internal only. Pass false.
    //! @return A duplicate copy of the original MSElementDescr
    DGNPLATFORM_EXPORT MSElementDescrPtr Duplicate(bool copyScheduledXaChanges=true, bool loadPersistentXasAsChanges=false) const;

    //! Set the ElementID of this MSElementDescr to zero.
    DGNPLATFORM_EXPORT void ClearElementId();

    //! Change the DgnModel of this MSElementDescr, and all of its components, to the supplied model.
    //! @param[in] model The model to store in this MSElementDescr.
    DGNPLATFORM_EXPORT void SetDgnModel(DgnModelR model);

    //! Get the DgnModel of this MSElementDescr
    DgnModelR GetDgnModel() const {return *m_dgnModel;}

    //! Get the number of bytes currently allocated for element data by this MSElementDescr
    size_t GetAllocSize() const {return m_allocBytes;}

    //! Reserve memory for element data in this MSElementDescr.
    //! @param[in] newSize This MSElementDescr will hold at least this number of bytes for element data.
    //! @param[in] preserveData If true, and if the current size is smaller than requested size, then the existing element data
    //! is preserved. Otherwise, the content of the newly reserved memory is undefined.
    DGNPLATFORM_EXPORT void ReserveMemory(size_t newSize, bool preserveData=true);

    //! Get the ElementRef for this MSElementDescr
    ElementRefP GetElementRef() const {return m_elementRef;}

    //! Set the ElementRef for this MSElementDescr
    void SetElementRef(ElementRefP elRef) {m_elementRef = elRef;}

    //! Get a const reference to this MSElementDescr's element data.
    DgnElementCR Element() const {return *m_el;}

    //! Get a writeable reference to this MSElementDescr's element data.
    DgnElementR ElementR() {return *m_el;}

    //! Get the DgnItemId of this MSElementDescr
    DgnItemId GetItemId() const {return m_itemId;}

    //! Set the DgnItemId of this MSElementDescr
    void SetItemId(DgnItemId val) {m_itemId = val;}

    //! Get the element Handler for this MSElementDescr
    HandlerP GetElementHandler() const {return m_handler;}

    //! Insert a linkage into an element
    //! @param[in] linkageOffset where to insert the new linkage in the element
    //! @param[in] newLinkageHeader new linkage header
    //! @param[in] newLinkageData new linkage data. Must already be padded to 2*mdlLinkage_getSize(&newLinkageHeader) bytes!
    DGNPLATFORM_EXPORT BentleyStatus InsertLinkage(size_t linkageOffset, LinkageHeaderCR newLinkageHeader, void const* newLinkageData);

    //! Append a linkage to an element
    //! @param[in] newLinkageHeader new linkage header
    //! @param[in] newLinkageData  new linkage data. NB must already be padded to 2*mdlLinkage_getSize(&newLinkageHeader) bytes!
    //! @remarks May reallocate the element
    DGNPLATFORM_EXPORT BentleyStatus AppendLinkage(LinkageHeaderCR newLinkageHeader, void const* newLinkageData);

    //! Replace an existing linkage in an element. The size of the replacement linkage can be the same or different from the size of the existing linkage. If different, linkages that follow are moved.
    //! @param[in] existingLinkage start of the existing linkage in the element, updated to point to start of replacement linkage (in case eh is reallocated)
    //! @param[in] newLinkageHeader new linkage header
    //! @param[in] newLinkageData new linkage data. Must already be padded to 2*mdlLinkage_getSize(&newLinkageHeader) bytes!
    //! @note May reallocate the element
    DGNPLATFORM_EXPORT BentleyStatus ReplaceLinkage(LinkageHeaderP* existingLinkage, LinkageHeaderCR newLinkageHeader, void const* newLinkageData);

    //! Dump information about this MSElementDescr to console
    DGNPLATFORM_EXPORT void DebugDump(CharCP indent="") const;

    //! Get the number of MSElementDescr's that are currently extant (have been constructed but not destroyed.) Can be used to detect leaks.
    DGNPLATFORM_EXPORT static size_t DebugGetExtantCount();

    //! Get the total number of MSElementDescr's that have ever been constructed since the beginning of the session.
    DGNPLATFORM_EXPORT static size_t DebugGetTotalCount();

//__PUBLISH_SECTION_END__
    //! Replace the element held by this MSElementDescr with a new element.
    //! @param[in] element the new element for this MSElementDescr.
    //! @note If the new element requires more memory than this MSElementDescr currently holds, memory will
    //! be reallocated and subsequent calls to #Element return a different value
    DGNPLATFORM_EXPORT void ReplaceElement(DgnElementCR element);

    DGNPLATFORM_EXPORT BentleyStatus CopyXAttributesTo(MSElementDescrR sink) const;
    DGNPLATFORM_EXPORT XAttributeChangeSetP GetXAttributeChangeSet() const;
    XAttributeChangeSetP QueryXAttributeChangeSet() const {return m_attributes;}
    void AppendXAttributeChangeSet(XAttributeChangeSet const&);
    void DonateXAttributeChangeSetTo(MSElementDescr*);

    //! Set the elementRef of this MSElementDescr, and all of its components, to zero.
    DGNPLATFORM_EXPORT void ClearElementRef();

    void SetElementHandler(HandlerP handler) {m_handler = handler;}
    void GetHeaderFieldsFrom(ElementRefCR);

//__PUBLISH_SECTION_START__
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
