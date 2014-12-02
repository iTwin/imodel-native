/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsDb.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/LsLocal.h>

static Utf8CP DGNPROPERTYBLOB_CompId                = "compId";
static Utf8CP DGNPROPERTYBLOB_CompType              = "compType";
static Utf8CP DGNPROPERTYBLOB_Flags                 = "flags";
static Utf8CP DGNPROPERTYBLOB_UnitDef               = "unitDef";


//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//---------------------------------------------------------------------------------------
void V10ComponentBase::GetDescription (Utf8P target) const
    {
    BeStringUtilities::Strncpy(target, LS_MAX_DESCR, m_descr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//---------------------------------------------------------------------------------------
void V10ComponentBase::SetDescription (Utf8CP source)
    {
    memset (m_descr, 0, sizeof (m_descr));
    BeStringUtilities::Strncpy(m_descr, LS_MAX_DESCR, source);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsStrokePatternComponent::CreateRscFromDgnDb(LineCodeRsc** rscOut, DgnProjectR project, UInt32 componentId)
    {
    *rscOut = NULL;
    UInt32  propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::LineCode(), componentId, 0))
        return BSIERROR;

    V10LineCode* lineCodeData = (V10LineCode*)_alloca (propertySize);

    project.QueryProperty(lineCodeData, propertySize, LineStyleProperty::LineCode(), componentId, 0);
    BeAssert (propertySize == V10LineCode::GetBufferSize(lineCodeData->m_nStrokes));

    size_t  rscSize = LC_RSCSIZE(lineCodeData->m_nStrokes);
    LineCodeRsc*    rsc = (LineCodeRsc*)memutil_calloc (1, rscSize, 'lsUP');

    lineCodeData->GetDescription (rsc->descr);

    rsc->phase = lineCodeData->m_phase;
    rsc->options = lineCodeData->m_options;
    rsc->maxIterate = lineCodeData->m_maxIterate;

    rsc->nStrokes = lineCodeData->m_nStrokes;

    for (UInt32 index = 0; index < lineCodeData->m_nStrokes; ++index)
        {
        rsc->stroke[index].length       = lineCodeData->m_stroke[index].m_length;
        rsc->stroke[index].width        = lineCodeData->m_stroke[index].m_width;
        rsc->stroke[index].endWidth     = lineCodeData->m_stroke[index].m_endWidth;
        rsc->stroke[index].strokeMode   = lineCodeData->m_stroke[index].m_strokeMode;
        rsc->stroke[index].widthMode    = lineCodeData->m_stroke[index].m_widthMode;
        rsc->stroke[index].capMode      = lineCodeData->m_stroke[index].m_capMode;
        }

    *rscOut = rsc;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsPointComponent::CreateRscFromDgnDb(LinePointRsc** rscOut, DgnProjectR project, UInt32 componentId)
    {
    *rscOut = NULL;
    UInt32  propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::LinePoint(), componentId, 0))
        return BSIERROR;

    V10LinePoint* linePointData = (V10LinePoint*)_alloca (propertySize);

    project.QueryProperty(linePointData, propertySize, LineStyleProperty::LinePoint(), componentId, 0);
    BeAssert (propertySize == V10LinePoint::GetBufferSize(linePointData->m_nSymbols));

    size_t  rscSize = LP_RSCSIZE(linePointData->m_nSymbols);
    LinePointRsc*    rsc = (LinePointRsc*)memutil_calloc (1, rscSize, 'lsUP');

    linePointData->GetDescription (rsc->descr);

    UInt32  numberSymbols = linePointData->m_nSymbols;

    rsc->nSym = numberSymbols;

    rsc->lcType = linePointData->m_lcType;
    rsc->lcID = linePointData->m_lcID;

    for (UInt32 index = 0; index < numberSymbols; ++index)
        {
        rsc->symbol[index].symType  = linePointData->m_symbol[index].m_symType;
        rsc->symbol[index].symID    = linePointData->m_symbol[index].m_symID;
        rsc->symbol[index].strokeNo = linePointData->m_symbol[index].m_strokeNo;
        rsc->symbol[index].mod1     = linePointData->m_symbol[index].m_mod1;
        rsc->symbol[index].xOffset  = linePointData->m_symbol[index].m_xOffset;
        rsc->symbol[index].yOffset  = linePointData->m_symbol[index].m_yOffset;
        rsc->symbol[index].zAngle   = linePointData->m_symbol[index].m_zAngle;
        }

    *rscOut = rsc;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsCompoundComponent::CreateRscFromDgnDb(LineStyleRsc** rscOut, DgnProjectR project, UInt32 componentId)
    {
    *rscOut = NULL;
    UInt32 propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::Compound(), componentId, 0))
        return BSIERROR;

    V10Compound* compoundData = (V10Compound*)_alloca (propertySize);

    project.QueryProperty(compoundData, propertySize, LineStyleProperty::Compound(), componentId, 0);

    UInt32  numberComponents = compoundData->m_nComponents;
    size_t  rscSize = LS_RSCSIZE(numberComponents);

    LineStyleRsc*    rsc = (LineStyleRsc*)memutil_calloc (1, rscSize, 'lsUP');

    BeAssert (propertySize == V10Compound::GetBufferSize(numberComponents));

    compoundData->GetDescription (rsc->descr);

    rsc->nComp = numberComponents;
    for (UInt32 index = 0; index < numberComponents; ++index)
        {
        rsc->component[index].type      = compoundData->m_component[index].m_type;
        rsc->component[index].id        = compoundData->m_component[index].m_id;
        rsc->component[index].offset    = compoundData->m_component[index].m_offset;
        }

    *rscOut = rsc;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsSymbolComponent::CreateRscFromDgnDb(PointSymRsc** rscOut, DgnProjectR project, UInt32 componentId)
    {
    *rscOut = NULL;
    UInt32 propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::PointSym(), componentId, 0))
        return BSIERROR;

    V10Symbol* symbolData = (V10Symbol*)_alloca (propertySize);

    project.QueryProperty(symbolData, propertySize, LineStyleProperty::PointSym(), componentId, 0);

    UInt32  graphicsId = symbolData->m_symbolId;
    UInt32  graphicsIdSize = 0;

    PointSymRsc::SymbolType    symbolType = PointSymRsc::ST_XGraphics;
    if (BE_SQLITE_ROW != project.QueryPropertySize(graphicsIdSize, LineStyleProperty::SymbolXGraphics(), graphicsId, 0))
        {
        symbolType = PointSymRsc::ST_Elements;
        project.QueryPropertySize(graphicsIdSize, LineStyleProperty::SymbolElements(), graphicsId, 0);
        }

    size_t  rscSize = sizeof (PointSymRsc) + graphicsIdSize;
    PointSymRsc* psr = (PointSymRsc*)memutil_calloc(1, rscSize, 'lsUP');

    symbolData->GetDescription (psr->header.descr);

    project.QueryProperty(psr->symBuf, graphicsIdSize, 
                            PointSymRsc::ST_XGraphics == symbolType ? LineStyleProperty::SymbolXGraphics() : LineStyleProperty::SymbolElements(), graphicsId, 0);

    psr->nBytes = graphicsIdSize;

    psr->header.range = symbolData->m_range;
    psr->header.scale = symbolData->m_scale;
    psr->symFlags = symbolData->m_symFlags;
    psr->SetSymbolType (symbolType);

    *rscOut = psr;
    return BSISUCCESS;
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static void symbolRangeUnion (SymbolRange& range, DgnElementCP pElm)
    {
    if (pElm->IsGraphic())
        {
        DRange3d   dRange = pElm->GetRange();

        if (dRange.low.x < range.low.x)
            range.low.x = dRange.low.x;

        if (dRange.low.y < range.low.y)
            range.low.y = dRange.low.y;

        if (dRange.low.z < range.low.z)
            range.low.z = dRange.low.z;

        if (dRange.high.x > range.high.x)
            range.high.x = dRange.high.x;

        if (dRange.high.y > range.high.y)
            range.high.y = dRange.high.y;

        if (dRange.high.z > range.high.z)
            range.high.z = dRange.high.z;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
void LsSymbolComponent::StreamElements (bvector<byte>&elementData, MSElementDescrVec const& elms)
    {
    for (auto child : elms)
        {
        byte* data = (byte*)&child->Element();
        elementData.insert(elementData.end(), data, data + child->Element().Size());
        StreamElements (elementData, child->Components());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LineStyleUpgradeProcessor::UpgradePointSymbol (UInt32& v10Id, LStyleDefEntryElm const& lStyleDef)
    {
    v10Id = 0;

    //  Most of the fields are unused.  Only the descr and the list of components are used.
    //  Table entry should identify type (Compound), descr, and ID. The blob is just a list of components.
    //  To retrieve it, create a LineStyleRsc, zero it, add descr, and component numbers.
    BeAssert (LsElementType::PointSymbol == (LsElementType)lStyleDef.type);

    UInt32 bufferSize = V10Symbol::GetBufferSize();
    V10Symbol*v10Symbol = (V10Symbol*)_alloca(bufferSize);
    memset (v10Symbol, 0, bufferSize);
    
    SetDescription (v10Symbol, lStyleDef);

    PointSymRsc&     psr = (PointSymRsc&)lStyleDef.data;

    UInt64 const* dependents = GetDependents(lStyleDef);

    //  We expect to find 1 dependent, a cell that contains the elements that
    //  are part of the symbol.
    BeAssert (1 == lStyleDef.numDependents);

    //  Cast away const so we can temporarily change next
    MSElementDescrP cell = const_cast<MSElementDescrP>(FindComponent (dependents[0]));
    BeAssert (NULL != cell);

    if (NULL == cell)
        return LINESTYLE_STATUS_InvalidForV8Symbol;

    BeAssert (cell->Element().GetLegacyType() == CELL_HEADER_ELM);
    bvector<byte>   data;

    LsSymbolComponent::StreamElements (data, cell->Components());                       

    UInt32      symbolGraphicsId;
    LsSymbolComponent::AddSymbolGraphicsAsProperty (symbolGraphicsId, GetProject(), &data[0], data.size(), PointSymRsc::ST_Elements);

    psr.header.range.low.x = 
    psr.header.range.low.y = 
    psr.header.range.low.z = 1e37;

    psr.header.range.high.x = 
    psr.header.range.high.y = 
    psr.header.range.high.z = -1e37;

    for (auto& child : cell->Components())
        symbolRangeUnion (psr.header.range, &child->Element());

    v10Symbol->m_range = psr.header.range;

    if (cell->Element().IsGraphic() && cell->Element().Is3d())
        psr.symFlags |= LSSYM_3D;

    v10Symbol->m_symFlags = psr.symFlags;
    v10Symbol->m_symbolId = symbolGraphicsId;
    v10Symbol->m_scale = psr.header.scale;
    v10Symbol->m_symbolId = symbolGraphicsId;

    return LsComponent::AddComponentAsProperty (v10Id, GetProject(), LineStyleProperty::PointSym(), v10Symbol, bufferSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
MSElementDescrCP LineStyleUpgradeProcessor::FindComponent (UInt64 v8Id)
    {
    if (NULL == m_v8Components)
        return NULL;

    for (auto& edP :  m_v8Components->Components())
        {
        if (edP->ElementR().GetElementId().GetValue() == v8Id)
            return edP.get();
        }

    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
UInt64 const* LineStyleUpgradeProcessor::GetDependents (LStyleDefEntryElm const& lStyleDef)
    {
    return (UInt64*)(lStyleDef.data + lStyleDef.dataSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//---------------------------------------------------------------------------------------
void LineStyleUpgradeProcessor::SetDescription (V10ComponentBase*v10, LStyleDefEntryElm const& lStyleDef)
    {
    //  For all of the types the description is the first thing in the data.  It is 
    //  a locale encoded string so we probably should convert.  
    //  WIP do locale to Ut8 conversion
    v10->SetDescription ((CharCP)lStyleDef.data);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LineStyleUpgradeProcessor::UpgradeCompoundType (UInt32&v10Id, LStyleDefEntryElm const& lStyleDef)
    {
    v10Id = 0;

    BeAssert (0 == lStyleDef.componentCount);
    BeAssert (LsElementType::Compound == (LsElementType)lStyleDef.type);

    LineStyleRsc const&     lsr = (LineStyleRsc const&)lStyleDef.data;

    UInt32 bufferSize = V10Compound::GetBufferSize(lsr.nComp);
    V10Compound*v10Compound = (V10Compound*)_alloca(bufferSize);
    memset(v10Compound, 0, bufferSize);
    
    SetDescription (v10Compound, lStyleDef);
    v10Compound->m_nComponents = lsr.nComp;
    v10Compound->m_auxType = lsr.auxType;

    UInt64 const* dependents = GetDependents(lStyleDef);

    BeAssert (lStyleDef.numDependents == lsr.nComp);

    for (unsigned i = 0; i < lsr.nComp; ++i)
        {
        ComponentInfo const& comp = lsr.component[i];
        V10ComponentInfo& v10comp = v10Compound->m_component[i];

        v10comp.m_type = (UInt32)remapRscTypeToElmType(comp.type);
        if (LsElementType::Internal == (LsElementType)v10comp.m_type)
            v10comp.m_id = comp.id;
        else
            UpgradeLsComponent(v10comp.m_id, dependents[i], (LsElementType)v10comp.m_type);

        v10comp.m_offset = comp.offset;
        }

    return LsComponent::AddComponentAsProperty(v10Id, GetProject(), LineStyleProperty::Compound(), v10Compound, bufferSize);
    }

//---------------------------------------------------------------------------------------
// This assumes that the ID's have already been updated.
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LsCompoundComponent::AddToProject (UInt32& v10Id, DgnProjectR project, LineStyleRsc const& lsr)
    {
    v10Id = 0;

    UInt32 bufferSize = V10Compound::GetBufferSize(lsr.nComp);
    V10Compound*v10Compound = (V10Compound*)_alloca(bufferSize);
    memset (v10Compound, 0, bufferSize);
    
    v10Compound->SetDescription (lsr.descr);
    v10Compound->m_nComponents = lsr.nComp;
    v10Compound->m_auxType = lsr.auxType;

    for (unsigned i = 0; i < lsr.nComp; ++i)
        {
        ComponentInfo const& comp = lsr.component[i];
        V10ComponentInfo& v10comp = v10Compound->m_component[i];

        v10comp.m_type = comp.type;
        v10comp.m_id = comp.id;
        v10comp.m_offset = comp.offset;
        }

    return LsComponent::AddComponentAsProperty(v10Id, project, LineStyleProperty::Compound(), v10Compound, bufferSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LineStyleUpgradeProcessor::UpgradeLineCode (UInt32& v10Id, LStyleDefEntryElm const& lStyleDef)
    {
    BeAssert (0 == lStyleDef.componentCount);
    BeAssert (0 == lStyleDef.numDependents);
    BeAssert (0 == lStyleDef.dependentsSize);
    BeAssert (LsElementType::LineCode == (LsElementType)lStyleDef.type);

    LineCodeRsc const&     lcr = (LineCodeRsc const&)lStyleDef.data;

    UInt32 bufferSize = V10LineCode::GetBufferSize(lcr.nStrokes);
    V10LineCode*v10LineCode = (V10LineCode*)_alloca(bufferSize);
    memset (v10LineCode, 0, bufferSize);

    v10LineCode->SetDescription(lcr.descr);
    v10LineCode->m_phase = lcr.phase;
    v10LineCode->m_maxIterate = lcr.maxIterate;
    v10LineCode->m_options = lcr.options;
    v10LineCode->m_nStrokes = lcr.nStrokes;

    for (unsigned i = 0; i < lcr.nStrokes; ++i)
        {
        StrokeData const& stroke = lcr.stroke[i];
        V10StrokeData & v10stroke = v10LineCode->m_stroke[i];

        v10stroke.m_length = stroke.length;
        v10stroke.m_width = stroke.width;
        v10stroke.m_endWidth = stroke.endWidth;

        v10stroke.m_strokeMode = stroke.strokeMode;
        v10stroke.m_widthMode = stroke.widthMode;
        v10stroke.m_capMode = stroke.capMode;
        v10stroke.m_bReserved = 0;
        }

    return LsComponent::AddComponentAsProperty(v10Id, GetProject(), LineStyleProperty::LineCode(), v10LineCode, bufferSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LineStyleUpgradeProcessor::UpgradeLinePoint (UInt32& v10Id, LStyleDefEntryElm const& lStyleDef)
    {
    //  Most of the fields are unused.  Only the descr and the list of components are used.
    //  Table entry should identify type (Compound), descr, and ID. The blob is just a list of components.
    //  To retrieve it, create a LineStyleRsc, zero it, add descr, and component numbers.
    BeAssert (0 == lStyleDef.componentCount);
    BeAssert (LsElementType::LinePoint == (LsElementType)lStyleDef.type);

    LinePointRsc const&     lpr = (LinePointRsc const&)lStyleDef.data;
    UInt64 const* dependents = GetDependents(lStyleDef);

    UInt32 bufferSize = V10LinePoint::GetBufferSize(lpr.nSym);
    V10LinePoint*v10LinePoint = (V10LinePoint*)_alloca(bufferSize);
    memset (v10LinePoint, 0, bufferSize);

    v10LinePoint->SetDescription(lpr.descr);
    v10LinePoint->m_nSymbols = lpr.nSym;

    v10LinePoint->m_lcType = (UInt32)remapRscTypeToElmType(lpr.lcType);

    UInt32  lcID = lpr.lcID;
    if (LsElementType::Internal != (LsElementType)v10LinePoint->m_lcType)
        UpgradeLsComponent(lcID, dependents[0], (LsElementType)v10LinePoint->m_lcType);

    v10LinePoint->m_lcID = lcID;

    BeAssert (lpr.nSym == lStyleDef.numDependents - 1);

    for (unsigned i = 0; i < lpr.nSym; ++i)
        {
        PointSymInfo const&                 sym = lpr.symbol[i];
        V10PointSymbolInfo &                v10sym = v10LinePoint->m_symbol[i];

        v10sym.m_symType = (UInt32)remapRscTypeToElmType((UInt32)sym.symType);
        UInt32 symbolId;

        // sym.symID has the original rscID, dependents[i+1] has the corresponding element ID.
        // sym.symID is useless here.
        UpgradeLsComponent (symbolId, dependents[i+1], (LsElementType)v10sym.m_symType);
        v10sym.m_symID = symbolId;

        v10sym.m_strokeNo = sym.strokeNo;
        v10sym.m_mod1 = sym.mod1;
        v10sym.m_xOffset = sym.xOffset;
        v10sym.m_yOffset = sym.yOffset;
        v10sym.m_zAngle = sym.zAngle;
        }

    return LsComponent::AddComponentAsProperty (v10Id, GetProject(), LineStyleProperty::LinePoint(), v10LinePoint, bufferSize);
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LsPointComponent::AddToProject (UInt32& newId, DgnProjectR project, LinePointRsc const& lpr)
    {
    UInt32 bufferSize = V10LinePoint::GetBufferSize(lpr.nSym);
    V10LinePoint*v10LinePoint = (V10LinePoint*)_alloca(bufferSize);
    memset (v10LinePoint, 0, bufferSize);

    v10LinePoint->SetDescription(lpr.descr);
    v10LinePoint->m_nSymbols = lpr.nSym;

    v10LinePoint->m_lcType = lpr.lcType;
    v10LinePoint->m_lcID = lpr.lcID;


    for (unsigned i = 0; i < lpr.nSym; ++i)
        {
        PointSymInfo const&         sym = lpr.symbol[i];
        V10PointSymbolInfo &        v10sym = v10LinePoint->m_symbol[i];

        v10sym.m_symType = sym.symType;
        v10sym.m_symID = sym.symID;

        v10sym.m_strokeNo = sym.strokeNo;
        v10sym.m_mod1 = sym.mod1;
        v10sym.m_xOffset = sym.xOffset;
        v10sym.m_yOffset = sym.yOffset;
        v10sym.m_zAngle = sym.zAngle;
        }

    return LsComponent::AddComponentAsProperty (newId, project, LineStyleProperty::LinePoint(), v10LinePoint, bufferSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LsStrokePatternComponent::AddToProject (UInt32& newId, DgnProjectR project, LineCodeRsc const& lcr)
    {
    UInt32 bufferSize = V10LineCode::GetBufferSize(lcr.nStrokes);
    V10LineCode*v10LineCode = (V10LineCode*)_alloca(bufferSize);
    memset (v10LineCode, 0, bufferSize);

    v10LineCode->SetDescription(lcr.descr);
    v10LineCode->m_phase = lcr.phase;
    v10LineCode->m_maxIterate = lcr.maxIterate;
    v10LineCode->m_options = lcr.options;
    v10LineCode->m_nStrokes = lcr.nStrokes;

    for (unsigned i = 0; i < lcr.nStrokes; ++i)
        {
        StrokeData const& stroke = lcr.stroke[i];
        V10StrokeData & v10stroke = v10LineCode->m_stroke[i];

        v10stroke.m_length = stroke.length;
        v10stroke.m_width = stroke.width;
        v10stroke.m_endWidth = stroke.endWidth;

        v10stroke.m_strokeMode = stroke.strokeMode;
        v10stroke.m_widthMode = stroke.widthMode;
        v10stroke.m_capMode = stroke.capMode;
        v10stroke.m_bReserved = 0;
        }

    return LsComponent::AddComponentAsProperty(newId, project, LineStyleProperty::LineCode(), v10LineCode, bufferSize);
    }

#if defined (NEEDS_WORK_DGNITEM)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleUpgradeProcessor::LineStyleUpgradeProcessor (DgnProjectR project, MSElementDescrCP components) :
                            m_v8Components(components), m_dgnProject (project)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LineStyleUpgradeProcessor::UpgradeLineStyleV8toV10 (LStyleNameEntryElm const& lStyle, Utf8CP alternateName)
    {
    BeAssert (lStyle.flags.isElement);

    UInt32   v10ComponentId;
    UpgradeLsComponent(v10ComponentId, lStyle.refersTo, remapRscTypeToElmType(lStyle.type));

    Utf8String  lineStyleName;
    BeStringUtilities::Utf16ToUtf8 (lineStyleName, lStyle.name);

    if (NULL == alternateName)
        alternateName = lineStyleName.c_str();

    DgnStyleId  origId ((UInt32)lStyle.id);
    DbResult result = GetProject().Styles ().LineStyles().InsertWithId (origId, alternateName, v10ComponentId, lStyle.type, 
                        lStyle.styleFlags, lStyle.auxInfo[1]/LSUNIT_FACTOR);

    if (BE_SQLITE_DONE == result)
        return LINESTYLE_STATUS_Success;

    return IsConstraintDbResult(result) ? LINESTYLE_STATUS_SQLITE_Constraint : LINESTYLE_STATUS_SQLITE_Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LineStyleUpgradeProcessor::UpgradeLsComponent (UInt32& v10Id, UInt64 componentId, LsElementType type)
    {
    if (LsElementType::Internal == type)
        {
        v10Id = (UInt32)componentId;
        return LINESTYLE_STATUS_Success;
        }

    bmap <UInt64, UInt32>::iterator it = m_idMap.find(componentId);
    if (it != m_idMap.end())
        {
        v10Id = it->second;
        return LINESTYLE_STATUS_Success;
        }

    MSElementDescrCP v8component = FindComponent(componentId);
    if (NULL == v8component)
        return LINESTYLE_STATUS_ComponentNotFound;

    LineStyleStatus retval = LINESTYLE_STATUS_Success;
    LStyleDefEntryElm const& el= (LStyleDefEntryElm const&)v8component->Element();
    switch ((LsElementType)el.type)
        {
        case LsElementType::LineCode:
            retval = UpgradeLineCode (v10Id, el);
            break;

        case LsElementType::Compound:
            retval = UpgradeCompoundType (v10Id, el);
            break;

        case LsElementType::LinePoint:
            retval = UpgradeLinePoint (v10Id, el);
            break;

        case LsElementType::PointSymbol:
            retval = UpgradePointSymbol (v10Id, el);
            break;
        }

    m_idMap[componentId] = v10Id;
    return retval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
RefCountedPtr<LineStyleUpgradeProcessor> LineStyleUpgradeProcessor::Create (DgnProjectR project, MSElementDescrCP components)
    {
    return new LineStyleUpgradeProcessor (project, components);
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//--------------+------------------------------------------------------------------------
void LsDefinition::InitializeJsonObject (Json::Value& jsonObj)
    {
    InitializeJsonObject (jsonObj, m_location.GetRscID(), static_cast <UInt16> (m_location.GetElementType()),
                          m_attributes, (UInt32) m_unitDef); // WIP: m_unitDef double --> UInt32 conversion
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
void LsDefinition::InitializeJsonObject (Json::Value& jsonObj, UInt32 componentId, UInt16 componentType, UInt32 flags, double unitDefinition)
    {
    jsonObj.clear();

    jsonObj[DGNPROPERTYBLOB_CompId] = componentId;
    jsonObj[DGNPROPERTYBLOB_CompType] = (UInt32)remapRscTypeToElmType(componentType);
    jsonObj[DGNPROPERTYBLOB_Flags] = flags;
    jsonObj[DGNPROPERTYBLOB_UnitDef] = unitDefinition;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
double LsDefinition::GetUnitDef (Json::Value& lsDefinition)
    {
    return lsDefinition[DGNPROPERTYBLOB_UnitDef].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
UInt32 LsDefinition::GetAttributes (Json::Value& lsDefinition)
    {
    return lsDefinition[DGNPROPERTYBLOB_Flags].asUInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
UInt32 LsDefinition::GetComponentType (Json::Value& lsDefinition)
    {
    return lsDefinition[DGNPROPERTYBLOB_CompType].asUInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
UInt32 LsDefinition::GetComponentId (Json::Value& lsDefinition)
    {
    return lsDefinition[DGNPROPERTYBLOB_CompId].asUInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void LsComponent::QueryComponentIds(bset<LsComponentId>& ids, DgnProjectCR project, LsResourceType lsType)
    {
    //  No default constructor so we have to initialize it.
    LineStyleProperty::ComponentProperty spec = LineStyleProperty::Compound();

    switch(lsType)
        {
        case LsResourceType::Compound:
            break;
        case LsResourceType::LineCode:
            spec = LineStyleProperty::LineCode();
            break;
        case LsResourceType::LinePoint:
            spec = LineStyleProperty::LinePoint();
            break;
        case LsResourceType::PointSymbol:
            spec = LineStyleProperty::PointSym();
            break;

        default:
            BeAssert(false && "bad lsType argument to QueryComponentIds");
            return;
        }

    Statement stmt;
    stmt.Prepare (project, SqlPrintfString("SELECT Id FROM " BEDB_TABLE_Property " WHERE Namespace=? AND Name=?"));

    stmt.BindText(1, spec.GetNamespace(), Statement::MAKE_COPY_No);
    stmt.BindText(2, spec.GetName(), Statement::MAKE_COPY_No);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        LsComponentId     componentId;
        componentId.m_id = stmt.GetValueInt(0);
        componentId.m_type = (UInt32)lsType;
        ids.insert(componentId);
        };
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
BentleyStatus LsStrokePatternComponent::GetRscFromDgnDb(LineCodeRscPtr& ptr, DgnProjectR project, UInt32 componentId)
    {
    return CreateRscFromDgnDb(&ptr.m_data, project, componentId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
BentleyStatus LsCompoundComponent::GetRscFromDgnDb(LineStyleRscPtr& ptr, DgnProjectR project, UInt32 componentId)
    {
    return CreateRscFromDgnDb(&ptr.m_data, project, componentId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
BentleyStatus LsSymbolComponent::GetRscFromDgnDb(PointSymRscPtr& ptr, DgnProjectR project, UInt32 componentId)
    {
    return CreateRscFromDgnDb(&ptr.m_data, project, componentId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
BentleyStatus LsPointComponent::GetRscFromDgnDb(LinePointRscPtr& ptr, DgnProjectR project, UInt32 componentId)
    {
    return CreateRscFromDgnDb(&ptr.m_data, project, componentId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
LineCodeRscPtr::~LineCodeRscPtr() { memutil_free(m_data); }
LinePointRscPtr::~LinePointRscPtr() { memutil_free(m_data); }
LineStyleRscPtr::~LineStyleRscPtr() { memutil_free(m_data); }
PointSymRscPtr::~PointSymRscPtr() { memutil_free(m_data); }

