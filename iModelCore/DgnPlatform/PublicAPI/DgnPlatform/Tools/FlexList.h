/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Tools/FlexList.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley/HeapZone.h>
#include <Bentley/bvector.h>

BEGIN_BENTLEY_NAMESPACE

/*=================================================================================**//**
* FlexList - A FlexList is a memory efficient way of storing a variable length list of pointers, when it is known
*            that the list will most often hold zero or one entries. This class relies on its entries being pointers
*            since it uses the low bit of the entry's value for its own internal purposes.
* @bsiclass                                                     Keith.Bentley   06/03
+===============+===============+===============+===============+===============+======*/
template <class PTYPE> class FlexList
{
public:
    typedef FlexList<PTYPE> _MyType;

/*=================================================================================**//**
* FlexListEntry
* @bsiclass                                                     KeithBentley    10/00
+===============+===============+===============+===============+===============+======*/
struct  FlexListEntry
{
private:
    FlexListEntry*  m_next;
    PTYPE           m_entry;

public:
    inline void Init (PTYPE entry, FlexListEntry *next)
        {
        m_next  = next;
        m_entry = entry;
        }

    inline FlexListEntry*       GetNext() {return m_next;}
    inline void                 SetNext(FlexListEntry* newNext){m_next = newNext;}
    inline PTYPE                GetEntry() {return m_entry;}
    inline void                 Free (HeapZone* zone) {zone->Free (this, sizeof(FlexListEntry));}
};

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   10/04
+===============+===============+===============+===============+===============+======*/
struct          Iterator
{
private:
    PTYPE           m_ptr;
    FlexListEntry*  m_next;

public:
    explicit Iterator (_MyType& list)
        {
        if (list.IsSingleEntry())
            {
            m_ptr  = list.m_list;
            m_next = NULL;
            }
        else
            {
            FlexListEntry* first = list.GetList();
            m_ptr  = first->GetEntry();
            m_next = first->GetNext();
            }
        }
    explicit Iterator (FlexListEntry* entry)
        {
        m_ptr  = entry->GetEntry();
        m_next = entry->GetNext();
        }

    Iterator ToNext()   {return Iterator(m_next);}
    PTYPE GetPtr()      {return m_ptr;}
};

private:
    PTYPE       m_list;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
FlexListEntry*  GetList ()
    {
    return  ((FlexListEntry*) (((intptr_t) m_list) & ~0x01));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
void     SetList (FlexListEntry* root)
    {
    m_list = (NULL == root) ? NULL : ((PTYPE) ((intptr_t) root | 0x01));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsSingleEntry ()
    {
    // the low bit on means m_list is a ptr to a FlexListEntry
    return  ( (-1 == (intptr_t) m_list) || (0 == ((intptr_t) m_list & 0x01)) );
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
FlexList ()
    {
    m_list = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
FlexList (PTYPE  value)
    {
    m_list = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
void     Add
(
PTYPE       ptr,
HeapZone*   zone
)
    {
    if (NULL == m_list)
        {
        m_list = ptr;
        return;
        }

    // if the list has -1 (meaning all), get rid of that and substitute the new ptr.
    if (ContainsAll())
        {
        m_list = ptr;
        return;
        }

    // if ptr is already in list, stop
    if (Contains (ptr))
        return;

    FlexListEntry*    old, *newEntry;
    newEntry = (FlexListEntry*) zone->Alloc (sizeof(FlexListEntry));

    // if we already have a single list entry, we need to turn it into a multi-list
    if (IsSingleEntry())
        {
        old = (FlexListEntry*) zone->Alloc (sizeof(FlexListEntry));
        old->Init (m_list, NULL);
        }
    else
        old = GetList();

    newEntry->Init (ptr, old);
    SetList (newEntry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    06/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ContainsAll()
    {
    return (-1 == (intptr_t) m_list);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool     Contains
(
PTYPE       ptr
)
    {
    if (NULL == m_list)
        return  false;

    if (ContainsAll())
        return true;

    if (IsSingleEntry())
        return (m_list == ptr);

    FlexListEntry   *tEntry = GetList ();

    while (tEntry)
        {
        if (tEntry->GetEntry() == ptr)
            return  true;

        tEntry = tEntry->GetNext();
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            Drop
(
PTYPE       ptr,
HeapZone*   zone
)
    {
    if (IsSingleEntry())
        {
        // if m_list is -1 (containsAll true), then we want to NULL the list so it now contains none. Otherwise
        //  a new element can't be removed from the displayset (TR#115950).
        if (!ContainsAll() && m_list != ptr)
            return;

        m_list = NULL;
        return;
        }

    FlexListEntry   *prev, *tEntry, *pNext;

    prev        = NULL;
    tEntry      = GetList ();

    while (tEntry)
        {
        pNext = tEntry->GetNext();

        if (tEntry->GetEntry() == ptr)
            {
            if (prev) // unlink it
                prev->SetNext (pNext);
            else
                SetList (pNext);

            // free it
            zone->Free (tEntry, sizeof(FlexListEntry));
            tEntry = pNext;
            }
        else
            {
            prev    = tEntry;
            tEntry  = pNext;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            DropAll
(
HeapZone*    zone
)
    {
    if (!IsSingleEntry())
        {
        FlexListEntry   *freeEntry, *thisEntry;

        thisEntry   = GetList ();

        while (thisEntry)
            {
            freeEntry = thisEntry;
            thisEntry = thisEntry->GetNext();

            zone->Free (freeEntry, sizeof(FlexListEntry));
            }
        }

    m_list = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/06
+---------------+---------------+---------------+---------------+---------------+------*/
int             GetNumEntries()
    {
    if (NULL == m_list)
        return  0;

    if (IsSingleEntry())
        return  1;

    int count = 0;
    for (FlexListEntry* entry = GetList(); entry != NULL; entry = entry->GetNext())
        count++;

    return  count;
    }
};

/*=================================================================================**//**
* FlexArray - A flexArray is similar to a FlexList, except that entries are inserted and retrieved by array index.
*             FlexArray similarly relies on its entries being pointers since it uses the low bit
*             of the entry's value for its own internal purposes.
* @bsiclass                                                     Keith.Bentley   06/03
+===============+===============+===============+===============+===============+======*/
template <class PTYPE> class FlexArray
{
    PTYPE       m_list;

/*=================================================================================**//**
* FaArray
* @bsiclass                                                     KeithBentley    10/00
+===============+===============+===============+===============+===============+======*/
struct  FaArray
    {
    struct Header
        {
        int     size;
        }       m_hdr;
    PTYPE       m_entries[1];
};

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
FlexArray ()
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
FlexArray (PTYPE  value)
    {
    m_list = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            Init ()
    {
    m_list = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsSingleEntry ()
    {
    // the low bit on means m_list is a ptr to a FaArray
    return  (0 == ((int) m_list & 0x01));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
FaArray* GetArray ()
    {
    return  ((FaArray*) (((int) m_list) & ~0x01));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     SetArray (FaArray* root)
    {
    m_list = (NULL == root) ? NULL : ((PTYPE) ((int) root | 0x01));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/05
+---------------+---------------+---------------+---------------+---------------+------*/
void    ClearNewEntries (int oldEnd)
    {
    FaArray* array = GetArray();
    for (int i=oldEnd; i<array->m_hdr.size-1; i++)
        array->m_entries[i] = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
PTYPE*      GetEntryAddr (int index, HeapZone* zone)
    {
    if (index == 0 && IsSingleEntry())
        return  &m_list;

    BeAssert (index >= 0);

    FaArray* array = GetArray();

    if (IsSingleEntry())
        {
        PTYPE first = m_list;    // save current first entry
        SetArray (array = (FaArray*) zone->Alloc (sizeof(FaArray) + (index * sizeof(PTYPE))));

        array->m_entries[0] = first;
        array->m_hdr.size = index+1;
        ClearNewEntries (1);
        }
    else if (array->m_hdr.size <= index)
        {
        int oldNumEntries = array->m_hdr.size;
        int oldSize = sizeof(FaArray) + ((oldNumEntries-1) * sizeof(PTYPE));
        int newSize = sizeof(FaArray) + (index * sizeof(PTYPE));

        SetArray (array = (FaArray*) zone->Realloc (array, newSize, oldSize));
        array->m_hdr.size = index+1;
        ClearNewEntries (oldNumEntries);
        }

    return  (array->m_entries + index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
inline PTYPE    GetEntry (int index)
    {
    if (index == 0)
        {
        if (IsSingleEntry())
            return  m_list;

        return  GetArray()->m_entries[0];
        }

    FaArray*    array = GetArray();
    if (IsSingleEntry() || index >= array->m_hdr.size)
        return  NULL;

    return  array->m_entries[index];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
inline int      GetNumEntries()
    {
    if (NULL == m_list)
        return  0;

    return IsSingleEntry() ? 1 : GetArray()->m_hdr.size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
inline void     SetEntry
(
PTYPE       ptr,
int         index,
HeapZone*   zone
)
    {
    BeAssert (index >= 0);
    if (index < 0)
        return;

    *GetEntryAddr (index, zone) = ptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
inline void     Empty (HeapZone* zone)
    {
    if (!IsSingleEntry())
        {
        FaArray*    array = GetArray();
        zone->Free (array, sizeof(FaArray) + ((array->m_hdr.size-1) * sizeof(PTYPE)));
        }

    m_list = NULL;
    }
};


/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   06/03
+===============+===============+===============+===============+===============+======*/
template <class ETYPE> class HZArray
{
    struct  HzaHeader
        {
        int      m_nEntries;
        ETYPE    m_entries[1];
        };

    static const int ESIZE   = sizeof(ETYPE);
    static const int HDRSIZE = sizeof(HzaHeader);
    HzaHeader*  m_array;

    int         ArraySize (int index)   {return HDRSIZE + (index * ESIZE);} // there's one entry in HzaHeader
    int         CurrArraySize()         {return ArraySize (m_array->m_nEntries-1);}

public:
    HZArray ()                      {Init();}
    void        Init ()             {m_array = NULL;}
    ETYPE*      GetBase()           {return m_array->m_entries;}
    int         GetNumEntries()     {return (NULL == m_array) ? 0 : m_array->m_nEntries;}
    ETYPE*      AddEntry (HeapZone* zone) {return GetEntryAddr (GetNumEntries(), zone);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetArray (void* array, int index)
    {
    m_array = (HzaHeader*) array;
    m_array->m_nEntries = index+1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
ETYPE*      GetEntryAddr (int index, HeapZone* zone)
    {
    if (NULL == m_array)
        SetArray (zone->Alloc (ArraySize (index)), index);
    else if (m_array->m_nEntries <= index)
        SetArray (zone->Realloc (m_array, ArraySize(index), CurrArraySize()), index);

    return  (m_array->m_entries + index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     SetEntry (ETYPE ptr, int index, HeapZone*  zone)
    {
    BeAssert (index >= 0);
    if (index < 0)
        return;

    *GetEntryAddr (index, zone) = ptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     Empty (HeapZone* zone)
    {
    if (m_array)
        zone->Free (m_array, CurrArraySize());

    m_array = NULL;
    }
};

END_BENTLEY_NAMESPACE
