/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsName.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

static DgnHost::Key s_allMapsKey;
static DgnHost::Key s_systemLsFileInfoKey;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct LineStyleRangeCollector : IGeometryProcessor
{
private:
    LsComponentR        m_component;
    DRange3d            m_range;
    Transform           m_currentTransform;
    DPoint3d            m_points[2];

protected:

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
explicit LineStyleRangeCollector(LsComponentR component, DPoint3d points[2]) 
: m_component(component) 
    {
    //  Make sure to include both end points in case the line style has pading at the beginning or end.
    m_range = DRange3d::From(points[0], points[1]);
    m_currentTransform.InitIdentity();
    m_points[0] = points[0];
    m_points[1] = points[1];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
//  virtual bool _ProcessAsFacets(bool isPolyface) const override {return false;}
//  virtual bool _ProcessAsBody(bool isCurved) const override {return false;}
//  virtual void _AnnounceContext(ViewContextR context) override {m_context = &context;}
//  virtual void _AnnounceTransform(TransformCP trans) override {if (trans) m_currentTransform = *trans; else m_currentTransform.InitIdentity();}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
bool _ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic&) override
    {
    DRange3d  range;

    curves.GetRange(range, m_currentTransform);
    m_range.Extend(range);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
virtual void _OutputGraphics(ViewContext& viewContext) override
    {
    Render::GraphicPtr  graphic = viewContext.CreateGraphic();

    LineStyleSymb   lineStyleSymb;
    lineStyleSymb.Init(nullptr);
    //  lineStyleSymb.SetScale(m_scaleFactor);

    m_component._StrokeLineString(*graphic, &viewContext, &lineStyleSymb, m_points, 2, false);
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
static void Process(DRange3dR range, LsComponentR component, DPoint3d points[2])
    {
    LineStyleRangeCollector  processor(component, points);

    BeAssert(nullptr != component.GetDgnDbP());
    GeometryProcessor::Process(processor, *component.GetDgnDbP());

    processor.GetRange(range);
    }

}; // LineStyleRangeCollector

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
void LsDefinition::SetHWStyle (LsComponentId componentIDIn)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    uint32_t componentID = componentIDIn.GetValue();
    m_hardwareLineCode = -1;
    if (LsComponentType::Internal == componentIDIn.GetType())
        {
        // Linecode only if hardware bit is set and masked value is within range.
        if ( (0 != (componentID & LSID_HARDWARE)) && ((componentID & LSID_HWMASK) <= MAX_LINECODE))
            m_hardwareLineCode = (long)componentID & LSID_HWMASK;
        else
            m_hardwareLineCode = 0;
        }
#endif
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
    m_texture = nullptr;
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
    LsComponentId compId = GetComponentId(lsDefinition);

    m_location.SetLocation (project, compId);
    SetHWStyle(compId);
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

#define NUMBER_ITERATIONS_ComponentStroker   (1)
#define MAX_XRANGE_RATIO (256 * NUMBER_ITERATIONS_ComponentStroker)

//=======================================================================================
//! Base class for StrokeComponentForRange and ComponentToTextureStroker
// @bsiclass                                                    John.Gooding    10/2015
//=======================================================================================
struct ComponentStroker
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

DgnDbR GetDgnDb() const { return m_dgndb; }
double GetLength() {return m_points[1].x;}
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
};  // StrokeComponentForRange

//=======================================================================================
//! Used to generate a texture based on a line style.
// @bsiclass                                                    John.Gooding    08/2015
//=======================================================================================
struct ComponentToTextureStroker : ComponentStroker
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
Render::GraphicPtr Stroke(ViewContextR context) const
    {
    GraphicParams         elemMatSymb;

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


    //  Create the graphic
    Render::GraphicPtr graphic = context.CreateGraphic(Graphic::CreateParams(context.GetViewport(), m_transformForTexture));

    //  Add symbology
    graphic->ActivateGraphicParams(elemMatSymb);
    m_component->_StrokeLineString(*graphic, &context, &lineStyleSymb, m_points, 2, false);

    return graphic;
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2016
//---------------------------------------------------------------------------------------
static void initializePoints(DPoint3d points[2], LsComponentR component, double scale)
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

    points[0].Init(0, 0, 0);
    points[1].Init(length, 0, 0);
    }

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
Render::TexturePtr LsDefinition::GenerateTexture(double& textureDrawWidth, ViewContextR viewContext, LineStyleSymbR lineStyleSymb)
    {
    double unitDef = GetUnitsDefinition();
    if (unitDef < mgds_fc_epsilon)
        unitDef = 1.0;

    textureDrawWidth = 0;
    DgnViewportP vp = viewContext.GetViewport();
    if (vp == nullptr)
        return nullptr;

    //  Assume the caller already knows this is something that must be converted but does not know it can be converted.
    BeAssert(m_lsComp->GetComponentType() != LsComponentType::RasterImage);

    //  A component may cache information used from running texture generation for another component.
    //  If so, reset it now.  _StartTextureGeneration also traverses the children.
    m_lsComp->_StartTextureGeneration();

    if (m_lsComp->_IsOkayForTextureGeneration() == Dgn::LsOkayForTextureGeneration::NotAllowed)
        return nullptr;

    //  Something in the component tree may be modified for texture generation.  For example, the size of an iteration
    //  may be expanded to accomodate a symbol that overflows.
    LsComponentPtr  comp = m_lsComp->_GetForTextureGeneration();
    if (comp.IsNull())
        return nullptr;

    //  Get just the range of the components.  Don't let any scaling enter into this.
    DPoint3d  points[2];
    initializePoints(points, *m_lsComp, 1.0);

    DRange3d  lsRange;
    LineStyleRangeCollector::Process(lsRange, *comp, points);

    uint32_t  scaleFactor = 1;
    DRange2d range2d = getAdjustedRange(scaleFactor, lsRange, comp->_GetLength());

    SymbologyQueryResults  symbologyResults;
    comp->_QuerySymbology(symbologyResults);
    ColorDef lineColor, fillColor;
    bool isColorBySymbol = symbologyResults.IsColorBySymbol(lineColor, fillColor) && !symbologyResults.IsColorByLevel();

    uint32_t lineWeight;
    bool isWeightBySymbol = symbologyResults.IsWeightBySymbol(lineWeight);
    if (!isWeightBySymbol)
        lineWeight = 0;

    ComponentToTextureStroker   stroker(viewContext.GetDgnDb(), scaleFactor, lineColor, fillColor, lineWeight, *comp);
    GraphicPtr graphic = stroker.Stroke(viewContext);

    Render::GeometryTexturePtr texture = vp->GetRenderTarget()->CreateGeometryTexture(*graphic, range2d, isColorBySymbol, false, 0);
    texture->_QueueTask();
    texture->ReleaseGraphic();

    double yRange = range2d.high.y - range2d.low.y;
    if (0.0 == yRange)
        yRange = 1;

    textureDrawWidth = yRange * unitDef;
    return texture.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Texture* LsDefinition::GetTexture(ViewContextR viewContext, LineStyleSymbR lineStyleSymb, bool forceTexture, double scaleWithoutUnits) 
    {
    if (!m_lsComp.IsValid())
        return nullptr;

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

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
                    DgnPlatformLib::GetHost().GetGraphicsAdmin()._DefineTextureId (m_textureHandle = reinterpret_cast <uintptr_t> (this), imageSize, true, 5, &alpha.front());
#endif
                    }
                else
                    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
                    DgnPlatformLib::GetHost().GetGraphicsAdmin()._DefineTextureId (m_textureHandle = reinterpret_cast <uintptr_t> (this), imageSize, true, 0, image);
#endif
                    }
                }
            m_hasTextureWidth = m_lsComp->_GetTextureWidth(m_textureWidth) == BSISUCCESS;
            }
        else if (forceTexture)
            {
            m_textureInitialized = true;
            //  Convert this type to texture on the fly if possible
            m_texture = GenerateTexture(m_textureWidth, viewContext, lineStyleSymb);
            m_hasTextureWidth = true;
            }
        }

    if (m_texture.IsValid() && m_lsComp.IsValid() && m_hasTextureWidth)
        lineStyleSymb.SetWidth (m_textureWidth * scaleWithoutUnits);
    
    return m_texture.get();
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

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsCache::Load ()
    {
    if (IsLoaded())
        return SUCCESS;

    //  Signal that this should abort a query and should not trigger an assertion failure in GraphicsAndQuerySequencer::CheckSQLiteOperationAllowed
    TreeLoaded ();

    for (auto const& ls : LineStyleElement::MakeIterator(m_dgnDb))
        {
        DgnStyleId  styleId (ls.GetElementId().GetValue());
        Utf8String name(ls.GetName());
        Utf8String  data (ls.GetData());

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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    03/2016
//---------------------------------------------------------------------------------------
LsDefinition* LsDefinition::Clone()
    {
    Json::Value jsonObj(Json::objectValue);
    LsDefinition::InitializeJsonObject(jsonObj, GetLocation()->GetComponentId(), GetAttributes(), m_unitDef);

    LsDefinition* retval = new LsDefinition(_GetName(), *GetLocation()->GetDgnDb(), jsonObj, m_styleId);

    return retval;
    }