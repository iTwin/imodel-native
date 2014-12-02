/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/XAttributeHandlerManager.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

typedef void const* VoidCP;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool defaultIsXAttributeDataEqual (VoidCP data1, int size1, VoidCP data2, int size2)
    {
    return (size1==size2) && 0==memcmp(data1,data2,size1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2005
+---------------+---------------+---------------+---------------+---------------+------*/
bool XAttributeHandler::_AreEqual (VoidCP data1, int size1, VoidCP data2, int size2, UInt32, double, double)
    {
    return defaultIsXAttributeDataEqual (data1, size1, data2, size2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2005
+---------------+---------------+---------------+---------------+---------------+------*/
bool XAttributeHandler::AreEqual (VoidCP data1, int size1, VoidCP data2, int size2, UInt32 comparisonFlags, double distanceTolerance, double directionTolerance)
    {
    return  _AreEqual (data1, size1, data2, size2, comparisonFlags, distanceTolerance, directionTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerP XAttributeHandle::GetHandler () const
    {
    return m_project->Domains().FindXAttributeHandler(GetHandlerId());
    }

#if defined ELEMENT_LOADING_REWORK
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerP ElementHandle::XAttributeIter::GetHandler () const
    {
    return  XAttributeHandlerManager::FindHandler (GetHandlerId());
    }
#endif


static  ElementRefAppData::Key  s_key;
/*=================================================================================**//**
* @bsiclass                                                     JoshSchifter    11/07
+===============+===============+===============+===============+===============+======*/
struct          HostTxnHandlerFlag : ElementRefAppData
{
static  ElementRefAppData::Key& GetAppDataKey()
    {
    return s_key;
    }

static  bool IsElementFlagged (ElementRefP elemRef)
    {
    return (NULL != elemRef->FindAppData (HostTxnHandlerFlag::GetAppDataKey()));
    }

static  void AddFlagToElement (ElementRefP elemRef)
    {
    if (IsElementFlagged (elemRef))
        return;

    HeapZone&               zone  = elemRef->GetHeapZone();
    HostTxnHandlerFlag*     flag  = new ((HostTxnHandlerFlag*) zone.Alloc (sizeof(HostTxnHandlerFlag))) HostTxnHandlerFlag ();;

    elemRef->AddAppData (HostTxnHandlerFlag::GetAppDataKey(), flag, zone);
    }

virtual void        _OnCleanup (ElementRefP host, bool unloadingCache, HeapZone& zone) override
    {
    if (!unloadingCache)
        zone.Free (this, sizeof *this);
    }
};

#if defined ELEMENT_LOADING_REWORK
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttributeHandlerManager::NotifyHostTransactionHandlers (ElementRefP elemRef, ChangeTrackAction action, TransactionType transType, bool isPreChange)
    {
    if (!HostTxnHandlerFlag::IsElementFlagged (elemRef))
        return;

    FOR_EACH (XAttributeCollection::Entry const& xAttr, XAttributeCollection(elemRef))
        {
        XAttributeHandlerP                  handler = xAttr.GetHandler();
        IXAttributeHostTransactionHandler*  transHandler = handler ? handler->GetIXAttributeHostTransactionHandler () : NULL;

        if (NULL == transHandler)
            continue;

        switch (action)
            {
            case ChangeTrackAction::Delete:
                {
                if (isPreChange)
                    transHandler->_OnPreHostDelete (xAttr, transType);
                else
                    BeAssert(0);
                break;
                }
            case ChangeTrackAction::Add:
                {
                if (isPreChange)
                    BeAssert(0);
                else
                    transHandler->_OnPostHostAdd (xAttr, transType);
                break;
                }
            case ChangeTrackAction::Modify:
                {
                if (isPreChange)
                    transHandler->_OnPreHostChange (xAttr, transType);
                else
                    transHandler->_OnPostHostChange (xAttr, transType);
                break;
                }
            default:
                {
                BeAssert(0);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttributeHandlerManager::RegisterForHostTransactionNotification (XAttributeHandleCR xAttr)
    {
    XAttributeHandlerP                  handler = xAttr.GetHandler();
    IXAttributeHostTransactionHandler*  transHandler = handler ? handler->GetIXAttributeHostTransactionHandler () : NULL;

    if (NULL != transHandler)
        HostTxnHandlerFlag::AddFlagToElement (xAttr.GetElementRef());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XAttributeHandler::WriteXAttributeChangeSet (ElementRefP elRef, XAttributeChangeSetP attrs, bool isAdd)
    {
    if (NULL == attrs)
        return SUCCESS;

    ITxn& txn = elRef->GetDgnProject()->GetTxnManager().GetCurrentTxn();
    if (attrs->IsPurge())
        {
        while (true)
            {
            XAttributeHandle it = XAttributeCollection(elRef).begin();
            if (!it.IsValid())
                break;
            txn.DeleteXAttribute (it);
            }
        }

    StatusInt    status1;
    StatusInt    status = SUCCESS;
    for (XAttributeChangeSet::T_Iterator it = attrs->Begin();  it != attrs->End(); ++it)
        {
        XAttributeChangeCR attr = *it;

        switch (XAttributeChange::NormalizeChangeType(attr.GetChangeType()))
            {
            case XAttributeChange::CHANGETYPE_Write:
                {
                // re-instating a historical version
                XAttributeHandle it (elRef, attr.GetHandlerId(), attr.GetId());
                if (it.IsValid())
                    status1 = txn.ReplaceXAttributeData (it, attr.PeekData(), attr.GetSize());
                else
                    status1 = txn.AddXAttribute (elRef, attr.GetHandlerId(), attr.GetId(), attr.PeekData(), attr.GetSize(), NULL);
                break;
                }

            case XAttributeChange::CHANGETYPE_Delete:
                {
                XAttributeHandle it (elRef, attr.GetHandlerId(), attr.GetId());
                if (it.IsValid ())
                    status1 = txn.DeleteXAttribute (it);
                else
                    status1 = SUCCESS;
                break;
                }

            default:
                BeAssert (false); // invalid status
                status1 = ERROR;
                break;
            }

        if (SUCCESS != status1 && SUCCESS == status) // ***NEEDS WORK: Let the caller decide if a single failure should cancel all XAttr changes.
            status = status1;
        }

    if (SUCCESS == status)
        attrs->Clear ();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool isXAttributeDataEqual (XAttributeChangeCR x, XAttributeChangeCR x2, UInt32 comparisonFlags, double distanceTolerance, double directionTolerance)
    {
#if defined ELEMENT_LOADING_REWORK
    XAttributeHandlerP xh = XAttributeHandlerManager::FindHandler (x.GetHandlerId());
    if (NULL != xh)
        return xh->AreEqual (x.PeekData(), x.GetSize(), x2.PeekData(), x2.GetSize(), comparisonFlags, distanceTolerance, directionTolerance);
#endif

    return defaultIsXAttributeDataEqual (x.PeekData(), x.GetSize(), x2.PeekData(), x2.GetSize());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool            XAttributeHandler::AreXAttributeChangeSetsEqual (XAttributeChangeSetCP x1, XAttributeChangeSetCP x2, UInt32 comparisonFlags,
                                                                 double distanceTolerance, double directionTolerance)
    {
    if (NULL == x1 && NULL == x2)
        return true;

    if (NULL == x1 || NULL == x2)
        return false;

    return x1->IsEqual (*x2, isXAttributeDataEqual, comparisonFlags, distanceTolerance, directionTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool            XAttributeHandler::AreXAttributeChangeSetsEqual (MSElementDescrCP ed1, MSElementDescrCP ed2, UInt32 comparisonFlags,
                                                                 double distanceTolerance, double directionTolerance)
    {
    XAttributeChangeSetCP x1 = ed1->QueryXAttributeChangeSet ();
    XAttributeChangeSetCP x2 = ed2->QueryXAttributeChangeSet ();
    return XAttributeHandler::AreXAttributeChangeSetsEqual (x1, x2, comparisonFlags, distanceTolerance, directionTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool     isEditXAttributeIterDataEqual
(
ElementHandle::XAttributeIter const& x,
ElementHandle::XAttributeIter const& x2,
UInt32          comparisonFlags,
double          distanceTolerance,
double          directionTolerance
)
    {
#if defined ELEMENT_LOADING_REWORK
    XAttributeHandlerP xh = XAttributeHandlerManager::FindHandler (x.GetHandlerId());
    if (NULL != xh)
        return xh->AreEqual (x.PeekData(), x.GetSize(), x2.PeekData(), x2.GetSize(), comparisonFlags, distanceTolerance, directionTolerance);
#endif

    return defaultIsXAttributeDataEqual (x.PeekData(), x.GetSize(), x2.PeekData(), x2.GetSize());
    }

//=======================================================================================
//! Uniquely identifies an XAttribute within an element
//! @bsiclass                                                     Sam.Wilson      01/2005
//=======================================================================================
// ***************************** PERSISTENT VALUES *********************************
//  *****************************   DO NOT CHANGE   *********************************
struct XAUniqueId
{
UInt32  m_handler;
UInt32  m_id;

XAUniqueId () {}
XAUniqueId (UInt32 h, UInt32 i);
explicit XAUniqueId (XAttributeHandle const&);

explicit XAUniqueId (XAttributeChange const&);

XAttributeHandlerId GetHandlerId () const;

UInt32 GetId () const;

UInt64 AsUInt64 () const;

bool operator == (XAUniqueId const& cc) const {return m_handler==cc.m_handler && m_id==cc.m_id;}
bool operator < (XAUniqueId const& cc) const {return AsUInt64() < cc.AsUInt64();}

StatusInt CheckIsValid () const;

StatusInt Load (DataInternalizer& source);
void Store (DataExternalizer& sink) const;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2005
+---------------+---------------+---------------+---------------+---------------+------*/
XAUniqueId::XAUniqueId (UInt32 h, UInt32 i)
    :
    m_handler(h),
    m_id(i)
    {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2005
+---------------+---------------+---------------+---------------+---------------+------*/
XAUniqueId::XAUniqueId (XAttributeHandle const& xAttr)
    {
    m_handler = xAttr.GetHandlerId().GetId();
    m_id = xAttr.GetId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2005
+---------------+---------------+---------------+---------------+---------------+------*/
XAUniqueId::XAUniqueId (XAttributeChange const& xc)
    {
    m_handler = xc.GetHandlerId().GetId();
    m_id = xc.GetId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2005
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerId  XAUniqueId::GetHandlerId () const
    {
    return XAttributeHandlerId ((m_handler&0xffff0000)>>16, (m_handler&0xffff));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2005
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          XAUniqueId::GetId () const
    {
    return m_id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2005
+---------------+---------------+---------------+---------------+---------------+------*/
UInt64          XAUniqueId::AsUInt64 () const
    {
    return ((UInt64)m_handler << 32) | m_id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XAUniqueId::CheckIsValid () const
    {
    if (0 == m_handler)   // A XAttribute handler ID is never 0
        return ERROR;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool            XAttributeHandler::AreXAttributesEqual (ElementHandleCR eh1, ElementHandleCR eh2, UInt32 comparisonFlags,
                                                        double  distanceTolerance, double directionTolerance)
    {
    //  NEEDS WORK: This algorithm is O[n*m]
    //              In order to make it O[n*log n] we could load all XAttributes into memory (into an XAttributeChangeSet).
    //              So, this is a straightforward speed vs. space trade-off.
    bset<XAUniqueId> inboth;
    for (ElementHandle::XAttributeIter xa1 (eh1); xa1.IsValid(); xa1.ToNext())
        {
        ElementHandle::XAttributeIter xa2 (eh2, xa1.GetHandlerId(), xa1.GetId());
        if (!xa2.IsValid())
            return false;

        if (!isEditXAttributeIterDataEqual (xa1, xa2, comparisonFlags, distanceTolerance, directionTolerance))
            return false;

        inboth.insert (XAUniqueId(xa1.GetHandlerId().GetId(), xa1.GetId()));
        }

    //  Are all xas in eh2 also in eh1?
    for (ElementHandle::XAttributeIter xa2 (eh2); xa2.IsValid(); xa2.ToNext())
        {
        if (inboth.find (XAUniqueId(xa2.GetHandlerId().GetId(), xa2.GetId())) == inboth.end())
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Utility function to copy XAs from element to element.
* @bsimethod                                    Sam.Wilson                      12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XAttributeHandler::CopyPersistentXAttributes (ElementRefP dst, ElementRefP src)
    {
    FOR_EACH (XAttributeCollection::Entry const& iXA, XAttributeCollection (src))
        {
        Int64 rowid;
        UInt32 id = iXA.GetId();
        DgnModels::XAttributeFlags flags;
        dst->GetDgnProject()->Models().InsertXAttribute (rowid, dst->GetElementId(), iXA.GetHandlerId(), id, iXA.GetSize(), iXA.PeekData(), flags);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Utility function to copy XAs from element to element.
* @bsimethod                                    Sam.Wilson                      12/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       XAttributeHandler::CopyPersistentXAttributes (ElementHandleCR dst, ElementHandleCR src)
    {
    return CopyPersistentXAttributes (dst.GetElementRef(), src.GetElementRef());
    }
