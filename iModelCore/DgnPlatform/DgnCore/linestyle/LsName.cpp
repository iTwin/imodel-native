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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void LsDefinition::SetName (Utf8CP name)
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

    m_rasterInitialized = false;
    m_rasterTexture = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinition::LsDefinition (Utf8CP name, DgnDbR project, Json::Value& lsDefinition, DgnStyleId styleId)
    {
    Init (name, lsDefinition, styleId);
    LsComponentType compType = GetComponentType(lsDefinition);
    LsComponentId compId = GetComponentId(lsDefinition);

    m_location.SetLocation (project, compType, compId);
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

    Utf8String retval;
    return Utf8String(retval);
    }

//  Logic to make a line style behave as a raster line style.
//---------------------------------------------------------------------------------------
// This color may possibly come from the element, or it may come from the line style.
// If it comes from the element, do we need a different map for every color?
//
// The texture may also have to reflect differences in line style overrides.
//
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
/* static */  void CreateGeometryMapMaterial(ViewContextR context, IStrokeForCache& stroker, intptr_t textureId)
    {
    context.GetIViewDraw ().DefineQVGeometryMap (textureId, stroker, nullptr, false, context, false);
    return;
    }

//=======================================================================================
//! Used to generate a texture based on a line style.
// @bsiclass                                                    John.Gooding    08/2015
//=======================================================================================
struct ComponentToTextureStroker : IStrokeForCache
{
private:
    ViewContextR        m_viewContext;
    LineStyleSymbR      m_lineStyleSymb;
    LsDefinitionR       m_lsDef;
    DPoint3d            m_points[2];
    double              m_multiplier;
    double              m_length;
    Transform           m_transformForTexture;

public:

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
ComponentToTextureStroker(ViewContextR viewContext, LineStyleSymbR lineStyleSymb, LsDefinitionR lsDef) : m_viewContext(viewContext), m_lineStyleSymb(lineStyleSymb), m_lsDef(lsDef) 
    {
    LsComponentCP    topComponent = lsDef.GetComponentCP (nullptr);
    m_length = topComponent->_GetLength() * lineStyleSymb.GetScale();
    if (m_length <  mgds_fc_epsilon)
        m_length = 1.0;   //  Apparently nothing is length dependent.

    //  NEEDSWORK_LINESTYLES decide how to scale when creating texture.
    m_multiplier = 1024.0 * 1024 * 1024;
    m_transformForTexture.InitFromScaleFactors(m_multiplier, m_multiplier, m_multiplier);
    m_points[0].Init(0, 0, 0);
    m_points[1].Init(m_length, 0, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
void _StrokeForCache(ViewContextR context) override
    {
    ElemDisplayParams   savedParams(context.GetCurrentDisplayParams());
    ElemMatSymb         savedMatSymb (*context.GetElemMatSymb());

    context.GetIDrawGeom().ActivateMatSymb(&savedMatSymb);

    //  Use the current symbology, activating it here.  We may also activate symbology when drawing 
    //  symbols. 

    //  Compute size of one iteration and stroke a line for that size.
    LsComponentCP    topComponent = m_lsDef.GetComponentCP (nullptr);


    context.PushTransform(m_transformForTexture);

    topComponent->_StrokeLineString(&context, &m_lineStyleSymb, m_points, 2, false);

    context.PopTransformClip();

    context.GetCurrentDisplayParams() = savedParams;
    *context.GetElemMatSymb() = savedMatSymb;
    }

    DgnDbR _GetDgnDb() const override {return m_viewContext.GetDgnDb();}
    Graphic* _GetGraphic(DgnViewportCR vp) const override {return nullptr;}
    void _SaveGraphic(DgnViewportCR vp, GraphicR graphic) const override {}

};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
intptr_t  LsDefinition::GenerateRasterTexture(ViewContextR viewContext, LineStyleSymbR lineStyleSymb)
    {
    //  Assume the caller already knows this is something that must be converted but does not know it can be converted.
    BeAssert(m_lsComp->GetComponentType() != LsComponentType::RasterImage);
    ComponentToTextureStroker   stroker(viewContext, lineStyleSymb, *this);
    
    viewContext.GetIViewDraw ().DefineQVGeometryMap (intptr_t(this), stroker, NULL, false, viewContext, false);
    return intptr_t(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t LsDefinition::GetRasterTexture (ViewContextR viewContext, LineStyleSymbR lineStyleSymb, bool forceRaster, double scale) 
    {
    if (!m_lsComp.IsValid())
        {
        BeAssert(0 == m_rasterTexture);
        return m_rasterTexture;
        }

    if (!m_rasterInitialized)
        {
        m_rasterInitialized = true;
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

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
                    DgnPlatformLib::GetHost().GetGraphicsAdmin()._DefineTextureId (m_rasterTexture = reinterpret_cast <uintptr_t> (this), imageSize, true, 5, &alpha.front());
#endif
                    }
                else
                    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
                    DgnPlatformLib::GetHost().GetGraphicsAdmin()._DefineTextureId (m_rasterTexture = reinterpret_cast <uintptr_t> (this), imageSize, true, 0, image);
#endif
                    }
                }
            }
        else if (forceRaster)
            {
            //  Convert this type to raster on the fly if possible
#if TRYING_DIRECT_LINESTYLES
            m_rasterTexture = 0; 
#else
            m_rasterTexture = GenerateRasterTexture(viewContext, lineStyleSymb);
#endif
            }
        }
    

    double          rasterWidth;

    if (0 != m_rasterTexture &&
        m_lsComp.IsValid() &&
        SUCCESS == m_lsComp->_GetRasterTextureWidth (rasterWidth))
        lineStyleSymb.SetWidth (rasterWidth * scale);
    
    return m_rasterTexture;
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
        DgnStyleId  styleId ((int64_t)stmt.GetValueInt(0));
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

