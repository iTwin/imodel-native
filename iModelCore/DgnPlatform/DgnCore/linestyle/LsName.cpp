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
    LineStyleSymbR 		m_lsSymb;

protected:

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
explicit LineStyleRangeCollector(LsComponentR component, LineStyleSymbR lsSymb, DPoint3d points[2]) 
: m_component(component), m_lsSymb(lsSymb) 
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
    Render::GraphicBuilderPtr  graphic = viewContext.CreateGraphic();
    Render::GraphicParams   defaultParams;
    LineStyleContext lsContext(*graphic, defaultParams, &viewContext);

    m_component._StrokeLineString(*graphic, lsContext, &m_lsSymb, m_points, 2, false);
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
static void Process(DRange3dR range, LsComponentR component, LineStyleSymbR lsSymb, DPoint3d points[2])
    {
    LineStyleRangeCollector  processor(component, lsSymb, points);

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

    m_firstTextureInitialized = false;
    m_texturesNotSupported = false;
    m_usesSymbolWeight = false;
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
    if (scale == 0)
        scale = 1;

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
//! Used to generate a texture based on a line style.
// @bsiclass                                                    John.Gooding    08/2015
//=======================================================================================
struct ComponentToTextureStroker : ComponentStroker
{
private:
    LineStyleSymbR      m_lsSymb;
    Transform           m_transformForTexture;
    ColorDef            m_lineColor;
    ColorDef            m_fillColor;
    uint32_t            m_lineWeight;

public:

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
ComponentToTextureStroker(DgnDbR dgndb, LineStyleSymbR lsSymb, ColorDef lineColor, ColorDef fillColor, uint32_t lineWeight, LsComponentR component) : 
            ComponentStroker(dgndb, component, lsSymb.GetScale()), m_lsSymb(lsSymb), m_lineColor(lineColor), m_fillColor(fillColor), m_lineWeight(lineWeight)
    {
    //  If a modified copy is required, the caller passed the copy. 
    BeAssert(component._IsOkayForTextureGeneration() == LsOkayForTextureGeneration::NoChangeRequired);

#if defined (BENTLEYCONFIG_GRAPHICS_DIRECTX)
    //  This is to compensate for a bug in QV using DirectX. QV creates the mirror image.
    DVec3d normal;
    normal.Init(0, 1, 0);
    DPoint3d zero;
    zero.Zero();
    m_transformForTexture.InitFromMirrorPlane(zero, normal);
#else
    m_transformForTexture.InitIdentity();
#endif
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

    //  Create the graphic
    Render::GraphicBuilderPtr graphic = context.CreateGraphic(Graphic::CreateParams(context.GetViewport(), m_transformForTexture));

    //  Add symbology
    graphic->ActivateGraphicParams(elemMatSymb);
    LineStyleContext lsContext(*graphic, elemMatSymb, &context);
    m_component->_StrokeLineString(*graphic, lsContext, &m_lsSymb, m_points, 2, false);
    graphic->Close();

    return graphic;
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2016
//---------------------------------------------------------------------------------------
static void initializePoints(DPoint3d points[2], LsComponentR component, double scale)
    {
    double length = component._GetLengthForTexture() * NUMBER_ITERATIONS_ComponentStroker;

    if (length <  mgds_fc_epsilon)
        {
        //  Apparently nothing is length dependent.
        length = component._GetMaxWidth(nullptr);
        if (length <  mgds_fc_epsilon)
            length = 2048.00;
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
    if (xRange < 0.0001)
        {
        scaleFactor = (uint32_t)ceil(1/xRange);
        range2d.high.Scale(scaleFactor);
        range2d.low.Scale(scaleFactor);
        xRange = range2d.high.x - range2d.low.x;
        }

   //  QV recommends the range should be either equal or proportional by powers of two (for
   //  example, the width is 2X, 4X, or 8X the height).  The range must be centered around 0 
   //  for the texture to be applied to the line correctly.
    double yVal = xRange/2.0;
    bool changed = false;
    //  Keep doubling while it is both higher than the high and lower than the low.
    while (yVal < range2d.high.y || -yVal > range2d.low.y)
        {
        changed = true;
        yVal *= 2.0;
        }

    if (!changed)
        {
        //  We didn't have to increase the Y range. Maybe we can decrease it.
        while (yVal/2.0 >= range2d.high.y && -yVal/2.0 <= range2d.low.y)
            {
            yVal /= 2.0;
            }
        }

    if (yVal < 2)
        yVal = 2;

    //  Force the range to be centered at 0.  Othwerwise, it won't be centered on the line.
    range2d.low.y = -yVal;
    range2d.high.y = yVal;

    return range2d;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2015
//---------------------------------------------------------------------------------------
StatusInt LsDefinition::GenerateTexture(TextureDescr& textureDescr, ViewContextR viewContext, LineStyleSymbR lineStyleSymbIn, uint32_t weight)
    {
    textureDescr.m_hasTextureWidth = false;
    textureDescr.m_textureWidth = 0;

    double unitDef = GetUnitsDefinition();
    if (unitDef == 0)
        unitDef = 1.0;

    DgnViewportP vp = viewContext.GetViewport();
    if (vp == nullptr)
        return BSIERROR;

    //  We create the texture in the component's units.  Then to convert to the line style's units we set the true width to the component width times unitDef.
    //  It seems more logical to work in the line style's units.  However, the lines in a texture look much better if QV draws them in the component's units. It seems that
    //  scaling down works much better than scaling up when we use the texture.
    //
    //  I expected this approach would be unusable due to line weights get scaled when we use the texture.  Surprisingly, the line weights work properly with this approach.
    LineStyleSymb lineStyleSymb = lineStyleSymbIn;

    lineStyleSymb.SetScale(lineStyleSymb.GetScale()/unitDef);
    if (lineStyleSymb.HasOrgWidth())
        {
        lineStyleSymb.SetWidth(lineStyleSymb.GetOriginWidth()/unitDef);
        //  The caller enforces this when we are generating a texture.
        BeAssert(lineStyleSymb.GetOriginWidth() == lineStyleSymb.GetEndWidth());
        }

    double componentScaleFactor = lineStyleSymb.GetScale();

    //  Assume the caller already knows this is something that must be converted but does not know it can be converted.
    BeAssert(m_lsComp->GetComponentType() != LsComponentType::RasterImage);

    //  A component may cache information used from running texture generation for another component.
    //  If so, reset it now.  _StartTextureGeneration also traverses the children.
    m_lsComp->_StartTextureGeneration();

    if (m_lsComp->_IsOkayForTextureGeneration() == Dgn::LsOkayForTextureGeneration::NotAllowed)
        {
        m_firstTextureInitialized = true;
        m_texturesNotSupported = true;
        return BSIERROR;
        }

    //  Something in the component tree may be modified for texture generation.  For example, the size of an iteration
    //  may be expanded to accomodate a symbol that overflows.
    LsComponentPtr  comp = m_lsComp->_GetForTextureGeneration();
    if (comp.IsNull())
        {
        m_firstTextureInitialized = true;
        m_texturesNotSupported = true;
        return BSIERROR;
        }

    //  Get just the range of the components.  
    DPoint3d  points[2];
    initializePoints(points, *m_lsComp, componentScaleFactor);

    DRange3d  lsRange;
    LineStyleRangeCollector::Process(lsRange, *comp, lineStyleSymb, points);

    uint32_t  scaleFactor = 1;
    DRange2d range2d = getAdjustedRange(scaleFactor, lsRange, componentScaleFactor * comp->_GetLengthForTexture());

    SymbologyQueryResults  symbologyResults;
    comp->_QuerySymbology(symbologyResults);
    ColorDef lineColor, fillColor;
    bool isColorBySymbol = symbologyResults.IsColorBySymbol(lineColor, fillColor) && !symbologyResults.IsColorByLevel();

    uint32_t lineWeight;
    m_usesSymbolWeight = symbologyResults.IsWeightBySymbol(lineWeight);
    if (!m_usesSymbolWeight)
        lineWeight = weight;

    ComponentToTextureStroker   stroker(viewContext.GetDgnDb(), lineStyleSymb, lineColor, fillColor, lineWeight, *comp);
    GraphicPtr graphic = stroker.Stroke(viewContext);

    if (!graphic.IsValid() || viewContext.CheckStop())
        {
        //  If aborted due to checkstop, we want to try again.  Otherwise, we assume it is an unrecoverable error.
        if (!viewContext.CheckStop())
            {
            m_firstTextureInitialized = true;
            m_texturesNotSupported = true;
            }

        return BSIERROR;
        }

    textureDescr.m_texture = vp->GetRenderTarget()->CreateGeometryTexture(*graphic, range2d, isColorBySymbol, false);

    double yRange = range2d.high.y - range2d.low.y;
    BeAssert(0.0 != yRange);
    if (0.0 == yRange)
        yRange = 1;

    textureDescr.m_hasTextureWidth = true;
    textureDescr.m_textureWidth = yRange * unitDef;

    m_firstTextureInitialized = true;

    return BSISUCCESS;    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    05/2016
//---------------------------------------------------------------------------------------
StatusInt LsDefinition::GetGeometryTexture(TextureDescr& tDescr, ViewContextR viewContext, LineStyleSymbR lineStyleSymb, uint32_t weight)
    {
    if (m_usesSymbolWeight)
        weight = 0;   // This line style does not use the current element's weight so there is no sense distinguishing based on weight.

    uint32_t modifiers = 0;
    if (lineStyleSymb.IsScaled())
        modifiers |= STYLEMOD_SCALE;
    if (lineStyleSymb.HasOrgWidth())
        modifiers |= STYLEMOD_SWIDTH;

    //  Dash and gap scales purposely omitted since the MicroStation user interface does not provide any way to set them.

    TextureParams params(weight, modifiers, lineStyleSymb.GetScale(), lineStyleSymb.GetOriginWidth());

    if (m_firstTextureInitialized)
        {
        if (m_texturesNotSupported)
            return BSIERROR;

        ParamsToTexture_t::iterator tDescrIter = m_textures.find(params);
        if (tDescrIter != m_textures.end())
            {
            tDescr = tDescrIter->second;
            return BSISUCCESS;
            }
        }

    if (LsDefinition::GenerateTexture(tDescr,  viewContext, lineStyleSymb, weight) != BSISUCCESS)
        return BSIERROR;

    m_textures[params] = tDescr;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Texture* LsDefinition::GetTexture(ViewContextR viewContext, LineStyleSymbR lineStyleSymb, bool forceTexture, uint32_t weight) 
    {
    if (!m_lsComp.IsValid())
        return nullptr;

    if (m_lsComp->GetComponentType() == LsComponentType::RasterImage)
        {
        if (!m_firstTextureInitialized)
            {
            uint8_t const* image;
            Point2d     imageSize;
            uint32_t      flags = 0;

            m_firstTextureInitialized = true;
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
            }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
        m_hasTextureWidth = m_lsComp->_GetTextureWidth(m_textureWidth) == BSISUCCESS;
        if (m_texture.IsValid() && m_lsComp.IsValid() && m_hasTextureWidth)
            lineStyleSymb.SetWidth (m_textureWidth * __unused);
    
        return m_texture.get();
#endif
        return nullptr;
        }

    if (forceTexture)
        {
        switch(viewContext.GetDrawPurpose())
            {
            case DrawPurpose::CreateScene:
            case DrawPurpose::Progressive:
            case DrawPurpose::Plot:
            case DrawPurpose::Dynamics:
            case DrawPurpose::Redraw:
            case DrawPurpose::Heal:
                {
                BeAssert (weight >=0 && weight < 32);
                TextureDescr    tDescr;
                if (GetGeometryTexture(tDescr, viewContext, lineStyleSymb, weight) != BSISUCCESS)
                    return nullptr;

                //  Do not apply the scaling factor here.  It must be applied when generating the texture, not when applying the texture.
                //  Line widths should not be scaled, but scaling the result texture would scale the line widths.
                if (tDescr.m_texture.IsValid() && tDescr.m_hasTextureWidth)
                    lineStyleSymb.SetWidth (tDescr.m_textureWidth);

                return tDescr.m_texture.get();
                }
                break;
            default:
                //  Very rare to get here. It does happen if checkstop stops us from creating the texture and then the user tries to pick the element.
                return nullptr;
            }
        }

    return nullptr;
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2016
//---------------------------------------------------------------------------------------
bool TextureParams::operator< (struct TextureParams const&rhs) const
    {
    if (m_flags < rhs.m_flags)
        return true;
    if (m_flags > rhs.m_flags)
        return false;

    if (m_lineWeight < rhs.m_lineWeight)
        return true;
    if (m_lineWeight > rhs.m_lineWeight)
        return false;

    if (m_scale < rhs.m_scale)
        return true;
    if (m_scale > rhs.m_scale)
        return false;

    if (m_styleWidth < rhs.m_styleWidth)
        return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2016
//---------------------------------------------------------------------------------------
TextureParams::TextureParams(uint32_t lineWeight, uint32_t flags, double scale, double styleWidth) :
        m_lineWeight(lineWeight), m_flags(flags), m_scale(scale), m_styleWidth(styleWidth)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2016
//---------------------------------------------------------------------------------------
TextureParams::TextureParams() :
        m_lineWeight(0), m_flags(0), m_scale(0), m_styleWidth(0)
    {
    }