/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/PersistentElementPath.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define ROOT_MODEL_ID_INDUCER ((UInt64)-2)
#define HOME_MODEL_ID_INDUCER ((UInt64)-3)

inline bool IsRootModelIdInducer (UInt64 e) {return ROOT_MODEL_ID_INDUCER == e;}
inline bool IsHomeModelIdInducer (UInt64 e) {return HOME_MODEL_ID_INDUCER == e;}

typedef ElementRefP (T_FindElemByID) (DgnModelP, ElementId);

struct PepReader;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static ElementRefP findLiveElemById (DgnModelP cache, ElementId eid)
    {
    ElementRefP ref = cache->FindElementById (eid);
    return (NULL != ref && !ref->IsDeleted()) ? ref : NULL;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    SamWilson                       04/01
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnModelP getCache (DgnProjectP file, DgnModelId mid)
    {
    if (NULL == file)
        {
        BeAssert (false);
        return NULL;
        }

    if (mid == (DgnModelId)-1)
        return file->Models().GetDictionaryModel ();

    return file->Models().FindModelById (mid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP    LoadModelById (DgnModelId mid, DgnModelP homeModel)
    {
    if (homeModel->GetModelId() == mid)
        return homeModel;

    return getCache (&homeModel->GetDgnProject(), mid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
static int      pushModelID (T_ElementIdInternalVector& ids, DgnModelId mid, UInt64 inducer)
    {
    ids.push_back (inducer);
    int i = static_cast<int>(ids.size());
    ids.push_back (mid.GetValue()); // UInt64
    return i;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt parseModelID (DgnModelId& mid, T_ElementIdInternalVector::iterator& it, UInt64 inducer, bool movePastMid = true)
    {
    if (inducer != *it)
        return ERROR;

    ++it;
    UInt64 midValue = *it;

    if (movePastMid)
        ++it;

    mid = (DgnModelId)midValue;

    return SUCCESS;
    }


/*=================================================================================**//**
*
* Used by PersistentElementPath to store ElementIds.
*
*   A DisplayPath is an array of ElementIds, plus a home DgnModel. The DgnModel
*   is not stored. The array of ElementIds is stored in a series of IdBlocks.
*
*   A single IdBlock stores up to 4 ElementIds. A single IdBlock is defined as follows:
*
*       flags [e [e [e [e]]]]
*
*       where:  flags       ssssrlcc
*                   where:  s    IsSmallId?
*                           r    does block contain remapKeys, instead of ElementIDs?
*                           l    IsLast?
*                           c    count of element IDs to follow
*               e           element ID: 4 bytes if flags.IsSmallId(i) == true; else 8 bytes
*
*   Note: in order to use a 2-bit field to capture a count of up to 4, we define 0 to mean 4
*           and we disallow a count of 0.
*
*   To capture more than 4 ElementIds, IdBlocks are stored contiguously. The IsLast flag
*   on a block indicates if another block follows:
*
*       IdBlock{F}IdBlock{F}...IdBlock{T}
*
*
* @bsiclass                                                     Sam.Wilson      12/2004
+===============+===============+===============+===============+===============+======*/
struct IdBlock
    {
    UInt8   m_flags;
    // ... up to 4 small or large IDs

    IdBlock () {m_flags=0;}

    //  Is the ith ElementId in this block a 32-bit value?
    bool    IsSmallId(UInt8 i) const            {return ( (m_flags >> 4) & (1<<i) ) != 0;}
    void    SetSmallId(UInt8 i)                 {          m_flags |=      (1<<i) << 4; }

    //  Is this the last IdBlock block in the chain? Else, there's another to follow
    bool    IsLast() const                      {return (m_flags &  4) != 0;}
    void    SetLast()                           {        m_flags |= 4;}

    //  Is this the last IdBlock block in the chain? Else, there's another to follow
    bool    IsRemapKeys() const                 {return (m_flags &  8) != 0;}
    void    SetRemapKeys()                      {        m_flags |= 8;}

    //  Get the number of ElementIds to follow
    UInt8   GetCount () const
        {
        UInt8 c = m_flags &  3;
        if (0 ==c)          // 00 => 4
            return 4;
        return c;
        }

    void    SetCount (UInt8 c)
        {
        if (0 == c)
            {
            BeAssert (false);
            return;
            }

        m_flags &= ~3;
        if (c < 4)          // 00 => 4
            m_flags |= c;
        }

    StatusInt CheckIntegrity () const
        {
        UInt8 c = GetCount ();

        if (0==c && !IsLast())
            return ERROR;

        UInt8 uflags = 0;
        for (UInt8 i=0; i<c; ++i)
            uflags |= (1<<i);       // one flag for each used slot

        UInt8 llll = m_flags>>4;    // one flag for each short ID
        if (llll & ~uflags)         // any flag set for an unused slot?
            return ERROR;

        return SUCCESS;
        }

    UInt64 GetElementId (UInt8 j, DataInternalizer& source)
        {
        UInt64   e;
        if (!IsSmallId (j))
            {
            source.get (&e);
            }
        else
            {
            UInt32  e32;
            source.get (&e32);
            e = e32;
            }
        return e;
        }

    void    PutElementId (UInt64 e, UInt8 j, DataExternalizer* sink)
        {
        if (!IsSmallId (j))
            sink->put (e);
        else
            {
            BeAssert ((0xffffffff00000000LL & e) == 0);
            sink->put ((UInt32)e);
            }
        }

static UInt32  SizeofChain (UInt8 const*, UInt32 maxBytes);

static UInt8* Alloc (UInt32 sz)
        {
        return (UInt8*) malloc (sz);
        }

static void  Free (UInt8* block)
        {
        free (block);
        }

static UInt8* Copy (UInt8 const* source, UInt32 sz)
        {
        if (0 == sz)            // NB: if source has 0 bytes, we must return NULL.
            return NULL;        //      Otherwise, Alloc will return a couple of bytes of unitialized junk.
        UInt8* b = Alloc (sz);
        memcpy (b, source, sz);
        return b;
        }

static UInt8* Copy (UInt8 const* source)
        {
        return Copy (source, SizeofChain(source,0));
        }

static void Dump (Utf8String&, UInt8*, UInt32 maxBytes);

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
// static
UInt32          IdBlock::SizeofChain
(
UInt8 const*    base,
UInt32          maxBytes
)
    {
    IdBlock const* block = (IdBlock*) base;
    if (NULL == block)
        return 0;

    bool wasLast;
    do  {
        if (maxBytes && ((UInt8*)block - base) >= (int)maxBytes)
            {
            BeAssert (false);
            return 0;
            }

        if (block->CheckIntegrity() != SUCCESS)
            {
            BeAssert (false);
            return 0;
            }

        UInt16 sz1 = sizeof (block->m_flags);
        if (1 == maxBytes) // empty persistent path
            return sz1;

        UInt8 c = block->GetCount ();

        for (UInt8 j = 0; j < c; ++j)
            {
            if (block->IsSmallId (j))
                sz1 += sizeof (UInt32);
            else
                sz1 += sizeof (ElementId);
            }

        wasLast = block->IsLast ();

        block = (IdBlock*) ((UInt8*)block + sz1);
        }
    while (!wasLast);

    return static_cast<UInt32>(reinterpret_cast<UInt8 const*>(block) - base);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
// static
void IdBlock::Dump
(
Utf8String&     str,
UInt8*          storage,
UInt32          maxBytes
)
    {
    str.append ("PersistentElementPath {");

    DataInternalizer source (storage, SizeofChain (storage, maxBytes));

    while (true)
        {
        IdBlock block;
        source.get (&block.m_flags);

        str.append ("[");
        if (block.IsLast ())
            str.append ("last ");
        if (block.IsRemapKeys())
            str.append ("remapKeys ");
        
        str.append (Utf8PrintfString ("%d:", block.GetCount()));

        for (UInt8 j = 0; j < block.GetCount(); ++j)
            {
            UInt64 e = block.GetElementId (j, source);
            if (IsRootModelIdInducer(e))
                str.append ("(RootModelId)");
            else if (IsHomeModelIdInducer (e))
                str.append ("(HomeModelId)");
            else
                {
                char const* s = block.IsSmallId(j)? "(UInt32)" : "";
                str.append (Utf8PrintfString ("%hs%llu ", s, e));
                }
            }

        str.append ("]");

        if (block.IsLast ())
            break;
        }

    str.append ("}");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
// static
static UInt8*  storageFromIds
(
T_ElementIdInternalVector&      ids,
bool                    isRemapKeys
)
    {
    DataExternalizer sink;

    UInt32 nPath = static_cast<UInt32>(ids.size ());

    //  ---------------------------------------------------------
    //  Store the ElementIds in the complete path in compressed form, in blocks of 4.
    UInt32 nPathRemaining = nPath;
    UInt32 nblks = (nPath + 3) / 4;

    for (UInt32 i = 0; i < nblks; ++i)
        {
        //  --------------------------------
        //  Store flags
        IdBlock blockFlags;

        UInt8 c = (nPathRemaining < 4)? (UInt8)nPathRemaining: 4;
        blockFlags.SetCount (c);

        nPathRemaining -= c;
        if (nPathRemaining == 0)
            blockFlags.SetLast ();

        if (isRemapKeys)
            blockFlags.SetRemapKeys ();

        for (UInt8 j = 0; j < c; ++j)           // detect small IDs
            {
            UInt64  e = ids[i*4 + j];
            if (0 == (e & 0xffffffff00000000LL))
                blockFlags.SetSmallId (j);
            }

        sink.put (blockFlags.m_flags);

        //  --------------------------------
        //  Store Ids
        for (UInt8 j = 0; j < c; ++j)
            {
            blockFlags.PutElementId (ids[i*4 + j], j, &sink);
            }
        }

    //  Capture streamed data
    return IdBlock::Copy (sink.getBuf(), static_cast<UInt16>(sink.getBytesWritten()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       idsFromStorage
(
T_ElementIdInternalVector*      ids,
UInt8 const*            storage,
UInt32                  maxBytes,
bool*                   isRemapKeys
)
    {
    if (NULL == storage)
        return ERROR;

    DataInternalizer source (storage, IdBlock::SizeofChain(storage,maxBytes));
    IdBlock block;
    source.get (&block.m_flags);

    while (true)
        {
        if (block.CheckIntegrity() != SUCCESS)
            {
            BeAssert (false);
            return ERROR;
            }

        UInt8   c = block.GetCount ();

        if (isRemapKeys)
            *isRemapKeys = block.IsRemapKeys ();
        else
        if (block.IsRemapKeys ())
            return ERROR;

        for (UInt8 j=0; j<c; ++j)
            ids->push_back (block.GetElementId (j, source));

        if (block.IsLast ())
            return SUCCESS;

        source.get (&block.m_flags);
        }
    while (source.getRemainingSize() >= sizeof (block.m_flags));

    BeAssert (false);     // we should never get here, but should exit after finishing the last block.
    return ERROR;
    }

// -------------------------------------------------------------------------------------
//  NB: A persistent path will be of the form:
//                   IMPORTER ONLY
//                   VVVVVVVVVVVVVVV
//       [  [-3 MID] refelementid* ] [-2 MID] elementid+
//          \______/ \___________/   \______/ \________/
//            HMID     ref prefix      RMID    element path
//
//  HMID identifies the home model within the file identified by the supplied home model or home cache.
//  Note that there will only be an HMID if there is also a ref prefix.
//  refelementid* is zero or more element IDs, identifying reference attachments
//      refelementid[0] is a reference attachment element in home model
//      refelementid[i] is a reference attachment element in reference i-1
//      refelementid[n] is the default root model
//  RMID identifies the root model within the file identified by refelementid[n]
//  elementid+ is one or more element IDs, identifying the root element within the root model.
//      Examples of multiple element IDs:
//      featuremodelheader ... featuremodelcomponent
//      scInst0 scDefHdr0 ... scInstm scDefHdrm element
// -------------------------------------------------------------------------------------
struct PepWriter
    {
    DgnModelId          m_homeModelID;
    T_ElementIdInternalVector   m_prefix;
    T_ElementIdInternalVector   m_path;
    DgnModelId          m_rootModelID;
    DgnModelP           m_homeCache;

    PepWriter () : m_homeCache(NULL) {;}

    void    SetPath (PepReader const&);

    void    SetOptionalHomeModelId (DgnModelId mid) {m_homeModelID=mid;}

    void    SetOptionalRootModelId (DgnModelId mid) {m_rootModelID=mid;}

    StatusInt SetPath (DisplayPathCP dp)
        {
        BeAssert (m_path.empty ());
        //  ---------------------------------------------------------
        //  append the path within the root model to the root element
        //  NB: Save everything up to and including the cursor element; ignore everything after the cursor.
        for (int i=0; i<=dp->GetCursorIndex(); ++i)
            {
            ElementRefP ref = dp->GetPathElem (i);

            if (NULL == ref)
                m_path.push_back (0);
            else
                m_path.push_back (ref->GetElementId().GetValue());
            }
        return SUCCESS;
        }

    void    SetElement (UInt64 eid)
        {
        BeAssert (m_path.empty ());
        m_path.push_back (eid);
        }

    void    SetReferenceElementTarget (UInt64 eid, DgnModelP homeDgnModel)
        {
        BeAssert (m_path.empty ());
        // ***TRICKY: Adding the option model ID allows me to put a reference attachment element in the path part of the PEP.
        //              If I were to push type 100 all by itself, it would be interpreted as the reference attachment prefix.
        SetOptionalRootModelId (homeDgnModel->GetModelId());
        m_path.push_back (eid);
        }

    UInt8*  ToStorage ()
        {
        T_ElementIdInternalVector ids;

        if (m_homeModelID.IsValid())   // See Note "Model id inducer and DICTIONARY_MODEL_ID"
            pushModelID (ids, m_homeModelID, HOME_MODEL_ID_INDUCER);

        ids.insert (ids.end(), m_prefix.begin(), m_prefix.end());

        if (m_rootModelID.IsValid())   // See Note "Model id inducer and DICTIONARY_MODEL_ID"
            pushModelID (ids, m_rootModelID, ROOT_MODEL_ID_INDUCER);

        ids.insert (ids.end(), m_path.begin(), m_path.end());

        return storageFromIds (ids, false);
        }
    };


/*=================================================================================**//***
* @bsiclass                                                     Sam.Wilson      10/2007
+===============+===============+===============+===============+===============+======*/
struct PepReader
{
    DgnModelP        m_homeModel;  // Where the path starts. The HMID or first reference attachment element must be found here.
    DgnModelP           m_currentCache; // The cache where the next ElementId will be read.
    DisplayPath         m_dp;         // The path of elements resolved. Its root model will be the model ref of the
                                      // last element resolved. The root model may be NULL in case we used an optional ModelId to reach the root cache.
    T_StdElementRefSet* m_refs_;      // Optionally collect ElementRefs that we resolve, including both reference prefix and element path
    PersistentElementPath::PathProcessor* m_processor;

    StatusInt           m_readError;
    T_ElementIdInternalVector   m_ids;
    T_ElementIdInternalVector::iterator m_it;
    T_ElementIdInternalVector::iterator m_itEnd;

    T_FindElemByID*     m_findElemByIdFunc;
    bool                m_fillModels;

    PepReader (UInt8* storage, T_FindElemByID* f = findLiveElemById)
        :
        m_homeModel (NULL),
        m_currentCache (NULL),
        m_refs_ (NULL),
        m_findElemByIdFunc (f),
        m_fillModels (false),
        m_processor (NULL)
        {
        m_readError = idsFromStorage (&m_ids, storage, 0, NULL);
        if (m_readError != SUCCESS)
            return;
        m_it = m_ids.begin ();
        m_itEnd = m_ids.end ();
        }

    bool IsEmpty () const
        {
        return m_ids.empty ();
        }

    void SetElementRefCollector (T_StdElementRefSet* r) {m_refs_ = r;}
    void CollectElementRef (ElementRefP ref) {if (m_refs_) m_refs_->insert (ref);}

    /*---------------------------------------------------------------------------------**//**
    * Record the home model, where we should read the FIRST model or element in the path.
    *
    * Home model is the default root model. So, this function also calls SetCurentModel (hm).
    * @bsimethod                                    Sam.Wilson                      08/2008
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetHomeModel (DgnModelP hm)
        {
        m_homeModel = hm;
        SetCurrentModel (hm, m_homeModel);
        }

    /*---------------------------------------------------------------------------------**//**
    * Set the model where we should read the NEXT model or element in the path.
    * This also sets the DisplayPath root model, in case this is the last model reference in the path.
    * @bsimethod                                    Sam.Wilson                      09/2008
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetCurrentModel (DgnModelP rm, DgnModelP rc)
        {
        m_currentCache = rc;
        m_dp.SetRoot (rm);
        }

    /*---------------------------------------------------------------------------------**//**
    * Get the model we were should read the NEXT model or element in the path.
    * May be null if we used an optional home or root model ref.
    * See GetCurrentCache.
    * @bsimethod                                    Sam.Wilson                      09/2008
    +---------------+---------------+---------------+---------------+---------------+------*/
    DgnModelP GetCurrentModel () const
        {
        return m_dp.GetRoot();
        }

    /*---------------------------------------------------------------------------------**//**
    * Get the model we were should read the NEXT model or element in the path.
    * @bsimethod                                    Sam.Wilson                      09/2008
    +---------------+---------------+---------------+---------------+---------------+------*/
    DgnModelP GetCurrentCache () const {BeAssert (NULL != m_currentCache); return m_currentCache;}

    /*---------------------------------------------------------------------------------**//**
    * Get the file we were should read the NEXT model or element in the path.
    * @bsimethod                                    Sam.Wilson                      09/2008
    +---------------+---------------+---------------+---------------+---------------+------*/
    DgnProjectP GetCurrentFile () const {BeAssert (NULL != m_currentCache); return m_currentCache? &m_currentCache->GetDgnProject(): NULL;}

    /*---------------------------------------------------------------------------------**//**
    *      [  [-3 MID] refelementid* ] [-2 MID] elementid+
    *         ^
    * @bsimethod                                    Sam.Wilson                      10/2007
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusInt EvaluateOptionalHomeModel ()
        {
        BeAssert (m_homeModel != NULL);

        if (m_it == m_itEnd)
            {
            BeAssert (false);
            return ERROR;
            }

        DgnModelId mid;
        if (parseModelID (mid, m_it, HOME_MODEL_ID_INDUCER) != SUCCESS)
            return SUCCESS; // if there is no optional ModelId, that's not an error.

        DgnModelP m = LoadModelById (mid, m_homeModel);

        if (m_processor)
            m_processor->OnModelId (mid, m, GetCurrentFile());

        if (NULL == m)
/*<==*/     return ERROR;

        SetHomeModel (m);
        return SUCCESS;
        }

    bool dgnModelIsFilled (DgnModelP dgnModel, DgnPlatform::DgnModelSections sections) {return DgnModelSections::None != dgnModel->IsFilled(sections);}
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      01/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElementRefP GetElementByIDInSection (UInt64 id, DgnModelSections section)
        {
        DgnModelP dgnModel = GetCurrentCache();
        if (NULL == dgnModel)
            return NULL;

        if (!dgnModelIsFilled (dgnModel, section) /*&& !DependencyManager::GetInModelLoading ()*/)
            dgnModel->GetDgnProject().Models().FillSectionsInModel (*dgnModel, section);

        return m_findElemByIdFunc (dgnModel, ElementId (id));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      01/2009
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElementRefP GetElementByID (UInt64 id)
        {
        ElementRefP ref = m_findElemByIdFunc (GetCurrentCache(), ElementId (id));
        if (NULL != ref)
            return ref;

        if (!m_fillModels)
            return NULL;

        if ((ref = GetElementByIDInSection (id, DgnModelSections::ControlElements)) != NULL)
            return NULL;

        return GetElementByIDInSection (id, DgnModelSections::GraphicElements);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      08/2008
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElementId PeekFirstElementID ()
        {
        DgnModelId __;
        parseModelID (__, m_it, HOME_MODEL_ID_INDUCER);     // step over any/all modelID inducers
        parseModelID (__, m_it, ROOT_MODEL_ID_INDUCER);     //      "           "
        return (m_it == m_itEnd) ? ElementId() : ElementId (*m_it);                 // first real ElementId
        }

    /*---------------------------------------------------------------------------------**//**
    *       [ [-3 MID] refelementid* ] [-2 MID] elementid+
    *                                  ^
    * @bsimethod                                    Sam.Wilson                      10/2007
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusInt       EvaluateOptionalRootModel ()
        {
        BeAssert (NULL != GetCurrentFile ());

        if (m_it == m_itEnd)
            {
            BeAssert (false);
            return ERROR;
            }

        DgnModelId mid;
        if (parseModelID (mid, m_it, ROOT_MODEL_ID_INDUCER) != SUCCESS)
            return SUCCESS;

        DgnModelP cache = getCache (GetCurrentFile(), mid);

        if (m_processor)
            m_processor->OnModelId (mid, cache, GetCurrentFile());

        if (NULL == cache)
/*<==*/     return ERROR;

        SetCurrentModel (cache, cache);
        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    *        [ [-3 MID] refelementid* ] [-2 MID] elementid+
    *                                            ^
    * @bsimethod                                    Sam.Wilson                      10/2007
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusInt       EvaluateElementPath ()
        {
        BeAssert (NULL != GetCurrentCache ());

        for ( ; m_it != m_itEnd; ++m_it)
            {
            UInt64 e = *m_it;

            ElementRefP ref = GetElementByID (e);

            CollectElementRef (ref);

            if (m_processor)
                m_processor->OnElementId (ElementId (e), ref, GetCurrentCache());

            if (NULL == ref)
 /*<==*/        return ERROR;

            m_dp.PushPathElem (ref);
            }

        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      11/2004
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusInt EvaluateDisplayPath0 (DgnModelP homeModel)
        {
        SetHomeModel (homeModel);
        return !IsEmpty ()
            && EvaluateOptionalHomeModel () == SUCCESS
            && EvaluateOptionalRootModel () == SUCCESS
            && EvaluateElementPath () == SUCCESS
            ? SUCCESS: ERROR;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      10/2007
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElementRefP EvaluateElementRef (DgnModelP cache)
        {
        if (EvaluateDisplayPath0 (cache) != SUCCESS)
            return NULL;

        return m_dp.GetTailElem ();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      11/2004
    +---------------+---------------+---------------+---------------+---------------+------*/
    DisplayPathPtr EvaluateDisplayPath (DgnModelP homeModel)
        {
        if (EvaluateDisplayPath0 (homeModel) != SUCCESS)
            return NULL;

        return new DisplayPath (m_dp);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      11/2004
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElementHandle EvaluateElemHandle (DgnModelP homeModel)
        {
        if (EvaluateDisplayPath0 (homeModel) != SUCCESS)
            return ElementHandle();

        return ElementHandle(m_dp.GetTailElem());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      10/2007
    +---------------+---------------+---------------+---------------+---------------+------*/
    ElementRefP EvaluateFirstElementRef (DgnModelP cache)
        {
        // Note: We want the first ref, whatever it is. So, if there is a reference prefix, then return the first ref attachment element.
        SetHomeModel (cache);
        //  [[-3 MID] refelementid* ] [-2 MID] elementid+
        //  ^?                        ^?
        if (IsEmpty ()
         || EvaluateOptionalHomeModel () != SUCCESS
         || EvaluateOptionalRootModel () != SUCCESS)
            return NULL;

        //  [ [-3 MID] refelementid* ] [-2 MID] elementid+
        //             ^
        // or:
        //                             [-2 MID] elementid+
        //                                      ^
        return GetElementByID (*m_it);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Sam.Wilson      12/03
    +---------------+---------------+---------------+---------------+---------------+------*/
    StatusInt EvaluateRootModel (DgnProjectP& file, DgnModelId& mid, DgnModelP homeCache)
        {
        if (IsEmpty ())
            return ERROR;

        //  [[-3 MID] refelementid* ] [-2 MID] elementid+
        //  ^?                        ^?
        if (parseModelID (mid, m_it, ROOT_MODEL_ID_INDUCER) == SUCCESS)
            {
            //  ---------------------------------------------------------
            //       [-2 MID] elementid+
            //  Special case: There is no reference attachment prefix, and there is a root model ID inducer
            //  ---------------------------------------------------------
            file = &homeCache->GetDgnProject();
            return SUCCESS;
            }

        SetHomeModel (homeCache);

        //  ---------------------------------------------------------
        //  General case: There is a prefix or there is no root model ID inducer.
        //  ---------------------------------------------------------

        //       [[-3 MID] refelementid* ] [-2 MID] elementid+
        //       ^
        if (EvaluateOptionalHomeModel() != SUCCESS)
            return ERROR;

        file = &m_dp.GetRoot()->GetDgnProject();
        mid  = m_dp.GetRoot()->GetModelId();

        //       [[-3 MID] refelementid* ] [-2 MID] elementid+
        //                                 ^
        parseModelID (mid, m_it, ROOT_MODEL_ID_INDUCER);

        return SUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      10/2007
    +---------------+---------------+---------------+---------------+---------------+------*/
    void DisclosePointers (T_StdElementRefSet& refs, DgnModelP homeModel)
        {
        if (m_readError != SUCCESS)
            {
            refs.insert ((ElementRefP)NULL);
            return;
            }
        SetElementRefCollector (&refs);
        EvaluateDisplayPath0 (homeModel);
        }

}; // PepReader

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void PepWriter::SetPath (PepReader const& existingPep)
    {
    m_path = existingPep.m_ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void PersistentElementPath::GetInternalIds (T_ElementIdInternalVector& ids) const
    {
    idsFromStorage (&ids, m_storage, 0, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void PersistentElementPath::Clear ()
    {
    if (NULL != m_storage)
        {
        IdBlock::Free (m_storage);
        m_storage = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void            PersistentElementPath::Copy
(
PersistentElementPath const& cc
)
    {
    m_storage = IdBlock::Copy (cc.m_storage);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentElementPath::Load
(
DataInternalizer&   source
)
    {
    //  *** TRICKY: Compute size by scanning forward in stream. If we ever
    //              change the stream class to disallow this, we can rewrite
    //              this function to parse the blocks one at a time.
    UInt32 sz = IdBlock::SizeofChain (source.getPos(), static_cast<UInt32>(source.getRemainingSize()));
    if (0 == sz)
        {       // This only happens if stored data is corrupt or if source.getPos() is NULL
        m_storage = NULL;
        return ERROR;
        }

    if (1 == sz)
        {       // Continue to support old (bad!) way of storing empty PEP in stream all by itself
        m_storage = NULL;
        UInt8 dummy;
        source.get (&dummy);   // read the dummy persistent element path data
        }
    else
        {
        m_storage = IdBlock::Alloc (sz);
        source.get (m_storage, sz);
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentElementPath::Load (byte const* bytes, size_t nbytes)
    {
    DataInternalizer source (bytes, nbytes);
    return Load (source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void            PersistentElementPath::Store
(
DataExternalizer*  sink
)   const
    {
    if (NULL == m_storage)
        {   // Special case of an empty path: Store something that Load can read (instead of simply rejecting as invalid)
        PersistentElementPath emptyPath ((ElementRefP)NULL);
        emptyPath.Store (sink);
        return;
        }

    UInt32 sz = IdBlock::SizeofChain (m_storage, 0);
    sink->put ((UInt8*)m_storage, sz);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void            PersistentElementPath::Store (bvector<byte>& sink)   const
    {
    DataExternalizer stream;
    Store (&stream);
    sink.assign (stream.getBuf(), stream.getBuf()+stream.getBytesWritten());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       PersistentElementPath::FromUtf8String (Utf8CP str)
    {
    // string is expected to be a stream of hex values, where each value has exactly 2 hexits
    size_t nchars = strlen(str);
    UInt8* buf = (UInt8*) _alloca (nchars/2);
    size_t nbuf = 0;
    for (Utf8CP p = str; 0 !=*p && nbuf < nchars/2; p += 2)
        {
        Utf8Char hex[3];
        hex[0] = p[0];
        hex[1] = p[1];
        hex[2] = 0;
        int v;
        BE_STRING_UTILITIES_UTF8_SSCANF (hex, "%x", &v);         // a 2-hexit value will be at most 0xff (255)
        buf[nbuf++] = (UInt8) (v & 0xFF);        // so this will always fit in a UInt8
        }
    DataInternalizer source (buf, nbuf);
    return Load (source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      PersistentElementPath::ToUtf8String () const
    {
    DataExternalizer sink;
    Store (&sink);

    size_t sz = sink.getBytesWritten ();
    Utf8P p = (Utf8P) _alloca ((sz*2 + 1)*(sizeof(Utf8Char)));

    UInt8 const* buf = sink.getBuf ();
    Utf8P pnext = p;
    for (size_t i=0; i<sz; ++i)
        {
        BeStringUtilities::Snprintf (pnext, 3, "%2.2x", buf[i]);   // make sure we generate two hexits
        pnext += 2;                         // step over two hexits
        }
    return p;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool PersistentElementPath::IsEmpty () const
    {
    return (NULL == m_storage);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementPath::~PersistentElementPath ()
    {
    Clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementPath& PersistentElementPath::operator= (PersistentElementPath const& cc)
    {
    Clear ();
    Copy (cc);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementPath::PersistentElementPath (PersistentElementPath const& cc)
    {
    Copy (cc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementPath::PersistentElementPath (DataInternalizer& source)
    {
    Load (source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementPath::PersistentElementPath ()
    {
    m_storage = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementPath::PersistentElementPath (DisplayPathCP path)
    {
    PepWriter pep;
    pep.SetPath (path);
    m_storage = pep.ToStorage ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementPath::PersistentElementPath (DgnModelP model, ElementRefP ref)
    {
    PepWriter pep;
    if (NULL == ref)
        {
        pep.SetElement (ElementId().GetValueUnchecked());
        }
    else
        {
        DisplayPath path (ref, model);
        pep.SetPath (&path);
        }
    m_storage = pep.ToStorage ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementPath::PersistentElementPath
(
DgnModelP    model,
ElementId       eid
)
    {
    PepWriter pep;
    pep.SetElement (eid.GetValue());
    m_storage = pep.ToStorage ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementPath::PersistentElementPath (ElementRefP rootElement)
    {
    PepWriter pep;

    if (NULL == rootElement)
        {
        pep.SetElement (ElementId().GetValueUnchecked());
        }
    else
        {
        pep.SetElement (rootElement->GetElementId().GetValue());
        }

    m_storage = pep.ToStorage ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementPath::PersistentElementPath (ElementId eid)
    {
    PepWriter pep;
    pep.SetElement (eid.GetValue());
    m_storage = pep.ToStorage ();
    }

/*---------------------------------------------------------------------------------**//**
* Build a PEP that will look like a QFR PEP to use in ECXARelationships
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementPath::PersistentElementPath (DgnModelId mid, ElementId eid)
    {
    PepWriter pep;
    pep.SetOptionalRootModelId (mid);
    pep.SetElement (eid.GetValue());
    m_storage = pep.ToStorage ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayPathPtr PersistentElementPath::GetDisplayPath (DgnModelP homeModel) const
    {
    PepReader pep (m_storage);
    return pep.EvaluateDisplayPath (homeModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PersistentElementPath::ProcessPath (PathProcessor& proc, DgnModelP homeModel)
    {
    PepReader pep (m_storage);
    pep.m_processor = &proc;
    return pep.EvaluateDisplayPath0 (homeModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      05/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool PersistentElementPath::DependsOnElementRef (ElementRefP root, DgnModelP homeModel) const
    {
    DisplayPathPtr p = GetDisplayPath (homeModel);
    if (NULL == p.get())
/*<*/   return false;

    return p->Contains(root);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/03
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP PersistentElementPath::EvaluateFirstElementRef (DgnModelP homeCache) const
    {
    PepReader pep (m_storage);
    return pep.EvaluateFirstElementRef (homeCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PersistentElementPath::EvaluateRootModel (DgnProjectP& file, DgnModelId& mid, DgnModelP homeCache) const
    {
    PepReader pep (m_storage);
    return pep.EvaluateRootModel (file, mid, homeCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/03
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle PersistentElementPath::EvaluateElement (DgnModelP homeModel) const
    {
    if (NULL == homeModel)
        {
        BeAssert (false && "PEP::EvaluateElement requires a starting modelref");
        return ElementHandle ();
        }
    PepReader pep (m_storage);
    return pep.EvaluateElemHandle (homeModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/03
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP PersistentElementPath::EvaluateElementRef (DgnModelP homeCache) const
    {
    PepReader pep (m_storage);
    return pep.EvaluateElementRef (homeCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/2008
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle PersistentElementPath::EvaluateElementFromHost (ElementHandleCR hostElement) const
    {
    DgnModelP homeModel = hostElement.GetDgnModelP ();
    if (NULL != homeModel)
        return EvaluateElement (homeModel);

    ElementRefP ref = hostElement.GetElementRef ();
    if (NULL != ref)
        {
        return ElementHandle (EvaluateElementRef (ref->GetDgnModelP ()));
        }

    return ElementHandle ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/2008
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId PersistentElementPath::GetFirstElementId () const
    {
    if (NULL == m_storage)
        return ElementId();
    PepReader pep (m_storage);
    return pep.PeekFirstElementID ();
    }

static ElementRefP findElemByID (DgnModelP model, ElementId elementID) {return (NULL != model) ? model->FindElementById (elementID) : NULL;}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool PersistentElementPath::EqualElementRef (ElementRefP refToMatch, DgnModelP homeCache) const
    {
    PepReader pep (m_storage, findElemByID);   // (find root even if it's deleted)
    ElementRefP root = pep.EvaluateElementRef (homeCache);
    return root == refToMatch;
    }

/*---------------------------------------------------------------------------------**//**
    If the XAttribute contains references to elements, the handler should report
    those other elements.
* @param    refs    IN  where to store root ElementRefs
* @param    xa      IN  the XAttribute to check for references
* @bsimethod                                    Sam.Wilson                      12/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void            PersistentElementPath::DisclosePointers
(
T_StdElementRefSet*     refs,
DgnModelP            homeModel
)
    {
    PepReader pep (m_storage, findElemByID);
    pep.DisclosePointers (*refs, homeModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PersistentElementPath::Dump () const
    {
    Utf8String str;
    if (NULL != m_storage)
        IdBlock::Dump(str, m_storage, 0);
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PersistentElementPath::IsExactMatch (PersistentElementPath const& rhs) const
    {
    UInt32 sz = IdBlock::SizeofChain (m_storage, 0);
    UInt32 rhsSz = IdBlock::SizeofChain (rhs.m_storage, 0);

    if (sz != rhsSz)
        return false;

    return (0 == memcmp(m_storage, rhs.m_storage, (size_t)rhsSz));
    }
