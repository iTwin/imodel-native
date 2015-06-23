/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/QvElemSet.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#define INVALID_QvElem      ((QvElem*) 0x0001)

BENTLEY_NAMESPACE_TYPEDEFS(HeapZone)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
template <class _QvKey> struct QvElemSet : DgnElement::AppData
{
protected:
    struct  Entry
        {
        _QvKey      m_key;
        QvElem*     m_qvElem;
        Entry*      m_next;

        void SetValue (QvElem* qvElem) {if (m_qvElem) m_key.DeleteQvElem (m_qvElem); m_qvElem = qvElem;}
        void Clear (){m_qvElem = NULL;}

        Entry (_QvKey const& key, QvElem* qvElem, Entry* next) : m_key (key), m_qvElem (qvElem), m_next (next) {}
        };


    HeapZone&   m_zone;
    Entry*      m_entry;

    ~QvElemSet() {FreeAll (false);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void FreeAll (bool qvCacheDeleted)
    {
    for (Entry* thisEntry=m_entry, *next; thisEntry; thisEntry=next)
        {
        next = thisEntry->m_next;

        if (!qvCacheDeleted)
            thisEntry->SetValue(NULL);

        thisEntry->~Entry ();
        m_zone.Free (thisEntry, sizeof (Entry));
        }
    }

public:
    QvElemSet (HeapZone& zone) : m_zone(zone) {m_entry = NULL;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            Add (_QvKey const& key, QvElem* qvElem)
    {
    Entry* prevEntry = NULL;
    Entry* nextEntry = m_entry;

    for (; nextEntry; prevEntry=nextEntry, nextEntry=nextEntry->m_next)
        {
        if (nextEntry->m_key.LessThan(key)) // sort them by key
            continue;

        if (!nextEntry->m_key.Equal (key))
            break;

        nextEntry->SetValue (qvElem);      // already exists, just change it
        return ;
        }

    Entry* newEntry = (Entry*) m_zone.Alloc (sizeof (Entry));
    new (newEntry) Entry (key, qvElem, nextEntry);

    if (prevEntry)
        prevEntry->m_next = newEntry;
    else
        m_entry = newEntry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Drop (_QvKey const& key)
    {
    for (Entry* prev=NULL, *thisEntry=m_entry; thisEntry; prev=thisEntry, thisEntry=thisEntry->m_next)
        {
        if (thisEntry->m_key.LessThan(key)) // entries are sorted by key
            continue;

        if (!thisEntry->m_key.Equal(key))
            break;

        if (prev)
            prev->m_next = thisEntry->m_next;
        else
            m_entry = thisEntry->m_next;

        thisEntry->SetValue (NULL);
        m_zone.Free (thisEntry, sizeof(Entry));
        return  SUCCESS;
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DropRange (_QvKey const& lowRange, _QvKey const& highRange)
    {
    Entry* previous, *thisEntry;
    for (previous = NULL, thisEntry = m_entry; NULL != thisEntry && thisEntry->m_key.LessThan (lowRange);
                previous = thisEntry, thisEntry = thisEntry->m_next)
        ;

    if (NULL == thisEntry)
        return ERROR;

    for (Entry* next = thisEntry->m_next; NULL != thisEntry && (thisEntry->m_key.LessThan (highRange) || thisEntry->m_key.Equal (highRange)); thisEntry = next)
        {
        thisEntry->SetValue (NULL);
        thisEntry->~Entry ();
        m_zone.Free (thisEntry, sizeof (Entry));
        }

    if (NULL != previous)
        previous->m_next = thisEntry;
    else
        m_entry = thisEntry;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem*         Find (_QvKey const& key)
    {
    for (Entry* thisEntry = m_entry; thisEntry; thisEntry = thisEntry->m_next)
        {
        if (thisEntry->m_key.LessThan(key)) // entries are sorted by key
            continue;

        return thisEntry->m_key.Equal(key) ? thisEntry->m_qvElem : NULL;
        }
    return  NULL;
    }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

