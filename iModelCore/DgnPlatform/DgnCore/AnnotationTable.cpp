//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/AnnotationTable.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/AnnotationTable.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationTextBlockPersistence.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
#define PARAM_ECInstanceId                      "ECInstanceId"
#define PARAM_ElementId                         "ElementId"
#define PARAM_AspectId                          "ECInstanceId"      // NEEDSWORK: Is this needed?

#define HEADER_PARAM_RowCount                   "RowCount"
#define HEADER_PARAM_ColumnCount                "ColumnCount"
#define HEADER_PARAM_TextStyleId                "TextStyleId"
#define HEADER_PARAM_TitleRowCount              "TitleRowCount"
#define HEADER_PARAM_HeaderRowCount             "HeaderRowCount"
#define HEADER_PARAM_FooterRowCount             "FooterRowCount"
#define HEADER_PARAM_HeaderColumnCount          "HeaderColumnCount"
#define HEADER_PARAM_FooterColumnCount          "FooterColumnCount"
#define HEADER_PARAM_BreakType                  "BreakType"
#define HEADER_PARAM_BreakPosition              "BreakPosition"
#define HEADER_PARAM_BreakLength                "BreakLength"
#define HEADER_PARAM_BreakGap                   "BreakGap"
#define HEADER_PARAM_RepeatHeaders              "RepeatHeaders"
#define HEADER_PARAM_RepeatFooters              "RepeatFooters"
#define HEADER_PARAM_DefaultColumnWidth         "DefaultColumnWidth"
#define HEADER_PARAM_DefaultRowHeight           "DefaultRowHeight"
#define HEADER_PARAM_DefaultMarginTop           "DefaultMarginTop"
#define HEADER_PARAM_DefaultMarginBottom        "DefaultMarginBottom"
#define HEADER_PARAM_DefaultMarginLeft          "DefaultMarginLeft"
#define HEADER_PARAM_DefaultMarginRight         "DefaultMarginRight"
#define HEADER_PARAM_DefaultCellAlignment       "DefaultCellAlignment"
#define HEADER_PARAM_DefaultCellOrientation     "DefaultCellOrientation"
#define HEADER_PARAM_FillSymbologyKeyOddRow     "FillSymbologyKeyOddRow"
#define HEADER_PARAM_FillSymbologyKeyEvenRow    "FillSymbologyKeyEvenRow"
#define HEADER_PARAM_TitleRowTextStyle          "TitleRowTextStyle"
#define HEADER_PARAM_HeaderRowTextStyle         "HeaderRowTextStyle"
#define HEADER_PARAM_FooterRowTextStyle         "FooterRowTextStyle"
#define HEADER_PARAM_HeaderColumnTextStyle      "HeaderColumnTextStyle"
#define HEADER_PARAM_FooterColumnTextStyle      "FooterColumnTextStyle"
#define HEADER_PARAM_BackupTextHeight           "BackupTextHeight"
#define HEADER_PARAM_DataSourceProviderId       "DataSourceProviderId"
#define HEADER_PARAM_BodyTextHeight             "BodyTextHeight"
#define HEADER_PARAM_TitleRowTextHeight         "TitleRowTextHeight"
#define HEADER_PARAM_HeaderRowTextHeight        "HeaderRowTextHeight"
#define HEADER_PARAM_FooterRowTextHeight        "FooterRowTextHeight"
#define HEADER_PARAM_HeaderColumnTextHeight     "HeaderColumnTextHeight"
#define HEADER_PARAM_FooterColumnTextHeight     "FooterColumnTextHeight"

#define ROW_PARAM_Index                         "RowIndex"
#define ROW_PARAM_HeightLock                    "HeightLock"
#define ROW_PARAM_Height                        "Height"

#define COLUMN_PARAM_Index                      "ColumnIndex"
#define COLUMN_PARAM_WidthLock                  "WidthLock"
#define COLUMN_PARAM_Width                      "Width"

#define CELL_PARAM_Index                        "CellIndex"
#define CELL_PARAM_TextBlock                    "TextBlock"
#define CELL_PARAM_FillKey                      "FillKey"
#define CELL_PARAM_Alignment                    "Alignment"
#define CELL_PARAM_Orientation                  "Orientation"
#define CELL_PARAM_MarginTop                    "MarginTop"
#define CELL_PARAM_MarginBottom                 "MarginBottom"
#define CELL_PARAM_MarginLeft                   "MarginLeft"
#define CELL_PARAM_MarginRight                  "MarginRight"

