/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Tools/KeyTree.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley/HeapZone.h>
#include    <string.h>
#include    <limits>
#include    <limits.h>
#include    <Bentley/BeStringUtilities.h>

BEGIN_BENTLEY_NAMESPACE

enum
    {
    KEYRANGE_NoOverlap     = 0,
    KEYRANGE_Contained     = 1,
    KEYRANGE_Overlap       = 2,
    INITIAL_SIZE        = 10,

    NUM_LEAFENTRIES     = 50,
    NUM_INTERNALENTRIES = 20,
    };

#ifdef DEBUG_KEYTREE
inline void doIndent(int num) {for (int i=0;i<num;i++) printf (" ");}
#endif

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    01/01
+===============+===============+===============+===============+===============+======*/
template <class KTYPE> struct KeyRange
{
    typedef KeyRange<KTYPE>                 T_KeyRange;

    KTYPE     m_low;
    KTYPE     m_high;

/*---------------------------------------------------------------------------------**//**
* Initializes this range structure to the specified low and high values.
* @bsimethod                                                    ShaunSewall     01/00
+---------------+---------------+---------------+---------------+---------------+------*/
inline void Init (KTYPE low, KTYPE high)
    {
    m_low   = low;
    m_high  = high;
    }

/*---------------------------------------------------------------------------------**//**
* Initializes this range structure to the same low and high value.
* @bsimethod                                                    ShaunSewall     01/00
+---------------+---------------+---------------+---------------+---------------+------*/
inline void Init (KTYPE match)
    {
    m_low   = match;
    m_high  = match;
    }

/*---------------------------------------------------------------------------------**//**
* Returns <code>true</code> if this structure identifies a <i>null</i> range.  A null
* range is defined as having the low value be greater than the high value.
* @bsimethod                                                    KeithBentley    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool IsNull()
    {
    return (m_low > m_high);
    }

/*---------------------------------------------------------------------------------**//**
* Extends this range to accomodate the specified range.
* @bsimethod                                                    KeithBentley    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
inline void Extend (T_KeyRange const* pRange)
    {
    if (pRange->m_low < m_low)
        m_low = pRange->m_low;

    if (pRange->m_high > m_high)
        m_high = pRange->m_high;
    }

/*---------------------------------------------------------------------------------**//**
* Extends this range to accomodate the specified key.
* @bsimethod                                                    KeithBentley    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
inline void Extend (KTYPE newKey)
    {
    if (newKey < m_low)
        m_low = newKey;

    if (newKey > m_high)
        m_high = newKey;
    }

/*---------------------------------------------------------------------------------**//**
* Returns <code>true</code> if this range contains the specified key.
* @bsimethod                                                    KeithBentley    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool Contains (KTYPE key) const
    {
    if ((key < m_low) || (key > m_high))
        return  false;

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* Returns <code>true</code> if this range contains the specified other range.
* @bsimethod                                                    ShaunSewall     01/00
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool Contains (T_KeyRange const* pRange) const
    {
    if ((pRange->m_high > m_high) || (pRange->m_low < m_low))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Returns one of the RANGE_xxx value indicating the relationship of this range to the
* specified other range.
* @bsimethod                                                    KeithBentley    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
inline int TestRange (T_KeyRange *pOther)
    {
    if ((m_low > pOther->m_high) || (m_high < pOther->m_low))
        return KEYRANGE_NoOverlap;

    if ((m_low < pOther->m_low) && (m_high > pOther->m_high))
        return KEYRANGE_Contained;

    return KEYRANGE_Overlap;
    }
};

template <class ETYPE, class KTYPE>  struct  IParent;
template <class ETYPE, class KTYPE>  class LeafNode;
template <class ETYPE, class KTYPE>  class InternalNode;

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    01/01
+===============+===============+===============+===============+===============+======*/
template <class ETYPE, class KTYPE> struct IChild
{
protected:
    typedef KeyRange<KTYPE>         T_KeyRange;
    typedef IParent<ETYPE,KTYPE>    T_IParent;
    typedef LeafNode<ETYPE,KTYPE>   T_LeafNode;

public:
    typedef StatusInt (*Processor) (const ETYPE*, void*);

public:
    virtual void                Add (ETYPE* entry) = 0;
    virtual void                GetExactNodeRange (T_KeyRange *pRange) = 0;
    virtual bool                ContainsKey (KTYPE key) const = 0;
    virtual void                SetNodeRange (const T_KeyRange *pRange) = 0;
    virtual void                SetParent (T_IParent*  newParent) = 0;
    virtual ETYPE*              Find(KTYPE key, T_LeafNode** pLeaf, int* pIndex) = 0;
    virtual ETYPE*              FindEntry (ETYPE* searchFor, T_LeafNode** pLeaf, int* pIndex) = 0;
    virtual int                 CountNodes () const = 0;
    virtual void                Dump(int indent)  = 0;
    virtual StatusInt           Process  (T_KeyRange*, Processor, void*) const = 0;
    virtual bool                IsEmpty () const = 0;
    virtual T_LeafNode const*   GetFirstNode() const = 0;
};

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    01/01
+===============+===============+===============+===============+===============+======*/
template <class ETYPE, class KTYPE> struct IParent
{
    typedef KeyRange<KTYPE>            T_KeyRange;
    typedef IChild<ETYPE,KTYPE>        T_IChild;
    typedef LeafNode<ETYPE,KTYPE>      T_LeafNode;
    typedef InternalNode<ETYPE,KTYPE>  T_InternalNode;

public:
    virtual void                IncreaseRange (const T_KeyRange  *pRange) = 0;
    virtual void                AddChildNode (T_IChild*  oNewNode) = 0;
    virtual void                RemoveKey (KTYPE key) = 0;
    virtual void                DropChildNode (T_IChild* oChildNode, bool) = 0;
    virtual T_LeafNode const*   NextSibling (T_IChild const* me) const = 0;
    virtual T_InternalNode*     NewInternalNode (IParent* parent) = 0;
    virtual T_LeafNode*         NewLeafNode (IParent* parent) = 0;
    virtual void                Free (T_IChild* child, bool leaf) = 0;
};

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    12/97
+===============+===============+===============+===============+===============+======*/
template <class ETYPE, class KTYPE> class KeyRangeNode : public IChild<ETYPE,KTYPE>
{
    typedef KeyRange<KTYPE>         T_KeyRange;
    typedef IParent<ETYPE,KTYPE>    T_IParent;

protected:
    T_IParent*          m_parent;
    T_KeyRange          m_range;
    bool                m_isSloppy;
    int                 m_nEntries;

public:
/*---------------------------------------------------------------------------------**//**
* Constructs an node using the specified maximum capacity.
* @param        maxEntries      The maximum capacity for this tree node.
* @bsimethod                                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
KeyRangeNode(T_IParent* parent)
    {
    m_parent     = parent;
    m_nEntries   = 0;
    m_isSloppy   = false;
    InitRange();
    }

    virtual void        CalculateNodeRange() = 0;
    void                SetParent(T_IParent*  newParent) {m_parent = newParent;}
    T_IParent const*    GetParent () const               {return m_parent;}
    bool                IsSloppy() const                 {return m_isSloppy;}
    void                CheckSloppy() const              {if (IsSloppy()){((KeyRangeNode*)this)->CalculateNodeRange();}}
    bool                ContainsKey (KTYPE key) const    {CheckSloppy(); return m_range.Contains(key);}
    int                 GetCount () const                {return m_nEntries;}

/*---------------------------------------------------------------------------------**//**
* Initializes the range for this node.
* @bsimethod                                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void InitRange(KTYPE min, KTYPE max)
    {
    m_range.Init (min, max);
    m_isSloppy = false;
    }

/*---------------------------------------------------------------------------------**//**
* Initializes the range for this node.
* @bsimethod                                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void InitRange()
    {
    InitRange (ETYPE::GetMaxKey(), ETYPE::GetMinKey());
    }

/*---------------------------------------------------------------------------------**//**
* Returns the current range of this node.  This could be greater than the exact range if
* this node is <i>sloppy</i>.
* @see          #getExactNodeRange
* @bsimethod                                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
T_KeyRange* GetNodeRange(T_KeyRange* pRange)
    {
    *pRange = m_range;
    return pRange;
    }

/*---------------------------------------------------------------------------------**//**
* This method returns the exact range of this node. It will be recalculated if this node
* is currently "sloppy".
* @param        pRange  Output parameter which will hold the range.
* @return       <CODE>true</CODE> will be returned if the range is valid.
* @see          #getNodeRange
* @see          #isSloppy
* @bsimethod                                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void GetExactNodeRange (T_KeyRange* pRange)
    {
    if (m_isSloppy)
        CalculateNodeRange ();

    *pRange = m_range;
    }

/*---------------------------------------------------------------------------------**//**
* Sets the range of this node.
* @bsimethod                                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void SetNodeRange(T_KeyRange const* pRange)
    {
    m_range     = *pRange;
    m_isSloppy  = false;
    }
};


/*=================================================================================**//**
* @bsiclass                                                     ShaunSewall     02/00
+===============+===============+===============+===============+===============+======*/
template <class ETYPE, class KTYPE> class  LeafNode : public KeyRangeNode <ETYPE, KTYPE>
{
    typedef IParent<ETYPE,KTYPE>                T_IParent;
    typedef IChild<ETYPE,KTYPE>                 T_IChild;
    typedef KeyRange<KTYPE>                     T_KeyRange;
    typedef ETYPE const*                        T_ETYPEP;
    typedef KeyRangeNode<ETYPE,KTYPE>           T_KeyRangeNode;
    typedef typename T_KeyRangeNode::Processor  Processor;
    typedef typename T_KeyRangeNode::T_LeafNode T_LeafNode;
    using  T_KeyRangeNode::m_nEntries;
    using  T_KeyRangeNode::m_range;
    using  T_KeyRangeNode::m_isSloppy;
    using  T_KeyRangeNode::m_parent;

    Byte m_entData[NUM_LEAFENTRIES * sizeof(ETYPE)];  // don't want constructors we get if we use ETYPE[]

public:

    int                 CountNodes() const         {return 1;}
    bool                CheckIndex(int index)      {return (index >= 0) && (index < m_nEntries);}
    T_ETYPEP            Entries() const            {return (T_ETYPEP) m_entData;}
    T_ETYPEP            GetEntry(int index) const  {return Entries() + index;}
    T_ETYPEP            FirstEntry() const         {return Entries();}
    T_ETYPEP            LastEntry() const          {return GetEntry(m_nEntries-1);}
    bool                IsEmpty() const            {return 0 == this->GetCount();}
    T_LeafNode const*   GetFirstNode () const      {return  this;}

/*---------------------------------------------------------------------------------**//**
* Constructs a new LeafNode with the specified maximum number of entries.
* @param        parent      the parent of this node
* @param        maxEntries  the maximum capacity for this node.
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
LeafNode (T_IParent* parent) : KeyRangeNode<ETYPE,KTYPE> (parent)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
void AddEntry (ETYPE const* entry)
    {
    int     index   = m_nEntries;
    KTYPE   key     = entry->GetKey ();

    if (key < m_range.m_high)
        {
        // binary search for position to put new entry
        for (int begin=0, end=m_nEntries; begin < end;)
            {
            index = begin + (end - begin - 1)/2;
            KTYPE thisId = GetEntry(index)->GetKey ();
            if (key < thisId)
                end = index;
            else if (thisId < key)
                begin = ++index;
            else
                {
                BeDataAssert (false); // duplicate keys!
                break;
                }
            }


        ETYPE const*  tEntry = GetEntry (index);
        memmove ((void*) (tEntry+1), tEntry, (m_nEntries-index) * sizeof (ETYPE));
        }

    memcpy ((void*) GetEntry(index), entry, sizeof(ETYPE));
    m_nEntries++;
    }

/*---------------------------------------------------------------------------------**//**
* Adds an entry to this node.
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
void Add (ETYPE* entry)
    {
    if (m_isSloppy)
        CalculateNodeRange();

    AddEntry (entry);

    KTYPE   key;
    if (!m_range.Contains (key = entry->GetKey()))
        {
        m_range.Extend (key);
        m_parent->IncreaseRange (&m_range);
        }

    if (NUM_LEAFENTRIES == this->GetCount ())
        SplitLeafNode ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
void CalculateNodeRange()
    {
    // just get the first and last entry, since entries are always sorted.
    this->InitRange (FirstEntry()->GetKey(), LastEntry()->GetKey());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
ETYPE* Find (KTYPE key, LeafNode** pLeaf, int* pIndex)
    {
    for (int begin=0, end=m_nEntries; begin < end;)
        {
        int index = begin + (end - begin - 1)/2;
        KTYPE thisKey = GetEntry(index)->GetKey();
        if (key < thisKey)
            end = index;
        else if (thisKey < key)
            begin = index + 1;
        else
            {
            if (pLeaf)
                *pLeaf = this;

            if (pIndex)
                *pIndex = index;

            return const_cast <ETYPE*> (GetEntry(index));
            }
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
ETYPE* FindEntry (ETYPE* searchEntry, LeafNode** pLeaf, int* pIndex)
    {
    KTYPE key = searchEntry->GetKey();

    for (T_ETYPEP curr=FirstEntry(), last=LastEntry(); curr <= last; ++curr)
        {
        if (key == curr->GetKey () && (curr->GetValue() == searchEntry->GetValue()))
            {
            if (pLeaf)
                *pLeaf = this;

            if (pIndex)
                *pIndex = static_cast<int>(curr - FirstEntry());

            return const_cast <ETYPE*> (curr);
            }
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Removes the entry at the specified index.
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
void RemoveEntry (int index, ETYPE* copyOfEntry)
    {
    if (!CheckIndex(index))
        return;

    ETYPE const*    entry = GetEntry (index);

    if (NULL != copyOfEntry)
        memcpy (copyOfEntry, entry, sizeof (ETYPE));

    KTYPE   key = entry->GetKey ();

    if (index < --m_nEntries)
        memmove ((void*) entry, entry+1, (m_nEntries-index) * sizeof(ETYPE));

    m_isSloppy = true;

    m_parent->RemoveKey (key);

    if (0 == this->GetCount ())
        m_parent->DropChildNode (this, true);
    }

/*---------------------------------------------------------------------------------**//**
* Splits a leaf node that has exceeded its maximum number of entries.
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
void SplitLeafNode()
    {
    if (NULL == m_parent)
        return;

    LeafNode*   newNode     = m_parent->NewLeafNode (m_parent);
    int         numEntries  = this->GetCount();
    int         start       = (numEntries - (numEntries / 5)) + 1;

    for (int i = start; i < numEntries; i++)
        newNode->AddEntry (GetEntry(i));

    m_nEntries = start;
    CalculateNodeRange();

    newNode->CalculateNodeRange();
    m_parent->AddChildNode (newNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    LeafNode                                        ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
void Dump (int indent)
    {
#ifdef DEBUG_KEYTREE
    doIndent (indent);
    printf ("leaf {%d} low=%d, high=%d\n", m_nEntries, m_range.m_low, m_range.m_high);

    for (T_ETYPEP curr=FirstEntry(), last=LastEntry(); curr <= last; curr++)
        {
        if (curr->GetKey() > m_range.m_high || curr->GetKey() < m_range.m_low)
            {
            BeAssert (0);
            }
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* Invoke a callback on each entry in the leaf node.
* @bsimethod    LeafNode                                        SamWilson       04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Process (T_KeyRange* range, Processor proc, void *pArg) const
    {
    for (T_ETYPEP curr=FirstEntry(), last=LastEntry(); curr <= last; curr++)
        {
        if (range)
            {
            if (curr->GetKey() < range->m_low)
                continue;

            if (curr->GetKey() > range->m_high)
                return  SUCCESS;
            }

        if (SUCCESS != proc (curr, pArg))
            return  ERROR;
        }

    return  SUCCESS;
    }
};

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    12/97
+===============+===============+===============+===============+===============+======*/
template <class ETYPE, class KTYPE> class InternalNode : public KeyRangeNode <ETYPE,KTYPE>, IParent<ETYPE,KTYPE>
{
    typedef KeyRange<KTYPE>                     T_KeyRange;
    typedef IParent<ETYPE,KTYPE>                T_IParent;
    typedef IChild<ETYPE,KTYPE>                 T_IChild;
    typedef T_IChild*                           T_IChildP;
    typedef T_IChild**                          T_IChildH;
    typedef T_IChild* const*                    T_IChildCH;
    typedef LeafNode <ETYPE, KTYPE>             T_LeafNode;
    typedef InternalNode <ETYPE, KTYPE>         T_InternalNode;
    typedef KeyRangeNode<ETYPE,KTYPE>           T_KeyRangeNode;
    typedef typename T_KeyRangeNode::Processor  Processor;
    using  T_KeyRangeNode::m_nEntries;
    using  T_KeyRangeNode::m_range;
    using  T_KeyRangeNode::m_isSloppy;
    using  T_KeyRangeNode::m_parent;

    T_IChildP   m_children[NUM_INTERNALENTRIES];

public:
    InternalNode (T_IParent* parent) : KeyRangeNode<ETYPE,KTYPE> (parent) {}
    T_InternalNode*     NewInternalNode (T_IParent* parent) {return m_parent->NewInternalNode (parent);}
    T_LeafNode*         NewLeafNode (T_IParent* parent)     {return m_parent->NewLeafNode (parent);}
    void                Free (T_IChild* child, bool leaf)   {m_parent->Free (child, leaf);}

    T_IChildCH          GetEntryC (int i) const     {return m_children+i;}
    T_IChildCH          FirstEntryC()     const     {return GetEntryC(0);}
    T_IChildCH          LastEntryC()      const     {return GetEntryC(m_nEntries-1);}
    T_IChildH           GetEntry (int i)            {return m_children+i;}
    T_IChildH           FirstEntry()                {return GetEntry(0);}
    T_IChildH           LastEntry()                 {return GetEntry(m_nEntries-1);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod    InternalNode                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
int CountEntries ()
    {
    int     count=0;
    for (T_IChildH curr = FirstEntry(), last = LastEntry(); curr <= last ; curr++)
        count += (*curr)->CountEntries();

    return  count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    InternalNode                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsEmpty() const
    {
    for (int i=0; i<m_nEntries; i++)
        {
        if (!m_children[i]->IsEmpty())
             return false;
        }

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
int CountNodes () const
    {
    int count = 1;

    for (int i=0; i<m_nEntries; i++)
        {
        count += m_children[i]->CountNodes();
        }

    return  count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    InternalNode                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
T_IChildP ChooseBestNode (KTYPE key)
    {
    if (m_isSloppy)
        CalculateNodeRange ();

    if (key >= m_range.m_high)
        return *LastEntry();

    if (key <= m_range.m_low)
        return  *FirstEntry();

    T_IChild*   lastNode = NULL;
    KTYPE       distance, lastDist = ETYPE::GetMaxKey();

    for (T_IChildH curr = FirstEntry(), last = LastEntry(); curr <= last ; curr++)
        {
        T_KeyRange  thisRange;
        (*curr)->GetExactNodeRange (&thisRange);

        if (key >= thisRange.m_low)
            {
            if (key <= thisRange.m_high)
                {
                return  *curr;
                }

            lastDist = (KTYPE) (key - thisRange.m_high);
            lastNode = *curr;
            }
        else
            {
            distance = (KTYPE)(thisRange.m_low - key);

            if (distance < lastDist)
                return  *curr;
            else
                return  lastNode;
            }
        }

    return  lastNode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    InternalNode                                    KeithBentley    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
ETYPE* Find (KTYPE key, T_LeafNode** pLeaf, int* pIndex)
    {
    for (int begin=0, end=m_nEntries; begin < end;)
        {
        int index = begin + (end - begin - 1)/2;

        T_KeyRange  thisRange;
        m_children[index]->GetExactNodeRange (&thisRange);

        if (key < thisRange.m_low)
            end = index;
        else if (thisRange.m_high < key)
            begin = index + 1;
        else
            return m_children[index]->Find (key, pLeaf, pIndex);
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    InternalNode                                    KeithBentley    07/99
+---------------+---------------+---------------+---------------+---------------+------*/
ETYPE* FindEntry (ETYPE* searchEntry, T_LeafNode** pLeaf, int* pIndex)
    {
    ETYPE*      entry;
    T_KeyRange  thisRange;
    KTYPE       key = searchEntry->GetKey();

    for (T_IChildH curr = FirstEntry(), last = LastEntry(); curr <= last ; curr++)
        {
        (*curr)->GetExactNodeRange (&thisRange);

        if (key > thisRange.m_high)
            continue;

        if (key < thisRange.m_low)
            return NULL;

        if (NULL != (entry = (*curr)->FindEntry (searchEntry, pLeaf, pIndex)))
            return  entry;
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    InternalNode                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void Add (ETYPE* entry)
    {
    ChooseBestNode (entry->GetKey())->Add (entry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    InternalNode                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void RemoveKey (KTYPE key)
    {
    // if we're already sloppy, DON'T look at the range members - they may be invalid.
    if (m_isSloppy || m_range.m_low == key || m_range.m_high == key)
        {
        m_isSloppy = true;

        if (NULL != m_parent)
            m_parent->RemoveKey (key);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool Contains (T_IChildP child)
    {
    for (T_IChildH curr = FirstEntry(), last = LastEntry(); curr <= last ; curr++)
        {
        if (*curr == child)
            return  true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    InternalNode                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void AddChildNode (T_IChild* oNewNode)
    {
    if (Contains (oNewNode))
        return;

    oNewNode->SetParent(this);

    T_KeyRange   range;
    oNewNode->GetExactNodeRange (&range);
    m_range.Extend (&range);

    int index = 0;
    for (; index <m_nEntries; index++)
        {
        T_KeyRange   thisRange;
        m_children[index]->GetExactNodeRange (&thisRange);

        if (range.m_high <= thisRange.m_low)
            {
            memmove ((void*) &m_children[index+1], &m_children[index], (m_nEntries-index) * sizeof (T_IChildP));
            break;
            }
        }

    m_children[index] = oNewNode;

    if (NUM_INTERNALENTRIES == ++m_nEntries)
        SplitInternalNode ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    InternalNode                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void IncreaseRange (T_KeyRange const* pRange)
    {
    if (m_isSloppy)
        {
        CalculateNodeRange ();
        }
    else
        {
        if (m_range.Contains (pRange))
            return;

        m_range.Extend (pRange);
        }

    m_parent->IncreaseRange (&m_range);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DropEntry (T_IChildP child)
    {
    for (T_IChildH curr = FirstEntry(), last = LastEntry(); curr <= last ; curr++)
        {
        if (*curr == child)
            {
            int index = static_cast<int>(curr - FirstEntry());
            if (index < --m_nEntries)
                memmove ((void*) curr, curr+1, (m_nEntries-index) * sizeof (T_IChildP));
            return  SUCCESS;
            }
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* Removes the specified child node from this internal node. During a "merge" it is
* necessary to defer removal of empty nodes until the validateChanged method.
* @param        oChildNode      The child node to drop
* @bsimethod    InternalNode                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void DropChildNode (T_IChild* oChildNode, bool leaf)
    {
    if (ERROR == DropEntry (oChildNode))
        return;

    m_parent->Free (oChildNode, leaf);

    if (0 == this->GetCount())
        {
        if (NULL != m_parent)
            m_parent->DropChildNode (this, false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    InternalNode                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void SplitInternalNode()
    {
    if (NULL == m_parent)
        return;

    T_InternalNode*     oNewNode = m_parent->NewInternalNode (m_parent);
    int                 nEntries = this->GetCount();
    int                 start = (nEntries - (nEntries/4)) + 1;

    for (int i = start; i < nEntries; i++)
        oNewNode->AddChildNode (m_children[i]);

    m_nEntries = start;
    CalculateNodeRange();

    m_parent->AddChildNode (oNewNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    InternalNode                                    KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void CalculateNodeRange ()
    {
    T_KeyRange    currRange;

    this->InitRange();

    for (T_IChildH curr = FirstEntry(), last = LastEntry(); curr <= last ; curr++)
        {
        (*curr)->GetExactNodeRange (&currRange);
        m_range.Extend (&currRange);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    InternalNode                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
void Dump (int indent)
    {
#ifdef DEBUG_KEYTREE
    doIndent (indent);
    printf ("internal {%d} low=%d, high=%d\n", m_nEntries, m_range.m_low, m_range.m_high);

    for (T_IChildH curr = FirstEntry(), last = LastEntry(); curr <= last ; curr++)
        (*curr)->Dump(indent+3);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* Tell all leaf nodes to process their entries
* @bsimethod    InternalNode                                    SamWilson       04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Process (T_KeyRange* range, Processor proc, void *pArg) const
    {
    for (T_IChildCH curr = FirstEntryC(), last = LastEntryC(); curr <= last ; curr++)
        {
        if (range)
            {
            T_KeyRange  tRange;

            (*curr)->GetExactNodeRange (&tRange);

            if (range->m_low > tRange.m_high)
                continue;

            if (range->m_high < tRange.m_low)
                return  SUCCESS;
            }

        if (SUCCESS != (*curr)->Process (range, proc, pArg))
            return  ERROR;
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
T_LeafNode const* GetFirstNode () const
    {
    return  (*FirstEntryC())->GetFirstNode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
T_LeafNode const* NextSibling (T_IChild const* from) const
    {
    for (T_IChildCH curr = FirstEntryC(), last = LastEntryC(); curr <= last ; curr++)
        {
        if (from == *curr)
            {
            if (++curr > last)
                {
                T_IChild const* mySibling = m_parent->NextSibling (this);

                if (NULL == mySibling)
                    return  NULL;

                return  mySibling->GetFirstNode ();
                }

            return  (*curr)->GetFirstNode();
            }
        }

    BeAssert (0);
    return  NULL;
    }
};


/*=================================================================================**//**
*  A key tree is a templated, ordered collection of entries which hold an <i>internal</i> key.
* Entries must be unique within a key tree, but multiple entries can have
* the same key.  The entry's key must not change since this defines the sort criteria.
* @bsiclass                                                     ShaunSewall     02/00
+===============+===============+===============+===============+===============+======*/
template <class ETYPE, class KTYPE> class KeyTree : public IParent <ETYPE,KTYPE>
{
    typedef KeyRange<KTYPE>                 T_KeyRange;
    typedef LeafNode <ETYPE, KTYPE>         T_LeafNode;
    typedef InternalNode <ETYPE, KTYPE>     T_InternalNode;
    typedef IChild <ETYPE, KTYPE>           T_IChild;
    typedef IParent<ETYPE, KTYPE>           T_IParent;

    FixedSizePool1 m_leafPool;
    FixedSizePool1 m_internalPool;

    T_IChild*   m_root;

public:
    typedef StatusInt (*Processor) (ETYPE const*, void*);

    /*=================================================================================**//**
    * @bsiclass                                                         KeithBentley    01/01
    +===============+===============+===============+===============+===============+======*/
    struct Iterator
        {
        T_LeafNode const*   m_currNode;
        int                 m_index;

        Iterator () {m_currNode = NULL; m_index = 0;}
        Iterator (T_LeafNode const* node, int index) {m_currNode = node; m_index = index;}

        ETYPE const* Entry() {return m_currNode ? m_currNode->GetEntry(m_index) : NULL;}
        Iterator Next ()
            {
            if (NULL == m_currNode)
                return  Iterator();

            T_LeafNode const*  nextNode  = m_currNode;
            int                nextIndex = m_index+1;

            if (nextIndex >= m_currNode->GetCount())
                {
                nextIndex = 0;
                nextNode  = m_currNode->GetParent()->NextSibling (m_currNode);
                }
            return  Iterator (nextNode, nextIndex);
            }
        };

    KeyTree ()
        {
        m_leafPool.SetSize (sizeof(T_LeafNode), 4);
        m_internalPool.SetSize (sizeof(T_InternalNode), 4);
        m_root  = NULL;
        }
    virtual ~KeyTree () {Empty();}

    void               IncreaseRange (T_KeyRange const* pNewRange){}
    void               RemoveKey(KTYPE   key)          {}
    bool               ContainsKey (KTYPE key) const   {return (NULL != Get (key));}
    T_IChild*          GetRoot()                       {return m_root;}
    T_LeafNode const*  NextSibling (T_IChild const* curr) const {return  NULL;}

/*---------------------------------------------------------------------------------**//**
* Adds a new entry to this tree.
* @param        entry   the entry to add
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
void Add (ETYPE* entry)
    {
    if (NULL == entry)
        return;

    if (NULL == m_root)
        m_root = NewLeafNode (this);

    m_root->Add (entry);
    }

/*---------------------------------------------------------------------------------**//**
* Empty the entire contents of this tree.
* @see          #IsEmpty
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
void Empty()
    {
    m_leafPool.purge_memory_and_reinitialize();
    m_internalPool.purge_memory_and_reinitialize();
    m_root = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Tests if this tree is empty.
* @see          #Empty
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsEmpty() const
    {
    return  (NULL == m_root) ? true : m_root->IsEmpty();
    }

/*---------------------------------------------------------------------------------**//**
* Returns the first entry in this tree that has the specified key.
* @param        key     the key for which to return the entry.
* @return       the entry or <code>null</code> if no entry with the specified key is found.
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
ETYPE* Get (KTYPE key) const
    {
    if ((NULL == m_root) || !m_root->ContainsKey (key))
        return NULL;

    return  m_root->Find (key, NULL, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Return the total number of entries in this tree.
* @return       the entry count
* @bsimethod                                                    ShaunSewall     03/99
+---------------+---------------+---------------+---------------+---------------+------*/
int GetCount ()
    {
    return  (NULL == m_root) ? 0 : m_root->CountEntries ();
    }

/*---------------------------------------------------------------------------------**//**
* Removes the first entry that has the specified key.
* @param        key     the key for which to remove the entry.
* @param        copyOfEntry if not NULL, and if the key exists, the entry is copied here before it is removed.
* @return       SUCCESS if the key was found and removed, ERROR if it is not in the tree
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Remove (KTYPE key, ETYPE* copyOfEntry)
    {
    if (NULL != m_root)
        {
        T_LeafNode*  leaf;
        int          index;

        if (NULL != m_root->Find (key, &leaf, &index))
            {
            leaf->RemoveEntry (index, copyOfEntry);
            return  SUCCESS;
            }
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* Removes the entry that has the specified key and value
* @param        key     the key for which to remove the entry.
* @param        copyOfEntry if not NULL, and if the key exists, the entry is copied here before it is removed.
* @return       SUCCESS if the key was found and removed, ERROR if it is not in the tree
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Remove (ETYPE* searchFor, ETYPE* copyOfEntry)
    {
    if (NULL != m_root)
        {
        T_LeafNode*  leaf;
        int          index;

        if (NULL != m_root->FindEntry (searchFor, &leaf, &index))
            {
            leaf->RemoveEntry (index, copyOfEntry);
            return  SUCCESS;
            }
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
T_InternalNode* NewInternalNode (T_IParent* parent)
    {
    return new ((T_InternalNode*) m_internalPool.malloc ()) T_InternalNode (parent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
T_LeafNode* NewLeafNode (T_IParent* parent)
    {
    return new ((T_LeafNode*) m_leafPool.malloc ()) T_LeafNode (parent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
void Free (T_IChild* child, bool leaf)
    {
    if (leaf)
        m_leafPool.free (child);
    else
        m_internalPool.free (child);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
void AddChildNode (T_IChild* newNode)
    {
    T_KeyRange          range;
    T_IChild*           currentRoot = m_root;
    T_InternalNode*     newRoot;

    newRoot = new ((T_InternalNode*) m_internalPool.malloc ()) T_InternalNode (this);

    newRoot->AddChildNode (newNode);

    newNode->GetExactNodeRange (&range);
    newRoot->SetNodeRange (&range);

    if (NULL != currentRoot)
        newRoot->AddChildNode (currentRoot);

    m_root = newRoot;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ShaunSewall     02/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DropChildNode (T_IChild* childNode, bool)
    {
    if (m_root == childNode)
        m_root = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    01/01
+---------------+---------------+---------------+---------------+---------------+------*/
void Dump ()
    {
    if (m_root)
        m_root->Dump(0);
    }

/*---------------------------------------------------------------------------------**//**
* Invoke a callback on each entry in the tree
* @bsimethod                                                    SamWilson       04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Process (Processor proc, void* pArg) const
    {
    return  (m_root) ? m_root->Process (NULL, proc, pArg) : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Invoke a callback on each entry in the tree with a key between low and high (inclusive)
* @bsimethod                                                    SamWilson       04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Process (KTYPE low, KTYPE high, Processor proc, void* pArg) const
    {
    T_KeyRange  tRange;

    tRange.Init(low, high);
    return  (m_root) ? m_root->Process (&tRange, proc, pArg) : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
Iterator Find (KTYPE key)
    {
    T_LeafNode*  leaf;
    int          index;

    if (NULL == m_root || (NULL == m_root->Find (key, &leaf, &index)))
        return  Iterator();

    return Iterator (leaf, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
Iterator FirstEntry ()
    {
    return  Iterator ((m_root ? m_root->GetFirstNode() : NULL), 0);
    }
};


/*=================================================================================**//**
* A simplified version of KeyTree.
* Simplifying assumptions:
*   the Key type (KTYPE) is a uint32_t in the range 0-ffffffff.
*   the Value type (VTYPE) is assumed to have the following characteristics
*       can be passed by value
*       0 means a Nil value
* @bsiclass                                                     SamWilson       12/01
+===============+===============+===============+===============+===============+======*/
template <class VTYPE, class KTYPE>
struct          UInt32KeyTree
    {
    struct      Node
        {
    public:
        KTYPE   m_key;
        VTYPE   m_value;

    private:
        Node& operator=(const Node&)        { return *this; } // NO!

    public:
        Node (KTYPE k, VTYPE v) : m_key (k), m_value (v) {;}

        //  required of every KeyTree node:
        static uint32_t GetMinKey() {return 0;}
        static uint32_t GetMaxKey() {return UINT32_MAX;}
        uint32_t GetKey ()       const       { return (uint32_t)m_key; }
        VTYPE   GetValue ()     const       { return m_value; }

        //  Some sugar *** EXPERIMENT ***
        operator VTYPE () { return GetValue (); }
        operator KTYPE () { return this->getKey (); }

        //  Helper functions for UInt32KeyTree deleteXXX utility methods
        static StatusInt deleteValue1 (const Node*pEV, void*)
            {
            if (pEV->m_value)
                delete pEV->m_value;
            return SUCCESS;
            }

        static StatusInt deleteKey1 (const Node*pEV, void*)
            {
            if (pEV->m_key)
                delete pEV->m_key;
            return SUCCESS;
            }

        static StatusInt deleteBoth (const Node*pEV, void*)
            {
            if (pEV->m_value)
                delete pEV->m_value;
            if (pEV->m_key)
                delete pEV->m_key;
            return SUCCESS;
            }
        };

private:
    KeyTree <Node, uint32_t>                  m_tree;

    UInt32KeyTree (const UInt32KeyTree&)              {;}
    UInt32KeyTree& operator=(const UInt32KeyTree&)  { return *this; } // NO

public:
    UInt32KeyTree ()     {}

    //  Utility methods for cases when value and/or key is a new'd pointer
    void deleteValues ()                    {m_tree.Process (Node::deleteValue1, NULL);}
    void deleteKeys ()                      {m_tree.Process (Node::deleteKey1, NULL);}
    void deleteKeysAndValues ()             {m_tree.Process (Node::deleteBoth1, NULL);}

    //  Tree access
    void        Add (KTYPE key, VTYPE value)
        {
        BeAssert (!m_tree.ContainsKey ((uint32_t)key)); // (There should never be dups!)
        Node newNode (key, value);
        m_tree.Add (&newNode);      // KeyTree makes a copy
        }

    VTYPE       Find (KTYPE key)
        {
        Node *node = m_tree.Get ((uint32_t)key);
        return node? node->GetValue (): 0;
        }

    typedef     StatusInt (*Processor) (const Node*, void*);
    StatusInt   Process (Processor proc, void *arg)
        {
        return m_tree.Process (proc, arg);
        }
    };

static int const        _LT_ = -1;
static int const        _EQ_ =  0;
static int const        _GT_ =  1;

static WChar const* MINKEY = (WChar const*) INTPTR_MIN;     // value that is guaranteed to be smaller than all pointers, including NULL
static WChar const* MAXKEY = (WChar const*) INTPTR_MAX;     // value that is guaranteed to be greater than all real pointers

/*=================================================================================**//**
* MSWCharIKey - key that ignores case
* @bsiclass                                                     KeithBentley    10/00
+===============+===============+===============+===============+===============+======*/
struct MSWCharIKey
{
        WChar*        m_key;

public:
    MSWCharIKey ()                   {m_key = NULL;}
    MSWCharIKey (WChar const* key) {m_key = (WChar*) key;}
    WChar const* GetValue () const {return m_key;}

    inline int compare (MSWCharIKey other) const
        {
        if (other.m_key == m_key)   return  _EQ_;   // must be first
        if (other.m_key == MINKEY)  return  _GT_;
        if (other.m_key == MAXKEY)  return  _LT_;
        if (m_key == MINKEY)        return  _LT_;
        if (m_key == MAXKEY)        return  _GT_;

        return  BeStringUtilities::Wcsicmp (m_key, other.m_key);
        }

    bool operator>  (MSWCharIKey other) const {return   compare (other) >  _EQ_;}
    bool operator<  (MSWCharIKey other) const {return   compare (other) <  _EQ_;}
    bool operator>= (MSWCharIKey other) const {return   compare (other) >= _EQ_;}
    bool operator<= (MSWCharIKey other) const {return   compare (other) <= _EQ_;}
    bool operator== (MSWCharIKey other) const {return   compare (other) == _EQ_;}
    WChar const* operator- (MSWCharIKey other) const
        {
        int     cmp = compare (other);
        if (cmp < 0)    return  MINKEY;
        if (cmp > 0)    return  MAXKEY;
        return  0;
        }
};

/*=================================================================================**//**
* MSWCharKey - key that is case sensitive
* @bsiclass                                                     KeithBentley    10/00
+===============+===============+===============+===============+===============+======*/
struct MSWCharKey
{
        WChar*        m_key;

public:
    MSWCharKey ()                   {m_key = NULL;}
    MSWCharKey (WChar const* key) {m_key = (WChar*) key;}
    WChar const* GetValue () const {return m_key;}

    inline int compare (MSWCharKey other) const
        {
        if (other.m_key == m_key)   return  _EQ_;   // must be first
        if (other.m_key == MINKEY)  return  _GT_;
        if (other.m_key == MAXKEY)  return  _LT_;
        if (m_key == MINKEY)        return  _LT_;
        if (m_key == MAXKEY)        return  _GT_;

        return  wcscmp (m_key, other.m_key);
        }

    bool operator>  (MSWCharKey other) const {return    compare (other) >  _EQ_;}
    bool operator<  (MSWCharKey other) const {return    compare (other) <  _EQ_;}
    bool operator>= (MSWCharKey other) const {return    compare (other) >= _EQ_;}
    bool operator<= (MSWCharKey other) const {return    compare (other) <= _EQ_;}
    bool operator== (MSWCharKey other) const {return    compare (other) == _EQ_;}
    WChar const* operator- (MSWCharKey other) const
        {
        int     cmp = compare (other);
        if (cmp < 0)    return  MINKEY;
        if (cmp > 0)    return  MAXKEY;
        return  0;
        }
};

static char const* MINCKEY = (char const*) INTPTR_MIN;     // value that is guaranteed to be smaller than all pointers, including NULL
static char const* MAXCKEY = (char const*) INTPTR_MAX;     // value that is guaranteed to be greater than all real pointers
/*=================================================================================**//**
* MSCharIKey - key that ignores case
* @bsiclass                                                     KeithBentley    10/00
+===============+===============+===============+===============+===============+======*/
struct MSCharIKey
{
        char*        m_key;

public:
    MSCharIKey ()                   {m_key = NULL;}
    MSCharIKey (char const* key) {m_key = (char*) key;}
    char const* GetValue () const {return m_key;}

    inline int compare (MSCharIKey other) const
        {
        if (other.m_key == m_key)   return  _EQ_;   // must be first
        if (other.m_key == MINCKEY)  return  _GT_;
        if (other.m_key == MAXCKEY)  return  _LT_;
        if (m_key == MINCKEY)        return  _LT_;
        if (m_key == MAXCKEY)        return  _GT_;

        return  BeStringUtilities::Stricmp (m_key, other.m_key);
        }

    bool operator>  (MSCharIKey other) const {return   compare (other) >  _EQ_;}
    bool operator<  (MSCharIKey other) const {return   compare (other) <  _EQ_;}
    bool operator>= (MSCharIKey other) const {return   compare (other) >= _EQ_;}
    bool operator<= (MSCharIKey other) const {return   compare (other) <= _EQ_;}
    bool operator== (MSCharIKey other) const {return   compare (other) == _EQ_;}
    char const* operator- (MSCharIKey other) const
        {
        int     cmp = compare (other);
        if (cmp < 0)    return  MINCKEY;
        if (cmp > 0)    return  MAXCKEY;
        return  0;
        }
};

END_BENTLEY_NAMESPACE
