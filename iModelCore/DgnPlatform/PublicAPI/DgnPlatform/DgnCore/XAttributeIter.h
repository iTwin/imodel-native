/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/XAttributeIter.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma  once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <Bentley/Bentley.h>
#include <DgnPlatform/DgnCore/DgnCore.h>
#include <DgnPlatform/DgnCore/DgnFile.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
 @addtogroup XAttributes

An XAttribute is data that is stored \b on an element, not \b in an element. An XAttribute is \em not part of
the element's data (that is, visible through an DgnElement). By way of contrast, a user data linkage is part of an element's data.

<h3>XAttributes Vs. Linkages</h3>
Both XAttributes and Linkages provide a way for applications to persistently store additional variable length data with
an element. The primary distinction between XAttributes and Linkages is that XAttributes can be accessed (read/written) \em independently
of one another and independent of the element data. Linkages, on the other hand, can only be accessed by reading and writing the entire element
(that is, the element's data plus all Linkages) together. Also, there is a maximum size for an element's data plus all it's
linkgages (MAX_ELEMENT_SIZE.) There is no absolute maximum size for an individual XAttribute, nor is there a limit on the number of
XAttributes per element.

Generally speaking, XAttributes and Linkages are substitutable concepts. That is, any place where you can use a Linkage, you can use an
XAttribute. However, XAttributes are generally preferable for the reasons stated above (independent access and lack of size limitations),
with the caveat that XAttributes are not supported on older versions of MicroStation.

<h3>Identifying XAttributes</h3>
XAttributes are stored on an element and are always referenced within the context of that element. Within a single element, XAttributes are
identified by two parts: an XAttributeHandlerId and an XAttributeId. The XAttributeHandlerId specifies the "type" of the XAttribute, and
designates the XAttributeHandler that, if present, will handle events for the XAttribute. A single element may contain XAttributes with many differnet
XAttributeHandlerIds. However, all of the XAttributes on an element that have the same XAttributeHandlerId must have a unique XAttributeId.
It should be obvious though that multiple XAttributes on the same element may have the same XAttributeId if they have different
XAttributeHandlerIds.

<h3>Accessing XAttributes</h3>
The XAttributes of an element are accessed through an XAttributeHandle. The methods on XAttributeHandle provide access to the
information in an XAttribute. An XAttributeHandle can be constructed from an ElementRefP, XAttributeHandlerId,
and XAttributeId, or by using an XAttributeCollection::const_iterator (which is a subclass of XAttributeHandle) to step through the individual
XAttributes of an element. \b NOTE: Any additions or deletions of XAttributes to an element will invalidate all existing XAttributeHandles for
that element.

See ElementHandle::XAttributeIter for an XAttribute iterator that takes scheduled changes into account.
<h3>Modifying XAttributes</h3>
Generally, XAttributes are added, modified, or removed from an existing element via its ElementRefP, using the XAttribute methods in
the Transaction Manager, BentleyApi::DgnPlatform::ITxnManager. The Transaction Manager guarantees that all element changes are properly journalled so that if the changes are reversed via an
Undo, the previous state of all changed elements, including XAttribute data, are reinstated together.
See BentleyApi::DgnPlatform::ITxn::AddXAttribute, etc.

However, sometimes it is necessary to store changes to XAttribute data in memory alongside the changes to the element data. For example,
when constructing new elements, there is no ElementRef yet to hold the XAttribute. For this purpose, you can "schedule" XAttribute changes
via methods on EditElementHandle. These changes are stored with the element when and if the modifications in the EditElementHandle are committed.
@see EditElementHandle::ScheduleWriteXAttribute and EditElementHandle::ScheduleDeleteXAttribute.

 @beginGroup
 @bsiclass
+===============+===============+===============+===============+===============+======*/

//=======================================================================================
//! A "handle" that can be used to access a single XAttribute of a persistent element. \b Note: Additions or deletions of XAttributes
//! on an element will invalidate all XAttributeHandle's for that element.
//! @bsiclass
//=======================================================================================
struct  XAttributeHandle
{
    enum {INVALID_XATTR_ID = 0xffffffff, MATCH_ANY_ID = INVALID_XATTR_ID};

protected:
    mutable bool        m_cached;
    mutable void*       m_data;
    Int64               m_rowid;
    ElementId           m_elementId;
    XAttributeHandlerId m_handlerId;
    DgnProjectP         m_project;
    ElementRefP         m_elRef;
    UInt32              m_id;
    UInt32              m_size;
    DgnModels::XAttributeFlags m_flags;

    void Copy(XAttributeHandleCR);
    void DoSelect();
    DGNPLATFORM_EXPORT void FreeData() const;
    BeSQLite::DbResult ReadData (void* data, UInt32 size) const;

public:
    void Invalidate() {FreeData(); m_rowid=-1;}
    void ClearElemRefCache() const;
    DGNPLATFORM_EXPORT XAttributeHandle (XAttributeHandleCR);
    DGNPLATFORM_EXPORT XAttributeHandleR operator= (XAttributeHandleCR);
     ~XAttributeHandle() {FreeData();}

    //! Construct a blank, invalid XAttributeHandle.
    DGNPLATFORM_EXPORT XAttributeHandle ();

//__PUBLISH_SECTION_END__
    XAttributeHandle (ElementRefP, XAttributeHandlerId handler, UInt32 id, Int64 rowid, UInt32 size, DgnModels::XAttributeFlags flags);
    DGNPLATFORM_EXPORT BeSQLite::DbResult ModifyData (void const* data, UInt32 offset, UInt32 size, DgnModels::XAttributeFlags flags);
    DGNPLATFORM_EXPORT BeSQLite::DbResult ReplaceData (void const* data, UInt32 size, DgnModels::XAttributeFlags flags);
    DGNPLATFORM_EXPORT BeSQLite::DbResult DeleteFromFile();
    DGNPLATFORM_EXPORT XAttributeHandle (ElementId elId, DgnProjectR, XAttributeHandlerId handlerId, UInt32 id);

//__PUBLISH_SECTION_START__
    //! Construct an XAttributeHandle that will reference a particular XAttribute, or will be invalid. You must call #IsValid to determine
    //! whether the XAttribute was found or not.
    //! @param elRef IN the ElementRefP to search for the specfied XAttribute
    //! @param handler IN the XAttributeHandlerId of the XAttribute.
    //! @param id IN the XAttributeId of the XAttribute.
    DGNPLATFORM_EXPORT XAttributeHandle (ElementRefP elRef, XAttributeHandlerId handler, UInt32 id);

    //! Determine whether this XAttributeHandle is valid or not.
    //! @return true if this XAttributeHandle is valid. If this method returns false, all other methods on this XAttributeHandle will fail or crash.
    bool IsValid() const {return -1 != m_rowid;}

    //! Get the ElementRefP from which this XAttributeHandle was constructed.
    ElementRefP GetElementRef() const {return m_elRef;}

    //! Get the XAttributeId of the XAttribute this XAttributeHandle references.
    UInt32 GetId() const {return m_id;}

    //! Get the XAttributeHandlerId of the XAttribute this XAttributeHandle references.
    XAttributeHandlerId GetHandlerId() const {return m_handlerId;}

    //! Get the number of bytes of data in the XAttribute this XAttributeHandle references. Only meaningful if IsValid() is true;
    UInt32 GetSize () const {return m_size;}
    Int64 GetRowId() const {return m_rowid;}

    //! Returns true if and only the XAttribute data is already in memory.  Returns false if the element does not have
    //! the requested XAttribute or it has to be loaded from a persistent store.
    DGNPLATFORM_EXPORT static bool IsXAttributeCached (ElementRefP elRef, XAttributeHandlerId handlerId, UInt32 id);

    //! Get a \c const pointer to the data for the XAttribute this XAttributeHandle references. This pointer is a direct pointer into
    //! the element cache. It will remain valid only while this XAttributeHandle is valid and referencing this XAttribute,
    //! but will \b not be valid thereafter. Therefore, do not save this pointer outside of methods that work directly with this XAttributeHandle.
    //! @remarks It is \em never safe, valid, or legal to cast away the const-ness of this pointer.
    DGNPLATFORM_EXPORT void const* PeekData() const;

    DGNPLATFORM_EXPORT XAttributeHandlerP GetHandler() const;

    //! Convenience method to determine whether an XAttribute exists on a given element.
    static bool HasAttribute (ElementRefP elRef, XAttributeHandlerId handler, UInt32 id)
        {
        XAttributeHandle test (elRef, handler, id);
        return  test.IsValid();
        }
};

//=======================================================================================
//! An collection that can be used to iterate over the XAttributes of an ElementRef.
//! \em Note: ANY additions or deletions of XAttributes on an element will invalidate all
//! XAttributeCollections and iterators for that element.
// @bsiclass                                                    Keith.Bentley   06/11
//=======================================================================================
struct XAttributeCollection
{
private:
    mutable BeSQLite::CachedStatementPtr m_sql;
    ElementRefP         m_elRef;
    XAttributeHandlerId m_searchId;
    BeSQLite::DbResult PrepareQuery() const;

public:
    XAttributeCollection() : m_elRef(NULL), m_searchId(0,0){}

    //! Construct an XAttributeCollection to iterate over XAttributes of an element.
    //! @param elRef IN the ElementRefP to search for the specfied XAttributes
    //! @param handlerId IN the XAttributeHandlerId to search for. If invalid, return all XAttriubutes
    XAttributeCollection (ElementRefP elRef, XAttributeHandlerId handlerId=XAttributeHandlerId(0,0)) : m_elRef(elRef), m_searchId(handlerId) {}

    //! The const_iterator value for an XAttributeCollection. This class is a subclass of XAttributeHandle, so all of its
    //! properties apply. In particular, you must call "IsValid" to determine whether an Entry currently holds valid information or not.
    struct Entry : XAttributeHandle, std::iterator<std::input_iterator_tag, Entry const>
        {
    private:
        friend struct XAttributeCollection;
        BeSQLiteStatementP m_sql;
        DGNPLATFORM_EXPORT void Step();

    public:
        Entry () : m_sql(NULL) {}
        Entry (ElementRefP elRef, XAttributeHandlerId handler, UInt32 id) : m_sql(NULL), XAttributeHandle(elRef,handler,id) {}
        Entry (BeSQLiteStatementP sql, ElementRefP elRef) : m_sql(sql)
                    {m_elRef=elRef; m_project = NULL == elRef ? NULL : elRef->GetDgnProject(); Step();}

        Entry& operator++() {Step(); return *this;}
        Entry const& operator* () const {return *this;}
        bool operator!=(Entry const& rhs) const {return (IsValid() != rhs.IsValid());}
        bool operator==(Entry const& rhs) const {return (IsValid() == rhs.IsValid());}
        };

    //! Get the HandlerId to which this collection applies.
    XAttributeHandlerId GetSearchHandlerId() const {return m_searchId;}

    //! Get the ElementRef to which this collection applies.
    ElementRefP GetElementRef() const {return m_elRef;}

    DGNPLATFORM_EXPORT void Reset (ElementRefP elRef, XAttributeHandlerId id);

    typedef Entry const_iterator;
    DGNPLATFORM_EXPORT Entry begin() const;
    Entry end() const {return Entry ();}
};

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
