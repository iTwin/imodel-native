/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ImportEntities.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DGNDBSYNC_DWG

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

void Initialize (DwgDbEntityCR ent, DrawParameters const* parentParams = nullptr)
    {
    m_color = ent.GetEntityColor ();
    m_layerId = ent.GetLayerId ();
    m_linetypeId = ent.GetLinetypeId ();
    m_materialId = ent.GetMaterialId ();
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

    // if this is a new entity, use master file:
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
    DrawParameters (DwgDbEntityCR ent, DwgImporter& importer, DwgDbEntityCP parent = nullptr) : m_dwgImporter(importer)
        {
        if (nullptr != parent)
            {
            DrawParameters  parentParams(*parent, importer);
            this->Initialize (ent, &parentParams);
            return;
            }
        this->Initialize (ent);
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
    params.SetFillDisplay(DwgGiFillType::Always == m_filltype ? Render::FillDisplay::Always : Render::FillDisplay::Never);
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
        DgnMaterialId   materialId = this->GetDgnMaterial ();
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

    BeDataAssert (false && "Invalid DWG linetype object ID!!");
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
DgnMaterialId   GetDgnMaterial () const
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

    BeDataAssert (false && "Invalid DWG material object ID!!");
    return  DgnMaterialId();
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
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
struct GeometryFactory : public IDwgDrawGeometry
{
typedef DwgImporter::T_GeometryBuilderList          BuilderList;
typedef DwgImporter::T_GeometryBuilderList&         BuilderListR;
typedef DwgImporter::T_GeometryBuilderList const&   BuilderListCR;
typedef DwgImporter::GeometryBuilderInfo            BuilderInfo;
typedef DwgImporter::GeometryBuilderInfo&           BuilderInfoR;
typedef DwgImporter::GeometryBuilderInfo const&     BuilderInfoCR;

private:
    // members used by the toolkit
    bvector<Transform>                  m_transformStack;
    Transform                           m_baseTransform;
    Transform                           m_currentTransform;
    DrawParameters&                     m_drawParams;
    // members used locally
    DwgImporter::ElementCreateParams&   m_createParams;
    DwgImporter::GeometryOptions*       m_geometryOptions;
    BuilderList                         m_outputGeometryList;
    DwgImporter::T_PartIndexList        m_outputPartIndexList;
    DwgDbEntityCP                       m_entity;
    BentleyStatus                       m_status;
    Transform                           m_worldToElement;
    bool                                m_isDrawingBlock;
    bool                                m_isAssembly;
    DwgDbSpatialFilterCP                m_spatialFilter;
    BentleyApi::MD5                     m_geometryHash;
    bvector<Utf8String>                 m_partNamespaceStack;
    BuilderInfo                         m_invalidBuilder;
    
public:
GeometryFactory (DwgImporter::ElementCreateParams& createParams, DrawParameters& drawParams, DwgImporter::GeometryOptions& opts, DwgDbEntityCP ent) : m_drawParams(drawParams), m_createParams(createParams)
    {
    m_geometryOptions = &opts;
    m_entity = ent;
    m_status = BSISUCCESS;
    m_transformStack.clear ();
    m_baseTransform = createParams.m_transform;
    m_currentTransform = m_baseTransform;
    m_outputGeometryList.clear ();
    m_outputPartIndexList.clear ();
    m_worldToElement.InitIdentity ();
    m_isDrawingBlock = false;
    m_isAssembly = false;
    m_spatialFilter = nullptr;
    m_geometryHash.Reset ();
    m_partNamespaceStack.clear ();
    }

BuilderListCR   GetGeometryBuilderList () const { return m_outputGeometryList; }
DwgImporter::T_PartIndexList const& GetPartIndexList () const { return m_outputPartIndexList; }
BentleyStatus   GetStatus () { return m_status; }
// tracking block drawing - all blocks.
bool            IsDrawingBlock () { return m_isDrawingBlock; }
void            SetDrawingBlock (bool inBlock) { m_isDrawingBlock = inBlock; }
// tracking part creation - only named, non-xref, non-layout blocks.
bool            IsAssembly () { return m_isAssembly; }
void            SetIsAssembly (bool isAssembly) { m_isAssembly = isAssembly; }
void            SetSpatialFilter (DwgDbSpatialFilterCP filter) { m_spatialFilter = filter; }
void            PushPartNamespace (Utf8StringCR name) { m_partNamespaceStack.push_back(name); }
void            PopPartNamespace () { m_partNamespaceStack.pop_back(); }
Utf8StringCR    GetPartNamespace () { return m_partNamespaceStack.back(); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    HashTextData (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, double h, double w, double oblique, DwgStringCR string, DgnFontCP font)
    {
    m_geometryHash.Reset ();
    m_geometryHash.Add (&position, sizeof position);
    m_geometryHash.Add (&normal, sizeof normal);
    m_geometryHash.Add (&xdir, sizeof xdir);
    m_geometryHash.Add (&h, sizeof h);
    m_geometryHash.Add (&w, sizeof w);
    m_geometryHash.Add (&oblique, sizeof oblique);
    if (!string.IsEmpty())
        m_geometryHash.Add (string.AsBufferPtr(), string.GetBufferSize());
    if (nullptr != font)
        {
        Utf8StringCR    name = font->GetName ();
        if (!name.empty())
            m_geometryHash.Add (name.data(), name.size() * sizeof(Utf8Char));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void    HashMeshData (size_t nPoints, DPoint3dCP points, DwgGiVertexDataCP verts, size_t nEdges, DwgGiEdgeDataCP edges, size_t nFaces, DwgGiFaceDataCP faces, size_t nFaceList, int32_t const* faceLoops, bool autonorm)
    {
    uint8_t const*      visibilities = nullptr;
    DwgCmEntityColorCP  colors = nullptr;
    DwgTransparencyCP   transparencies = nullptr;
    DwgDbObjectIdCP     layers = nullptr;
    DwgDbObjectIdCP     ltypes = nullptr;
    DwgDbObjectIdCP     materials = nullptr;
    DVec3dCP            normals = nullptr;
    DPoint3dCP          mapCoords = nullptr;

    m_geometryHash.Reset ();

    if (nPoints > 0)
        {
        // hash vertices
        if (nullptr != points)
            m_geometryHash.Add (points, nPoints * sizeof(DPoint3d));

        // hash vertex data
        if (nullptr != verts && nullptr != (colors = verts->GetTrueColors()))
            {
            for (size_t v = 0; v < nPoints; v++)
                {
                uint32_t    rgb = colors[v].GetRGB ();
                m_geometryHash.Add (&rgb, sizeof(uint32_t));
                }
            }
        }

    if (nullptr != edges && nEdges > 0)
        {
        // hash edge data
        if (nullptr != (visibilities = edges->GetVisibility()))
            m_geometryHash.Add (visibilities, nEdges * sizeof(uint8_t));

        if (nullptr != (colors = edges->GetTrueColors()))
            {
            for (size_t e = 0; e < nEdges; e++)
                {
                uint32_t    rgb = colors[e].GetRGB ();
                m_geometryHash.Add (&rgb, sizeof(uint32_t));
                }
            }
        if (nullptr != (layers = edges->GetLayers()))
            {
            for (size_t e = 0; e < nEdges; e++)
                {
                uint64_t    ehandle = layers[e].ToUInt64 ();
                m_geometryHash.Add (&ehandle, sizeof(uint64_t));
                }
            }
        if (nullptr != (ltypes = edges->GetLinetypes()))
            {
            for (size_t e = 0; e < nEdges; e++)
                {
                uint64_t    ehandle = ltypes[e].ToUInt64 (); 
                m_geometryHash.Add (&ehandle, sizeof(uint64_t));
                }
            }
        }

    if (nFaces > 0 && nFaceList > 0)
        {
        // hash face data
        if (nullptr != faces)
            {
            if (nullptr != (colors = faces->GetTrueColors()))
                {
                for (size_t f = 0; f < nFaces; f++)
                    {
                    uint32_t    rgb = colors[f].GetRGB ();
                    m_geometryHash.Add (&rgb, sizeof(uint32_t));
                    }
                }
            if (nullptr != (transparencies = faces->GetTransparencies()))
                {
                for (size_t f = 0; f < nFaces; f++)
                    {
                    uint32_t    whole = transparencies[f].SerializeOut ();
                    m_geometryHash.Add (&whole, sizeof(uint32_t));
                    }
                }
            if (nullptr != (layers = faces->GetLayers()))
                {
                for (size_t f = 0; f < nFaces; f++)
                    {
                    uint64_t    ehandle = layers[f].ToUInt64 ();
                    m_geometryHash.Add (&ehandle, sizeof(uint64_t));
                    }
                }
            if (nullptr != (materials = faces->GetMaterials()))
                {
                for (size_t f = 0; f < nFaces; f++)
                    {
                    uint64_t    ehandle = layers[f].ToUInt64 ();
                    m_geometryHash.Add (&ehandle, sizeof(uint64_t));
                    }
                }
            if (nullptr != (normals = faces->GetNormals()))
                {
                for (size_t f = 0; f < nFaces; f++)
                    m_geometryHash.Add (&normals[f], sizeof(DVec3d));
                }
            }

        // hash face topology
        if (nullptr != faceLoops)
            m_geometryHash.Add (faceLoops, nFaceList * sizeof(uint32_t));
        }

    m_geometryHash.Add (&autonorm, sizeof autonorm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ApplyThickness (ICurvePrimitivePtr const& curvePrimitive, DVec3dCR normal, bool closed = false)
    {
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
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Circle (DPoint3dCR center, double radius, DVec3dCR normal) override
    {
    DEllipse3d      circle = DEllipse3d::FromCenterNormalRadius (center, normal, radius);
    circle.MakeFullSweep ();

    ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateArc (circle);
    if (primitive.IsValid() && !this->ApplyThickness(primitive, normal, true))
        {
        if (this->IsAssembly())
            {
            m_geometryHash.Reset ();
            m_geometryHash.Add (&center, sizeof center);
            m_geometryHash.Add (&radius, sizeof radius);
            m_geometryHash.Add (&normal, sizeof normal);
            }

        this->AppendGeometry (*primitive.get());
        }
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
        {
        if (this->IsAssembly())
            {
            m_geometryHash.Reset ();
            m_geometryHash.Add (&point1, sizeof point1);
            m_geometryHash.Add (&point2, sizeof point2);
            m_geometryHash.Add (&point3, sizeof point3);
            }

        this->AppendGeometry (*primitive.get());
        }
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
            {
            if (this->IsAssembly())
                {
                m_geometryHash.Reset ();
                m_geometryHash.Add (&center, sizeof center);
                m_geometryHash.Add (&rad, sizeof rad);
                m_geometryHash.Add (&normal, sizeof normal);
                m_geometryHash.Add (&start, sizeof start);
                m_geometryHash.Add (&swept, sizeof swept);
                m_geometryHash.Add (&arcType, sizeof arcType);
                }

            this->AppendGeometry (*primitive.get());
            }
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
            {
            if (this->IsAssembly())
                {
                m_geometryHash.Reset ();
                m_geometryHash.Add (&start, sizeof start);
                m_geometryHash.Add (&point, sizeof point);
                m_geometryHash.Add (&end, sizeof end);
                m_geometryHash.Add (&arcType, sizeof arcType);
                }

            this->AppendGeometry (*primitive.get());
            }
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
        {
        if (this->IsAssembly())
            {
            m_geometryHash.Reset ();
            m_geometryHash.Add (&curve.type, sizeof curve.type);
            m_geometryHash.Add (&curve.rational, sizeof curve.rational);
            m_geometryHash.Add (&curve.display, sizeof curve.display);
            m_geometryHash.Add (&curve.params, sizeof curve.params);
            m_geometryHash.Add (curve.GetPoleCP(), curve.GetNumPoles() * sizeof(DPoint3d));
            m_geometryHash.Add (curve.GetKnotCP(), curve.GetNumKnots() * sizeof(double));
            m_geometryHash.Add (curve.GetWeightCP(), curve.GetNumPoles() * sizeof(double));
            }

        this->AppendGeometry (*primitive.get());
        }
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
            {
            if (this->IsAssembly())
                {
                m_geometryHash.Reset ();
                m_geometryHash.Add (&ellipse.center, sizeof ellipse.center);
                m_geometryHash.Add (&ellipse.vector0, sizeof ellipse.vector0);
                m_geometryHash.Add (&ellipse.vector90, sizeof ellipse.vector90);
                m_geometryHash.Add (&ellipse.start, sizeof ellipse.start);
                m_geometryHash.Add (&ellipse.sweep, sizeof ellipse.sweep);
                }

            this->AppendGeometry (*primitive.get());
            }
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
            {
            if (this->IsAssembly())
                {
                m_geometryHash.Reset ();

                bvector <bvector<bvector<DPoint3d>>> loops;
                if (edges.CollectLinearGeometry(loops))
                    {
                    for (auto& loop : loops)
                        for (auto& loopPoints : loop)
                            m_geometryHash.Add (&loopPoints, sizeof(DPoint3d));
                    }
                }

            this->AppendGeometry (*geom.get());
            }
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

        if (this->IsAssembly())
            {
            m_geometryHash.Reset ();
            m_geometryHash.Add (points, nPoints * sizeof(DPoint3d));
            }

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
        if (this->IsAssembly())
            {
            m_geometryHash.Reset ();
            plineFactory.HashAndAppendTo (m_geometryHash);
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
        if (this->IsAssembly())
            {
            m_geometryHash.Reset ();
            plineFactory.HashAndAppendTo (m_geometryHash);
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

    if (this->IsAssembly())
        {
        size_t  nEdges = (nRows - 1) * nColumns + nRows * (nColumns -1);
        size_t  nFaces = (nRows -1) * (nColumns - 1);
        this->HashMeshData (nTotal, points, verts, nEdges, edges, nFaces, faces, 0, nullptr, autonorm);
        }

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
        meshPoints.push_back (points[i]);

    GeometricPrimitivePtr   pface = GeometricPrimitive::Create (dgnPolyface);
    if (pface.IsValid())
        this->AppendGeometry (*pface.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Shell (size_t nPoints, DPoint3dCP points, size_t nFaceList, int32_t const* faces, DwgGiEdgeDataCP edgeData = nullptr, DwgGiFaceDataCP faceData = nullptr, DwgGiVertexDataCP vertData = nullptr, DwgResBufCP xData = nullptr, bool autonorm = false) override
    {
    if (this->IsAssembly())
        {
        // count up faces and edges - nFaceList is the size of faces.
        size_t  nSolidFaces = 0, nEdges = 0;
        if (nullptr != edgeData && nullptr != faces && nFaceList > 0)
            {
            for (size_t f = 0; f < nFaceList; f += abs(faces[f]) + 1)
                {
                nEdges += abs(faces[f]);
                if (faces[f] > 0)
                    nSolidFaces++;
                }
            }
        this->HashMeshData (nPoints, points, vertData, nEdges, edgeData, nSolidFaces, faceData, nFaceList, faces, autonorm);
        }

    // DwgGiFillType::Always == m_drawParams._GetFillType()
    /*-----------------------------------------------------------------------------------
    A filled hatch, or a filled object created by an object enabler, may be tessellated to
    a huge number of shapes, which can significantly increase the assembly size. Ideally,
    we would want polyface headers for the sake of performance and smaller file size, but 
    at the moment, polyface header is not supported for 2D model!
    -----------------------------------------------------------------------------------*/
    if (!m_createParams.GetModel().Is3d())
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
                importer.ReportIssue (DwgImporter::IssueSeverity::Warning, DwgImporter::IssueCategory::UnexpectedData(), DwgImporter::Issue::Message(), Utf8PrintfString("skipped a shell having face vertex count of %d [expected %d]", nVertices, maxFaceVertices).c_str());
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
        this->AppendGeometry (*pface.get());
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
        importer.ReportIssue (DwgImporter::IssueSeverity::Info, DwgImporter::IssueCategory::UnexpectedData(), DwgImporter::Issue::Message(), "skipped an empty text!");
        return  BSIERROR;
        }

    if (!importer.ArePointsValid(&position, 1, m_drawParams.GetSourceEntity()))
        {
        importer.ReportIssue (DwgImporter::IssueSeverity::Info, DwgImporter::IssueCategory::UnexpectedData(), DwgImporter::Issue::InvalidRange(), "skipped a text out of range!");
        return  BSIERROR;
        }

    Utf8String  textString(string.c_str());
    textString.Trim ();
    if (textString.empty())
        {
        importer.ReportIssue (DwgImporter::IssueSeverity::Info, DwgImporter::IssueCategory::UnexpectedData(), DwgImporter::Issue::Message(), "skipped a text of all white spaces!");
        return  BSIERROR;
        }

    // get ECS - do not compound it with currTrans - handled in central factory.
    RotMatrix   ecs;
    DwgHelper::ComputeMatrixFromXZ (ecs, xdir, normal);

    dgnText->SetText (textString.c_str());
    dgnText->SetOrigin (position);
    dgnText->SetOrientation (ecs);
    
    // caller to set style specific data
    return  BSISUCCESS;
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
        {
        if (this->IsAssembly())
            this->HashTextData (position, normal, xdir, h, w, oblique, string, font);

        this->AppendGeometry (*primitive.get());
        }
    // NEEDSWORK - extrude text grlyphs
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _Text (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, DwgStringCR string, bool raw, DwgGiTextStyleCR giStyle) override
    {
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
        importer.ReportIssue (DwgImporter::IssueSeverity::Info, DwgImporter::IssueCategory::MissingData(), DwgImporter::Issue::Message(), Utf8PrintfString("no font found in text style %s - using default font", fname.c_str()).c_str());

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
        {
        if (this->IsAssembly())
            {
            double  oblique = fontType == DgnFontType::TrueType ? 0.0 : giStyle.GetObliqueAngle();
            this->HashTextData (position, normal, xdir, height, width, oblique, string, font);
            }

        this->AppendGeometry (*primitive.get());
        }

    // NEEDSWORK - extrude text grlyphs

    // draw underlines and overlines decorated by the escape codes:
    for (auto const& underline : underlines)
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateLine (underline);
        if (primitive.IsValid())
            this->AppendGeometry (*primitive.get());
        }
    for (auto const& overline : overlines)
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateLine (overline);
        if (primitive.IsValid())
            this->AppendGeometry (*primitive.get());
        }

    // now draw strike through
    if (giStyle.IsStrikethrough())
        {
        ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateLine (strikeThrough);
        if (primitive.IsValid())
            this->AppendGeometry (*primitive.get());
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
        {
        if (this->IsAssembly())
            {
            m_geometryHash.Reset ();
            m_geometryHash.Add (&origin, sizeof origin);
            m_geometryHash.Add (&point, sizeof point);
            }

        this->AppendGeometry (*ray.get());
        }
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
        bool    wasInBlock = this->IsDrawingBlock ();
        this->SetDrawingBlock (true);

        // set part namespace "block name-fileId" for the new block we are about to draw - only use outermost block:
        if (m_partNamespaceStack.empty())
            {
            Utf8PrintfString    partNamespace("%ls-%d", block->GetName().c_str(), DwgSyncInfo::GetDwgFileId(*block->GetDatabase()));
            this->PushPartNamespace (partNamespace);
            }

        try
            {
            drawable.Draw (*this, *m_geometryOptions, m_drawParams);
            }
        catch (...)
            {
            BeAssert (false && L"Exception thrown drawing a nested block!");
            }

        // done drawing the block - revert drawing controls back to previous state:
        this->SetDrawingBlock (wasInBlock);
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
    DwgDbBlockReferenceCP   blockRef = DwgDbBlockReference::Cast (child.get());
    if (nullptr != blockRef)
        this->DrawBlockAttributes (*blockRef);

    if (!child.IsNull())
        m_drawParams.CopyFrom (savedParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DrawBlockAttributes (DwgDbBlockReferenceCR blockRef)
    {
    DwgDbObjectIterator iter = blockRef.GetAttributeIterator ();
    if (!iter.IsValid())
        return  BSISUCCESS;

    // save off last DrawParameters
    DrawParameters  savedParams = m_drawParams;
    uint32_t        count = 0;

    // draw each and every attribute followed the block reference:
    for (iter.Start(); !iter.Done(); iter.Next())
        {
        DwgDbEntityPtr  attrib(iter.GetObjectId(), DwgDbOpenMode::ForRead);
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
        {
        if (this->IsAssembly())
            {
            m_geometryHash.Reset ();
            m_geometryHash.Add (&dots.front(), dots.size() * sizeof(DPoint3d));
            }

        this->AppendGeometry (*primitive.get());
        }
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

        if (this->IsAssembly())
            {
            m_geometryHash.Reset ();
            m_geometryHash.Add (&points[0], 2 * sizeof(DPoint3d));
            }

        this->AppendGeometry (*line.get());

        m_currentTransform = curr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _PushModelTransform (TransformCR newTransform) override
    {
    Transform   topTransform;

    if (m_transformStack.empty())
        topTransform = newTransform;
    else
        topTransform.InitProduct (m_transformStack.back(), newTransform);

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
virtual void    _PushClipBoundary (DwgGiClipBoundaryCP doundary) override
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
    geometry.TransformInPlace (m_currentTransform);

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

    if (this->IsAssembly())
        this->AppendSharedGeometryPart (geometry, display);
    else
        this->AppendPrimitiveGeometry (geometry, display);

    // spin the progress meter
    m_drawParams.GetDwgImporter().Progress ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AppendPrimitiveGeometry (GeometricPrimitiveR geometry, Render::GeometryParamsCR display)
    {
    BuilderInfoR    builderInfo = this->GetOrAddGeometryBuilder (m_outputGeometryList, geometry, display);
    if (!builderInfo.m_geometryBuilder.IsValid())
        return  BSIERROR;
    
    builderInfo.m_geometryBuilder->Append (display);

    // untransform geometry
    geometry.TransformInPlace (m_worldToElement);

    return  builderInfo.m_geometryBuilder->Append(geometry) ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AppendSharedGeometryPart (GeometricPrimitiveCR geometry, Render::GeometryParamsCR display)
    {
    DwgDbEntityCP   entity = m_drawParams.GetSourceEntity ();
    uint64_t        entityHandle = nullptr == entity ? 0 : entity->GetObjectId().ToUInt64();
    DRange3d        partRange = DRange3d::NullRange ();
    size_t          partIndex = 0;

    BuilderListR    sharedParts = m_drawParams.GetDwgImporter().GetSharedPartListR ();
    BuilderInfoR    builderInfo = this->GetOrAddGeometryBuilder (sharedParts, geometry, display, entityHandle, &partIndex);

    if (builderInfo.m_partId.IsValid())
        {
        // found an existing part - use it:
        if (BSISUCCESS != DgnGeometryPart::QueryGeometryPartRange(partRange, m_createParams.GetModel().GetDgnDb(), builderInfo.m_partId))
            builderInfo.m_partId = DgnGeometryPartId ();

        Transform   geomToWorld;
        if (!geometry.GetLocalCoordinateFrame(geomToWorld))
            geomToWorld.InitIdentity ();

        // transform the part from world to geometry, transform geometry to new world, then from world back to element:
        m_worldToElement = Transform::FromProduct (builderInfo.m_transform, geomToWorld, builderInfo.m_transform);
        }
    else
        {
        // no part is found - create a new one:
        this->CreateGeometryPart (builderInfo, partRange, geometry, entity, sharedParts.size());
        if (!builderInfo.m_geometryBuilder.IsValid())
            return  BSIERROR;
        }

    if (!builderInfo.m_partId.IsValid())
        {
        BeAssert (false && "invalid geometry part!");
        return  BSIERROR;
        }

    // we should have a valid geometry part - append geometry params to part:
    builderInfo.m_geometryBuilder->Append (display);

    // append part to the geometry builder:
    BentleyStatus   status = BSIERROR;
    if (builderInfo.m_geometryBuilder->Append(builderInfo.m_partId, m_worldToElement, partRange))
        {
        // collect the geometry builder as the output for the caller:
        if (partIndex >= 0 && partIndex < sharedParts.size())
            m_outputPartIndexList.push_back (partIndex);
        else
            BeAssert (false && "Unexpected shared part index!");
        status = BSISUCCESS;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CreateGeometryPart (BuilderInfoR builderInfo, DRange3dR partRange, GeometricPrimitiveCR geometry, DwgDbEntityCP entity, size_t partIndex)
    {
    // create a new part
    DwgImporter&    importer = m_drawParams.GetDwgImporter ();
    DgnDbR          db = m_createParams.GetModel().GetDgnDb ();
    DwgDbDatabaseP  dwg = m_drawParams.GetDatabase ();
    if (nullptr == dwg)
        dwg = &importer.GetDwgDb ();
    
    Utf8StringCR    nameSpace = this->GetPartNamespace ();
    Utf8StringCR    codeValue = builderInfo.GetPartCodeValue (entity, partIndex);
    DgnCode         partCode = importer.CreateCode (codeValue, nameSpace);

    // create a new geometry builder:
    GeometryBuilderPtr  geomBuilder = GeometryBuilder::CreateGeometryPart (db, m_createParams.GetModel().Is3d());
    if (!geomBuilder.IsValid())
        return  BSIERROR;

    // create a new geometry part:
    DgnGeometryPartPtr  geomPart = DgnGeometryPart::Create (db, partCode);
    if (!geomPart.IsValid())
        return  BSIERROR;

    // set the geometry into the builder
    geomBuilder->Append (geometry);

    // set the label to source entity name, should the primitive represent an entity:
    if (nullptr != entity)
        {
        Utf8PrintfString    label("%ls", entity->GetDxfName().c_str());
        geomPart->SetUserLabel (label.c_str());
        }

    // insert the part to DgnDb, and append its ID to our output builder:
    if (BSISUCCESS == geomBuilder->Finish(*geomPart) && db.Elements().Insert<DgnGeometryPart>(*geomPart).IsValid())
        {
        partRange = geomPart->GetBoundingBox ();

        builderInfo.m_partId = geomPart->GetId ();
        builderInfo.m_transform = m_worldToElement;
        builderInfo.m_partNamespace = nameSpace;
        builderInfo.m_geometryHashVal = m_geometryHash.GetHashVal ();

        return  BSISUCCESS;
        }

    m_status = BSIERROR;
    return  BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BuilderInfoR    GetOrAddGeometryBuilder (BuilderListR builderList, GeometricPrimitiveCR geometry, Render::GeometryParamsCR display, uint64_t entityHandle = 0, size_t* indexOut = nullptr)
    {
    // search the input list and find geometry builder that has same display params:
    struct FindGeometryPredicate
        {
        bool    m_isAssembly;
        DwgImporter::GeometryBuilderInfo const& m_newBuilder;

        FindGeometryPredicate (DwgImporter::GeometryBuilderInfo const& in, bool forPart) : m_newBuilder(in), m_isAssembly(forPart) {}

        // compare all display data for a part geometry but only compare category for an individual geometry:
        bool IsSameBuilder(DwgImporter::GeometryBuilderInfo const& oldBuilder) { return m_isAssembly ? m_newBuilder.IsSameDisplay(oldBuilder) : m_newBuilder.m_categoryId == oldBuilder.m_categoryId; }
        bool operator () (DwgImporter::GeometryBuilderInfo const& oldBuilder) { return this->IsSameBuilder(oldBuilder); }
        };

    // init a new builder from input info:
    BuilderInfo builderInfo (display);
    if (this->IsAssembly())
        {
        // set required params for unique parts:
        builderInfo.m_entityHandle = entityHandle;
        builderInfo.m_scales.Init (m_currentTransform.ColumnX().Normalize(), m_currentTransform.ColumnY().Normalize(), m_currentTransform.ColumnZ().Normalize());
        builderInfo.m_partNamespace = this->GetPartNamespace ();
        builderInfo.m_geometryHashVal = m_geometryHash.GetHashVal ();
        }

    FindGeometryPredicate   predicate (builderInfo, this->IsAssembly());

    auto   entry = std::find_if (builderList.begin(), builderList.end(), predicate);

    if (builderList.end() == entry)
        {
        // add a new geometry builder
        this->CreateGeometryBuilder (builderInfo, geometry);

        if (!builderInfo.m_geometryBuilder.IsValid())
            return  m_invalidBuilder;

        if (nullptr != indexOut)
            *indexOut = entry - builderList.begin();

        builderList.push_back (builderInfo);
        return  builderList.back ();
        }
    else
        {
        // use existing geometry builder
        if (nullptr != indexOut)
            *indexOut = entry - builderList.begin();
        return *entry;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            CreateGeometryBuilder (BuilderInfoR builderInfo, GeometricPrimitiveCR geometry)
    {
    Transform   geomToWorld;
    if (!geometry.GetLocalCoordinateFrame(geomToWorld))
        geomToWorld.InitIdentity ();

    DPoint3d    placementPoint;
    geomToWorld.GetTranslation (placementPoint);

    RotMatrix   matrix;
    geomToWorld.GetMatrix (matrix);

    YawPitchRollAngles  angles;
    YawPitchRollAngles::TryFromRotMatrix (angles, matrix);

    DgnModelR   toModel = m_createParams.GetModelR ();
    if (!toModel.Is3d())
        {
        // Element2d only uses Yaw:
        if (0.0 != angles.GetPitch().Degrees() || 0.0 != angles.GetRoll().Degrees())
            {
            angles = YawPitchRollAngles (AngleInDegrees::FromDegrees(0.0), angles.GetPitch(), angles.GetRoll());
            geomToWorld = Transform::FromProduct (geomToWorld, angles.ToTransform(DPoint3d::FromZero()));
            }
        // and has no z-translation:
        placementPoint.z = 0.0;
        geomToWorld.SetTranslation (placementPoint);
        }

    builderInfo.m_geometryBuilder = GeometryBuilder::Create (toModel, builderInfo.m_categoryId, geomToWorld);
    if (!builderInfo.m_geometryBuilder.IsValid())
        {
        BeAssert (false);
        m_status = BSIERROR;
        }

    // will need to untranform geoemtry
    m_worldToElement.InverseOf (geomToWorld);
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
        // we are in a modelspace, in the masterfile or an xref, get a spatial category and sub-category from the syncInfo:
        categoryId = importer.FindCategoryFromSyncInfo (layerId, xrefDwg);
        subcategoryId = importer.FindSubCategoryFromSyncInfo (layerId, xrefDwg);

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
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgImporter::GeometryBuilderInfo::GeometryBuilderInfo (Render::GeometryParamsCR geomParams, uint64_t entityHandle, GeometryBuilderP builder)
    {
    m_geometryBuilder = builder;
    m_categoryId = geomParams.GetCategoryId ();
    m_subCategoryId = geomParams.GetSubCategoryId ();
    m_materialId = geomParams.IsMaterialFromSubCategoryAppearance() ? DgnMaterialId() : geomParams.GetMaterialId();
    m_weight = geomParams.IsWeightFromSubCategoryAppearance() ? 0 : geomParams.GetWeight();
    m_lineColor = geomParams.IsLineColorFromSubCategoryAppearance() ? ColorDef() : geomParams.GetLineColor();
    m_fillColor = geomParams.IsFillColorFromSubCategoryAppearance() ? ColorDef() : geomParams.GetFillColor();
    m_fillDisplay = geomParams.GetFillDisplay ();
    m_transparency = geomParams.GetTransparency ();
    m_fillTransparency = geomParams.GetFillTransparency ();

    if (geomParams.IsLineStyleFromSubCategoryAppearance())
        {
        m_linestyleId = DgnStyleId ();
        m_linestyleScale = 1.0;
        }
    else
        {
        m_linestyleId = geomParams.GetLineStyle()->GetStyleId ();
        m_linestyleScale = geomParams.GetLineStyle()->GetLineStyleSymb().GetScale ();
        }

    m_gradient = geomParams.GetGradient ();
    m_entityHandle = entityHandle;
    m_scales.One ();
    m_partCodeValue.clear ();
    m_partNamespace.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::GeometryBuilderInfo::IsSameDisplay (DwgImporter::GeometryBuilderInfo const& other) const
    {
    // compare display params:
    if (m_categoryId != other.m_categoryId ||
        m_subCategoryId != other.m_subCategoryId ||
        m_materialId != other.m_materialId ||
        m_weight != other.m_weight ||
        m_lineColor != other.m_lineColor ||
        m_fillColor != other.m_fillColor ||
        m_fillDisplay != other.m_fillDisplay ||
        m_transparency != other.m_transparency ||
        m_fillTransparency != other.m_fillTransparency ||
        m_linestyleId != other.m_linestyleId || 
        fabs(m_linestyleScale - other.m_linestyleScale) > 1.0e-3)
        return  false;

    // compare part params:
    if (m_entityHandle != other.m_entityHandle || 
        !m_scales.IsEqual(other.m_scales, 1.0e-3) || 
        m_partNamespace.CompareToI(other.m_partNamespace) != 0 ||
        memcmp(m_geometryHashVal.m_buffer, other.m_geometryHashVal.m_buffer, sizeof m_geometryHashVal.m_buffer) != 0)
        return  false;

    if (m_gradient.IsValid() && other.m_gradient.IsValid())
        return  *m_gradient.get() == *other.m_gradient.get();

    return  m_gradient.IsValid() == other.m_gradient.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::GeometryBuilderInfo::BuildPartCodeValue (Utf8StringR out, DwgDbEntityCP entity, size_t partIndex)
    {
    DwgDbObjectId   objectId;
    if (nullptr != entity)
        objectId = entity->GetObjectId ();

    // set code value to be parent entity handle + part index
    if (objectId.IsValid())
        out.Sprintf ("%llx-%lld", objectId.ToUInt64(), partIndex);
    else
        out.Sprintf ("#%ld", partIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR    DwgImporter::GeometryBuilderInfo::GetPartCodeValue (DwgDbEntityCP entity, size_t partIndex)
    {
    if (m_partCodeValue.empty())
        DwgImporter::GeometryBuilderInfo::BuildPartCodeValue (m_partCodeValue, entity, partIndex);

    return  m_partCodeValue;
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

        // create shared parts for the block
        factory.SetIsAssembly (true);

        // set outermost part namespace as "blockname-fileId"
        DwgDbBlockTableRecordPtr    block(insert->GetBlockTableRecordId(), DwgDbOpenMode::ForRead);
        if (block.OpenStatus() == DwgDbStatus::Success)
            {
            Utf8PrintfString    partNamespace("%ls-%d", block->GetName().c_str(), DwgSyncInfo::GetDwgFileId(*block->GetDatabase()));
            factory.PushPartNamespace (partNamespace);
            }
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
        DwgDbObjectIterator attrIter = blockRef->GetAttributeIterator ();
        if (attrIter.IsValid())
            {
            attrIter.Start ();
            if (!attrIter.Done())
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
    GeometryFactory     factory(createParams, drawParams, geomOptions, entity);

    // prepare for import
    PrepareEntityForImport (drawParams, factory, entity);

    // draw entity and create geometry from it in our factory:
    try
        {
        entity->Draw (factory, geomOptions, drawParams);
        }
    catch (...)
        {
        this->ReportError (IssueCategory::Unknown(), Issue::Exception(), IssueReporter::FmtEntity(*entity).c_str());
        }

    T_PartIndexList const& partIndices = factory.GetPartIndexList ();
    T_GeometryBuilderList const& geometryBuilders = partIndices.empty() ? factory.GetGeometryBuilderList() : this->GetSharedPartList();
    if (geometryBuilders.empty() && this->_SkipEmptyElement(entity))
        {
        this->ReportIssue (IssueSeverity::Warning, IssueCategory::MissingData(), Issue::EmptyGeometry(), Utf8PrintfString("%ls, ID=%llx", entity->GetClassName().c_str(), entityId).c_str());
        return  BSIERROR;
        }

    // create a new or update existing DgnDb element from the geometry collected:
    status = this->_CreateOrUpdateElement (results, inputs.GetTargetModelR(), geometryBuilders, partIndices, createParams, *entity, inputs.GetClassId());

    if (BSISUCCESS == status)
        m_entitiesImported++;

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::ImportNewEntity (ElementImportResults& results, ElementImportInputs& inputs, DwgDbObjectIdCR desiredOwnerId, Utf8StringCR desiredCode)
    {
    DwgDbEntityP    entity = inputs.GetEntityP ();
    if (nullptr == entity)
        return  BSIERROR;

    DwgGiDrawablePtr    drawable = entity->GetDrawable ();
    if (!drawable.IsValid())
        return  BSIERROR;

    // set DgnDbElement creation options
    ElementCreateParams createParams(inputs.GetTargetModelR());

    BentleyStatus   status = this->_GetElementCreateParams (createParams, inputs.GetTransform(), *entity, desiredCode.c_str());
    if (BSISUCCESS != status)
        return  status;

    // set DWG entity draw options:
    GeometryOptions     geomOptions = this->_GetCurrentGeometryOptions ();
    DrawParameters      drawParams (*entity, *this);
    GeometryFactory     factory(createParams, drawParams, geomOptions, entity);

    // prepare for import
    if (PrepareEntityForImport(drawParams, factory, entity))
        {
        // entity is changed, make sure drawble still valid
        drawable = entity->GetDrawable ();
        if (!drawable.IsValid())
            return  BSIERROR;
        }

    // draw drawble and create geometry in our factory:
    try
        {
        drawable->Draw (factory, geomOptions, drawParams);
        }
    catch (...)
        {
        this->ReportError (IssueCategory::Unknown(), Issue::Exception(), IssueReporter::FmtEntity(*entity).c_str());
        }

    T_PartIndexList const& partIndices = factory.GetPartIndexList ();
    T_GeometryBuilderList const& geometryBuilders = partIndices.empty() ? factory.GetGeometryBuilderList() : this->GetSharedPartList();
    if (geometryBuilders.empty())
        {
        this->ReportIssue (IssueSeverity::Warning, IssueCategory::MissingData(), Issue::EmptyGeometry(), Utf8PrintfString("%ls", entity->GetClassName().c_str()).c_str());
        return  BSIERROR;
        }

    // create a new or update existing DgnDb element from the geometry collected:
    status = this->_CreateOrUpdateElement (results, inputs.GetTargetModelR(), geometryBuilders, partIndices, createParams, *entity, inputs.GetClassId());

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
        label.Assign (entity.GetClassName().c_str());
    
    // for a block reference, use block name:
    DwgDbBlockReferenceCP   insert = DwgDbBlockReference::Cast(&entity);
    if (nullptr != insert)
        {
        DwgDbBlockTableRecordPtr block (insert->GetBlockTableRecordId(), DwgDbOpenMode::ForRead);
        if (!block.IsNull())
            {
            DwgString   blockName = block->GetName ();
            if (!blockName.IsEmpty())
                {
                Utf8PrintfString    insertName("%s - %ls", label.c_str(), blockName.c_str());
                label.assign (insertName.c_str());
                }
            }
        }
    return  label;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_CreateOrUpdateElement (ElementImportResults& results, DgnModelR targetModel, T_GeometryBuilderList const& geometryBuilders, T_PartIndexList const& partIndices, ElementCreateParams const& createParams, DwgDbEntityCR entity, DgnClassId dgnClass)
    {
    uint64_t    entityId = entity.GetObjectId().ToUInt64 ();

    // find the element handler
    ElementHandlerP handler = dgn_ElementHandler::Element::FindHandler (targetModel.GetDgnDb(), dgnClass);
    if (nullptr == handler)
        {
        BeAssert (false && L"Null element handler!");
        return  BSIERROR;
        }

    // DgnCode & label for the entity
    DgnCode     elementCode = createParams.GetElementCode ();
    Utf8String  label = this->_GetElementLabel (entity);

    DgnElement::CreateParams    elementParams(targetModel.GetDgnDb(), targetModel.GetModelId(), dgnClass, elementCode, label.c_str(), DgnElementId());

    // if no geometry collected, return an empty element as a host element for ElementAspects etc
    size_t  numBuilders = geometryBuilders.size ();
    if (numBuilders == 0)
        {
        results.m_importedElement = handler->Create (elementParams);

        auto geomSource = results.m_importedElement->ToGeometrySourceP ();
        if (nullptr == geomSource)
            {
            BeAssert (false && L"Null geometry source!");
            return  BSIERROR;
            }
        geomSource->SetCategoryId (createParams.m_categoryId);
        return  BSISUCCESS;
        }

    BentleyStatus   status = BSISUCCESS;

    // create new elements from the geometry builders and add them to output results:
    if (partIndices.empty())
        {
        // individual geometries
        size_t  failed = 0;
        for (auto entry : geometryBuilders)
            {
            status = this->CreateElement (results, entry, elementParams, elementCode, handler);
            if (status != BSISUCCESS)
                failed++;
            }

        if (failed > 0)
            {
            this->ReportError (IssueCategory::VisualFidelity(), Issue::Error(), Utf8PrintfString("%lld out of %lld individual geometry element(s) is/are not created for entity[%s, handle=%llx]!", failed, geometryBuilders.size(), label.c_str(), entityId).c_str());
            if (BSISUCCESS == status)
                status = BSIERROR;
            }
        }
    else
        {
        // shared part geometries
        size_t  failed = 0;
        for (auto index : partIndices)
            {
            if (index >= numBuilders)
                {
                BeAssert (false && "unexpected part array index!");
                status = BSIERROR;
                break;
                }

            auto entry = geometryBuilders[index];

            status = this->CreateElement (results, entry, elementParams, elementCode, handler);
            if (status != BSISUCCESS)
                failed++;
            }

        if (failed > 0)
            {
            this->ReportError (IssueCategory::VisualFidelity(), Issue::Error(), Utf8PrintfString("%lld out of %lld shared part geometry element(s) is/are not created for entity[%s, handle=%llx]!", failed, partIndices.size(), label.c_str(), entityId).c_str());
            if (BSISUCCESS == status)
                status = BSIERROR;
            }
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::CreateElement (ElementImportResults& results, GeometryBuilderInfo& builderInfo, DgnElement::CreateParams& elementParams, DgnCodeCR parentCode, ElementHandlerP handler)
    {
    auto    builder = builderInfo.m_geometryBuilder;
    if (!builder.IsValid() || handler == nullptr)
        {
        BeDataAssert (false && L"Invalid GeometricBuilder and/or ElementHandler!");
        return  BSIERROR;
        }

    if (results.m_importedElement.IsValid())
        {
        // a parent element has been created - set element params for children
        Utf8PrintfString    codeValue("%s-%d", parentCode.GetValue(), results.m_childElements.size() + 1);
        DgnCode             childCode = this->CreateCode (codeValue, parentCode.GetScope());
        elementParams.SetCode (childCode);
        }

    // create a new element from current geometry builder:
    DgnElementPtr   element = handler->Create (elementParams);
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

    // set category and save geometry source to the new element
    if (DgnDbStatus::Success != geomSource->SetCategoryId(builderInfo.m_categoryId) || BSISUCCESS != builder->Finish(*geomSource))
        return  BSIERROR;

    if (!results.m_importedElement.IsValid())
        {
        // the first geometry builder is the parent element:
        results.m_importedElement = element;
        }
    else
        {
        // rest of the geomtry builder collection are children:
        ElementImportResults    childResults(element.get());
        results.m_childElements.push_back (childResults);
        }

    return  BSISUCCESS;
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

    if (model.Is3d())
        {
        // get spatial category & subcategory from the syncInfo:
        params.m_categoryId = this->FindCategoryFromSyncInfo (layerId, xrefDwg);
        params.m_subCategoryId = this->FindSubCategoryFromSyncInfo (layerId, xrefDwg);
        }
    else
        {
        // get or create a drawing category and/or a subcategory for the layer & the viewport:
        DwgDbObjectId   viewportId = this->GetCurrentViewportId ();
        params.m_categoryId = this->GetOrAddDrawingCategory (params.m_subCategoryId, layerId, viewportId, model, xrefDwg);
        }

    if (!params.m_categoryId.IsValid())
        {
        this->ReportError(IssueCategory::CorruptData(), Issue::ImportFailure(), Utf8PrintfString("[%s] - Invalid layer %llx", IssueReporter::FmtEntity(ent).c_str(), ent.GetLayerId().ToUInt64()).c_str());
        return BSIERROR;
        }

    // WIP - apply entity ECS?
    // Transform   ecs;
    // ent.GetEcs (ecs);
    // params.m_transform.InitProduct (ecs, toDgn);
    params.m_transform = toDgn;

    params.m_placementPoint = DwgHelper::DefaultPlacementPoint (ent);

    Utf8String      codeNamespace = model.GetName ();
    Utf8String      codeValue;
    if (nullptr != desiredCode)
        codeValue.assign (desiredCode);
    else
        codeValue.Sprintf ("%llx", ent.GetObjectId().ToUInt64());
    params.m_elementCode = this->CreateCode (codeValue, codeNamespace); // *** ? TBD: m_params.GetNamePrefix().append(codeNamespace));

    return  BSISUCCESS;
    }

#ifdef CHECK_XDATA
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void     CheckXData (DwgDbEntityCR entity)
    {
    DwgResBufIterator   xdata = entity.GetXData (DwgString());
    if (xdata.IsValid())
        {
        LOG_ENTITY.tracev("Checking XDATA for %ls:", entity.GetDxfName().c_str());

        for (DwgResBufP curr = xdata->Start(); curr != xdata->End(); curr = curr->Next())
            {
            switch (curr->GetDataType())
                {
                case DwgResBuf::DataType::Integer8:
                    LOG_ENTITY.tracev ("XDATA Int8= %d", curr->GetInteger8());
                    break;
                case DwgResBuf::DataType::Integer16:
                    LOG_ENTITY.tracev ("XDATA Int16= %d", curr->GetInteger16());
                    break;
                case DwgResBuf::DataType::Integer32:
                    LOG_ENTITY.tracev ("XDATA Int32= %d", curr->GetInteger32());
                    break;
                case DwgResBuf::DataType::Integer64:
                    LOG_ENTITY.tracev ("XDATA Int64= %I64d", curr->GetInteger64());
                    break;
                case DwgResBuf::DataType::Double:
                    LOG_ENTITY.tracev ("XDATA Double= %g", curr->GetDouble());
                    break;
                case DwgResBuf::DataType::Text:
                    LOG_ENTITY.tracev ("XDATA String= %ls", curr->GetString().c_str());
                    break;
                case DwgResBuf::DataType::BinaryChunk:
                    {
                    DwgBinaryData   data;
                    if (DwgDbStatus::Success == curr->GetBinaryData(data))
                        LOG_ENTITY.tracev ("XDATA Binary data size = %ld", data.GetSize());
                    else
                        BeDataAssert(false && "failed extracting binary xdata!");
                    break;
                    }
                case DwgResBuf::DataType::Handle:
                    LOG_ENTITY.tracev ("XDATA Handle= %ls", curr->GetHandle().AsAscii().c_str());
                    break;
                case DwgResBuf::DataType::HardOwnershipId:
                case DwgResBuf::DataType::SoftOwnershipId:
                case DwgResBuf::DataType::HardPointerId:
                case DwgResBuf::DataType::SoftPointerId:
                    LOG_ENTITY.tracev ("XDATA ObjectId= %ls", curr->GetObjectId().ToAscii().c_str());
                    break;
                case DwgResBuf::DataType::Point3d:
                    {
                    DPoint3d    p;
                    if (DwgDbStatus::Success == curr->GetPoint3d(p))
                        LOG_ENTITY.tracev ("XDATA Point3d= %g, %g, %g", p.x, p.y, p.z);
                    else
                        BeDataAssert(false && "failed extracting Point3d xdata!");
                    break;
                    }
                case DwgResBuf::DataType::None:
                case DwgResBuf::DataType::NotRecognized:
                default:
                    LOG_ENTITY.tracev ("XDATA Unexpected type!!!");
                }
            }
        }
    }
#endif  // CHECK_XDATA

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImporter::_FilterEntity (DwgDbEntityCR entity, DwgDbSpatialFilterP spatialFilter)
    {
    // don't draw invisible entities:
    if (DwgDbVisibility::Invisible == entity.GetVisibility())
        return  true;

    // trivial reject clipped away entity:
    if (nullptr != spatialFilter && spatialFilter->IsEntityFilteredOut(entity))
        return  true;

#ifdef CHECK_XDATA
    CheckXData (entity);
#endif

    return  false;
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

    DwgProtocalExtension*   objExt = DwgProtocalExtension::Cast (entity->QueryX(DwgProtocalExtension::Desc()));
    if (nullptr != objExt)
        {
        ProtocalExtensionContext context(inputs, results);
        DwgUpdater* updater = nullptr;

        if (this->_IsUpdating() && nullptr != (updater = dynamic_cast<DwgUpdater*>(this)))
            return objExt->_ConvertToBim (context, *updater);
        else
            return objExt->_ToBim (context, *this);
        }

    DwgDbBlockReferenceP    insert = DwgDbBlockReference::Cast (entity);
    if (nullptr != insert && insert->IsXAttachment())
        return  this->_ImportXReference (*insert, inputs);
    else if (nullptr != insert)
        return  this->_ImportBlockReference (results, inputs);

    return  this->_ImportEntity (results, inputs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DwgImporter::OpenAndImportEntity (ElementImportInputs& inputs)
    {
    inputs.m_entity.OpenObject (inputs.m_entityId, DwgDbOpenMode::ForRead);

    if (inputs.m_entity.OpenStatus() != DwgDbStatus::Success)
        {
        Utf8PrintfString  msg("Entity: %ls, ID= %llx", inputs.m_entityId.GetClassName().c_str(), inputs.m_entityId.ToUInt64());
        this->ReportError (IssueCategory::UnexpectedData(), Issue::CantOpenObject(), msg.c_str());
        return;
        }

    this->Progress ();

    if (!this->_FilterEntity(inputs.GetEntity(), inputs.GetSpatialFilter()))
        {
        ElementImportResults    results;
        if (BSISUCCESS == this->ImportEntity(results, inputs))
            {
            this->_InsertResults (results, DgnElementId());
            this->InsertResultsInSyncInfo (results, inputs.GetEntity(), inputs.GetModelMapping().GetModelSyncInfoId());
            }
        }
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

    DwgDbBlockChildIterator     entityIter = modelspace->GetBlockChildIterator ();
    if (!entityIter.IsValid())
        return  BSIERROR;

    ResolvedModelMapping    modelMap = this->FindModel (m_modelspaceId, m_rootTransform, DwgSyncInfo::ModelSourceType::ModelOrPaperSpace);
    if (!modelMap.IsValid())
        {
        this->ReportError (IssueCategory::UnexpectedData(), Issue::Error(), "cannot find the default target model!");
        return  BSIERROR;
        }

    if (this->_ShouldSkipModel(modelMap))
        return  BSISUCCESS;

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
    inputs.SetTransform (m_rootTransform);
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
        for (entityIter.Start(); !entityIter.Done(); entityIter.Step())
            {
            inputs.SetEntityId (entityIter.GetEntityId());
            this->OpenAndImportEntity (inputs);
            }
        }

    // restore original PDSIZE
    if (resetPDSIZE)
        m_dwgdb->SetPDSIZE (currentPDSIZE);
        
    // free memory
    m_dgndb->Memory().PurgeUntil(1024 * 1024);
    
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
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportBlockReference (ElementImportResults& results, ElementImportInputs& inputs)
    {
    // first import the insert entity
    BentleyStatus   status = this->_ImportEntity (results, inputs);

    // get imported DgnElement
    DgnElementP     hostElement = nullptr;
    if (BSISUCCESS == status)
        hostElement = results.GetImportedElement ();

    if (nullptr == hostElement)
        return  BSIERROR;

    // then import attributes, if any.  Do not bail out even if above call failed - always check for attributes
    DwgDbBlockReferenceCP   insert = DwgDbBlockReference::Cast(inputs.GetEntityP());
    if (nullptr == insert)
        {
        BeAssert (false && "Not a DwgDbBlockReference!");
        return  BSIERROR;
        }

    // create elements from attributes attached to the block reference:
    AttributeFactory        attribFactory(*this, *hostElement, results, inputs);
    attribFactory.CreateElements (*insert);

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_ImportXReference (DwgDbBlockReferenceCR xrefInsert, ElementImportInputs& inputs)
    {
    /*-----------------------------------------------------------------------------------
    This method imports an xref insert entity during entity importing phase.  Do not try
    to skip model during updating.

    All xref files should have been loaded during model discovering phase in the block section.
    If an xref file is not loaded, it is a serious error and we should not bother to try
    loading the file.

    However, xref instances may not all be seen in the block section, therefore there is 
    a chance that this xref insert could be new and a model will need to be created.
    -----------------------------------------------------------------------------------*/
    DwgDbObjectId               xrefblockId = xrefInsert.GetBlockTableRecordId ();
    DwgDbBlockTableRecordPtr    xrefBlock(xrefblockId, DwgDbOpenMode::ForRead);
    if (xrefBlock.IsNull())
        {
        this->ReportError (IssueCategory::CorruptData(), Issue::CantOpenObject(), "xref block %ls");
        return  BSIERROR;
        }

    // skip overlaid xRef if it is nested in another xRef:
    if (xrefBlock->IsOverlayReference() && m_currentXref.IsValid() && m_currentXref.GetDatabase() != m_dwgdb.get())
        return  BSISUCCESS;

    // save currentXref before recurse into a nested xref:
    DwgXRefHolder   savedCurrentXref = m_currentXref;

    auto found = std::find_if (m_loadedXrefFiles.begin(), m_loadedXrefFiles.end(), [&](DwgXRefHolder const& xh){ return xrefblockId==xh.GetBlockIdInParentFile(); });
    if (found != m_loadedXrefFiles.end())
        {
        // this xref has been previously loaded - set it as current:
        m_currentXref = *found;
        }
    else
        {
        // the xref file has not been be loaded in block section - skip it!
        m_currentXref = savedCurrentXref;
        return  BSIERROR;
        }

    // get or create a model for the xRefBlock with the blockReference's transformation:
    Transform   xtrans = inputs.GetTransform ();
    this->CompoundModelTransformBy (xtrans, xrefInsert);
    
    DgnModelP               model = nullptr;
    ResolvedModelMapping    modelMap = this->GetOrCreateModelFromBlock (*xrefBlock.get(), xtrans, &xrefInsert, m_currentXref.GetDatabase());
    if (!modelMap.IsValid() || nullptr == (model = modelMap.GetModel()))
        {
        this->ReportError (IssueCategory::UnexpectedData(), Issue::CantCreateModel(), IssueReporter::FmtModel(*xrefBlock).c_str());
        m_currentXref = savedCurrentXref;
        return  BSIERROR;
        }

    // and or update the model in current as well as the loaded xref cache:
    m_currentXref.AddDgnModelId (model->GetModelId());
    if (found != m_loadedXrefFiles.end())
        found->AddDgnModelId (model->GetModelId());

    this->SetTaskName (ProgressMessage::TASK_IMPORTING_MODEL(), model->GetName().c_str());
    this->Progress ();

    // get the modelspace block from the xRef DwgDb
    DwgDbBlockTableRecordPtr    xModelspace (m_currentXref.GetDatabase()->GetModelspaceId(), DwgDbOpenMode::ForRead);
    if (xModelspace.IsNull())
        {
        this->ReportError (IssueCategory::CorruptData(), Issue::CantOpenObject(), Utf8PrintfString("modelspace of the xref %s", m_currentXref.GetPath().c_str()).c_str());
        m_currentXref = savedCurrentXref;
        return  BSIERROR;
        }

    // clipped xReference
    DwgDbSpatialFilterPtr   spatialFilter;
    if (DwgDbStatus::Success == xrefInsert.OpenSpatialFilter(spatialFilter, DwgDbOpenMode::ForRead))
        LOG_ENTITY.tracev ("xRef %ls is clipped!", xrefBlock->GetPath().c_str());

    ElementImportInputs     childInputs (*model);
    childInputs.SetClassId (this->_GetElementType(*xrefBlock.get()));
    childInputs.SetTransform (xtrans);
    childInputs.SetSpatialFilter (spatialFilter.get());
    childInputs.SetModelMapping (modelMap);

    // SortEnts table
    DwgDbSortentsTablePtr   sortentsTable;
    if (DwgDbStatus::Success == xModelspace->OpenSortentsTable(sortentsTable, DwgDbOpenMode::ForRead))
        {
        // import entities in sorted order:
        DwgDbObjectIdArray  entities;
        if (DwgDbStatus::Success == sortentsTable->GetFullDrawOrder(entities))
            {
            for (DwgDbObjectIdCR id : entities)
                {
                childInputs.SetEntityId (id);
                this->OpenAndImportEntity (childInputs);
                }
            }
        }
    else
        {
        // import entities in database order:
        DwgDbBlockChildIterator     entityIter = xModelspace->GetBlockChildIterator ();
        if (entityIter.IsValid())
            {
            // fill the xref model with entities
            for (entityIter.Start(); !entityIter.Done(); entityIter.Step())
                {
                childInputs.SetEntityId (entityIter.GetEntityId());
                this->OpenAndImportEntity (childInputs);
                }
            }
        }

    // restore current xref
    m_currentXref = savedCurrentXref;

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::DwgXRefHolder::InitFrom (DwgDbBlockTableRecordCR xrefBlock, DwgImporter& importer)
    {
    if (xrefBlock.IsExternalReference())
        {
        m_path.assign (xrefBlock.GetPath().c_str());

        // try to use existing xRef DwgDb
        m_xrefDatabase = xrefBlock.GetXrefDatabase ();
        
        if (m_xrefDatabase.IsNull())
            {
            DwgImportHost&  host = DwgImportHost::GetHost ();
            BeFileName      found;

            // try resolving the file path
            if (!m_path.DoesPathExist() && DwgDbStatus::Success == host._FindFile(found, m_path.c_str(), xrefBlock.GetDatabase().get(), AcadFileType::XRefDrawing))
                m_path = found;

            if (m_path.DoesPathExist())
                {
                // if the DWG file has been previously loaded, use it
                m_xrefDatabase = importer.FindLoadedXRef (m_path);

                // try creating a new DwgDb for the xref, but do not allow circular referencing:
                if (!m_xrefDatabase.IsValid() && !m_path.EqualsI(importer.GetRootDwgFileName()))
                    {
                    importer.SetStepName (DwgImporter::ProgressMessage::STEP_OPENINGFILE(), m_path.c_str());
                    m_xrefDatabase = host.ReadFile (m_path, false, false, FileShareMode::DenyNo);
                    }
                }
            }

        if (m_xrefDatabase.IsValid())
            {
            // the nested block name should propagated into root file
            m_prefixInRootFile = xrefBlock.GetName().GetWCharCP ();
            m_blockIdInParentFile = xrefBlock.GetObjectId ();
            m_spaceIdInRootFile = importer.GetCurrentSpaceId ();
            return  BSISUCCESS;
            }
        }

    m_path.clear ();
    m_prefixInRootFile.clear ();
    m_blockIdInParentFile.SetNull ();
    m_spaceIdInRootFile.SetNull ();

    return  BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbDatabaseP  DwgImporter::FindLoadedXRef (BeFileNameCR path)
    {
    struct FindXrefPredicate
        {
        BeFileName  m_filePath;
        FindXrefPredicate (BeFileNameCR inPath) : m_filePath (inPath) {}
        bool operator () (DwgXRefHolder const& xh) { return m_filePath.CompareTo(xh.GetPath()) == 0; }
        };

    FindXrefPredicate   pred(path);
    auto found = std::find_if (m_loadedXrefFiles.begin(), m_loadedXrefFiles.end(), pred);

    return found == m_loadedXrefFiles.end() ? nullptr : found->GetDatabase();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus     DwgImporter::_InsertResults (ElementImportResults& results, DgnElementId parentId)
    {
    if (!results.m_importedElement.IsValid())
        return DgnDbStatus::Success;

    if (parentId.IsValid())
        results.m_importedElement->SetParentId (parentId, m_dgndb->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements));

    // insert the primary element
    DgnDbStatus status = DgnDbStatus::Success;
    DgnCode     code = results.m_importedElement->GetCode ();
    auto        ret = GetDgnDb().Elements().Insert (*results.m_importedElement, &status);

    if (DgnDbStatus::DuplicateCode == status)
        {
        Utf8String  duplicateMessage;
        duplicateMessage.Sprintf ("Duplicate element code '%s' ignored", code.GetValue().c_str());
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
        LOG_ENTITY.tracev ("Inserted %s, %s", DwgImporter::IssueReporter::FmtElement(*ret).c_str(), ret->GetDisplayLabel().c_str());

    // insert the children of the primary elements, if any
    for (ElementImportResults& child : results.m_childElements)
        {
        status = this->_InsertResults (child, results.m_importedElement->GetElementId());
        if (DgnDbStatus::Success != status)
            return status;
        }
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::InsertResultsInSyncInfo (ElementImportResults& results, DwgDbEntityCR entity, DwgSyncInfo::DwgModelSyncInfoId const& modelSyncId)
    {
    DgnElementP element = results.GetImportedElement ();
    if (nullptr == element || !modelSyncId.IsValid())
        return  BSIERROR;

    // a block reference needs a secondary hash from its block definition:
    bool    hash2nd = this->GetOptions().GetSyncBlockChanges() && DwgDbBlockReference::Cast (&entity);

    // create a provenence from the source DWG object:
    DwgSyncInfo::DwgObjectProvenance    entityprov (*DwgDbObject::Cast(&entity), this->GetSyncInfo(), this->GetCurrentIdPolicy(), hash2nd);

    // directly add the new element into DwgSyncInfo:
    BentleyStatus   status = this->GetSyncInfo().InsertElement (element->GetElementId(), entity, entityprov, modelSyncId);

    if (BSISUCCESS != status)
        {
        uint64_t    entityId = entity.GetObjectId().ToUInt64 ();
        this->ReportError (IssueCategory::Sync(), Issue::Error(), Utf8PrintfString("failed inserting element into DwgSynchInfo for entityId=%llx!", entityId).c_str());
        }

    return  status;
    }
