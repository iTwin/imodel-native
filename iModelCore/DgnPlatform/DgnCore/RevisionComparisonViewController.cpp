/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/RevisionComparisonViewController.h>

USING_REVISION_COMPARISON_NAMESPACE
USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentState ComparisonData::GetPersistentState(DgnElementId id) const
    {
    auto iter = m_persistent.find(PersistentState(id, DbOpcode::Insert));
    return m_persistent.end() != iter ? *iter : PersistentState();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
TransientState ComparisonData::GetTransientState(DgnElementId id) const
    {
    // NB: This is inefficient, but it's also not actually used.
    auto iter = std::find_if(m_transient.begin(), m_transient.end(), [=](TransientState const& arg) { return arg.m_element->GetElementId() == id; });
    return m_transient.end() != iter ? *iter : TransientState();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ComparisonData::ContainsElement(DgnElementCP element) const
    {
    auto iter = std::find_if(m_persistent.begin(), m_persistent.end(), [=](PersistentState const& arg) { return arg.m_elementId == element->GetElementId(); });
    return (m_persistent.end() != iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ComparisonData::GetDbOpcode(DgnElementId elementId, BeSQLite::DbOpcode& opcode)
    {
    // Obtain the type of Opcode associated with an element ID
    PersistentState pers = GetPersistentState(elementId);
    TransientState tran = GetTransientState(elementId);

    if (pers.IsValid())
        opcode = pers.m_opcode;
    if (tran.IsValid())
        opcode = tran.m_opcode;

    return (pers.IsValid() || tran.IsValid()) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
 RevisionComparison::Controller::Controller(SpatialViewDefinition const& view, ComparisonData const& data, Show flags) : T_Super(view), m_comparisonData(&data), m_show(flags)//, m_label(TextString::Create())
    {
    m_cnmHandler = [](RevisionComparison::ControllerPtr controller){ };

    // Build the opcode cache
    for (auto state : m_comparisonData->GetPersistentStates())
        m_persistentOpcodeCache[state.m_elementId] = state.m_opcode;
    for (auto state : m_comparisonData->GetTransientStates())
        m_transientOpcodeCache[state.m_element->GetElementId()] = state.m_opcode;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     01/18
//-------------------------------------------------------------------------------------------
void RevisionComparison::Controller::SetItemsDisplayHandler(std::function<void(RevisionComparison::ControllerPtr)> handler)
    {
    m_cnmHandler = handler;
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     01/18
//-------------------------------------------------------------------------------------------
void RevisionComparison::Controller::_OnCategoryChange(bool singleEnable)
    {
    T_Super::_OnCategoryChange(singleEnable);

    // TFS#798515: Provide callbacks for _OnCategoryChange so that version compare may sync view controllers
    m_cnmHandler(this);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     01/18
//-------------------------------------------------------------------------------------------
void RevisionComparison::Controller::_ChangeModelDisplay(DgnModelId modelId, bool onOff)
    {
    T_Super::_ChangeModelDisplay(modelId, onOff);

    // TFS#798515: Provide callbacks for _OnCategoryChange so that version compare may sync view controllers
    m_cnmHandler(this);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     11/17
//-------------------------------------------------------------------------------------------
void RevisionComparison::Controller::SetModelDisplay(DgnModelIdSet& modelIds, bool visible)
    {
    for (DgnModelId modelId : modelIds)
        ChangeModelDisplay(modelId, visible);
    }

//-------------------------------------------------------------------------------------------
// @bsimethod                                                 Diego.Pinate     11/17
//-------------------------------------------------------------------------------------------
void RevisionComparison::Controller::SetCategoryDisplay(DgnCategoryIdSet& categories, bool visible)
    {
    for (DgnCategoryId category : categories)
        ChangeCategoryDisplay(category, visible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    07/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr  RevisionComparison::Controller::_StrokeGeometry(ViewContextR context, GeometrySourceCR source, double pixelSize)
    {
    // Avoid letting user pick elements that are not being compared
    //if (nullptr != context.GetIPickGeom() && !m_comparisonData->ContainsElement(source.ToElement()))
    //    return nullptr;
    return T_Super::_StrokeGeometry(context, source, pixelSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Diego.Pinate    08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    RevisionComparison::Controller::SetVersionLabel(Utf8String label)
    {
#ifdef USE_LABEL
    m_labelString = label;

    TextStringStylePtr style = TextStringStyle::Create();
    style->SetFont(DgnFontManager::GetDecoratorFont());
    style->SetSize(14);

    RotMatrix textMatrix;
    textMatrix.InitFromScaleFactors(1.0, -1.0, 1.0);

    m_label = TextString::Create();
    m_label->SetStyle(*style);
    m_label->SetText(m_labelString.c_str());
    
    DPoint3d low        = DPoint3d::FromZero();
    DPoint3d position   = DPoint3d::FromZero();
    if (nullptr != m_vp)
        getViewCorners(low, position, 30, *m_vp);

    position.z = 0;
    position.x = low.x;
    m_label->SetOriginFromJustificationOrigin(position, TextString::HorizontalJustification::Left, TextString::VerticalJustification::Bottom);
    m_label->SetOrientation(textMatrix);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicP TransientState::GetGraphic(ViewContextR context) const
    {
    if (m_graphic.IsValid())
        return m_graphic.get();

    auto geomElem = m_element->ToGeometrySource();
    if (nullptr != geomElem)
        m_graphic = geomElem->Stroke(context, 0.0);

    return m_graphic.get();
    }