static const double s_doubleTol = 1.e-8;

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(AnnotationTableHandler)
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void     setWordWrapLength (AnnotationTextBlockR textBlock, double length)
    {
    textBlock.SetDocumentWidth (length);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
static double   getMinimumWordWrapLength (AnnotationTextBlockCR textBlock)
    {
    AnnotationTextBlockPtr  copyTextBlock = textBlock.Clone ();
    setWordWrapLength (*copyTextBlock, mgds_fc_epsilon);

    AnnotationTextBlockLayout layout(*copyTextBlock);
    DRange2dCR range  = layout.GetLayoutRange ();

#if defined (NEEDSWORK)
    if (copyTextBlock->GetProperties().IsVertical())
        return range.high.y - range.low.y;
    else
#endif
        return range.high.x - range.low.x;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
static double   getNonWrappedLength (AnnotationTextBlockCR textBlock)
    {
    AnnotationTextBlockPtr  copyTextBlock = textBlock.Clone ();
    setWordWrapLength (*copyTextBlock, 0.0);

    AnnotationTextBlockLayout layout(*copyTextBlock);
    DRange2dCR range  = layout.GetLayoutRange ();

#if defined (NEEDSWORK)
    if (copyTextBlock->GetProperties().IsVertical())
        return range.high.y - range.low.y;
    else
#endif
        return range.high.x - range.low.x;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
static Utf8String buildECSqlInsertString (Utf8CP schemaName, Utf8CP className, bvector<Utf8String> const& propertyNames, bool isUniqueAspect)
    {
    Utf8CP  idPropertyName = isUniqueAspect ? PARAM_ECInstanceId : PARAM_ElementId;

    Utf8PrintfString ecSql("INSERT INTO %s.%s (%s, ", schemaName, className, idPropertyName);
    Utf8PrintfString values(":%s, ", idPropertyName);

    bool addedOne = false;
    for (Utf8StringCR propertyName : propertyNames)
        {
        if (addedOne)
            {
            ecSql.append(", ");
            values.append(", ");
            }

        ecSql.append("[").append(propertyName).append("]");
        values.append(":").append(propertyName);
        addedOne = true;
        }

    ecSql.append(") VALUES (").append(values).append(")");

    return ecSql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
static Utf8String buildECSqlUpdateString (Utf8CP schemaName, Utf8CP className, bvector<Utf8String> const& propertyNames, bool isUniqueAspect)
    {
    Utf8PrintfString ecSql("UPDATE ONLY %s.%s SET ", schemaName, className);

    bool    addedOne  = false;
    for (Utf8StringCR propertyName : propertyNames)
        {
        if (addedOne)
            ecSql.append(", ");

        ecSql.append("[").append(propertyName).append("]");
        ecSql.append("=:").append(propertyName);
        addedOne = true;
        }

    Utf8CP  idPropertyName = isUniqueAspect ? PARAM_ECInstanceId : PARAM_ElementId;
    ecSql.append(" WHERE ");
    ecSql.append("[").append(idPropertyName).append("]");
    ecSql.append("=:").append(idPropertyName);

    if ( ! isUniqueAspect)
        {
        ecSql.append(" AND ");
        ecSql.append("[").append(PARAM_AspectId).append("]");
        ecSql.append("=:").append(PARAM_AspectId);
        }

    return ecSql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
static Utf8String buildECSqlDeleteString (Utf8CP schemaName, Utf8CP className, bool isUniqueAspect)
    {
    Utf8CP  idPropertyName = isUniqueAspect ? PARAM_ECInstanceId : PARAM_AspectId;

    Utf8PrintfString ecSql("DELETE FROM %s.%s WHERE ", schemaName, className);
    ecSql.append("[").append(idPropertyName).append("]");
    ecSql.append("=:").append(idPropertyName);

    return ecSql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
static Utf8String buildECSqlSelectString (Utf8CP schemaName, Utf8CP className, bvector<Utf8String> const& propertyNames, bool isUniqueAspect)
    {
    bool first = true;
    Utf8String ecSql("SELECT ");

    if ( ! isUniqueAspect)
        {
        // Always select aspectId first
        ecSql.append("i.[").append(PARAM_AspectId).append("]");
        first = false;
        }

    for (Utf8StringCR propertyName : propertyNames)
        {
        if ( ! first)
            ecSql.append(",");
        ecSql.append("i.[").append(propertyName).append("]");
        first = false;
        }

    Utf8PrintfString from(" FROM %s.%s i", schemaName, className);
    ecSql.append (from);

    Utf8CP  idPropertyName = isUniqueAspect ? PARAM_ECInstanceId : PARAM_ElementId;
    Utf8PrintfString whereStr(" WHERE %s=?", idPropertyName);
    ecSql.append (whereStr);

    return ecSql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
AspectTypeData const&   AnnotationTableAspect::GetAspectTypeData(AnnotationTableAspectType type)
    {
    static bvector<AspectTypeData> s_typeData;

    if (s_typeData.empty())
        {
        PropertyNames headerNames =  TableHeaderAspect::GetPropertyNames();
        PropertyNames rowNames =     AnnotationTableRow::GetPropertyNames();
        PropertyNames colNames =     AnnotationTableColumn::GetPropertyNames();
        PropertyNames cellNames =    AnnotationTableCell::GetPropertyNames();
// NEEDSWORK
//        PropertyNames mergeNames =   AnnotationTableMerge::GetPropertyNames();
//        PropertyNames fillNames =    AnnotationTableFill::GetPropertyNames();
//        PropertyNames symbNames =    AnnotationTableSymbology::GetPropertyNames();
//        PropertyNames edgeRunNames = AnnotationTableEdgeRun::GetPropertyNames();

        s_typeData = 
            {
            { AspectTypeData (AnnotationTableAspectType::Header,    headerNames,   true,  DGN_CLASSNAME_AnnotationTableHeader)     },
            { AspectTypeData (AnnotationTableAspectType::Row,       rowNames,      false, DGN_CLASSNAME_AnnotationTableRow)        },
            { AspectTypeData (AnnotationTableAspectType::Column,    colNames,      false, DGN_CLASSNAME_AnnotationTableColumn)     },
            { AspectTypeData (AnnotationTableAspectType::Cell,      cellNames,     false, DGN_CLASSNAME_AnnotationTableCell)       },
// NEEDSWORK
//            { AspectTypeData (AnnotationTableAspectType::Merge,     mergeNames,    false, DGN_CLASSNAME_AnnotationTableMerge)      },
//            { AspectTypeData (AnnotationTableAspectType::Fill,      fillNames,     false, DGN_CLASSNAME_AnnotationTableFill)       },
//            { AspectTypeData (AnnotationTableAspectType::Symbology, symbNames,     false, DGN_CLASSNAME_AnnotationTableSymbology)  },
//            { AspectTypeData (AnnotationTableAspectType::EdgeRun,   edgeRunNames,  false, DGN_CLASSNAME_AnnotationTableEdgeRun)    },
            };
        }

    return s_typeData[(uint32_t) type];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/ AnnotationTableAspect::AnnotationTableAspect (AnnotationTableAspectCR rhs)
    :
    m_table (rhs.m_table)
    {
    CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableAspect::CopyDataFrom (AnnotationTableAspectCR rhs)
    {
    m_aspectId   = rhs.m_aspectId;
    m_hasChanges = rhs.m_hasChanges;

    _CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool AnnotationTableAspect::BindIfNull (ECSqlStatement& statement, Utf8CP paramName, bool isNull)
    {
    if ( ! isNull)
        return false;

    statement.BindNull (statement.GetParameterIndex(paramName));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableAspect::BindInt (ECSqlStatement& statement, Utf8CP paramName, TableIntValue const& value)
    {
    if ( ! BindIfNull (statement, paramName, value.IsNull()))
        statement.BindInt (statement.GetParameterIndex(paramName), value.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableAspect::BindUInt (ECSqlStatement& statement, Utf8CP paramName, TableUIntValue const& value)
    {
    if ( ! BindIfNull (statement, paramName, value.IsNull()))
        statement.BindInt (statement.GetParameterIndex(paramName), value.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableAspect::BindInt64 (ECSqlStatement& statement, Utf8CP paramName, TableUInt64Value const& value)
    {
    if ( ! BindIfNull (statement, paramName, value.IsNull()))
        statement.BindInt64 (statement.GetParameterIndex(paramName), value.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableAspect::BindBool (ECSqlStatement& statement, Utf8CP paramName, TableBoolValue const& value)
    {
    if ( ! BindIfNull (statement, paramName, value.IsNull()))
        statement.BindBoolean (statement.GetParameterIndex(paramName), value.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableAspect::BindDouble (ECSqlStatement& statement, Utf8CP paramName, TableDoubleValue const& value)
    {
    if ( ! BindIfNull (statement, paramName, value.IsNull()))
        statement.BindDouble (statement.GetParameterIndex(paramName), value.GetValue());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
CachedECSqlStatementPtr AnnotationTableAspect::GetPreparedSelectStatement (AnnotationTableAspectType aspectType, AnnotationTableElementCR table)
    {
    AspectTypeData  typeData  = GetAspectTypeData (aspectType);
    Utf8StringR     sqlString = typeData.m_ecSqlSelectString;

    if (sqlString.empty())
        sqlString = buildECSqlSelectString (DGN_ECSCHEMA_NAME, typeData.m_ecClassName, typeData.m_propertyNames, typeData.m_isUniqueAspect);

    CachedECSqlStatementPtr statement = table.GetDgnDb().GetPreparedECSqlStatement(sqlString.c_str());
    if (!statement.IsValid())
        return nullptr;

    statement->BindId(1, table.GetElementId());

    return statement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableAspect::BindProperties (ECSqlStatement& statement, bool isUpdate)
    {
    Utf8CP  elemIdProp = _IsUniqueAspect() ? PARAM_ECInstanceId : PARAM_ElementId;

    statement.BindId (statement.GetParameterIndex(elemIdProp), GetTable().GetElementId());

    if (isUpdate && ! _IsUniqueAspect() && EXPECTED_CONDITION (m_aspectId.IsValid()))
        statement.BindInt64  (statement.GetParameterIndex(PARAM_AspectId), m_aspectId.GetValue());

    _BindProperties (statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTableAspect::InsertInDb()
    {
    AspectTypeData  typeData  = GetAspectTypeData (_GetAspectType());
    Utf8StringR     sqlString = typeData.m_ecSqlUpdateString;

    if (sqlString.empty())
        sqlString = buildECSqlInsertString (DGN_ECSCHEMA_NAME, typeData.m_ecClassName, typeData.m_propertyNames, typeData.m_isUniqueAspect);

    CachedECSqlStatementPtr statement = m_table.GetDgnDb().GetPreparedECSqlStatement(sqlString.c_str());
    if (!statement.IsValid())
        return ERROR;

    BindProperties (*statement, false);

    if (DbResult::BE_SQLITE_DONE != statement->Step())
        return ERROR;

    m_aspectId = m_table.GetDgnDb().GetLastInsertRowId();
    ClearHasChanges();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTableAspect::UpdateInDb()
    {
    AspectTypeData  typeData  = GetAspectTypeData (_GetAspectType());
    Utf8StringR     sqlString = typeData.m_ecSqlUpdateString;

    if (sqlString.empty())
        sqlString = buildECSqlUpdateString (DGN_ECSCHEMA_NAME, typeData.m_ecClassName, typeData.m_propertyNames, typeData.m_isUniqueAspect);

    CachedECSqlStatementPtr statement = m_table.GetDgnDb().GetPreparedECSqlStatement(sqlString.c_str());
    if (!statement.IsValid())
        return ERROR;

    BindProperties (*statement, true);

    if (DbResult::BE_SQLITE_DONE != statement->Step())
        return ERROR;

    // Step will return DONE even if zero rows are modified.
    if (0 == m_table.GetDgnDb().GetModifiedRowCount())
        return ERROR;

    ClearHasChanges();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTableAspect::DeleteAspectFromDb (AnnotationTableAspectType aspectType, uint64_t aspectId, AnnotationTableElementR table)
    {
    AspectTypeData  typeData  = GetAspectTypeData (aspectType);
    Utf8StringR     sqlString = typeData.m_ecSqlDeleteString;

    if (sqlString.empty())
        sqlString = buildECSqlDeleteString (DGN_ECSCHEMA_NAME, typeData.m_ecClassName, typeData.m_isUniqueAspect);

    CachedECSqlStatementPtr statement = table.GetDgnDb().GetPreparedECSqlStatement(sqlString.c_str());
    if (!statement.IsValid())
        return ERROR;

    statement->BindInt64 (statement->GetParameterIndex(PARAM_AspectId), aspectId);

    if (UNEXPECTED_CONDITION (BE_SQLITE_DONE != statement->Step()))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTableAspect::DeleteFromDb()
    {
    return DeleteAspectFromDb (_GetAspectType(), GetAspectId(), m_table);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableAspect::AssignProperties (ECSqlStatement const& statement)
    {
    int firstPropIndex = 0;
    int firstStatementColumn = 0;
    
    if ( ! _IsUniqueAspect ())
        {
        IECSqlValue const&  value = statement.GetValue (0);

        if (EXPECTED_CONDITION ( ! value.IsNull()))
            m_aspectId.SetValue (value.GetInt64());

        // We've already read AspectId (just above) and prop0 (before this method)
        // so we want to start on the second column which is propIndex 1
        //
        // Pictorially... the statement looks like this:
        //          Col 0    | Col 1 | Col 2 | Col3
        //          AspectId | prop0 | prop1 | prop2
        //                              |
        //   We want to start here------^
        //
        firstPropIndex = 1;
        firstStatementColumn = 2;
        }

    for (int statementColumn = firstStatementColumn, propIndex = firstPropIndex; statementColumn < statement.GetColumnCount (); statementColumn++, propIndex++)
        {
        IECSqlValue const&  value = statement.GetValue (statementColumn);

        if ( ! value.IsNull())
            _AssignValue (propIndex, value);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  AnnotationTableColumn::AnnotationTableColumn (AnnotationTableElementR table, int index)
    :
    AnnotationTableAspect (table), m_index (index)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  AnnotationTableColumn::AnnotationTableColumn (AnnotationTableColumnCR rhs) : AnnotationTableAspect (rhs)
    {
    CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableColumnR AnnotationTableColumn::operator= (AnnotationTableColumnCR rhs)
    {
    CopyDataFrom (rhs);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableColumn::_CopyDataFrom (AnnotationTableAspectCR rhsAspect)
    {
    AnnotationTableColumnCR   rhs = static_cast <AnnotationTableColumnCR> (rhsAspect);

    m_index             = rhs.m_index;
    m_widthLock         = rhs.m_widthLock;
    m_width             = rhs.m_width;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
PropertyNames   AnnotationTableColumn::GetPropertyNames()
    {
    PropertyNames names =
        {
        { (int) AnnotationTableColumn::PropIndex::ColumnIndex,  COLUMN_PARAM_Index      },
        { (int) AnnotationTableColumn::PropIndex::WidthLock,    COLUMN_PARAM_WidthLock  },
        { (int) AnnotationTableColumn::PropIndex::Width,        COLUMN_PARAM_Width      },
        };

    return names;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void AnnotationTableColumn::_FlushChangesToProperties()
    {
    double  defaultWidth = GetTable().GetDefaultColumnWidth();
    double  colWidth  = GetWidth();

    if (DoubleOps::WithinTolerance (defaultWidth, colWidth, s_doubleTol))
        m_width.Clear ();

    if ( ! GetWidthLock ())
        m_widthLock.Clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
bool    AnnotationTableColumn::_ShouldBePersisted() const
    {
    if (m_width.IsValid())         return true;
    if (m_widthLock.IsValid())     return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableColumn::_BindProperties(ECSqlStatement& statement)
    {
    statement.BindInt (statement.GetParameterIndex(COLUMN_PARAM_Index), m_index);

    BindBool    (statement, COLUMN_PARAM_WidthLock,       m_widthLock);
    BindDouble  (statement, COLUMN_PARAM_Width,           m_width);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void  AnnotationTableColumn::_AssignValue (int index, IECSqlValue const& value)
    {
    PropIndex propIndex = static_cast <PropIndex> (index);

    switch (propIndex)
        {
        case PropIndex::Width:          m_width.SetValue       (value.GetDouble());    break;
        case PropIndex::WidthLock:      m_widthLock.SetValue   (value.GetBoolean());   break;
        default:                        BeAssert (false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableColumn::SetIndex (uint32_t val)
    {
    m_index = val;
    SetHasChanges();

#if defined (NEEDSWORK)
    // Adjust all the edge runs owned by this column
    for (TextTableEdgeRun& edgeRun: m_edgeRuns)
        edgeRun.SetHostIndex (m_index);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<AnnotationTableCellP>     AnnotationTableColumn::FindCells() const
    {
    // return any cells that intersect this row
    bvector<AnnotationTableCellP>     cells;

    for (uint32_t rowIndex = 0; rowIndex < GetTable().GetRowCount(); rowIndex++)
        {
        // try to find a merge root in this row that spans this col
        uint32_t  colIndex = m_index;

        while (true)
            {
            AnnotationTableCellP cell = GetTable().GetCell (AnnotationTableCellIndex (rowIndex, colIndex));

            if (NULL != cell && colIndex + cell->GetColumnSpan() - 1 >= m_index)
                {
                cells.push_back (cell);
                break;
                }

            if (0 == colIndex)
                break;

            colIndex--;
            }
        }

    return cells;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void  AnnotationTableColumn::SetWidthLock   (bool   val)  { m_widthLock.SetValue (val);   SetHasChanges(); }
void  AnnotationTableColumn::SetWidthDirect (double val)  { m_width.SetValue (val);       SetHasChanges(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
double  AnnotationTableColumn::GetWidth () const      { return m_width.IsValid() ? m_width.GetValue() : GetTable().GetDefaultColumnWidth(); }
bool    AnnotationTableColumn::GetWidthLock () const  { return m_widthLock.IsValid() ? m_widthLock.GetValue() : true; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableColumn::ShrinkWidthToContents ()
    {
    double  minimumWidth = GetMinimumWidth(false);

    SetWidth (minimumWidth, SizeLockAction::NoChange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
TableHeaderFooterType   AnnotationTableColumn::GetHeaderFooterType () const
    {
    return GetTable().GetColumnHeaderFooterType (GetIndex());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::WidthChanged ()
    {
    /*-------------------------------------------------------------------------
        Layout the contents based on the new width and fix the row height if needed.
    -------------------------------------------------------------------------*/
    DVec2d                  cellSize = GetSize();
    TableCellMarginValues   margins  = GetMargins();
    double                  hMargins = margins.m_left + margins.m_right;

    FitContentToWidth (cellSize.x - hMargins);

    // Will only affect the last row in the cell's span
    AnnotationTableCellIndex    cellIndex   = GetIndex();
    uint32_t                    rowIndex    = cellIndex.row + GetRowSpan() - 1;
    AnnotationTableRowP         row         = GetTable().GetRow (rowIndex);

    DVec2d          cellNeededSize          = GetContentSize();
    double          additionalNeededHeight  = cellNeededSize.y - cellSize.y;

    // If the cell needs more height make the row taller
    if (0 < additionalNeededHeight)
        {
        double      newRowHeight = row->GetHeight() + additionalNeededHeight;

        row->SetHeight (newRowHeight, SizeLockAction::NoChange);
        return;
        }

    // See if we can shrink the row height
    if ( ! row->GetHeightLock() && 0 > additionalNeededHeight)
        row->ShrinkHeightToContents();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableColumn::SetWidth (double newValue, SizeLockAction lockAction)
    {
    double oldValue = GetWidth();

    SetWidthDirect (newValue);

    if (SizeLockAction::TurnOn == lockAction)
        SetWidthLock (true);
    else
    if (SizeLockAction::TurnOff == lockAction)
        SetWidthLock (false);

    if (DoubleOps::WithinTolerance (newValue, oldValue, s_doubleTol))
        return;

    /*-------------------------------------------------------------------------
        Layout the contents of each cell based on the new width and fix the
        row heights if needed.
    -------------------------------------------------------------------------*/
    for (AnnotationTableCellP const& cell: FindCells())
        cell->WidthChanged();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
double AnnotationTableColumn::GetMinimumWidth (bool allowedToChangeContentLayout) const
    {
    bool                colHasUnsharedCells = false;
    double              minimumWidth = 0.0;
    double              currColWidth = GetWidth();

    for (AnnotationTableCellP const& cell: FindCells())
        {
        DVec2d          cellSize                    = cell->GetSize();
        double          widthSuppliedByOtherCols    = cellSize.x - currColWidth;
        double          widthNeededForContent;

        if (cell->GetIndex().col == GetIndex() && 1 == cell->GetColumnSpan())
            colHasUnsharedCells = true;

        if (allowedToChangeContentLayout)
            widthNeededForContent = cell->GetFullyCompressedContentWidth();
        else
            widthNeededForContent = cell->GetContentSize().x;

        double          colNeededWidth  = widthNeededForContent - widthSuppliedByOtherCols;

        if (minimumWidth < colNeededWidth)
            minimumWidth = colNeededWidth;
        }

    if ( ! colHasUnsharedCells)
        {
        // If all the cells are merged with other columns, it is possible that the entire needed
        // width might be supplied by those columns.  But we don't want min width to be zero.
        // The alternate minimum is computed as if the column contained its own empty cells.
        double altMin = GetAlternateMinimumWidth();

        if (minimumWidth < altMin)
            minimumWidth = altMin;
        }

    return minimumWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
double AnnotationTableColumn::ComputeMinimumWidth () const
    {
    return GetMinimumWidth (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableColumn::SetWidth (double newWidth)
    {
    double  minimumWidth = GetMinimumWidth(true);

    // Never set the width smaller than the minimum
    if (newWidth < minimumWidth)
        newWidth = minimumWidth;

    SetWidth (newWidth, SizeLockAction::TurnOn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableColumn::SetWidthFromContents (bool allowedToChangeContentLayout)
    {
    double  maxNeededWidth = 0.0;
    double  currColWidth = GetWidth();

    /*-------------------------------------------------------------------------
        Find the maximum width needed based on the contents of the column.
    -------------------------------------------------------------------------*/
    for (AnnotationTableCellP const& cell: FindCells())
        {
        DVec2d  cellSize                    = cell->GetSize();
        double  widthSuppliedByOtherCols    = cellSize.x - currColWidth;
        double  widthNeededForContent;

        if (allowedToChangeContentLayout)
            widthNeededForContent = cell->GetFullyExpandedContentWidth();
        else
            widthNeededForContent = cell->GetContentSize().x;

        double  colNeededWidth  = widthNeededForContent - widthSuppliedByOtherCols;

        if (maxNeededWidth < colNeededWidth)
            maxNeededWidth = colNeededWidth;
        }

    /*-------------------------------------------------------------------------
        Clear the lock flag if we used the expanded width, this column is
        now free to grow/shrink with future content width changes.
    -------------------------------------------------------------------------*/
    SizeLockAction  lockAction = allowedToChangeContentLayout ? SizeLockAction::TurnOff : SizeLockAction::NoChange;

    SetWidth (maxNeededWidth, lockAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableColumn::SetWidthFromContents ()
    {
    SetWidthFromContents (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  AnnotationTableRow::AnnotationTableRow (AnnotationTableElementR table, int index)
    :
    AnnotationTableAspect (table), m_index (index)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  AnnotationTableRow::AnnotationTableRow (AnnotationTableRowCR rhs) : AnnotationTableAspect (rhs)
    {
    CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableRowR AnnotationTableRow::operator= (AnnotationTableRowCR rhs)
    {
    CopyDataFrom (rhs);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableRow::_CopyDataFrom (AnnotationTableAspectCR rhsAspect)
    {
    AnnotationTableRowCR   rhs = static_cast <AnnotationTableRowCR> (rhsAspect);

    m_index             = rhs.m_index;
    m_heightLock        = rhs.m_heightLock;
    m_height            = rhs.m_height;

    if (UNEXPECTED_CONDITION (m_cells.size() != rhs.m_cells.size()))
        return;

    for (AnnotationTableCellR cell : m_cells)
        cell.CopyDataFrom (rhs.m_cells[cell.GetIndex().col]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
PropertyNames   AnnotationTableRow::GetPropertyNames()
    {
    PropertyNames propNames =
        {
        { (int) AnnotationTableRow::PropIndex::RowIndex,   ROW_PARAM_Index       },
        { (int) AnnotationTableRow::PropIndex::HeightLock, ROW_PARAM_HeightLock  },
        { (int) AnnotationTableRow::PropIndex::Height,     ROW_PARAM_Height      },
        };

    return propNames;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void AnnotationTableRow::_FlushChangesToProperties()
    {
    double  defaultHeight = GetTable().GetDefaultRowHeight();
    double  rowHeight  = GetHeight();

    if (DoubleOps::WithinTolerance (defaultHeight, rowHeight, s_doubleTol))
        m_height.Clear ();

    if ( ! GetHeightLock ())
        m_heightLock.Clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
bool    AnnotationTableRow::_ShouldBePersisted() const
    {
    if (m_height.IsValid())         return true;
    if (m_heightLock.IsValid())     return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableRow::_BindProperties(ECSqlStatement& statement)
    {
    statement.BindInt (statement.GetParameterIndex(ROW_PARAM_Index), m_index);

    BindBool    (statement, ROW_PARAM_HeightLock,       m_heightLock);
    BindDouble  (statement, ROW_PARAM_Height,           m_height);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void  AnnotationTableRow::_AssignValue (int index, IECSqlValue const& value)
    {
    PropIndex propIndex = static_cast <PropIndex> (index);

    switch (propIndex)
        {
        case PropIndex::Height:         m_height.SetValue       (value.GetDouble());    break;
        case PropIndex::HeightLock:     m_heightLock.SetValue   (value.GetBoolean());   break;
        default:                        BeAssert (false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableRow::InitializeInternalCollections()
    {
    PRECONDITION(m_cells.empty(),);
#if defined (NEEDSWORK)
    PRECONDITION(m_edgeRuns.empty(),);
    PRECONDITION(m_fillRuns.empty(),);
#endif

    // Cells
    for (uint32_t colIndex = 0; colIndex < GetTable().GetColumnCount(); colIndex++)
        {
        AnnotationTableCell cell (GetTable(), AnnotationTableCellIndex (m_index, colIndex));
        m_cells.push_back (cell);
        }

#if defined (NEEDSWORK)
    // EdgeRuns
    TextTableEdgeRun edgeRun;
    edgeRun.Initialize (*m_table, EdgeRunHostType::Row, m_index);
    m_edgeRuns.push_back (edgeRun);

    // FillRuns
    TextTableFillRun fillRun;
    fillRun.Initialize (*m_table, m_index);
    m_fillRuns.push_back (fillRun);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableRow::SetIndex (uint32_t val)
    {
    m_index = val;
    SetHasChanges();

    // Adjust all the cells owned by this row
    for (AnnotationTableCell& cell: m_cells)
        cell.SetIndex (AnnotationTableCellIndex (m_index, cell.GetIndex().col));

#if defined (NEEDSWORK)
    // Adjust all the edge runs owned by this row
    for (TextTableEdgeRun& edgeRun: m_edgeRuns)
        edgeRun.SetHostIndex (m_index);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<AnnotationTableCellP>     AnnotationTableRow::FindCells() const
    {
    // return any cells that intersect this row
    bvector<AnnotationTableCellP>     cells;

    for (uint32_t colIndex = 0; colIndex < GetTable().GetColumnCount(); colIndex++)
        {
        // try to find a merge root in this column that spans this row
        uint32_t  rowIndex = m_index;

        while (true)
            {
            AnnotationTableCellP cell = GetTable().GetCell (AnnotationTableCellIndex (rowIndex, colIndex));

            if (NULL != cell && rowIndex + cell->GetRowSpan() - 1 >= m_index)
                {
                cells.push_back (cell);
                break;
                }

            if (0 == rowIndex)
                break;

            rowIndex--;
            }
        }

    return cells;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
double  AnnotationTableRow::GetHeight () const      { return m_height.IsValid() ? m_height.GetValue() : GetTable().GetDefaultRowHeight(); }
bool    AnnotationTableRow::GetHeightLock () const  { return m_heightLock.GetValue(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableRow::SetHeightLock    (bool   val)    { m_heightLock.SetValue (val); SetHasChanges(); }
void AnnotationTableRow::SetHeightDirect  (double val)    { m_height.SetValue (val);     SetHasChanges(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableRow::SetHeight (double newValue, SizeLockAction lockAction)
    {
    double oldValue = GetHeight();

    SetHeightDirect (newValue);

    if (SizeLockAction::TurnOn == lockAction)
        SetHeightLock (true);
    else
    if (SizeLockAction::TurnOff == lockAction)
        SetHeightLock (false);

    if (DoubleOps::WithinTolerance (newValue, oldValue, s_doubleTol))
        return;

    /*-------------------------------------------------------------------------
        Layout the contents of each cell based on the new height and fix the
        column widths if needed.
    -------------------------------------------------------------------------*/
    for (AnnotationTableCellP const& cell: FindCells())
        cell->HeightChanged();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableElement::ConsiderRegionForAlternateMinimumSize (double& min, AnnotationTableRegion region, bool isHeight) const
    {
    DVec2d                  minSize   = GetEmptyMinimumSizeWithoutMargins (region);
    TableCellMarginValues   margins   = GetDefaultMargins ();
    double                  size      = 0.0;

    if (isHeight)
        size = minSize.y + margins.m_top + margins.m_bottom;
    else
        size = minSize.x + margins.m_left + margins.m_right;

    if (min < size)
        min = size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
double AnnotationTableRow::GetAlternateMinimumHeight () const
    {
    double                  minHeight = 0.0;
    TableHeaderFooterType   rowType   = GetHeaderFooterType ();

    if (TableHeaderFooterType::Body == rowType)
        {
        GetTable().ConsiderRegionForAlternateMinimumSize (minHeight, AnnotationTableRegion::Body, true);
        return minHeight;
        }

    uint32_t  numHeaderCols = GetTable().GetHeaderColumnCount();
    uint32_t  numFooterCols = GetTable().GetFooterColumnCount();
    uint32_t  numBodyCols   = GetTable().GetColumnCount() - numHeaderCols - numFooterCols;

    if (0 != numHeaderCols)
        GetTable().ConsiderRegionForAlternateMinimumSize (minHeight, AnnotationTableRegion::HeaderColumn, true);

    if (0 != numFooterCols)
        GetTable().ConsiderRegionForAlternateMinimumSize (minHeight, AnnotationTableRegion::FooterColumn, true);

    if (0 != numBodyCols)
        GetTable().ConsiderRegionForAlternateMinimumSize (minHeight, AnnotationTableRegion::Body, true);

    return minHeight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
double AnnotationTableColumn::GetAlternateMinimumWidth () const
    {
    double  minWidth = 0.0;

    uint32_t  numTitleRows  = GetTable().GetTitleRowCount();
    uint32_t  numHeaderRows = GetTable().GetHeaderRowCount();
    uint32_t  numFooterRows = GetTable().GetFooterRowCount();
    uint32_t  numBodyRows   = GetTable().GetRowCount() - numTitleRows - numHeaderRows - numFooterRows;

    if (0 != numTitleRows)
        GetTable().ConsiderRegionForAlternateMinimumSize (minWidth, AnnotationTableRegion::TitleRow, false);

    if (0 != numHeaderRows)
        GetTable().ConsiderRegionForAlternateMinimumSize (minWidth, AnnotationTableRegion::HeaderRow, false);

    if (0 != numFooterRows)
        GetTable().ConsiderRegionForAlternateMinimumSize (minWidth, AnnotationTableRegion::FooterRow, false);

    if (0 != numBodyRows)
        {
        TableHeaderFooterType   colType   = GetHeaderFooterType ();
        AnnotationTableRegion   region    = AnnotationTableElement::GetTableRegionFromColumnType (colType);

        GetTable().ConsiderRegionForAlternateMinimumSize (minWidth, region, false);
        }

    return minWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
double AnnotationTableRow::GetMinimumHeight (bool allowedToChangeContentLayout) const
    {
    bool                rowHasUnsharedCells = false;
    double              minimumHeight = 0;
    double              currRowHeight = GetHeight();

    for (AnnotationTableCellP const& cell: FindCells())
        {
        DVec2d          cellSize                    = cell->GetSize();
        double          heightSuppliedByOtherRows   = cellSize.y - currRowHeight;
        double          heightNeededForContent;

        if (cell->GetIndex().row == GetIndex() && 1 == cell->GetRowSpan())
            rowHasUnsharedCells = true;

        if (allowedToChangeContentLayout)
            heightNeededForContent = cell->GetFullyCompressedContentHeight();
        else
            heightNeededForContent = cell->GetContentSize().y;

        double          rowNeededHeight = heightNeededForContent - heightSuppliedByOtherRows;

        if (minimumHeight < rowNeededHeight)
            minimumHeight = rowNeededHeight;
        }

    if ( ! rowHasUnsharedCells)
        {
        // If all the cells are merged with other rows, it is possible that the entire needed
        // height might be supplied by those rows.  But we don't want min height to be zero.
        // The alternate minimum is computed as if the row contained its own empty cells.
        double altMin = GetAlternateMinimumHeight();

        if (minimumHeight < altMin)
            minimumHeight = altMin;
        }

    return minimumHeight;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/14
+---------------+---------------+---------------+---------------+---------------+------*/
double AnnotationTableRow::ComputeMinimumHeight () const
    {
    return GetMinimumHeight (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableRow::SetHeight (double newHeight)
    {
    double  minimumHeight = GetMinimumHeight(true);

    // Never set the height smaller than the minimum
    if (newHeight < minimumHeight)
        newHeight = minimumHeight;

    SetHeight (newHeight, SizeLockAction::TurnOn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableRow::ShrinkHeightToContents ()
    {
    double  minimumHeight = GetMinimumHeight(false);

    SetHeight (minimumHeight, SizeLockAction::NoChange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::HeightChanged ()
    {
    /*-------------------------------------------------------------------------
        Layout the contents based on the new height and fix the column width if needed.
    -------------------------------------------------------------------------*/
    DVec2d                  cellSize = GetSize();
    TableCellMarginValues   margins  = GetMargins();
    double                  vMargins = margins.m_top + margins.m_bottom;

    FitContentToHeight (cellSize.y - vMargins);

    // Will only affect the last column in the cell's span
    AnnotationTableCellIndex    cellIndex   = GetIndex();
    uint32_t                    colIndex    = cellIndex.col + GetColumnSpan() - 1;
    AnnotationTableColumnP      column      = GetTable().GetColumn (colIndex);

    DVec2d          cellNeededSize          = GetContentSize();
    double          additionalNeededWidth   = cellNeededSize.x - cellSize.x;

    // If the cell needs more width make the column wider
    if (0 < additionalNeededWidth)
        {
        double      newColWidth = column->GetWidth() + additionalNeededWidth;

        column->SetWidth (newColWidth, SizeLockAction::NoChange);
        return;
        }

    // See if we can shrink the column width
    if ( ! column->GetWidthLock() && 0 > additionalNeededWidth)
        column->ShrinkWidthToContents();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableRow::SetHeightFromContents (bool allowedToChangeContentLayout)
    {
    double  maxNeededHeight = 0.0;
    double  currRowHeight = GetHeight();

    /*-------------------------------------------------------------------------
        Find the maximum height needed based on the contents of the row.
    -------------------------------------------------------------------------*/
    for (AnnotationTableCellP const& cell: FindCells())
        {
        DVec2d  cellSize                    = cell->GetSize();
        double  heightSuppliedByOtherRows   = cellSize.y - currRowHeight;
        double  heightNeededForContent;

        if (allowedToChangeContentLayout)
            heightNeededForContent = cell->GetFullyExpandedContentHeight();
        else
            heightNeededForContent = cell->GetContentSize().x;

        double  rowNeededHeight  = heightNeededForContent - heightSuppliedByOtherRows;

        if (maxNeededHeight < rowNeededHeight)
            maxNeededHeight = rowNeededHeight;
        }

    /*-------------------------------------------------------------------------
        Clear the lock flag if we used the expanded height, this row is
        now free to grow/shrink with future content height changes.
    -------------------------------------------------------------------------*/
    SizeLockAction  lockAction = allowedToChangeContentLayout ? SizeLockAction::TurnOff : SizeLockAction::NoChange;

    SetHeight (maxNeededHeight, lockAction);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableRow::SetHeightFromContents ()
    {
    SetHeightFromContents (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
TableHeaderFooterType   AnnotationTableRow::GetHeaderFooterType () const
    {
    return GetTable().GetRowHeaderFooterType (GetIndex());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */      TextBlockHolder::TextBlockHolder (AnnotationTableCellR c) : CellContentHolder (c)
    {
    m_textBlockState = TEXTBLOCK_STATE_Uninitialized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void        TextBlockHolder::_FlushChangesToProperties ()
    {
    if (m_textBlockState != TEXTBLOCK_STATE_Changed)
        return;

    if (m_textBlock->IsEmpty())
        {
        m_tableCell->ClearBinaryTextBlock();
        SetTextBlockDirect (nullptr);
        }
    else
        {
        bvector<Byte>   bytes;
        AnnotationTextBlockPersistence::EncodeAsFlatBuf (bytes, *m_textBlock);
        m_tableCell->AssignBinaryTextBlock (&bytes[0], bytes.size());
        }

    m_textBlockState = TEXTBLOCK_STATE_Initialized;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTextBlockLayoutCP     TextBlockHolder::GetTextBlockLayout () const
    {
    if (m_textBlockLayout.IsValid())
        return m_textBlockLayout.get();

    AnnotationTextBlockCP   textBlock = GetTextBlock();

    if (nullptr != textBlock)
        m_textBlockLayout = new AnnotationTextBlockLayout(*textBlock);

    return m_textBlockLayout.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTextBlockCP     TextBlockHolder::GetTextBlock () const
    {
    if (TEXTBLOCK_STATE_Uninitialized != m_textBlockState)
        return m_textBlock.get();

    size_t      numBytes;
    void const* serializedTextBlock = m_tableCell->GetBinaryTextBlock(&numBytes);

    if (nullptr != serializedTextBlock)
        {
        DgnDbR                  dgnDb = m_tableCell->GetTable().GetDgnDb();
        AnnotationTextBlockPtr  newTextBlock = AnnotationTextBlock::Create(dgnDb);

        AnnotationTextBlockPersistence::DecodeFromFlatBuf(*newTextBlock, static_cast<ByteCP> (serializedTextBlock), numBytes);
        SetTextBlockDirect (newTextBlock.get());
        }

    m_textBlockState = TEXTBLOCK_STATE_Initialized;

    return m_textBlock.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTextBlockP  TextBlockHolder::GetTextBlockForFieldProcessing()
    {
#if defined (NEEDSWORK)
    if (TEXTBLOCK_STATE_Uninitialized != m_textBlockState)
        {
        return m_textBlock.IsValid() && m_textBlock->ContainsFields() ? m_textBlock.get() : nullptr;
        }
    else
        {
        Utf8CP xml = m_tableCell->GetInstanceHolder().GetUtf8CP (TEXTTABLE_CELL_PROP_TextBlock);
        if (nullptr != xml && TextBlockXmlDeserializer::XmlContainsFields (xml))
            return const_cast<TextBlockP>(GetTextBlock());
        else
            return nullptr;
        }
#else
    BeAssert (false);
    return nullptr;
#endif
    }

#if defined (NEEDSWORK)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextBlockHolder::ReconcileJustificationForNewTextBlock ()
    {
    bool                        isFirstParagraph = true;
    bool                        foundMix = false;
    TextElementJustification    foundJust = TextElementJustification::Invalid;

    // Get the justification from the first paragraph
    ParagraphRange paragraphRange (*m_textBlock);
    for (ParagraphCR paragraph: paragraphRange)
        {
        ParagraphPropertiesCR paraProps = paragraph.GetProperties();

        if (isFirstParagraph)
            {
            foundJust        = paraProps.GetJustification();
            isFirstParagraph = false;
            continue;
            }

        if (paraProps.GetJustification() != foundJust)
            {
            foundMix = true;
            break;
            }
        }

    if (TextElementJustification::Invalid == foundJust)
        return;

    TableCellAlignment          alignFromJust = AnnotationTableCell::ToTableCellAlignment (foundJust);
    TextElementJustification    normalizeJust = AnnotationTableCell::ToTextElementJustification (alignFromJust);

    // Always set the justification even if there's no change.  It will turn on the override flag.
    m_textBlock->SetJustification (normalizeJust, paragraphRange);

    // Perform layout is only needed if something changed.
    if (foundMix || normalizeJust != foundJust)
        m_textBlock->PerformLayout ();

    // Set the cell property to match the text block.
    m_tableCell->SetAlignmentDirect (alignFromJust);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextBlockHolder::ReconcileCellPropertiesFromNewTextBlock ()
    {
    // Orientation
    bool newVertical = m_textBlock->GetProperties().IsVertical();
    bool oldVertical = TableCellOrientation::Vertical == m_tableCell->GetOrientation();

    if (newVertical != oldVertical)
        {
        TableCellOrientation    newOrientation = newVertical ? TableCellOrientation::Vertical : TableCellOrientation::Horizontal;

        // Set the cell property to match the text block.
        m_tableCell->SetOrientationDirect (newOrientation);
        }

    // Justification
    ReconcileJustificationForNewTextBlock ();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextBlockHolder::SetTextBlockDirect(AnnotationTextBlockP textBlock) const
    {
    m_textBlock         = textBlock;
    m_textBlockLayout   = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextBlockHolder::SetTextBlock (AnnotationTextBlockCR textBlock)
    {
    AnnotationTextBlockPtr  newTextBlock = textBlock.Clone();
    SetTextBlockDirect (newTextBlock.get());

#if defined (NEEDSWORK) // probably still needed
    ReconcileCellPropertiesFromNewTextBlock();
#endif

#if defined (NEEDSWORK) // probably not needed
    m_textBlock->SetUserOrigin (DPoint3d::FromZero());
    m_textBlock->SetOrientation (RotMatrix::FromIdentity());
    m_textBlock->RemoveAlongElementDependency();

    TextBlockPropertiesPtr  props = m_textBlock->GetProperties().Clone();

    if (m_tableCell->GetAnnotationTable().HasAnnotationScale())
        props->SetAnnotationScale (m_tableCell->GetAnnotationTable().GetAnnotationScale());
    else
        props->ClearAnnotationScale ();

    m_textBlock->SetProperties (*props);
#endif

    m_textBlockState = TEXTBLOCK_STATE_Changed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool            TextBlockHolder::_IsEmpty () const
    {
    if (TEXTBLOCK_STATE_Uninitialized != m_textBlockState)
        return m_textBlock.IsNull() ||  m_textBlock->IsEmpty();

    void const* serializedTextBlock = m_tableCell->GetBinaryTextBlock (nullptr);

    if (nullptr == serializedTextBlock)
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
DVec2d          TextBlockHolder::_GetSize () const
    {
    DVec2d   size;
    double   textWidth, textHeight;

    GetPaddedSizeAlignedWithTextBlock (textWidth, textHeight);

    switch (m_tableCell->GetOrientation())
        {
        default:
            {
            size.x = textWidth;
            size.y = textHeight;

            break;
            }
        case TableCellOrientation::Rotate90:
        case TableCellOrientation::Rotate270:
            {
            size.x = textHeight;
            size.y = textWidth;

            break;
            }
        }

    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
double            TextBlockHolder::GetMinimumWordWrapLength () const
    {
    AnnotationTextBlockCP textBlock = GetTextBlock();

    if (NULL == textBlock)
        return 0;

    return getMinimumWordWrapLength (*textBlock);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
double            TextBlockHolder::_GetFullyCompressedHeight () const
    {
    if (TableCellOrientation::Horizontal == m_tableCell->GetOrientation())
        return _GetSize ().y;

    return GetMinimumWordWrapLength ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
double            TextBlockHolder::_GetFullyCompressedWidth () const
    {
    if (TableCellOrientation::Horizontal != m_tableCell->GetOrientation())
        return _GetSize ().x;

    return GetMinimumWordWrapLength ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
double            TextBlockHolder::GetNonWrappedLength () const
    {
    AnnotationTextBlockCP textBlock = GetTextBlock();

    if (NULL == textBlock)
        return 0;

    return getNonWrappedLength (*textBlock);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
double            TextBlockHolder::_GetFullyExpandedHeight () const
    {
    if (TableCellOrientation::Horizontal == m_tableCell->GetOrientation())
        return _GetSize ().y;

    return GetNonWrappedLength ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
double            TextBlockHolder::_GetFullyExpandedWidth () const
    {
    if (TableCellOrientation::Horizontal != m_tableCell->GetOrientation())
        return _GetSize ().x;

    return GetNonWrappedLength ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextBlockHolder::_SetAlignment (TableCellAlignment newAlignment)
    {
    AnnotationTextBlockCP   textBlock   = GetTextBlock();

    if (NULL == textBlock)
        return;

    AnnotationTextBlockPtr                          copyTextBlock = textBlock->Clone();
    AnnotationTextBlock::HorizontalJustification    newJust = AnnotationTableCell::ToTextBlockJustification (newAlignment);

    copyTextBlock->SetJustification (newJust);
    SetTextBlock(*copyTextBlock);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextBlockHolder::_SetOrientation (TableCellOrientation newOrientation)
    {
    AnnotationTextBlockCP   textBlock   = GetTextBlock();

    if (NULL == textBlock)
        return;

#if defined (NEEDSWORK)
    AnnotationTextBlockPtr  copyTextBlock = textBlock->Clone();
    bool                    isVertical = (TableCellOrientation::Vertical == newOrientation);

    copyTextBlock->SetIsVertical (isVertical);
    SetTextBlock(*copyTextBlock);
#else
    BeAssert(false); // text block does not support vertical
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextBlockHolder::SetWordWrapLength (double length)
    {
    AnnotationTextBlockCP   textBlock   = GetTextBlock();

    if (NULL == textBlock)
        return;

    AnnotationTextBlockPtr  copyTextBlock = textBlock->Clone();
    setWordWrapLength (*copyTextBlock, length);

    SetTextBlock(*copyTextBlock);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextBlockHolder::_FitContentToHeight (double height)
    {
    if (TableCellOrientation::Horizontal == m_tableCell->GetOrientation())
        return;

    uint32_t            rowIndex = m_tableCell->GetIndex().row;
    AnnotationTableRowP row      = m_tableCell->GetTable().GetRow (rowIndex);

    // If the height is not locked remove the word wrap.
    if ( ! row->GetHeightLock())
        height = 0.0;

    SetWordWrapLength (height);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextBlockHolder::_FitContentToWidth (double width)
    {
    if (TableCellOrientation::Horizontal != m_tableCell->GetOrientation())
        return;

    uint32_t                colIndex = m_tableCell->GetIndex().col;
    AnnotationTableColumnP  col      = m_tableCell->GetTable().GetColumn (colIndex);

    // If the width is not locked remove the word wrap.
    if ( ! col->GetWidthLock())
        width = 0.0;

    SetWordWrapLength (width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
double          TextBlockHolder::ComputeDescenderAdjustment (AnnotationTextStyleCR textStyle)
    {
    bool    isTextStyleVertical = false;

#if defined (NEEDSWORK)
    textStyle.GetProperty (TextStyle_Vertical, isTextStyleVertical);
#endif

    if (isTextStyleVertical)
        return 0.0;

    double      height = textStyle.GetHeight();
    DgnFontId   fontId = textStyle.GetFontId();
    DgnFontCP   font   = textStyle.GetDgnDb().Fonts().FindFontById(fontId);

    if (UNEXPECTED_CONDITION (nullptr == font))
        return 0.0;

#if defined (NEEDSWORK)
    return font->GetDescenderRatio() * height;
#else
    return 0.3 * height;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
double          TextBlockHolder::ComputeDescenderAdjustment (AnnotationTextBlockLayoutCR textBlockLayout)
    {
#if defined (NEEDSWORK)
    if (textBlock.GetProperties().IsVertical())
        return 0.0;
#endif

    /*-------------------------------------------------------------------------
        Iterate the runs on the last line only.  Find the one with the
        largest descender.  We will use this to pad the bottom of the text
        block so that the descenders don't stick into the real margins.  We
        also pad the top so that the text looks balanced in the table cell.
    -------------------------------------------------------------------------*/
    AnnotationLayoutLineCollectionCR    lines = textBlockLayout.GetLines();
    AnnotationLayoutLinePtr const&      lastLine = lines.back();
    double                              descenderHeight = 0.0;
    double                              maxPositiveUnitOffset = 0.0;

    for (AnnotationLayoutRunPtr const& run : lastLine->GetRuns())
        {
        AnnotationTextStyleCPtr textStyle   = run->GetSeedRun().CreateEffectiveStyle ();
        double                  fontSize    = textStyle->GetHeight();
#if defined (NEEDSWORK)
        DgnFontCP               font        = dgnDb.Fonts().FindFontById(fontId);
        DgnFontId               fontId      = textStyle->GetFontId();
        double                  height      = fontSize * font.GetDescenderRatio();
#else
        double                  height      = fontSize * 0.3;
#endif

        if (descenderHeight < height)
            descenderHeight = height;

#if defined (NEEDSWORK)
        // DgnDb note...
        // Maybe we can remove this whole unit offset check.  It only exists for the case where a
        // fraction extends below a (potential) descender.  As of now we don't support 'shifted'
        // fractions, meaning the fraction must always sit on the baseline... so no fraction can
        // ever extend below the descenders
        if (maxPositiveUnitOffset < run->GetOffsetFromLine().y)
            maxPositiveUnitOffset = run->GetOffsetFromLine().y;
#endif
        }

    // Layout range already includes MaxUnitOffset.  If there are fractions,
    // we only want the part of the descender that extends past the fractions.
    return MAX (0.0, descenderHeight - maxPositiveUnitOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            TextBlockHolder::GetPaddedSizeAlignedWithTextBlock (double& width, double& height) const
    {
    AnnotationTextBlockCP textBlock = GetTextBlock();

    if (NULL == textBlock || textBlock->IsEmpty())
        {
        AnnotationTableRegion   region = m_tableCell->GetTableRegion();
        AnnotationTextStyleCP   textStyle = m_tableCell->GetTable().GetTextStyle (region);

        height = textStyle->GetHeight();
        width  = height * textStyle->GetWidthFactor();

        height += ComputeDescenderAdjustment (*textStyle);

        return;
        }

    AnnotationTextBlockLayoutCP layout = GetTextBlockLayout();

    DRange2dCR  range = layout->GetLayoutRange();
    double      descenderAdjustment = ComputeDescenderAdjustment (*layout);

    width  = range.high.x - range.low.x;
    height = range.high.y - range.low.y + 2*descenderAdjustment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  AnnotationTableCell::AnnotationTableCell (AnnotationTableElementR table, AnnotationTableCellIndex index)
    :
    AnnotationTableAspect (table), m_index (index), m_rawTextBlock(nullptr)
    {
    InitContentsToEmptyTextBlock();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  AnnotationTableCell::AnnotationTableCell (AnnotationTableCellCR rhs) : AnnotationTableAspect (rhs), m_rawTextBlock (nullptr)
    {
    CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableCellR AnnotationTableCell::operator= (AnnotationTableCellCR rhs)
    {
    CopyDataFrom (rhs);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableCell::ClearBinaryTextBlock ()
    {
    if (nullptr != m_rawTextBlock)
        free ((void*)m_rawTextBlock);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void  const* AnnotationTableCell::GetBinaryTextBlock (size_t* numBytes)
    {
    if (nullptr != numBytes)
        *numBytes = m_rawTextBlockBytes;

    return m_rawTextBlock;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableCell::AssignBinaryTextBlock (void const* data, size_t numBytes)
    {
    ClearBinaryTextBlock();

    if (nullptr == data)
        return;

    m_rawTextBlockBytes = numBytes;

    void * local = malloc (numBytes);
    memcpy (local, data, numBytes);
    m_rawTextBlock = (const Byte *)local;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableCell::_CopyDataFrom (AnnotationTableAspectCR rhsAspect)
    {
    AnnotationTableCellCR   rhs = static_cast <AnnotationTableCellCR> (rhsAspect);

    m_index             = rhs.m_index;

    AssignBinaryTextBlock (m_rawTextBlock, m_rawTextBlockBytes);

    m_fillKey           = rhs.m_fillKey;
    m_alignment         = rhs.m_alignment;
    m_orientation       = rhs.m_orientation;
    m_marginTop         = rhs.m_marginTop;
    m_marginBottom      = rhs.m_marginBottom;
    m_marginLeft        = rhs.m_marginLeft;
    m_marginRight       = rhs.m_marginRight;

    m_contentHolder     = rhs.m_contentHolder;

    if (nullptr != m_contentHolder)
        m_contentHolder->Initialize (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
PropertyNames   AnnotationTableCell::GetPropertyNames()
    {
    PropertyNames propNames =
        {
        { (int) AnnotationTableCell::PropIndex::CellIndex,      CELL_PARAM_Index         },
        { (int) AnnotationTableCell::PropIndex::TextBlock,      CELL_PARAM_TextBlock     },
        { (int) AnnotationTableCell::PropIndex::FillKey,        CELL_PARAM_FillKey       },
        { (int) AnnotationTableCell::PropIndex::Alignment,      CELL_PARAM_Alignment     },
        { (int) AnnotationTableCell::PropIndex::Orientation,    CELL_PARAM_Orientation   },
        { (int) AnnotationTableCell::PropIndex::MarginTop,      CELL_PARAM_MarginTop     },
        { (int) AnnotationTableCell::PropIndex::MarginBottom,   CELL_PARAM_MarginBottom  },
        { (int) AnnotationTableCell::PropIndex::MarginLeft,     CELL_PARAM_MarginLeft    },
        { (int) AnnotationTableCell::PropIndex::MarginRight,    CELL_PARAM_MarginRight   },
        };

    return propNames;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void AnnotationTableCell::_FlushChangesToProperties()
    {
    if (NULL == m_contentHolder)
        return;

    if (GetAlignment() == GetTable().GetDefaultCellAlignment())
        m_alignment.Clear();

    if (GetOrientation() == GetTable().GetDefaultCellOrientation())
        m_orientation.Clear();

    TableCellMarginValues   cellMargins     = GetMargins();
    TableCellMarginValues   defaultMargins  = GetTable().GetDefaultMargins();

    if (DoubleOps::WithinTolerance (defaultMargins.m_top, cellMargins.m_top, s_doubleTol))
        m_marginTop.Clear();

    if (DoubleOps::WithinTolerance (defaultMargins.m_bottom, cellMargins.m_bottom, s_doubleTol))
        m_marginBottom.Clear();

    if (DoubleOps::WithinTolerance (defaultMargins.m_left, cellMargins.m_left, s_doubleTol))
        m_marginLeft.Clear();

    if (DoubleOps::WithinTolerance (defaultMargins.m_right, cellMargins.m_right, s_doubleTol))
        m_marginRight.Clear();

    if ( ! m_fillKey.IsNull() && 0 == m_fillKey.GetValue())
        m_fillKey.Clear();

    m_contentHolder->_FlushChangesToProperties ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
bool    AnnotationTableCell::_ShouldBePersisted() const
    {
    if (nullptr == m_contentHolder)
        return false;

    if (nullptr != m_rawTextBlock)  return true;
    if (m_fillKey.IsValid())        return true;
    if (m_alignment.IsValid())      return true;
    if (m_orientation.IsValid())    return true;
    if (m_marginTop.IsValid())      return true;
    if (m_marginBottom.IsValid())   return true;
    if (m_marginLeft.IsValid())     return true;
    if (m_marginRight.IsValid())    return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
AnnotationTableCellIndex    AnnotationTableCell::GetCellIndex(ECSqlStatement& statement, uint32_t columnIndex)
    {
    if (statement.IsValueNull(columnIndex))
        return AnnotationTableCellIndex();

    AnnotationTableCellIndex    cellIndex;
    IECSqlStructValue const& cellIndexValue = statement.GetValueStruct(columnIndex);

    for (int iMember = 0; iMember < cellIndexValue.GetMemberCount(); iMember++)
        {
        IECSqlValue const& memberValue = cellIndexValue.GetValue(iMember);
        ECPropertyCP memberProperty = memberValue.GetColumnInfo().GetProperty();

        if (UNEXPECTED_CONDITION (memberProperty == nullptr))
            return cellIndex;

        Utf8CP memberName = memberProperty->GetName().c_str();

        if (0 == BeStringUtilities::Stricmp("RowIndex", memberName))
            cellIndex.row = memberValue.GetInt();
        else if (0 == BeStringUtilities::Stricmp("ColumnIndex", memberName))
            cellIndex.col = memberValue.GetInt();
        else
            BeAssert(false);
        }

    return cellIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableCell::BindCellIndex(ECSqlStatement& statement)
    {
    int cellIndex = statement.GetParameterIndex(CELL_PARAM_Index);
    IECSqlStructBinder& binder = statement.BindStruct(cellIndex);

    ECSqlStatus status;

    status = binder.GetMember("RowIndex").BindInt(m_index.row);
    BeAssert(status == ECSqlStatus::Success);

    status = binder.GetMember("ColumnIndex").BindInt(m_index.col);
    BeAssert(status == ECSqlStatus::Success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableCell::_BindProperties(ECSqlStatement& statement)
    {
    BindCellIndex (statement);

    if (nullptr != m_rawTextBlock)
        statement.BindBinary(statement.GetParameterIndex(CELL_PARAM_TextBlock), m_rawTextBlock, static_cast<int>(m_rawTextBlockBytes), IECSqlBinder::MakeCopy::Yes);
    else
        statement.BindNull(statement.GetParameterIndex(CELL_PARAM_TextBlock));

    BindUInt   (statement, CELL_PARAM_FillKey,      m_fillKey);
    BindUInt   (statement, CELL_PARAM_Alignment,    m_alignment);
    BindUInt   (statement, CELL_PARAM_Orientation,  m_orientation);
    BindDouble (statement, CELL_PARAM_MarginTop,    m_marginTop);
    BindDouble (statement, CELL_PARAM_MarginBottom, m_marginBottom);
    BindDouble (statement, CELL_PARAM_MarginLeft,   m_marginLeft);
    BindDouble (statement, CELL_PARAM_MarginRight,  m_marginRight);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void  AnnotationTableCell::_AssignValue (int index, IECSqlValue const& value)
    {
    PropIndex propIndex = static_cast <PropIndex> (index);

    switch (propIndex)
        {
        case PropIndex::TextBlock:
            {
            int         size;
            void const* data = value.GetBinary (&size);
            AssignBinaryTextBlock (data, size);
            break;
            }
        case PropIndex::FillKey:        m_fillKey.SetValue      (value.GetInt());       break;
        case PropIndex::Alignment:      m_alignment.SetValue    (value.GetInt());       break;
        case PropIndex::Orientation:    m_orientation.SetValue  (value.GetInt());       break;
        case PropIndex::MarginTop:      m_marginTop.SetValue    (value.GetDouble());    break;
        case PropIndex::MarginBottom:   m_marginBottom.SetValue (value.GetDouble());    break;
        case PropIndex::MarginLeft:     m_marginLeft.SetValue   (value.GetDouble());    break;
        case PropIndex::MarginRight:    m_marginRight.SetValue  (value.GetDouble());    break;
        default:                        BeAssert (false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableCell::SetIndex (AnnotationTableCellIndexCR val)
    {
    m_index = val;
    SetHasChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableRegion AnnotationTableCell::GetTableRegion () const
    {
    return GetTable().GetTableRegion (m_index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTextBlock::HorizontalJustification AnnotationTableCell::ToTextBlockJustification (TableCellAlignment cellAlignment)
    {
    switch (cellAlignment)
        {
        default:
        case TableCellAlignment::LeftTop:
        case TableCellAlignment::LeftMiddle:
        case TableCellAlignment::LeftBottom:    return AnnotationTextBlock::HorizontalJustification::Left;
        case TableCellAlignment::CenterTop:
        case TableCellAlignment::CenterMiddle:
        case TableCellAlignment::CenterBottom:  return AnnotationTextBlock::HorizontalJustification::Center;
        case TableCellAlignment::RightTop:
        case TableCellAlignment::RightMiddle:
        case TableCellAlignment::RightBottom:   return AnnotationTextBlock::HorizontalJustification::Right;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void        AnnotationTableCell::SetAlignmentDirect (TableCellAlignment alignment)
    {
    m_alignment.SetValue (static_cast<uint32_t>(alignment));
    SetHasChanges(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void        AnnotationTableCell::SetAlignment(TableCellAlignment alignment)
    {
    if (EXPECTED_CONDITION (NULL != m_contentHolder))
        m_contentHolder->_SetAlignment (alignment);

    SetAlignmentDirect (alignment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/13
+---------------+---------------+---------------+---------------+---------------+------*/
TableCellAlignment      AnnotationTableCell::GetAlignment() const
    {
    return m_alignment.IsValid() ? static_cast<TableCellAlignment> (m_alignment.GetValue()) : GetTable().GetDefaultCellAlignment();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void        AnnotationTableCell::SetOrientationDirect (TableCellOrientation orientation)
    {
    m_orientation.SetValue (static_cast<uint32_t>(orientation));
    SetHasChanges(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void        AnnotationTableCell::SetOrientation (TableCellOrientation orientation)
    {
#if defined (NEEDSWORK)
    DVec2d      oldContentSize  = GetContentSize();

    if (EXPECTED_CONDITION (NULL != m_contentHolder))
        m_contentHolder->_SetOrientation (orientation);
#endif
    SetOrientationDirect (orientation);
#if defined (NEEDSWORK)
    SetSizeFromContents (&oldContentSize);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
TableCellOrientation      AnnotationTableCell::GetOrientation() const
    {
    return m_orientation.IsValid() ? static_cast<TableCellOrientation> (m_orientation.GetValue()) : GetTable().GetDefaultCellOrientation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void        AnnotationTableCell::SetMarginsDirect (TableCellMarginValuesCR newValues)
    {
    m_marginTop.SetValue (newValues.m_top);
    m_marginBottom.SetValue (newValues.m_bottom);
    m_marginLeft.SetValue (newValues.m_left);
    m_marginRight.SetValue (newValues.m_right);
    SetHasChanges(); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void        AnnotationTableCell::SetMargins (TableCellMarginValuesCR newValues)
    {
#if defined (NEEDSWORK)
    DVec2d      oldContentSize  = GetContentSize();
#endif

    SetMarginsDirect(newValues);
#if defined (NEEDSWORK)
    SetSizeFromContents(&oldContentSize);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void     setDoubleIfNotNull (double& out, TableDoubleValue const& val)
    {
    if ( ! val.IsNull())
        out = val.GetValue();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
TableCellMarginValues   AnnotationTableCell::GetMargins () const
    {
    TableCellMarginValues   margins = GetTable().GetDefaultMargins();

    setDoubleIfNotNull (margins.m_top,    m_marginTop);
    setDoubleIfNotNull (margins.m_bottom, m_marginBottom);
    setDoubleIfNotNull (margins.m_left,   m_marginLeft);
    setDoubleIfNotNull (margins.m_right,  m_marginRight);

    return margins;
    }

#if defined (NEEDSWORK)
/*-------------------------------------------------------------------------------------*
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DVec2d          AnnotationTableCell::GetTextSize () const
    {
    TextBlockCP     textBlock;
    RunPropertiesCP runProps;
    DVec2d          textSize;

    if (NULL != (textBlock = GetTextBlock()) &&
        NULL != (runProps  = textBlock->GetFirstRunProperties()))
        {
        textSize = DVec2d::From(runProps->GetFontSize());
        }
    else
        {
        DgnModelP       model     = m_table->GetModel();
        AnnotationTableRegion region    = GetTableRegion();
        DgnTextStyleCP  textStyle = m_table->GetTextStyle (region);

        // TextStyle already has the table's annotation scale in it.
        textSize.x = textStyle->GetWidth(model);
        textSize.y = textStyle->GetHeight(model);
        }

    return textSize;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DVec2d          AnnotationTableCell::GetSize () const
    {
    DVec2d      size = DVec2d::From (0.0, 0.0);
    uint32_t    rowSpan = GetRowSpan();
    uint32_t    colSpan = GetColumnSpan();

    for (uint32_t iRow = m_index.row; iRow < m_index.row + rowSpan; iRow++)
        {
        AnnotationTableRowCP row = GetTable().GetRow (iRow);

        size.y += row->GetHeight();
        }

    for (uint32_t iCol = m_index.col; iCol < m_index.col + colSpan; iCol++)
        {
        AnnotationTableColumnCP col = GetTable().GetColumn (iCol);

        size.x += col->GetWidth();
        }

    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DVec2d          AnnotationTableCell::GetAvailableContentSize () const
    {
    DVec2d                  size    = GetSize();
    TableCellMarginValues   margins = GetMargins();

    size.x -= (margins.m_left + margins.m_right);
    size.y -= (margins.m_top + margins.m_bottom);

    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::FitContentToHeight (double height)
    {
    if (UNEXPECTED_CONDITION (NULL == m_contentHolder))
        return;

    m_contentHolder->_FitContentToHeight (height);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::FitContentToWidth (double width)
    {
    if (UNEXPECTED_CONDITION (NULL == m_contentHolder))
        return;

    m_contentHolder->_FitContentToWidth (width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
DVec2d          AnnotationTableElement::GetEmptyMinimumSizeWithoutMargins (AnnotationTableRegion region) const
    {
    AnnotationTextStyleCP   textStyle = GetTextStyle (region);
    DVec2d                  size;

    size.y = textStyle->GetHeight ();
    size.x = size.y * textStyle->GetWidthFactor ();

    size.y += 2 * TextBlockHolder::ComputeDescenderAdjustment (*textStyle);

    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
DVec2d          AnnotationTableCell::GetEmptyMinimumSizeWithoutMargins () const
    {
    return GetTable().GetEmptyMinimumSizeWithoutMargins (GetTableRegion());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
double          AnnotationTableCell::GetFullyCompressedContentHeight () const
    {
    TableCellMarginValues   margins  = GetMargins();
    double                  minContentHeight;

    if ( ! IsEmpty())
        minContentHeight = m_contentHolder->_GetFullyCompressedHeight();
    else
        minContentHeight = GetEmptyMinimumSizeWithoutMargins().y;

    return minContentHeight + margins.m_top + margins.m_bottom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
double          AnnotationTableCell::GetFullyCompressedContentWidth () const
    {
    TableCellMarginValues   margins  = GetMargins();
    double                  minContentWidth;

    if ( ! IsEmpty())
        minContentWidth = m_contentHolder->_GetFullyCompressedWidth();
    else
        minContentWidth = GetEmptyMinimumSizeWithoutMargins().x;

    return minContentWidth + margins.m_left + margins.m_right;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
double          AnnotationTableCell::GetFullyExpandedContentHeight () const
    {
    TableCellMarginValues   margins  = GetMargins();
    double                  expandedHeight;

    if ( ! IsEmpty())
        expandedHeight = m_contentHolder->_GetFullyExpandedHeight();
    else
        expandedHeight = GetEmptyMinimumSizeWithoutMargins().y;

    return expandedHeight + margins.m_top + margins.m_bottom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
double          AnnotationTableCell::GetFullyExpandedContentWidth () const
    {
    TableCellMarginValues   margins  = GetMargins();
    double                  expandedWidth;

    if ( ! IsEmpty())
        expandedWidth = m_contentHolder->_GetFullyExpandedWidth();
    else
        expandedWidth = GetEmptyMinimumSizeWithoutMargins().x;

    return expandedWidth + margins.m_left + margins.m_right;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DVec2d          AnnotationTableCell::GetContentSize () const
    {
    TableCellMarginValues   margins = GetMargins();
    DVec2d                  size;

    if ( ! IsEmpty())
        size = m_contentHolder->_GetSize ();
    else
        size = GetEmptyMinimumSizeWithoutMargins ();

    size.x += margins.m_left + margins.m_right;
    size.y += margins.m_top  + margins.m_bottom;

    return size;
    }

#if defined (NEEDSWORK)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d        AnnotationTableCell::GetContentOrigin (DPoint3dCR cellOrigin, DVec3dCR xCellVec, DVec3dCR yCellVec) const
    {
    DVec2d                  cellSize = GetSize();
    DVec2d                  contentBox;
    DVec3d                  xContentVec, yContentVec;
    double                  topContentMargin, bottomContentMargin;
    double                  leftContentMargin, rightContentMargin;
    DPoint3d                contentOrigin;
    TableCellMarginValues   marginValues = GetMargins();

    switch (GetOrientation())
        {
        default:
            {
            //  +-----------+
            //  | oo>       |
            //  |           |
            //  |           |
            //  |           |
            //  +-----------+

            contentBox          = cellSize;
            contentOrigin       = cellOrigin;
            xContentVec         = xCellVec;
            yContentVec         = yCellVec;
            topContentMargin    = marginValues.m_top;
            bottomContentMargin = marginValues.m_bottom;
            leftContentMargin   = marginValues.m_left;
            rightContentMargin  = marginValues.m_right;

            break;
            }
        case TableCellOrientation::Rotate90:
            {
            //  +-----------+
            //  |         o |
            //  |         o |
            //  |         v |
            //  |           |
            //  +-----------+

            contentBox.Init (cellSize.y, cellSize.x);
            contentOrigin.SumOf (cellOrigin, xCellVec, cellSize.x);
            xContentVec.Init   (yCellVec);
            yContentVec.Negate (xCellVec);
            topContentMargin    = marginValues.m_right;
            bottomContentMargin = marginValues.m_left;
            leftContentMargin   = marginValues.m_top;
            rightContentMargin  = marginValues.m_bottom;

            break;
            }
        case TableCellOrientation::Rotate270:
            {
            //  +-----------+
            //  |           |
            //  | ^         |
            //  | o         |
            //  | o         |
            //  +-----------+

            contentBox.Init (cellSize.y, cellSize.x);
            contentOrigin.SumOf (cellOrigin, yCellVec, cellSize.y);
            xContentVec.Negate (yCellVec);
            yContentVec.Init   (xCellVec);
            topContentMargin    = marginValues.m_left;
            bottomContentMargin = marginValues.m_right;
            leftContentMargin   = marginValues.m_bottom;
            rightContentMargin  = marginValues.m_top;

            break;
            }
        }

    // ContentOrigin is now at the (text POV) upper-left corner of the cell.
    // Adjust it to the upper-left corner of the usable area.
    contentOrigin.SumOf (contentOrigin, xContentVec, leftContentMargin);
    contentOrigin.SumOf (contentOrigin, yContentVec, topContentMargin);

    // ContentBox is now the full size of the cell.
    // Shrink it to be the size of the usable area.
    contentBox.x -= leftContentMargin + rightContentMargin;
    contentBox.y -= topContentMargin + bottomContentMargin;

    double              xAdjust = 0;
    double              yAdjust = 0;
    TableCellAlignment  alignment = GetAlignment();
    HorizontalAlignment hAlign = ToHorizontalAlignment (alignment);
    VerticalAlignment   vAlign = ToVerticalAlignment (alignment);

    switch (hAlign)
        {
        case HorizontalAlignment::Left:   xAdjust += 0;                     break;
        case HorizontalAlignment::Center: xAdjust += contentBox.x / 2.0;    break;
        case HorizontalAlignment::Right:  xAdjust += contentBox.x;          break;
        }

    switch (vAlign)
        {
        case VerticalAlignment::Top:      yAdjust += 0;                     break;
        case VerticalAlignment::Middle:   yAdjust += contentBox.y / 2.0;    break;
        case VerticalAlignment::Bottom:   yAdjust += contentBox.y;          break;
        }

    contentOrigin.SumOf (contentOrigin, xContentVec, xAdjust);
    contentOrigin.SumOf (contentOrigin, yContentVec, yAdjust);

    return contentOrigin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d        AnnotationTableCell::ComputeContentOrigin () const
    {
    RotMatrix   rMatrix = m_table->GetRotation();
    DVec3d      xVec, yVec;

    rMatrix.GetColumn (xVec, 0);
    rMatrix.GetColumn (yVec, 1);
    yVec.Negate();

    DPoint3d    cellOrigin = ComputeOrigin();

    return GetContentOrigin (cellOrigin, xVec, yVec);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
RotMatrix       AnnotationTableCell::GetContentRotation () const
    {
    RotMatrix   tableRotation   = m_table->GetRotation();
    RotMatrix   contentRotation;

    switch (GetOrientation())
        {
        default:
            {
            contentRotation = tableRotation;
            break;
            }
        case TableCellOrientation::Rotate90:
            {
            RotMatrix   adjustment = RotMatrix::FromIdentity();
            DVec3d      xVec, yVec, zVec;

            adjustment.GetColumns (xVec, yVec, zVec);
            yVec.Negate();

            adjustment = RotMatrix::FromColumnVectors (yVec, xVec, zVec);
            contentRotation.InitProduct (adjustment, tableRotation);

            break;
            }
        case TableCellOrientation::Rotate270:
            {
            RotMatrix   adjustment = RotMatrix::FromIdentity();
            DVec3d      xVec, yVec, zVec;

            adjustment.GetColumns (xVec, yVec, zVec);
            xVec.Negate();

            adjustment = RotMatrix::FromColumnVectors (yVec, xVec, zVec);
            contentRotation.InitProduct (adjustment, tableRotation);

            break;
            }
        }

    return contentRotation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AnnotationTableCell::ComputeOrigin (DPoint3dR cellOrigin, SubTable const& subTable, DVec3dCR xVec, DVec3dCR yVec) const
    {
    cellOrigin = subTable.m_origin;

    bool foundIt = false;

    for (TextTableRowCP const& row : subTable.m_rows)
        {
        if (row->GetIndex() == m_index.row)
            {
            foundIt = true;
            break;
            }

        cellOrigin.SumOf (cellOrigin, yVec, row->GetHeight());
        }

    if ( ! foundIt)
        return ERROR;

    foundIt = false;

    for (TextTableColumnCP const& col : subTable.m_columns)
        {
        if (col->GetIndex() == m_index.col)
            {
            foundIt = true;
            break;
            }

        cellOrigin.SumOf (cellOrigin, xVec, col->GetWidth());
        }

    return foundIt ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d        AnnotationTableCell::ComputeOrigin () const
    {
    RotMatrix   rMatrix = m_table->GetRotation();
    DVec3d      xVec, yVec;

    rMatrix.GetColumn (xVec, 0);
    rMatrix.GetColumn (yVec, 1);
    yVec.Negate();

    SubTables subTables;

    m_table->LayoutSubTables (subTables);

    for (SubTable const& subTable : subTables)
        {
        DPoint3d    cellOrigin;

        if (SUCCESS == ComputeOrigin (cellOrigin, subTable, xVec, yVec))
            return cellOrigin;
        }

    BeAssert (false);
    return m_table->GetOrigin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::ApplyTextStyleByRegion ()
    {
    TextBlockCP textBlock = GetTextBlock();

    if (NULL == textBlock)
        return;

    AnnotationTableRegion     region      = GetTableRegion();
    DgnTextStylePtr     textStyle   = m_table->GetTextStyle (region)->Copy();

    /*---------------------------------------------------------------------
        Each cell gets to specify its own justification.
    ---------------------------------------------------------------------*/
    TableCellAlignment          alignment     = GetAlignment();
    TextElementJustification    justification = ToTextElementJustification (alignment);
    textStyle->SetProperty (TextStyle_Justification, static_cast <UInt32> (justification));

    TextBlockPtr copyTextBlock = textBlock->Clone();
    copyTextBlock->ApplyTextStyle(*textStyle, false, copyTextBlock->Begin(), copyTextBlock->End());
    copyTextBlock->PerformLayout();

    SetTextBlock(*copyTextBlock);
    }

#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AnnotationTableCell::IsEmpty () const
    {
    return (NULL != m_contentHolder) ? m_contentHolder->_IsEmpty() : true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTextBlockCP     AnnotationTableCell::GetTextBlock () const
    {
    if (NULL == m_contentHolder)
        return NULL;

    TextBlockHolder* holder = m_contentHolder->_AsTextBlockHolder();

    if (NULL == holder)
        return NULL;

    return holder->GetTextBlock();
    }

#if defined (NEEDSWORK)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/14
+---------------+---------------+---------------+---------------+---------------+------*/
TextBlockP      AnnotationTableCell::GetTextBlockForFieldProcessing()
    {
    TextBlockHolder* holder = nullptr != m_contentHolder ? m_contentHolder->_AsTextBlockHolder() : nullptr;
    return nullptr != holder ? holder->GetTextBlockForFieldProcessing() : nullptr;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::SetSizeFromContents (DVec2dCP oldContentSize)
    {
    // Will only affect the last row and column in the cell's span
    AnnotationTableCellIndex    cellIndex   = GetIndex();
    AnnotationTableRowP         row         = GetTable().GetRow    (cellIndex.row + GetRowSpan() - 1);
    AnnotationTableColumnP      column      = GetTable().GetColumn (cellIndex.col + GetColumnSpan() - 1);
    double                      rowHeight   = row->GetHeight();
    double                      colWidth    = column->GetWidth();

    DVec2d                      currSize    = GetSize();
    double                      heightSuppliedByOtherRows   = currSize.y - rowHeight;
    double                      widthSuppliedByOtherColumns = currSize.x - colWidth;

    DVec2d                      neededSize      = GetContentSize();
    double                      rowNeededHeight = neededSize.y - heightSuppliedByOtherRows;
    double                      colNeededWidth  = neededSize.x - widthSuppliedByOtherColumns;

    /// Height
    if (rowHeight < rowNeededHeight)
        {
        row->SetHeight (rowNeededHeight, SizeLockAction::TurnOff);
        }
    else
    if ( ! row->GetHeightLock() && rowHeight > rowNeededHeight)
        {
        if (nullptr == oldContentSize || oldContentSize->y > currSize.y)
            row->SetHeightFromContents();
        }

    /// Width
    if (colWidth < colNeededWidth)
        {
        column->SetWidth (colNeededWidth, SizeLockAction::TurnOff);
        }
    else
    if ( ! column->GetWidthLock() && colWidth > colNeededWidth)
        {
        if (nullptr == oldContentSize || oldContentSize->x > currSize.x)
            column->SetWidthFromContents(false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableCell::InitContentsToEmptyTextBlock ()
    {
    m_contentHolder = CellContentHolderPtr (new TextBlockHolder (*this));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::SetTextBlockDirect (AnnotationTextBlockCR textBlock)
    {
    TextBlockHolder* holder = m_contentHolder->_AsTextBlockHolder();

    if (NULL == holder)
        {
        BeAssert (false); // needs work, if there is a holder, switch it to a TextBlockHolder

        // Needs to call this, but also clean up instance data from the other holder
        //InitContentsToEmptyTextBlock();

        return;
        }

    holder->SetTextBlock (textBlock);
    SetHasChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::SetTextBlock (AnnotationTextBlockCR textBlock)
    {
    DVec2d      oldContentSize  = GetContentSize();

    SetTextBlockDirect (textBlock);
    SetSizeFromContents (&oldContentSize);
    }

#if defined (NEEDSWORK)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::SetTextString (WCharCP newString)
    {
    TextBlockPtr    textBlock   = CreateEmptyTextBlock();

    if (NULL != newString && L'\0' != *newString)
        textBlock->AppendText (newString);

    SetTextBlock (*textBlock);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableCell::ClearContents ()
    {
    SetTextString (L"");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void   AnnotationTableCell::SetFillKey (UInt32 fillKey)   {                 m_instanceHolder.SetInteger (TEXTTABLE_CELL_PROP_FillKey, fillKey); }
void   AnnotationTableCell::ClearFillKey ()               {                 m_instanceHolder.SetToNull  (TEXTTABLE_CELL_PROP_FillKey); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void   AnnotationTableCell::ApplyToFillRuns ()
    {
    TextTableRowP   row             = m_table->GetRow (m_index.row);
    FillRunsR       fillRuns        = row->GetFillRuns();
    UInt32          verticalSpan    = GetRowSpan();
    ECValue         val             = m_instanceHolder.GetValue (TEXTTABLE_CELL_PROP_FillKey);

    if ( ! val.IsNull() && 0 == (UInt32) val.GetInteger())
        {
        fillRuns.CreateGap (NULL, m_index.col, GetColumnSpan());
        }
    else
        {
        UInt32  runKey = val.IsNull() ? 0 : (UInt32) val.GetInteger();

        fillRuns.ApplyRun (runKey, verticalSpan, m_index.col, GetColumnSpan());
        fillRuns.MergeRedundantRuns (m_table);
        }

    for (UInt32 iRow = 1; iRow < verticalSpan; iRow++)
        {
        UInt32          rowIndex        = m_index.row + iRow;
        TextTableRowP   spannedRow      = m_table->GetRow (rowIndex);
        FillRunsR       spannedFillRuns = spannedRow->GetFillRuns();

        spannedFillRuns.CreateGap (NULL, m_index.col, GetColumnSpan());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void   AnnotationTableCell::SetFillSymbology (TableSymbologyValuesCR symb)
    {
    if ( ! symb.HasFillVisible())
        {
        ClearFillKey();
        }
    else
    if ( ! symb.GetFillVisible())
        {
        SetFillKey(0);
        }
    else
        {
        if (UNEXPECTED_CONDITION ( ! symb.HasFillColor()))
            return;

        FillEntry      newFill;
        newFill.SetFillColor (symb.GetFillColor());

        UInt32 newKey = m_table->GetFillDictionary().FindOrAddFill (newFill);

        SetFillKey (newKey);
        }

    ApplyToFillRuns();
    }

#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        AnnotationTableCell::GetRowSpan () const
    {
#if defined (NEEDSWORK)
    MergeEntryCP entry = m_table->GetMergeDictionary().GetMerge (m_index);

    if (NULL == entry)
        return 1;

    return entry->GetRowSpan();
#else
    return 1;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        AnnotationTableCell::GetColumnSpan () const
    {
#if defined (NEEDSWORK)
    MergeEntryCP entry = m_table->GetMergeDictionary().GetMerge (m_index);

    if (NULL == entry)
        return 1;

    return entry->GetColumnSpan();
#else
    return 1;
#endif
    }

#if defined (NEEDSWORK)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool          AnnotationTableCell::IndexIsInSpan (TableCellIndexCR index) const
    {
    TableCellIndex  myIndex = GetIndex();

    if (myIndex.row > index.row)
        return false;

    if (myIndex.col > index.col)
        return false;

    if (myIndex.row + GetRowSpan() < index.row)
        return false;

    if (myIndex.col + GetColumnSpan() < index.col)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::SetAsMergedCellRoot (UInt32 rowSpan, UInt32 colSpan)
    {
    if (IsMergedCellInterior())
        InitContentsToEmptyTextBlock();

    MergeEntryP entry = m_table->GetMergeDictionary().GetMerge (m_index);

    if (NULL != entry)
        {
        entry->SetRowSpan (rowSpan);
        entry->SetColumnSpan (colSpan);
        return;
        }

    MergeEntry merge (m_index);

    merge.SetRowSpan (rowSpan);
    merge.SetColumnSpan (colSpan);

    EXPECTED_CONDITION (SUCCESS == m_table->GetMergeDictionary().AddMerge (merge));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::DeleteMergeCellInteriorRow ()
    {
    MergeEntryP merge = m_table->GetMergeDictionary().GetMerge (m_index);

    if (UNEXPECTED_CONDITION (NULL == merge))
        return;

    UInt        rowSpan = merge->GetRowSpan();

    if (UNEXPECTED_CONDITION (rowSpan <= 1))
        return;

    merge->SetRowSpan (rowSpan - 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::DeleteMergeCellInteriorColumn ()
    {
    MergeEntryP merge = m_table->GetMergeDictionary().GetMerge (m_index);

    if (UNEXPECTED_CONDITION (NULL == merge))
        return;

    UInt        colSpan = merge->GetColumnSpan();

    if (UNEXPECTED_CONDITION (colSpan <= 1))
        return;

    merge->SetColumnSpan (colSpan - 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AnnotationTableCell::IsMergedCellInterior () const
    {
    return NULL == m_contentHolder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::SetAsMergedCellInterior (bool isMerged)
    {
    if (isMerged)
        {
        m_contentHolder = NULL;
        return;
        }

    if (IsMergedCellInterior())
        InitContentsToEmptyTextBlock();
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
PropertyNames   TableHeaderAspect::GetPropertyNames()
    {
    PropertyNames propNames =
        {
        { (int) TableHeaderAspect::PropIndex::RowCount,                HEADER_PARAM_RowCount                  },
        { (int) TableHeaderAspect::PropIndex::ColumnCount,             HEADER_PARAM_ColumnCount               },
        { (int) TableHeaderAspect::PropIndex::TextStyleId,             HEADER_PARAM_TextStyleId               },
        { (int) TableHeaderAspect::PropIndex::TitleRowCount,           HEADER_PARAM_TitleRowCount             }, 
        { (int) TableHeaderAspect::PropIndex::HeaderRowCount,          HEADER_PARAM_HeaderRowCount            }, 
        { (int) TableHeaderAspect::PropIndex::FooterRowCount,          HEADER_PARAM_FooterRowCount            }, 
        { (int) TableHeaderAspect::PropIndex::HeaderColumnCount,       HEADER_PARAM_HeaderColumnCount         }, 
        { (int) TableHeaderAspect::PropIndex::FooterColumnCount,       HEADER_PARAM_FooterColumnCount         }, 
        { (int) TableHeaderAspect::PropIndex::BreakType,               HEADER_PARAM_BreakType                 }, 
        { (int) TableHeaderAspect::PropIndex::BreakPosition,           HEADER_PARAM_BreakPosition             }, 
        { (int) TableHeaderAspect::PropIndex::BreakLength,             HEADER_PARAM_BreakLength               }, 
        { (int) TableHeaderAspect::PropIndex::BreakGap,                HEADER_PARAM_BreakGap                  }, 
        { (int) TableHeaderAspect::PropIndex::RepeatHeaders,           HEADER_PARAM_RepeatHeaders             }, 
        { (int) TableHeaderAspect::PropIndex::RepeatFooters,           HEADER_PARAM_RepeatFooters             }, 
        { (int) TableHeaderAspect::PropIndex::DefaultColumnWidth,      HEADER_PARAM_DefaultColumnWidth        }, 
        { (int) TableHeaderAspect::PropIndex::DefaultRowHeight,        HEADER_PARAM_DefaultRowHeight          }, 
        { (int) TableHeaderAspect::PropIndex::DefaultMarginTop,        HEADER_PARAM_DefaultMarginTop          }, 
        { (int) TableHeaderAspect::PropIndex::DefaultMarginBottom,     HEADER_PARAM_DefaultMarginBottom       }, 
        { (int) TableHeaderAspect::PropIndex::DefaultMarginLeft,       HEADER_PARAM_DefaultMarginLeft         }, 
        { (int) TableHeaderAspect::PropIndex::DefaultMarginRight,      HEADER_PARAM_DefaultMarginRight        }, 
        { (int) TableHeaderAspect::PropIndex::DefaultCellAlignment,    HEADER_PARAM_DefaultCellAlignment      }, 
        { (int) TableHeaderAspect::PropIndex::DefaultCellOrientation,  HEADER_PARAM_DefaultCellOrientation    }, 
        { (int) TableHeaderAspect::PropIndex::FillSymbologyKeyOddRow,  HEADER_PARAM_FillSymbologyKeyOddRow    }, 
        { (int) TableHeaderAspect::PropIndex::FillSymbologyKeyEvenRow, HEADER_PARAM_FillSymbologyKeyEvenRow   }, 
        { (int) TableHeaderAspect::PropIndex::TitleRowTextStyle,       HEADER_PARAM_TitleRowTextStyle         }, 
        { (int) TableHeaderAspect::PropIndex::HeaderRowTextStyle,      HEADER_PARAM_HeaderRowTextStyle        }, 
        { (int) TableHeaderAspect::PropIndex::FooterRowTextStyle,      HEADER_PARAM_FooterRowTextStyle        }, 
        { (int) TableHeaderAspect::PropIndex::HeaderColumnTextStyle,   HEADER_PARAM_HeaderColumnTextStyle     }, 
        { (int) TableHeaderAspect::PropIndex::FooterColumnTextStyle,   HEADER_PARAM_FooterColumnTextStyle     }, 
        { (int) TableHeaderAspect::PropIndex::BackupTextHeight,        HEADER_PARAM_BackupTextHeight          }, 
        { (int) TableHeaderAspect::PropIndex::DataSourceProviderId,    HEADER_PARAM_DataSourceProviderId      }, 
        { (int) TableHeaderAspect::PropIndex::BodyTextHeight,          HEADER_PARAM_BodyTextHeight            }, 
        { (int) TableHeaderAspect::PropIndex::TitleRowTextHeight,      HEADER_PARAM_TitleRowTextHeight        }, 
        { (int) TableHeaderAspect::PropIndex::HeaderRowTextHeight,     HEADER_PARAM_HeaderRowTextHeight       }, 
        { (int) TableHeaderAspect::PropIndex::FooterRowTextHeight,     HEADER_PARAM_FooterRowTextHeight       }, 
        { (int) TableHeaderAspect::PropIndex::HeaderColumnTextHeight,  HEADER_PARAM_HeaderColumnTextHeight    }, 
        { (int) TableHeaderAspect::PropIndex::FooterColumnTextHeight,  HEADER_PARAM_FooterColumnTextHeight    }, 
        };

    return propNames;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void  TableHeaderAspect::_AssignValue (int index, IECSqlValue const& value)
    {
    PropIndex propIndex = static_cast <PropIndex> (index);

    switch (propIndex)
        {
        case    PropIndex::RowCount:                    m_rowCount.SetValue (value.GetInt());           break;
        case    PropIndex::ColumnCount:                 m_columnCount.SetValue (value.GetInt());        break;
        case    PropIndex::TextStyleId:                 m_textStyleId.SetValue (value.GetInt64());      break;
        case    PropIndex::TitleRowCount:               m_titleRowCount           = value.GetInt();        break;
        case    PropIndex::HeaderRowCount:              m_headerRowCount          = value.GetInt();        break;
        case    PropIndex::FooterRowCount:              m_footerRowCount          = value.GetInt();        break;
        case    PropIndex::HeaderColumnCount:           m_headerColumnCount       = value.GetInt();        break;
        case    PropIndex::FooterColumnCount:           m_footerColumnCount       = value.GetInt();        break;
        case    PropIndex::BreakType:                   m_breakType               = value.GetInt();        break;
        case    PropIndex::BreakPosition:               m_breakPosition           = value.GetInt();        break;
        case    PropIndex::BreakLength:                 m_breakLength.SetValue (value.GetDouble());     break;
        case    PropIndex::BreakGap:                    m_breakGap.SetValue (value.GetDouble());     break;
        case    PropIndex::RepeatHeaders:               m_repeatHeaders.SetValue (value.GetBoolean());    break;
        case    PropIndex::RepeatFooters:               m_repeatFooters.SetValue (value.GetBoolean());    break;
        case    PropIndex::DefaultColumnWidth:          m_defaultColumnWidth.SetValue (value.GetDouble()); break;
        case    PropIndex::DefaultRowHeight:            m_defaultRowHeight.SetValue (value.GetDouble());   break;
        case    PropIndex::DefaultMarginTop:            m_defaultMarginTop.SetValue (value.GetDouble());        break;
        case    PropIndex::DefaultMarginBottom:         m_defaultMarginBottom.SetValue (value.GetDouble());        break;
        case    PropIndex::DefaultMarginLeft:           m_defaultMarginLeft.SetValue (value.GetDouble());        break;
        case    PropIndex::DefaultMarginRight:          m_defaultMarginRight.SetValue (value.GetDouble());        break;
        case    PropIndex::DefaultCellAlignment:        m_defaultCellAlignment    = value.GetInt();        break;
        case    PropIndex::DefaultCellOrientation:      m_defaultCellOrientation    = value.GetInt();        break;
        case    PropIndex::FillSymbologyKeyOddRow:      m_fillSymbologyKeyOddRow  = value.GetInt();        break;
        case    PropIndex::FillSymbologyKeyEvenRow:     m_fillSymbologyKeyEvenRow = value.GetInt();        break;
        case    PropIndex::TitleRowTextStyle:           m_titleRowTextStyle.SetValue (value.GetInt64());        break;
        case    PropIndex::HeaderRowTextStyle:          m_headerRowTextStyle.SetValue (value.GetInt64());        break;
        case    PropIndex::FooterRowTextStyle:          m_footerRowTextStyle.SetValue (value.GetInt64());        break;
        case    PropIndex::HeaderColumnTextStyle:       m_headerColumnTextStyle.SetValue (value.GetInt64());        break;
        case    PropIndex::FooterColumnTextStyle:       m_footerColumnTextStyle.SetValue (value.GetInt64());        break;
        case    PropIndex::BackupTextHeight:            m_backupTextHeight        = value.GetInt();        break;
        case    PropIndex::DataSourceProviderId:        m_dataSourceProviderId    = value.GetInt();        break;
        case    PropIndex::BodyTextHeight:              m_bodyTextHeight.SetValue (value.GetDouble());        break;
        case    PropIndex::TitleRowTextHeight:          m_titleRowTextHeight.SetValue (value.GetDouble());        break;
        case    PropIndex::HeaderRowTextHeight:         m_headerRowTextHeight.SetValue (value.GetDouble());        break;
        case    PropIndex::FooterRowTextHeight:         m_footerRowTextHeight.SetValue (value.GetDouble());        break;
        case    PropIndex::HeaderColumnTextHeight:      m_headerColumnTextHeight.SetValue (value.GetDouble());        break;
        case    PropIndex::FooterColumnTextHeight:      m_footerColumnTextHeight.SetValue (value.GetDouble());        break;
        default:                                        BeAssert (false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
/*ctor*/    TableHeaderAspect::TableHeaderAspect(AnnotationTableElementR t) : AnnotationTableAspect (t)
    {
    Invalidate();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void    TableHeaderAspect::Invalidate()
    {
    m_rowCount.Clear();
    m_columnCount.Clear();
    m_textStyleId.Clear();
    m_titleRowCount             = 0;
    m_headerRowCount            = 0;
    m_footerRowCount            = 0;
    m_headerColumnCount         = 0;
    m_footerColumnCount         = 0;
    m_breakType                 = 0;
    m_breakPosition             = 0;
    m_breakLength.Clear();
    m_breakGap.Clear();
    m_repeatHeaders.Clear();
    m_repeatFooters.Clear();
    m_defaultColumnWidth.Clear();
    m_defaultRowHeight.Clear();
    m_defaultMarginTop.Clear();
    m_defaultMarginBottom.Clear();
    m_defaultMarginLeft.Clear();
    m_defaultMarginRight.Clear();
    m_defaultCellAlignment      = 0;
    m_fillSymbologyKeyOddRow    = 0;
    m_fillSymbologyKeyEvenRow   = 0;
    m_titleRowTextStyle.Clear();
    m_headerRowTextStyle.Clear();
    m_footerRowTextStyle.Clear();
    m_headerColumnTextStyle.Clear();
    m_footerColumnTextStyle.Clear();
    m_backupTextHeight          = 0.0;
    m_dataSourceProviderId      = 0;
    m_bodyTextHeight.Clear();
    m_titleRowTextHeight.Clear();
    m_headerRowTextHeight.Clear();
    m_footerRowTextHeight.Clear();
    m_headerColumnTextHeight.Clear();
    m_footerColumnTextHeight.Clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
void    TableHeaderAspect::_CopyDataFrom (AnnotationTableAspectCR rhsAspect)
    {
    TableHeaderAspect const&   rhs = static_cast <TableHeaderAspect const&> (rhsAspect);

    m_rowCount                  = rhs.m_rowCount;
    m_columnCount               = rhs.m_columnCount;
    m_textStyleId               = rhs.m_textStyleId;
    m_titleRowCount             = rhs.m_titleRowCount;
    m_headerRowCount            = rhs.m_headerRowCount;
    m_footerRowCount            = rhs.m_footerRowCount;
    m_headerColumnCount         = rhs.m_headerColumnCount;
    m_footerColumnCount         = rhs.m_footerColumnCount;
    m_breakType                 = rhs.m_breakType;
    m_breakPosition             = rhs.m_breakPosition;
    m_breakLength               = rhs.m_breakLength;
    m_breakGap                  = rhs.m_breakGap;
    m_repeatHeaders             = rhs.m_repeatHeaders;
    m_repeatFooters             = rhs.m_repeatFooters;
    m_defaultColumnWidth        = rhs.m_defaultColumnWidth;
    m_defaultRowHeight          = rhs.m_defaultRowHeight;
    m_defaultMarginTop          = rhs.m_defaultMarginTop;
    m_defaultMarginBottom       = rhs.m_defaultMarginBottom;
    m_defaultMarginLeft         = rhs.m_defaultMarginLeft;
    m_defaultMarginRight        = rhs.m_defaultMarginRight;
    m_defaultCellAlignment      = 0;
    m_fillSymbologyKeyOddRow    = 0;
    m_fillSymbologyKeyEvenRow   = 0;
    m_titleRowTextStyle         = rhs.m_titleRowTextStyle;
    m_headerRowTextStyle        = rhs.m_headerRowTextStyle;
    m_footerRowTextStyle        = rhs.m_footerRowTextStyle;
    m_headerColumnTextStyle     = rhs.m_headerColumnTextStyle;
    m_footerColumnTextStyle     = rhs.m_footerColumnTextStyle;
    m_backupTextHeight          = 0.0;
    m_dataSourceProviderId      = 0;
    m_bodyTextHeight            = rhs.m_bodyTextHeight;
    m_titleRowTextHeight        = rhs.m_titleRowTextHeight;
    m_headerRowTextHeight       = rhs.m_headerRowTextHeight;
    m_footerRowTextHeight       = rhs.m_footerRowTextHeight;
    m_headerColumnTextHeight    = rhs.m_headerColumnTextHeight;
    m_footerColumnTextHeight    = rhs.m_footerColumnTextHeight;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
void    TableHeaderAspect::_BindProperties(ECSqlStatement& statement)
    {
    BindUInt    (statement, HEADER_PARAM_RowCount,                  m_rowCount);
    BindUInt    (statement, HEADER_PARAM_ColumnCount,               m_columnCount);
    BindInt64   (statement, HEADER_PARAM_TextStyleId,               m_textStyleId);
    BindUInt    (statement, HEADER_PARAM_TitleRowCount,             m_titleRowCount);
    BindUInt    (statement, HEADER_PARAM_HeaderRowCount,            m_headerRowCount);
    BindUInt    (statement, HEADER_PARAM_FooterRowCount,            m_footerRowCount);
    BindUInt    (statement, HEADER_PARAM_HeaderColumnCount,         m_headerColumnCount);
    BindUInt    (statement, HEADER_PARAM_FooterColumnCount,         m_footerColumnCount);
    BindUInt    (statement, HEADER_PARAM_BreakType,                 m_breakType);
    BindUInt    (statement, HEADER_PARAM_BreakPosition,             m_breakPosition);
    BindDouble  (statement, HEADER_PARAM_BreakLength,               m_breakLength);
    BindDouble  (statement, HEADER_PARAM_BreakGap,                  m_breakGap);
    BindBool    (statement, HEADER_PARAM_RepeatHeaders,             m_repeatHeaders);
    BindBool    (statement, HEADER_PARAM_RepeatFooters,             m_repeatFooters);
    BindDouble  (statement, HEADER_PARAM_DefaultColumnWidth,        m_defaultColumnWidth);
    BindDouble  (statement, HEADER_PARAM_DefaultRowHeight,          m_defaultRowHeight);
    BindDouble  (statement, HEADER_PARAM_DefaultMarginTop,          m_defaultMarginTop);
    BindDouble  (statement, HEADER_PARAM_DefaultMarginBottom,       m_defaultMarginBottom);
    BindDouble  (statement, HEADER_PARAM_DefaultMarginLeft,         m_defaultMarginLeft);
    BindDouble  (statement, HEADER_PARAM_DefaultMarginRight,        m_defaultMarginRight);
    BindUInt    (statement, HEADER_PARAM_DefaultCellAlignment,      m_defaultCellAlignment);
    BindUInt    (statement, HEADER_PARAM_DefaultCellOrientation,    m_defaultCellOrientation);
    BindUInt    (statement, HEADER_PARAM_FillSymbologyKeyOddRow,    m_fillSymbologyKeyOddRow);
    BindUInt    (statement, HEADER_PARAM_FillSymbologyKeyEvenRow,   m_fillSymbologyKeyEvenRow);
    BindInt64   (statement, HEADER_PARAM_TitleRowTextStyle,         m_titleRowTextStyle);
    BindInt64   (statement, HEADER_PARAM_HeaderRowTextStyle,        m_headerRowTextStyle);
    BindInt64   (statement, HEADER_PARAM_FooterRowTextStyle,        m_footerRowTextStyle);
    BindInt64   (statement, HEADER_PARAM_HeaderColumnTextStyle,     m_headerColumnTextStyle);
    BindInt64   (statement, HEADER_PARAM_FooterColumnTextStyle,     m_footerColumnTextStyle);
    statement.BindDouble    (statement.GetParameterIndex(HEADER_PARAM_BackupTextHeight),           m_backupTextHeight);
    statement.BindInt       (statement.GetParameterIndex(HEADER_PARAM_DataSourceProviderId),       m_dataSourceProviderId);
    BindDouble  (statement, HEADER_PARAM_BodyTextHeight,             m_bodyTextHeight);
    BindDouble  (statement, HEADER_PARAM_TitleRowTextHeight,         m_titleRowTextHeight);
    BindDouble  (statement, HEADER_PARAM_HeaderRowTextHeight,        m_headerRowTextHeight);
    BindDouble  (statement, HEADER_PARAM_FooterRowTextHeight,        m_footerRowTextHeight);
    BindDouble  (statement, HEADER_PARAM_HeaderColumnTextHeight,     m_headerColumnTextHeight);
    BindDouble  (statement, HEADER_PARAM_FooterColumnTextHeight,     m_footerColumnTextHeight);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
int         TableHeaderAspect::GetInteger (PropIndex propIndex) const
    {
    TableIntValue const* intValue = nullptr;

    switch (propIndex)
        {
        case PropIndex::DataSourceProviderId:      { break; }
        }

    if (EXPECTED_CONDITION (nullptr != intValue))
        return intValue->GetValue();

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
int         TableHeaderAspect::GetUInteger (PropIndex propIndex) const
    {
    TableUIntValue const* intValue = nullptr;

    switch (propIndex)
        {
        case PropIndex::RowCount:                  { intValue = &m_rowCount;                break; }
        case PropIndex::ColumnCount:               { intValue = &m_columnCount;             break; }
        case PropIndex::TitleRowCount:             { intValue = &m_titleRowCount;           break; }
        case PropIndex::HeaderRowCount:            { intValue = &m_headerRowCount;          break; }
        case PropIndex::FooterRowCount:            { intValue = &m_footerRowCount;          break; }
        case PropIndex::HeaderColumnCount:         { intValue = &m_headerColumnCount;       break; }
        case PropIndex::FooterColumnCount:         { intValue = &m_footerColumnCount;       break; }
        case PropIndex::BreakType:                 { intValue = &m_breakType;               break; }
        case PropIndex::BreakPosition:             { intValue = &m_breakPosition;           break; }
        case PropIndex::DefaultCellAlignment:      { intValue = &m_defaultCellAlignment;    break; }
        case PropIndex::DefaultCellOrientation:    { intValue = &m_defaultCellOrientation;  break; }
        case PropIndex::FillSymbologyKeyOddRow:    { intValue = &m_fillSymbologyKeyOddRow;  break; }
        case PropIndex::FillSymbologyKeyEvenRow:   { intValue = &m_fillSymbologyKeyEvenRow; break; }
        }

    if (EXPECTED_CONDITION (nullptr != intValue))
        return intValue->GetValue();

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
double          TableHeaderAspect::GetDouble (PropIndex propIndex) const
    {
    TableDoubleValue const* doubleValue = nullptr;

    switch (propIndex)
        {
        case PropIndex::BreakLength:              { doubleValue = &m_breakLength;             break; }
        case PropIndex::BreakGap:                 { doubleValue = &m_breakGap;                break; }
        case PropIndex::DefaultColumnWidth:       { doubleValue = &m_defaultColumnWidth;      break; }
        case PropIndex::DefaultRowHeight:         { doubleValue = &m_defaultRowHeight;        break; }
        case PropIndex::DefaultMarginTop:         { doubleValue = &m_defaultMarginTop;        break; }
        case PropIndex::DefaultMarginBottom:      { doubleValue = &m_defaultMarginBottom;     break; }
        case PropIndex::DefaultMarginLeft:        { doubleValue = &m_defaultMarginLeft;       break; }
        case PropIndex::DefaultMarginRight:       { doubleValue = &m_defaultMarginRight;      break; }
        case PropIndex::BodyTextHeight:           { doubleValue = &m_bodyTextHeight;          break; }
        case PropIndex::TitleRowTextHeight:       { doubleValue = &m_titleRowTextHeight;      break; }
        case PropIndex::HeaderRowTextHeight:      { doubleValue = &m_headerRowTextHeight;     break; }
        case PropIndex::FooterRowTextHeight:      { doubleValue = &m_footerRowTextHeight;     break; }
        case PropIndex::HeaderColumnTextHeight:   { doubleValue = &m_headerColumnTextHeight;  break; }
        case PropIndex::FooterColumnTextHeight:   { doubleValue = &m_footerColumnTextHeight;  break; }
        }

    if (EXPECTED_CONDITION (nullptr != doubleValue))
        return doubleValue->GetValue();

    return 0.0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
AnnotationTextStyleId  TableHeaderAspect::GetStyleId (PropIndex propIndex) const
    {
    TableUInt64Value const* int64Value = nullptr;

    switch (propIndex)
        {
        case PropIndex::TextStyleId:                { int64Value = &m_textStyleId;              break; }
        case PropIndex::TitleRowTextStyle:          { int64Value = &m_titleRowTextStyle;        break; }
        case PropIndex::HeaderRowTextStyle:         { int64Value = &m_headerRowTextStyle;       break; }
        case PropIndex::FooterRowTextStyle:         { int64Value = &m_footerRowTextStyle;       break; }
        case PropIndex::HeaderColumnTextStyle:      { int64Value = &m_headerColumnTextStyle;    break; }
        case PropIndex::FooterColumnTextStyle:      { int64Value = &m_footerColumnTextStyle;    break; }
        }

    AnnotationTextStyleId  styleId;

    if (EXPECTED_CONDITION (nullptr != int64Value))
        styleId = AnnotationTextStyleId(int64Value->GetValue());

    return styleId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void        TableHeaderAspect::SetInteger (int v, PropIndex propIndex)
    {
    TableIntValue* value = nullptr;

    switch (propIndex)
        {
        case PropIndex::TitleRowCount:             { break; }
        case PropIndex::HeaderRowCount:            { break; }
        case PropIndex::FooterRowCount:            { break; }
        case PropIndex::HeaderColumnCount:         { break; }
        case PropIndex::FooterColumnCount:         { break; }
        case PropIndex::BreakType:                 { break; }
        case PropIndex::BreakPosition:             { break; }
        case PropIndex::DefaultCellAlignment:      { break; }
        case PropIndex::DefaultCellOrientation:    { break; }
        case PropIndex::FillSymbologyKeyOddRow:    { break; }
        case PropIndex::FillSymbologyKeyEvenRow:   { break; }
        case PropIndex::TitleRowTextStyle:         { break; }
        case PropIndex::HeaderRowTextStyle:        { break; }
        case PropIndex::FooterRowTextStyle:        { break; }
        case PropIndex::HeaderColumnTextStyle:     { break; }
        case PropIndex::FooterColumnTextStyle:     { break; }
        case PropIndex::DataSourceProviderId:      { break; }
        default: { BeAssert (false); return; }
        }

    if (UNEXPECTED_CONDITION (nullptr == value))
        return;

    value->SetValue (v);
    SetHasChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void        TableHeaderAspect::SetUInteger (int v, PropIndex propIndex)
    {
    TableUIntValue* value = nullptr;

    switch (propIndex)
        {
        case PropIndex::RowCount:                  { value = &m_rowCount;      break; }
        case PropIndex::ColumnCount:               { value = &m_columnCount;   break; }
        default: { BeAssert (false); return; }
        }

    if (UNEXPECTED_CONDITION (nullptr == value))
        return;

    value->SetValue (v);
    SetHasChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void        TableHeaderAspect::SetBoolean (bool v, PropIndex propIndex)
    {
    TableBoolValue* value = nullptr;

    switch (propIndex)
        {
        case PropIndex::RepeatHeaders:      { value = &m_repeatHeaders;   break; }
        case PropIndex::RepeatFooters:      { value = &m_repeatFooters;   break; }
        default: { BeAssert (false); return; }
        }

    if (UNEXPECTED_CONDITION (nullptr == value))
        return;

    value->SetValue (v);
    SetHasChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void        TableHeaderAspect::SetDouble (double v, PropIndex propIndex)
    {
    TableDoubleValue* value = nullptr;

    switch (propIndex)
        {
        case PropIndex::BreakLength:              { value = &m_breakLength;             break; }
        case PropIndex::BreakGap:                 { value = &m_breakGap;                break; }
        case PropIndex::DefaultColumnWidth:       { value = &m_defaultColumnWidth;      break; }
        case PropIndex::DefaultRowHeight:         { value = &m_defaultRowHeight;        break; }
        case PropIndex::DefaultMarginTop:         { value = &m_defaultMarginTop;        break; }
        case PropIndex::DefaultMarginBottom:      { value = &m_defaultMarginBottom;     break; }
        case PropIndex::DefaultMarginLeft:        { value = &m_defaultMarginLeft;       break; }
        case PropIndex::DefaultMarginRight:       { value = &m_defaultMarginRight;      break; }
        case PropIndex::BodyTextHeight:           { value = &m_bodyTextHeight;          break; }
        case PropIndex::TitleRowTextHeight:       { value = &m_titleRowTextHeight;      break; }
        case PropIndex::HeaderRowTextHeight:      { value = &m_headerRowTextHeight;     break; }
        case PropIndex::FooterRowTextHeight:      { value = &m_footerRowTextHeight;     break; }
        case PropIndex::HeaderColumnTextHeight:   { value = &m_headerColumnTextHeight;  break; }
        case PropIndex::FooterColumnTextHeight:   { value = &m_footerColumnTextHeight;  break; }
        }

    if (UNEXPECTED_CONDITION (nullptr == value))
        return;

    value->SetValue (v);
    SetHasChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void        TableHeaderAspect::SetStyleId (AnnotationTextStyleId v, PropIndex propIndex)
    {
    TableUInt64Value* value = nullptr;

    switch (propIndex)
        {
        case PropIndex::TextStyleId:                { value = &m_textStyleId;              break; }
        case PropIndex::TitleRowTextStyle:          { value = &m_titleRowTextStyle;        break; }
        case PropIndex::HeaderRowTextStyle:         { value = &m_headerRowTextStyle;       break; }
        case PropIndex::FooterRowTextStyle:         { value = &m_footerRowTextStyle;       break; }
        case PropIndex::HeaderColumnTextStyle:      { value = &m_headerColumnTextStyle;    break; }
        case PropIndex::FooterColumnTextStyle:      { value = &m_footerColumnTextStyle;    break; }
        }

    if (UNEXPECTED_CONDITION (nullptr == value))
        return;

    value->SetValue (v.GetValue());
    SetHasChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
uint32_t                AnnotationTableElement::GetRowCount ()                  const     { return m_tableHeader.GetUInteger (TableHeaderAspect::PropIndex::RowCount); }
uint32_t                AnnotationTableElement::GetColumnCount ()               const     { return m_tableHeader.GetUInteger (TableHeaderAspect::PropIndex::ColumnCount); }
uint32_t                AnnotationTableElement::GetTitleRowCount()              const     { return m_tableHeader.GetUInteger (TableHeaderAspect::PropIndex::TitleRowCount); }
uint32_t                AnnotationTableElement::GetHeaderRowCount()             const     { return m_tableHeader.GetUInteger (TableHeaderAspect::PropIndex::HeaderRowCount); }
uint32_t                AnnotationTableElement::GetFooterRowCount()             const     { return m_tableHeader.GetUInteger (TableHeaderAspect::PropIndex::FooterRowCount); }
uint32_t                AnnotationTableElement::GetHeaderColumnCount()          const     { return m_tableHeader.GetUInteger (TableHeaderAspect::PropIndex::HeaderColumnCount); }
uint32_t                AnnotationTableElement::GetFooterColumnCount()          const     { return m_tableHeader.GetUInteger (TableHeaderAspect::PropIndex::FooterColumnCount); }
double                  AnnotationTableElement::GetDefaultRowHeight ()          const     { return m_tableHeader.GetDouble   (TableHeaderAspect::PropIndex::DefaultRowHeight); }
double                  AnnotationTableElement::GetDefaultColumnWidth ()        const     { return m_tableHeader.GetDouble   (TableHeaderAspect::PropIndex::DefaultColumnWidth); }
TableCellOrientation    AnnotationTableElement::GetDefaultCellOrientation ()    const     { return static_cast<TableCellOrientation> (m_tableHeader.GetUInteger  (TableHeaderAspect::PropIndex::DefaultCellOrientation)); }
TableCellAlignment      AnnotationTableElement::GetDefaultCellAlignment ()      const     { return static_cast<TableCellAlignment> (m_tableHeader.GetUInteger  (TableHeaderAspect::PropIndex::DefaultCellAlignment)); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
TableCellMarginValues      AnnotationTableElement::GetDefaultMargins () const
    {
    TableCellMarginValues   margins;

    margins.m_top    = m_tableHeader.GetDouble   (TableHeaderAspect::PropIndex::DefaultMarginTop);
    margins.m_bottom = m_tableHeader.GetDouble   (TableHeaderAspect::PropIndex::DefaultMarginBottom);
    margins.m_left   = m_tableHeader.GetDouble   (TableHeaderAspect::PropIndex::DefaultMarginLeft);
    margins.m_right  = m_tableHeader.GetDouble   (TableHeaderAspect::PropIndex::DefaultMarginRight);

    return margins;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTextStyleId  AnnotationTableElement::GetTextStyleId (AnnotationTableRegion region) const
    {
    switch (region)
        {
        default:
        case AnnotationTableRegion::Body:           return m_tableHeader.GetStyleId (TableHeaderAspect::PropIndex::TextStyleId);
        case AnnotationTableRegion::TitleRow:       return m_tableHeader.GetStyleId (TableHeaderAspect::PropIndex::TitleRowTextStyle);
        case AnnotationTableRegion::HeaderRow:      return m_tableHeader.GetStyleId (TableHeaderAspect::PropIndex::HeaderRowTextStyle);
        case AnnotationTableRegion::FooterRow:      return m_tableHeader.GetStyleId (TableHeaderAspect::PropIndex::FooterRowTextStyle);
        case AnnotationTableRegion::HeaderColumn:   return m_tableHeader.GetStyleId (TableHeaderAspect::PropIndex::HeaderColumnTextStyle);
        case AnnotationTableRegion::FooterColumn:   return m_tableHeader.GetStyleId (TableHeaderAspect::PropIndex::FooterColumnTextStyle);
        }
    }

#if defined (NEEDSWORK)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void     doScaleTextStyle (DgnTextStyleR style, double scale)
    {
    double  height, width;

    style.GetProperty (TextStyle_Height, height);
    style.GetProperty (TextStyle_Width,  width);

    style.SetProperty (TextStyle_Height, height * scale);
    style.SetProperty (TextStyle_Width,  width  * scale);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTextStyleCP  AnnotationTableElement::GetTextStyle (AnnotationTableRegion region) const
    {
    AnnotationTextStyleId   textStyleId = GetTextStyleId(region);

    if ( ! textStyleId.IsValid())
        {
        region = AnnotationTableRegion::Body;
        textStyleId = GetTextStyleId(region);
        }

    bmap<AnnotationTableRegion, AnnotationTextStyleCPtr>::iterator    it = m_textStyles.find (region);

    if (it != m_textStyles.end())
        return it->second.get();

    AnnotationTextStyleCPtr textStyle = AnnotationTextStyle::QueryStyle (textStyleId, GetDgnDb());

    // If textStyle can't be found, make a default one with the backup height.  Presumably this
    // can only happen if the style was deleted in an old version since we no longer allow used
    // styles to be deleted.
    if (textStyle.IsNull())
        {
#if defined (NEEDSWORK)
        textStyle = DgnTextStyle::Create (L"", *m_model->GetDgnFileP());

        double  uorScale     = textStyle->GetUorScale (m_model);
        double  backupHeight = GetBackupTextHeight() / uorScale;

        textStyle->SetProperty (TextStyle_Height, backupHeight);
        textStyle->SetProperty (TextStyle_Width,  backupHeight);
#else
    BeAssert(false);  // NEEDSWORK
#endif
        }
    else
        {
        // If the textStyle has zero height (ex. from dwg), set it to use the backup.
        double height = textStyle->GetHeight ();

        if (DoubleOps::WithinTolerance (height, 0.0, s_doubleTol))
            {
#if defined (NEEDSWORK)
            double  uorScale   = textStyle->GetUorScale (m_model);
            double  backupHeight = GetBackupTextHeight() / uorScale;

            textStyle->SetProperty (TextStyle_Height, backupHeight);
#else
    BeAssert(false);  // I don't think we need this in DgnDb
#endif
            }
        }

#if defined (NEEDSWORK)
    double heightOverride;
    if (SUCCESS == GetTextHeightOverride (heightOverride, region))
        doScaleTextStyle (*textStyle, heightOverride / textStyle->GetHeight(m_model));

    if (HasAnnotationScale())
        doScaleTextStyle (*textStyle, GetAnnotationScale ());
#endif

    m_textStyles[region] = textStyle;

    return textStyle.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void AnnotationTableElement::SetRowCount                (uint32_t             v)    { m_tableHeader.SetUInteger (           v, TableHeaderAspect::PropIndex::RowCount); }
void AnnotationTableElement::SetColumnCount             (uint32_t             v)    { m_tableHeader.SetUInteger (           v, TableHeaderAspect::PropIndex::ColumnCount); }
void AnnotationTableElement::SetTitleRowCount           (uint32_t             v)    { m_tableHeader.SetUInteger (           v, TableHeaderAspect::PropIndex::TitleRowCount); }
void AnnotationTableElement::SetHeaderRowCount          (uint32_t             v)    { m_tableHeader.SetUInteger (           v, TableHeaderAspect::PropIndex::HeaderRowCount); }
void AnnotationTableElement::SetFooterRowCount          (uint32_t             v)    { m_tableHeader.SetUInteger (           v, TableHeaderAspect::PropIndex::FooterRowCount); }
void AnnotationTableElement::SetHeaderColumnCount       (uint32_t             v)    { m_tableHeader.SetUInteger (           v, TableHeaderAspect::PropIndex::HeaderColumnCount); }
void AnnotationTableElement::SetFooterColumnCount       (uint32_t             v)    { m_tableHeader.SetUInteger (           v, TableHeaderAspect::PropIndex::FooterColumnCount); }
void AnnotationTableElement::SetBreakType               (TableBreakType       v)    { m_tableHeader.SetUInteger ((uint32_t) v, TableHeaderAspect::PropIndex::BreakType); }
void AnnotationTableElement::SetBreakPosition           (TableBreakPosition   v)    { m_tableHeader.SetUInteger ((uint32_t) v, TableHeaderAspect::PropIndex::BreakPosition); }
void AnnotationTableElement::SetBreakLength             (double               v)    { m_tableHeader.SetDouble   (           v, TableHeaderAspect::PropIndex::BreakLength); }
void AnnotationTableElement::SetRepeatHeaders           (bool                 v)    { m_tableHeader.SetBoolean  (           v, TableHeaderAspect::PropIndex::RepeatHeaders); }
void AnnotationTableElement::SetRepeatFooters           (bool                 v)    { m_tableHeader.SetBoolean  (           v, TableHeaderAspect::PropIndex::RepeatFooters); }
void AnnotationTableElement::SetDefaultColumnWidth      (double               v)    { m_tableHeader.SetDouble   (           v, TableHeaderAspect::PropIndex::DefaultColumnWidth); }
void AnnotationTableElement::SetDefaultRowHeight        (double               v)    { m_tableHeader.SetDouble   (           v, TableHeaderAspect::PropIndex::DefaultRowHeight); }
void AnnotationTableElement::SetDefaultCellAlignment    (TableCellAlignment   v)    { m_tableHeader.SetUInteger ((uint32_t) v, TableHeaderAspect::PropIndex::DefaultCellAlignment); }
void AnnotationTableElement::SetDefaultCellOrientation  (TableCellOrientation v)    { m_tableHeader.SetUInteger ((uint32_t) v, TableHeaderAspect::PropIndex::DefaultCellOrientation); }
void AnnotationTableElement::SetFillSymbologyForOddRow  (uint32_t             v)    { m_tableHeader.SetUInteger (           v, TableHeaderAspect::PropIndex::FillSymbologyKeyOddRow); }
void AnnotationTableElement::SetFillSymbologyForEvenRow (uint32_t             v)    { m_tableHeader.SetUInteger (           v, TableHeaderAspect::PropIndex::FillSymbologyKeyEvenRow); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::SetTextStyleIdDirect (AnnotationTextStyleId val, AnnotationTableRegion region)
    {
    switch (region)
        {
        default:
        case AnnotationTableRegion::Body:           m_tableHeader.SetStyleId (val, TableHeaderAspect::PropIndex::TextStyleId);              break;
        case AnnotationTableRegion::TitleRow:       m_tableHeader.SetStyleId (val, TableHeaderAspect::PropIndex::TitleRowTextStyle);        break;
        case AnnotationTableRegion::HeaderRow:      m_tableHeader.SetStyleId (val, TableHeaderAspect::PropIndex::HeaderRowTextStyle);       break;
        case AnnotationTableRegion::FooterRow:      m_tableHeader.SetStyleId (val, TableHeaderAspect::PropIndex::FooterRowTextStyle);       break;
        case AnnotationTableRegion::HeaderColumn:   m_tableHeader.SetStyleId (val, TableHeaderAspect::PropIndex::HeaderColumnTextStyle);    break;
        case AnnotationTableRegion::FooterColumn:   m_tableHeader.SetStyleId (val, TableHeaderAspect::PropIndex::FooterColumnTextStyle);    break;
        }
    }

#if defined (NEEDSWORK)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::ClearTextStyleFromCache (AnnotationTableRegion region)
    {
    auto it = m_textStyles.find (region);
    if (it != m_textStyles.end())
        m_textStyles.erase (it);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::SetTextStyleId (AnnotationTextStyleId val, AnnotationTableRegion region)
    {
    SetTextStyleIdDirect (val, region);
#if defined (NEEDSWORK)
    SetTextHeightOverride (NULL, region);

    bool                    isRow;
    TableHeaderFooterType   type;

    ClassifyTableRegion (type, isRow, region);

    if (isRow)
        {
        for (TextTableRowR row : m_rows)
            {
            if (row.GetHeaderFooterType() != type)
                continue;

            row.ApplyHeaderFooterType ();
            }
        }
    else
        {
        for (TextTableColumnR col : m_columns)
            {
            if (col.GetHeaderFooterType() != type)
                continue;

            col.ApplyHeaderFooterType ();
            }
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::SetDefaultMargins (TableCellMarginValuesCR margins)
    {
    m_tableHeader.SetDouble (margins.m_top,    TableHeaderAspect::PropIndex::DefaultMarginTop);
    m_tableHeader.SetDouble (margins.m_bottom, TableHeaderAspect::PropIndex::DefaultMarginBottom);
    m_tableHeader.SetDouble (margins.m_left,   TableHeaderAspect::PropIndex::DefaultMarginLeft);
    m_tableHeader.SetDouble (margins.m_right,  TableHeaderAspect::PropIndex::DefaultMarginRight);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
/* ctor */ AnnotationTableElement::AnnotationTableElement(CreateParams const& params) : T_Super(params), m_tableHeader (*this) { }
AnnotationTableElementPtr AnnotationTableElement::Create(CreateParams const& params) { return new AnnotationTableElement(params); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
AnnotationTableElementPtr AnnotationTableElement::Create (uint32_t rowCount, uint32_t columnCount, AnnotationTextStyleId textStyleId, double backupTextHeight, CreateParams const& params)
    {
    AnnotationTableElementPtr   table = Create (params);

    if (table.IsNull())
        return nullptr;

    table->SetRowCount (rowCount);
    table->SetColumnCount (columnCount);
    table->SetTextStyleId (textStyleId, AnnotationTableRegion::Body);
    //table->SetBackupTextHeight (backupTextHeight);

    table->Initialize (true);

    return table;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableElement::Initialize (bool isNewTable)
    {
    PRECONDITION(m_rows.empty(),);
    PRECONDITION(m_columns.empty(),);

    m_columns.reserve (GetColumnCount());

    for (uint32_t columnIndex = 0; columnIndex < GetColumnCount(); columnIndex++)
        {
        AnnotationTableColumn column (*this, columnIndex);
        m_columns.push_back (column);
        }

    m_rows.reserve (GetRowCount());

    for (uint32_t rowIndex = 0; rowIndex < GetRowCount(); rowIndex++)
        {
        AnnotationTableRow row (*this, rowIndex);
        m_rows.push_back (row);

        m_rows[rowIndex].InitializeInternalCollections();
        }

    if (isNewTable)
        {
        AnnotationTextStyleCP  textStyle =  GetTextStyle (AnnotationTableRegion::Body);

        double textHeight = textStyle->GetHeight ();
        double textWidth  = textHeight * textStyle->GetWidthFactor  ();

        TableCellMarginValues   margins;

        margins.m_top    = 0.2 * textHeight;
        margins.m_bottom = 0.2 * textHeight;
        margins.m_left   = 0.4 * textWidth;
        margins.m_right  = 0.4 * textWidth;

        SetDefaultMargins (margins);

        textHeight += 2 * TextBlockHolder::ComputeDescenderAdjustment (*textStyle);

        SetDefaultRowHeight (textHeight + margins.m_bottom + margins.m_top);
        SetDefaultColumnWidth (10 * textWidth + margins.m_left + margins.m_right);

#if defined (NEEDSWORK)
        SymbologyEntry symbology(0);
        symbology.SetVisible (true);
        symbology.SetColor (0);
        symbology.SetLineStyle (0);
        symbology.SetWeight (0);

        m_symbologyDictionary.AddSymbology (symbology);
#endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
static TableHeaderFooterType  getHeaderFooterType (uint32_t numTitles, uint32_t numHeaders, uint32_t numFooters, uint32_t count, uint32_t index)
    {
    uint32_t    maxTitleIndex  = numTitles;
    uint32_t    maxHeaderIndex = numHeaders + numTitles;
    uint32_t    minFooterIndex = count - numFooters;

    if (index < maxTitleIndex)
        return TableHeaderFooterType::Title;

    if (index < maxHeaderIndex)
        return TableHeaderFooterType::Header;

    if (index >= minFooterIndex)
        return TableHeaderFooterType::Footer;

    return TableHeaderFooterType::Body;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
TableHeaderFooterType   AnnotationTableElement::GetRowHeaderFooterType (uint32_t rowIndex) const
    {
    uint32_t    numTitles  = GetTitleRowCount();
    uint32_t    numHeaders = GetHeaderRowCount();
    uint32_t    numFooters = GetFooterRowCount();

    return getHeaderFooterType (numTitles, numHeaders, numFooters, GetRowCount(), rowIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
TableHeaderFooterType   AnnotationTableElement::GetColumnHeaderFooterType (uint32_t colIndex) const
    {
    uint32_t    numTitles  = 0;
    uint32_t    numHeaders = GetHeaderColumnCount();
    uint32_t    numFooters = GetFooterColumnCount();

    return getHeaderFooterType (numTitles, numHeaders, numFooters, GetColumnCount(), colIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableElement::ClassifyTableRegion (TableHeaderFooterType& type, bool& isRow, AnnotationTableRegion region)
    {
    switch (region)
        {
        default:
        case AnnotationTableRegion::Body:           isRow = true;  type = TableHeaderFooterType::Body;    break;
        case AnnotationTableRegion::TitleRow:       isRow = true;  type = TableHeaderFooterType::Title;   break;
        case AnnotationTableRegion::HeaderRow:      isRow = true;  type = TableHeaderFooterType::Header;  break;
        case AnnotationTableRegion::FooterRow:      isRow = true;  type = TableHeaderFooterType::Footer;  break;
        case AnnotationTableRegion::HeaderColumn:   isRow = false; type = TableHeaderFooterType::Header;  break;
        case AnnotationTableRegion::FooterColumn:   isRow = false; type = TableHeaderFooterType::Footer;  break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableRegion AnnotationTableElement::GetTableRegionFromRowType (TableHeaderFooterType type)
    {
    switch (type)
        {
        case    TableHeaderFooterType::Title:    return AnnotationTableRegion::TitleRow;
        case    TableHeaderFooterType::Header:   return AnnotationTableRegion::HeaderRow;
        case    TableHeaderFooterType::Footer:   return AnnotationTableRegion::FooterRow;
        }

    return  AnnotationTableRegion::Body;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableRegion AnnotationTableElement::GetTableRegionFromColumnType (TableHeaderFooterType type)
    {
    switch (type)
        {
        case    TableHeaderFooterType::Header:   return AnnotationTableRegion::HeaderColumn;
        case    TableHeaderFooterType::Footer:   return AnnotationTableRegion::FooterColumn;
        }

    return  AnnotationTableRegion::Body;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableRegion AnnotationTableElement::GetTableRegion (AnnotationTableCellIndexCR cellIndex) const
    {
    TableHeaderFooterType   type   = GetRowHeaderFooterType (cellIndex.row);
    AnnotationTableRegion   region = GetTableRegionFromRowType (type);

    if (AnnotationTableRegion::Body != region)
        return region;

    type = GetColumnHeaderFooterType (cellIndex.col);

    return GetTableRegionFromColumnType (type);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableRowP   AnnotationTableElement::GetRow (uint32_t rowIndex)
    {
    if (GetRowCount() <= rowIndex)
        return NULL;

    return &m_rows[rowIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableRowCP  AnnotationTableElement::GetRow (uint32_t rowIndex) const
    {
    return (const_cast <AnnotationTableElement*> (this))->GetRow (rowIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::DeleteAspect (AnnotationTableAspectCR aspect)
    {
    if ( ! aspect.HasValidAspectId())
        return;

    AnnotationTableAspectDescr  aspectDescr (aspect._GetAspectType(), aspect.GetAspectId());
    m_aspectsPendingDelete.push_back (aspectDescr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AnnotationTableElement::DeleteRow (uint32_t rowIndex)
    {
    if (GetRowCount() <= rowIndex)
        return ERROR;

    if (1 == GetRowCount())
        return ERROR;

    AnnotationTableRowP  oldRow = GetRow(rowIndex);

    // Adjust the title/header/footer count if necessary.
    BumpRowHeaderFooterCount (*oldRow, false);

    bvector <AnnotationTableCellP> cellsWithHeightChanges;

#if defined (NEEDSWORK)
    // Fix merge entries for any cells that span the row.
    for (AnnotationTableCellP const& cell: oldRow->FindCells())
        {
        AnnotationTableCellIndexCR  rootIndex = cell->GetIndex();
        MergeEntryCP                merge = m_mergeDictionary.GetMerge (rootIndex);

        if (NULL == merge)  // not a merged cell
            continue;

        // Interior cells are being removed
        if (rootIndex.row != rowIndex)
            {
            cell->DeleteMergeCellInteriorRow();
            cellsWithHeightChanges.push_back (cell);
            continue;
            }

        uint32_t  rowSpan = merge->GetRowSpan();
        uint32_t  colSpan = merge->GetColumnSpan();

        // The root of a merged cell is being removed
        m_mergeDictionary.DeleteMerge (rootIndex, *this);

        // If the cell only spanned one row we are done
        if (1 >= rowSpan)
            continue;

        // If the cell will only span one row and column we are done
        if (1 >= colSpan && 1 >= (rowSpan - 1))
            continue;

        // Make a new merge on the cell in the next row
        TableCellIndex  newRootIndex (rowIndex + 1, rootIndex.col);
        AnnotationTableCellP  newRoot = GetCell (newRootIndex, true);

        if (UNEXPECTED_CONDITION (NULL == newRoot))
            continue;

        newRoot->SetAsMergedCellRoot (rowSpan - 1, colSpan);
        cellsWithHeightChanges.push_back (newRoot);
        }

    // Delete the vertical edgeRuns that span the row.
    GetLeftEdgeRuns().DeleteSpan (*this, rowIndex, 1);

    for (AnnotationTableColumnR column: m_columns)
        column.GetEdgeRuns().DeleteSpan (*this, rowIndex, 1);

    // Delete horizontal edge runs owned by the row.
    for (AnnotationTableEdgeRunR edgeRun : oldRow->GetEdgeRuns())
        edgeRun.DeleteAspect(*this);
#endif
    // Not safe to use this anymore
    oldRow = NULL;

    // Remove the instance for the deleted row, it's cells and it's edgeRuns
    bvector<AnnotationTableRow>::iterator erasePos = m_rows.begin() + rowIndex;
    DeleteAspect (*erasePos);

    for (AnnotationTableCellR cell: erasePos->GetCellVectorR())
        DeleteAspect (cell);

#if defined (NEEDSWORK)
    for (AnnotationTableEdgeRunR run: erasePos->GetEdgeRuns())
        DeleteAspect (run.GetInstanceHolderR());
#endif

    // fixup the affected objects
    m_rows.erase (erasePos);

    uint32_t  adjustedIndex = rowIndex;
    for (bvector<AnnotationTableRow>::iterator rowIter = erasePos; rowIter < m_rows.end(); rowIter++)
        rowIter->SetIndex (adjustedIndex++);

#if defined (NEEDSWORK)
    m_mergeDictionary.AdjustMergesAfterIndex (rowIndex, true, false);
#endif

    SetRowCount (static_cast <uint32_t> (m_rows.size()));

    // Layout any cells that span the new row.
    for (AnnotationTableCellP const& cell: cellsWithHeightChanges)
        cell->SetSizeFromContents(nullptr);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableColumnP   AnnotationTableElement::GetColumn (uint32_t colIndex)
    {
    if (GetColumnCount() <= colIndex)
        return NULL;

    return &m_columns[colIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableColumnCP  AnnotationTableElement::GetColumn (uint32_t colIndex) const
    {
    return (const_cast <AnnotationTableElement*> (this))->GetColumn (colIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AnnotationTableElement::DeleteColumn (uint32_t colIndex)
    {
    if (GetColumnCount() <= colIndex)
        return ERROR;

    if (1 == GetColumnCount())
        return ERROR;

    AnnotationTableColumnP   oldColumn = GetColumn(colIndex);

    // Adjust the title/header/footer count if necessary.
    BumpColumnHeaderFooterCount (*oldColumn, false);

    bvector <AnnotationTableCellIndex> cellsWithWidthChanges;

#if defined (NEEDSWORK)
    // Fix colSpan for any cells that span the column.
    for (AnnotationTableCellP const& cell: oldColumn->FindCells())
        {
        AnnotationTableCellIndexCR    rootIndex = cell->GetIndex();
        MergeEntryCP        merge = m_mergeDictionary.GetMerge (rootIndex);

        if (NULL == merge)  // not a merged cell
            continue;

        // an interior cell is being removed
        if (rootIndex.col != colIndex)
            {
            cell->DeleteMergeCellInteriorColumn();
            cellsWithWidthChanges.push_back (rootIndex);
            continue;
            }

        uint32_t  rowSpan = merge->GetRowSpan();
        uint32_t  colSpan = merge->GetColumnSpan();

        // The root of a merged cell is being removed
        m_mergeDictionary.DeleteMerge (rootIndex, *this);

        // If the cell only spanned one column we are done
        if (1 >= colSpan)
            continue;

        // If the cell will only span one row and column we are done
        if (1 >= rowSpan && 1 >= (colSpan - 1))
            continue;

        // Make a new merge on the cell in the next column
        AnnotationTableCellIndex  newRootIndex (rootIndex.row, colIndex + 1);
        AnnotationTableCellP  newRoot = GetCell (newRootIndex, true);

        if (UNEXPECTED_CONDITION (NULL == newRoot))
            continue;

        newRoot->SetAsMergedCellRoot (rowSpan, colSpan - 1);
        cellsWithWidthChanges.push_back (rootIndex);    // looks like a typo but its ok.  After the col is deleted this will
                                                        // be the index for newRoot.
        }

    // Delete the horizontal edgeRuns that span the column.
    GetTopEdgeRuns().DeleteSpan (*this, colIndex, 1);

    for (AnnotationTableRowR row: m_rows)
        row.GetEdgeRuns().DeleteSpan (*this, colIndex, 1);

    // Delete vertical edge runs owned by the column
    for (AnnotationTableEdgeRunR edgeRun : oldColumn->GetEdgeRuns())
        edgeRun.DeleteInstance(*this);
#endif

    // Not safe to use this anymore
    oldColumn = NULL;

    // Delete the column
    bvector<AnnotationTableColumn>::iterator colErasePos = m_columns.begin() + colIndex;
    DeleteAspect (*colErasePos);

    m_columns.erase (colErasePos);

    // Adjust all the subsequent columns
    uint32_t  adjustedIndex = colIndex;
    for (bvector<AnnotationTableColumn>::iterator colIter = colErasePos; colIter < m_columns.end(); colIter++)
        colIter->SetIndex (adjustedIndex++);

    for (AnnotationTableRow& row: m_rows)
        {
        bvector<AnnotationTableCell>& cells = row.GetCellVectorR();

        // Delete the cell
        bvector<AnnotationTableCell>::iterator cellErasePos = cells.begin() + colIndex;
        DeleteAspect (*cellErasePos);

        cells.erase (cellErasePos);

        // Adjust all the subsequent cells
        adjustedIndex = colIndex;
        for (bvector<AnnotationTableCell>::iterator cellIter = cellErasePos; cellIter < cells.end(); cellIter++)
            cellIter->SetIndex (AnnotationTableCellIndex (row.GetIndex(), adjustedIndex++));
        }

#if defined (NEEDSWORK)
    m_mergeDictionary.AdjustMergesAfterIndex (colIndex, false, false);
#endif

    SetColumnCount (static_cast <uint32_t> (m_columns.size()));

    // Layout any cells that used to span the column.
    for (AnnotationTableCellIndex const& index: cellsWithWidthChanges)
        {
        AnnotationTableCellP  cell = GetCell (index);
        cell->WidthChanged();
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableCellP  AnnotationTableElement::GetCell (AnnotationTableCellIndexCR cellIndex, bool allowMergedInteriors) const
    {
    if (GetRowCount()    <= cellIndex.row ||
        GetColumnCount() <= cellIndex.col)
        {
        return NULL;
        }

    AnnotationTableElementR      nonConstThis = const_cast <AnnotationTableElementR> (*this);
    AnnotationTableCellR  cell = nonConstThis.m_rows[cellIndex.row].GetCellVectorR()[cellIndex.col];

#if defined (NEEDSWORK)
    if ( ! allowMergedInteriors && cell.IsMergedCellInterior())
        return NULL;
#endif

    return &cell;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableCellP  AnnotationTableElement::GetCell (AnnotationTableCellIndexCR cellIndex) const
    {
    return GetCell (cellIndex, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableElement::BumpRowHeaderFooterCount (AnnotationTableRowCR row, bool add)
    {
    uint32_t  increment = add ? 1 : -1;

    switch (row.GetHeaderFooterType())
        {
        case TableHeaderFooterType::Title:
            SetTitleRowCount (GetTitleRowCount() + increment);
            break;
        case TableHeaderFooterType::Header:
            SetHeaderRowCount (GetHeaderRowCount() + increment);
            break;
        case TableHeaderFooterType::Footer:
            SetFooterRowCount (GetFooterRowCount() + increment);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableElement::BumpColumnHeaderFooterCount (AnnotationTableColumnCR col, bool add)
    {
    uint32_t  increment = add ? 1 : -1;

    switch (col.GetHeaderFooterType())
        {
        case TableHeaderFooterType::Header:
            SetHeaderColumnCount (GetHeaderColumnCount() + increment);
            break;
        case TableHeaderFooterType::Footer:
            SetFooterColumnCount (GetFooterColumnCount() + increment);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
double  AnnotationTableElement::GetWidth () const
    {
    double  value = 0.0;

    for (AnnotationTableColumnCR col : m_columns)
        value += col.GetWidth();

    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
double  AnnotationTableElement::GetHeight () const
    {
    double  value = 0.0;

    for (AnnotationTableRowCR row : m_rows)
        value += row.GetHeight();

    return value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTableSerializer::SerializeAspectChanges (AnnotationTableAspectR aspect)
    {
    if ( ! aspect.HasChanges())
        return SUCCESS;

    aspect._FlushChangesToProperties();

    bool    aspectIsMandatory = aspect._IsRequiredOnElement ();
    bool    shouldBePersisted = aspectIsMandatory || aspect._ShouldBePersisted ();

    // if all the properties are default we don't need the aspect anymore
    if ( ! shouldBePersisted)
        return aspect.DeleteFromDb();

    BentleyStatus status;

    if (aspect.HasValidAspectId ())
        status = aspect.UpdateInDb();
    else
        status = aspect.InsertInDb();

    if (SUCCESS != status)
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationTableSerializer::SerializeTableToDb()
    {
    bool    bFailed = false;

    /*-------------------------------------------------------------------------
        Remove deleted aspects
    -------------------------------------------------------------------------*/
    for (AnnotationTableAspectDescr& aspectDesc : m_table.GetAspectsPendingDelete())
        AnnotationTableAspect::DeleteAspectFromDb (aspectDesc.m_type, aspectDesc.m_aspectId, m_table);

    /*-------------------------------------------------------------------------
        Table header data
    -------------------------------------------------------------------------*/
    bFailed |= (SUCCESS != SerializeAspectChanges (m_table.GetHeaderAspect()));

    /*-------------------------------------------------------------------------
        Column data
    -------------------------------------------------------------------------*/
    for (AnnotationTableColumnR column: m_table.GetColumnVectorR())
        {
        bFailed |= (SUCCESS != SerializeAspectChanges (column));
        }

    /*-------------------------------------------------------------------------
        Row data
    -------------------------------------------------------------------------*/
    //for (auto row = m_table.GetRowVectorR().rbegin(); row != m_table.GetRowVectorR().rend(); ++row)
    for (AnnotationTableRowR row: m_table.GetRowVectorR())
        {
        bFailed |= (SUCCESS != SerializeAspectChanges (row));

        /*-------------------------------------------------------------------------
            Cell data
        -------------------------------------------------------------------------*/
        for (AnnotationTableCellR cell: row.GetCellVectorR())
            bFailed |= (SUCCESS != SerializeAspectChanges (cell));
        }

    m_table.GetAspectsPendingDelete().clear();

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationTableElement::SaveChanges()
    {
    AnnotationTableSerializer   serializer (*this);
    serializer.SerializeTableToDb();

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationTableElement::_InsertInDb()
    {
    DgnDbStatus status = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationTableElement::_UpdateInDb()
    {
    DgnDbStatus status = T_Super::_UpdateInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationTableElement::_LoadFromDb()
    {
    DgnDbStatus status = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != status)
        return status;

    CachedECSqlStatementPtr statement;

    /*-------------------------------------------------------------------------
        Table header data
    -------------------------------------------------------------------------*/
    statement = AnnotationTableAspect::GetPreparedSelectStatement (AnnotationTableAspectType::Header, *this);
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
        return DgnDbStatus::ReadError;

    if (DbResult::BE_SQLITE_ROW != statement->Step())
        return DgnDbStatus::ReadError;

    m_tableHeader.AssignProperties (*statement);
    m_tableHeader.SetAspectId (GetElementId().GetValue());

    Initialize (false);

    /*-------------------------------------------------------------------------
        Row data
    -------------------------------------------------------------------------*/
    statement = AnnotationTableAspect::GetPreparedSelectStatement (AnnotationTableAspectType::Row, *this);
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
        return DgnDbStatus::ReadError;

    while (DbResult::BE_SQLITE_ROW == statement->Step())
        {
        int rowIndex = statement->GetValue(1).GetInt();

        AnnotationTableRowP row = GetRow (rowIndex);
        row->AssignProperties (*statement);
        }

    /*-------------------------------------------------------------------------
        Column data
    -------------------------------------------------------------------------*/
    statement = AnnotationTableAspect::GetPreparedSelectStatement (AnnotationTableAspectType::Column, *this);
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
        return DgnDbStatus::ReadError;

    while (DbResult::BE_SQLITE_ROW == statement->Step())
        {
        int columnIndex = statement->GetValue(1).GetInt();

        AnnotationTableColumnP column = GetColumn (columnIndex);
        column->AssignProperties (*statement);
        }

    /*-------------------------------------------------------------------------
        Cell data
    -------------------------------------------------------------------------*/
#if defined (NEEDSWORK)
    if (!skipCells)
#endif
        LoadCells ();

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                                   Josh.Schifter   10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableElement::LoadCells ()
    {
#if defined (NEEDSWORK)
    if (table.AreCellsLoaded())
        return;
#endif
    CachedECSqlStatementPtr statement = AnnotationTableAspect::GetPreparedSelectStatement (AnnotationTableAspectType::Cell, *this);
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
        return;

    while (DbResult::BE_SQLITE_ROW == statement->Step())
        {
        AnnotationTableCellIndex    cellIndex = AnnotationTableCell::GetCellIndex (*statement, 1);
        AnnotationTableCellP        cell      = GetCell (cellIndex);

#if defined (NEEDSWORK)
        if (UNEXPECTED_CONDITION (nullptr == cell || cell->IsMergedCellInterior()))
            continue;
#else
        if (UNEXPECTED_CONDITION (nullptr == cell))
            continue;
#endif

        cell->AssignProperties (*statement);

#if defined (NEEDSWORK)
        cell->ApplyToFillRuns();
#endif
        }

#if defined (NEEDSWORK)
    table.m_cellsLoaded = true;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void AnnotationTableElement::Clear()
    {
    m_tableHeader.Invalidate();
    m_rows.clear();
    m_columns.clear();
    m_textStyles.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
void AnnotationTableElement::_CopyFrom(DgnElementCR rhsElement)
    {
    T_Super::_CopyFrom(rhsElement);

    AnnotationTableElementCP rhs = dynamic_cast<AnnotationTableElementCP>(&rhsElement);
    if (nullptr == rhs)
        return;

    Clear();

    m_tableHeader.CopyDataFrom (rhs->m_tableHeader);

    Initialize (false);

    for (bpair<AnnotationTableRegion, AnnotationTextStyleCPtr> const& entry : rhs->m_textStyles)
        m_textStyles[entry.first] = entry.second->Clone();

    for (AnnotationTableColumnCR rhsCol : rhs->m_columns)
        m_columns[rhsCol.GetIndex()].CopyDataFrom (rhsCol);

    for (AnnotationTableRowCR rhsRow : rhs->m_rows)
        m_rows[rhsRow.GetIndex()].CopyDataFrom (rhsRow);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
bool AnnotationTableElement::IsValid() const
    {
    return (0 < GetRowCount() && 0 < GetColumnCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void AnnotationTableElement::UpdateGeometryRepresentation()
    {
    if (! IsValid())
        return;

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*GetModel(), m_categoryId, m_placement.GetOrigin(), m_placement.GetAngles());

    DPoint3d points[] =
        {
        DPoint3d::From(0,0,0),
        DPoint3d::From(10,0,0),
        DPoint3d::From(10,-10,0),
        };
    ICurvePrimitivePtr primitive = ICurvePrimitive::CreateLineString(points, _countof(points));
    CurveVectorPtr curveVector = CurveVector::Create(primitive, CurveVector::BOUNDARY_TYPE_Open);

    builder->Append (*curveVector);
    builder->SetGeomStreamAndPlacement(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationTableElement::_OnInsert()
    {
    DgnDbStatus status = T_Super::_OnInsert();
    if (DgnDbStatus::Success != status)
        return status;

    UpdateGeometryRepresentation();
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationTableElement::_OnUpdate(DgnElementCR original)
    {
    DgnDbStatus status = T_Super::_OnUpdate(original);
    if (DgnDbStatus::Success != status)
        return status;

    UpdateGeometryRepresentation();
    return DgnDbStatus::Success;
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
