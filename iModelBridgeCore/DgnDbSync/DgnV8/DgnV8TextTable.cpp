/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/DgnV8TextTable.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <DgnPlatform/AnnotationTable.h>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

//=======================================================================================
//! Handles conversion from DgnV8 TextTable element to DgnDb AnnotationTable.
// @bsiclass
//=======================================================================================
struct V8TextTableToDgnDbConverter
{
static BentleyStatus    TextStyleIdFromV8(DgnElementId& dbTextStyleId, DgnV8Api::ElementId v8TextStyleId, DgnV8Api::DgnFile&, Converter&);
static BentleyStatus    ColorFromV8(ColorDefR, uint32_t, DgnV8Api::DgnFile&);
static void             TableSymbologyValuesFromV8 (AnnotationTableSymbologyValuesR, DgnV8Api::TableSymbologyValues const&, DgnV8Api::DgnFile&, Converter&);

static void             ConvertHeaderProperties(AnnotationTableR, DgnV8Api::TextTable const&, double uorScale, Converter&);
static void             ConvertColumnProperties(AnnotationTableColumnR, DgnV8Api::TextTableColumn const&, double uorScale, Converter&);
static void             ConvertRowProperties(AnnotationTableRowR, DgnV8Api::TextTableRow const&, double uorScale, Converter&);
static void             ConvertCellProperties(AnnotationTableCellR, DgnV8Api::TextTableCell const&, double uorScale, Converter&);
static void             ConvertEdgeSymbology (AnnotationTableR, DgnV8Api::TextTable const&, DgnV8Api::TableCellIndex const&, DgnV8Api::TableCellListEdges, DgnV8Api::DgnFile&, Converter&);
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    12/15
//--------------+---------------+---------------+---------------+---------------+--------
BentleyStatus V8TextTableToDgnDbConverter::TextStyleIdFromV8(DgnElementId& dbTextStyleId, DgnV8Api::ElementId v8TextStyleId, DgnV8Api::DgnFile& v8File, Converter& converter)
    {
    if (0 == v8TextStyleId)
        return ERROR;

    dbTextStyleId = converter._RemapV8TextStyle(v8File, v8TextStyleId);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    12/15
//--------------+---------------+---------------+---------------+---------------+--------
BentleyStatus V8TextTableToDgnDbConverter::ColorFromV8(ColorDefR dbColor, uint32_t v8Color, DgnV8Api::DgnFile& v8File)
    {
    if (DgnV8Api::COLOR_BYLEVEL == v8Color)
        return ERROR;

    DgnV8Api::IntColorDef v8ColorDef;
    if (SUCCESS != DgnV8Api::DgnColorMap::ExtractElementColorInfo(&v8ColorDef, nullptr, nullptr, nullptr, nullptr, v8Color, v8File))
        return ERROR;

    dbColor = ColorDef(v8ColorDef.m_int);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    12/15
//--------------+---------------+---------------+---------------+---------------+--------
void V8TextTableToDgnDbConverter::TableSymbologyValuesFromV8(AnnotationTableSymbologyValuesR dbSymb, DgnV8Api::TableSymbologyValues const& v8Symb, DgnV8Api::DgnFile& v8File, Converter& converter)
    {
    dbSymb.Clear();

    if (v8Symb.HasLineVisible())
        dbSymb.SetLineVisible (v8Symb.GetLineVisible());

    if (v8Symb.HasLineColor())
        {
        ColorDef    dbColor;

        if (SUCCESS == ColorFromV8 (dbColor, v8Symb.GetLineColor(), v8File))
            dbSymb.SetLineColor (dbColor);
        }

    if (v8Symb.HasLineStyle())
        {
        if (DgnV8Api::STYLE_BYLEVEL != v8Symb.GetLineStyle() && !IS_LINECODE(v8Symb.GetLineStyle()))
            {
            double unitsScale;
            DgnStyleId mappedStyleId = converter._RemapLineStyle(unitsScale, v8File, v8Symb.GetLineStyle(), true);
            if (mappedStyleId.IsValid())
                dbSymb.SetLineStyle (mappedStyleId, 1.0);
            }
        }

    if (v8Symb.HasLineWeight())
        {
        if (DgnV8Api::WEIGHT_BYLEVEL != v8Symb.GetLineWeight())
            dbSymb.SetLineWeight (v8Symb.GetLineWeight());
        }

    if (v8Symb.HasFillVisible())
        dbSymb.SetFillVisible (v8Symb.GetFillVisible());

    if (v8Symb.HasFillColor())
        {
        ColorDef    dbColor;

        if (SUCCESS == ColorFromV8 (dbColor, v8Symb.GetFillColor(), v8File))
            dbSymb.SetFillColor (dbColor);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    12/15
//--------------+---------------+---------------+---------------+---------------+--------
void V8TextTableToDgnDbConverter::ConvertColumnProperties(AnnotationTableColumnR dbColumn, DgnV8Api::TextTableColumn const& v8Column, double uorScale, Converter& converter)
    {
    dbColumn.SetWidthDirect (uorScale * v8Column.GetWidth());
    dbColumn.SetWidthLock   (v8Column.GetWidthLock());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    12/15
//--------------+---------------+---------------+---------------+---------------+--------
void V8TextTableToDgnDbConverter::ConvertRowProperties(AnnotationTableRowR dbRow, DgnV8Api::TextTableRow const& v8Row, double uorScale, Converter& converter)
    {
    dbRow.SetHeightDirect (uorScale * v8Row.GetHeight());
    dbRow.SetHeightLock   (v8Row.GetHeightLock());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    12/15
//--------------+---------------+---------------+---------------+---------------+--------
void V8TextTableToDgnDbConverter::ConvertCellProperties(AnnotationTableCellR dbCell, DgnV8Api::TextTableCell const& v8Cell, double uorScale, Converter& converter)
    {
    dbCell.SetAlignment ((TableCellAlignment) v8Cell.GetAlignment());
    dbCell.SetOrientation ((TableCellOrientation) v8Cell.GetOrientation());

    DgnV8Api::TableCellMarginValues v8Margins = v8Cell.GetMargins();
    TableCellMarginValues           dbMargins = { uorScale * v8Margins.m_top, uorScale * v8Margins.m_bottom, uorScale * v8Margins.m_left, uorScale * v8Margins.m_right };

    dbCell.SetMarginsDirect (dbMargins);

    uint32_t    rowSpan = v8Cell.GetRowSpan();
    uint32_t    colSpan = v8Cell.GetColumnSpan();

    if (0 != rowSpan || 0 != colSpan)
        dbCell.GetTable().MergeCells (dbCell.GetIndex(), rowSpan, colSpan);

    DgnV8Api::TableSymbologyValuesPtr  v8Symbology = DgnV8Api::TableSymbologyValues::Create();
    v8Cell.GetFillSymbology (*v8Symbology);

    if (v8Symbology->HasFillVisible())
        {
        DgnV8Api::DgnFile*  dgnFile = const_cast <DgnV8Api::TextTableCell&> (v8Cell).GetTextTable().GetModel()->GetDgnFileP();

        AnnotationTableSymbologyValues    dbSymbology;
        TableSymbologyValuesFromV8 (dbSymbology, *v8Symbology, *dgnFile, converter);
        dbCell.SetFillSymbology (dbSymbology);
        }

    DgnV8Api::TextBlock const* v8TextBlock = v8Cell.GetTextBlock();

    if (nullptr != v8TextBlock && ! v8TextBlock->IsEmpty())
        {
        AnnotationTextBlockPtr dbTextBlock = converter._ConvertV8TextBlock(*v8TextBlock);
        dbCell.SetTextBlock (*dbTextBlock);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    12/15
//--------------+---------------+---------------+---------------+---------------+--------
void V8TextTableToDgnDbConverter::ConvertEdgeSymbology (AnnotationTableR dbTable, DgnV8Api::TextTable const& v8Table, DgnV8Api::TableCellIndex const& v8CellIndex, DgnV8Api::TableCellListEdges v8Edges, DgnV8Api::DgnFile& v8DgnFile, Converter& converter)
    {
    DgnV8Api::TextTableCell const*  v8Cell = v8Table.GetCell (v8CellIndex);

    if (nullptr == v8Cell)
        return;

    Bentley::bvector<DgnV8Api::TableSymbologyValuesPtr>    v8Symbologies;

    v8Cell->GetEdgeSymbology (v8Symbologies, DgnV8Api::TableCellListEdges::Top);

    if (UNEXPECTED_CONDITION (0 == v8Symbologies.size()))
        return;

    bvector<AnnotationTableCellIndex> dbCellIndex = { AnnotationTableCellIndex (v8CellIndex.row, v8CellIndex.col) };

    AnnotationTableSymbologyValues    dbSymbology;
    V8TextTableToDgnDbConverter::TableSymbologyValuesFromV8 (dbSymbology, *v8Symbologies[0], v8DgnFile, converter);
    dbTable.SetEdgeSymbology (dbSymbology, TableCellListEdges::Top, dbCellIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    12/15
//--------------+---------------+---------------+---------------+---------------+--------
void V8TextTableToDgnDbConverter::ConvertHeaderProperties(AnnotationTableR dbTable, DgnV8Api::TextTable const& v8Table, double uorScale, Converter& converter)
    {
    DgnV8Api::DgnFile*  dgnFile = v8Table.GetModel()->GetDgnFileP();

    dbTable.SetTitleRowCount        (v8Table.GetTitleRowCount());
    dbTable.SetHeaderRowCount       (v8Table.GetHeaderRowCount());
    dbTable.SetFooterRowCount       (v8Table.GetFooterRowCount());
    dbTable.SetHeaderColumnCount    (v8Table.GetHeaderColumnCount());
    dbTable.SetFooterColumnCount    (v8Table.GetFooterColumnCount());
    dbTable.SetBreakType            ((TableBreakType) v8Table.GetBreakType());
    dbTable.SetBreakPosition        ((TableBreakPosition) v8Table.GetBreakPosition());
    dbTable.SetBreakLength          (uorScale * v8Table.GetBreakLength());
    dbTable.SetBreakGap             (uorScale * v8Table.GetBreakGap());
    dbTable.SetRepeatHeaders        (v8Table.GetRepeatHeaders());
    dbTable.SetRepeatFooters        (v8Table.GetRepeatFooters());
    dbTable.SetDefaultColumnWidth   (uorScale * v8Table.GetDefaultColumnWidth());
    dbTable.SetDefaultRowHeight     (uorScale * v8Table.GetDefaultRowHeight());

    DgnV8Api::TableCellMarginValues v8Margins = v8Table.GetDefaultMargins();
    TableCellMarginValues           margins   = { uorScale * v8Margins.m_top, uorScale * v8Margins.m_bottom, uorScale * v8Margins.m_left, uorScale * v8Margins.m_right };
    dbTable.SetDefaultMargins       (margins);

    dbTable.SetDefaultCellAlignment     ((TableCellAlignment) v8Table.GetDefaultCellAlignment());
    dbTable.SetDefaultCellOrientation   ((TableCellOrientation) v8Table.GetDefaultCellOrientation());

    DgnV8Api::TableSymbologyValuesPtr  v8Symbology = DgnV8Api::TableSymbologyValues::Create();
    AnnotationTableSymbologyValues    dbSymbology;

    v8Table.GetDefaultFill (*v8Symbology, DgnV8Api::TableRows::Even);
    TableSymbologyValuesFromV8 (dbSymbology, *v8Symbology, *dgnFile, converter);
    dbTable.SetDefaultFill (dbSymbology, TableRows::Even);

    v8Symbology = DgnV8Api::TableSymbologyValues::Create();
    v8Table.GetDefaultFill (*v8Symbology, DgnV8Api::TableRows::Odd);
    TableSymbologyValuesFromV8 (dbSymbology, *v8Symbology, *dgnFile, converter);
    dbTable.SetDefaultFill (dbSymbology, TableRows::Odd);

    DgnElementId    dbTextStyleId;
    if (SUCCESS == TextStyleIdFromV8 (dbTextStyleId, v8Table.GetTextStyleId (DgnV8Api::TextTableRegion::Body), *dgnFile, converter))
        dbTable.SetTextStyleId (dbTextStyleId, AnnotationTableRegion::Body);

    if (SUCCESS == TextStyleIdFromV8 (dbTextStyleId, v8Table.GetTextStyleId (DgnV8Api::TextTableRegion::TitleRow), *dgnFile, converter))
        dbTable.SetTextStyleId (dbTextStyleId, AnnotationTableRegion::TitleRow);

    if (SUCCESS == TextStyleIdFromV8 (dbTextStyleId, v8Table.GetTextStyleId (DgnV8Api::TextTableRegion::HeaderRow), *dgnFile, converter))
        dbTable.SetTextStyleId (dbTextStyleId, AnnotationTableRegion::HeaderRow);

    if (SUCCESS == TextStyleIdFromV8 (dbTextStyleId, v8Table.GetTextStyleId (DgnV8Api::TextTableRegion::FooterRow), *dgnFile, converter))
        dbTable.SetTextStyleId (dbTextStyleId, AnnotationTableRegion::FooterRow);

    if (SUCCESS == TextStyleIdFromV8 (dbTextStyleId, v8Table.GetTextStyleId (DgnV8Api::TextTableRegion::HeaderColumn), *dgnFile, converter))
        dbTable.SetTextStyleId (dbTextStyleId, AnnotationTableRegion::HeaderColumn);

    if (SUCCESS == TextStyleIdFromV8 (dbTextStyleId, v8Table.GetTextStyleId (DgnV8Api::TextTableRegion::FooterColumn), *dgnFile, converter))
        dbTable.SetTextStyleId (dbTextStyleId, AnnotationTableRegion::FooterColumn);

/*
    // NEEDSWORK
        BackupTextHeight        = 29,
        DataSourceProviderId    = 30,
        BodyTextHeight          = 31,
        TitleRowTextHeight      = 32,
        HeaderRowTextHeight     = 33,
        FooterRowTextHeight     = 34,
        HeaderColumnTextHeight  = 35,
        FooterColumnTextHeight  = 36,
        DefaultTextSymbKey      = 37,
*/
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    11/15
//--------------+---------------+---------------+---------------+---------------+--------
void ConvertV8TextTableToDgnDbExtension::Register()
    {
    ConvertV8TextTableToDgnDbExtension* instance = new ConvertV8TextTableToDgnDbExtension();
    RegisterExtension(DgnV8Api::TextTableHandler::GetInstance(), *instance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    12/15
//--------------+---------------+---------------+---------------+---------------+--------
bool ConvertV8TextTableToDgnDbExtension::_GetBasisTransform(Bentley::Transform& transform, DgnV8EhCR eh, Converter& converter)
    {
    DgnV8Api::TextTableHandler& handler = DgnV8Api::TextTableHandler::GetInstance();

    if (UNEXPECTED_CONDITION (&eh.GetHandler() != &handler))
        return false;

    Bentley::DPoint3d   origin;
    Bentley::RotMatrix  rotation;

    handler.GetTransformOrigin (eh, origin);
    handler.GetOrientation (eh, rotation);

    transform.InitFrom (rotation, origin);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2017
//---------------+---------------+---------------+---------------+---------------+-------
BisConversionRule ConvertV8TextTableToDgnDbExtension::_DetermineBisConversionRule(DgnV8EhCR v8eh, DgnDbR dgndb, bool isModel3d)
    {
    DgnV8Api::DisplayHandler* v8DisplayHandler = v8eh.GetDisplayHandler();
    if (nullptr == v8DisplayHandler)
        return BisConversionRule::ToDefaultBisBaseClass;
    return BisConversionRule::ToAspectOnly;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    11/15
//--------------+---------------+---------------+---------------+---------------+--------
void ConvertV8TextTableToDgnDbExtension::_DetermineElementParams(DgnClassId& dbClass, DgnCode& dbCode, DgnCategoryId& dbCategory, DgnV8EhCR v8Eh, Converter& converter, ECObjectsV8::IECInstance const* primaryV8Instance, ResolvedModelMapping const& mm)
    {
    // We only want to override class; accept converter's default logic for code and category.
    DgnV8Api::DisplayHandler* v8DisplayHandler = v8Eh.GetDisplayHandler();
    if (nullptr == v8DisplayHandler)
        return;
    
    if (v8DisplayHandler->Is3dElem(v8Eh.GetElementCP()))
        {
        // 3d TextTables are represented as GenericGraphic3d elements.
        dbClass = converter.GetDgnDb().Domains().GetClassId(generic_ElementHandler::Graphic3d::GetHandler());
        return;
        }

    dbClass = DgnDbApi::AnnotationTable::QueryClassId(converter.GetDgnDb()); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   JoshSchifter    11/15
//--------------+---------------+---------------+---------------+---------------+--------
void ConvertV8TextTableToDgnDbExtension::_ProcessResults(ElementConversionResults& results, DgnV8EhCR v8Eh, ResolvedModelMapping const& v8mm, Converter& converter)
    {
    // We only ever expect a single result element; text has no public children.
    if (!results.m_element.IsValid() || !results.m_childElements.empty())
        {
        BeAssert (false);
        return;
        }

    AnnotationTableP dbTable = dynamic_cast<AnnotationTableP>(results.m_element.get());

    bool isModel3d = v8mm.GetDgnModel().Is3d();
    if (isModel3d)
        {
        // 3d TextTables are represented as GenericGraphic3d elements.
        BeAssert (nullptr == dbTable);
        return;
        }

    if (UNEXPECTED_CONDITION (nullptr == dbTable))
        return;

    DgnV8Api::TextTablePtr  v8Table   = DgnV8Api::TextTableHandler::GetInstance().GetTextTableForElement (v8Eh);
    if (UNEXPECTED_CONDITION (!v8Table.IsValid()))
        return;
    DgnV8Api::DgnFile&      v8DgnFile = *v8Table->GetModel()->GetDgnFileP();

    uint32_t                rowCount      = v8Table->GetRowCount();
    uint32_t                colCount      = v8Table->GetColumnCount();
    DgnV8Api::ElementId     v8TextStyleId = v8Table->GetTextStyleId (DgnV8Api::TextTableRegion::Body);
    DgnElementId            textStyleId   = converter._RemapV8TextStyle (v8DgnFile, v8TextStyleId);

    dbTable->BootStrap (rowCount, colCount, textStyleId, 1.0);

    double uorScale = v8mm.GetTransform().ColumnXMagnitude();

    V8TextTableToDgnDbConverter::ConvertHeaderProperties(*dbTable, *v8Table, uorScale, converter);

    for (DgnV8Api::TextTableColumn const& v8Column : v8Table->GetColumnVector())
        V8TextTableToDgnDbConverter::ConvertColumnProperties(*dbTable->GetColumn (v8Column.GetIndex()), v8Column, uorScale, converter);

    for (DgnV8Api::TextTableRow const& v8Row : v8Table->GetRowVector())
        V8TextTableToDgnDbConverter::ConvertRowProperties(*dbTable->GetRow (v8Row.GetIndex()), v8Row, uorScale, converter);

    for (DgnV8Api::TextTableCell const& v8Cell : v8Table->GetCellCollection())
        {
        DgnV8Api::TableCellIndex    v8CellIndex = v8Cell.GetIndex();
        AnnotationTableCellIndex    dbCellIndex (v8CellIndex.row, v8CellIndex.col);

        V8TextTableToDgnDbConverter::ConvertCellProperties(*dbTable->GetCell (dbCellIndex), v8Cell, uorScale, converter);
        }

    for (uint32_t iCol = 0; iCol < v8Table->GetColumnCount(); ++iCol)
        {
        for (uint32_t iRow = 0; iRow < v8Table->GetRowCount(); ++iRow)
            {
            DgnV8Api::TableCellIndex    v8CellIndex (iRow, iCol);

            if (0 == iRow) // Top horizontal edge symbology
                V8TextTableToDgnDbConverter::ConvertEdgeSymbology (*dbTable, *v8Table, v8CellIndex, DgnV8Api::TableCellListEdges::Top, v8DgnFile, converter);

            if (0 == iCol) // Left vertical edge symbology
                V8TextTableToDgnDbConverter::ConvertEdgeSymbology (*dbTable, *v8Table, v8CellIndex, DgnV8Api::TableCellListEdges::Left, v8DgnFile, converter);

            // All other horizontal edge symbology
            V8TextTableToDgnDbConverter::ConvertEdgeSymbology (*dbTable, *v8Table, v8CellIndex, DgnV8Api::TableCellListEdges::Bottom, v8DgnFile, converter);

            // All other vertical edge symbology
            V8TextTableToDgnDbConverter::ConvertEdgeSymbology (*dbTable, *v8Table, v8CellIndex, DgnV8Api::TableCellListEdges::Right, v8DgnFile, converter);
            }
        }
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
