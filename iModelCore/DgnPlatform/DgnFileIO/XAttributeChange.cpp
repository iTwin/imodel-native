/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnFileIO/XAttributeChange.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (_MSC_VER)
    #pragma warning (disable:4291)
#endif // defined (_MSC_VER)

/*=================================================================================**//**
* A ref-counted pointer to a hunk of dynamically allocated data.
* Uses the uStn-wide small block heap to allocate itself and its data.
* @bsiclass                                                     Sam.Wilson      02/2006
+===============+===============+===============+===============+===============+======*/
class           XAttributeChangeDataHolder
{
public:
    UInt32      m_refCount;
    UInt32      m_size;
    void*       m_data;

public:

    XAttributeChangeDataHolder (size_t sz, void const* data)
        :
        m_refCount (0),
        m_size (static_cast<UInt32>(sz))
        {
        if (m_size == 0)
            m_data = NULL;
        else
            {
            m_data = malloc (m_size);
            memcpy (m_data, data, m_size);
            }
        #if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
            ++s_count;
        #endif
        }

   ~XAttributeChangeDataHolder ()
        {
        if (NULL != m_data)
            free (m_data);
        #if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
            --s_count;
        #endif
        }

    UInt32      AddRef()
        {
        return ++m_refCount;
        }

    UInt32      Release()
        {
        if (1 < m_refCount--)
            return  m_refCount;

        delete this;
        return  0;
        }

    void*       operator new (size_t nBytes)
        {
        #if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
            ++s_ndcount;
        #endif
        return malloc (nBytes);
        }

    void        operator delete (void*p, size_t nBytes)
        {
        #if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
            --s_ndcount;
        #endif
        free (p);
        }

    static
    XAttributeChangeDataHolder* s_nilObject;

    static
    XAttributeChangeDataHolder* GetSingletonNilObject ()
        {
        if (NULL == s_nilObject)
            {
            s_nilObject = new XAttributeChangeDataHolder (0, NULL);
            s_nilObject->AddRef ();
            }
        return s_nilObject;
        }

    #if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
        static UInt32   s_count;
        static UInt32   s_ndcount;
    #endif // defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)

}; // XAttributeChangeDataHolder


XAttributeChangeDataHolder* XAttributeChangeDataHolder::s_nilObject;

