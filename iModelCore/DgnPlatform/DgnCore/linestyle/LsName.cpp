/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsName.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

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
Transform           m_currentTransform;

protected:

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
explicit LineStyleRangeCollector(IStrokeForCache& stroker) : m_stroker(stroker)
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
static void Process(DRange3dR range, IStrokeForCache& stroker)
    {
    LineStyleRangeCollector  processor(stroker);

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
    m_hasTextureWidth = false;
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
    SetHWStyle (compType, compId);
    m_componentLookupFailed = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsDefinition::~LsDefinition()
    {
    SetName (NULL);
    if (m_textureInitialized)
        T_HOST.GetGraphicsAdmin()._DeleteTexture (m_textureHandle);
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

#define NUMBER_ITERATIONS_ComponentStroker   (1)
#define MAX_XRANGE_RATIO (256 * NUMBER_ITERATIONS_ComponentStroker)

//=======================================================================================
//! Base class for StrokeComponentForRange and ComponentToTextureStroker
// @bsiclass                                                    John.Gooding    10/2015
//=======================================================================================
struct ComponentStroker : Dgn::IStrokeForCache
    {
protected:
    DgnDbR              m_dgndb;
    LsComponentPtr      m_component;
    DPoint3d            m_points[2];
public:

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
ComponentStroker(DgnDbR dgndb, LsComponentR component, double scale) : m_dgndb(dgndb), m_component(&component)
    {
    double length = component._GetLength() * NUMBER_ITERATIONS_ComponentStroker;

    if (length <  mgds_fc_epsilon)
        {
        //  Apparently nothing is length dependent.
        length = component._GetMaxWidth(nullptr);
        if (length <  mgds_fc_epsilon)
            length = 1.0;
        }

    length *= scale;

    m_points[0].Init(0, 0, 0);
    m_points[1].Init(length, 0, 0);
    }

int32_t _GetQvIndex() const override {return 1;}
QvElemP _GetQvElem(double pixelSize) const override {return nullptr;}
void _SaveQvElem(QvElemP, double pixelSize = 0.0, double sizeDependentRatio = 0.0) const override {}
DgnDbR _GetDgnDb() const override { return m_dgndb; }
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
StrokeComponentForRange(DgnDbR dgndb, LsComponentR component) : ComponentStroker(dgndb, component, 1.0)
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
    LineStyleRangeCollector::Process(range, *this);
    }
};

//=======================================================================================
//! Used to generate a texture based on a line style.
// @bsiclass                                                    John.Gooding    08/2015
//=======================================================================================
struct          ComponentToTextureStroker : ComponentStroker
{
private:
    double              m_scaleFactor;
    Transform           m_transformForTexture;
    ColorDef            m_lineColor;
    ColorDef            m_fillColor;
    uint32_t            m_lineWeight;

public:

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
ComponentToTextureStroker(DgnDbR dgndb, double scaleFactor, ColorDef lineColor, ColorDef fillColor, uint32_t lineWeight, LsComponentR component) : 
            ComponentStroker(dgndb, component, scaleFactor), m_scaleFactor(scaleFactor), m_lineColor(lineColor), m_fillColor(fillColor), m_lineWeight(lineWeight)
    {
    //  If a modified copy is required, the caller passed the copy. 
    BeAssert(component._IsOkayForTextureGeneration() == LsOkayForTextureGeneration::NoChangeRequired);

    //  NEEDSWORK_LINESTYLES -- it doesn't make sense to mirror this. Figure out why QV needs it
    DVec3d normal;
    normal.Init(0, 1, 0);
    DPoint3d zero;
    zero.Zero();
    m_transformForTexture.InitFromMirrorPlane(zero, normal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
void _StrokeForCache(ViewContextR context, double pixelSize = 0.0) override
    {
    ElemMatSymb         elemMatSymb;

    elemMatSymb.Init();
    //  Generally the line style will be drawn with the color of the element. We accomplish that
    //  by passing QV_GEOTEXTURE_DEFERCLRSEL to  _DefineQVGeometryMap.
    //
    //  A line style may specify that a symbol get its color from the symbol.  In a vector line style,
    //  it is possible for different symbols to have different colors and for some symbols to be
    //  drawn with the element color while others are drawn with the symbol color. For a texture
    //  style the entire line style has to be drawn with the same color.
    elemMatSymb.SetLineColor(m_lineColor);
    elemMatSymb.SetFillColor(m_fillColor);
    elemMatSymb.SetWidth(m_lineWeight);

    LineStyleSymb   lineStyleSymb;
    lineStyleSymb.Init(nullptr);
    lineStyleSymb.SetScale(m_scaleFactor);

    context.GetIDrawGeom().ActivateMatSymb(&elemMatSymb);

    context.PushTransform(m_transformForTexture);
    m_component->_StrokeLineString(&context, &lineStyleSymb, m_points, 2, false);
    context.PopTransformClip();
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2015
//---------------------------------------------------------------------------------------
static DRange2d getAdjustedRange(uint32_t& scaleFactor, DRange3dCR lsRange, double componentLength)
    {
    scaleFactor = 1;

    //  Use the stroked range, accounting for any leading or trailing pad.
    DRange2d range2d;
    range2d.low.x = std::min(0.0, lsRange.low.x);
    range2d.low.y = lsRange.low.y;
    range2d.high.x = std::max(lsRange.high.x, componentLength);
    range2d.high.y = lsRange.high.y;

    double xRange = range2d.high.x - range2d.low.x;
    BeAssert(xRange != 0.0);
    if (xRange == 0.0)
        return range2d;

    if (xRange > range2d.high.y * MAX_XRANGE_RATIO)
        range2d.high.y = 2 * xRange/MAX_XRANGE_RATIO;

    if (xRange > -range2d.low.y * MAX_XRANGE_RATIO)
        range2d.low.y = -2 * xRange/MAX_XRANGE_RATIO;

    //  if xRange is too small  StrokeComponentForRange will fail when it creates the viewport because it will be smaller than the minimum.
    if (xRange < 1)
        {
        scaleFactor = (uint32_t)ceil(1/xRange);
        range2d.high.Scale(scaleFactor);
        range2d.low.Scale(scaleFactor);
        xRange = range2d.high.x - range2d.low.x;
        }

    //  Theoretically we could make the image smaller and save memory by just guaranteeing that
    //  the size of the Y range is a multiple of 2 of the X range or vice versa.  However, I don't
    //  think QV is detecting that correctly so for now I am just going for the same size.  Without
    //  this change to the range QV scales the contents of the geometry map in one direction or the other.
    double yVal = xRange/2.0;
    bool changed = false;
    while (yVal < range2d.high.y || -yVal > range2d.low.y)
        {
        changed = true;
        yVal *= 2.0;
        }

    if (!changed)
        {
        while (yVal/2.0 >= range2d.high.y && -yVal/2.0 <= range2d.low.y)
            {
            yVal /= 2.0;
            }
        }

    if (yVal < 2)
        yVal = 2;

    range2d.low.y = -yVal;
    range2d.high.y = yVal;

    return range2d;
    }

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

    //  Get just the range of the components.  Don't let any scaling enter into this.
    DRange3d  lsRange;
    StrokeComponentForRange rangeStroker(viewContext.GetDgnDb(), *comp);
    rangeStroker.ComputeRange(lsRange);

    uint32_t  scaleFactor = 1;
    DRange2d range2d = getAdjustedRange(scaleFactor, lsRange, comp->_GetLength());

    SymbologyQueryResults  symbologyResults;
    comp->_QuerySymbology(symbologyResults);
    ColorDef lineColor, fillColor;
    bool isColorBySymbol = symbologyResults.IsColorBySymbol(lineColor, fillColor) && !symbologyResults.IsColorByLevel();
    if (!isColorBySymbol)
        {
        //  This should not matter because we pass false for isColorBySymbol causing DefineQVGeometryMap to use QV_GEOTEXTURE_DEFERCLRSEL
        //  However, at the time this was tested it QV_GEOTEXTURE_DEFERCLRSEL did not provide the expected behavior.
        lineColor = ColorDef::White();
        fillColor = ColorDef::White();
        }

    uint32_t lineWeight;
    bool isWeightBySymbol = symbologyResults.IsWeightBySymbol(lineWeight);
    if (!isWeightBySymbol)
        lineWeight = 0;

    ComponentToTextureStroker   stroker(viewContext.GetDgnDb(), scaleFactor, lineColor, fillColor, lineWeight, *comp);

    viewContext.GetIViewDraw ().DefineQVGeometryMap (intptr_t(this), stroker, range2d, isColorBySymbol, viewContext, false);

    m_hasTextureWidth = true;
    m_textureWidth = scaleFactor * (range2d.high.y - range2d.low.y);
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
        if (m_lsComp->GetComponentType() == LsComponentType::RasterImage)
            {
            uint8_t const* image;
            Point2d     imageSize;
            uint32_t      flags = 0;

            m_textureInitialized = true;
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
            m_hasTextureWidth = m_lsComp->_GetTextureWidth(m_textureWidth) == BSISUCCESS;
            }
        else if (forceTexture)
            {
            m_textureInitialized = true;
            //  Convert this type to texture on the fly if possible
            m_textureHandle = GenerateTexture(viewContext, lineStyleSymb);
            }
        }
    

    if (0 != m_textureHandle && m_lsComp.IsValid() && m_hasTextureWidth)
        lineStyleSymb.SetWidth (m_textureWidth * scale);
    
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
    delete m_nameRec;
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

