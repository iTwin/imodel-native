/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/XGraphicsCache.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt dropSymbolInstance (EditElementHandleR eh, ElementRefP instanceRef, XGraphicsSymbolR symbol)
    {
    if (eh.GetElementRef() == instanceRef)
        {
        StatusInt               status;
        XGraphicsContainer      instanceContainer;

        if (SUCCESS == (status = instanceContainer.ExtractFromElement (eh)))
            status = instanceContainer.DropSymbolInstance (eh, symbol);

        return status;
        }

    return ERROR;
    }
#endif

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      04/09
+===============+===============+===============+===============+===============+======*/
struct XGraphicsSymbolCacheAppData : DbAppData
{
    XGraphicsSymbolCache       m_symbolCache;

    static DbAppData::Key const& GetKey() {static DbAppData::Key s_key; return s_key;}
    virtual void            _OnCleanup (BeSQLiteDbR host) override {delete this;}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      07/05
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsSymbolCacheAppData (DgnProjectR dgnProject) : m_symbolCache (dgnProject)  { }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsSymbolCache::XGraphicsSymbolCache (DgnProjectR dgnProject) : m_dgnProject (dgnProject), m_isDirty (false)
    {
    //  The MicroStation implementation loads all of the symbols here. We never want all of the symbols loaded except
    //  when creating new symbols.  Therefore, we don't do anything here.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     04/13
//---------------------------------------------------------------------------------------
void XGraphicsSymbolCache::CountSymbolDependents (DgnModelR model)
    {
    PersistentElementRefListIterator iter;
    DgnModelId clientModelId = model.GetModelId();

    for (PersistentElementRefP el = iter.GetFirstElementRef (model.GetGraphicElementsP()); NULL != el; el = iter.GetNextElementRef())
        {
        T_XGraphicsSymbolIds ids;
        XGraphicsSymbolCache::ExtractSymbolIds (ids, ElementHandle (el));

        if (ids.size() == 0)
            continue;
        BeAssert(ids.size() == 1);
        ElementId id = el->GetElementId();
        for (auto const&symbolIter : ids)
            {
            bpair<BeRepositoryBasedId, SymbolUseMapEntry> pairToInsert(symbolIter, SymbolUseMapEntry(clientModelId, id));
            auto result = m_dependentMap.insert(pairToInsert);
            if (result.second == false)
                result.first->second.m_nUses += 1;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
size_t XGraphicsSymbolCache::GetXGraphicsSymbolDependentCount (DgnModelId&dgnModelId, BeRepositoryBasedId&clientId, bool&clientIsStamp, BeRepositoryBasedId const& symbolId)
    {
    auto iter = m_dependentMap.find (symbolId);
    if (m_dependentMap.end() == iter)
        {
        BeAssert(false && "Have a symbol with 0 uses");
        return 0;
        }

    clientIsStamp = false;  //  not supporting nested symbols for now
    dgnModelId = iter->second.m_firstUseModelId;
    clientId = iter->second.m_firstUseEleId;

    return iter->second.m_nUses;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    02/2014
//---------------------------------------------------------------------------------------
void XGraphicsSymbolCache::AddSymbolToTree(XGraphicsSymbolP symbol)
    {
    auto treeIter = m_symbolsTree.find (symbol->GetDataSize());
    if (m_symbolsTree.end() == treeIter)
        treeIter = m_symbolsTree.insert (make_bpair (symbol->GetDataSize(), bvector <XGraphicsSymbolP> ())).first;

    treeIter->second.push_back (symbol);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     06/13
//---------------------------------------------------------------------------------------
void XGraphicsSymbolCache::RemoveSymbolFromTree(XGraphicsSymbolP symbol)
    {
    auto treeIter = m_symbolsTree.find (symbol->GetDataSize());
    if (m_symbolsTree.end() != treeIter)
        {
        for (bvector <XGraphicsSymbolP>::iterator iter = treeIter->second.begin(); treeIter->second.end() != iter; ++iter)
            {
            if (*iter != symbol)
                continue;
            treeIter->second.erase (iter);
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void XGraphicsSymbolCache::Compress (DgnModelR model, bool expandSingleInstances)
    {
    size_t                  minCompressDependents = expandSingleInstances ? 1 : 0;

    //  WARNING: m_symbols is not really the list of all symbols.  It is the list of symbols that have not 
    //  been excluded for additional consideration.  A step is eliminated for additional consideration if
    //  -- it has more than one use
    //  -- it has only one use and that is in the model currently being processed
    T_Symbols&          symbols = m_symbols;
    DgnModelId          currModelId = model.GetModelId();
    for (size_t i=0; i<symbols.size();)
        {
        XGraphicsSymbolP    symbol = symbols[i];

        BeRepositoryBasedId symbolId = symbol->GetParentId();
        BeRepositoryBasedId clientEleId;
        DgnModelId clientModelId;
        bool                clientIsStamp;
        size_t              nDependents = GetXGraphicsSymbolDependentCount (clientModelId, clientEleId, clientIsStamp, symbolId);

        BeAssert(0 != symbolId.GetValue());
        if (nDependents > minCompressDependents)
            {
            //  No step can decide to delete it so remove it from future consideration
            RemoveSymbolFromTree(symbol);
            delete symbol;
            symbols.erase (symbols.begin() + i);
            continue;
            }

        if (currModelId != clientModelId)
            {
            //  We want to delete this one but the one element using it belongs to a different model.
            i++;
            continue;
            }

#if defined (NEEDS_WORK_DGNITEM)
        ElementRefP      dependent = model.FindElementById(ElementId(clientEleId.GetValue()));
        ElementRefP      symbolParentRef;

        for (symbolParentRef = dependent; NULL != symbolParentRef->GetParentElementRef(); symbolParentRef = symbolParentRef->GetParentElementRef())
            ;

        EditElementHandle  symbolParent (symbolParentRef);

        if (SUCCESS == dropSymbolInstance (symbolParent, dependent, *symbol))
            symbolParent.ReplaceInModel(symbolParentRef);

        RemoveSymbolFromTree(symbol);
        XGraphicsSymbolStamp::Delete(DgnStampId(symbolId.GetValue()), m_dgnProject);
        delete symbol;
        symbols.erase (symbols.begin() + i);
#endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      05/09
+---------------+---------------+---------------+---------------+---------------+------*/
void  XGraphicsSymbolCache::Dump (DgnModelP modelRef)
    {
#ifdef NEEDS_WORK
    DgnModelP                   dgnModel = modelRef;
    XGraphicsSymbolCacheR       cache = Get (*dgnModel);
    UInt32                      minInstanceCount = 0xffff, maxInstanceCount = 0, totalInstanceCount = 0, totalSize = 0;

    for (bvector <XGraphicsSymbolP>::iterator curr = cache->m_symbols.begin(), end = cache->m_symbols.end(); curr != end; curr++)
        {
        UInt32     instanceCount = dgnModel->FindByElementId((*curr)->GetElementId())->GetDependents (NULL, 0);

        minInstanceCount = std::min (minInstanceCount, instanceCount);
        maxInstanceCount = std::max (maxInstanceCount, instanceCount);
        totalInstanceCount += instanceCount;
        totalSize          += (UInt32) (*curr)->GetDataSize();
        }

    printf ("%d Symbols - Min Instance Count: %d, Max InstanceCount: %d, AverageInstanceCount: %f, Total InstanceCount: %d, Total Size: %d\n",
            cache->m_symbols.size(), minInstanceCount, maxInstanceCount, (double) totalInstanceCount / (double) cache->m_symbols.size(), totalInstanceCount, totalSize);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            XGraphicsSymbolCache::IsLoaded (DgnProjectR dgnProject)
    {
    return NULL != dgnProject.AppData().Find (XGraphicsSymbolCacheAppData::GetKey());
    }

#if defined(NOTNOW)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            XGraphicsSymbolCache::Free (DgnModelR dgnModel)
    {
    dgnModel.DropAppData (XGraphicsSymbolCacheAppData::GetKey());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsSymbolCacheR      XGraphicsSymbolCache::Get (DgnProjectR dgnProject)
    {
    XGraphicsSymbolCacheAppData*  appData = (XGraphicsSymbolCacheAppData*) dgnProject.AppData().Find (XGraphicsSymbolCacheAppData::GetKey());

    if (NULL == appData)
        {
        appData = new XGraphicsSymbolCacheAppData (dgnProject);

        dgnProject.AppData().Add (XGraphicsSymbolCacheAppData::GetKey(), appData);
        }

    return appData->m_symbolCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsSymbolCache::~XGraphicsSymbolCache ()
    {
    for (size_t i=0; i < m_symbols.size(); i++)   
        delete m_symbols.at(i);
    }

static double       s_symbolMatchTolerance = .35;           // points in the PlantSpace pipe cells differ by this much.

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
static StatusInt defineSymbolStamp (DgnStampId& stampId, XGraphicsContainerR symbol, TransformCP transform, XGraphicsSymbolIdCP xGraphicsSymbolId, DgnProjectCR dgnProject, DgnModelR dgnModel)
    {
    DRange3d range;
    symbol.CalculateRange(range, dgnModel, NULL);
    XGraphicsSymbolStamp::Create(stampId, dgnProject, symbol, range, transform, xGraphicsSymbolId, dgnModel.Is3d());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt XGraphicsSymbolCache::AddSymbol (TransformR transform, size_t& index, XGraphicsContainerR parent, XGraphicsContainerR symbol, DgnModelR dgnModel, Symbology const* symbology)
    {
    Transform           basisTransform, inverseTransform;
    XGraphicsContainer  untransformedSymbol = symbol;
    T_Symbols&          symbols = m_symbols;

    transform.initIdentity ();

    if (SUCCESS != symbol.GetBasisTransform (basisTransform) || !inverseTransform.inverseOf (&basisTransform))
        return ERROR;

    TransformInfo   inverseTransformInfo;

    inverseTransformInfo.GetTransformR() = inverseTransform;
    untransformedSymbol.OnTransform (inverseTransformInfo);

    auto treeIter = m_symbolsTree.find (untransformedSymbol.GetDataSize());
    if (m_symbolsTree.end() != treeIter)
        {
        for (auto& existingSymbol : treeIter->second)
            {
            if (existingSymbol->IsEqual (untransformedSymbol, s_symbolMatchTolerance, symbology))
                {
                index = parent.GetSymbolCount();
                parent.AddSymbol (existingSymbol->GetParentId());
                delete &symbol;

                transform.productOf (&basisTransform, existingSymbol->GetTransform());

                return SUCCESS;
                }
            }
        }

    EditElementHandle       eh;
    StatusInt               status;
    DgnStampId stampId;

    if (SUCCESS != (status = defineSymbolStamp (stampId, symbol, &inverseTransform, NULL, m_dgnProject, dgnModel)))
        return status;

    m_isDirty = true;
    untransformedSymbol.SetStampId(stampId);
    index = parent.GetSymbolCount();
    parent.AddSymbol (stampId);
    symbols.push_back (new XGraphicsSymbol (untransformedSymbol, &inverseTransform, symbology));
    AddSymbolToTree(symbols.back());
    delete &symbol;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/10
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsSymbol::XGraphicsSymbol (XGraphicsContainerR container, TransformCP transform, Symbology const* symbology) : XGraphicsContainer (container), m_transform (NULL == transform  ?NULL : new Transform (*transform)), m_symbology (NULL), m_qvElemSet (NULL)
    {
    if (NULL != symbology)
        {
        m_symbology = new Symbology ();
        *m_symbology = *symbology;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/10
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsSymbol::~XGraphicsSymbol ()
    {
    DELETE_AND_CLEAR (m_symbology);
    DELETE_AND_CLEAR (m_transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool XGraphicsSymbol::IsEqual (XGraphicsContainer const& rhs, double distanceTolerance, Symbology const* symbology)
    {
    if (NULL != symbology && NULL != m_symbology)
        {
        if (symbology->color != m_symbology->color ||
            symbology->style != m_symbology->style ||
            symbology->weight != m_symbology->weight)
            return false;
        }
    return T_Super::IsEqual (rhs, distanceTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsSymbolId::XGraphicsSymbolId (ElementHandleCR eh, Int32 symbolId)
    {
    m_handlerId = eh.GetHandler().GetHandlerId();
    m_symbolId = symbolId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsSymbolP        XGraphicsSymbol::Create (ElementHandleCR eh, IStrokeForCache& stroker, double pixelSize)
    {
    XGraphicsSymbolP    symbol  = new XGraphicsSymbol (NULL);

    if (SUCCESS != symbol->CreateFromStroker (stroker, eh, pixelSize))
        DELETE_AND_CLEAR (symbol);

    return symbol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsSymbolP    XGraphicsSymbolCache::FindOrAddMatchingSymbol (XGraphicsSymbolIdCR id, XGraphicsSymbolP symbol)
    {
    BeAssert(false && "using FindOrAddMatchingSymbol");
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
XGraphicsSymbolP XGraphicsSymbolCache::GetSymbol (ElementHandleCR eh, Int32 id, IStrokeForCache& stroker, double pixelSize)
    {
    BeAssert(false && "using XGraphicsSymbolCache::GetSymbol");
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId   XGraphicsSymbolCache::GetSymbolElementId (ElementHandleCR eh, Int32 id, IStrokeForCache& stroker, double pixelSize)
    {
    BeAssert(false && "in GetSymbolElementId");
    return ElementId();
    }

enum SymbolHeaderMask
    {
    SHM_Is3d        = 1,
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void XGraphicsSymbolStamp::Append(DgnStamps::StampData& stampData, void const*data, size_t dataSize)
    {
    if (0 == dataSize)
        return;

    size_t currSize = stampData.size();

    stampData.resize (currSize + dataSize);
    memcpy (&(stampData.at(currSize)), data, dataSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void XGraphicsSymbolStamp::AppendSection(DgnStamps::StampData& stampData, XGraphicsSymbolStamp::OpCode opCodeIn, size_t dataSizeIn, void const*data)
    {
    UInt32 dataSize = (UInt32)dataSizeIn;
    UInt16 opCode = (UInt16)opCodeIn;

    Append(stampData, &opCode, sizeof (opCode));
    Append(stampData, &dataSize, sizeof(dataSize));
    Append(stampData, data, dataSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
XGraphicsSymbolStamp::XGraphicsSymbolStamp(DgnStamps::StampData& stampData, DgnStampId symbolId, DgnProjectCR project, bool is3d, DRange3dCR range) : m_project(project), m_symbolId(symbolId), m_is3d(is3d), m_range(range)
    {
    m_stampData = &stampData;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
XGraphicsSymbolStampPtr XGraphicsSymbolStamp::Get(DgnProjectCR project, DgnStampId symbolId)
    {
    DgnStamps::StampName   name(XGraphicsSymbolStamp::GetStampNamespace(), XGraphicsSymbolStamp::GetStampName(), symbolId);
    DgnStamps::StampDataPtr stampDataPtr = project.Stamps().FindStamp(name);

    if (!stampDataPtr.IsValid())
        return NULL;

    DgnStamps::StampData& stampData = *stampDataPtr.get();
    UInt16 stampHeader = stampData[0] + (stampData[1] << 8);
    bool is3d = (stampHeader & SHM_Is3d) != 0;
    DRange3d range;
    memcpy(&range, &stampData[2], sizeof(range));
    return new XGraphicsSymbolStamp(stampData, symbolId, project, is3d, range);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
BeSQLite::DbResult XGraphicsSymbolStamp::Delete(DgnStampId stampId, DgnProjectR project)
    {
    DgnStamps::StampName   name(XGraphicsSymbolStamp::GetStampNamespace(), XGraphicsSymbolStamp::GetStampName(), stampId);
    return project.Stamps().DeleteStamp(name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
BeSQLite::DbResult XGraphicsSymbolStamp::Create(DgnStampId& stampId, DgnProjectCR project, XGraphicsContainerCR xgraphics, DRange3dCR range, TransformCP transform, XGraphicsSymbolIdCP xGraphicsSymbolId, bool is3d)
    {
    DgnStamps::StampData    stampData;
    size_t      size = xgraphics.GetDataSize();
    UInt16      header = is3d ? SHM_Is3d : 0;

    stampData.reserve(size + sizeof(*transform) + sizeof(range) + 200);

    //  Header is 2 bytes of flags followed by a DRanged3d.
    Append(stampData, &header, sizeof(header));
    Append(stampData, &range, sizeof(range));

    AppendSection(stampData, OP_XGraphics, size, const_cast<XGraphicsContainerR>(xgraphics).GetData());

    //  add symbol ID's
    T_XGraphicsSymbolIds const & symbols = xgraphics.GetSymbols ();
    if (symbols.size() > 0)
        {
        BeRepositoryBasedId const*symbolIdsData = &symbols[0];
        AppendSection(stampData, OP_SymbolIds, sizeof (*symbolIdsData)*symbols.size(), symbolIdsData);
        }

    if (NULL != transform)
        AppendSection(stampData, OP_Transform, sizeof(*transform), transform);

    if (NULL != xGraphicsSymbolId)
        AppendSection(stampData, OP_SymbolId, sizeof(*xGraphicsSymbolId), xGraphicsSymbolId);

    //  InsertStamp stores the DgnStampId in the name.
    DgnStamps::StampName   name(XGraphicsSymbolStamp::GetStampNamespace(), XGraphicsSymbolStamp::GetStampName());
    BeSQLite::DbResult result = project.Stamps().InsertStamp(name, stampData);

    stampId = name.GetId1();

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void const*XGraphicsSymbolStamp::GetData(UInt32& streamSize, OpCode testOpCode) const
    {
    if (!m_stampData.IsValid())
        {
        streamSize = 0;
        return NULL;
        }

    DgnStamps::StampData&stampData = *m_stampData;
    UInt32 offset = 6;  //  bytes for opcode and size
    UInt32 headerSize = sizeof(UInt16) + sizeof (DRange3d);

    //  Header is in first 2 bytes. Start searching at 2.
    for(UInt8*data = &stampData[headerSize], *endData = &stampData[0]+stampData.size(); data < endData; data += streamSize+offset)
        {
        OpCode opCode = (OpCode)(*data | (data[1] << 8));
        streamSize = data[2] | data[3] << 8 | data[4] << 16 | data[5] << 24;
        if (opCode == testOpCode)
            return data+offset;
        }

    streamSize = 0;
    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void const*XGraphicsSymbolStamp::GetXGraphicStream(UInt32& streamSize) const
    {
    return GetData(streamSize, OP_XGraphics);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsSymbolStamp::GetTransform(TransformR transform) const
    {
    UInt32 streamSize;
    void const*data = GetData(streamSize, XGraphicsSymbolStamp::OP_Transform);
    if (NULL == data)
        return BSIERROR;

    BeAssert(streamSize == sizeof(transform));
    memcpy(&transform, data, sizeof transform);
    return BSISUCCESS;
    }

#if defined(NOTNOW)
    //  Graphite does not support nested symbols or symbol indexing.
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
BentleyStatus XGraphicsSymbolStamp::GetGraphicsSymbolId(XGraphicsSymbolId&symbolId) const
    {
    UInt32 streamSize;
    void const*data = GetData(streamSize, XGraphicsSymbolStamp::OP_SymbolId);
    if (NULL == data)
        return BSIERROR;

    BeAssert(streamSize == sizeof(symbolId));
    memcpy(&symbolId, data, sizeof symbolId);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
void const*XGraphicsSymbolStamp::GetSymbolIds(UInt32& numberIds) const
    {
    UInt32 streamSize;
    void const*data = GetData(streamSize, OP_SymbolIds);
    BeAssert((streamSize & 7) == 0);
    numberIds = streamSize/sizeof(BeRepositoryBasedId);
    //! This points to the IDs but they are not memory aligned.  Accessing them by casting to a DgnStamp* may produce an alignment fault.
    return data;
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
DgnProjectCR XGraphicsSymbolStamp::GetDgnProject() const
    {
    return m_project;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2014
//---------------------------------------------------------------------------------------
XGraphicsSymbolStamp::~XGraphicsSymbolStamp()
    {
    BeAssert(m_stampData.IsValid());

    UInt32 stampRefCount = m_stampData->GetRefCount();
    BeAssert(stampRefCount > 0);
    if (2 == stampRefCount)
        //  This counts the reference this object holds and the reference the cache holds.
        ViewContext::FreeQvElems(*this);
    }