#if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
UInt32 XAttributeChangeDataHolder::s_count;
UInt32 XAttributeChangeDataHolder::s_ndcount;
UInt32 XAttributeChange::s_count;
UInt32 XAttributeChangeSet::s_count;
UInt32 XAttributeChangeSet::s_ndcount;
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool operator<  (XAttributeChangeCR c1, XAttributeChangeCR c2)
    {
    return c1.AttributeUInt64Key() < c2.AttributeUInt64Key();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool operator== (XAttributeChangeCR c1, XAttributeChangeCR c2)
    {
    return c1.AttributeUInt64Key() == c2.AttributeUInt64Key();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
inline UInt64   XAttributeChange::AttributeUInt64Key () const
    {
    return ((UInt64)m_handlerId.GetId() << 32) | (UInt64)m_attrId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChange::XAttributeChange (XAttributeHandlerIdCR a, UInt32 k2, XAttributeChangeDataHolder& xad, ChangeType ct) :
    m_handlerId (a),
    m_attrId (k2),
    m_data (&xad),
    m_changeType (ct)
    {
    m_data->AddRef ();
    #if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
        ++s_count;
    #endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChange::XAttributeChange (XAttributeChangeCR source)
    :
    m_handlerId (source.m_handlerId),
    m_attrId (source.m_attrId),
    m_changeType (source.m_changeType),
    m_data (source.m_data)
    {
    if (m_data)
        {
        m_data->AddRef ();
    #if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
        ++s_count;
    #endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChange::~XAttributeChange ()
    {
    if (m_data)
        {
        m_data->Release ();
    #if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
        --s_count;
    #endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChange XAttributeChange::GetNilObject () // static
    {
    return XAttributeChange (XAttributeHandlerId(0,0), 0, *XAttributeChangeDataHolder::GetSingletonNilObject(), CHANGETYPE_Unknown);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void const*     XAttributeChange::PeekData () const
    {
    return m_data->m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2006
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          XAttributeChange::GetSize () const
    {
    return m_data->m_size;
    }

#if defined (TEST_XAttributeChange)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void            XAttributeChange::Dump () const
    {
    char* status[] = {"?", "Add", "Delete", "Replace"};
    printf ("%x %x %d %p %hs\n", m_handlerId.GetId(), m_attrId, GetSize(), PeekData(), status[m_changeType]);
    }

#endif //defined (TEST_XAttributeChange)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttributeChangeSet::CopySchedule (XAttributeChangeSetCR source)
    {
    m_purgeBefore = source.m_purgeBefore;
    for (T_ConstIterator it = source.Begin();  it != source.End();  ++it)
        {
        XAttributeChangeCR x = *it;
        Schedule (x.GetHandlerId(), x.GetId(), x.GetSize(), x.PeekData(), x.GetChangeType());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeSet::~XAttributeChangeSet ()
    {
    #if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
        --s_count;
    #endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XAttributeChangeSet::Cancel (T_Iterator it)
    {
    if (End() == it)
        return ERROR;

    m_changed.erase (it);
    m_purgeBefore = false;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2005
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeSet::XAttributeChangeSet ()
    {
    m_purgeBefore = false;
    #if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
        ++s_count;
    #endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void            XAttributeChangeSet::Clear ()
    {
    m_purgeBefore = false;
    m_changed.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XAttributeChangeSet::Schedule (XAttributeHandlerIdCR a, UInt32 k2, size_t sz, void const* d,
                                               XAttributeChange::ChangeType status)
    {
    return Schedule (a, k2, sz, d, status, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XAttributeChangeSet::Schedule
(
XAttributeHandlerIdCR           a,
UInt32                          k2,
size_t                          sz,
void const*                     d,
XAttributeChange::ChangeType    status,
bool                            replaceExisting
)
    {
    XAttributeChangeDataHolder* ccData = new XAttributeChangeDataHolder (sz, d);
    bpair<T_Iterator,bool> iv = m_changed.insert (XAttributeChange (a, k2, *ccData, status));
    if (iv.second)
        return SUCCESS;

    if (replaceExisting)
        {
        Cancel (iv.first);
        return Schedule (a, k2, sz, d, status, false);
        }

    //  This looks like a leak, but it's not. The temporary XAttributeChange object
    //  adds a reference to ccData. The m_changed set makes a copy of the temp and installs this second copy in the set's tree.
    //  The set's copy adds another reference to ccData. The temp then goes out of scope, releasing
    //  its reference to ccData. Thus, we are left with the XAttributeChange that is
    //  in the m_changed set holding the one and only reference to ccData. When the
    //  m_changed set is destroyed or if this particular XAttributeChange is removed from it,
    //  the destructor of the set's XAttributeChange object will release the remaining reference, causing
    //  the ccData object to delete itself.
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeSet::T_Iterator XAttributeChangeSet::Begin ()
    {
    return m_changed.begin ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeSet::T_Iterator XAttributeChangeSet::End ()
    {
    return m_changed.end ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeSet::T_ConstIterator XAttributeChangeSet::Begin () const
    {
    return m_changed.begin ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeSet::T_ConstIterator XAttributeChangeSet::End () const
    {
    return m_changed.end ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          XAttributeChangeSet::Size () const
    {
    return static_cast<UInt32>(m_changed.size ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeSet::T_Iterator XAttributeChangeSet::Find (XAttributeChangeCR c)
    {
    return m_changed.find (c);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeSet::T_Iterator XAttributeChangeSet::Find (XAttributeHandlerIdCR xHandlerId, UInt32 xAttrId)
    {
    return m_changed.find (XAttributeChange(xHandlerId, xAttrId, *XAttributeChangeDataHolder::GetSingletonNilObject(), XAttributeChange::CHANGETYPE_Unknown));
    }

/*---------------------------------------------------------------------------------**//**
// static
* @bsimethod                                    Sam.Wilson                      07/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XAttributeChangeSet::LoadXasAsWrites (MSElementDescrP  ed)
    {
    for (XAttributeCollection::const_iterator xa : XAttributeCollection (ed->GetElementRef()))
        {
        XAttributeChangeSetP set = ed->GetXAttributeChangeSet();
        if (NULL == set)
            return ERROR;

        // Note: if XA is already in the set, this does nothing
        set->Schedule (xa.GetHandlerId(), xa.GetId(), xa.GetSize(), xa.PeekData(), XAttributeChange::CHANGETYPE_Write);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
// static
* @bsimethod                                    Sam.Wilson                      07/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XAttributeChangeSet::CopyPersistentXas
(
MSElementDescrP sink,
MSElementDescrCP source,
XAttributeChange::ChangeType xct
)
    {
    XAttributeChangeSetP sinkSet = NULL;
    for (XAttributeCollection::const_iterator xa : XAttributeCollection (source->GetElementRef()))
        {
        if (NULL == sinkSet)
            {
            sinkSet = sink->GetXAttributeChangeSet();
            if (NULL == sinkSet)
/*<*/           return ERROR;
            }

        // Note: if XA is already in the sink set, this does nothing
        sinkSet->Schedule (xa.GetHandlerId(), xa.GetId(), xa.GetSize(), xa.PeekData(), xct);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
// static
* @bsimethod                                    Sam.Wilson                      04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChange::ChangeType XAttributeChange::NormalizeChangeType (XAttributeChange::ChangeType ct)
    {
    if (ct == XAttributeChange::CHANGETYPE_HistoricalVersion)
        return XAttributeChange::CHANGETYPE_Write;

    if (ct == XAttributeChange::CHANGETYPE_HistoricalVersionDidNotExist)
        return XAttributeChange::CHANGETYPE_Delete;

    return ct;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool     defaultIsXAttributeDataEqual
(
XAttributeChangeCR x,
XAttributeChangeCR x2,
UInt32          ,
double          ,
double
)
    {
    if (x.GetSize() != x2.GetSize())
        return false;

    return 0==memcmp (x.PeekData(), x2.PeekData(), x.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool            XAttributeChangeSet::IsEqual
(
XAttributeChangeSetCR  s2,
T_IsEqual*             f,
UInt32                 comparisonFlags,
double                 distanceTolerance,
double                 directionTolerance
)
    const
    {
    //  Test same # changes
    if (Size() != s2.Size())
        return false;

    if (NULL == f)
        f = defaultIsXAttributeDataEqual;

    T_ConstIterator it = Begin();
    T_ConstIterator it2 = s2.Begin();
    for ( ; it != End() && it2 != s2.End();  ++it, ++it2)
        {
        XAttributeChangeCR x = *it;
        XAttributeChangeCR x2 = *it2;

        //  Test same XAttribute HandlerID and item ID
        if (x.AttributeUInt64Key() != x2.AttributeUInt64Key())
            return false;

        //  Test same change type
        XAttributeChange::ChangeType ct  = XAttributeChange::NormalizeChangeType (x.GetChangeType());
        XAttributeChange::ChangeType ct2 = XAttributeChange::NormalizeChangeType (x2.GetChangeType());

        if (ct != ct2)
            return false;

        //  Test same data, if applicable
        if (ct == XAttributeChange::CHANGETYPE_Write
         && !f (x, x2, comparisonFlags, distanceTolerance, directionTolerance))
            return false;
        }

    return true;
    }
END_BENTLEY_DGNPLATFORM_NAMESPACE
