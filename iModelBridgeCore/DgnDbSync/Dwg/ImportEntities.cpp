/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportEntities.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct DrawParameters : public IDwgDrawParameters
{
friend struct GeometryFactory;

    // Effective symbology used for child's ByBlock symbology
    struct EffectiveByBlock
        {
        DwgCmEntityColor    m_color;
        DwgDbObjectId       m_linetypeId;
        DwgDbObjectId       m_materialId;
        DwgDbLineWeight     m_weight;
        DwgDbObjectId       m_layerId;
    public:
        void    Set (DwgCmEntityColorCR color, DwgDbObjectIdCR ltype, DwgDbObjectIdCR material, DwgDbLineWeight weight, DwgDbObjectIdCR layer)
            {
            m_color = color;
            m_linetypeId = ltype;
            m_materialId = material;
            m_weight = weight;
            m_layerId = layer;
            }
        void    CopyFrom (EffectiveByBlock const& other)
            {
            m_color = other.m_color;
            m_linetypeId = other.m_linetypeId;
            m_materialId = other.m_materialId;
            m_weight = other.m_weight;
            m_layerId = other.m_layerId;
            }
        };  // EffectiveByBlock

private:
    DwgCmEntityColor    m_color;
    DwgDbObjectId       m_layerId;
    DwgDbObjectId       m_linetypeId;
    std::ptrdiff_t      m_markerId;
    DwgGiFillType       m_filltype;
    DwgGiFillCP         m_fill;
    DwgDbLineWeight     m_weight;
    uint32_t            m_mappedDgnWeight;  // only a performance need
    double              m_linetypeScale;
    double              m_thickness;
    DwgTransparency     m_transparency;
    DwgDbObjectId       m_materialId;
    DwgImporter&        m_dwgImporter;
    DwgDbEntityCP       m_sourceEntity;
    EffectiveByBlock    m_effectiveByBlock;
    GradientSymbPtr     m_gradientFromHatch;
    DwgDbDatabasePtr    m_dwgdb;
    bool                m_isLayerByBlock;
    bool                m_isParentLayerFrozen;
    bool                m_isDisplayed;
    DwgDbObjectId       m_layer0Id;

protected:
void CopyFrom (DrawParameters const& params)
    {
    m_color = params._GetColor ();
    m_layerId = params._GetLayer ();
    m_linetypeId = params._GetLineType ();
    m_materialId = params._GetMaterial ();
    m_transparency = params._GetTransparency ();
    m_weight = params._GetLineWeight ();
    m_filltype = params._GetFillType ();
    m_fill = params._GetFill ();
    m_linetypeScale = params._GetLineTypeScale ();
    m_thickness = params._GetThickness ();
    m_mappedDgnWeight = params.m_mappedDgnWeight;
    m_markerId = params.m_markerId;
    m_sourceEntity = params.m_sourceEntity;
    m_effectiveByBlock.CopyFrom (params.m_effectiveByBlock);
    m_gradientFromHatch = params.m_gradientFromHatch;
    m_dwgdb = params.m_dwgdb;
    m_isLayerByBlock = params.m_isLayerByBlock;
    m_isParentLayerFrozen = params.m_isParentLayerFrozen;
    m_isDisplayed = params.m_isDisplayed;
    m_layer0Id = params.m_layer0Id;
    }

void Initialize (DwgDbEntityCR ent, DrawParameters const* parentParams = nullptr, DwgDbEntityCP templateEntity = nullptr)
    {
    m_color = ent.GetEntityColor ();
    m_layerId = ent.GetLayerId ();
    if (!m_layerId.IsValid() && nullptr != templateEntity)
        m_layerId = templateEntity->GetLayerId ();
    m_linetypeId = ent.GetLinetypeId ();
    if (!m_linetypeId.IsValid() && nullptr != templateEntity)
        m_linetypeId = templateEntity->GetLinetypeId ();
    m_materialId = ent.GetMaterialId ();
    if (!m_materialId.IsValid() && nullptr != templateEntity)
        m_materialId = templateEntity->GetMaterialId ();
    m_transparency = ent.GetTransparency ();
    m_weight = ent.GetLineweight ();
    m_mappedDgnWeight = m_dwgImporter.GetOptions().GetDgnLineWeight (m_weight);
    m_markerId = -1;
    m_filltype = DwgGiFillType::Default;
    m_fill = nullptr;
    m_linetypeScale = ent.GetLinetypeScale ();
    m_thickness = this->InitThicknessFromEntity(ent);
    m_sourceEntity = &ent;
    m_dwgdb = ent.GetDatabase ();

    // if this is a new entity, use template DWG or fallback to master file:
    if (m_dwgdb.IsNull() && nullptr != templateEntity)
        m_dwgdb = templateEntity->GetDatabase ();
    if (m_dwgdb.IsNull())
        m_dwgdb = &m_dwgImporter.GetDwgDb ();

    // layer0 is a "layer by block":
    if (!m_dwgdb.IsNull())
        {
        m_layer0Id = m_dwgdb->GetLayer0Id ();
        m_isLayerByBlock = m_layerId == m_layer0Id;
        }
    else
        {
        // handle error
        m_layer0Id.SetNull ();
        m_isLayerByBlock = false;
        }

    if (nullptr == parentParams)
        m_effectiveByBlock.Set (m_color, m_linetypeId, m_materialId, m_weight, m_layerId);
    else
        this->ResolveEffectiveByBlockSymbology (*parentParams);

    this->SetGradientColorFromHatch (DwgDbHatch::Cast(m_sourceEntity));
    this->ResolveDisplayStatus (parentParams);
    }

double  InitThicknessFromEntity (DwgDbEntityCR ent)
    {
    // thickness is based on entity type:
    DwgDbArcCP          arc;
    DwgDbCircleCP       circle;
    DwgDb2dPolylineCP   pline2d;
    DwgDbPolylineCP     pline;
    DwgDbLineCP         line;
    DwgDbPointCP        point;
    DwgDbSolidCP        solid;
    DwgDbTextCP         text;
    DwgDbTraceCP        trace;

    if (nullptr != (arc = DwgDbArc::Cast(&ent)))                    return GETTHICKNESS_From(arc);
    else if (nullptr != (circle = DwgDbCircle::Cast(&ent)))         return GETTHICKNESS_From(circle);
    else if (nullptr != (pline2d = DwgDb2dPolyline::Cast(&ent)))    return GETTHICKNESS_From(pline2d);
    else if (nullptr != (pline = DwgDbPolyline::Cast(&ent)))        return GETTHICKNESS_From(pline);
    else if (nullptr != (line = DwgDbLine::Cast(&ent)))             return GETTHICKNESS_From(line);
    else if (nullptr != (point = DwgDbPoint::Cast(&ent)))           return GETTHICKNESS_From(point);
    else if (nullptr != (solid = DwgDbSolid::Cast(&ent)))           return GETTHICKNESS_From(solid);
    else if (nullptr != (trace = DwgDbTrace::Cast(&ent)))           return GETTHICKNESS_From(trace);
    else if (nullptr != (text = DwgDbText::Cast(&ent)))             return GETTHICKNESS_From(text);

    return  0.0;
    }

public:
    DrawParameters (DrawParameters const& params) : m_dwgImporter(params.m_dwgImporter)
        {
        this->CopyFrom (params);
        }
    DrawParameters (DwgDbEntityCR ent, DwgImporter& importer, DwgDbEntityCP parent = nullptr, DwgDbEntityCP templateEntity = nullptr) : m_dwgImporter(importer)
        {
        if (nullptr != parent)
            {
            DrawParameters  parentParams(*parent, importer);
            this->Initialize (ent, &parentParams, templateEntity);
            return;
            }
        this->Initialize (ent, nullptr, templateEntity);
        }

// methods called from DwgDb
virtual DwgCmEntityColorCR  _GetColor () const override { return m_color; }
virtual DwgDbObjectIdCR     _GetLayer () const override { return m_layerId; }
virtual DwgDbObjectIdCR     _GetLineType () const override { return m_linetypeId; }
virtual DwgGiFillType       _GetFillType () const override { return m_filltype; }
virtual DwgGiFillCP         _GetFill () const override { return m_fill; }
virtual DwgDbLineWeight     _GetLineWeight () const override { return m_weight; }
virtual double              _GetLineTypeScale () const override { return m_linetypeScale; }
virtual double              _GetThickness () const override { return m_thickness; }
virtual DwgTransparencyCR   _GetTransparency () const override { return m_transparency; }
virtual DwgDbObjectIdCR     _GetMaterial () const override { return m_materialId; }
virtual void _SetColor (DwgCmEntityColorCR color) override { m_color = color; }
virtual void _SetLayer (DwgDbObjectIdCR layerId) override { m_layerId = layerId; }
virtual void _SetLineType (DwgDbObjectIdCR linetypeId) override { m_linetypeId = linetypeId; }
virtual void _SetSelectionMarker (std::ptrdiff_t markerId) override { m_markerId = markerId; }
virtual void _SetFillType (DwgGiFillType filltype) override { m_filltype = filltype; }
virtual void _SetFill (DwgGiFillCP fill) override { m_fill = fill; }
virtual void _SetLineWeight (DwgDbLineWeight weight) override { m_weight = weight; m_mappedDgnWeight = m_dwgImporter.GetOptions().GetDgnLineWeight(weight); }
virtual void _SetLineTypeScale (double scale) override { m_linetypeScale = scale; }
virtual void _SetThickness (double thickness) override { m_thickness = thickness; }
virtual void _SetTransparency (DwgTransparency transparency) override { m_transparency = transparency; }
virtual void _SetMaterial (DwgDbObjectIdCR materialId) override { m_materialId = materialId; }

// methods called from DwgImporter
DwgDbEntityCP   GetSourceEntity () const { return m_sourceEntity; }
DwgDbDatabaseP  GetDatabase () { return m_dwgdb.get(); }
DwgImporter&    GetDwgImporter () { return  m_dwgImporter; }
bool            IsDisplayed () { return m_isDisplayed; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            SetGradientColorFromHatch (DwgDbHatchCP hatch)
    {
    /*-----------------------------------------------------------------------------------
    When we draw a hatch entity with gradient fill with, two things happen:

    1) The world/viewportDraw does not call _SetFill for sub-entity, so we have explicitly 
        extract its gradient fill before drawing it.
    2) The world/viewportDraw creates mesh, which is a undesired geometry for a fill element.
        The caller shall re-set the source hatch entity to a solid fill so we will get a
        simple shape from the hatch.

    To workaround both issues above, we extract gradient fill from the hatch, convert it
    as DGN gradient color, and save the converted gradient in this DrawParameters, which
    will be used later to set display parameters.  To prevent mesh created by a toolkit,
    remove the gradient by setting it to a simple solids fill.  This step should done by
    the caller.
    -----------------------------------------------------------------------------------*/
    if (nullptr != hatch)
        {
        DwgGiGradientFillPtr    dwgGradient = DwgGiGradientFill::CreateFrom (hatch);
        if (!dwgGradient.IsNull() && !(m_gradientFromHatch = GradientSymb::Create()).IsNull())
            DwgHelper::GetDgnGradientColor (*m_gradientFromHatch.get(), *dwgGradient);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ResolveDisplayStatus (DrawParameters const* parentParams)
    {
    DwgDbLayerTableRecordPtr    layer;

    m_isParentLayerFrozen = false;
    m_isDisplayed = true;

    // first check parent's layer status:
    if (nullptr != parentParams)
        {
        if (DwgDbStatus::Success == layer.OpenObject(parentParams->m_layerId, DwgDbOpenMode::ForRead))
            m_isParentLayerFrozen = layer->IsFrozen();
        // a frozen parent layer turns off all children, except when the parent layer is "0":
        if ((parentParams->m_isParentLayerFrozen || m_isParentLayerFrozen) && !parentParams->m_isLayerByBlock)
            m_isDisplayed = false;
        }

    // if the parent layer has effectively frozen this entity, do not display it:
    if (!m_isDisplayed)
        return;

    // resolve layer "0" to parent's display status:
    if (m_isLayerByBlock && nullptr != parentParams)
        {
        m_isDisplayed = parentParams->m_isDisplayed;
        return;
        }

    // then check this entity's layer status:
    DwgDbObjectId   layerId = m_isLayerByBlock ? m_effectiveByBlock.m_layerId : m_layerId;
    if (DwgDbStatus::Success == layer.OpenObject(layerId, DwgDbOpenMode::ForRead))
        m_isDisplayed = !layer->IsFrozen() && !layer->IsOff();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ResolveRootEffectiveByBlockSymbology ()
    {
    // This is the root entity to which all nested sub-entities may resolve their ByBlock symbologies.
    if (m_effectiveByBlock.m_color.IsByLayer())
        this->GetColorFromLayer (m_effectiveByBlock.m_color);
    else if (m_effectiveByBlock.m_color.IsByBlock())
        m_effectiveByBlock.m_color.SetColorIndex (255);   // White to indicate an orphaned ByBlock color

    if (!m_dwgdb.IsNull())
        {
        if (m_effectiveByBlock.m_linetypeId == m_dwgdb->GetLinetypeByBlockId())
            m_effectiveByBlock.m_linetypeId = m_dwgdb->GetLinetypeContinuousId ();
        else if (m_effectiveByBlock.m_linetypeId == m_dwgdb->GetLinetypeByLayerId())
            this->GetLinetypeFromLayer (m_effectiveByBlock.m_linetypeId);

        if (m_effectiveByBlock.m_materialId == m_dwgdb->GetMaterialByBlockId())
            m_effectiveByBlock.m_materialId = m_dwgdb->GetMaterialGlobalId ();
        else if (m_effectiveByBlock.m_materialId == m_dwgdb->GetMaterialByLayerId())
            this->GetMaterialFromLayer (m_effectiveByBlock.m_materialId);
        }

    if (m_effectiveByBlock.m_weight == DwgDbLineWeight::WeightByLayer)
        m_effectiveByBlock.m_weight = this->GetWeightFromLayer ();
    else if (m_effectiveByBlock.m_weight == DwgDbLineWeight::WeightByBlock)
        m_effectiveByBlock.m_weight = DwgDbLineWeight::Weight000;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ResolveEffectiveByBlockSymbology (DrawParameters const& newParent)
    {
    /*-----------------------------------------------------------------------------------
    If current drawable uses a ByBlock symbology, we resolve effective ByBlock symbology
    from the input parent symbology, such that the drawable's children will be able to
    apply their ByBlock symbologies.

    Otherwise, we simply apply the drawable's symbology as the effective ByBlock symbology
    for the children.

    If current drawable uses a ByLayer symbology and the layer is "0", aka as a layer ByBlock,
    we resolve the layer to parent's layer, and in a resursive mannar.  That is, if the 
    parent's layer is "0", resolve it up one more level to grandparent's layer.  In a deeply 
    nested block, our ByBlock layer should be the last layer that is not layer "0".  If all 
    children are on layer "0", it should resolve all the way up to the layer of the root 
    block reference in the model/paper space.  Even if the root entity is on layer "0", 
    its symbology is still effective.
    -----------------------------------------------------------------------------------*/
    if (!m_effectiveByBlock.m_layerId.IsValid() || (m_isLayerByBlock && newParent.m_layerId != m_layer0Id))
        m_effectiveByBlock.m_layerId = newParent.m_layerId;

#ifdef DEBUG_EFFECTIVE_BYBLOCK
    if (true)
        {
        DwgDbLayerTableRecordPtr    layer(m_effectiveByBlock.m_layerId, DwgDbOpenMode::ForRead);
        DwgDbLayerTableRecordPtr    entLayer(m_layerId, DwgDbOpenMode::ForRead);
        LOG.debugv ("effective ByBlock layer \"%ls\" for layer \"%ls\"[%s]...", layer->GetName().c_str(), entLayer->GetName().c_str(), m_isLayerByBlock ? "layer ByBlock" : "not ByBlock");
        }
#endif

    if (this->IsColorByBlock())
        {
        // resolve color for the children using ByBlock
        if (newParent.m_color.IsByLayer() || (m_color.IsByLayer() && m_isLayerByBlock))
            {
            newParent.GetColorFromLayer (m_effectiveByBlock.m_color);
            }
        else if (newParent.m_color.IsByBlock())
            {
            // resolve to grandparent's color
            if (newParent.m_effectiveByBlock.m_color.IsByBlock())
                m_effectiveByBlock.m_color.SetColorIndex (255);
            else if (newParent.m_effectiveByBlock.m_color.IsByLayer())
                newParent.GetColorFromLayer (m_effectiveByBlock.m_color);
            else
                m_effectiveByBlock.m_color = newParent.m_effectiveByBlock.m_color;
            }
        else
            {
            // true color
            m_effectiveByBlock.m_color = newParent.m_color;
            }
        }
    else
        {
        // apply drawable's color as the effective ByBlock for the children
        m_effectiveByBlock.m_color = m_color;
        }

    if (!m_dwgdb.IsNull() && this->IsLinetypeByBlock())
        {
        // resolve linetyle for the children using ByBlock
        DwgDbObjectId   linetypeByLayer = m_dwgdb->GetLinetypeByLayerId ();
        DwgDbObjectId   linetypeByBlock = m_dwgdb->GetLinetypeByBlockId ();

        if (newParent.m_linetypeId == linetypeByLayer || (m_linetypeId == linetypeByLayer && m_isLayerByBlock))
            {
            newParent.GetLinetypeFromLayer (m_effectiveByBlock.m_linetypeId);
            }
        else if (newParent.m_linetypeId == linetypeByBlock)
            {
            // resolve to grandparent's linetype
            if (newParent.m_effectiveByBlock.m_linetypeId == linetypeByBlock)
                m_effectiveByBlock.m_linetypeId = m_dwgdb->GetLinetypeContinuousId ();
            else if (newParent.m_effectiveByBlock.m_linetypeId == linetypeByLayer)
                newParent.GetLinetypeFromLayer (m_effectiveByBlock.m_linetypeId);
            else
                m_effectiveByBlock.m_linetypeId = newParent.m_effectiveByBlock.m_linetypeId;
            }
        else
            {
            m_effectiveByBlock.m_linetypeId = newParent.m_linetypeId;
            }
        }
    else
        {
        // apply drawable's linetype as the effective ByBlock for the children
        m_effectiveByBlock.m_linetypeId = m_linetypeId;
        }

    if (!m_dwgdb.IsNull() && this->IsMaterialByBlock())
        {
        // resolve material for the children using ByBlock
        DwgDbObjectId   materialByLayer = m_dwgdb->GetMaterialByLayerId ();
        DwgDbObjectId   materialByBlock = m_dwgdb->GetMaterialByBlockId ();

        if (newParent.m_materialId == materialByLayer || (m_materialId == materialByLayer && m_isLayerByBlock))
            {
            newParent.GetMaterialFromLayer (m_effectiveByBlock.m_materialId);
            }
        else if (newParent.m_materialId == materialByBlock)
            {
            // resolve to grandparent's material
            if (newParent.m_effectiveByBlock.m_materialId == materialByBlock)
                m_effectiveByBlock.m_materialId = m_dwgdb->GetMaterialGlobalId ();
            else if (newParent.m_effectiveByBlock.m_materialId == materialByLayer)
                newParent.GetMaterialFromLayer (m_effectiveByBlock.m_materialId);
            else
                m_effectiveByBlock.m_materialId = newParent.m_effectiveByBlock.m_materialId;
            }
        else
            {
            m_effectiveByBlock.m_materialId = newParent.m_materialId;
            }
        }
    else
        {
        // apply drawable's material as the effective ByBlock for the children
        m_effectiveByBlock.m_materialId = m_materialId;
        }

    if (this->IsWeightByBlock())
        {
        // resolve lineweight for children using ByBlock
        if (newParent.m_weight == DwgDbLineWeight::WeightByLayer || (m_weight == DwgDbLineWeight::WeightByLayer && m_isLayerByBlock))
            {
            m_effectiveByBlock.m_weight = newParent.GetWeightFromLayer ();
            }
        else if (newParent.m_weight == DwgDbLineWeight::WeightByBlock)
            {
            // resolve to grandparent's weight
            if (newParent.m_effectiveByBlock.m_weight == DwgDbLineWeight::WeightByBlock)
                m_effectiveByBlock.m_weight = DwgDbLineWeight::Weight000;
            else if (newParent.m_effectiveByBlock.m_weight == DwgDbLineWeight::WeightByLayer)
                m_effectiveByBlock.m_weight = newParent.GetWeightFromLayer ();
            else
                m_effectiveByBlock.m_weight = newParent.m_effectiveByBlock.m_weight;
            }
        else
            {
            m_effectiveByBlock.m_weight = newParent.m_weight;
            }
        }
    else
        {
        // apply drawable's weight as the effective ByBlock for the children
        m_effectiveByBlock.m_weight = m_weight;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsColorByBlock () const
    {
    // resolve ByLayer color of layer0 to block's effective color:
    bool    isColorByBlock = m_color.IsByBlock ();
    if (!isColorByBlock)
        isColorByBlock = m_isLayerByBlock && m_color.IsByLayer();

    return  isColorByBlock;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsLinetypeByBlock () const
    {
    // resolve ByLayer linetype of layer0 to block's effective linetype:
    bool    isLinetypeByBlock = !m_dwgdb.IsNull() && m_linetypeId == m_dwgdb->GetLinetypeByBlockId();
    if (!isLinetypeByBlock)
        isLinetypeByBlock = m_isLayerByBlock && !m_dwgdb.IsNull() && m_linetypeId == m_dwgdb->GetLinetypeByLayerId();

    return  isLinetypeByBlock;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsMaterialByBlock () const
    {
    // resolve ByLayer material of layer0 to block's effective material:
    bool    isMaterialByBlock = !m_dwgdb.IsNull() && m_materialId == m_dwgdb->GetMaterialByBlockId();
    if (!isMaterialByBlock)
        isMaterialByBlock = !m_dwgdb.IsNull() && m_materialId == m_dwgdb->GetMaterialByLayerId();

    return  isMaterialByBlock;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IsWeightByBlock () const
    {
    // resolve ByLayer weight of layer0 to block's effective weight:
    bool    isWeightByBlock = m_weight == DwgDbLineWeight::WeightByBlock;
    if (!isWeightByBlock)
        isWeightByBlock = m_weight == DwgDbLineWeight::WeightByLayer;

    return  isWeightByBlock;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Render::FillDisplay GetFillDisplay () const
    {
    /*-----------------------------------------------------------------------------------
    Get fill display from m_fillType, which may be set by an object enabler.
    A LWPolyline is never filled, but its OE may call _SetFill (unexpected though)!
    -----------------------------------------------------------------------------------*/
    double width = 0.0;
    auto lwpline = DwgDbPolyline::Cast (this->GetSourceEntity());
    if (nullptr != lwpline && (!lwpline->GetConstantWidth(width) || width < 1.0e-5))
        return  Render::FillDisplay::Never;

    return DwgGiFillType::Always == m_filltype ? Render::FillDisplay::Always : Render::FillDisplay::Never;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    GetDisplayParams (Render::GeometryParams& params)
    {
    /*-----------------------------------------------------------------------------------
    Convert sub-entity traits to Render::GeometryParams except for category & sub-category.
    Caller needs to set category and sub-category before calling this method, which should 
    clear all appearance overrides, such that sub-category symbology become effective.  
    This method adds an override to use explicitly use a resolved symbology except for
    ByLayer cases that can be translated to "using sub-category appearance".
    -----------------------------------------------------------------------------------*/

    ColorDef    dgnColor;
    bool        colorResolved = false;
    if (!this->CanUseByLayer(m_color.IsByLayer()))
        {
        // set effective color
        dgnColor = this->GetDgnColor ();
        params.SetLineColor (dgnColor);
        colorResolved = true;
        }

    // set effective fill color
    params.SetFillDisplay (this->GetFillDisplay());
    if (params.GetFillDisplay() != Render::FillDisplay::Never)
        {
        GradientSymbPtr     dgnGradient = GradientSymb::Create ();
        DwgGiGradientFillCP dwgGradient = dynamic_cast<DwgGiGradientFillCP> (m_fill);
        if (nullptr != dwgGradient && dgnGradient.IsValid())
            {
            // apply gradient fill set by sub-entity's draw method:
            DwgHelper::GetDgnGradientColor (*dgnGradient.get(), *dwgGradient);
            params.SetGradient (dgnGradient.get());
            }
        else if (m_gradientFromHatch.IsValid())
            {
            // apply gradient fill from pre-saved gradient hatch entity:
            params.SetGradient (m_gradientFromHatch.get());
            }
        else
            {
            // apply solid fill
            if (!colorResolved)
                dgnColor = this->GetDgnColor ();
            params.SetFillColor (dgnColor);
            }
        }

    if (!this->CanUseByLayer(m_dwgdb.IsValid() && m_dwgdb->GetLinetypeByLayerId() == m_linetypeId))
        {
        // set effective line style
        bool                isContinuous = false;
        double              scale = 1.0;
        DgnStyleId          styleId = this->GetDgnLineStyle (isContinuous, scale);
        LineStyleParams     lsParams;
        lsParams.Init ();
        // apply line style scale:
        if (!isContinuous && scale > 0.0 && fabs(scale - 1.0) > 1.0e-3)
            lsParams.SetScale (scale);
        LineStyleInfoPtr    dgnStyle = LineStyleInfo::Create (styleId, &lsParams);
        params.SetLineStyle (dgnStyle.get());
        }

    if (!this->CanUseByLayer(m_weight == DwgDbLineWeight::WeightByLayer))
        {
        // set effective line weight
        uint32_t    dgnWeight = this->GetDgnWeight ();
        params.SetWeight (dgnWeight);
        }

    if (!this->CanUseByLayer(m_transparency.IsByLayer()))
        {
        // set effective transparency
        double      transparency = DwgHelper::GetTransparencyFromDwg (m_transparency, &m_layerId, nullptr);
        params.SetTransparency (transparency);
        if (params.GetFillDisplay() != Render::FillDisplay::Never)
            params.SetFillTransparency (transparency);
        }

    if (!this->CanUseByLayer(m_dwgdb.IsValid() && m_dwgdb->GetMaterialByLayerId() == m_materialId))
        {
        // set effective material
        RenderMaterialId   materialId = this->GetDgnMaterial ();
        params.SetMaterialId (materialId);
        }

    params.SetGeometryClass (Render::DgnGeometryClass::Primary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef        GetDgnColor ()
    {
    DwgCmEntityColor    effectiveColor;

    if (this->IsColorByBlock())
        effectiveColor = m_effectiveByBlock.m_color;
    else if (m_color.IsByLayer())
        this->GetColorFromLayer (effectiveColor);
    else
        effectiveColor = m_color;

    if (effectiveColor.IsByACI())
        return  DwgHelper::GetColorDefFromACI (effectiveColor.GetIndex()) ;

    return ColorDef (effectiveColor.GetRed(), effectiveColor.GetGreen(), effectiveColor.GetBlue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GetColorFromLayer (DwgCmEntityColorR colorOut) const
    {
    DwgDbObjectId   layerId = (m_isLayerByBlock && m_effectiveByBlock.m_layerId.IsValid()) ? m_effectiveByBlock.m_layerId : m_layerId;

    DwgDbLayerTableRecordPtr    layer(layerId, DwgDbOpenMode::ForRead);
    if (!layer.IsNull())
        {
        colorOut = layer->GetColor ();
#ifdef DEBUG_BYLAYER_COLOR
        LOG_ENTITY.debugv ("using color=%d from layer \"%ls\"...", colorOut.GetIndex(), layer->GetName().c_str());
#endif
        return  true;
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        GetDgnWeight ()
    {
    DwgDbLineWeight effectiveWeight = DwgDbLineWeight::Weight000;

    if (this->IsWeightByBlock())
        effectiveWeight = m_effectiveByBlock.m_weight;
    else if (DwgDbLineWeight::WeightByLayer == m_weight)
        effectiveWeight = this->GetWeightFromLayer ();
    else
        effectiveWeight = m_weight;

    if (DwgDbLineWeight::WeightByDefault == effectiveWeight)
        effectiveWeight = DwgDbLineWeight::Weight000;

    return  m_dwgImporter.GetOptions().GetDgnLineWeight (effectiveWeight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbLineWeight GetWeightFromLayer () const
    {
    DwgDbObjectId   layerId = (m_isLayerByBlock && m_effectiveByBlock.m_layerId.IsValid()) ? m_effectiveByBlock.m_layerId : m_layerId;

    DwgDbLayerTableRecordPtr    layer(layerId, DwgDbOpenMode::ForRead);
    if (!layer.IsNull())
        return layer->GetLineweight ();

    return  DwgDbLineWeight::Weight000;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyleId      GetDgnLineStyle (bool& isContinuous, double& effectiveScale) const
    {
    DwgDbObjectId       effectiveLinetype = m_linetypeId;

    if (!m_dwgdb.IsNull() && this->IsLinetypeByBlock())
        {
        if (m_linetypeId == m_dwgdb->GetLinetypeByLayerId())
            this->GetLinetypeFromLayer (effectiveLinetype);
        else if (m_linetypeId == m_dwgdb->GetLinetypeByBlockId())
            effectiveLinetype = m_effectiveByBlock.m_linetypeId;
        else
            effectiveLinetype = m_linetypeId;
        }

    isContinuous = m_dwgdb.IsNull() || effectiveLinetype == m_dwgdb->GetLinetypeContinuousId();

    if (isContinuous)
        effectiveScale = 1.0;
    else
        effectiveScale = this->GetEffectiveLinetypeScale ();

    if (effectiveLinetype.IsValid())
        return  m_dwgImporter.GetDgnLineStyleFor (effectiveLinetype);

    return  DgnStyleId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GetLinetypeFromLayer (DwgDbObjectIdR ltypeOut) const
    {
    DwgDbObjectId   layerId = (m_isLayerByBlock && m_effectiveByBlock.m_layerId.IsValid()) ? m_effectiveByBlock.m_layerId : m_layerId;

    DwgDbLayerTableRecordPtr    layer(layerId, DwgDbOpenMode::ForRead);
    if (!layer.IsNull())
        {
        ltypeOut = layer->GetLinetypeId ();
        return  true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          GetEffectiveLinetypeScale () const
    {
    /*-----------------------------------------------------------------------------------
    DGN linestyles we have created from DWG linetype table are only applied by LTSCALE.
    Check all below rules to apply a required scale:

        1) Modelspace annotation scale is in effect when MSLTSCALE=1
        2) Paperspace annotation scale is in effect when PSLTSCALE=1
        3) This instance scale, m_linetypeScale, but possibly compounded by 1) or 2).
    -----------------------------------------------------------------------------------*/
    double          scale = m_linetypeScale;
    DwgDbObjectId   ownerId;
    if (nullptr != m_sourceEntity && !(ownerId = m_sourceEntity->GetOwnerId()).IsNull())
        {
        // WIP - modelspace entities in paperspace viewports are affected by viewport scales

        DwgDbDatabaseR  dwg = m_dwgImporter.GetDwgDb ();

        // modelspace entities in modelspace views are affected by CANNOSCALE
        if (ownerId == m_dwgImporter.GetModelSpaceId())
            {
            if (dwg.GetMSLTSCALE())
                scale /= dwg.GetCANNOSCALE ();
            }
        }

    return  scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMaterialId   GetDgnMaterial () const
    {
    DwgDbObjectId       effectiveMaterial = m_materialId;

    if (!m_dwgdb.IsNull() && this->IsMaterialByBlock())
        {
        // try to resolve ByLayer & ByBlock materials
        if (m_materialId == m_dwgdb->GetMaterialByLayerId())
            this->GetMaterialFromLayer (effectiveMaterial);
        else if (m_materialId == m_dwgdb->GetMaterialByBlockId())
            effectiveMaterial = m_effectiveByBlock.m_materialId;
        else
            effectiveMaterial = m_materialId;
        }

    if (!effectiveMaterial.IsValid() && !m_dwgdb.IsNull())
        effectiveMaterial == m_dwgdb->GetMaterialGlobalId();

    if (effectiveMaterial.IsValid())
        return  m_dwgImporter.GetDgnMaterialFor (effectiveMaterial);

    return  RenderMaterialId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GetMaterialFromLayer (DwgDbObjectIdR materialOut) const
    {
    DwgDbLayerTableRecordPtr    layer(m_layerId, DwgDbOpenMode::ForRead);
    if (!layer.IsNull())
        {
        materialOut = layer->GetMaterialId ();
        return  materialOut.IsValid();
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId   GetEffectiveLayerId () const
    {
    DwgDbObjectId   layerId = m_isLayerByBlock ? m_effectiveByBlock.m_layerId : m_layerId;
    return  layerId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            CanUseByLayer (bool isByLayer)
    {
    // check if a ByLayer can be translated to "using sub-category appearance":
    if (!isByLayer)
        return  false;

    // if the layer does not float to the parent layer, we can apply ByLayer:
    if (!m_isLayerByBlock)
        return  true;

    // we are on layer "0" - only allow entities directly placed in modelspace or a paperspace for ByLayer:
    if (nullptr != m_sourceEntity)
        {
        // check modelspace of the master file:
        DwgDbObjectId   ownerId = m_sourceEntity->GetOwnerId ();
        if (m_dwgImporter.GetModelSpaceId() == ownerId)
            return  true;

        // check a paperspace(a paperspace should have never made into an xref):
        DwgDbBlockTableRecordPtr  block(ownerId, DwgDbOpenMode::ForRead);
        if (block.OpenStatus() == DwgDbStatus::Success)
            return  block->IsLayout();
        }

    // do not allow ByLayer for all other cases!
    return  false;
    }

};  // DrawParameters


/*=================================================================================**//**
* GeometryFactory collects geometries from the entity draw callback of a working toolkit, 
* and caches them into a local per block-geometry map.  A separate block stack helps tracking 
* & setting the effective block that is currently being drawn by the toolkit.  The nested 
* The flat block-geometry map does not capture the nestedness of blocks, but is more 
* efficient for building a geometry map for shared geometry parts used in next step.  
* When a same nested block is drawn more than once, the output map will have "duplicated" 
* geometries per block.  It is for this reason, only the outermost block, i.e. all geometries 
* collected for the root block from which this class is instantiated can be used for importer's 
* block-geometry map reuse purpose.
*
* @see ElementFactory
*
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct GeometryFactory : public IDwgDrawGeometry
{
    // Block information saved off in a stack to track currently drawn block.
    struct BlockInfo
        {
    private:
        DwgDbObjectId           m_blockId;
        Utf8String              m_blockName;
        DwgSyncInfo::DwgFileId  m_fileId;
    public:
        BlockInfo (DwgDbObjectIdCR id, DwgStringCR name, DwgSyncInfo::DwgFileId file) : m_blockId(id), m_fileId(file)
            {
            m_blockName.Assign (name.c_str());
            }
        DwgDbObjectIdCR GetBlockId () const { return m_blockId; }
        Utf8StringCR    GetBlockName () const { return m_blockName; }
        DwgSyncInfo::DwgFileId  GetFileId () const { return m_fileId; }
        };  // BlockInfo

private:
    // members used by the toolkit
    bvector<Transform>                  m_transformStack;
    Transform                           m_baseTransform;
    Transform                           m_currentTransform;
    DrawParameters&                     m_drawParams;
    // members used locally
    DwgImporter::ElementCreateParams&   m_createParams;
    DwgImporter::GeometryOptions*       m_geometryOptions;
    DwgImporter::T_BlockGeometryMap     m_outputGeometryMap;
    DwgDbEntityCP                       m_entity;
    BentleyStatus                       m_status;
    Transform                           m_worldToElement;
    DwgDbSpatialFilterCP                m_spatialFilter;
    bvector<BlockInfo>                  m_blockStack;
    bvector<int64_t>                    m_parasolidBodies;
    bool                                m_isTargetModel2d;
    
public:
// the constructor
GeometryFactory (DwgImporter::ElementCreateParams& createParams, DrawParameters& drawParams, DwgImporter::GeometryOptions& opts, DwgDbEntityCP ent) : m_drawParams(drawParams), m_createParams(createParams)
    {
    m_geometryOptions = &opts;
    m_entity = ent;
    m_status = BSISUCCESS;
    m_transformStack.clear ();
    // will create geometries local to the input entity
    m_baseTransform.InitIdentity ();
    m_currentTransform = m_baseTransform;
    m_worldToElement.InitIdentity ();
    m_spatialFilter = nullptr;
    m_parasolidBodies.clear ();
    m_isTargetModel2d = !createParams.GetModel().Is3d ();

    // start block stack by input entity's block
    auto dwg = nullptr == ent ? m_drawParams.GetDatabase() : ent->GetDatabase().get();
    auto blockId = nullptr == ent ? dwg->GetModelspaceId() : ent->GetOwnerId();
    m_blockStack.push_back (BlockInfo(blockId, L"ModelSpace", DwgSyncInfo::GetDwgFileId(*dwg)));
    }

// the destructor
~GeometryFactory ()
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    auto nBreps = m_parasolidBodies.size ();
    if (nBreps > 0)
        ::PK_ENTITY_delete ((int)nBreps, reinterpret_cast<PK_ENTITY_t*>(&m_parasolidBodies.front()));
#endif
    }

DwgImporter::T_BlockGeometryMap& GetOutputGeometryMap () { return m_outputGeometryMap; }
BentleyStatus   GetStatus () { return m_status; }

// tracking block drawing - all blocks.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    PushDrawingBlock (DwgDbBlockTableRecordCR block)
    {
    auto dwg = block.GetDatabase ();
    if (!dwg.IsValid())
        dwg = m_drawParams.GetDatabase ();
    BeAssert (dwg.IsValid());
    BlockInfo entry (block.GetObjectId(), block.GetName(), DwgSyncInfo::GetDwgFileId(*dwg));
    m_blockStack.push_back (entry);
    }
bool        IsDrawingBlock () { return m_blockStack.size() > 1; }
void        PopDrawingBlock () { m_blockStack.pop_back(); }
BlockInfo&  GetCurrentBlock () { return m_blockStack.back(); }
void        SetSpatialFilter (DwgDbSpatialFilterCP filter) { m_spatialFilter = filter; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ApplyThickness (ICurvePrimitivePtr const& curvePrimitive, DVec3dCR normal, bool closed = false)
    {
    if (m_isTargetModel2d)
        return  false;

    double  thickness = m_drawParams._GetThickness ();

    if (ISVALID_Thickness(thickness) && curvePrimitive.IsValid())
        {
        CurveVectorPtr  curveVector = CurveVector::Create (curvePrimitive);
        if (curveVector.IsValid())
            {
            DgnExtrusionDetail  profile(curveVector, DVec3d::FromScale(normal, thickness), closed);
            ISolidPrimitivePtr  solid = ISolidPrimitive::CreateDgnExtrusion (profile);
            if (solid.IsValid())
                {
                GeometricPrimitivePtr   extruded = GeometricPrimitive::Create (solid.get());

                this->AppendGeometry (*extruded.get());
                return  true;
                }
            }
        }
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ApplyThickness (GeometricPrimitivePtr const& geomPrimitive, DVec3dCR normal, bool closed = false)
    {
    double  thickness = m_drawParams._GetThickness ();

    if (ISVALID_Thickness(thickness) && geomPrimitive.IsValid())
        return  this->ApplyThickness (geomPrimitive->GetAsICurvePrimitive(), normal, closed);

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ApplyColorOverride (DwgGiFaceDataCP faceData, DwgGiEdgeDataCP edgeData)
    {
    /*-----------------------------------------------------------------------------------
    Shell and mesh may have face and/or edge data overrides. Apply a color override from 
    the first face or the first edge - ignore varying colors for now.
    -----------------------------------------------------------------------------------*/
    if (nullptr == faceData && nullptr == edgeData)
        return;

    DwgCmEntityColor    faceColor, edgeColor;
    bool    hasFaceColor = false, hasEdgeColor = false;
    if (nullptr != faceData)
        {
        DwgCmEntityColorCP  trueColors = faceData->GetTrueColors ();
        int16_t const*      indexColors = faceData->GetColors ();
        if (nullptr != trueColors)
            {
            // face overridden by the true color of the first face
            faceColor = trueColors[0];
            hasFaceColor = true;
            }
        else if (nullptr != indexColors)
            {
            // face overridden by index color of the first face
            faceColor.SetColorIndex (indexColors[0]);
            hasFaceColor = true;
            }
        }
    if (nullptr != edgeData)
        {
        DwgCmEntityColorCP  trueColors = edgeData->GetTrueColors ();
        int16_t const*      indexColors = edgeData->GetColors ();
        if (nullptr != trueColors)
            {
            // edge overridden by the true color of the first edge
            edgeColor = trueColors[0];
            hasEdgeColor = true;
            }
        else if (nullptr != indexColors)
            {
            // edge overridden by index color of the first edge
            edgeColor.SetColorIndex (indexColors[0]);
            hasEdgeColor = true;
            }
        }
    if (!hasFaceColor && !hasEdgeColor)
        return;

    auto regenType = m_geometryOptions->_GetRegenType ();
    bool rendered = DwgGiRegenType::HideOrShadeCommand == regenType || DwgGiRegenType::RenderCommand == regenType;

    DwgCmEntityColor    colorOverride;
    if (hasFaceColor)
        colorOverride = faceColor;
    if (hasEdgeColor && (!hasFaceColor || !rendered))
        colorOverride = edgeColor;

    m_drawParams._SetColor (colorOverride);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Circle (DPoint3dCR center, double radius, DVec3dCR normal) override
    {
    DEllipse3d      circle = DEllipse3d::FromCenterNormalRadius (center, normal, radius);
    circle.MakeFullSweep ();

    ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (circle);
    if (primitive.IsValid() && !this->ApplyThickness(primitive, normal, true))
        this->AppendGeometry (*primitive.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Circle (DPoint3dCR point1, DPoint3dCR point2, DPoint3dCR point3) override
    {
    DEllipse3d      circle = DEllipse3d::FromPointsOnArc (point1, point2, point3);
    circle.MakeFullSweep ();
    
    ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (circle);
    if (primitive.IsValid())
        this->AppendGeometry (*primitive.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _CircularArc (DPoint3dCR center, double rad, DVec3dCR normal, DVec3dCR start, double swept, DwgGiArcType arcType = DwgGiArcType::Simple) override
    {
    DVec3d      xAxis = DVec3d::From (start);
    xAxis.Normalize ();

    DVec3d      yAxis = DVec3d::FromNormalizedCrossProduct (DVec3d::From(normal), xAxis);

    xAxis.Scale (rad);
    yAxis.Scale (rad);

    DEllipse3d  arc = DEllipse3d::FromVectors (center, xAxis, yAxis, 0.0, swept);

    if (DwgGiArcType::Chord != arcType && DwgGiArcType::Sector != arcType)
        {
        // a simple ellipse
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (arc);
        if (primitive.IsValid() && !this->ApplyThickness(primitive, normal))
            this->AppendGeometry (*primitive.get());
        }

    // WIP - create a filled area to represent a filled chord or a filled pie shape enclosed by the elliptic arc:
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _CircularArc (DPoint3dCR start, DPoint3dCR point, DPoint3dCR end, DwgGiArcType arcType = DwgGiArcType::Simple) override
    {
    DEllipse3d      arc = DEllipse3d::FromPointsOnArc (start, point, end);

    if (DwgGiArcType::Chord != arcType && DwgGiArcType::Sector != arcType)
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (arc);
        if (primitive.IsValid())
            this->AppendGeometry (*primitive.get());
        }
    // WIP - create a filled area to represent a filled chord or a filled pie shape enclosed by the elliptic arc:
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Curve (MSBsplineCurveCR curve) override
    {
    ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateBsplineCurve (curve);
    if (primitive.IsValid())
        this->AppendGeometry (*primitive.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Ellipse (DEllipse3dCR ellipse, DwgGiArcType arcType = DwgGiArcType::Simple) override
    {
    if (DwgGiArcType::Chord != arcType && DwgGiArcType::Sector != arcType)
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (ellipse);
        if (primitive.IsValid())
            this->AppendGeometry (*primitive.get());
        }
    // WIP - create a filled area to represent a filled chord or a filled pie shape enclosed by the elliptic arc:
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Edge (CurveVectorCR edges) override
    {
    for (auto primitive : edges)
        {
        GeometricPrimitivePtr   geom = GeometricPrimitive::Create (primitive);
        if (geom.IsValid())
            this->AppendGeometry (*geom.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Polyline (size_t nPoints, DPoint3dCP points, DVec3dCP normal = nullptr, int64_t subentMarker = -1) override
    {
    if (0 == nPoints)
        return;

    ICurvePrimitivePtr  linestring = ICurvePrimitive::CreateLineString (points, nPoints);
    if (linestring.IsValid())
        {
        if (nullptr != normal && this->ApplyThickness(linestring, *normal))
            return;

        this->AppendGeometry (*linestring.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Pline (DwgDbPolylineCR pline, size_t fromIndex = 0, size_t numSegs = 0) override
    {
    /*-----------------------------------------------------------------------------------
    A closed polyline in ACAD is not rendered (i.e. appears hollow in a render mode) but a closed shape is rendered 
    as filled in MicroStation.  For the purpose of worlDraw/viewportDraw there is no need to make a closed polyline 
    as a shape.  Instead, a linestring is much better representing a polyline's display in all modes.
    -----------------------------------------------------------------------------------*/
    DwgGiRegenType  regenType = m_geometryOptions->_GetRegenType ();
    bool            forceOpen = DwgGiRegenType::HideOrShadeCommand == regenType || DwgGiRegenType::RenderCommand == regenType;

    PolylineFactory plineFactory (pline, !forceOpen);
    CurveVectorPtr  curveVector = plineFactory.CreateCurveVector (false);
    if (curveVector.IsValid())
        {
        auto shape = plineFactory.ApplyConstantWidthTo (curveVector);
        if (shape.IsValid())
            {
            curveVector = shape;
            m_drawParams._SetFillType (DwgGiFillType::Always);
            }

        if (m_isTargetModel2d)
            {
            this->AppendGeometry (*curveVector.get());
            return;
            }

        // extrude the polyline if it has a thickness
        GeometricPrimitivePtr   extruded = plineFactory.ApplyThicknessTo (curveVector);
        if (extruded.IsValid())
            this->AppendGeometry (*extruded.get());
        else
            this->AppendGeometry (*curveVector.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Polygon (size_t nPoints, DPoint3dCP points) override
    {
    PolylineFactory plineFactory (nPoints, points, true);
    CurveVectorPtr  curveVector = plineFactory.CreateCurveVector (false);
    if (curveVector.IsValid())
        {
        if (m_isTargetModel2d)
            {
            this->AppendGeometry (*curveVector.get());
            return;
            }

        // extrude the polyline if it has a thickness
        GeometricPrimitivePtr   extruded = plineFactory.ApplyThicknessTo (curveVector);
        if (extruded.IsValid())
            this->AppendGeometry (*extruded.get());
        else
            this->AppendGeometry (*curveVector.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Mesh (size_t nRows, size_t nColumns, DPoint3dCP points, DwgGiEdgeDataCP edges = nullptr, DwgGiFaceDataCP faces = nullptr, DwgGiVertexDataCP verts = nullptr, bool autonorm = false) override
    {
    size_t      nTotal = nRows * nColumns;
    if (0 == nTotal)
        return;

    // if the toolkit has not called SetFillType yet, ACAD should display this mesh as unfilled - TFS 527878.
    if (DwgGiFillType::Default == m_drawParams._GetFillType())
        m_drawParams._SetFillType(DwgGiFillType::Never);

    if (2 == nRows && 2 == nColumns)
        {
        // make a simple shape
        DPoint3d    shapePoints[5];

        shapePoints[0] = shapePoints[4] = points[0];
        shapePoints[1] = points[1];
        shapePoints[2] = points[3];
        shapePoints[3] = points[2];
        
        PolylineFactory factory (5, shapePoints, true);
        CurveVectorPtr  curveVector = factory.CreateCurveVector (false);
        if (curveVector.IsValid())
            this->AppendGeometry (*curveVector.get());

        return;
        }

    PolyfaceHeaderPtr       dgnPolyface = PolyfaceHeader::CreateQuadGrid (static_cast<int>(nColumns));
    BlockedVectorDPoint3dR  meshPoints = dgnPolyface->Point ();

    for (size_t i = 0; i < nTotal; i++)
        {
        if (m_isTargetModel2d && fabs(points[i].z) > 1.0e-4)
            return;
        meshPoints.push_back (points[i]);
        }

    GeometricPrimitivePtr   pface = GeometricPrimitive::Create (dgnPolyface);
    if (pface.IsValid())
        {
        this->ApplyColorOverride (faces, edges);
        this->AppendGeometry (*pface.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Shell (size_t nPoints, DPoint3dCP points, size_t nFaceList, int32_t const* faces, DwgGiEdgeDataCP edgeData = nullptr, DwgGiFaceDataCP faceData = nullptr, DwgGiVertexDataCP vertData = nullptr, DwgResBufCP xData = nullptr, bool autonorm = false) override
    {
    // DwgGiFillType::Always == m_drawParams._GetFillType()
    /*-----------------------------------------------------------------------------------
    A filled hatch, or a filled object created by an object enabler, may be tessellated to
    a huge number of shapes, which can significantly increase the assembly size. Ideally,
    we would want polyface headers for the sake of performance and smaller file size, but 
    at the moment, polyface header is not supported for 2D model!
    -----------------------------------------------------------------------------------*/
    if (m_isTargetModel2d)
        {
        // create shapes
        size_t  nVertices=0, maxFaceVertices = 0;

        if (nFaceList > 1000)
            {
            // avoid looping through millions of faces just to find max face vertices - it usually is 3 or 4.
            maxFaceVertices = 100;
            if (abs(faces[0]) > maxFaceVertices)
                maxFaceVertices = 20 * abs(faces[0]);
            }
        else
            {
            for (size_t jFace = 0; jFace < nFaceList; jFace += nVertices)
                if ((nVertices = abs (faces[jFace++])) > maxFaceVertices)
                    maxFaceVertices = nVertices;
            }

        DwgImporter&    importer = m_drawParams.GetDwgImporter ();
        for (size_t jFace = 0; jFace < nFaceList;)
            {
            // a positive face vertex count denotes a solid loop whereas a negative count denotes a hole loop:
            CurveVector::BoundaryType   loopType = faces[jFace] < 0 ? CurveVector::BOUNDARY_TYPE_Inner : CurveVector::BOUNDARY_TYPE_Outer;

            nVertices = abs (faces[jFace++]);
            if (nVertices > maxFaceVertices)
                {
                BeDataAssert (false && L"unexpected max face vertices!");
                Utf8PrintfString msg("skipped a shell having face vertex count of %d [expected %d]", nVertices, maxFaceVertices);
                if (m_entity != nullptr)
                    msg += Utf8PrintfString(" ID=%ls", m_entity->GetObjectId().ToAscii().c_str());
                importer.ReportIssue (DwgImporter::IssueSeverity::Warning, IssueCategory::UnexpectedData(), Issue::Message(), msg.c_str());
                return;
                }

            /*-----------------------------------------------------------------------------------------
            Collect shape vertices, but skip those overlapping with the start point - a solid or trace
            entity can have these.  When drawing a solid or trace, the input point array already has the
            3rd and 4th point swapped, as they should.  In case of a "triangle" solid/trace, as cases 
            seen in TFS 489127 and old TR 138302, the last vertex is swapped as the 3rd vertex, which 
            overlaps with the first vertex of the face.  Regardless what entity we are drawing, we simply 
            skip any a vertex that is overlapping with the start point.
            -----------------------------------------------------------------------------------------*/
            DPoint3dArray   faceVertices;
            for (size_t kVertex = 0; kVertex < nVertices; kVertex++)
                {
                DPoint3d    vertex = points[faces[jFace++]];

                // trivial rejecting 3D element
                if (fabs(vertex.z) > 1.0e-4)
                    return;

                if (kVertex == 0 || !faceVertices[0].IsEqual(vertex))
                    faceVertices.push_back (vertex);
                }

            CurveVectorPtr          face = CurveVector::CreateLinear (faceVertices, loopType, false);
            GeometricPrimitivePtr   primitive = GeometricPrimitive::Create (face);
            if (primitive.IsValid())
                this->AppendGeometry (*primitive.get());
            }
        return;
        }

    // Create our polyface header
    PolyfaceHeaderPtr       dgnPolyface = PolyfaceHeader::CreateVariableSizeIndexed ();
    BlockedVectorDPoint3dR  pointArray = dgnPolyface->Point ();

    // Build polyface point array directly from DWG vertex list:
    for (size_t iVertex = 0; iVertex < nPoints; iVertex++)
        pointArray.push_back (points[iVertex]);

    // Add polyface face one at a time from DWG face list:
    uint8_t const*          edgeVisibilities = (nullptr == edgeData) ? nullptr : edgeData->GetVisibility ();
    bvector<int>            pointIndices;
    for (size_t iFace = 0; iFace < nFaceList; )
        {
        size_t    nFaceVertices = abs (faces[iFace++]);

        pointIndices.clear ();

        for (size_t iFaceVertex = 0; iFaceVertex < nFaceVertices; iFaceVertex++)
            {
            int32_t     pointIndex = abs(faces[iFace++]) + 1;

            // turn off invisible edges
            if (nullptr != edgeVisibilities)
                {
                DwgGiVisibility visible = static_cast <DwgGiVisibility> (*edgeVisibilities++);

                if (DwgGiVisibility::Invisible == visible)
                    pointIndex = -pointIndex;
                }
                
            pointIndices.push_back ((int)pointIndex);
            }
        dgnPolyface->AddIndexedFacet (pointIndices);
        }

    GeometricPrimitivePtr   pface = GeometricPrimitive::Create (dgnPolyface);
    if (pface.IsValid())
        {
        this->ApplyColorOverride (faceData, edgeData);
        this->AppendGeometry (*pface.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SetText (Dgn::TextStringPtr& dgnText, DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, DwgStringCR string)
    {
    if (!dgnText.IsValid())
        return  BSIERROR;

    DwgImporter&    importer = m_drawParams.GetDwgImporter ();
    if (string.IsEmpty())
        {
        Utf8PrintfString    msg ("skipped empty text entity, ID=%ls!", m_entity == nullptr ? L"?" : m_entity->GetObjectId().ToAscii().c_str());
        importer.ReportIssue (DwgImporter::IssueSeverity::Info, IssueCategory::UnexpectedData(), Issue::Message(), msg.c_str());
        return  BSIERROR;
        }

    if (!importer.ArePointsValid(&position, 1, m_drawParams.GetSourceEntity()))
        {
        Utf8PrintfString    msg ("skipped out of range text entity, ID=%ls!", m_entity == nullptr ? L"?" : m_entity->GetObjectId().ToAscii().c_str());
        importer.ReportIssue (DwgImporter::IssueSeverity::Info, IssueCategory::UnexpectedData(), Issue::InvalidRange(), msg.c_str());
        return  BSIERROR;
        }

    Utf8String  textString(string.c_str());
    textString.Trim ();
    if (textString.empty())
        {
        Utf8PrintfString    msg ("skipped text containing all white spaces, ID=%ls!", m_entity == nullptr ? L"?" : m_entity->GetObjectId().ToAscii().c_str());
        importer.ReportIssue (DwgImporter::IssueSeverity::Info, IssueCategory::UnexpectedData(), Issue::Message(), msg.c_str());
        return  BSIERROR;
        }

    // unmirror text on XY plane
    DVec3d xAxis, zAxis;
    xAxis.Normalize (xdir);
    zAxis.Normalize (normal);
    if (fabs(zAxis.z + 1.0) < 0.001)
        {
        xAxis.Negate ();
        zAxis.Negate ();
        }

    // get ECS - do not compound it with currTrans - handled in central factory.
    RotMatrix   ecs;
    DwgHelper::ComputeMatrixFromXZ (ecs, xAxis, zAxis);

    dgnText->SetText (textString.c_str());
    dgnText->SetOrigin (position);
    dgnText->SetOrientation (ecs);
    
    // caller to set style specific data
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DropText (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, DwgStringCR string, bool raw, DwgGiTextStyleCR giStyle)
    {
    // only drop a multibyte text using an effective bigfont.
    auto bigfont = giStyle.GetBigFontFileName ();
    if (string.GetLength() < 1 || string.IsAscii() || bigfont.empty())
        return  false;

    // if bigfont is used, but cannot be found by the toolkit, don't bother dropping it.
    WString found;
    if (DwgImportHost::GetHost()._FindFile(found, bigfont.c_str(), m_drawParams.GetDatabase(), AcadFileType::CompiledShapeFile) != DwgDbStatus::Success)
        return  false;

    // get the text size
    DPoint2d    size;
    ShapeTextProcessor  processor(const_cast<DwgGiTextStyleR>(giStyle), string);
    processor.SetIsRaw (raw);
    if (DwgDbStatus::Success != processor.GetExtents(size))
        return  false;

    // calc tolerance
    auto    deviation = size.y != 0.0 ? size.y : size.x;
    deviation *= 0.01;

    // drop text to a collection of line strings, in font unit size:
    bvector<DPoint3dArray>  linestrings;
    if (DwgDbStatus::Success != processor.Drop(linestrings, deviation))
        return  false;

    // transform dropped geometry from ECS to WCS.
    RotMatrix   rotation;
    DwgHelper::ComputeMatrixFromXZ (rotation, xdir, normal);

    // scale font size to world size:
    if (size.y > 1.0e-5)
        rotation.ScaleColumns (size.y, size.y, size.y);

    Transform   ecs = Transform::From (rotation);
    ecs.SetTranslation (position);

    // save geometry primitives
    for (auto linestring : linestrings)
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateLineString (linestring);
        if (!primitive.IsValid())
            return false;

        primitive->TransformInPlace (ecs);
        this->AppendGeometry (*primitive.get());
        }
    
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Text (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, double h, double w, double oblique, DwgStringCR string) override
    {
    Dgn::TextStringPtr dgnText = TextString::Create ();
    if (BSISUCCESS != this->SetText(dgnText, position, normal, xdir, string))
        {
        m_status = BSIERROR;
        return;
        }

    // set default font
    DgnFontCP   font = m_drawParams.GetDwgImporter().GetDefaultFont();
    if (nullptr == font)
        font = &DgnFontManager::GetAnyLastResortFont ();
    
    Dgn::TextStringStyleR    style = dgnText->GetStyleR ();
    style.SetFont (*font);
    style.SetSize (w, h);
    style.SetIsItalic (0.0 != oblique);

    GeometricPrimitivePtr   primitive = GeometricPrimitive::Create (dgnText);
    if (primitive.IsValid())
        this->AppendGeometry (*primitive.get());
    // NEEDSWORK - extrude text glyphs
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Text (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, DwgStringCR string, bool raw, DwgGiTextStyleCR giStyle) override
    {
    // try dropping multibyte text string using a bigfont:
    if (this->DropText(position, normal, xdir, string, raw, giStyle))
        return;

    Dgn::TextStringPtr  dgnText = TextString::Create ();
    if (BSISUCCESS != this->SetText(dgnText, position, normal, xdir, string))
        {
        m_status = BSIERROR;
        return;
        }

    // try to get & resolve font
    DwgImporter&    importer = m_drawParams.GetDwgImporter ();
    DgnFontType     fontType = DgnFontType::Shx;
    DwgFontInfo     fontInfo;
    DgnFontCP       font = nullptr;
    if (DwgDbStatus::Success != giStyle.GetFontInfo(fontInfo))
        {
        WString     fname = giStyle.GetFileName ();
        importer.ReportIssue (DwgImporter::IssueSeverity::Info, IssueCategory::MissingData(), Issue::Message(), Utf8PrintfString("no font found in text style %s - using default font", fname.c_str()).c_str());

        font = importer.GetDefaultFont();
        }
    else
        {
        font = importer.GetDgnFontFor (fontInfo);
        if (nullptr == font)
            font = fontInfo.GetTypeFace().empty() ? &DgnFontManager::GetLastResortTrueTypeFont() : &DgnFontManager::GetLastResortShxFont();
        }
    if (nullptr == font)
        {
        BeAssert (false && L"null DgnFont!");
        m_status = BSIERROR;
        return;
        }

    Dgn::TextStringStyleR   dgnStyle = dgnText->GetStyleR ();
    dgnStyle.SetFont (*font);

    // get & set other text style params
    double  height = giStyle.GetTextSize ();
    double  width = height * giStyle.GetWidthFactor();
    
    dgnStyle.SetSize (width, height);
    dgnStyle.SetIsBold (fontInfo.IsBold());
    dgnStyle.SetIsUnderlined (giStyle.IsUnderlined());

    if (fontType == DgnFontType::TrueType)
        dgnStyle.SetIsItalic (fontInfo.IsItalic());
    else
        dgnStyle.SetIsItalic (giStyle.GetObliqueAngle() != 0);

    // decode escape codes if the input string is not raw primitives:
    bvector<DSegment3d> underlines, overlines;
    if (!raw)
        DwgHelper::ConvertEscapeCodes (*dgnText.get(), &underlines, &overlines);

    // get strike through line segment before dgnText may be changed:
    DSegment3d  strikeThrough;
    if (giStyle.IsStrikethrough())
        {
        DRange2d    range = dgnText->GetRange ();

        strikeThrough.point[0].x = range.low.x;
        strikeThrough.point[0].y = range.low.y + 0.5 * range.YLength();
        strikeThrough.point[0].z = 0.0;

        strikeThrough.point[1].x = range.high.x;
        strikeThrough.point[1].y = strikeThrough.point[0].y;
        strikeThrough.point[1].z = 0.0;
        
        dgnText->ComputeTransform().Multiply (&strikeThrough.point[0], &strikeThrough.point[0], 2);
        }

    GeometricPrimitivePtr   primitive = GeometricPrimitive::Create (dgnText);
    if (primitive.IsValid())
        this->AppendGeometry (*primitive.get());

    // NEEDSWORK - extrude text grlyphs

    // draw underlines and overlines decorated by the escape codes:
    for (auto const& underline : underlines)
        {
        ICurvePrimitivePtr  line = ICurvePrimitive::CreateLine (underline);
        if (line.IsValid())
            this->AppendGeometry (*line.get());
        }
    for (auto const& overline : overlines)
        {
        ICurvePrimitivePtr  line = ICurvePrimitive::CreateLine (overline);
        if (line.IsValid())
            this->AppendGeometry (*line.get());
        }

    // now draw strike through
    if (giStyle.IsStrikethrough())
        {
        ICurvePrimitivePtr  line = ICurvePrimitive::CreateLine (strikeThrough);
        if (line.IsValid())
            this->AppendGeometry (*line.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Xline (DPoint3dCR point1, DPoint3dCR point2) override
    {
    // WIP - support LINKAGEID_InfiniteLine
    DVec3d      vector = DVec3d::FromStartEnd (point1, point2);
    double      length = 1.0e5;
    if (nullptr != m_entity)
        {
        DwgDbDatabasePtr    dwg = m_entity->GetDatabase ();
        if (!dwg.IsNull())
            length = dwg->GetEXTMIN().Distance (dwg->GetEXTMAX()) * 2;
        }

    // shooting the vector from point1 to a far distance
    DSegment3d  line;
    line.InitZero ();
    line.point[1].SumOf (point1, vector, length);

    // shooting the inverted vector to a far distance
    vector.Negate ();
    line.point[0].SumOf (point1, vector, length);

    ICurvePrimitivePtr  xline = ICurvePrimitive::CreateLine (line);
    if (xline.IsValid())
        this->AppendGeometry (*xline.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Ray (DPoint3dCR origin, DPoint3dCR point) override
    {
    // WIP - support LINKAGEID_InfiniteLine
    DVec3d      vector = DVec3d::FromStartEnd (origin, point);
    double      length = 1.0e5;
    if (nullptr != m_entity)
        {
        DwgDbDatabasePtr    dwg = m_entity->GetDatabase ();
        if (!dwg.IsNull())
            length = dwg->GetEXTMIN().Distance (dwg->GetEXTMAX()) * 2;
        }
    vector.Scale (length);

    DSegment3d  line = DSegment3d::FromOriginAndDirection (origin, vector);

    ICurvePrimitivePtr  ray = ICurvePrimitive::CreateLine (line);
    if (ray.IsValid())
        this->AppendGeometry (*ray.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Draw (DwgGiDrawableR drawable) override
    {
    if (nullptr == m_geometryOptions)
        {
        BeAssert (false && L"Uninitlized geometry options!!");
        m_status = BSIERROR;
        return;
        }

#ifdef DEBUG_NESTED_DRAWABLE
    DwgDbEntityP    dbgent = drawable.GetEntityP ();
    if (nullptr == dbgent)
        {
        DwgDbBlockTableRecordP dbgblock = drawable.GetBlockP ();
        if (nullptr == dbgblock)
            LOG_ENTITY.debugv ("drawing a non-entity, non-block, handle=%ls...", drawable.GetId().ToAscii().c_str());
        else
            LOG_ENTITY.debugv ("drawing block def \"%ls\" w/handle=%ls...", dbgblock->GetName().c_str(), dbgblock->GetObjectId().ToAscii().c_str());
        }
    else
        {
        LOG_ENTITY.debugv ("drawing %ls with handle=%ls...", dbgent->GetDxfName().c_str(), dbgent->GetObjectId().ToAscii().c_str());
        }
#endif

    // do not draw attrdef in a block other than a modelspace or paperspace:
    if (this->IsDrawingBlock() && nullptr != drawable.GetAttributeDefinitionP())
        return;

    DwgDbBlockTableRecordP block = drawable.GetBlockP ();
    if (nullptr != block)
        {
        // draw a block, potentially nested:
        this->PushDrawingBlock (*block);

        try
            {
            drawable.Draw (*this, *m_geometryOptions, m_drawParams);
            }
        catch (...)
            {
            BeAssert (false && L"Exception thrown drawing a nested block!");
            }

        // done drawing the block - revert drawing controls back to previous state:
        this->PopDrawingBlock ();
        return;
        }

    // we want to use "this" factory to collect sub-entities, so override "this" sub-entity traits:
    DrawParameters  savedParams = m_drawParams;
    DwgDbEntityPtr  child(drawable.GetId(), DwgDbOpenMode::ForRead);
    if (!child.IsNull())
        {
        // don't bother to add invisible primitives.
        if (DwgDbVisibility::Invisible == child->GetVisibility())
            return;
        // trivial reject the entity if it is clipped away as a whole
        if (nullptr != m_spatialFilter && m_spatialFilter->IsEntityFilteredOut(*child.get()))
            return;
        // skip this entity if it should not be drawn
        if (this->SkipBlockChildGeometry(child.get()))
            return;

        // give protocal extensions a chance to create their own geometry:
        if (this->CreateBlockChildGeometry(child.get()) == BSISUCCESS)
            return;

        m_drawParams.Initialize (*child.get(), &savedParams);

        // gradient fill causes world/viewportDraw to create mesh - remove gradient by setting the hatch as a solid fill:
        DwgDbHatchP hatch = DwgDbHatch::Cast (child.get());
        if (nullptr != hatch && hatch->IsGradient() && DwgDbStatus::Success == hatch->UpgradeOpen())
            {
            hatch->SetPattern (DwgDbHatch::PatternType::PreDefined, DwgString(L"SOLID"));
            hatch->DowngradeOpen ();
            }
        }

    // draw a primitive - could be a child entity, a primitive of an exploded parenty geometry, etc:
    try
        {
        drawable.Draw (*this, *m_geometryOptions, m_drawParams);
        }
    catch (...)
        {
        BeAssert (false && L"Exception thrown drawing a nested block!");
        m_status = BSIERROR;
        }

    // for a nested a block reference followed by attributes, draw attributes now:
    DwgDbEntityP    entity = child.get ();
    if (nullptr != entity)
        {
        DwgDbBlockReferenceCP   blockRef = DwgDbBlockReference::Cast (entity);
        if (nullptr != blockRef)
            this->DrawBlockAttributes (*blockRef);

        m_drawParams.CopyFrom (savedParams);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SkipBlockChildGeometry (DwgDbEntityCP entity)
    {
    if (nullptr == entity)
        return  true;

    // filter out geometries on layer Defpoints if their parent is of a dimension that is not displayed:
    bool shouldSkip = false;
    auto dwg = entity->GetDatabase ();
    bool isOnDefpoints = dwg.IsValid() && entity->GetLayerId() == dwg->GetLayerDefpointsId();
    if (isOnDefpoints && nullptr != m_entity && m_entity->IsDimension())
        {
        DwgDbLayerTableRecordPtr parentLayer(m_entity->GetLayerId(), DwgDbOpenMode::ForRead);
        shouldSkip = parentLayer.OpenStatus() == DwgDbStatus::Success && (parentLayer->IsOff() || parentLayer->IsFrozen());
        }

    return  shouldSkip;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CreateBlockChildGeometry (DwgDbEntityCP entity)
    {
    auto* objExt = DwgProtocolExtension::Cast (entity->QueryX(DwgProtocolExtension::Desc()));
    if (nullptr == objExt)
        return  BSIERROR;

    // trivial reject 3D elements in a 2D model
    if (m_isTargetModel2d && DwgBrepExt::Cast(objExt) != nullptr && DwgDbRegion::Cast(entity) == nullptr && DwgDbPlaneSurface::Cast(entity) == nullptr)
        return  BSIERROR;

    auto geom = objExt->_ConvertToGeometry (entity, m_drawParams.GetDwgImporter());
    if (!geom.IsValid())
        return  BSIERROR;

    this->AppendGeometry (*geom.get());

#if defined (BENTLEYCONFIG_PARASOLID)
    // should the protocol extension create a Parasolid body, save the tag to be freed after all elements created.
    auto brep = geom->GetAsIBRepEntity ();
    if (brep.IsValid())
        {
        bool owned = false;
        auto tag = PSolidUtil::GetEntityTag (*brep, &owned);
        if (!owned)
            m_parasolidBodies.push_back (tag);
        }
#endif
    
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DrawBlockAttributes (DwgDbBlockReferenceCR blockRef)
    {
    DwgDbObjectIteratorPtr  iter = blockRef.GetAttributeIterator ();
    if (!iter.IsValid() || !iter->IsValid())
        return  BSISUCCESS;

    // save off last DrawParameters
    DrawParameters  savedParams = m_drawParams;
    uint32_t        count = 0;

    // draw each and every attribute followed the block reference:
    for (iter->Start(); !iter->Done(); iter->Next())
        {
        DwgDbEntityPtr  attrib(iter->GetObjectId(), DwgDbOpenMode::ForRead);
        if (!attrib.IsNull())
            {
            DwgGiDrawablePtr    drawable = attrib->GetDrawable ();
            if (drawable.IsValid())
                {
                // set DrawParameters from this attribute and use block's DrawParameters for ByBlock symbology:
                m_drawParams.Initialize (*attrib.get(), &savedParams);
                // draw this attribute:
                drawable->Draw (*this, *m_geometryOptions, m_drawParams);
                count++;
                }
            }
        }

    // restore last DrawParameters
    if (count > 0)
        m_drawParams.CopyFrom (savedParams);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Image (DwgGiImageBGRA32CR image, DPoint3dCR pos, DVec3dCR u, DVec3dCR v, DwgGiTransparencyMode mode = DwgGiTransparencyMode::Alpha8Bit) override
    {
    BeAssert (false && L"implement _Image!!!");
    m_status = BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _RowOfDots (size_t count, DPoint3dCR start, DVec3dCR step) override
    {
    if (count < 1)
        return;

    DPoint3dArray   dots;
    for (size_t i = 0; i < count; i++)
        {
        if (0 == i)
            dots.push_back (start);
        else
            dots.push_back (DPoint3d::FromSumOf(dots.back(), step));
        }

    ICurvePrimitivePtr  primitive = ICurvePrimitive::CreatePointString (dots);
    if (primitive.IsValid())
        this->AppendGeometry (*primitive.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _WorldLine (DPoint3d points[2]) override
    {
    DSegment3d  worldLine = DSegment3d::From (points[0], points[1]);

    ICurvePrimitivePtr  line = ICurvePrimitive::CreateLine (worldLine);
    if (line.IsValid())
        {
        Transform   curr = m_currentTransform;
        m_currentTransform = m_baseTransform;

        this->AppendGeometry (*line.get());

        m_currentTransform = curr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _PushModelTransform (TransformCR newTransform) override
    {
    // trival reject invalid input
    Transform   validTransform = ::isnan(newTransform.ColumnX().Magnitude()) ? Transform::FromIdentity() : newTransform;
    Transform   topTransform;

    if (m_transformStack.empty())
        topTransform = validTransform;
    else
        topTransform.InitProduct (m_transformStack.back(), validTransform);

    // push the componded transformation on top of the stack
    m_transformStack.push_back (topTransform);

    // update current transformation
    m_currentTransform.InitProduct (m_baseTransform, topTransform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _PopModelTransform () override
    {
    if (!m_transformStack.empty())
        m_transformStack.pop_back ();
    else
        BeAssert (false && L"Popping an empty transform stack!!");

    if (m_transformStack.empty())
        m_currentTransform = m_baseTransform;
    else
        m_currentTransform.InitProduct (m_baseTransform, m_transformStack.back());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _GetModelTransform (TransformR outTransform) override
    {
    if (!m_transformStack.empty())
        outTransform = m_transformStack.back ();
    else
        outTransform.InitIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PushClipBoundary (DwgGiClipBoundaryCP boundary) override
    {
    BeAssert (false && "_PushClipBoundary not implemented!");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _PopClipBoundary () override
    {
    BeAssert (false && "_PopClipBoundary not implemented!");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            AppendGeometry (ICurvePrimitiveCR primitive)
    {
    GeometricPrimitivePtr   geom = GeometricPrimitive::Create (primitive);
    if (geom.IsValid())
        this->AppendGeometry (*geom.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            AppendGeometry (CurveVectorCR curveVector)
    {
    GeometricPrimitivePtr   geom = GeometricPrimitive::Create (curveVector);
    if (geom.IsValid())
        this->AppendGeometry (*geom.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            AppendGeometry (GeometricPrimitiveR geometry)
    {
    // get or create a category & sub-category for this piece of geometry:
    DgnCategoryId       categoryId;
    DgnSubCategoryId    subCategoryId;
    this->ComputeCategory (categoryId, subCategoryId);

    // set category & sub-category in GeometryParams - it clears appearance overrides:
    Render::GeometryParams   display;
    display.SetCategoryId (categoryId);
    display.SetSubCategoryId (subCategoryId);

    // convert other sub-entity traits to GeometryParams:
    m_drawParams.GetDisplayParams (display);

    auto block = this->GetCurrentBlock ();
    auto blockId = block.GetBlockId ();

    // build a new cache entry for the geometry
    DwgImporter::GeometryEntry   geomEntry;
    geomEntry.SetGeometry (geometry.Clone().get());
    geomEntry.SetGeometryParams (display);
    geomEntry.SetTransform (m_currentTransform);
    geomEntry.SetBlockName (block.GetBlockName());
    geomEntry.SetBlockId (blockId);
    geomEntry.SetDwgFileId (block.GetFileId());

    // cache the new entry for the block
    auto found = m_outputGeometryMap.find (blockId);
    if (found == m_outputGeometryMap.end())
        {
        DwgImporter::T_GeometryList emptyList;
        DwgImporter::T_BlockGeometryEntry entry(blockId, emptyList);
        m_outputGeometryMap.insert (entry);
        found = m_outputGeometryMap.find (blockId);
        }
    found->second.push_back (geomEntry);

    // spin the progress meter
    m_drawParams.GetDwgImporter().Progress ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            ComputeCategory (DgnCategoryId& categoryId, DgnSubCategoryId& subcategoryId)
    {
    DgnModelR       toModel = m_createParams.GetModelR ();
    DwgDbObjectId   layerId = m_drawParams.GetEffectiveLayerId ();
    DwgImporter&    importer = m_drawParams.GetDwgImporter ();
    DwgDbDatabaseP  xrefDwg = nullptr == m_entity ? nullptr : m_entity->GetDatabase().get();

    // check if this entity is in an xref:
    if (nullptr != xrefDwg && xrefDwg == &importer.GetDwgDb())
        xrefDwg = nullptr;

    if (toModel.Is3d())
        {
        // we are in a modelspace, of the master file or an xref file, get a spatial category and a sub-category from the syncInfo:
        categoryId = importer.GetSpatialCategory (subcategoryId, layerId, xrefDwg);

        if (this->IsDrawingBlock())
            {
            // we are creating a shared geometry part - resolve its visibility:
            DgnDbR  db = toModel.GetDgnDb ();
            DgnSubCategoryCPtr  subcategory = DgnSubCategory::Get (db, subcategoryId);
            DgnSubCategory::Appearance  appearance;

            if (subcategory.IsValid() && m_drawParams.IsDisplayed() != (appearance = subcategory->GetAppearance()).IsVisible())
                {
                // find a different sub-category that matches this part's display status:
                for (auto entry : DgnSubCategory::MakeIterator(db, categoryId))
                    {
                    DgnSubCategoryCPtr  checkSubcat = DgnSubCategory::Get (db, entry.GetId<DgnSubCategoryId>());
                    if (checkSubcat.IsValid() && m_drawParams.IsDisplayed() == checkSubcat->GetAppearance().IsVisible())
                        {
                        subcategoryId = checkSubcat->GetSubCategoryId();
                        return;
                        }
                    }

                // a subcategory with the same on/off status does not exist - add one:
                appearance.SetInvisible (!appearance.IsInvisible());
                subcategoryId = m_drawParams.GetDwgImporter().InsertAlternateSubCategory (subcategory, appearance);
                }
            }
        }
    else
        {
        // we are in a paperspace - get or add a drawing category per layer & viewport:
        DwgDbObjectId   viewportId = importer.GetCurrentViewportId ();
        categoryId = importer.GetOrAddDrawingCategory (subcategoryId, layerId, viewportId, toModel, xrefDwg);
        }

    if (!categoryId.IsValid())
        {
        BeAssert (false && "failed category computation!");
        categoryId = m_createParams.GetCategoryId ();
        }
    if (!subcategoryId.IsValid())
        {
        BeAssert (false && "failed sub-category computation!");
        subcategoryId = m_createParams.GetSubCategoryId ();
        }
    }

};  // GeometryFactory


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
ElementFactory::ElementFactory (DwgImporter::ElementImportResults& results, DwgImporter::ElementImportInputs& inputs, DwgImporter::ElementCreateParams& params, DwgImporter& importer)
    : m_results(results), m_inputs(inputs), m_createParams(params), m_importer(importer), 
      m_elementParams(inputs.GetTargetModelR().GetDgnDb(), inputs.GetTargetModelR().GetModelId(), inputs.GetClassId())
    {
    // update DgnCode & caller will set user label as needed
    m_elementCode = m_createParams.GetElementCode ();
    m_elementParams.SetCode (m_elementCode);
    m_elementLabel = m_importer._GetElementLabel (m_inputs.GetEntity());
    m_elementParams.SetUserLabel (m_elementLabel.c_str());

    m_partModel = m_importer.GetOrCreateJobDefinitionModel ();
    if (!m_partModel.IsValid())
        {
        // should not occur but back it up anyway!
        m_importer.ReportError (IssueCategory::Unknown(), Issue::MissingJobDefinitionModel(), "GeometryParts");
        m_partModel = &m_importer.GetDgnDb().GetDictionaryModel ();
        }
    m_geometryMap = nullptr;

    m_modelTransform = m_inputs.GetTransform ();
    m_baseTransform = Transform::FromIdentity ();
    m_invBaseTransform = Transform::FromIdentity ();
    m_hasBaseTransform = false;
    m_basePartScale = 0.0;
    m_is3d = m_inputs.GetTargetModelR().Is3d ();
    m_canCreateSharedParts = m_importer.GetOptions().IsBlockAsSharedParts ();
    m_sourceBlockId.SetNull ();
    m_sourceLayerId.SetNull ();

    // find the element handler
    m_elementHandler = dgn_ElementHandler::Element::FindHandler (m_inputs.GetTargetModelR().GetDgnDb(), m_inputs.GetClassId());
    BeAssert(nullptr != m_elementHandler && "Null element handler!");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    ElementFactory::SetDefaultCreation ()
    {
    // if user does not want shared parts, honor the request:
    if (!m_canCreateSharedParts)
        return;

    DwgDbBlockReferenceCP   insert = DwgDbBlockReference::Cast(m_inputs.GetEntityP());
    if (nullptr != insert)
        {
        // prefer share parts for a named block
        m_canCreateSharedParts = true;

        auto blockTrans = Transform::FromIdentity ();
        insert->GetBlockTransform (blockTrans);

        m_sourceBlockId = insert->GetBlockTableRecordId ();
        m_sourceLayerId = insert->GetLayerId ();

        // don't need to create shared parts for an anoymouse block
        DwgDbBlockTableRecordPtr    block (m_sourceBlockId, DwgDbOpenMode::ForRead);
        if (block.OpenStatus() == DwgDbStatus::Success && block->IsAnonymous())
            m_canCreateSharedParts = false;

        // this call may also reset m_canCreateSharedParts=false
        this->SetBaseTransform (blockTrans);
        }
    else
        {
        // individual elements for all other entities
        m_canCreateSharedParts = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    ElementFactory::SetBaseTransform (TransformCR blockTrans)
    {
    /*-----------------------------------------------------------------------------------
    DgnDb geometry requires normalized and uniformly scaled transform.  To create shared
    parts we normalize base transform and save a uniform scale if it has one. We will call 
    ApplyUniformPartScale to apply this scale on a part transform at a later step creating
    elements for the parts.

    For a base transform from which we cannot create parts, we will create individual 
    elements instead of shared parts.  We will directly transform individual geometry 
    in-place as needed via TransformGeometry.

    A local part scale, m_basePartScale, is saved here only for the purpose of caching share
    parts in block-parts map for this block.  The cached parts are only used during the same 
    import session. If this block is instanced multiple times at the same scale in the model,
    the cached parts can be directly re-used, bypassing the whole step of CreateSharedParts().
    In essense, m_basePartScale serves for the sake of performance.  it does not participate 
    in the calculation of the share parts.

    The input transform is the block transform before model transform. Model transform is
    applied to the translation.
    -----------------------------------------------------------------------------------*/
    auto blockToModel = Transform::FromProduct (m_modelTransform, blockTrans);

    if (!DwgHelper::GetTransformForSharedParts(&m_baseTransform, &m_basePartScale, blockToModel))
        m_canCreateSharedParts = false;

    if (this->Validate2dTransform(m_baseTransform) && m_canCreateSharedParts && m_basePartScale != 0.0)
        DwgHelper::NegateScaleForSharedParts (m_basePartScale, blockTrans);

    m_invBaseTransform.InverseOf (m_baseTransform);

    if (!m_canCreateSharedParts)
        m_basePartScale = 0.0;

    m_hasBaseTransform = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    ElementFactory::TransformGeometry (GeometricPrimitiveR geometry, TransformR geomTrans, double* partScale) const
    {
    /*-----------------------------------------------------------------------------------
    Factor out "valid" matrix parts of the input DWG geometry transform, build a return 
    transform from a pure rotaton and a translation, and then apply scales & skews to the
    input geometry in-place.

    When the geometry is used to build a share part, i.e. partScale != nullptr, a part scale
    is calculated based on the input geometry transform.  The caller will save the scale
    into the part entry, which will be inverse applied to the geometry element at next step.

    When the geometry is used to create an individual geometry, i.e. nullptr == partScale, 
    there is no part scale applied.
    -----------------------------------------------------------------------------------*/
    auto inplaceTrans = Transform::FromProduct (m_modelTransform, geomTrans);
    auto matrix = inplaceTrans.Matrix ();
    auto translation = inplaceTrans.Translation ();

    if (nullptr != partScale)
        *partScale = 0.0;

    RotMatrix   rotation, skew;
    if (matrix.RotateAndSkewFactors(rotation, skew, 0, 1))
        {
        // apply skew transform in place
        if (skew.IsIdentity())
            {
            // no scales
            inplaceTrans.InitIdentity ();
            }
        else
            {
            if (nullptr != partScale)
                {
                // scales exist and we are creating a shared part - only if it is uniformaly scaled, we will apply the scale on the part:
                RotMatrix   rigid;
                double      scale = 1.0;
                if (skew.IsRigidSignedScale(rigid, scale))
                    *partScale = scale;
                }
            inplaceTrans.InitFrom (skew);
            }

        // return a transform of pure ration + translation for GeometryBuilder
        geomTrans.InitFrom (rotation, translation);
        }
    else
        {
        // should not occur!
        geomTrans.InitIdentity ();
        }

    // remove/invert the part scale, if building parts
    double scale = nullptr == partScale ? 0.0 : *partScale;
    this->ApplyPartScale (inplaceTrans, scale, true);

    if (!inplaceTrans.IsIdentity())
        geometry.TransformInPlace (inplaceTrans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void    ElementFactory::ApplyPartScale (TransformR transform, double scale, bool invert) const
    {
    // valid only for building parts - input scale should have been set to 0 for individual elements.
    scale = fabs (scale);
    if (scale > 1.0e-10 && fabs(scale - 1.0) > 0.001)
        {
        if (invert)
            scale = 1.0 / scale;
        transform.ScaleMatrixColumns (scale, scale, scale);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ElementFactory::Validate2dTransform (TransformR transform) const
    {
    DPoint3d    placementPoint;
    transform.GetTranslation (placementPoint);

    RotMatrix   matrix;
    transform.GetMatrix (matrix);

    YawPitchRollAngles  angles;
    YawPitchRollAngles::TryFromRotMatrix (angles, matrix);

    bool changed = false;

    // validate Placement2d
    if (!m_is3d)
        {
        // Element2d only uses Yaw:
        if (0.0 != angles.GetPitch().Degrees() || 0.0 != angles.GetRoll().Degrees())
            {
            angles = YawPitchRollAngles (AngleInDegrees::FromDegrees(0.0), angles.GetPitch(), angles.GetRoll());
            transform = Transform::FromProduct (transform, angles.ToTransform(DPoint3d::FromZero()));
            // reset near zero rotation components which will cause GeometryBuilder to fail
            for (int i = 0; i < 3; i++)
                {
                for (int j = 0; j < 3; j++)
                    {
                    if (fabs(transform.form3d[i][j]) < 1.0e-4)
                        transform.form3d[i][j] = 0.0;
                    }
                }
            changed = true;
            }
        // and has no z-translation:
        placementPoint.z = 0.0;
        transform.SetTranslation (placementPoint);
        }
    return  changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  ElementFactory::BuildPartCodeValue (DwgImporter::GeometryEntry const& geomEntry, size_t partNo)
    {
    // set a persistent code value, "blockName[fileId:blockId]-partIndex"
    auto name = geomEntry.GetBlockName().c_str ();
    auto blockid = geomEntry.GetBlockId().ToUInt64 ();
    auto fileid = geomEntry.GetDwgFileId ();

    // a different code value for a mirrored block
    if (m_basePartScale < 0.0)
        return Utf8PrintfString ("%s[%d:%llx:m]-%lld", name, fileid, blockid, partNo);
    else
        return Utf8PrintfString ("%s[%d:%llx]-%lld", name, fileid, blockid, partNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ElementFactory::NeedsSeparateElement (DgnCategoryId id) const
    {
    if (!m_geometryBuilder.IsValid())
        return  false;
    // different category    
    if (m_geometryBuilder->GetGeometryParams().GetCategoryId() != id)
        return  true;
    // too big of an element
    return  m_geometryBuilder->GetCurrentSize() > 20000000;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeometryPartId ElementFactory::CreateGeometryPart (DRange3dR range, double& partScale, TransformR geomToLocal, Utf8StringCR partTag, DwgImporter::GeometryEntry const& geomEntry)
    {
    DgnGeometryPartId   partId;
    partId.Invalidate ();

    auto& db = m_importer.GetDgnDb ();
    auto partBuilder = GeometryBuilder::CreateGeometryPart (db, m_is3d);
    if (!partBuilder.IsValid())
        return  partId;

    auto geomPart = DgnGeometryPart::Create (*m_partModel);
    if (!geomPart.IsValid())
        return  partId;

    // show part name as block name
    geomPart->SetUserLabel (geomEntry.GetBlockName().c_str());

    // build a valid part transform, and transform geometry in-place as necessary
    auto geometry = geomEntry.GetGeometry ();
    this->TransformGeometry (geometry, geomToLocal, &partScale);

    partBuilder->Append (geometry);

    // insert the new part to db
    if (partBuilder->Finish(*geomPart) != BSISUCCESS || !db.Elements().Insert<DgnGeometryPart>(*geomPart).IsValid())
        return  partId;

    partId = geomPart->GetId ();
    range = geomPart->GetBoundingBox ();

    // insert the new part into the syncInfo
    DwgSyncInfo::GeomPart   syncPart(partId, partTag);
    if (syncPart.Insert(db) != BeSQLite::BE_SQLITE_DONE)
        m_importer.ReportError (IssueCategory::Sync(), Issue::Error(), "failed adding geometry part in the SyncInfo");

    return  partId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementFactory::GetGeometryPart (DRange3dR range, double& partScale, TransformR geomToLocal, DgnGeometryPartId partId, DwgImporter::GeometryEntry const& geomEntry)
    {
    auto& db = m_importer.GetDgnDb ();
    if (DgnGeometryPart::QueryGeometryPartRange(range, db, partId) != BSISUCCESS)
        {
        BeAssert (false && "cannot query existing part range!!");
        return  BSIERROR;
        }

    // build a valid part transform from DWG transform
    geomToLocal.InitProduct (m_modelTransform, geomToLocal);
    DwgHelper::GetTransformForSharedParts (&geomToLocal, &partScale, geomToLocal);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::GetOrCreateGeometryPart (DwgImporter::SharedPartEntry& part, DwgImporter::GeometryEntry const& geomEntry, size_t partNo)
    {
    // this method creates a new shared part geometry
    auto& db = m_importer.GetDgnDb ();
    auto partTag = this->BuildPartCodeValue (geomEntry, partNo);
    auto geomToLocal = geomEntry.GetTransform ();
    double partScale = 0.0;
    DRange3d range;

    DgnGeometryPartId       partId;
    DwgSyncInfo::GeomPart   syncPart;
    if (DwgSyncInfo::GeomPart::FindByTag(syncPart, db, partTag.c_str()) == BSISUCCESS)
        partId = syncPart.GetPartId ();

    if (!partId.IsValid())
        {
        // create a new geometry part:
        partId = this->CreateGeometryPart (range, partScale, geomToLocal, partTag, geomEntry);
        if (!partId.IsValid())
            return  BSIERROR;
        }
    else
        {
        // use existing geometry part
        if (this->GetGeometryPart(range, partScale, geomToLocal, partId, geomEntry) != BSISUCCESS)
            return  BSIERROR;
        }

    part.SetTransform (Transform::FromProduct(m_invBaseTransform, geomToLocal));
    part.SetGeometryParams (geomEntry.GetGeometryParams());
    part.SetPartScale (partScale);
    part.SetPartId (partId);
    part.SetPartRange (range);

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::CreateSharedParts ()
    {
    BentleyStatus status = BSIERROR;
    if (nullptr == m_geometryMap || !m_sourceBlockId.IsValid())
        return  status;

    size_t  partIndex = 0;
    DwgImporter::T_SharedPartList parts;

    // create shared parts from geometry collection
    for (auto blockEntry : *m_geometryMap)
        {
        // build parts for all geometries, including nested blocks
        for (auto geomEntry : blockEntry.second)
            {
            // create a new geometry builder:
            DwgImporter::SharedPartEntry  part;
            status = this->GetOrCreateGeometryPart (part, geomEntry, partIndex++);
            if (status == BSISUCCESS)
                parts.push_back (part);

            m_importer.Progress ();
            }
        }

    // cache the parts created for this block
    auto& blockPartsMap = m_importer.GetBlockPartsR ();
    DwgImporter::SharedPartKey  key(m_sourceBlockId, m_sourceLayerId, m_basePartScale);
    blockPartsMap.insert (DwgImporter::T_BlockPartsEntry(key, parts));

    // create part elements from the part cache:
    status = this->CreatePartElements (parts);
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::CreatePartElements (DwgImporter::T_SharedPartList const& parts)
    {
    // create shared part elements
    BentleyStatus   status = BSIERROR;
    if (parts.empty())
        return  status;

    auto& targetModel = m_inputs.GetTargetModelR ();

    size_t  failed = 0;
    m_geometryBuilder = nullptr;

    // create elements from shared parts
    for (auto part : parts)
        {
        auto partCategoryId = part.GetGeometryParams().GetCategoryId ();

        // if the part is on a different category, create a new element and reset geometry builder for a new run
        if (this->NeedsSeparateElement(partCategoryId))
            status = this->CreateElement ();

        // initialize geometry builder for a new run, either for the 1st element or for a separated child element
        if (!m_geometryBuilder.IsValid())
            m_geometryBuilder = GeometryBuilder::Create (targetModel, partCategoryId, m_baseTransform);
        
        if (!m_geometryBuilder.IsValid())
            {
            BeAssert (false && "GeometryBuilder::Create has failed!");
            continue;
            }

        // apply base part scale
        auto geomToLocal = part.GetTransform ();
        this->ApplyPartScale (geomToLocal, part.GetPartScale(), false);

        m_geometryBuilder->Append (part.GetGeometryParams());
        m_geometryBuilder->Append (part.GetPartId(), geomToLocal, part.GetPartRange());

        m_importer.Progress ();
        }

    if (!m_geometryBuilder.IsValid())
        {
        BeAssert (false && "GeometryBuilder::Create has failed!");
        return  BSIERROR;
        }

    status = this->CreateElement ();
    if (status != BSISUCCESS)
        failed++;

    if (failed > 0)
        {
        Utf8PrintfString msg("%lld out of %lld shared part geometry element(s) is/are not created for entity[%s, handle=%llx]!", failed, parts.size(), m_elementCode.GetValueUtf8CP(), m_inputs.GetEntityId().ToUInt64());
        m_importer.ReportError (IssueCategory::VisualFidelity(), Issue::Error(), msg.c_str());
        if (BSISUCCESS == status)
            status = BSIERROR;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::CreateIndividualElements ()
    {
    BentleyStatus status = BSIERROR;
    if (nullptr == m_geometryMap)
        return  status;

    // create new elements from the geometry collection
    auto headerCategoryId = m_createParams.GetCategoryId ();
    auto& targetModel = m_inputs.GetTargetModelR ();
    auto firstLocalToGeom = Transform::FromIdentity ();
    auto firstGeomToLocal = Transform::FromIdentity ();
    auto firstGeomToWorld = Transform::FromIdentity ();
    auto firstWorldToGeom = Transform::FromIdentity ();

    size_t  failed = 0, total = 0;
    for (auto blockEntry : *m_geometryMap)
        {
        for (auto geomEntry : blockEntry.second)
            {
            auto currentGeomToWorld = geomEntry.GetTransform ();
            auto categoryId = geomEntry.GetGeometryParams().GetCategoryId ();
            auto geometry = geomEntry.GetGeometryP ();
            if (nullptr == geometry)
                {
                BeAssert (false && "Invalid geometry!");
                continue;
                }

            // transform DWG geometry
            this->TransformGeometry (*geometry, currentGeomToWorld);

            // if the geometry is on a different category, create a separate element and reset geometry builder for a new run
            if (this->NeedsSeparateElement(categoryId))
                status = this->CreateElement ();

            if (!m_geometryBuilder.IsValid())
                {
                Transform   localToWorld;
                if (m_hasBaseTransform)
                    {
                    // build geometries relative to the base transform
                    localToWorld = m_baseTransform;

                    firstGeomToLocal.InitProduct (m_invBaseTransform, currentGeomToWorld);
                    firstLocalToGeom = firstGeomToLocal.ValidatedInverse ();
                    }
                else
                    {
                    // build geometries relative to the first geometry
                    if (geometry->GetLocalCoordinateFrame(firstLocalToGeom))
                        this->Validate2dTransform (firstLocalToGeom);
                    else
                        firstLocalToGeom.InitIdentity ();
                    firstGeomToLocal = firstLocalToGeom.ValidatedInverse ();

                    localToWorld = Transform::FromProduct (currentGeomToWorld, firstLocalToGeom);
                    }

                m_geometryBuilder = GeometryBuilder::Create (targetModel, categoryId, localToWorld);
                if (!m_geometryBuilder.IsValid())
                    continue;
                
                // save off of the first geometry transform for rest of the geometries in the collection
                firstGeomToWorld = currentGeomToWorld;
                firstWorldToGeom = currentGeomToWorld.ValidatedInverse ();
                }

            if (m_geometryBuilder.IsValid())
                {
                Transform currentLocalToGeom;
                if (firstGeomToWorld.IsEqual(currentGeomToWorld))
                    {
                    // the first geometry or one has the same tranform
                    currentLocalToGeom = firstLocalToGeom;
                    }
                else
                    {
                    // transform geometry relative to first local
                    auto toFirstGeom = Transform::FromProduct (firstWorldToGeom, currentGeomToWorld);
                    auto toFirstLocal = Transform::FromProduct (firstGeomToLocal, toFirstGeom);
                    currentLocalToGeom = toFirstLocal.ValidatedInverse ();
                    }

                if (!currentLocalToGeom.IsIdentity())
                    {
                    // transform geometry relative to the first geometry
                    auto geomToLocal = currentLocalToGeom.ValidatedInverse ();
                    geometry->TransformInPlace (geomToLocal);

                    // transform pattern but not linestyles
                    auto& display = geomEntry.GetGeometryParamsR ();
                    display.ApplyTransform (geomToLocal, 0x01);
                    }

                m_geometryBuilder->Append (geomEntry.GetGeometryParams());
                m_geometryBuilder->Append (*geometry);

                m_importer.Progress ();
                }

            total++;
            }
        }

    if (m_geometryBuilder.IsValid())
        {
        status = this->CreateElement ();
        if (status != BSISUCCESS)
            failed++;
        }

    if (failed > 0)
        {
        Utf8PrintfString msg("%lld out of %lld individual geometry element(s) is/are not created for entity[%s, handle=%llx]!", failed, total, m_elementCode.GetValueUtf8CP(), m_inputs.GetEntityId().ToUInt64());
        m_importer.ReportError (IssueCategory::VisualFidelity(), Issue::Error(), msg.c_str());
        if (BSISUCCESS == status)
            status = BSIERROR;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::CreateEmptyElement ()
    {
    m_results.m_importedElement = m_elementHandler->Create (m_elementParams);

    auto geomSource = m_results.m_importedElement->ToGeometrySourceP ();
    if (nullptr == geomSource)
        {
        BeAssert (false && L"Null geometry source!");
        return  BSIERROR;
        }
    geomSource->SetCategoryId (m_createParams.GetCategoryId());
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::CreateElement ()
    {
    // create elements from current GeometryBuilder containing up to date geometry collection, then flush the builder.
    if (!m_geometryBuilder.IsValid() || nullptr == m_elementHandler)
        {
        BeAssert (false && L"Invalid GeometricBuilder and/or ElementHandler!");
        return  BSIERROR;
        }

#ifdef NEED_UNIQUE_CODE_PER_ELEMENT
    if (m_results.m_importedElement.IsValid())
        {
        // a parent element has been created - set element params for children
        Utf8PrintfString    codeValue("%s-%d", m_elementCode.GetValue(), m_results.m_childElements.size() + 1);
        DgnCode             childCode = m_importer.CreateCode (codeValue);
        m_elementParams.SetCode (childCode);
        }
#endif  // NEED_UNIQUE_CODE_PER_ELEMENT

    // create a new element from current geometry builder:
    DgnElementPtr   element = m_elementHandler->Create (m_elementParams);
    if (!element.IsValid())
        {
        BeAssert (false && L"Null element!");
        return  BSIERROR;
        }

    auto geomSource = element->ToGeometrySourceP ();
    if (nullptr == geomSource)
        {
        BeAssert (false && L"Null geometry source!");
        return  BSIERROR;
        }

    // category must be preset through GeometryBuilder inialization or else Finish will fail
    auto categoryId = m_geometryBuilder->GetGeometryParams().GetCategoryId ();

    // set category and save geometry source to the new element
    if (DgnDbStatus::Success != geomSource->SetCategoryId(categoryId) || BSISUCCESS != m_geometryBuilder->Finish(*geomSource))
        return  BSIERROR;

    if (!m_results.m_importedElement.IsValid())
        {
        // the first geometry builder is the parent element:
        m_results.m_importedElement = element;
        }
    else
        {
        // rest of the geomtry collection are children:
        DwgImporter::ElementImportResults    childResults(element.get());
        m_results.m_childElements.push_back (childResults);
        }

    // reset the geometry builder for next new element
    m_geometryBuilder = nullptr;

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ElementFactory::CreateElements (DwgImporter::T_BlockGeometryMap const* geometryMap)
    {
    if (nullptr == geometryMap)
        return  BSIERROR;

    m_geometryMap = geometryMap;
    m_geometryBuilder = nullptr;

    // if no geometry collected, return an empty element as a host element for ElementAspects etc
    size_t  num = m_geometryMap->size ();
    if (num == 0)
        return  this->CreateEmptyElement ();

    // check the inputs and set the factory to create either shared parts or individual elements
    this->SetDefaultCreation ();

    // begin creating elements from the input geometry collection
    if (m_canCreateSharedParts)
        return this->CreateSharedParts ();
    else
        return this->CreateIndividualElements ();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     PrepareEntityForImport (DrawParameters& drawParams, GeometryFactory& factory, DwgDbEntityP entity)
    {
    bool    entityUpdated = false;

    // resolve the root effective ByBlock symbology:
    drawParams.ResolveRootEffectiveByBlockSymbology ();

    // remove gradient from hatch as otherwise we will get a mesh. We don't the hatch anymore - GradientSymb has already been saved:
    DwgDbHatchP         hatch = DwgDbHatch::Cast (entity);
    if (nullptr != hatch && hatch->IsGradient() && DwgDbStatus::Success == hatch->UpgradeOpen())
        {
        hatch->SetHatchType (DwgDbHatch::HatchType::Hatch);
        hatch->SetPattern (DwgDbHatch::PatternType::PreDefined, DwgString(L"SOLID"));
        hatch->DowngradeOpen ();

        entityUpdated = true;
        }

    DwgDbBlockReferenceCP   insert = DwgDbBlockReference::Cast(entity);
    if (nullptr != insert)
        {
        // if a block reference is clipped, get the clipper and set it into the geometry factory:
        DwgDbSpatialFilterPtr   spatialFilter;
        if (DwgDbStatus::Success == insert->OpenSpatialFilter(spatialFilter, DwgDbOpenMode::ForRead))
            factory.SetSpatialFilter (spatialFilter.get());
        }

    return  entityUpdated;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::_SkipEmptyElement (DwgDbEntityCP entity)
    {
    // create an empty host element for block attributes
    DwgDbBlockReferenceCP   blockRef = DwgDbBlockReference::Cast(entity);
    if (nullptr != blockRef)
        {
        DwgDbObjectIteratorPtr  attrIter = blockRef->GetAttributeIterator ();
        if (attrIter.IsValid() && attrIter->IsValid())
            {
            attrIter->Start ();
            if (!attrIter->Done())
                return  false;

            // no variable attributes - does the block have constant attrdef's?
            DwgDbObjectIdArray  ids;
            if (this->GetConstantAttrdefIdsFor(ids, blockRef->GetBlockTableRecordId()) && !ids.empty())
                return  false;
            }
        }

    // by default, no empty elements should be created!
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportEntity (ElementImportResults& results, ElementImportInputs& inputs)
    {
    DwgDbEntityP    entity = inputs.GetEntityP ();
    if (nullptr == entity)
        return  BSIERROR;

    uint64_t    entityId = entity->GetObjectId().ToUInt64 ();

    // set DgnDbElement creation options
    ElementCreateParams  createParams(inputs.GetTargetModelR());

    BentleyStatus   status = this->_GetElementCreateParams (createParams, inputs.GetTransform(), *entity);
    if (BSISUCCESS != status)
        return  status;

    // if the entity has a parent, use it for ByBlock symbology:
    DwgDbEntityCP   parent = inputs.GetParentEntity ();

    // set DWG entity draw options:
    GeometryOptions     geomOptions = this->_GetCurrentGeometryOptions ();
    DrawParameters      drawParams (*entity, *this, parent);
    GeometryFactory     geomFactory(createParams, drawParams, geomOptions, entity);

    // prepare for import
    PrepareEntityForImport (drawParams, geomFactory, entity);

    // draw entity and create geometry from it in our factory:
    try
        {
        entity->Draw (geomFactory, geomOptions, drawParams);
        }
    catch (...)
        {
        this->ReportError (IssueCategory::Unknown(), Issue::Exception(), IssueReporter::FmtEntity(*entity).c_str());
        }

    auto geometryMap = geomFactory.GetOutputGeometryMap ();
    if (geometryMap.empty() && this->_SkipEmptyElement(entity))
        {
        this->ReportIssue (IssueSeverity::Warning, IssueCategory::MissingData(), Issue::EmptyGeometry(), Utf8PrintfString("%ls, ID=%llx", entity->GetDwgClassName().c_str(), entityId).c_str());
        return  BSIERROR;
        }

    // create elements from collected geometries
    ElementFactory  elemFactory(results, inputs, createParams, *this);
    status = elemFactory.CreateElements (&geometryMap);

    if (BSISUCCESS == status)
        m_entitiesImported++;

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::ImportNewEntity (ElementImportResults& results, ElementImportInputs& inputs, DwgDbObjectIdCR desiredOwnerId)
    {
    DwgDbEntityP    entity = inputs.GetEntityP ();
    if (nullptr == entity)
        return  BSIERROR;

    // the parent entity for ByBlock symbology, and template entity for ID dependent symbology (layer, linetype, etc):
    DwgDbEntityCP   parentEntity = inputs.GetParentEntity ();
    DwgDbEntityCP   templateEntity = (entity->GetObjectId().IsValid() || inputs.GetTemplateEntity() == nullptr) ? entity : inputs.GetTemplateEntity();

    DwgGiDrawablePtr    drawable = entity->GetDrawable ();
    if (!drawable.IsValid())
        return  BSIERROR;

    // set DgnDbElement creation options
    ElementCreateParams createParams(inputs.GetTargetModelR());

    BentleyStatus   status = this->_GetElementCreateParams (createParams, inputs.GetTransform(), *templateEntity);
    if (BSISUCCESS != status)
        return  status;

    // set DWG entity draw options:
    GeometryOptions     geomOptions = this->_GetCurrentGeometryOptions ();
    DrawParameters      drawParams (*entity, *this, parentEntity, templateEntity);
    GeometryFactory     geomFactory(createParams, drawParams, geomOptions, entity);

    // prepare for import
    if (PrepareEntityForImport(drawParams, geomFactory, entity))
        {
        // entity is changed, make sure drawble still valid
        drawable = entity->GetDrawable ();
        if (!drawable.IsValid())
            return  BSIERROR;
        }

    // draw drawble and create geometry in our factory:
    try
        {
        drawable->Draw (geomFactory, geomOptions, drawParams);
        }
    catch (...)
        {
        this->ReportError (IssueCategory::Unknown(), Issue::Exception(), IssueReporter::FmtEntity(*entity).c_str());
        }

    auto geometryMap = geomFactory.GetOutputGeometryMap ();
    if (geometryMap.empty())
        {
        this->ReportIssue (IssueSeverity::Warning, IssueCategory::MissingData(), Issue::EmptyGeometry(), Utf8PrintfString("%ls", entity->GetDwgClassName().c_str()).c_str());
        return  BSIERROR;
        }

    ElementFactory  elemFactory(results, inputs, createParams, *this);
    status = elemFactory.CreateElements (&geometryMap);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::_GetElementLabel (DwgDbEntityCR entity)
    {
    // default element label to entity name:
    Utf8String  label(entity.GetDxfName().c_str());
    if (label.empty())
        label.Assign (entity.GetDwgClassName().c_str());
    
    // for a named block reference, use block name:
    DwgDbBlockReferenceCP   insert = DwgDbBlockReference::Cast(&entity);
    if (nullptr != insert)
        {
        DwgDbBlockTableRecordPtr block (insert->GetBlockTableRecordId(), DwgDbOpenMode::ForRead);
        if (!block.IsNull() && !block->IsAnonymous())
            {
            // display block name as element label:
            DwgString   blockName = block->GetName ();
            if (!blockName.IsEmpty())
                {
                Utf8String  insertName(blockName.c_str());
                label.assign (insertName.c_str());
                }
            }
        }
    return  label;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_GetElementCreateParams (DwgImporter::ElementCreateParams& params, TransformCR toDgn, DwgDbEntityCR ent, Utf8CP desiredCode)
    {
    DgnModelCR      model = params.GetModel ();
    DwgDbObjectId   layerId = ent.GetLayerId ();
    DwgDbDatabaseP  xrefDwg = ent.GetDatabase().get ();

    // use entity's database if it's in an xref file; otherwise use current database:
    if (nullptr != xrefDwg && xrefDwg == m_dwgdb.get())
        xrefDwg = nullptr;

    // GeometryParts 
    params.m_geometryPartsModel = this->GetGeometryPartsModel ();

    if (model.Is3d())
        {
        // get spatial category & subcategory from the syncInfo:
        params.m_categoryId = this->GetSpatialCategory (params.m_subCategoryId, layerId, xrefDwg);
        }
    else
        {
        // get or create a drawing category and/or a subcategory for the layer & the viewport:
        DwgDbObjectId   viewportId = this->GetCurrentViewportId ();
        params.m_categoryId = this->GetOrAddDrawingCategory (params.m_subCategoryId, layerId, viewportId, model, xrefDwg);
        }

    if (!params.m_categoryId.IsValid())
        {
        Utf8PrintfString msg("[%s] - Invalid layer %llx", IssueReporter::FmtEntity(ent).c_str(), ent.GetLayerId().ToUInt64());
        this->ReportError(IssueCategory::CorruptData(), Issue::ImportFailure(), msg.c_str());
        return BSIERROR;
        }

    // WIP - apply entity ECS?
    // Transform   ecs;
    // ent.GetEcs (ecs);
    // params.m_transform.InitProduct (ecs, toDgn);
    params.m_transform = toDgn;

    params.m_placementPoint = DwgHelper::DefaultPlacementPoint (ent);

#ifdef NEED_UNIQUE_CODE_PER_ELEMENT
    Utf8String      codeNamespace = model.GetName ();
    Utf8String      codeValue;
    if (nullptr != desiredCode)
        codeValue.assign (desiredCode);
    else
        codeValue.Sprintf ("%s:%llx", codeNamespace.c_str(), ent.GetObjectId().ToUInt64());
    params.m_elementCode = this->CreateCode (codeValue);
#else
    if (nullptr != desiredCode)
        params.m_elementCode = this->CreateCode (desiredCode);
#endif  // NEED_UNIQUE_CODE_PER_ELEMENT

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::_FilterEntity (ElementImportInputs& inputs) const
    {
    auto entity = inputs.GetEntityP ();
    if (nullptr == entity)
        return  true;

    // don't draw invisible entities:
    if (DwgDbVisibility::Invisible == entity->GetVisibility())
        return  true;

    // trivial reject clipped away entity:
    auto spatialFilter = inputs.GetSpatialFilter ();
    if (nullptr != spatialFilter && spatialFilter->IsEntityFilteredOut(*entity))
        return  true;

    // trivial reject 3D elements in a 2D model
    if (!inputs.GetTargetModel().Is3d())
        {
        if (DwgDb3dSolid::Cast(entity) != nullptr ||
            DwgDbBody::Cast(entity) != nullptr ||
            DwgDbExtrudedSurface::Cast(entity) != nullptr ||
            DwgDbLoftedSurface::Cast(entity) != nullptr ||
            DwgDbNurbSurface::Cast(entity) != nullptr ||
            DwgDbRevolvedSurface::Cast(entity) != nullptr ||
            DwgDbSweptSurface::Cast(entity) != nullptr)
            return  true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportEntityByProtocolExtension (ElementImportResults& results, ElementImportInputs& inputs, DwgProtocolExtension& objExt)
    {
    ProtocolExtensionContext context(inputs, results);
    return objExt._ConvertToBim (context, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::ImportEntity (ElementImportResults& results, ElementImportInputs& inputs)
    {
    // if the entity is extended to support ToDgnDb method, let it take over the importing:
    DwgDbEntityPtr&         entity = inputs.m_entity;
    if (entity.IsNull())
        return  BSIERROR;

    DwgProtocolExtension*   objExt = DwgProtocolExtension::Cast (entity->QueryX(DwgProtocolExtension::Desc()));
    if (nullptr != objExt)
        return  this->_ImportEntityByProtocolExtension (results, inputs, *objExt);

    return  this->_ImportEntity (results, inputs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::ImportOrUpdateEntity (ElementImportInputs& inputs)
    {
    DwgDbObjectP    object = DwgDbObject::Cast (inputs.GetEntityP());
    if (nullptr == object)
        return  BentleyStatus::BSIERROR;

    IDwgChangeDetector& changeDetector = this->_GetChangeDetector ();
    IDwgChangeDetector::DetectionResults    detectionResults;
    ElementImportResults    elementResults;
    BentleyStatus   status = BentleyStatus::SUCCESS;

    // consult the sync info to see if the entity has been changed, and what action to take:
    if (changeDetector._IsElementChanged(detectionResults, *this, *object, inputs.GetModelMapping()))
        {
        // set existing element in elementResults primarily for entity protocol extensions:
        elementResults.SetExistingElement (detectionResults.GetObjectMapping());
        // create a new non-database resident element
        status = this->ImportEntity (elementResults, inputs);
        }

    if (BentleyStatus::SUCCESS != status)
        return  status;

    // act based on change detector results:
    switch (detectionResults.GetChangeType())
        {
        case IDwgChangeDetector::ChangeType::None:
            {
            // no change - just update input entity for output results
            elementResults.SetExistingElement (detectionResults.GetObjectMapping());
            changeDetector._OnElementSeen (*this, detectionResults.GetExistingElementId());
            break;
            }

        case IDwgChangeDetector::ChangeType::Insert:
            {
            // new entity - insert results into BIM
            this->InsertResults (elementResults);
            if (elementResults.GetImportedElement() != nullptr)
                changeDetector._OnElementSeen (*this, elementResults.GetImportedElement()->GetElementId());
            break;
            }

        case IDwgChangeDetector::ChangeType::Update:
            {
            // existing element needs update
            this->UpdateResults (elementResults, detectionResults.GetExistingElementId());
            changeDetector._OnElementSeen (*this, detectionResults.GetExistingElementId());
            }
        }

    this->InsertOrUpdateResultsInSyncInfo (elementResults, detectionResults, inputs.GetEntity(), inputs.GetModelMapping().GetModelSyncInfoId());

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgImporter::OpenAndImportEntity (ElementImportInputs& inputs)
    {
    inputs.m_entity.OpenObject (inputs.m_entityId, DwgDbOpenMode::ForRead);

    if (inputs.m_entity.OpenStatus() != DwgDbStatus::Success)
        {
        Utf8PrintfString  msg("Entity: %ls, ID= %llx", inputs.m_entityId.GetDwgClassName().c_str(), inputs.m_entityId.ToUInt64());
        this->ReportError (IssueCategory::UnexpectedData(), Issue::CantOpenObject(), msg.c_str());
        return;
        }

    this->Progress ();

    if (!this->_FilterEntity(inputs))
        this->ImportOrUpdateEntity (inputs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportEntitySection ()
    {
    this->SetStepName (ProgressMessage::STEP_IMPORTING_ENTITIES());

    DwgDbBlockTableRecordPtr    modelspace(m_modelspaceId, DwgDbOpenMode::ForRead);
    if (modelspace.IsNull())
        return  BSIERROR;

    DwgDbBlockChildIteratorPtr  entityIter = modelspace->GetBlockChildIterator ();
    if (!entityIter.IsValid() || !entityIter->IsValid())
        return  BSIERROR;

    ResolvedModelMapping    modelMap = this->FindModel (m_modelspaceId, this->GetRootTransform(), DwgSyncInfo::ModelSourceType::ModelSpace);
    if (!modelMap.IsValid())
        {
        this->ReportError (IssueCategory::UnexpectedData(), Issue::Error(), "cannot find the default target model!");
        return  BSIERROR;
        }

    if (this->_GetChangeDetector()._ShouldSkipModel(*this, modelMap))
        {
        // no entity change is detected in modelspace - check all xRef files attached to it:
        if (this->ShouldSkipAllXrefs(modelMap, m_modelspaceId))
            return  BSISUCCESS;
        }

    // set modelspace as current space being processed
    m_currentspaceId = m_modelspaceId;

    // set geometry options to draw all entities in the entity section
    DwgDbObjectId       activeVport = m_dwgdb->GetActiveModelspaceViewportId ();
    GeometryOptions&    currOptions = this->_GetCurrentGeometryOptions ();
    currOptions.SetDatabase (m_dwgdb.get());
    currOptions.SetViewportId (activeVport);

    size_t  numIsolines = m_dwgdb->GetISOLINES ();
    currOptions.SetNumberOfIsolines (numIsolines == 0 ? 1 : numIsolines);

    DwgDbViewportTableRecordPtr vport(activeVport, DwgDbOpenMode::ForRead);

    if (this->GetOptions().IsRenderableGeometryPrefered())
        {
        // user prefers renderable geometry - use Isometric view and regen for rendering:
        currOptions.SetRegenType (DwgGiRegenType::RenderCommand);
        currOptions.SetViewDirection (DVec3d::From(-0.577350269189626, -0.57735026918962573, 0.57735026918962573));
        currOptions.SetCameraUpDirection (DVec3d::UnitY());
        currOptions.SetCameraLocation (DPoint3d::FromZero());
        }
    else
        {
        // create geometry based on the default settings of the active viewport:
        if (DwgDbStatus::Success == vport.OpenStatus())
            {
            DVec3d  normal = vport->GetViewDirection ();
            normal.Normalize ();

            currOptions.SetViewDirection (normal);
            currOptions.SetRegenType (DwgGiRegenType::StandardDisplay);

            // Set cameraUp direction to the y-axis of the view:
            double          twist = vport->GetViewTwist ();
            if (fabs(twist) > 0.01)
                currOptions.SetCameraUpDirection (DVec3d::From(sin(twist), cos(twist), 0.0));
            // Camera is always at 0,0,0 in a viewport?
            currOptions.SetCameraLocation (DPoint3d::FromZero());

            DwgDbVisualStylePtr     vistyle(vport->GetVisualStyle(), DwgDbOpenMode::ForRead);
            if (DwgDbStatus::Success == vistyle.OpenStatus())
                {
                Render::RenderMode  renderMode = DwgHelper::GetRenderModeFromVisualStyle (*vistyle.get());
                if (renderMode != Render::RenderMode::Wireframe)
                    currOptions.SetRegenType (DwgGiRegenType::RenderCommand);
                }
            }
        }

    double  currentPDSIZE = m_dwgdb->GetPDSIZE ();
    bool    resetPDSIZE = false;

    // set currently drawing viewport range:
    if (DwgDbStatus::Success == vport.OpenStatus())
        {
        currOptions.SetViewportRange (DwgHelper::GetRangeFrom(vport->GetCenterPoint(), vport->GetWidth(), vport->GetHeight()));

        // change percentage PDSIZE to an absolute size to get a desired PDMODE geometry:
        if (m_dwgdb->GetPDMODE() != 0 && m_dwgdb->GetPDSIZE() <= 0.0)
            {
            m_dwgdb->SetPDSIZE (DwgHelper::GetAbsolutePDSIZE(m_dwgdb->GetPDSIZE(), vport->GetHeight()));
            resetPDSIZE = true;
            }
        vport.CloseObject ();
        }

    this->SetTaskName (ProgressMessage::TASK_IMPORTING_MODEL(), modelMap.GetModel()->GetName().c_str());
    this->Progress ();

    ElementImportInputs     inputs (*modelMap.GetModel());
    inputs.SetClassId (this->_GetElementType(*modelspace.get()));
    inputs.SetTransform (this->GetRootTransform());
    inputs.SetSpatialFilter (nullptr);
    inputs.SetModelMapping (modelMap);

    // SortEnts table
    DwgDbSortentsTablePtr   sortentsTable;
    if (DwgDbStatus::Success == modelspace->OpenSortentsTable(sortentsTable, DwgDbOpenMode::ForRead))
        {
        // import entities in sorted order:
        DwgDbObjectIdArray  entities;
        if (DwgDbStatus::Success == sortentsTable->GetFullDrawOrder(entities))
            {
            for (DwgDbObjectIdCR id : entities)
                {
                inputs.SetEntityId (id);
                this->OpenAndImportEntity (inputs);
                }
            }
        }
    else
        {
        // import entities in database order
        for (entityIter->Start(); !entityIter->Done(); entityIter->Step())
            {
            inputs.SetEntityId (entityIter->GetEntityId());
            this->OpenAndImportEntity (inputs);
            }
        }

    // restore original PDSIZE
    if (resetPDSIZE)
        m_dwgdb->SetPDSIZE (currentPDSIZE);
        
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::_ArePointsValid (DPoint3dCP checkPoints, size_t numPoints, DwgDbEntityCP entity)
    {
    static double   s_tooBigCoordinate = 1.0e10;
    for (DPoint3dCP lastPoint = checkPoints + numPoints; checkPoints < lastPoint; checkPoints++)
        {
        if (checkPoints->x < -s_tooBigCoordinate || checkPoints->x > s_tooBigCoordinate ||
            checkPoints->y < -s_tooBigCoordinate || checkPoints->y > s_tooBigCoordinate ||
            checkPoints->z < -s_tooBigCoordinate || checkPoints->z > s_tooBigCoordinate)
            return  false;
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId      DwgImporter::_GetElementType (DwgDbBlockTableRecordCR block)
    {
    BentleyApi::ECN::ECClassCP  elementClass = nullptr;

    // a physical class for a modelspace or a drawing graphic class for a paperspace entity
    if (block.IsModelspace())
        elementClass = this->GetDgnDb().Schemas().GetClass (GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject);
    else if (block.IsLayout())
        elementClass = this->GetDgnDb().Schemas().GetClass (BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic);
    else if (block.IsExternalReference())
        elementClass = this->GetDgnDb().Schemas().GetClass (GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject);
    else
        BeAssert (false && L"Unexpected element mapping!");

    if (nullptr == elementClass)
        {
        this->ReportError (IssueCategory::MissingData(), Issue::ImportFailure(), "Missing ECSchemas or element class not mapped!");
        return DgnClassId ();
        }

    return DgnClassId (elementClass->GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus     DwgImporter::InsertResults (ElementImportResults& results)
    {
    if (!results.m_importedElement.IsValid())
        return DgnDbStatus::Success;

    // insert the primary element
    DgnDbStatus status = DgnDbStatus::Success;
    DgnCode     code = results.m_importedElement->GetCode ();
    auto        ret = GetDgnDb().Elements().Insert (*results.m_importedElement, &status);

    if (DgnDbStatus::DuplicateCode == status)
        {
        Utf8String  duplicateMessage;
        duplicateMessage.Sprintf ("Duplicate element code '%s' ignored", code.GetValueUtf8().c_str());
        ReportIssue (IssueSeverity::Warning, IssueCategory::InconsistentData(), Issue::Message(), duplicateMessage.c_str());

        DgnDbStatus setStatus = results.m_importedElement->SetCode (DgnCode::CreateEmpty());
        BeAssert(DgnDbStatus::Success == setStatus);

        ret = GetDgnDb().Elements().Insert (*results.m_importedElement, &status);
        }

    if (DgnDbStatus::Success != status)
        {
        ReportError (IssueCategory::VisualFidelity(), Issue::Message(), Utf8PrintfString("Unable to insert element [status=%d]", status).c_str());
        return status;
        }

    if (ret.IsValid())  // an Invalid element is acceptable and means it was purposefully discarded during import
        LOG_ENTITY.tracev ("Inserted %s, %s", IssueReporter::FmtElement(*ret).c_str(), ret->GetDisplayLabel().c_str());

    DgnElementId    parentId = results.m_importedElement->GetElementId ();

    // insert the children of the primary elements, if any
    for (ElementImportResults& child : results.m_childElements)
        {
        DgnElementP childElement = child.GetImportedElement ();
        if (childElement == nullptr)
            continue;

        childElement->SetParentId (parentId, m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));

        status = this->InsertResults (child);
        if (DgnDbStatus::Success != status)
            return status;
        }
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::InsertOrUpdateResultsInSyncInfo (ElementImportResults& results, IDwgChangeDetector::DetectionResults const& updatePlan, DwgDbEntityCR entity, DwgSyncInfo::DwgModelSyncInfoId const& modelSyncId)
    {
    // This method attempts to insert a new, or to update an existing, element in the sync info.
    if (updatePlan.GetChangeType() == IDwgChangeDetector::ChangeType::None)
        return  BSISUCCESS;
    
    DgnElementP element = results.GetImportedElement ();
    if (nullptr == element || !modelSyncId.IsValid())
        return  BSIERROR;

    BentleyStatus   status = BentleyStatus::SUCCESS;

    // a block reference needs a secondary hash from its block definition:
    bool    hash2nd = this->GetOptions().GetSyncBlockChanges() && DwgDbBlockReference::Cast (&entity);

    // create a provenence from the source DWG object:
    DwgSyncInfo::DwgObjectProvenance    entityprov (*DwgDbObject::Cast(&entity), this->GetSyncInfo(), this->GetCurrentIdPolicy(), hash2nd);
    // insert a new or update an existing element
    if (updatePlan.GetChangeType() == IDwgChangeDetector::ChangeType::Insert)
        status = this->GetSyncInfo().InsertElement (element->GetElementId(), entity, entityprov, modelSyncId);
    else
        status = this->GetSyncInfo().UpdateElement (element->GetElementId(), entity, entityprov);

    if (BSISUCCESS != status)
        {
        uint64_t    entityId = entity.GetObjectId().ToUInt64 ();
        Utf8PrintfString msg("failed inserting element into DwgSynchInfo for entityId=%llx!", entityId);
        this->ReportError (IssueCategory::Sync(), Issue::Error(), msg.c_str());
        }

    // insert or update children
    for (auto& child : results.m_childElements)
        status = this->InsertOrUpdateResultsInSyncInfo (child, updatePlan, entity, modelSyncId);

    this->Progress ();

    return  status;
    }
