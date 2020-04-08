/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::EffectiveByBlock::Set (DwgCmEntityColorCR color, DwgDbObjectIdCR ltype, DwgDbObjectIdCR material, DwgDbLineWeight weight, DwgDbObjectIdCR layer)
    {
    m_color = color;
    m_linetypeId = ltype;
    m_materialId = material;
    m_weight = weight;
    m_layerId = layer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::EffectiveByBlock::CopyFrom (EffectiveByBlock const& other)
    {
    m_color = other.m_color;
    m_linetypeId = other.m_linetypeId;
    m_materialId = other.m_materialId;
    m_weight = other.m_weight;
    m_layerId = other.m_layerId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::CopyFrom (DrawParameters const& params)
    {
    m_color = params._GetColor ();
    m_layerId = params._GetLayer ();
    m_linetypeId = params._GetLineType ();
    m_materialId = params._GetMaterial ();
    m_transparency = params._GetTransparency ();
    m_weight = params._GetLineWeight ();
    m_filltype = params._GetFillType ();
    m_fill = params._GetFill() == nullptr ? nullptr : params._GetFill()->Clone();
    m_linetypeScale = params._GetLineTypeScale ();
    m_thickness = params._GetThickness ();
    m_mappedDgnWeight = params.m_mappedDgnWeight;
    m_markerId = params.m_markerId;
    m_sourceEntity = params.m_sourceEntity;
    m_effectiveByBlock.CopyFrom (params.m_effectiveByBlock);
    m_dwgdb = params.m_dwgdb;
    m_isLayerByBlock = params.m_isLayerByBlock;
    m_isParentLayerFrozen = params.m_isParentLayerFrozen;
    m_isDisplayed = params.m_isDisplayed;
    m_layer0Id = params.m_layer0Id;
    m_canOverrideEntityMaterial = params.m_canOverrideEntityMaterial;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::Initialize (DwgDbEntityCR ent, DrawParameters const* parentParams, DwgDbEntityCP templateEntity)
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
    m_canOverrideEntityMaterial = m_dwgImporter._AllowEntityMaterialOverrides(ent);
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
    m_displayPriority = m_dwgImporter.GetCurrentDisplayPriority ();

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

    this->ResolveDisplayStatus (parentParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
double DrawParameters::InitThicknessFromEntity (DwgDbEntityCR ent)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DrawParameters::DrawParameters (DrawParameters const& params) : m_dwgImporter(params.m_dwgImporter)
    {
    this->CopyFrom (params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DrawParameters::DrawParameters (DwgDbEntityCR ent, DwgImporter& importer, DwgDbEntityCP parent, DwgDbEntityCP templateEntity) : m_dwgImporter(importer)
    {
    if (nullptr != parent)
        {
        DrawParameters  parentParams(*parent, importer);
        this->Initialize (ent, &parentParams, templateEntity);
        return;
        }
    this->Initialize (ent, nullptr, templateEntity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::_SetColor (DwgCmEntityColorCR color)
    {
    m_color = color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::_SetLayer (DwgDbObjectIdCR layerId)
    {
    m_layerId = layerId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::_SetLineType (DwgDbObjectIdCR linetypeId)
    {
    m_linetypeId = linetypeId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::_SetSelectionMarker (std::ptrdiff_t markerId)
    {
    m_markerId = markerId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::_SetFillType (DwgGiFillType filltype)
    {
    m_filltype = filltype;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::_SetFill (DwgGiFillCP fill)
    {
    m_fill = fill == nullptr ? nullptr : fill->Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::_SetLineWeight (DwgDbLineWeight weight)
    {
    m_weight = weight;
    m_mappedDgnWeight = m_dwgImporter.GetOptions().GetDgnLineWeight(weight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::_SetLineTypeScale (double scale)
    {
    m_linetypeScale = scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::_SetThickness (double thickness)
    {
    m_thickness = thickness;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::_SetTransparency (DwgTransparency transparency)
    {
    m_transparency = transparency;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::_SetMaterial (DwgDbObjectIdCR materialId)
    {
    if (m_canOverrideEntityMaterial)
        m_materialId = materialId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::SetGradientColorFromHatch (DwgDbHatchCP hatch)
    {
    /*-----------------------------------------------------------------------------------
    When we draw a hatch entity with gradient fill, two things happen:

    1) The world/viewportDraw does not call _SetFill for sub-entity, so we have explicitly 
        extract its gradient fill before drawing it.
    2) The world/viewportDraw creates mesh, which is a undesired geometry for a fill element.

    To workaround issue 1 above, we extract gradient fill from the hatch, convert it
    as DGN gradient color, and save the converted gradient in this DrawParameters.
    For issue 2, prior to RealDWG 2019, the caller could re-set the source hatch entity to 
    a solid fill so we could get a simple shape from the hatch.  That workaround no longer 
    works since RealDWG 2019.  We now have an option to convert filled or gradient hatch to
    filled element through the hatch protocol extension.  This is the default option.
    User can turn the option off to draw filled hatch as mesh, in which case this setting 
    is still necessary.
    -----------------------------------------------------------------------------------*/
    if (nullptr != hatch)
        m_fill = DwgGiGradientFill::CreateFrom (hatch);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawParameters::ResolveDisplayStatus (DrawParameters const* parentParams)
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
void DrawParameters::ResolveRootEffectiveByBlockSymbology ()
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
void DrawParameters::ResolveEffectiveByBlockSymbology (DrawParameters const& newParent)
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
bool DrawParameters::IsColorByBlock () const
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
bool DrawParameters::IsLinetypeByBlock () const
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
bool DrawParameters::IsMaterialByBlock () const
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
bool DrawParameters::IsWeightByBlock () const
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
Render::FillDisplay DrawParameters::GetFillDisplay () const
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
void DrawParameters::GetDisplayParams (Render::GeometryParams& params)
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
        DwgGiGradientFillCP dwgGradient = static_cast<DwgGiGradientFillCP> (m_fill.get());
        if (nullptr != dwgGradient && dgnGradient.IsValid())
            {
            // apply gradient fill set by sub-entity's draw method:
            DwgHelper::GetDgnGradientColor (*dgnGradient.get(), *dwgGradient);
            params.SetGradient (dgnGradient.get());
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
    params.SetDisplayPriority (m_displayPriority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DrawParameters::GetDgnColor ()
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
bool DrawParameters::GetColorFromLayer (DwgCmEntityColorR colorOut) const
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
uint32_t DrawParameters::GetDgnWeight ()
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
DwgDbLineWeight DrawParameters::GetWeightFromLayer () const
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
DgnStyleId DrawParameters::GetDgnLineStyle (bool& isContinuous, double& effectiveScale) const
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
bool DrawParameters::GetLinetypeFromLayer (DwgDbObjectIdR ltypeOut) const
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
double DrawParameters::GetEffectiveLinetypeScale () const
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
RenderMaterialId DrawParameters::GetDgnMaterial () const
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
bool DrawParameters::GetMaterialFromLayer (DwgDbObjectIdR materialOut) const
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
DwgDbObjectId DrawParameters::GetEffectiveLayerId () const
    {
    DwgDbObjectId   layerId = m_isLayerByBlock ? m_effectiveByBlock.m_layerId : m_layerId;
    return  layerId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DrawParameters::CanUseByLayer (bool isByLayer)
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



    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryFactory::GeometryFactory (DwgImporter::ElementCreateParams& createParams, DrawParameters& drawParams, DwgImporter::GeometryOptions& opts, DwgDbEntityCP ent) : m_drawParams(drawParams), m_createParams(createParams)
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
    auto dwg = nullptr == ent || ent->GetDatabase().IsNull() ? m_drawParams.GetDatabase() : ent->GetDatabase().get();
    auto blockId = nullptr == ent || !ent->GetOwnerId().IsValid() ? dwg->GetModelspaceId() : ent->GetOwnerId();
    m_blockStack.push_back (BlockInfo(blockId, L"ModelSpace", DwgSourceAspects::GetRepositoryLinkId(*dwg)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryFactory::~GeometryFactory ()
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    auto nBreps = m_parasolidBodies.size ();
    if (nBreps > 0)
        ::PK_ENTITY_delete ((int)nBreps, reinterpret_cast<PK_ENTITY_t*>(&m_parasolidBodies.front()));
#endif
    }

// tracking block drawing - all blocks.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryFactory::PushDrawingBlock (DwgDbBlockTableRecordCR block)
    {
    auto dwg = block.GetDatabase ();
    if (!dwg.IsValid())
        dwg = m_drawParams.GetDatabase ();
    BeAssert (dwg.IsValid());
    BlockInfo entry (block.GetObjectId(), block.GetName(), DwgSourceAspects::GetRepositoryLinkId(*dwg));
    m_blockStack.push_back (entry);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryFactory::ApplyThickness (ICurvePrimitivePtr const& curvePrimitive, DVec3dCR normal, bool closed)
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
bool GeometryFactory::ApplyThickness (GeometricPrimitivePtr const& geomPrimitive, DVec3dCR normal, bool closed)
    {
    double  thickness = m_drawParams._GetThickness ();

    if (ISVALID_Thickness(thickness) && geomPrimitive.IsValid())
        return  this->ApplyThickness (geomPrimitive->GetAsICurvePrimitive(), normal, closed);

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryFactory::ApplyColorOverride (DwgGiFaceDataCP faceData, DwgGiEdgeDataCP edgeData)
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
void GeometryFactory::_Circle (DPoint3dCR center, double radius, DVec3dCR normal)
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
void GeometryFactory::_Circle (DPoint3dCR point1, DPoint3dCR point2, DPoint3dCR point3)
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
void GeometryFactory::_CircularArc (DPoint3dCR center, double rad, DVec3dCR normal, DVec3dCR start, double swept, DwgGiArcType arcType)
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
void GeometryFactory::_CircularArc (DPoint3dCR start, DPoint3dCR point, DPoint3dCR end, DwgGiArcType arcType)
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
void GeometryFactory::_Curve (MSBsplineCurveCR curve)
    {
    ICurvePrimitivePtr  primitive = ICurvePrimitive::CreateBsplineCurve (curve);
    if (primitive.IsValid())
        this->AppendGeometry (*primitive.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryFactory::_Ellipse (DEllipse3dCR ellipse, DwgGiArcType arcType)
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
void GeometryFactory::_Edge (CurveVectorCR edges)
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
void GeometryFactory::_Polyline (size_t nPoints, DPoint3dCP points, DVec3dCP normal, int64_t subentMarker)
    {
    if (0 == nPoints)
        return;

    // CADmep OE 2020 has sent us bogus data - VSTS 217594
    if (isnan(points[0].x) || isnan(points[0].y) || isnan(points[0].z))
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
void GeometryFactory::_Pline (DwgDbPolylineCR pline, size_t fromIndex, size_t numSegs)
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
            curveVector = shape;

        if (plineFactory.HasAppliedWidths())
            {
            m_drawParams._SetFillType (DwgGiFillType::Always);
            m_drawParams.SetLinetypeContinuous ();
            }

        if (m_isTargetModel2d)
            {
            this->AppendGeometry (*curveVector.get());
            return;
            }

        // extrude the polyline if it has a thickness
        GeometricPrimitivePtr   extruded = plineFactory.ApplyThicknessTo (curveVector);
        if (extruded.IsValid())
            {
            m_drawParams.SetLinetypeContinuous ();
            this->AppendGeometry (*extruded.get());
            }
        else
            {
            this->AppendGeometry (*curveVector.get());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryFactory::_Polygon (size_t nPoints, DPoint3dCP points)
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
void GeometryFactory::_Mesh (size_t nRows, size_t nColumns, DPoint3dCP points, DwgGiEdgeDataCP edges, DwgGiFaceDataCP faces, DwgGiVertexDataCP verts, bool autonorm)
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
void GeometryFactory::_Shell (size_t nPoints, DPoint3dCP points, size_t nFaceList, int32_t const* faces, DwgGiEdgeDataCP edgeData, DwgGiFaceDataCP faceData, DwgGiVertexDataCP vertData, DwgResBufCP xData, bool autonorm)
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
BentleyStatus GeometryFactory::SetText (Dgn::TextStringPtr& dgnText, DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, DwgStringCR string, bool* mirrored)
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

    Utf8String  testString(string.c_str());
    testString.Trim ();
    if (testString.empty())
        {
        Utf8PrintfString    msg ("skipped text containing all white spaces, ID=%ls!", m_entity == nullptr ? L"?" : m_entity->GetObjectId().ToAscii().c_str());
        importer.ReportIssue (DwgImporter::IssueSeverity::Info, IssueCategory::UnexpectedData(), Issue::Message(), msg.c_str());
        return  BSIERROR;
        }

    if (mirrored != nullptr)
        *mirrored = false;

    // unmirror text on XY plane
    DVec3d xAxis, zAxis;
    xAxis.Normalize (xdir);
    zAxis.Normalize (normal);
    if (fabs(zAxis.z + 1.0) < 0.001)
        {
        xAxis.Negate ();
        zAxis.Negate ();
        if (mirrored != nullptr)
            *mirrored = true;
        }

    // get ECS - do not compound it with currTrans - handled in central factory.
    RotMatrix   ecs;
    DwgHelper::ComputeMatrixFromXZ (ecs, xAxis, zAxis);

    dgnText->SetText (DwgHelper::ToUtf8CP(string));
    dgnText->SetOrigin (position);
    dgnText->SetOrientation (ecs);
    
    // caller to set style specific data
    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryFactory::DropText (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, DwgStringCR string, bool raw, DwgGiTextStyleCR giStyle)
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
void GeometryFactory::_Text (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, double h, double w, double oblique, DwgStringCR string)
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
void GeometryFactory::_Text (DPoint3dCR position, DVec3dCR normal, DVec3dCR xdir, DwgStringCR string, bool raw, DwgGiTextStyleCR giStyle)
    {
    // try dropping multibyte text string using a bigfont:
    if (this->DropText(position, normal, xdir, string, raw, giStyle))
        return;

    bool mirrored = false;
    Dgn::TextStringPtr  dgnText = TextString::Create ();
    if (BSISUCCESS != this->SetText(dgnText, position, normal, xdir, string, &mirrored))
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
        fontType = DwgHelper::GetFontType (fontInfo);
        font = importer.GetDgnFontFor (fontInfo);
        if (nullptr == font)
            font = fontType==DgnFontType::TrueType ? &DgnFontManager::GetLastResortTrueTypeFont() : &DgnFontManager::GetLastResortShxFont();
        if (font != nullptr)
            fontType = font->GetType ();
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

    if ((giStyle.IsBackward() && !mirrored) || giStyle.IsUpsideDown())
        DwgHelper::ResetPositionForBackwardAndUpsideDown(*dgnText, giStyle.IsBackward(), giStyle.IsUpsideDown());

    // decode escape codes if the input string is not raw primitives:
    bvector<DSegment3d> underlines, overlines;
    if (!raw)
        {
        DwgHelper::ConvertEscapeCodes (*dgnText.get(), &underlines, &overlines);
        }
    else if (giStyle.IsUnderlined())
        {
        // Workaround for Dgn::TextStringStyle::SetIsUnderlined - it does not seem to have an effect!
        DSegment3d  line;
        if (DwgHelper::CreateUnderOrOverline(*dgnText.get(), line, true) == BSISUCCESS)
            underlines.push_back (line);
        }
    else if (giStyle.IsOverlined())
        {
        DSegment3d  line;
        if (DwgHelper::CreateUnderOrOverline(*dgnText.get(), line, false) == BSISUCCESS)
            overlines.push_back (line);
        }

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
void GeometryFactory::_Xline (DPoint3dCR point1, DPoint3dCR point2)
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
void GeometryFactory::_Ray (DPoint3dCR origin, DPoint3dCR point)
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
void GeometryFactory::_Draw (DwgGiDrawableR drawable)
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
    DwgGiRegenType  savedRegentype = m_geometryOptions->_GetRegenType ();
    DwgDbEntityP    entity = nullptr;
    DwgDbEntityPtr  child(drawable.GetId(), DwgDbOpenMode::ForRead);
    if (child.OpenStatus() == DwgDbStatus::Success)
        {
        /*-------------------------------------------------------------------------------
        We are drawing a child entity of current block in the draw stack.
        Trivial reject the child as necessary, or create geometry from it if it has an
        protocal extention; otherwise prepare for the next draw call in the stack.
        -------------------------------------------------------------------------------*/
        entity = child.get ();
        if (!this->PreprocessBlockChildGeometry(entity, savedParams))
            return;
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
    if (nullptr != entity)
        {
        DwgDbBlockReferenceCP   blockRef = DwgDbBlockReference::Cast (entity);
        if (nullptr != blockRef)
            this->DrawBlockAttributes (*blockRef);

        // restore params & options for next child or popping back to previous draw stack
        m_drawParams.CopyFrom (savedParams);
        m_geometryOptions->SetRegenType (savedRegentype);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryFactory::PreprocessBlockChildGeometry (DwgDbEntityP entity, DrawParameters& savedParams)
    {
    if (entity == nullptr)
        return  false;

    // don't bother to add invisible primitives.
    if (DwgDbVisibility::Invisible == entity->GetVisibility())
        return  false;
    // trivial reject the entity if it is clipped away as a whole
    if (nullptr != m_spatialFilter && m_spatialFilter->IsEntityFilteredOut(*entity))
        return  false;
    // skip this entity if it should not be drawn
    if (this->SkipBlockChildGeometry(entity))
        return  false;

    m_drawParams.Initialize (*entity, &m_drawParams);

    // give protocal extensions a chance to create their own geometry:
    if (this->CreateBlockChildGeometry(entity) == BSISUCCESS)
        {
        m_drawParams.CopyFrom (savedParams);
        return  false;
        }

    // tell the toolkit to draw a failed ASM body as renderable geometry
    if (DwgHelper::IsAsmObject(entity))
        m_geometryOptions->SetRegenType (DwgGiRegenType::RenderCommand);

    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryFactory::SkipBlockChildGeometry (DwgDbEntityCP entity)
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
BentleyStatus GeometryFactory::CreateBlockChildGeometry (DwgDbEntityCP entity)
    {
    auto* objExt = DwgProtocolExtension::Cast (entity->QueryX(DwgProtocolExtension::Desc()));
    if (nullptr == objExt)
        return  BSIERROR;

    // trivial reject 3D elements in a 2D model
    if (m_isTargetModel2d && DwgHelper::IsNonPlanarAsmObject(entity))
        return  BSIERROR;

    auto geom = objExt->_ConvertToGeometry (entity, m_isTargetModel2d, m_drawParams.GetDwgImporter(), &m_drawParams);
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
BentleyStatus GeometryFactory::DrawBlockAttributes (DwgDbBlockReferenceCR blockRef)
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
void GeometryFactory::_Image (DwgGiImageBGRA32CR image, DPoint3dCR pos, DVec3dCR u, DVec3dCR v, DwgGiTransparencyMode mode)
    {
    BeAssert (false && L"implement _Image!!!");
    m_status = BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryFactory::_RowOfDots (size_t count, DPoint3dCR start, DVec3dCR step)
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
void GeometryFactory::_WorldLine (DPoint3d points[2])
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
void GeometryFactory::_PushModelTransform (TransformCR newTransform)
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
void GeometryFactory::_PopModelTransform ()
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
void GeometryFactory::_GetModelTransform (TransformR outTransform)
    {
    if (!m_transformStack.empty())
        outTransform = m_transformStack.back ();
    else
        outTransform.InitIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryFactory::_PushClipBoundary (DwgGiClipBoundaryCP boundary)
    {
    BeAssert (false && "_PushClipBoundary not implemented!");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryFactory::_PopClipBoundary ()
    {
    BeAssert (false && "_PopClipBoundary not implemented!");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryFactory::AppendGeometry (ICurvePrimitiveCR primitive)
    {
    GeometricPrimitivePtr   geom = GeometricPrimitive::Create (primitive);
    if (geom.IsValid())
        this->AppendGeometry (*geom.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryFactory::AppendGeometry (CurveVectorCR curveVector)
    {
    GeometricPrimitivePtr   geom = GeometricPrimitive::Create (curveVector);
    if (geom.IsValid())
        this->AppendGeometry (*geom.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryFactory::AppendGeometry (GeometricPrimitiveR geometry)
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

    auto& block = this->GetCurrentBlock ();
    auto blockId = block.GetBlockId ();
    auto perBlockCount = block.GetAndIncrementGeometryCount ();

    // build a new cache entry for the geometry
    DwgImporter::GeometryEntry   geomEntry;
    geomEntry.SetGeometry (geometry.Clone().get());
    geomEntry.SetGeometryParams (display);
    geomEntry.SetTransform (m_currentTransform);
    geomEntry.SetBlockName (block.GetBlockName());
    geomEntry.SetBlockId (blockId);
    geomEntry.SetDwgFileId (block.GetFileId());
    geomEntry.SetPartIndex (perBlockCount);

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
void GeometryFactory::ComputeCategory (DgnCategoryId& categoryId, DgnSubCategoryId& subcategoryId)
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
