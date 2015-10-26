/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsName.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

// this is bogus.
#if defined (DGN_PLATFORM_MT)
#undef DGN_PLATFORM_MT
#include    <RmgrTools/Tools/rmgrstrl.h>
#define DGNPLATFORM_MT
#else
#include    <RmgrTools/Tools/rmgrstrl.h>
#endif

static DgnHost::Key s_allMapsKey;
static DgnHost::Key s_systemLsFileInfoKey;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct LineStyleRangeCollector : IElementGraphicsProcessor
{
private:

IStrokeForCache&    m_stroker;
DRange3d            m_range;
ViewContextP        m_context;
DgnCategoryId       m_categoryId;
Transform           m_currentTransform;

protected:

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
explicit LineStyleRangeCollector(IStrokeForCache& stroker, DgnCategoryId categoryId) : m_stroker(stroker), m_categoryId(categoryId)
    {
    m_range.Init();
    m_currentTransform.InitIdentity();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
virtual bool _ProcessAsFacets(bool isPolyface) const override {return false;}
virtual bool _ProcessAsBody(bool isCurved) const override {return false;}
virtual void _AnnounceContext(ViewContextR context) override {m_context = &context;}
virtual void _AnnounceTransform(TransformCP trans) override {if (trans) m_currentTransform = *trans; else m_currentTransform.InitIdentity();}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
virtual BentleyStatus _ProcessCurveVector(CurveVectorCR curves, bool isFilled) override
    {
    DRange3d  range;

    curves.GetRange(range, m_currentTransform);
    m_range.Extend(range);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
virtual void _OutputGraphics(ViewContext& context) override
    {
    context.GetCurrentDisplayParams().SetCategoryId(m_categoryId);
    m_stroker._StrokeForCache(context);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
void GetRange(DRange3dR range)
    {
    range = m_range;
    }

public:

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
static void Process(DRange3dR range, IStrokeForCache& stroker, DgnCategoryId categoryId)
    {
    LineStyleRangeCollector  processor(stroker, categoryId);

    ElementGraphicsOutput::Process(processor, stroker._GetDgnDb());

    processor.GetRange(range);
    }

}; // LineStyleRangeCollector

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsDefinition::SetName
(
Utf8CP          name
)
    {
    if (m_name.m_key)
        FREE_AND_CLEAR (m_name.m_key);

    if (name)
        {
        m_name.m_key = BeStringUtilities::Strdup (name);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void LsDefinition::SetHWStyle (LsComponentType componentType, LsComponentId componentIDIn)
    {
    uint32_t componentID = componentIDIn.GetValue();
    m_hardwareLineCode = -1;
    if (LsComponentType::Internal == componentType)
        {
        // Linecode only if hardware bit is set and masked value is within range.
        if ( (0 != (componentID & LSID_HARDWARE)) && ((componentID & LSID_HWMASK) <= MAX_LINECODE))
            m_hardwareLineCode = (long)componentID & LSID_HWMASK;
        else
            m_hardwareLineCode = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void LsDefinition::Init(Utf8CP name, Json::Value& lsDefinition, DgnStyleId styleId)
    {
    m_isDirty           = false;
    m_lsComp            = NULL;

    m_attributes        = GetAttributes(lsDefinition);
    m_unitDef           = GetUnitDef(lsDefinition);
    m_styleId       = styleId;

    m_hardwareLineCode  = -1;
    m_maxWidth          = 0.0;
    m_componentLoadPostProcessed = false;

    m_name = NULL;
    SetName (name);

    m_textureInitialized = false;
    m_textureHandle = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2015
//---------------------------------------------------------------------------------------
void LsDefinition::Destroy(LsDefinitionP def) { delete def; }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinition::LsDefinition (Utf8CP name, DgnDbR project, Json::Value& lsDefinition, DgnStyleId styleId)
    {
    Init (name, lsDefinition, styleId);
    LsComponentType compType = GetComponentType(lsDefinition);
    LsComponentId compId = GetComponentId(lsDefinition);

    m_location.SetLocation (project, compType, compId);
    m_rasterComponentId = GetRasterComponentId(lsDefinition);
    SetHWStyle (compType, compId);
    m_componentLookupFailed = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinition::~LsDefinition()
    {
    SetName (NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    08/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsDefinition::Commit ()
    {
    BeAssert (0 && "unable to commit definition");
    
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String         LsDefinition::GetStyleName () const
    {
    Utf8CP   lsName = _GetName();
    if (NULL == lsName)
        return Utf8String (L"");

    return Utf8String(lsName);
    }

//=======================================================================================
//! Base class for StrokeComponentForRange and ComponentToTextureStroker
// @bsiclass                                                    John.Gooding    10/2015
//=======================================================================================
struct ComponentStroker : Dgn::IStrokeForCache
    {
protected:
    ViewContextR        m_viewContext;
    LsComponentPtr      m_component;
    DPoint3d            m_points[2];
public:

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
ComponentStroker(ViewContextR viewContext, LsComponentR component, double scale) : m_viewContext(viewContext), m_component(&component)
    {
    double length = component._GetLength();
    if (length <  mgds_fc_epsilon)
        length = 1.0;   //  Apparently nothing is length dependent.

    length *= scale;

    //  NEEDSWORK_LINESTYLES decide how to scale when creating texture.
    m_points[0].Init(0, 0, 0);
    m_points[1].Init(length, 0, 0);
    }

int32_t _GetQvIndex() const override {return 1;}
QvElemP _GetQvElem(double pixelSize = 0.0) const override {return nullptr;}
void _SaveQvElem(QvElemP, double pixelSize = 0.0, double sizeDependentRatio = 0.0) const override {}
DgnDbR _GetDgnDb() const override { return m_viewContext.GetDgnDb(); }
};

//=======================================================================================
//! Used to calculate the range of the line style.
// @bsiclass                                                    John.Gooding    09/2015
//=======================================================================================
struct          StrokeComponentForRange : ComponentStroker
{
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
StrokeComponentForRange(ViewContextR viewContext, LsComponentR component) : ComponentStroker(viewContext, component, 1.0)
    {
    //  It should have already created a copy of the components if that is necessary
    BeAssert(m_component->_IsOkayForTextureGeneration() == LsOkayForTextureGeneration::NoChangeRequired);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
void _StrokeForCache(ViewContextR context, double pixelSize = 0.0) override
    {
    LineStyleSymb defaultLsSymb;
    defaultLsSymb.Init(nullptr);
    m_component->_StrokeLineString(&context, &defaultLsSymb, m_points, 2, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
void ComputeRange(DRange3dR range)
    {
    LineStyleRangeCollector::Process(range, *this, m_viewContext.GetCurrentDisplayParams().GetCategoryId());
    }
};

// WIP It should be possible to eliminate this.  This is just used for experimenting.
static  int TEST_LS_SCALE = 1;

//=======================================================================================
//! Used to generate a texture based on a line style.
// @bsiclass                                                    John.Gooding    08/2015
//=======================================================================================
struct          ComponentToTextureStroker : ComponentStroker
{
private:
    LineStyleSymbR      m_lineStyleSymb;
    double              m_multiplier;
    Transform           m_transformForTexture;

public:

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
ComponentToTextureStroker(ViewContextR viewContext, LineStyleSymbR lineStyleSymb, LsComponentR component) : ComponentStroker(viewContext, component, lineStyleSymb.GetScale()), m_lineStyleSymb(lineStyleSymb)
    {
    //  If a modified copy is required, the caller passed the copy. 
    BeAssert(component._IsOkayForTextureGeneration() == LsOkayForTextureGeneration::NoChangeRequired);

    //  m_length = m_component->_GetLength() * lineStyleSymb.GetScale(); -- maybe push lineStyleSymb to ComponentStroker

    //  NEEDSWORK_LINESTYLES decide how to scale when creating texture.
    m_multiplier = TEST_LS_SCALE;
    m_transformForTexture.InitFromScaleFactors(m_multiplier, m_multiplier, m_multiplier);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
void _StrokeForCache(ViewContextR context, double pixelSize = 0.0) override
    {
    ElemDisplayParams   savedParams(context.GetCurrentDisplayParams());
    ElemMatSymb         savedMatSymb (*context.GetElemMatSymb());

    //  Use the current symbology, activating it here.  We may also activate symbology when drawing 
    //  symbols. 

    context.GetIDrawGeom().ActivateMatSymb(&savedMatSymb);

    context.PushTransform(m_transformForTexture);

    m_component->_StrokeLineString(&context, &m_lineStyleSymb, m_points, 2, false);

    context.PopTransformClip();

    context.GetCurrentDisplayParams() = savedParams;
    *context.GetElemMatSymb() = savedMatSymb;
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
intptr_t  LsDefinition::GenerateTexture(ViewContextR viewContext, LineStyleSymbR lineStyleSymb)
    {
    //  Assume the caller already knows this is something that must be converted but does not know it can be converted.
    BeAssert(m_lsComp->GetComponentType() != LsComponentType::RasterImage);
    m_lsComp->_StartTextureGeneration();

    if (m_lsComp->_IsOkayForTextureGeneration() == Dgn::LsOkayForTextureGeneration::NotAllowed)
        return 0;

    LsComponentPtr  comp = m_lsComp->_GetForTextureGeneration();
    if (comp.IsNull())
        return 0;

    DRange3d  lsRange;
    StrokeComponentForRange rangeStroker(viewContext, *comp);
    rangeStroker.ComputeRange(lsRange);

    ComponentToTextureStroker   stroker(viewContext, lineStyleSymb, *comp);

    DRange2d range2d;
    range2d.low.x = 0;  // maybe minimum of 0 and lsRange.low.x
    range2d.low.y = lsRange.low.y;
    range2d.high.x = std::max(lsRange.high.x, comp->_GetLength());
    range2d.high.y = lsRange.high.y;

    range2d.low.y *= lineStyleSymb.GetScale() * TEST_LS_SCALE;
    range2d.high.y *= lineStyleSymb.GetScale() * TEST_LS_SCALE;
    range2d.high.x *= lineStyleSymb.GetScale() * TEST_LS_SCALE;

    viewContext.GetIViewDraw ().DefineQVGeometryMap (intptr_t(this), stroker, range2d, false, viewContext, false);
    return intptr_t(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t     LsDefinition::GetTextureHandle (ViewContextR viewContext, LineStyleSymbR lineStyleSymb, bool forceTexture, double scale) 
    {
    if (!m_lsComp.IsValid())
        {
        BeAssert(0 == m_textureHandle);
        return m_textureHandle;
        }

    if (!m_textureInitialized)
        {
        m_textureInitialized = true;
        if (m_lsComp->GetComponentType() == LsComponentType::RasterImage)
            {
            uint8_t const* image;
            Point2d     imageSize;
            uint32_t      flags = 0;

            if (SUCCESS == m_lsComp->_GetRasterTexture (image, imageSize, flags))
                {
                if (0 != (flags & LsRasterImageComponent::FlagMask_AlphaOnly))       // Alpha Only.
                    {
                    size_t          imageBytes = imageSize.x * imageSize.y, inIndex = (flags & LsRasterImageComponent::FlagMask_AlphaChannel);
                    bool            invert     = 0 != (flags & LsRasterImageComponent::FlagMask_AlphaInvert);

                    bvector<uint8_t>   alpha (imageBytes);

                    for (size_t outIndex=0; outIndex < imageBytes; inIndex +=4, outIndex++)
                        alpha[outIndex] = invert ? (255 - image[inIndex]) : image[inIndex];

                    DgnPlatformLib::GetHost().GetGraphicsAdmin()._DefineTextureId (m_textureHandle = reinterpret_cast <uintptr_t> (this), imageSize, true, 5, &alpha.front());
                    }
                else
                    {
                    DgnPlatformLib::GetHost().GetGraphicsAdmin()._DefineTextureId (m_textureHandle = reinterpret_cast <uintptr_t> (this), imageSize, true, 0, image);
                    }
                }
            }
        else if (forceTexture)
            {
            //  Convert this type to texture on the fly if possible
#if TRYING_DIRECT_LINESTYLES
            m_textureHandle = 0; 
#else
            m_textureHandle = GenerateTexture(viewContext, lineStyleSymb);
#endif
            }
        }
    

    double          rasterWidth;

    if (0 != m_textureHandle &&
        m_lsComp.IsValid() &&
        SUCCESS == m_lsComp->_GetTextureWidth (rasterWidth))
        lineStyleSymb.SetWidth (rasterWidth * scale);
    
    return m_textureHandle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsCache::AddIdEntry(LsDefinitionP nameRec)
    {
    LsIdNode        idRec (nameRec->GetStyleId().GetValue(), nameRec->_GetName (), nameRec);

    m_idTree.Add (&idRec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsCache::RemoveIdEntry (DgnStyleId id)
    {
    LsIdNode    node (0,0,0);

    if (SUCCESS != m_idTree.Remove (id.GetValue(), &node))
        return  ERROR;

    node.Clear();
    return  SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2013
//---------------------------------------------------------------------------------------
LsCacheP  LsCache::GetDgnDbCache (DgnDbR project, bool loadIfNotLoaded)
    {
    return  project.Styles().LineStyles().GetLsCacheP (loadIfNotLoaded);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsIdNode::LsIdNode(int64_t id, Utf8CP name, LsDefinitionP nameRec)
    {
    m_id      = id;
    m_name    = NULL;
    m_nameRec = nameRec;

    SetName (name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsIdNode::SetName (Utf8CP name)
    {
    if (m_name)
        FREE_AND_CLEAR (m_name);

    if (name)
        m_name = BeStringUtilities::Strdup (name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsIdNode::Clear ()
    {
    SetName (NULL);
    m_id      = 0;
    m_nameRec = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    freeIdRec (LsIdNodeP node, void* arg)
    {
    (const_cast <LsIdNode*> (node))->Clear ();
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsCache::EmptyIdMap ()
    {
    // iterate tree, delete all entries
    m_idTree.Process (freeIdRec, NULL);
    m_idTree.Empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsCache::EmptyMaps()
    {
    EmptyIdMap ();
    m_isLoaded = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsCache::~LsCache ()
    {
    EmptyMaps ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbCR    LsCache::GetDgnDb () const
    {
    return m_dgnDb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2015
//---------------------------------------------------------------------------------------
LineStyleStatus DgnLineStyles::LoadStyle(LsDefinitionP&style, DgnStyleId styleId)
    {
    style = nullptr;
    if (!styleId.IsValid())
        return LINESTYLE_STATUS_BadArgument;

    Statement stmt;
    PrepareToQueryLineStyle(stmt, styleId);

    DbResult    dbResult = stmt.Step();
    if (dbResult != BE_SQLITE_ROW)
        return LINESTYLE_STATUS_StyleNotFound;

    Utf8String name(stmt.GetValueText(2));
    Utf8String  data ((Utf8CP)stmt.GetValueBlob(4));

    Json::Value  jsonObj (Json::objectValue);
    if (!Json::Reader::Parse(data, jsonObj))
        return LINESTYLE_STATUS_Error;

    style = new LsDefinition (name.c_str(), m_dgndb, jsonObj, styleId);

    return LINESTYLE_STATUS_Success;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsCache::Load ()
    {
    if (IsLoaded())
        return SUCCESS;

    TreeLoaded ();

    Statement stmt;
    m_dgnDb.Styles().LineStyles().PrepareToQueryAllLineStyles(stmt);

    while (stmt.Step() == BE_SQLITE_ROW)
        {
        DgnStyleId  styleId (stmt.GetValueUInt64(0));
        Utf8String name(stmt.GetValueText(2));
        Utf8String  data ((Utf8CP)stmt.GetValueBlob(4));

        Json::Value  jsonObj (Json::objectValue);
        if (!Json::Reader::Parse(data, jsonObj))
            return ERROR;

        LsDefinition* lsDef = new LsDefinition (name.c_str(), m_dgnDb, jsonObj, styleId);
        AddIdEntry(lsDef);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LsCachePtr LsCache::Create (DgnDbR project)
    {
    return new LsCache (project); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
                LsCacheStyleIterator::LsCacheStyleIterator (LsCacheCP map, bool wantBegin) 
    {
    m_index = 0;
    m_node = NULL;
    m_auxData = 0;

    if (wantBegin)
        {
        LsCacheP              nonConstMap = const_cast <LsCacheP> (map);
        T_LsIdIterator      iter = nonConstMap->FirstId();

        m_index = iter.m_index;
        m_node = iter.m_currNode;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsCacheStyleIterator&       LsCacheStyleIterator::operator++() 
    {
    T_LsIdIterator          iter ((T_LsIdTreeNode const*)m_node, m_index);
    
    iter = iter.Next ();

    m_index = iter.m_index;
    m_node = iter.m_currNode;

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsCacheStyleEntry const & LsCacheStyleIterator::operator* () const 
    {
    T_LsIdIterator          iter ((T_LsIdTreeNode const*)m_node, m_index);

    LsIdNodeP               idNode = iter.Entry ();
    
    return *static_cast <LsCacheStyleEntry const *> (idNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool                        LsCacheStyleIterator::operator==(LsCacheStyleIterator const& rhs) const 
    {
    return m_index == rhs.m_index && m_node == rhs.m_node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool                        LsCacheStyleIterator::operator!=(LsCacheStyleIterator const& rhs) const 
    {
    return !(*this == rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsCacheStyleIterator LsCache::begin () const 
    {
    return LsCacheStyleIterator (this, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsCacheStyleIterator LsCache::end   () const 
    {
    return LsCacheStyleIterator (this, false);
    }

/*----------------------------------------------------------------------------------*//**
* Find a style by id. If id > 0, first look in system map. If not there, look in modelref
* @bsimethod                                                    ChuckKirschman   09/02
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinitionP     LsCache::FindInMap (DgnDbR dgndb, DgnStyleId styleId)
    {
    LsCache*  lsMap = LsCache::GetDgnDbCache (dgndb, true);
    if (NULL == lsMap)
        {
        BeAssert (false);     // Should never occur
        return NULL;
        }

    return  lsMap->GetLineStyleP (styleId);
    }

/*---------------------------------------------------------------------------------**//**
* look up a linestyle in a LsCache by id. Return LsIdNodeP
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsIdNodeP       LsCache::FindId (DgnStyleId styleId) const
    {
    LsIdNodeP   retval = m_idTree.Get (styleId.GetValue());
    return retval;
    }

/*---------------------------------------------------------------------------------**//**
* look up a linestyle in a LsCache by id. Return name record.
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinitionP   LsCache::GetLineStyleP (DgnStyleId styleId) const
    {
    LsIdNodeP   idNode = FindId (styleId);
    return  (NULL != idNode) ? idNode->GetValue() : NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
Utf8String         LsCache::GetFileName () const 
    { 
    BeFileNameCR fileName = m_dgnDb.GetFileName();
    return Utf8String(fileName);
    }

LsDefinitionCP  LsCache::GetLineStyleCP (DgnStyleId styleId)  const { return GetLineStyleP (styleId); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsIdNodeP       LsCache::SearchIdsForName
(
Utf8CP  name
) const
    {
    LsCacheP      ncMap = const_cast <LsCacheP> (this);  //  key tree doesn't have a const iterator
    for (T_LsIdIterator curr = ncMap->FirstId(); curr.Entry(); curr = curr.Next())
        {
        Utf8CP currName =curr.Entry()->GetName();
        if (currName && !BeStringUtilities::Stricmp (currName, name))
            return  curr.Entry();
        }

    return  NULL;
    }

