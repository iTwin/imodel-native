//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/AnnotationTable.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/AnnotationTable.h>
#include <DgnPlatformInternal/DgnCore/Annotations/AnnotationTextBlockPersistence.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
#define PARAM_ECInstanceId                      "ECInstanceId"
#define PARAM_ElementId                         "ElementId"

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
#define HEADER_PARAM_TextSymbologyKey           "TextSymbKey"

#define ROW_PARAM_Index                         "RowIndex"
#define ROW_PARAM_HeightLock                    "HeightLock"
#define ROW_PARAM_Height                        "Height"

#define COLUMN_PARAM_Index                      "ColumnIndex"
#define COLUMN_PARAM_WidthLock                  "WidthLock"
#define COLUMN_PARAM_Width                      "Width"

#define CELLINDEX_PARAM_RowIndex                "RowIndex"
#define CELLINDEX_PARAM_ColumnIndex             "ColumnIndex"

#define CELL_PARAM_Index                        "CellIndex"
#define CELL_PARAM_TextBlock                    "TextBlock"
#define CELL_PARAM_FillKey                      "FillKey"
#define CELL_PARAM_Alignment                    "Alignment"
#define CELL_PARAM_Orientation                  "Orientation"
#define CELL_PARAM_MarginTop                    "MarginTop"
#define CELL_PARAM_MarginBottom                 "MarginBottom"
#define CELL_PARAM_MarginLeft                   "MarginLeft"
#define CELL_PARAM_MarginRight                  "MarginRight"

#define MERGEENTRY_PARAM_RootCell               "RootCell"
#define MERGEENTRY_PARAM_RowSpan                "RowSpan"
#define MERGEENTRY_PARAM_ColumnSpan             "ColumnSpan"

#define SYMBOLOGYENTRY_PARAM_Key                "SymbologyKey"
#define SYMBOLOGYENTRY_PARAM_Visible            "Visible"
#define SYMBOLOGYENTRY_PARAM_Color              "Color"
#define SYMBOLOGYENTRY_PARAM_Weight             "Weight"
#define SYMBOLOGYENTRY_PARAM_LineStyleId        "LineStyleId"
#define SYMBOLOGYENTRY_PARAM_LineStyleScale     "LineStyleScale"
#define SYMBOLOGYENTRY_PARAM_FillColor          "FillColor"

#define EDGERUN_PARAM_HostType                  "HostType"
#define EDGERUN_PARAM_Host                      "Host"
#define EDGERUN_PARAM_Start                     "Start"
#define EDGERUN_PARAM_Span                      "Span"
#define EDGERUN_PARAM_SymbologyKey              "SymbologyKey"

static const double s_doubleTol = 1.e-8;

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(AnnotationTableHandler)
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */  AnnotationTableSymbologyValues::AnnotationTableSymbologyValues ()
    : 
    m_lineVisibleFlag(false),
    m_colorFlag (false),
    m_weightFlag(false),
    m_styleFlag (false),
    m_fillVisibleFlag(false),
    m_fillColorFlag(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableSymbologyValues::Clear ()
    {
    m_lineVisibleFlag   = false;
    m_colorFlag         = false;
    m_weightFlag        = false;
    m_styleFlag         = false;
    m_fillVisibleFlag   = false;
    m_fillColorFlag     = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableSymbologyValues::SetLineVisible (bool     val)                { m_lineVisibleFlag = true;  m_lineVisibleVal = val; }
void    AnnotationTableSymbologyValues::SetLineColor   (ColorDef val)                { m_colorFlag       = true;  m_colorVal       = val; SetLineVisible (true); }
void    AnnotationTableSymbologyValues::SetLineWeight  (uint32_t val)                { m_weightFlag      = true;  m_weightVal      = val; SetLineVisible (true); }
void    AnnotationTableSymbologyValues::SetLineStyle   (DgnStyleId id, double scale) { m_styleFlag       = true;  m_styleIdVal     = id; m_styleScaleVal = scale; SetLineVisible (true); }

void    AnnotationTableSymbologyValues::SetFillVisible (bool     val)   { m_fillVisibleFlag = true;  m_fillVisibleVal = val; }
void    AnnotationTableSymbologyValues::SetFillColor   (ColorDef val)   { m_fillColorFlag   = true;  m_fillColorVal   = val; SetFillVisible (true); }

bool    AnnotationTableSymbologyValues::HasLineVisible () const       { return m_lineVisibleFlag;  }
bool    AnnotationTableSymbologyValues::HasLineColor   () const       { return m_colorFlag;        }
bool    AnnotationTableSymbologyValues::HasLineStyle   () const       { return m_styleFlag;        }
bool    AnnotationTableSymbologyValues::HasLineWeight  () const       { return m_weightFlag;       }
bool    AnnotationTableSymbologyValues::HasFillVisible () const       { return m_fillVisibleFlag;  }
bool    AnnotationTableSymbologyValues::HasFillColor   () const       { return m_fillColorFlag;    }

bool       AnnotationTableSymbologyValues::GetLineVisible ()    const { return m_lineVisibleVal;   }
ColorDef   AnnotationTableSymbologyValues::GetLineColor   ()    const { return m_colorVal;         }
uint32_t   AnnotationTableSymbologyValues::GetLineWeight  ()    const { return m_weightVal;        }
DgnStyleId AnnotationTableSymbologyValues::GetLineStyleId ()    const { return m_styleIdVal;       }
double     AnnotationTableSymbologyValues::GetLineStyleScale () const { return m_styleScaleVal;    }
bool       AnnotationTableSymbologyValues::GetFillVisible ()    const { return m_fillVisibleVal;   }
ColorDef   AnnotationTableSymbologyValues::GetFillColor   ()    const { return m_fillColorVal;     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool  AnnotationTableSymbologyValues::IsEquivalent   (AnnotationTableSymbologyValuesCR other) const
    {
    // If both lines invisible don't check the rest of the line props
    bool bothInvisible = HasLineVisible() && ! GetLineVisible() && other.HasLineVisible() && ! other.GetLineVisible();

    if ( ! bothInvisible)
        {
        if (HasLineVisible() != other.HasLineVisible())
            return false;

        if (HasLineVisible() && GetLineVisible() != other.GetLineVisible())
            return false;

        if (HasLineColor() != other.HasLineColor())
            return false;

        if (HasLineColor() && GetLineColor() != other.GetLineColor())
            return false;

        if (HasLineWeight() != other.HasLineWeight())
            return false;

        if (HasLineWeight() && GetLineWeight() != other.GetLineWeight())
            return false;

        if (HasLineStyle() != other.HasLineStyle())
            return false;

        if (HasLineStyle() && GetLineStyleId() != other.GetLineStyleId())
            return false;

        if (HasLineStyle() && GetLineStyleScale() != other.GetLineStyleScale())
            return false;
        }

    // If both lines have invisible fill don't check the rest of the fill props
    bool bothInvisibleFill = HasFillVisible() && ! GetFillVisible() && other.HasFillVisible() && ! other.GetFillVisible();

    if ( ! bothInvisibleFill)
        {
        if (HasFillVisible() != other.HasFillVisible())
            return false;

        if (HasFillVisible() && GetFillVisible() != other.GetFillVisible())
            return false;

        if (HasFillColor() != other.HasFillColor())
            return false;

        if (HasFillColor() && GetFillColor() != other.GetFillColor())
            return false;
        }

    return true;
    }

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
        ecSql.append("[").append(PARAM_ECInstanceId).append("]");
        ecSql.append("=:").append(PARAM_ECInstanceId);
        }

    return ecSql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
static Utf8String buildECSqlDeleteString (Utf8CP schemaName, Utf8CP className, bool isUniqueAspect)
    {
    Utf8PrintfString ecSql("DELETE FROM %s.%s WHERE ", schemaName, className);
    ecSql.append("[").append(PARAM_ECInstanceId).append("]");
    ecSql.append("=:").append(PARAM_ECInstanceId);

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
        ecSql.append("i.[").append(PARAM_ECInstanceId).append("]");
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
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
static Utf8String convertToComponents (Utf8CP aspectIndexProp)
    {
    // NEEDSWORK: this is working around the fact that ECSql does not support structs in
    //            GROUP BY clauses.  Seems to me that it should.
    //
    //            GROUP BY now supports structs... try removing this asap
    Utf8String  inStr (aspectIndexProp);

    if ( ! inStr.Equals ("CellIndex") &&  ! inStr.Equals ("RootCell"))
        return aspectIndexProp;

    Utf8String  outStr;

    outStr.append (aspectIndexProp).append (".RowIndex").append (", ");
    outStr.append (aspectIndexProp).append (".ColumnIndex");

    return outStr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
static Utf8String buildECSqlSelectDupeString (Utf8CP schemaName, Utf8CP className, Utf8CP aspectIndexProp)
    {
    Utf8String  propsString (PARAM_ElementId);
    propsString.append (", ").append (convertToComponents (aspectIndexProp));

    //      SELECT
    //          ElementId, prop[0], COUNT(*)
    //      FROM
    //          dgn.AnnotationTableRow
    //      GROUP BY
    //          ElementId, RowIndex
    //      HAVING 
    //          COUNT(*) > 1
    Utf8String sqlString ("SELECT ");
    sqlString.append (PARAM_ECInstanceId).append (", ");
    sqlString.append (propsString);
    sqlString.append (", COUNT(*)").append (" FROM ");
    sqlString.append (schemaName).append (".").append(className).append(" ");
    sqlString.append ("GROUP BY ");
    sqlString.append (propsString);
    sqlString.append (" HAVING COUNT(*) > 1");

    return sqlString;
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
        PropertyNames mergeNames =   MergeEntry::GetPropertyNames();
        PropertyNames symbNames =    SymbologyEntry::GetPropertyNames();
        PropertyNames edgeRunNames = AnnotationTableEdgeRun::GetPropertyNames();
// NEEDSWORK
//        PropertyNames fillNames =    AnnotationTableFill::GetPropertyNames();

        s_typeData = 
            {
            { AspectTypeData (AnnotationTableAspectType::Header,    headerNames,   true,  DGN_CLASSNAME_AnnotationTableHeader)     },
            { AspectTypeData (AnnotationTableAspectType::Row,       rowNames,      false, DGN_CLASSNAME_AnnotationTableRow)        },
            { AspectTypeData (AnnotationTableAspectType::Column,    colNames,      false, DGN_CLASSNAME_AnnotationTableColumn)     },
            { AspectTypeData (AnnotationTableAspectType::Cell,      cellNames,     false, DGN_CLASSNAME_AnnotationTableCell)       },
            { AspectTypeData (AnnotationTableAspectType::Merge,     mergeNames,    false, DGN_CLASSNAME_AnnotationTableMerge)      },
            { AspectTypeData (AnnotationTableAspectType::Symbology, symbNames,     false, DGN_CLASSNAME_AnnotationTableSymbology)  },
            { AspectTypeData (AnnotationTableAspectType::EdgeRun,   edgeRunNames,  false, DGN_CLASSNAME_AnnotationTableEdgeRun)    },
            };
        }

    return s_typeData[(uint32_t) type];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/ AnnotationTableAspect::AnnotationTableAspect (AnnotationTableAspectCR rhs, bool isNew)
    :
    m_table (rhs.m_table),
    m_hasChanges (false)
    {
    CopyDataFrom (rhs, isNew);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableAspectR AnnotationTableAspect::operator= (AnnotationTableAspectCR rhs)
    {
    CopyDataFrom (rhs, false);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableAspect::CopyDataFrom (AnnotationTableAspectCR rhs, bool isNew)
    {
    if (isNew)
        return;

    m_aspectId   = rhs.m_aspectId;
    m_hasChanges = rhs.m_hasChanges;
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
// @bsimethod                                                   Josh.Schifter   11/2015
//---------------------------------------------------------------------------------------
bool AnnotationTableAspect::DbContainsDuplicateRows (AnnotationTableAspectType aspectType, AnnotationTableElementCR table)
    {
    AspectTypeData  typeData  = GetAspectTypeData (aspectType);
    Utf8StringR     sqlString = typeData.m_ecSqlSelectDupeString;

    if (sqlString.empty())
        sqlString = buildECSqlSelectDupeString (DGN_ECSCHEMA_NAME, typeData.m_ecClassName, typeData.m_propertyNames[0].c_str());

    CachedECSqlStatementPtr statement = table.GetDgnDb().GetPreparedECSqlStatement(sqlString.c_str());
    if (UNEXPECTED_CONDITION (!statement.IsValid()))
        return false;

    if (DbResult::BE_SQLITE_DONE != statement->Step())
        return true;

    return false;
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
        statement.BindInt64  (statement.GetParameterIndex(PARAM_ECInstanceId), m_aspectId.GetValue());

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
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
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
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
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

    statement->BindInt64 (statement->GetParameterIndex(PARAM_ECInstanceId), aspectId);

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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
AnnotationTableCellIndex    AnnotationTableCellIndex::GetCellIndex(ECSqlStatement& statement, uint32_t columnIndex)
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

        if (0 == BeStringUtilities::Stricmp(CELLINDEX_PARAM_RowIndex, memberName))
            cellIndex.row = memberValue.GetInt();
        else if (0 == BeStringUtilities::Stricmp(CELLINDEX_PARAM_ColumnIndex, memberName))
            cellIndex.col = memberValue.GetInt();
        else
            BeAssert(false);
        }

    return cellIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableCellIndex::BindCellIndex(ECSqlStatement& statement, Utf8CP paramName, AnnotationTableCellIndexCR cellIndex)
    {
    int paramIndex = statement.GetParameterIndex(paramName);
    IECSqlStructBinder& binder = statement.BindStruct(paramIndex);

    ECSqlStatus status;

    status = binder.GetMember(CELLINDEX_PARAM_RowIndex).BindInt(cellIndex.row);
    BeAssert(status == ECSqlStatus::Success);

    status = binder.GetMember(CELLINDEX_PARAM_ColumnIndex).BindInt(cellIndex.col);
    BeAssert(status == ECSqlStatus::Success);
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
/*ctor*/  AnnotationTableRow::AnnotationTableRow (AnnotationTableRowCR rhs) : AnnotationTableAspect (rhs, false)
    {
    CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableRowR AnnotationTableRow::operator= (AnnotationTableRowCR rhs)
    {
    AnnotationTableAspect::operator= (rhs);
    CopyDataFrom (rhs);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableRow::CopyDataFrom (AnnotationTableRowCR rhs)
    {
    m_index             = rhs.m_index;
    m_heightLock        = rhs.m_heightLock;
    m_height            = rhs.m_height;
    m_cells             = rhs.m_cells;
    m_edgeRuns          = rhs.m_edgeRuns;
    m_fillRuns          = rhs.m_fillRuns;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
PropertyNames   AnnotationTableRow::GetPropertyNames()
    {
    PropertyNames propNames =
        {
        { (int) PropIndex::RowIndex,   ROW_PARAM_Index       },
        { (int) PropIndex::HeightLock, ROW_PARAM_HeightLock  },
        { (int) PropIndex::Height,     ROW_PARAM_Height      },
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
bool    AnnotationTableRow::_ShouldBePersisted (AnnotationTableSerializer&) const
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
    PRECONDITION(m_edgeRuns.empty(),);
    PRECONDITION(m_fillRuns.empty(),);

    // Cells
    for (uint32_t colIndex = 0; colIndex < GetTable().GetColumnCount(); colIndex++)
        {
        AnnotationTableCell cell (GetTable(), AnnotationTableCellIndex (m_index, colIndex));
        m_cells.push_back (cell);
        }

    // EdgeRuns
    AnnotationTableEdgeRun edgeRun (GetTable());
    edgeRun.Initialize (EdgeRunHostType::Row, m_index);
    m_edgeRuns.push_back (edgeRun);

    // FillRuns
    AnnotationTableFillRun fillRun;
    fillRun.Initialize (GetTable(), m_index);
    m_fillRuns.push_back (fillRun);
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

    // Adjust all the edge runs owned by this row
    for (AnnotationTableEdgeRun& edgeRun: m_edgeRuns)
        edgeRun.SetHostIndex (m_index);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
EdgeRunsR    AnnotationTableRow::GetEdgeRuns (bool top)
    {
    if (0 == m_index && top)
        return GetTable().GetTopEdgeRuns();

    if (top)
        return GetTable().GetRow (m_index - 1)->GetEdgeRuns();

    return GetEdgeRuns();
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
EdgeRunsP AnnotationTableElement::GetEdgeRuns (EdgeRunHostType hostType, uint32_t hostIndex)
    {
    switch (hostType)
        {
        case EdgeRunHostType::Top:
            {
            return &GetTopEdgeRuns();
            }
        case EdgeRunHostType::Left:
            {
            return &GetLeftEdgeRuns();
            }
        case EdgeRunHostType::Column:
            {
            AnnotationTableColumnP    column = GetColumn (hostIndex);

            if (UNEXPECTED_CONDITION (NULL == column))
                return NULL;

            return &column->GetEdgeRuns();
            }
        case EdgeRunHostType::Row:
            {
            AnnotationTableRowP    row = GetRow (hostIndex);

            if (UNEXPECTED_CONDITION (NULL == row))
                return NULL;

            return &row->GetEdgeRuns();
            }
        }

    BeAssert (false);
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
EdgeRunsCP    AnnotationTableElement::GetEdgeRuns (EdgeRunHostType hostType, uint32_t hostIndex) const
    {
    return (const_cast <AnnotationTableElementP> (this))->GetEdgeRuns (hostType, hostIndex);
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
        DgnFontStyle            fontStyle   = DgnFont::FontStyleFromBoldItalic(textStyle->IsBold(), textStyle->IsItalic());
        DgnFontCR               font        = textStyle->ResolveFont();
        double                  height      = fontSize * font.GetDescenderRatio(fontStyle);

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
/*ctor*/  AnnotationTableCell::AnnotationTableCell (AnnotationTableCellCR rhs) : AnnotationTableAspect (rhs, false), m_rawTextBlock (nullptr)
    {
    CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableCellR AnnotationTableCell::operator= (AnnotationTableCellCR rhs)
    {
    AnnotationTableAspect::operator= (rhs);
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
void    AnnotationTableCell::CopyDataFrom (AnnotationTableCellCR rhs)
    {
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
        { (int) PropIndex::CellIndex,      CELL_PARAM_Index         },
        { (int) PropIndex::TextBlock,      CELL_PARAM_TextBlock     },
        { (int) PropIndex::FillKey,        CELL_PARAM_FillKey       },
        { (int) PropIndex::Alignment,      CELL_PARAM_Alignment     },
        { (int) PropIndex::Orientation,    CELL_PARAM_Orientation   },
        { (int) PropIndex::MarginTop,      CELL_PARAM_MarginTop     },
        { (int) PropIndex::MarginBottom,   CELL_PARAM_MarginBottom  },
        { (int) PropIndex::MarginLeft,     CELL_PARAM_MarginLeft    },
        { (int) PropIndex::MarginRight,    CELL_PARAM_MarginRight   },
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
bool    AnnotationTableCell::_ShouldBePersisted (AnnotationTableSerializer&) const
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableCell::_DiscloseSymbologyKeys (bset<uint32_t>& keys)
    {
    if (m_fillKey.IsValid())
        keys.insert (m_fillKey.GetValue());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableCell::_BindProperties(ECSqlStatement& statement)
    {
    AnnotationTableCellIndex::BindCellIndex (statement, CELL_PARAM_Index, m_index);

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
    DVec2d      oldContentSize  = GetContentSize();

    if (EXPECTED_CONDITION (NULL != m_contentHolder))
        m_contentHolder->_SetOrientation (orientation);

    SetOrientationDirect (orientation);
    SetSizeFromContents (&oldContentSize);
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
    DVec2d      oldContentSize  = GetContentSize();

    SetMarginsDirect(newValues);
    SetSizeFromContents(&oldContentSize);
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::ApplyTextStyleByRegion ()
    {
    AnnotationTextBlockCP textBlock = GetTextBlock();

    if (NULL == textBlock)
        return;

    AnnotationTableRegion  region       = GetTableRegion();
    DgnElementId  textStyleId  = GetTable().GetTextStyleId (region);

    /*---------------------------------------------------------------------
        Each cell gets to specify its own justification.
    ---------------------------------------------------------------------*/
    TableCellAlignment                              alignment     = GetAlignment();
    AnnotationTextBlock::HorizontalJustification    justification = ToTextBlockJustification (alignment);

    AnnotationTextBlockPtr copyTextBlock = textBlock->Clone();
    copyTextBlock->SetJustification (justification);
    copyTextBlock->SetStyleId(textStyleId, SetAnnotationTextStyleOptions::Default);

    SetTextBlock(*copyTextBlock);
    }

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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void   AnnotationTableCell::SetFillKey (uint32_t fillKey)   { m_fillKey.SetValue (fillKey); SetHasChanges(); }
void   AnnotationTableCell::ClearFillKey ()                 { m_fillKey.Clear();            SetHasChanges(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void   AnnotationTableCell::ApplyToFillRuns ()
    {
    AnnotationTableRowP row             = GetTable().GetRow (m_index.row);
    FillRunsR           fillRuns        = row->GetFillRuns();
    uint32_t            verticalSpan    = GetRowSpan();

    if (m_fillKey.IsValid() && 0 == m_fillKey.GetValue())
        {
        fillRuns.CreateGap (NULL, m_index.col, GetColumnSpan());
        }
    else
        {
        uint32_t  runKey = m_fillKey.IsNull() ? 0 : m_fillKey.GetValue();

        fillRuns.ApplyRun (runKey, verticalSpan, m_index.col, GetColumnSpan());
        fillRuns.MergeRedundantRuns (&GetTable());
        }

    for (uint32_t iRow = 1; iRow < verticalSpan; iRow++)
        {
        uint32_t                rowIndex        = m_index.row + iRow;
        AnnotationTableRowP     spannedRow      = GetTable().GetRow (rowIndex);
        FillRunsR               spannedFillRuns = spannedRow->GetFillRuns();

        spannedFillRuns.CreateGap (NULL, m_index.col, GetColumnSpan());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void   AnnotationTableCell::SetFillSymbology (AnnotationTableSymbologyValuesCR symb)
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

        SymbologyEntry      newSymbology (GetTable(), 0);
        newSymbology.SetFillColor (symb.GetFillColor());

        uint32_t newKey = GetTable().GetSymbologyDictionary().FindOrAddSymbology (newSymbology);

        SetFillKey (newKey);
        }

    ApplyToFillRuns();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        AnnotationTableCell::GetRowSpan () const
    {
    MergeEntryCP entry = GetTable().GetMergeDictionary().GetMerge (m_index);

    if (NULL == entry)
        return 1;

    return entry->GetRowSpan();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        AnnotationTableCell::GetColumnSpan () const
    {
    MergeEntryCP entry = GetTable().GetMergeDictionary().GetMerge (m_index);

    if (NULL == entry)
        return 1;

    return entry->GetColumnSpan();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool          AnnotationTableCell::IndexIsInSpan (AnnotationTableCellIndexCR index) const
    {
    AnnotationTableCellIndex  myIndex = GetIndex();

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
void            AnnotationTableCell::SetAsMergedCellRoot (uint32_t rowSpan, uint32_t colSpan)
    {
    if (IsMergedCellInterior())
        InitContentsToEmptyTextBlock();

    MergeEntryP entry = GetTable().GetMergeDictionary().GetMerge (m_index);

    if (NULL != entry)
        {
        entry->SetRowSpan (rowSpan);
        entry->SetColumnSpan (colSpan);
        return;
        }

    MergeEntry merge (GetTable(), m_index);

    merge.SetRowSpan (rowSpan);
    merge.SetColumnSpan (colSpan);

    EXPECTED_CONDITION (SUCCESS == GetTable().GetMergeDictionary().AddMerge (merge));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::DeleteMergeCellInteriorRow ()
    {
    MergeEntryP merge = GetTable().GetMergeDictionary().GetMerge (m_index);

    if (UNEXPECTED_CONDITION (NULL == merge))
        return;

    uint32_t    rowSpan = merge->GetRowSpan();

    if (UNEXPECTED_CONDITION (rowSpan <= 1))
        return;

    merge->SetRowSpan (rowSpan - 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::DeleteMergeCellInteriorColumn ()
    {
    MergeEntryP merge = GetTable().GetMergeDictionary().GetMerge (m_index);

    if (UNEXPECTED_CONDITION (NULL == merge))
        return;

    uint32_t    colSpan = merge->GetColumnSpan();

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::GetFillSymbology (AnnotationTableSymbologyValuesR symb) const
    {
    uint32_t fillKey = 0;

    if (m_fillKey.IsValid())
        fillKey = m_fillKey.GetValue();

    if (0 == fillKey)
        {
        symb.SetFillVisible (false);
        return;
        }

    SymbologyDictionary const&   dictionary  = GetTable().GetSymbologyDictionary();
    SymbologyEntryCP             entry       = dictionary.GetSymbology (fillKey);

    if (UNEXPECTED_CONDITION (nullptr == entry))
        {
        symb.SetFillVisible (false);
        return;
        }

    symb.SetFillVisible (true);
    symb.SetFillColor (entry->GetFillColor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCell::GetEdgeSymbology (bvector<AnnotationTableSymbologyValues>& symb, TableCellListEdges edges) const
    {
    bvector<AnnotationTableCellIndex> indices;
    indices.push_back (m_index);
    GetTable().GetEdgeSymbology (symb, edges, indices);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  AnnotationTableColumn::AnnotationTableColumn (AnnotationTableElementR table, int index)
    :
    AnnotationTableAspect (table), m_index (index)
    {
    AnnotationTableEdgeRun edgeRun (table);
    edgeRun.Initialize (EdgeRunHostType::Column, index);
    m_edgeRuns.push_back (edgeRun);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  AnnotationTableColumn::AnnotationTableColumn (AnnotationTableColumnCR rhs) : AnnotationTableAspect (rhs, false)
    {
    CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableColumnR AnnotationTableColumn::operator= (AnnotationTableColumnCR rhs)
    {
    AnnotationTableAspect::operator= (rhs);
    CopyDataFrom (rhs);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableColumn::CopyDataFrom (AnnotationTableColumnCR rhs)
    {
    m_index             = rhs.m_index;
    m_widthLock         = rhs.m_widthLock;
    m_width             = rhs.m_width;
    m_edgeRuns          = rhs.m_edgeRuns;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
PropertyNames   AnnotationTableColumn::GetPropertyNames()
    {
    PropertyNames names =
        {
        { (int) PropIndex::ColumnIndex,  COLUMN_PARAM_Index      },
        { (int) PropIndex::WidthLock,    COLUMN_PARAM_WidthLock  },
        { (int) PropIndex::Width,        COLUMN_PARAM_Width      },
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

    if (GetWidthLock ()) // default is true
        m_widthLock.Clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
bool    AnnotationTableColumn::_ShouldBePersisted (AnnotationTableSerializer&) const
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

    // Adjust all the edge runs owned by this column
    for (AnnotationTableEdgeRun& edgeRun: m_edgeRuns)
        edgeRun.SetHostIndex (m_index);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/13
+---------------+---------------+---------------+---------------+---------------+------*/
EdgeRunsR    AnnotationTableColumn::GetEdgeRuns (bool left)
    {
    if (0 == m_index && left)
        return GetTable().GetLeftEdgeRuns();

    if (left)
        return GetTable().GetColumn (m_index - 1)->GetEdgeRuns();

    return GetEdgeRuns();
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
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  MergeEntry::MergeEntry (AnnotationTableElementR table, AnnotationTableCellIndex rootCell)
    :
    AnnotationTableAspect (table), m_rootCell (rootCell)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  MergeEntry::MergeEntry (MergeEntryCR rhs) : AnnotationTableAspect (rhs, false)
    {
    CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
MergeEntryR MergeEntry::operator= (MergeEntryCR rhs)
    {
    AnnotationTableAspect::operator= (rhs);
    CopyDataFrom (rhs);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    MergeEntry::CopyDataFrom (MergeEntryCR rhs)
    {
    m_rootCell      = rhs.m_rootCell;
    m_rowSpan       = rhs.m_rowSpan;
    m_columnSpan    = rhs.m_columnSpan;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
PropertyNames   MergeEntry::GetPropertyNames()
    {
    PropertyNames names =
        {
        { (int) PropIndex::RootCell,        MERGEENTRY_PARAM_RootCell       },
        { (int) PropIndex::RowSpan,         MERGEENTRY_PARAM_RowSpan        },
        { (int) PropIndex::ColumnSpan,      MERGEENTRY_PARAM_ColumnSpan     },
        };

    return names;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
bool    MergeEntry::_ShouldBePersisted (AnnotationTableSerializer&) const
    {
    if (m_rowSpan.IsValid())        return true;
    if (m_columnSpan.IsValid())     return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void    MergeEntry::_BindProperties(ECSqlStatement& statement)
    {
    AnnotationTableCellIndex::BindCellIndex (statement, MERGEENTRY_PARAM_RootCell, m_rootCell);

    BindUInt   (statement, MERGEENTRY_PARAM_RowSpan,        m_rowSpan);
    BindUInt   (statement, MERGEENTRY_PARAM_ColumnSpan,     m_columnSpan);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void  MergeEntry::_AssignValue (int index, IECSqlValue const& value)
    {
    PropIndex propIndex = static_cast <PropIndex> (index);

    switch (propIndex)
        {
        case PropIndex::RowSpan:        m_rowSpan.SetValue      (value.GetInt());    break;
        case PropIndex::ColumnSpan:     m_columnSpan.SetValue   (value.GetInt());   break;
        default:                        BeAssert (false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
AnnotationTableCellIndexCR    MergeEntry::GetRootIndex () const     { return m_rootCell; }
void MergeEntry::SetRootIndex (AnnotationTableCellIndexCR val)      { m_rootCell = val; SetHasChanges(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
void  MergeEntry::SetRowSpan     (uint32_t val)  { m_rowSpan.SetValue (val);    SetHasChanges(); }
void  MergeEntry::SetColumnSpan  (uint32_t val)  { m_columnSpan.SetValue (val); SetHasChanges(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
uint32_t  MergeEntry::GetRowSpan    () const  { return m_rowSpan.GetValue(); }
uint32_t  MergeEntry::GetColumnSpan () const  { return m_columnSpan.GetValue(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MergeDictionary::AddMerge (MergeEntryCR merge)
    {
    bpair <MergeMap::iterator, bool> retVal;

    retVal = insert (MergeMap::value_type (merge.GetRootIndex(), merge));

    return retVal.second ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/14
+---------------+---------------+---------------+---------------+---------------+------*/
MergeEntryP    MergeDictionary::GetMerge (AnnotationTableCellIndexCR index)
    {
    MergeMap::iterator entry = find (index);

    if (end() == entry)
        return NULL;

    return &entry->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   MergeDictionary::DeleteMerge (AnnotationTableCellIndexCR index, AnnotationTableElementR table)
    {
    MergeEntryP    entry = GetMerge (index);

    if (UNEXPECTED_CONDITION (NULL == entry))
        return ERROR;

    table.DeleteAspect (*entry);

    return (1 == erase (index)) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/14
+---------------+---------------+---------------+---------------+---------------+------*/
MergeEntryCP    MergeDictionary::GetMerge (AnnotationTableCellIndexCR index) const
    {
    return (const_cast <MergeDictionary*> (this))->GetMerge (index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/14
+---------------+---------------+---------------+---------------+---------------+------*/
void    MergeDictionary::AdjustMergesAfterIndex (uint32_t index, bool isRow, bool increment)
    {
    // A row/column was inserted or deleted.  All merges on subsequent rows/columns need
    // to have their rootIndex adjusted.  For merges that span the insert/delete these will
    // have their span's adjusted within Insert/DeleteRow.

    bvector<MergeEntry> mergesToAdjust;
    MergeMap::iterator  mergeMapIter = begin();

    while (mergeMapIter != end())
        {
        AnnotationTableCellIndexCR    rootIndex = mergeMapIter->first;

        if (isRow)
            {
            if (rootIndex.row >= index)
                mergesToAdjust.push_back (mergeMapIter->second);
            }
        else
            {
            if (rootIndex.col >= index)
                mergesToAdjust.push_back (mergeMapIter->second);
            }

        ++mergeMapIter;
        }

    for (MergeEntryR merge : mergesToAdjust)
        {
        AnnotationTableCellIndex  rootIndex = merge.GetRootIndex();
        uint32_t&                 toAdjust (isRow ? rootIndex.row : rootIndex.col);

        erase (rootIndex);

        if (increment)
            toAdjust += 1;
        else
            toAdjust -= 1;

        merge.SetRootIndex (rootIndex);
        AddMerge (merge);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  SymbologyEntry::SymbologyEntry (AnnotationTableElementR table, uint32_t key)
    :
    AnnotationTableAspect (table), m_key (key)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  SymbologyEntry::SymbologyEntry (SymbologyEntryCR rhs) : AnnotationTableAspect (rhs, false)
    {
    CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  SymbologyEntry::SymbologyEntry (SymbologyEntryCR rhs, bool isNew) : AnnotationTableAspect (rhs, isNew)
    {
    CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
SymbologyEntryR SymbologyEntry::operator= (SymbologyEntryCR rhs)
    {
    AnnotationTableAspect::operator= (rhs);
    CopyDataFrom (rhs);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    SymbologyEntry::CopyDataFrom (SymbologyEntryCR rhs)
    {
    m_key             = rhs.m_key;
    m_visible         = rhs.m_visible;
    m_color           = rhs.m_color;
    m_weight          = rhs.m_weight;
    m_lineStyleId     = rhs.m_lineStyleId;
    m_lineStyleScale  = rhs.m_lineStyleScale;
    m_fillColor       = rhs.m_fillColor;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   11/2015
//---------------------------------------------------------------------------------------
PropertyNames   SymbologyEntry::GetPropertyNames()
    {
    PropertyNames names =
        {
        { (int) PropIndex::Key,             SYMBOLOGYENTRY_PARAM_Key            },
        { (int) PropIndex::Visible,         SYMBOLOGYENTRY_PARAM_Visible        },
        { (int) PropIndex::Color,           SYMBOLOGYENTRY_PARAM_Color          },
        { (int) PropIndex::Weight,          SYMBOLOGYENTRY_PARAM_Weight         },
        { (int) PropIndex::LineStyleId,     SYMBOLOGYENTRY_PARAM_LineStyleId    },
        { (int) PropIndex::LineStyleScale,  SYMBOLOGYENTRY_PARAM_LineStyleScale },
        { (int) PropIndex::FillColor,       SYMBOLOGYENTRY_PARAM_FillColor      },
        };

    return names;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   11/2015
//---------------------------------------------------------------------------------------
bool    SymbologyEntry::_ShouldBePersisted (AnnotationTableSerializer&) const
    {
    if (0 == m_key)
        return true;    // Every table needs at least one entry.

    if (m_visible.IsValid())            return true;
    if (m_color.IsValid())              return true;
    if (m_weight.IsValid())             return true;
    if (m_lineStyleId.IsValid())        return true;
    //if (m_lineStyleScale.IsValid())   return true;  Don't test.  If id is null don't want scale.
    if (m_fillColor.IsValid())          return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   11/2015
//---------------------------------------------------------------------------------------
void    SymbologyEntry::_BindProperties(ECSqlStatement& statement)
    {
    statement.BindInt (statement.GetParameterIndex(SYMBOLOGYENTRY_PARAM_Key), m_key);

    BindBool    (statement, SYMBOLOGYENTRY_PARAM_Visible,        m_visible);
    BindUInt    (statement, SYMBOLOGYENTRY_PARAM_Color,          m_color);
    BindUInt    (statement, SYMBOLOGYENTRY_PARAM_Weight,         m_weight);
    BindInt64   (statement, SYMBOLOGYENTRY_PARAM_LineStyleId,    m_lineStyleId);
    BindDouble  (statement, SYMBOLOGYENTRY_PARAM_LineStyleScale, m_lineStyleScale);
    BindUInt    (statement, SYMBOLOGYENTRY_PARAM_FillColor,      m_fillColor);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   11/2015
//---------------------------------------------------------------------------------------
void  SymbologyEntry::_AssignValue (int index, IECSqlValue const& value)
    {
    PropIndex propIndex = static_cast <PropIndex> (index);

    switch (propIndex)
        {
        case PropIndex::Visible:        m_visible.SetValue          (value.GetBoolean());   break;
        case PropIndex::Color:          m_color.SetValue            (value.GetInt());       break;
        case PropIndex::Weight:         m_weight.SetValue           (value.GetInt());       break;
        case PropIndex::LineStyleId:    m_lineStyleId.SetValue      (value.GetInt64());     break;
        case PropIndex::LineStyleScale: m_lineStyleScale.SetValue   (value.GetDouble());    break;
        case PropIndex::FillColor:      m_fillColor.SetValue        (value.GetInt());       break;
        default:                        BeAssert (false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        SymbologyEntry::GetKey () const       { return m_key; }
void            SymbologyEntry::SetKey (uint32_t val) { m_key = val; SetHasChanges(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SymbologyEntry::HasVisible()        const { return      m_visible.IsValid();         }
bool            SymbologyEntry::HasColor()          const { return      m_color.IsValid();           }
bool            SymbologyEntry::HasWeight()         const { return      m_weight.IsValid();          }
bool            SymbologyEntry::HasLineStyle()      const { return      m_lineStyleId.IsValid();     }
bool            SymbologyEntry::GetVisible()        const { return      HasVisible() ? m_visible.GetValue() : true; }
ColorDef        SymbologyEntry::GetColor()          const { return      ColorDef(m_color.GetValue());}
uint32_t        SymbologyEntry::GetWeight()         const { return      m_weight.GetValue();         }
DgnStyleId      SymbologyEntry::GetLineStyleId()    const { return      DgnStyleId(m_lineStyleId.GetValue());    }
double          SymbologyEntry::GetLineStyleScale() const { return      m_lineStyleScale.GetValue(); }
ColorDef        SymbologyEntry::GetFillColor()      const { return      ColorDef(m_fillColor.GetValue());   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            SymbologyEntry::ClearVisible   ()                             { m_visible.Clear();                                              SetHasChanges(); }
void            SymbologyEntry::ClearColor     ()                             { m_color.Clear();                                                SetHasChanges(); }
void            SymbologyEntry::ClearWeight    ()                             { m_weight.Clear();                                               SetHasChanges(); }
void            SymbologyEntry::ClearLineStyle ()                             { m_lineStyleId.Clear(); m_lineStyleScale.Clear();                SetHasChanges(); }
void            SymbologyEntry::ClearFillColor ()                             { m_fillColor.Clear();                                            SetHasChanges(); }
void            SymbologyEntry::SetVisible     (bool     val)                 { m_visible.SetValue (val);                                       SetHasChanges(); }
void            SymbologyEntry::SetColor       (ColorDef val)                 { m_color.SetValue (val.GetValue());                              SetHasChanges(); }
void            SymbologyEntry::SetWeight      (uint32_t val)                 { m_weight.SetValue (val);                                        SetHasChanges(); }
void            SymbologyEntry::SetLineStyle   (DgnStyleId id, double scale)  { m_lineStyleId.SetValue (id.GetValue()); m_lineStyleScale.SetValue (scale); SetHasChanges(); }
void            SymbologyEntry::SetFillColor   (ColorDef val)                 { m_fillColor.SetValue (val.GetValue());                          SetHasChanges(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool     SymbologyEntry::IsEquivalent (SymbologyEntryCR other) const
    {
    // If both invisible don't check the rest.
    if ( ! GetVisible() && ! other.GetVisible())
        return true;

    if (GetVisible() != other.GetVisible())
        return false;

    if (GetColor() != other.GetColor())
        return false;

    if (GetWeight() != other.GetWeight())
        return false;

    if (GetLineStyleId() != other.GetLineStyleId())
        return false;

    if (GetLineStyleScale() != other.GetLineStyleScale())
        return false;

    if (GetFillColor() != other.GetFillColor())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t          SymbologyDictionary::FindOrAddSymbology (SymbologyEntryR entry)
    {
    uint32_t      lowestKey = 0;

    for (SymbologyMap::value_type const& mapEntry: *this)
        {
        if (lowestKey == mapEntry.first)
            lowestKey++;

        SymbologyEntryCR    currSymb = mapEntry.second;

        if (currSymb.IsEquivalent (entry))
            return mapEntry.first;
        }

    // If we get here we need to add it as new entry
    entry.SetKey (lowestKey);
    AddSymbology (entry);

    return lowestKey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SymbologyDictionary::AddSymbology (SymbologyEntryCR symbology)
    {
    bpair <SymbologyMap::iterator, bool> retVal;

    retVal = insert (SymbologyMap::value_type (symbology.GetKey(), symbology));

    return retVal.second ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
SymbologyDictionary::SymbologyDictionary () {}

// NEEDSWORK: remove?
//SymbologyDictionary::SymbologyDictionary (bool addDefaultSymb)
//    {
//    if ( ! addDefaultSymb)
//        return;
//
//    SymbologyEntry symbology(0);
//    symbology.SetColor (0);
//    symbology.SetLineStyle (0);
//    symbology.SetWeight (0);
//
//    AddSymbology (symbology);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
SymbologyEntryP    SymbologyDictionary::GetSymbology (uint32_t key)
    {
    SymbologyMap::iterator entry = find (key);

    if (end() == entry)
        return NULL;

    return &entry->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
SymbologyEntryCP    SymbologyDictionary::GetSymbology (uint32_t key) const
    {
    return (const_cast <SymbologyDictionary*> (this))->GetSymbology (key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  AnnotationTableEdgeRun::AnnotationTableEdgeRun (AnnotationTableElementR table) : AnnotationTableAspect (table) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  AnnotationTableEdgeRun::AnnotationTableEdgeRun (AnnotationTableEdgeRunCR rhs) : AnnotationTableAspect (rhs, false)
    {
    CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
/*ctor*/  AnnotationTableEdgeRun::AnnotationTableEdgeRun (AnnotationTableEdgeRunCR rhs, bool isNew) : AnnotationTableAspect (rhs, isNew)
    {
    CopyDataFrom (rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableEdgeRunR AnnotationTableEdgeRun::operator= (AnnotationTableEdgeRunCR rhs)
    {
    AnnotationTableAspect::operator= (rhs);
    CopyDataFrom (rhs);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void    AnnotationTableEdgeRun::CopyDataFrom (AnnotationTableEdgeRunCR rhs)
    {
    m_hostType          = rhs.m_hostType;
    m_host              = rhs.m_host;
    m_start             = rhs.m_start;
    m_span              = rhs.m_span;
    m_symbologyKey      = rhs.m_symbologyKey;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   11/2015
//---------------------------------------------------------------------------------------
PropertyNames   AnnotationTableEdgeRun::GetPropertyNames()
    {
    PropertyNames names =
        {
        { (int) PropIndex::HostType,      EDGERUN_PARAM_HostType         },
        { (int) PropIndex::Host,          EDGERUN_PARAM_Host             },
        { (int) PropIndex::Start,         EDGERUN_PARAM_Start            },
        { (int) PropIndex::Span,          EDGERUN_PARAM_Span             },
        { (int) PropIndex::SymbologyKey,  EDGERUN_PARAM_SymbologyKey     },
        };

    return names;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   11/2015
//---------------------------------------------------------------------------------------
bool    AnnotationTableEdgeRun::_ShouldBePersisted (AnnotationTableSerializer& serializer) const
    {
    // If the run uses a non-default symbology we need to store that.
    if (0 != GetSymbologyKey())
        return true;

    // If the run represents a span that is not 'natural' we need to store that.
    AnnotationTableElementR table      = serializer.GetElement();
    EdgeRunsP               edgeRuns   = GetHostEdgeRuns (table);
    uint32_t                startIndex = GetStartIndex ();

    if (UNEXPECTED_CONDITION (NULL == edgeRuns))
        return true;

    if (0 != startIndex && !edgeRuns->IsGapEnd (startIndex))
        return true;

    uint32_t    endIndex = GetEndIndex ();
    uint32_t    maxIndex = GetHostMaxIndex (table);

    if (maxIndex != endIndex && !edgeRuns->IsGapStart (endIndex, maxIndex))
        return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   11/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableEdgeRun::_BindProperties(ECSqlStatement& statement)
    {
    statement.BindInt (statement.GetParameterIndex(EDGERUN_PARAM_HostType),     (uint32_t)  m_hostType    );
    statement.BindInt (statement.GetParameterIndex(EDGERUN_PARAM_Host),                     m_host        );
    statement.BindInt (statement.GetParameterIndex(EDGERUN_PARAM_Start),                    m_start       );
    statement.BindInt (statement.GetParameterIndex(EDGERUN_PARAM_Span),                     m_span        );
    statement.BindInt (statement.GetParameterIndex(EDGERUN_PARAM_SymbologyKey),             m_symbologyKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   11/2015
//---------------------------------------------------------------------------------------
void  AnnotationTableEdgeRun::_AssignValue (int index, IECSqlValue const& value)
    {
    PropIndex propIndex = static_cast <PropIndex> (index);

    switch (propIndex)
        {
        case PropIndex::Host:          m_host         = value.GetInt();    break;
        case PropIndex::Start:         m_start        = value.GetInt();    break;
        case PropIndex::Span:          m_span         = value.GetInt();    break;
        case PropIndex::SymbologyKey:  m_symbologyKey = value.GetInt();    break;
        default:                       BeAssert (false);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableEdgeRun::_DiscloseSymbologyKeys (bset<uint32_t>& keys)
    {
    keys.insert (GetSymbologyKey());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      AnnotationTableEdgeRun::ToString() const
    {
    Utf8String     outStr, hostStr;

    switch (GetHostType())
        {
        case EdgeRunHostType::Top:      hostStr = "Top";                               break;
        case EdgeRunHostType::Row:      hostStr.Sprintf ("Row:%d", GetHostIndex());    break;
        case EdgeRunHostType::Left:     hostStr = "Left";                              break;
        case EdgeRunHostType::Column:   hostStr.Sprintf ("Col:%d", GetHostIndex());    break;
        }

    outStr.Sprintf ("Host %ls, Range (%d - %d), key %d, instance=%ls", hostStr.c_str(), GetStartIndex(), GetEndIndex(), GetSymbologyKey());

    return outStr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableEdgeRun::Initialize (EdgeRunHostType hostType, uint32_t hostIndex)
    {
    SetHostType (hostType);
    SetSymbologyKey (0);

    if (EdgeRunHostType::Column == hostType || EdgeRunHostType::Row == hostType)
        SetHostIndex (hostIndex);

    bool isHorizontal = (EdgeRunHostType::Top == hostType || EdgeRunHostType::Row == hostType);

    SetStartIndex (0);
    SetSpan (isHorizontal ? GetTable().GetColumnCount() : GetTable().GetRowCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        AnnotationTableEdgeRun::GetHostMaxIndex (AnnotationTableElementCR table) const
    {
    switch (GetHostType())
        {
        case EdgeRunHostType::Top:
        case EdgeRunHostType::Row:
            {
            return table.GetColumnCount();
            }
        case EdgeRunHostType::Left:
        case EdgeRunHostType::Column:
            {
            return table.GetRowCount();
            }
        }

    BeAssert(false);
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
EdgeRunsP       AnnotationTableEdgeRun::GetHostEdgeRuns (AnnotationTableElementR table) const
    {
    EdgeRunHostType hostType    = GetHostType();
    uint32_t        hostIndex   = GetHostIndex();

    return  table.GetEdgeRuns (hostType, hostIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AnnotationTableEdgeRun::CanMergeWith (AnnotationTableEdgeRunCR other) const
    {
    return (GetSymbologyKey() == other.GetSymbologyKey());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableEdgeRun::OnRemoved (AnnotationTableElementR table)
    {
    table.DeleteAspect (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        AnnotationTableEdgeRun::GetEndIndex () const
    {
    return GetStartIndex() + GetSpan();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */      AnnotationTableFillRun::AnnotationTableFillRun ()   { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */      AnnotationTableFillRun::AnnotationTableFillRun (AnnotationTableFillRunCR other, bool unused)  { From (other); }
void            AnnotationTableFillRun::Initialize (AnnotationTableFillRunCR other, bool)          { From (other); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableFillRunR   AnnotationTableFillRun::operator= (AnnotationTableFillRunCR other)
    {
    From (other);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableFillRun::From (AnnotationTableFillRunCR other)
    {
    m_hostIndex     = other.m_hostIndex;
    m_startIndex    = other.m_startIndex;
    m_span          = other.m_span;
    m_fillKey       = other.m_fillKey;
    m_verticalSpan  = other.m_verticalSpan;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableFillRun::Initialize (AnnotationTableElementCR table, uint32_t hostIndex)
    {
    m_hostIndex     = hostIndex;
    m_startIndex    = 0;
    m_span          = table.GetColumnCount();
    m_fillKey       = 0;
    m_verticalSpan  = 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AnnotationTableFillRun::CanMergeWith (AnnotationTableFillRunCR other) const
    {
    if (m_fillKey != other.m_fillKey)
        return false;

    if (m_verticalSpan != other.m_verticalSpan)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t          AnnotationTableFillRun::GetEndIndex () const
    {
    return GetStartIndex() + GetSpan();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        AnnotationTableFillRun::GetHostIndex    ()        const { return m_hostIndex; }
uint32_t        AnnotationTableFillRun::GetStartIndex   ()        const { return m_startIndex; }
uint32_t        AnnotationTableFillRun::GetSpan         ()        const { return m_span; }
uint32_t        AnnotationTableFillRun::GetFillKey      ()        const { return m_fillKey; }
uint32_t        AnnotationTableFillRun::GetVerticalSpan ()        const { return m_verticalSpan; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableFillRun::SetHostIndex    (uint32_t     val)  { m_hostIndex    = val; }
void            AnnotationTableFillRun::SetStartIndex   (uint32_t     val)  { m_startIndex   = val; }
void            AnnotationTableFillRun::SetSpan         (uint32_t     val)  { m_span         = val; }
void            AnnotationTableFillRun::SetFillKey      (uint32_t     val)  { m_fillKey      = val; }
void            AnnotationTableFillRun::SetVerticalSpan (uint32_t     val)  { m_verticalSpan = val; }

/*---------------------------------------------------------------------------------**//**
* This templated function is used to share logic used to modify both EdgeRuns and FillRuns.
*
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T_RunType, typename T_InitializerType>
static void     insertSpan (bvector<T_RunType>& runVector, uint32_t insertIndex, uint32_t insertSpan, T_InitializerType const& initializer)
    {
    bvector<T_RunType>::iterator    iter = runVector.begin();
    bvector<T_RunType>::iterator    seedRun = iter;
    bool                            expandRun = false;

    // Find the enclosing run (if any)
    while (iter < runVector.end())
        {
        if ((*iter).GetStartIndex() >= insertIndex)
            break;          // we passed it

        if ((*iter).GetEndIndex() < insertIndex)
            {
            seedRun = iter;
            ++iter;
            continue;       // not there yet
            }

        expandRun = true;   // found it
        break;
        }

    if (expandRun)
        {
        uint32_t  runSpan = (*iter).GetSpan();

        (*iter).SetSpan (runSpan + insertSpan);
        }
    else
        {
        T_RunType newRun = initializer.CreateNewRun (seedRun);

        newRun.SetStartIndex    (insertIndex);
        newRun.SetSpan          (insertSpan);
        iter = runVector.insert (iter, newRun);
        }

    ++iter;

    // After the insertion or expansion just push all the runs to the right
    while (iter < runVector.end())
        {
        uint32_t  runStartIndex = (*iter).GetStartIndex();

        (*iter).SetStartIndex (runStartIndex + insertSpan);

        ++iter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* This templated function is used to share logic used to modify both EdgeRuns and FillRuns.
*
* @bsimethod                                                    JoshSchifter    09/14
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T_RunType>
static typename bvector<T_RunType>::iterator    fillGap (bvector<T_RunType>& runVector, uint32_t insertIndex, uint32_t insertSpan)
    {
    bvector<T_RunType>::iterator    iter = runVector.begin();
    bvector<T_RunType>::iterator    seedRun = iter;
    bool                            expandRun = false;

    // Iterate to the run just after the gap
    while (iter < runVector.end())
        {
        if ((*iter).GetStartIndex() > insertIndex)
            break;          // we found a run to the right of the gap, good

        if ((*iter).GetEndIndex() <= insertIndex)
            {
            seedRun = iter;
            ++iter;
            continue;       // not there yet, keep looking
            }

        // The insertIndex is within a run.  It's supposed to be in a gap!
        expandRun = true;
        break;
        }

    if ( ! expandRun)
        {
        // The usual case.  Add a new run previous to the iter.  In the gap.

        // If there is a next run, make sure we don't overlap it
        if (iter != runVector.end() && (*iter).GetStartIndex() < insertIndex + insertSpan)
            {
            // This is not supposed to happen, we are supposed to be filling a gap.
            BeAssert(false);
            insertSpan = (*iter).GetStartIndex() - insertIndex;
            }

        // For the seed run, any run from the same vector will do since all the callers should
        // fix up the symbology key based on better context.  So, we could just use runVector.begin()
        // here and it shouldn't make any difference.  Instead, I'm using a run that is closer to the
        // new one.  If the callers don't do their job right this is more likely to be correct.
        T_RunType newRun (*seedRun);
        newRun.SetStartIndex    (insertIndex);
        newRun.SetSpan          (insertSpan);

        return runVector.insert (iter, newRun);
        }

    // Something went wrong.  Expand the current run to enclose the requested span.
    BeAssert(false);

    uint32_t  runStart = (*iter).GetStartIndex();
    uint32_t  runEnd   = (*iter).GetEndIndex();

    if (runEnd >= insertIndex + insertSpan)
        {
        // nothing to do, the run already encloses the requested span.
        }
    else
        {
        bvector<T_RunType>::iterator    lookAheadIter = iter;
        ++lookAheadIter;

            // If there is a next run, don't overlap it
        if (lookAheadIter != runVector.end() && (*lookAheadIter).GetStartIndex() < insertIndex + insertSpan)
            insertSpan = (*lookAheadIter).GetStartIndex() - insertIndex;

        uint32_t  newSpan = insertSpan - (insertIndex - runStart);
        (*iter).SetSpan (newSpan);
        }

    return iter;
    }

/*---------------------------------------------------------------------------------**//**
* This templated function is used to share logic used to modify both EdgeRuns and FillRuns.
*
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T_RunType>
static void     mergeRedundantRuns (bvector<T_RunType>& runVector, AnnotationTableElementP table)
    {
    if (runVector.empty())
        return;

    bvector<T_RunType>::iterator    iter = runVector.begin();

    while (true)
        {
        bvector<T_RunType>::iterator    nextIter = iter;
        ++nextIter;

        if (runVector.end() == nextIter)
            break;

        T_RunType&   thisRun = *iter;
        T_RunType&   nextRun = *nextIter;

        // If there is a gap between the runs they cannot be merged.
        if (thisRun.GetEndIndex() != nextRun.GetStartIndex())
            {
            ++iter;
            continue;
            }

        // If they don't have the same symbology they cannot be merged
        if ( ! thisRun.CanMergeWith (nextRun))
            {
            ++iter;
            continue;
            }

        // Consume the combined range into thisRun and delete nextRun
        thisRun.SetSpan (thisRun.GetSpan() + nextRun.GetSpan());

        if (NULL != table)
            nextRun.OnRemoved (*table);

        runVector.erase (nextIter);

        // Do NOT advance the iterator we want to consider thisRun again
        }
    }

/*---------------------------------------------------------------------------------**//**
* This templated function is used to share logic used to modify both EdgeRuns and FillRuns.
*
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T_RunType, typename T_RunModifier>
static typename bvector<T_RunType>::iterator    modifySpan (bvector<T_RunType>* removedRuns, bvector<T_RunType>& runVector, uint32_t modStartIndex, uint32_t modSpan, T_RunModifier& modifier)
    {
    /*----------------------------------------------------------------------------
      Splits any edges at the specified span start / end.  Call the modifier
      for each edge within the specified span.
    ----------------------------------------------------------------------------*/
    uint32_t                        modEndIndex = modStartIndex + modSpan;
    bvector<T_RunType>::iterator    iter = runVector.begin();

    while (iter < runVector.end())
        {
        uint32_t  runStartIndex = (*iter).GetStartIndex();
        uint32_t  runEndIndex   = (*iter).GetEndIndex();

        if (runStartIndex >= modEndIndex)
            {
            // mod     ooo
            // mod     oooo
            // run     ----|xxxxx|----
            // result  ----|xxxxx|-----     No change and stop
            break;
            }

        if (runEndIndex <= modStartIndex)
            {
            // mod                 ooo
            // mod                oooo
            // run     ----|xxxxx|----
            // result  ----|xxxxx|----      No change and continue
            ++iter;
            continue;
            }

        if (runStartIndex >= modStartIndex &&
            runEndIndex   <= modEndIndex)
            {
            // mod         oooooooooo
            // mod         ooooooo
            // mod       ooooooooo
            // mod       ooooooooooo
            // run     ----|xxxxx|----
            // result  ----ooooooo----      The whole run is consumed and continue

            if (modifier.CreateGap())
                {
                if (removedRuns)
                    removedRuns->push_back (*iter);

                iter = runVector.erase (iter);
                continue;
                }

            modifier.ModifyRun (*iter);
            ++iter;
            continue;
            }

        if (runStartIndex >= modStartIndex &&
            runEndIndex   >  modEndIndex)
            {
            // mod        ooooo
            // mod         oooo
            // run     ----|xxxxx|----
            // result  ----oooo|x|----      Push the run to the right and stop
            if ( ! modifier.CreateGap())
                {
                T_RunType newRun (*iter);
                newRun.SetStartIndex    (runStartIndex);
                newRun.SetSpan          (modEndIndex - runStartIndex);
                iter = runVector.insert (iter, newRun);
                modifier.ModifyRun (*iter);
                ++iter;
                }

            (*iter).SetStartIndex (modEndIndex);
            (*iter).SetSpan (runEndIndex - modEndIndex);

            break;
            }

        if (runStartIndex <  modStartIndex &&
            runEndIndex   <= modEndIndex)
            {
            // mod            oooo
            // mod            ooooo
            // run     ----|xxxxx|----
            // result  ----|x|oooo----      Push the run to the left and continue
            (*iter).SetSpan (modStartIndex - runStartIndex);

            if ( ! modifier.CreateGap())
                {
                T_RunType newRun (*iter, true);
                newRun.SetStartIndex    (modStartIndex);
                newRun.SetSpan          (runEndIndex - modStartIndex);
                iter = runVector.insert (++iter, newRun);
                modifier.ModifyRun (*iter);
                }

            ++iter;
            continue;
            }

        if (runStartIndex < modStartIndex &&
            runEndIndex   > modEndIndex)
            {
            // mod            o
            // run     ----|xxxxx|----
            // result  ----|x|o|x|----      Split the run in three and stop

            (*iter).SetSpan (modStartIndex - runStartIndex);

            bvector<T_RunType>::iterator    preSplitIter = iter;

            if ( ! modifier.CreateGap())
                {
                T_RunType newRun (*iter);
                newRun.SetStartIndex    (modStartIndex);
                newRun.SetSpan          (modEndIndex - modStartIndex);
                iter = runVector.insert (++iter, newRun);
                modifier.ModifyRun (*iter);
                }

            // This edge will occur after the modified span.
            T_RunType postSplitRun (*preSplitIter);
            postSplitRun.SetStartIndex    (modEndIndex);
            postSplitRun.SetSpan          (runEndIndex - modEndIndex);
            iter = runVector.insert (++iter, postSplitRun);

            break;
            }

        BeAssert (false);
        break;
        }

    return iter;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template <typename T_RunType>
struct RunGapCreator
{
bool  CreateGap () { return true; }
void  ModifyRun (T_RunType& run) {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FillRunSymbologyModifier
{
private:
    uint32_t                m_fillKey;
    uint32_t                m_verticalSpan;

public:
/* ctor */  FillRunSymbologyModifier (uint32_t fillKey, uint32_t verticalSpan)
    :
    m_fillKey (fillKey),
    m_verticalSpan (verticalSpan)
    {
    };

    bool  CreateGap () { return false; }

    void  ModifyRun (AnnotationTableFillRunR fillRun)
        {
        uint32_t          oldKey = fillRun.GetFillKey();
        uint32_t          oldVerticalSpan = fillRun.GetVerticalSpan();

        if (oldKey == m_fillKey && oldVerticalSpan == m_verticalSpan)
            return;

        fillRun.SetFillKey (m_fillKey);
        fillRun.SetVerticalSpan (m_verticalSpan);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            FillRuns::MergeRedundantRuns (AnnotationTableElementP table)
    {
    mergeRedundantRuns (*this, table);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            FillRuns::ApplyRun (uint32_t fillKey, uint32_t verticalSpan, uint32_t startIndex, uint32_t span)
    {
    FillRunSymbologyModifier modifier (fillKey, verticalSpan);

    modifySpan ((FillRuns*) NULL, *this, startIndex, span, modifier);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
FillRuns::iterator    FillRuns::CreateGap (FillRunsP removedRuns, uint32_t gapStartIndex, uint32_t gapSpan)
    {
    /*----------------------------------------------------------------------------
      Opens a non-adressed gap in a list of edge runs.
      Returns an iterator at the position before the gap suitable for insertion.
    ----------------------------------------------------------------------------*/
    RunGapCreator <AnnotationTableFillRun>  gapCreator;

    return modifySpan (removedRuns, *this, gapStartIndex, gapSpan, gapCreator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            FillRuns::InsertSpan (uint32_t startIndex, uint32_t span, IFillRunInitializer const& initializer)
    {
    insertSpan (*this, startIndex, span, initializer);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FillRunInitializer : IFillRunInitializer
    {
    uint32_t                    m_hostIndex;

    FillRunInitializer (uint32_t i) : m_hostIndex(i) { }

    virtual AnnotationTableFillRun CreateNewRun (AnnotationTableFillRunCP seedRun) const override
        {
        AnnotationTableFillRun newRun;

        if (nullptr != seedRun)
            {
            newRun = *seedRun;
            return newRun;
            }

        newRun.SetHostIndex (m_hostIndex);
        newRun.SetFillKey (0);

        return newRun;
        }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EdgeRunSymbologyModifier
{
private:
    SymbologyDictionary&                m_symbologyDictionary;
    AnnotationTableSymbologyValuesCR    m_symbology;

public:
/* ctor */  EdgeRunSymbologyModifier (AnnotationTableSymbologyValuesCR symb, SymbologyDictionary& dict)
    :
    m_symbology (symb),
    m_symbologyDictionary (dict)
    {};

    bool  CreateGap () { return false; }

    void  ModifyRun (AnnotationTableEdgeRunR edgeRun)
        {
        uint32_t            oldKey = edgeRun.GetSymbologyKey();
        SymbologyEntryP     fromDictionary = m_symbologyDictionary.GetSymbology(oldKey);

        if (UNEXPECTED_CONDITION (NULL == fromDictionary))
            return;

        bool needToChange = false;

        if ( ! needToChange && m_symbology.HasLineVisible())  needToChange |= (fromDictionary->GetVisible()        != m_symbology.GetLineVisible());
        if ( ! needToChange && m_symbology.HasLineColor())    needToChange |= (fromDictionary->GetColor()          != m_symbology.GetLineColor());
        if ( ! needToChange && m_symbology.HasLineStyle())    needToChange |= (fromDictionary->GetLineStyleId()    != m_symbology.GetLineStyleId());
        if ( ! needToChange && m_symbology.HasLineStyle())    needToChange |= (fromDictionary->GetLineStyleScale() != m_symbology.GetLineStyleScale());
        if ( ! needToChange && m_symbology.HasLineWeight())   needToChange |= (fromDictionary->GetWeight()         != m_symbology.GetLineWeight());

        if ( ! needToChange)
            return;

        SymbologyEntry      newSymbology (*fromDictionary, true);

        if (m_symbology.HasLineVisible())   newSymbology.SetVisible   (m_symbology.GetLineVisible());
        if (m_symbology.HasLineColor())     newSymbology.SetColor     (m_symbology.GetLineColor());
        if (m_symbology.HasLineStyle())     newSymbology.SetLineStyle (m_symbology.GetLineStyleId(), m_symbology.GetLineStyleScale());
        if (m_symbology.HasLineWeight())    newSymbology.SetWeight    (m_symbology.GetLineWeight());

        uint32_t newKey = m_symbologyDictionary.FindOrAddSymbology (newSymbology);

        edgeRun.SetSymbologyKey (newKey);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      EdgeRuns::ToString() const
    {
    uint32_t    count = 0;
    Utf8String  outStr;

    outStr.append ("   Run Collection:\n");

    for (AnnotationTableEdgeRunCR edgeRun: *this)
        {
        Utf8String runStr;

        runStr.Sprintf ("      %d) %ls\n", count++, edgeRun.ToString().c_str());

        outStr.append (runStr);
        }

    return outStr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool    EdgeRuns::IsGapEnd (uint32_t index) const
    {
    if (0 == index)
        return false;

    const_iterator  iter = begin();
    uint32_t        previousRunEnd = 0;

    while (iter < end())
        {
        AnnotationTableEdgeRunCR  edgeRun = *iter;
        uint32_t                  startIndex = edgeRun.GetStartIndex();

        if (startIndex > index)
            return false;

        if (startIndex == index)
            return startIndex > previousRunEnd;

        previousRunEnd = edgeRun.GetEndIndex();
        ++iter;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool    EdgeRuns::IsGapStart (uint32_t index, uint32_t maxIndex) const
    {
    const_iterator  iter = begin();
    bool            onRunEnd = false;
    uint32_t        endIndex = 0;

    while (iter < end())
        {
        AnnotationTableEdgeRunCR  edgeRun = *iter;

        if (onRunEnd)           // return true if this run is a gap
            return (edgeRun.GetStartIndex() > index);

        endIndex = edgeRun.GetEndIndex();

        if (endIndex > index)
            return false;       // we passed it

        if (endIndex < index)
            {
            ++iter;
            continue;           // not there yet
            }

        if (endIndex == index)
            {
            onRunEnd = true;
            ++iter;
            continue;           // check if the next run is a gap
            }
        }

    return maxIndex != endIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableEdgeRunCP  EdgeRuns::GetSpanningRun (uint32_t index) const
    {
    for (AnnotationTableEdgeRunCR edgeRun : *this)
        {
        if (edgeRun.GetStartIndex() > index)
            break;      // passed it

        if (edgeRun.GetEndIndex() <= index)
            continue;   // not there yet

        return &edgeRun;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            EdgeRuns::MergeRedundantRuns (AnnotationTableElementP table)
    {
    mergeRedundantRuns (*this, table);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
EdgeRuns::iterator    EdgeRuns::CreateGap (EdgeRunsP removedRuns, uint32_t gapStartIndex, uint32_t gapSpan)
    {
    /*----------------------------------------------------------------------------
      Opens a non-adressed gap in a list of edge runs.
      Returns an iterator at the position before the gap suitable for insertion.
    ----------------------------------------------------------------------------*/
    RunGapCreator <AnnotationTableEdgeRun>  gapCreator;

    return modifySpan (removedRuns, *this, gapStartIndex, gapSpan, gapCreator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    EdgeRuns::CloseSpan (EdgeRunsP removedRuns, uint32_t startIndex, uint32_t span)
    {
    EdgeRuns::iterator  iter = CreateGap (removedRuns, startIndex, span);

    while (iter < end())
        {
        uint32_t oldStartIndex = (*iter).GetStartIndex();

        (*iter).SetStartIndex (oldStartIndex - span);

        ++iter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    EdgeRuns::DeleteSpan (AnnotationTableElementR table, uint32_t startIndex, uint32_t span)
    {
    EdgeRuns    removedRuns;

    CloseSpan (&removedRuns, startIndex, span);

    for (AnnotationTableEdgeRunR edgeRun: removedRuns)
        table.DeleteAspect (edgeRun);

    MergeRedundantRuns (&table);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    EdgeRuns::InsertSpan (uint32_t startIndex, uint32_t span, IEdgeRunInitializer const& initializer)
    {
    insertSpan (*this, startIndex, span, initializer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    EdgeRuns::FillGap (uint32_t startIndex, uint32_t span)
    {
    if (UNEXPECTED_CONDITION (this->empty()))
        return;

    fillGap (*this, startIndex, span);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            EdgeRuns::Insert (AnnotationTableEdgeRunCR newEdgeRun)
    {
    iterator iter = CreateGap (NULL, newEdgeRun.GetStartIndex(), newEdgeRun.GetSpan());
    insert (iter, newEdgeRun);

    // This method is called during deserialization so there shouldn't be any redundant runs
    //MergeRedundantRuns();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            EdgeRuns::SetSymbology (AnnotationTableSymbologyValuesCR symb, uint32_t applyStartIndex, uint32_t span, SymbologyDictionary& dictionary)
    {
    EdgeRunSymbologyModifier modifier (symb, dictionary);

    modifySpan ((EdgeRuns*) nullptr, *this, applyStartIndex, span, modifier);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EdgeRunInitializer : IEdgeRunInitializer
    {
    AnnotationTableElementR m_table;
    EdgeRunHostType         m_hostType;
    uint32_t                m_hostIndex;

    EdgeRunInitializer (AnnotationTableElementR e, EdgeRunHostType t)             : m_table(e), m_hostType(t), m_hostIndex(0) { BeAssert (EdgeRunHostType::Left == m_hostType || EdgeRunHostType::Top == m_hostType); }
    EdgeRunInitializer (AnnotationTableElementR e, EdgeRunHostType t, uint32_t i) : m_table(e), m_hostType(t), m_hostIndex(i) { BeAssert (EdgeRunHostType::Row == m_hostType  || EdgeRunHostType::Column == m_hostType); }

    virtual AnnotationTableEdgeRun CreateNewRun (AnnotationTableEdgeRun const* seedRun) const override
        {
        AnnotationTableEdgeRun newRun(m_table);

        if (nullptr != seedRun)
            {
            newRun = *seedRun;
            return newRun;
            }

        newRun.SetHostType  (m_hostType);
        newRun.SetHostIndex (m_hostIndex);
        newRun.SetSymbologyKey (0);

        return newRun;
        }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EdgeRunApplySeedModifier
{
private:
    uint32_t  m_key;

public:
    /* ctor */  EdgeRunApplySeedModifier (uint32_t key) : m_key(key) {}

    bool  CreateGap () { return false; }
    void  ModifyRun (AnnotationTableEdgeRunR edgeRun) { edgeRun.SetSymbologyKey (m_key); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            EdgeRuns::ApplySeed (EdgeRunsCR seedRuns)
    {
    // Copy all the symbology keys from a seedRuns onto 'this'.

    // This is not efficient since it restarts at the beginning of 'this' vector for
    // every member of seedRuns vector.  Hopefully it won't matter because a) the
    // vectors will tend to be small and b) adding new rows/cols is rare.
    uint32_t  lastRunEnd = 0;

    for (AnnotationTableEdgeRunCR seedRun : seedRuns)
        {
        uint32_t seedStart = seedRun.GetStartIndex();
        uint32_t seedSpan  = seedRun.GetSpan();

        if (seedStart > lastRunEnd)
            CreateGap (NULL, lastRunEnd, seedStart - lastRunEnd);

        EdgeRunApplySeedModifier modifier (seedRun.GetSymbologyKey());
        modifySpan ((EdgeRuns*) nullptr, *this, seedStart, seedSpan, modifier);

        lastRunEnd = seedStart + seedSpan;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            EdgeRuns::ApplySeedAtIndex (uint32_t newRunIndex, uint32_t newRunSpan, uint32_t seedRunIndex)
    {
    // Copy symbology key from edge run at seedRunIndex onto newRunIndex with span of newRunSpan.
    AnnotationTableEdgeRunCP seedRun = this->GetSpanningRun (seedRunIndex);

    if (nullptr == seedRun)
        return;

    EdgeRunApplySeedModifier modifier (seedRun->GetSymbologyKey());
    modifySpan ((EdgeRuns*) nullptr, *this, newRunIndex, newRunSpan, modifier);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            EdgeRuns::ApplySeedKeyAtIndex (uint32_t newRunIndex, uint32_t newRunSpan, uint32_t symbologyKey)
    {
    // This is a dangerous method.  Bad things will happen if symbologyKey is not already in the dictionary.

    EdgeRunApplySeedModifier modifier (symbologyKey);
    modifySpan ((EdgeRuns*) nullptr, *this, newRunIndex, newRunSpan, modifier);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
PropertyNames   TableHeaderAspect::GetPropertyNames()
    {
    PropertyNames propNames =
        {
        { (int) PropIndex::RowCount,                HEADER_PARAM_RowCount                  },
        { (int) PropIndex::ColumnCount,             HEADER_PARAM_ColumnCount               },
        { (int) PropIndex::TextStyleId,             HEADER_PARAM_TextStyleId               },
        { (int) PropIndex::TitleRowCount,           HEADER_PARAM_TitleRowCount             }, 
        { (int) PropIndex::HeaderRowCount,          HEADER_PARAM_HeaderRowCount            }, 
        { (int) PropIndex::FooterRowCount,          HEADER_PARAM_FooterRowCount            }, 
        { (int) PropIndex::HeaderColumnCount,       HEADER_PARAM_HeaderColumnCount         }, 
        { (int) PropIndex::FooterColumnCount,       HEADER_PARAM_FooterColumnCount         }, 
        { (int) PropIndex::BreakType,               HEADER_PARAM_BreakType                 }, 
        { (int) PropIndex::BreakPosition,           HEADER_PARAM_BreakPosition             }, 
        { (int) PropIndex::BreakLength,             HEADER_PARAM_BreakLength               }, 
        { (int) PropIndex::BreakGap,                HEADER_PARAM_BreakGap                  }, 
        { (int) PropIndex::RepeatHeaders,           HEADER_PARAM_RepeatHeaders             }, 
        { (int) PropIndex::RepeatFooters,           HEADER_PARAM_RepeatFooters             }, 
        { (int) PropIndex::DefaultColumnWidth,      HEADER_PARAM_DefaultColumnWidth        }, 
        { (int) PropIndex::DefaultRowHeight,        HEADER_PARAM_DefaultRowHeight          }, 
        { (int) PropIndex::DefaultMarginTop,        HEADER_PARAM_DefaultMarginTop          }, 
        { (int) PropIndex::DefaultMarginBottom,     HEADER_PARAM_DefaultMarginBottom       }, 
        { (int) PropIndex::DefaultMarginLeft,       HEADER_PARAM_DefaultMarginLeft         }, 
        { (int) PropIndex::DefaultMarginRight,      HEADER_PARAM_DefaultMarginRight        }, 
        { (int) PropIndex::DefaultCellAlignment,    HEADER_PARAM_DefaultCellAlignment      }, 
        { (int) PropIndex::DefaultCellOrientation,  HEADER_PARAM_DefaultCellOrientation    }, 
        { (int) PropIndex::FillSymbologyKeyOddRow,  HEADER_PARAM_FillSymbologyKeyOddRow    }, 
        { (int) PropIndex::FillSymbologyKeyEvenRow, HEADER_PARAM_FillSymbologyKeyEvenRow   }, 
        { (int) PropIndex::TitleRowTextStyle,       HEADER_PARAM_TitleRowTextStyle         }, 
        { (int) PropIndex::HeaderRowTextStyle,      HEADER_PARAM_HeaderRowTextStyle        }, 
        { (int) PropIndex::FooterRowTextStyle,      HEADER_PARAM_FooterRowTextStyle        }, 
        { (int) PropIndex::HeaderColumnTextStyle,   HEADER_PARAM_HeaderColumnTextStyle     }, 
        { (int) PropIndex::FooterColumnTextStyle,   HEADER_PARAM_FooterColumnTextStyle     }, 
        { (int) PropIndex::BackupTextHeight,        HEADER_PARAM_BackupTextHeight          }, 
        { (int) PropIndex::DataSourceProviderId,    HEADER_PARAM_DataSourceProviderId      }, 
        { (int) PropIndex::BodyTextHeight,          HEADER_PARAM_BodyTextHeight            }, 
        { (int) PropIndex::TitleRowTextHeight,      HEADER_PARAM_TitleRowTextHeight        }, 
        { (int) PropIndex::HeaderRowTextHeight,     HEADER_PARAM_HeaderRowTextHeight       }, 
        { (int) PropIndex::FooterRowTextHeight,     HEADER_PARAM_FooterRowTextHeight       }, 
        { (int) PropIndex::HeaderColumnTextHeight,  HEADER_PARAM_HeaderColumnTextHeight    }, 
        { (int) PropIndex::FooterColumnTextHeight,  HEADER_PARAM_FooterColumnTextHeight    }, 
        { (int) PropIndex::DefaultTextSymbKey,      HEADER_PARAM_TextSymbologyKey          }, 
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
        case    PropIndex::RowCount:                    m_rowCount.SetValue (value.GetInt());               break;
        case    PropIndex::ColumnCount:                 m_columnCount.SetValue (value.GetInt());            break;
        case    PropIndex::TextStyleId:                 m_textStyleId.SetValue (value.GetInt64());          break;
        case    PropIndex::TitleRowCount:               m_titleRowCount           = value.GetInt();     break;
        case    PropIndex::HeaderRowCount:              m_headerRowCount          = value.GetInt();     break;
        case    PropIndex::FooterRowCount:              m_footerRowCount          = value.GetInt();     break;
        case    PropIndex::HeaderColumnCount:           m_headerColumnCount       = value.GetInt();     break;
        case    PropIndex::FooterColumnCount:           m_footerColumnCount       = value.GetInt();     break;
        case    PropIndex::BreakType:                   m_breakType               = value.GetInt();     break;
        case    PropIndex::BreakPosition:               m_breakPosition           = value.GetInt();     break;
        case    PropIndex::BreakLength:                 m_breakLength.SetValue (value.GetDouble());         break;
        case    PropIndex::BreakGap:                    m_breakGap.SetValue (value.GetDouble());            break;
        case    PropIndex::RepeatHeaders:               m_repeatHeaders.SetValue (value.GetBoolean());      break;
        case    PropIndex::RepeatFooters:               m_repeatFooters.SetValue (value.GetBoolean());      break;
        case    PropIndex::DefaultColumnWidth:          m_defaultColumnWidth.SetValue (value.GetDouble());  break;
        case    PropIndex::DefaultRowHeight:            m_defaultRowHeight.SetValue (value.GetDouble());    break;
        case    PropIndex::DefaultMarginTop:            m_defaultMarginTop.SetValue (value.GetDouble());    break;
        case    PropIndex::DefaultMarginBottom:         m_defaultMarginBottom.SetValue (value.GetDouble()); break;
        case    PropIndex::DefaultMarginLeft:           m_defaultMarginLeft.SetValue (value.GetDouble());   break;
        case    PropIndex::DefaultMarginRight:          m_defaultMarginRight.SetValue (value.GetDouble());  break;
        case    PropIndex::DefaultCellAlignment:        m_defaultCellAlignment = value.GetInt();        break;
        case    PropIndex::DefaultCellOrientation:      m_defaultCellOrientation = value.GetInt();      break;
        case    PropIndex::FillSymbologyKeyOddRow:      m_fillSymbologyKeyOddRow = value.GetInt();      break;
        case    PropIndex::FillSymbologyKeyEvenRow:     m_fillSymbologyKeyEvenRow = value.GetInt();     break;
        case    PropIndex::TitleRowTextStyle:           m_titleRowTextStyle.SetValue (value.GetInt64());    break;
        case    PropIndex::HeaderRowTextStyle:          m_headerRowTextStyle.SetValue (value.GetInt64());   break;
        case    PropIndex::FooterRowTextStyle:          m_footerRowTextStyle.SetValue (value.GetInt64());   break;
        case    PropIndex::HeaderColumnTextStyle:       m_headerColumnTextStyle.SetValue (value.GetInt64());break;
        case    PropIndex::FooterColumnTextStyle:       m_footerColumnTextStyle.SetValue (value.GetInt64());break;
        case    PropIndex::BackupTextHeight:            m_backupTextHeight        = value.GetInt();     break;
        case    PropIndex::DataSourceProviderId:        m_dataSourceProviderId    = value.GetInt();     break;
        case    PropIndex::BodyTextHeight:              m_bodyTextHeight.SetValue (value.GetDouble());           break;
        case    PropIndex::TitleRowTextHeight:          m_titleRowTextHeight.SetValue (value.GetDouble());       break;
        case    PropIndex::HeaderRowTextHeight:         m_headerRowTextHeight.SetValue (value.GetDouble());      break;
        case    PropIndex::FooterRowTextHeight:         m_footerRowTextHeight.SetValue (value.GetDouble());      break;
        case    PropIndex::HeaderColumnTextHeight:      m_headerColumnTextHeight.SetValue (value.GetDouble());   break;
        case    PropIndex::FooterColumnTextHeight:      m_footerColumnTextHeight.SetValue (value.GetDouble());   break;
        case    PropIndex::DefaultTextSymbKey:          m_defaultTextSymbKey.SetValue (value.GetInt());          break;
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
    m_defaultTextSymbKey.Clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
void    TableHeaderAspect::CopyDataFrom (TableHeaderAspect const& rhs)
    {
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
    m_defaultTextSymbKey        = rhs.m_defaultTextSymbKey;
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
    BindUInt    (statement, HEADER_PARAM_TextSymbologyKey,           m_defaultTextSymbKey);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    JoshSchifter    11/15
//---------------------------------------------------------------------------------------
void         TableHeaderAspect::_DiscloseSymbologyKeys (bset<uint32_t>& keys)
    {
    keys.insert (0);
    keys.insert (GetTable().GetDefaultTextSymbology());
    keys.insert (GetTable().GetFillSymbologyForOddRow());
    keys.insert (GetTable().GetFillSymbologyForEvenRow());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            TableHeaderAspect::_FlushChangesToProperties ()
    {
    if (0 == GetTable().GetFillSymbologyForOddRow())
        m_fillSymbologyKeyOddRow.Clear();

    if (0 == GetTable().GetFillSymbologyForEvenRow())
        m_fillSymbologyKeyEvenRow.Clear();

    if (0 == GetTable().GetDefaultTextSymbology())
        m_defaultTextSymbKey.Clear();

#if defined (NEEDSWORK)
    if (TableBreakType::None == GetBreakType())
        {
        m_instanceHolder.SetToNull (TEXTTABLE_TABLE_PROP_BreakLength);
        m_instanceHolder.SetToNull (TEXTTABLE_TABLE_PROP_BreakGap);
        }
    else
        {
        if (DoubleOps::WithinTolerance (GetDefaultBreakGap(), GetBreakGap(), s_doubleTol))
            m_instanceHolder.SetToNull (TEXTTABLE_TABLE_PROP_BreakGap);
        }
#endif
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
        case PropIndex::DefaultTextSymbKey:        { intValue = &m_defaultTextSymbKey;      break; }
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
DgnElementId  TableHeaderAspect::GetStyleId (PropIndex propIndex) const
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

    DgnElementId  styleId;

    if (EXPECTED_CONDITION (nullptr != int64Value))
        styleId = DgnElementId(int64Value->GetValue());

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
        case PropIndex::RowCount:                  { value = &m_rowCount;                   break; }
        case PropIndex::ColumnCount:               { value = &m_columnCount;                break; }
        case PropIndex::FillSymbologyKeyOddRow:    { value = &m_fillSymbologyKeyOddRow;     break; }
        case PropIndex::FillSymbologyKeyEvenRow:   { value = &m_fillSymbologyKeyEvenRow;    break; }
        case PropIndex::DefaultTextSymbKey:        { value = &m_defaultTextSymbKey;         break; }
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
void        TableHeaderAspect::SetStyleId (DgnElementId v, PropIndex propIndex)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableCellIterator::AnnotationTableCellIterator (AnnotationTableCellCollection const& collection, bool begin)
    :
    m_parentCollection (&collection)
    {
    m_cell = NULL;

    if (begin)
        m_cell = collection.m_table->GetCell (AnnotationTableCellIndex (0, 0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableCellIterator::MoveToNext ()
    {
    if (NULL == m_cell)
        return;

    AnnotationTableElementR   table       = *(m_parentCollection->m_table);
    AnnotationTableCellIndex  currIndex   = m_cell->GetIndex();

    uint32_t      colCount = table.GetColumnCount();
    uint32_t      rowCount = table.GetRowCount();

    while (true)
        {
        if (currIndex.row >= rowCount -1 && currIndex.col >= colCount - 1)
            {
            BeAssert (currIndex.row == rowCount -1 && currIndex.col == colCount - 1);

            m_cell = NULL;  // <= we have reached the end
            return;
            }

        if (currIndex.col < colCount -1)
            {
            while (currIndex.col < colCount -1)
                {
                // Move to the next column in this row
                currIndex.col++;

                // If we got a real cell return it
                if (NULL != (m_cell = table.GetCell (currIndex)))
                    return;

                // currIndex refers to a merged cell interior, keep going
                }

            continue; // no more columns, go around again
            }

        // Move to the first column of the next row
        currIndex.row++;
        currIndex.col = 0;

        // If we got a real cell return it
        if (NULL != (m_cell = table.GetCell (currIndex)))
            return;

        // currIndex refers to a merged cell interior, keep going
        }

    m_cell = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool  AnnotationTableCellIterator::IsDifferent (AnnotationTableCellIterator const& rhs) const
    {
    if (m_parentCollection != rhs.m_parentCollection)
        return true;

    return (m_cell != rhs.m_cell);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableCellR  AnnotationTableCellIterator::GetCurrent () const
    {
    return *m_cell;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */ AnnotationTableCellCollection::AnnotationTableCellCollection (AnnotationTableElementCR table) : m_table (const_cast <AnnotationTableElementP> (&table)) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableCellCollection::const_iterator  AnnotationTableCellCollection::begin() const { return new AnnotationTableCellIterator (*this, true);  }
AnnotationTableCellCollection::const_iterator  AnnotationTableCellCollection::end() const   { return new AnnotationTableCellIterator (*this, false); }

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
uint32_t                AnnotationTableElement::GetFillSymbologyForOddRow()     const     { return m_tableHeader.GetUInteger (TableHeaderAspect::PropIndex::FillSymbologyKeyOddRow); }
uint32_t                AnnotationTableElement::GetFillSymbologyForEvenRow()    const     { return m_tableHeader.GetUInteger (TableHeaderAspect::PropIndex::FillSymbologyKeyEvenRow); }
uint32_t                AnnotationTableElement::GetDefaultTextSymbology()       const     { return m_tableHeader.GetUInteger (TableHeaderAspect::PropIndex::DefaultTextSymbKey); }

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
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef        AnnotationTableElement::GetDefaultLineColor() const
    {
    SymbologyEntryCP symbology = m_symbologyDictionary.GetSymbology(0);

    if (UNEXPECTED_CONDITION (NULL == symbology))
        return ColorDef::White();

    return symbology->GetColor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        AnnotationTableElement::GetDefaultLineWeight() const
    {
    SymbologyEntryCP symbology = m_symbologyDictionary.GetSymbology(0);

    if (UNEXPECTED_CONDITION (NULL == symbology))
        return 0;

    return symbology->GetWeight();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyleId      AnnotationTableElement::GetDefaultLineStyleId() const
    {
    SymbologyEntryCP symbology = m_symbologyDictionary.GetSymbology(0);

    if (UNEXPECTED_CONDITION (NULL == symbology))
        return DgnStyleId();

    return symbology->GetLineStyleId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
double          AnnotationTableElement::GetDefaultLineStyleScale() const
    {
    SymbologyEntryCP symbology = m_symbologyDictionary.GetSymbology(0);

    if (UNEXPECTED_CONDITION (NULL == symbology))
        return 0;

    return symbology->GetLineStyleScale();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::GetDefaultFill (AnnotationTableSymbologyValuesR symb, TableRows rows) const
    {
    symb.Clear();

    uint32_t  fillKey = 0;

    switch (rows)
        {
        default:
        case TableRows::Odd:    fillKey  = GetFillSymbologyForOddRow();     break;
        case TableRows::Even:   fillKey  = GetFillSymbologyForEvenRow();    break;
        }

    if (0 == fillKey)
        {
        symb.SetFillVisible (false);
        return;
        }

    SymbologyDictionary const&  dictionary  = GetSymbologyDictionary();
    SymbologyEntryCP            entry        = dictionary.GetSymbology (fillKey);

    symb.SetFillVisible (true);
    symb.SetFillColor (entry->GetFillColor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AnnotationTableElement::HasDefaultTextWeight () const
    {
    uint32_t    symbKey   = GetDefaultTextSymbology();

    if (0 == symbKey)
        return false;

    SymbologyEntryCP entry = m_symbologyDictionary.GetSymbology(symbKey);
    return entry->HasWeight();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AnnotationTableElement::HasDefaultTextColor () const
    {
    uint32_t    symbKey   = GetDefaultTextSymbology();

    if (0 == symbKey)
        return false;

    SymbologyEntryCP entry = m_symbologyDictionary.GetSymbology(symbKey);
    return entry->HasColor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        AnnotationTableElement::GetDefaultTextWeight () const
    {
    uint32_t         symbKey = GetDefaultTextSymbology();
    SymbologyEntryCP entry   = m_symbologyDictionary.GetSymbology(symbKey);

    // Might not be a text symb stored but there should at least be a key==0 stored.
    if (UNEXPECTED_CONDITION (nullptr == entry))
        return 0;

    return entry->GetWeight();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef        AnnotationTableElement::GetDefaultTextColor () const
    {
    uint32_t         symbKey = GetDefaultTextSymbology();
    SymbologyEntryCP entry   = m_symbologyDictionary.GetSymbology(symbKey);

    // Might not be a text symb stored but there should at least be a key==0 stored.
    if (UNEXPECTED_CONDITION (nullptr == entry))
        return ColorDef::White();

    return entry->GetColor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId  AnnotationTableElement::GetTextStyleId (AnnotationTableRegion region) const
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
    DgnElementId   textStyleId = GetTextStyleId(region);

    if ( ! textStyleId.IsValid())
        {
        region = AnnotationTableRegion::Body;
        textStyleId = GetTextStyleId(region);
        }

    bmap<AnnotationTableRegion, AnnotationTextStyleCPtr>::iterator    it = m_textStyles.find (region);

    if (it != m_textStyles.end())
        return it->second.get();

    AnnotationTextStyleCPtr textStyle = AnnotationTextStyle::Get(GetDgnDb(), textStyleId);

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
void AnnotationTableElement::SetDefaultTextSymbology    (uint32_t             v)    { m_tableHeader.SetUInteger (           v, TableHeaderAspect::PropIndex::DefaultTextSymbKey); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnotationTableElement::SetBreakGap     (double val)
    {
    if (0 > val)
        return;

    m_tableHeader.SetDouble  (val, TableHeaderAspect::PropIndex::BreakGap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AnnotationTableElement::CanChangeRowHeaderFooterType (uint32_t rowIndex, TableHeaderFooterType newType) const
    {
    uint32_t  firstFooterIndex      = GetRowCount() - GetFooterRowCount();

    uint32_t  lastValidTitleIndex   = GetTitleRowCount();
    uint32_t  lastValidHeaderIndex  = GetTitleRowCount() + GetHeaderRowCount();

    uint32_t  firstValidHeaderIndex = 0 == lastValidTitleIndex   ? 0 : lastValidTitleIndex   - 1;
    uint32_t  firstValidBodyIndex   = 0 == lastValidHeaderIndex  ? 0 : lastValidHeaderIndex  - 1;
    uint32_t  firstValidFooterIndex = 0 == firstFooterIndex      ? 0 : firstFooterIndex      - 1;

    uint32_t  lastValidBodyIndex    = firstValidFooterIndex + 1;

    switch (newType)
        {
        case TableHeaderFooterType::Body:   return rowIndex >= firstValidBodyIndex   && rowIndex <= lastValidBodyIndex;
        case TableHeaderFooterType::Title:  return rowIndex <= lastValidTitleIndex;
        case TableHeaderFooterType::Header: return rowIndex >= firstValidHeaderIndex && rowIndex <= lastValidHeaderIndex;
        case TableHeaderFooterType::Footer: return rowIndex >= firstValidFooterIndex;
        default:                            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AnnotationTableElement::ChangeRowHeaderFooterType (uint32_t rowIndex, TableHeaderFooterType newType)
    {
    /*-------------------------------------------------------------------------
        Intended to be called only from AnnotationTableRow::SetHeaderFooterType
    -------------------------------------------------------------------------*/
    TableHeaderFooterType oldType = GetRowHeaderFooterType(rowIndex);

    if (oldType == newType)
        return SUCCESS;

    if ( ! CanChangeRowHeaderFooterType (rowIndex, newType))
        return ERROR;

    uint32_t  titleCount  = GetTitleRowCount();
    uint32_t  headerCount = GetHeaderRowCount();
    uint32_t  footerCount = GetFooterRowCount();

    switch (oldType)
        {
        case TableHeaderFooterType::Title:  SetTitleRowCount  (titleCount  - 1); break;
        case TableHeaderFooterType::Header: SetHeaderRowCount (headerCount - 1); break;
        case TableHeaderFooterType::Footer: SetFooterRowCount (footerCount - 1); break;
        }

    switch (newType)
        {
        case TableHeaderFooterType::Title:  SetTitleRowCount  (titleCount  + 1); break;
        case TableHeaderFooterType::Header: SetHeaderRowCount (headerCount + 1); break;
        case TableHeaderFooterType::Footer: SetFooterRowCount (footerCount + 1); break;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AnnotationTableElement::CanChangeColumnHeaderFooterType (uint32_t colIndex, TableHeaderFooterType newType) const
    {
    switch (newType)
        {
        case TableHeaderFooterType::Body:   return true;
        case TableHeaderFooterType::Header: return colIndex == 0;
        case TableHeaderFooterType::Footer: return colIndex == GetColumnCount() - 1;
        default:                            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AnnotationTableElement::ChangeColumnHeaderFooterType (uint32_t colIndex, TableHeaderFooterType newType)
    {
    /*-------------------------------------------------------------------------
        Intended to be called only from AnnotationTableColumn::SetHeaderFooterType
    -------------------------------------------------------------------------*/
    TableHeaderFooterType oldType = GetColumnHeaderFooterType(colIndex);

    if (oldType == newType)
        return SUCCESS;

    if ( ! CanChangeColumnHeaderFooterType (colIndex, newType))
        return ERROR;

    uint32_t  headerCount = GetHeaderColumnCount();
    uint32_t  footerCount = GetFooterColumnCount();

    switch (oldType)
        {
        case TableHeaderFooterType::Header: SetHeaderColumnCount (headerCount - 1); break;
        case TableHeaderFooterType::Footer: SetFooterColumnCount (footerCount - 1); break;
        }

    switch (newType)
        {
        case TableHeaderFooterType::Header: SetHeaderColumnCount (headerCount + 1); break;
        case TableHeaderFooterType::Footer: SetFooterColumnCount (footerCount + 1); break;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableRow::ApplyHeaderFooterType ()
    {
    bool                    rowIsEmpty  = true;
    bvector<uint32_t>       colsWithEmptyCells;

    /*---------------------------------------------------------------------
        Apply the appropriate 'region' TextStyle to every cell in the row.
    ---------------------------------------------------------------------*/
    for (AnnotationTableCellR cell : m_cells)
        {
        if (cell.IsMergedCellInterior())
            continue;

        AnnotationTextBlockCP textBlock = cell.GetTextBlock();

        if (NULL == textBlock)
            {
            colsWithEmptyCells.push_back (cell.GetIndex().col);
            continue;
            }

        cell.ApplyTextStyleByRegion ();
        rowIsEmpty = false;
        }

    // Setting textBlocks will recompute the row height but for empty rows need to do it manually.
    if (rowIsEmpty)
        {
        double  minHeight = GetMinimumHeight(false);

        // Here we know the entire row is empty so its ok to set the height directly
        if (GetHeight() < minHeight || false == GetHeightLock())
            SetHeightDirect (minHeight);
        }

    // Setting textBlocks will recompute the col widths but if the column has an empty cell in this row
    // we'll need to do it manually.
    for (uint32_t colIndex : colsWithEmptyCells)
        {
        AnnotationTableColumnP  column   = GetTable().GetColumn (colIndex);
        double                  minWidth = column->GetMinimumWidth(false);

        /*---------------------------------------------------------------------
          Don't use SetWidthDirect because we don't know if the whole column is empty.
        ---------------------------------------------------------------------*/
        if (column->GetWidth() < minWidth)
            column->SetWidth (minWidth);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AnnotationTableRow::SetHeaderFooterType (TableHeaderFooterType newType)
    {
    if (SUCCESS != GetTable().ChangeRowHeaderFooterType (m_index, newType))
        return ERROR;

    ApplyHeaderFooterType();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableColumn::ApplyHeaderFooterType ()
    {
    bool                    colIsEmpty  = true;
    bvector<uint32_t>       rowsWithEmptyCells;

    /*---------------------------------------------------------------------
        Apply the appropriate 'region' TextStyle to every cell in the column.
    ---------------------------------------------------------------------*/
    for (uint32_t rowIndex = 0; rowIndex < GetTable().GetRowCount(); rowIndex++)
        {
        AnnotationTableCellP  cell = GetTable().GetCell (AnnotationTableCellIndex (rowIndex, m_index));

        if (NULL == cell)
            continue;

        if (nullptr == cell->GetTextBlock())
            {
            rowsWithEmptyCells.push_back (rowIndex);
            continue;
            }

        cell->ApplyTextStyleByRegion ();
        colIsEmpty = false;
        }

    // Setting textBlocks will recompute the column width but for empty columns need to do it manually.
    if (colIsEmpty)
        {
        if (GetWidthLock())
            {
            double minWidth = GetMinimumWidth(false);

            if (GetWidth() < minWidth)
                SetWidthDirect (minWidth);
            }
        else
            {
            AnnotationTableRegion   region    = AnnotationTableElement::GetTableRegionFromColumnType (GetHeaderFooterType());
            AnnotationTextStyleCP   textStyle = GetTable().GetTextStyle (region);
            TableCellMarginValues   margins   = GetTable().GetDefaultMargins ();
            double                  width     = textStyle->GetHeight () * textStyle->GetWidthFactor ();

            // Here we know the entire column is empty so its ok to set the width directly
            SetWidthDirect (10 * width + margins.m_left + margins.m_right);
            }
        }

    // Setting textBlocks will recompute the row heights but if the row has an empty cell in this column
    // we'll need to do it manually.
    for (uint32_t rowIndex : rowsWithEmptyCells)
        {
        AnnotationTableRowP row       = GetTable().GetRow (rowIndex);
        double              minHeight = row->GetMinimumHeight(false);

        /*---------------------------------------------------------------------
          Don't use SetHeightDirect because we don't know if the whole row is empty.
        ---------------------------------------------------------------------*/
        if (row->GetHeight() < minHeight)
            row->SetHeight (minHeight);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AnnotationTableColumn::SetHeaderFooterType (TableHeaderFooterType newType)
    {
    if (SUCCESS != GetTable().ChangeColumnHeaderFooterType (m_index, newType))
        return ERROR;

    ApplyHeaderFooterType();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::SetTextStyleIdDirect (DgnElementId val, AnnotationTableRegion region)
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
void            AnnotationTableElement::SetTextStyleId (DgnElementId val, AnnotationTableRegion region)
    {
    SetTextStyleIdDirect (val, region);
#if defined (NEEDSWORK)
    SetTextHeightOverride (NULL, region);
#endif

    bool                    isRow;
    TableHeaderFooterType   type;

    ClassifyTableRegion (type, isRow, region);

    if (isRow)
        {
        for (AnnotationTableRowR row : m_rows)
            {
            if (row.GetHeaderFooterType() != type)
                continue;

            row.ApplyHeaderFooterType ();
            }
        }
    else
        {
        for (AnnotationTableColumnR col : m_columns)
            {
            if (col.GetHeaderFooterType() != type)
                continue;

            col.ApplyHeaderFooterType ();
            }
        }
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::SetDefaultLineColor (ColorDef val)
    {
    SymbologyEntryP symbology = m_symbologyDictionary.GetSymbology(0);

    if (UNEXPECTED_CONDITION (NULL == symbology))
        return;

    symbology->SetColor (val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::SetDefaultLineWeight (uint32_t val)
    {
    SymbologyEntryP symbology = m_symbologyDictionary.GetSymbology(0);

    if (UNEXPECTED_CONDITION (NULL == symbology))
        return;

    symbology->SetWeight (val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::SetDefaultLineStyle (DgnStyleId id, double scale)
    {
    SymbologyEntryP symbology = m_symbologyDictionary.GetSymbology(0);

    if (UNEXPECTED_CONDITION (NULL == symbology))
        return;

    symbology->SetLineStyle (id, scale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::SetDefaultFill (AnnotationTableSymbologyValuesCR val, TableRows rows)
    {
    bool    wantOdd  = false;
    bool    wantEven = false;

    switch (rows)
        {
        case TableRows::Odd:    wantOdd  = true;            break;
        case TableRows::Even:   wantEven = true;            break;
        default:                wantOdd = wantEven = true;  break;
        }

    uint32_t  newKey = 0;

    if (val.HasFillColor())
        {
        SymbologyEntry   newSymbology (*this, 0);
        newSymbology.SetFillColor (val.GetFillColor());

        newKey = m_symbologyDictionary.FindOrAddSymbology (newSymbology);
        }

    if (wantOdd)
        SetFillSymbologyForOddRow (newKey);

    if (wantEven)
        SetFillSymbologyForEvenRow (newKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::ChangeDefaultTextSymbology (TextSymb textSymb, TextSymbAction action, uint32_t value)
    {
    uint32_t    oldKey = GetDefaultTextSymbology();

    if (0 == oldKey)
        {
        if (TextSymbAction::Clear == action)
            return;

        SymbologyEntry newSymbology(*this, 0);
        newSymbology.SetVisible(true);

        if (TextSymb::Color == textSymb)
            newSymbology.SetColor(ColorDef(value));
        else
            newSymbology.SetWeight(value);

        uint32_t newKey = m_symbologyDictionary.FindOrAddSymbology (newSymbology);
        SetDefaultTextSymbology (newKey);
        return;
        }

    SymbologyEntryCP fromDictionary = m_symbologyDictionary.GetSymbology(oldKey);

    if (UNEXPECTED_CONDITION (NULL == fromDictionary))
        return;

    bool        isStored = false;
    uint32_t    storedValue = 0;

    if (TextSymb::Color == textSymb)
        {
        isStored    = fromDictionary->HasColor();
        storedValue = fromDictionary->GetColor().GetValue();
        }
    else
        {
        isStored = fromDictionary->HasWeight();
        storedValue = fromDictionary->GetWeight();
        }

    bool needToChange = false;

    if (TextSymbAction::Store == action)
        needToChange = (isStored == false) || storedValue != value;
    else
        needToChange = (isStored == true);

    if ( ! needToChange)
        return;

    SymbologyEntry  newSymbology (*fromDictionary, true);

    if (TextSymbAction::Clear == action)
        (TextSymb::Color == textSymb) ? newSymbology.ClearColor() : newSymbology.ClearWeight();
    else
        (TextSymb::Color == textSymb) ? newSymbology.SetColor(ColorDef(value)) : newSymbology.SetWeight(value);

    uint32_t newKey = 0;
    if (newSymbology.HasColor() || newSymbology.HasWeight())
        newKey = m_symbologyDictionary.FindOrAddSymbology (newSymbology);

    SetDefaultTextSymbology (newKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::SetDefaultTextColor (ColorDef value)  { ChangeDefaultTextSymbology (TextSymb::Color,  TextSymbAction::Store, value.GetValue()); }
void            AnnotationTableElement::SetDefaultTextWeight (uint32_t value) { ChangeDefaultTextSymbology (TextSymb::Weight, TextSymbAction::Store, value); }
void            AnnotationTableElement::ClearDefaultTextColor ()              { ChangeDefaultTextSymbology (TextSymb::Color,  TextSymbAction::Clear, 0); }
void            AnnotationTableElement::ClearDefaultTextWeight ()             { ChangeDefaultTextSymbology (TextSymb::Weight, TextSymbAction::Clear, 0); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
/* ctor */ AnnotationTableElement::AnnotationTableElement(CreateParams const& params) : T_Super(params), m_tableHeader (*this) { }
AnnotationTableElementPtr AnnotationTableElement::Create(CreateParams const& params) { return new AnnotationTableElement(params); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
AnnotationTableElementPtr AnnotationTableElement::Create (uint32_t rowCount, uint32_t columnCount, DgnElementId textStyleId, double backupTextHeight, CreateParams const& params)
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

    AnnotationTableEdgeRun topEdgeRun (*this);
    topEdgeRun.Initialize (EdgeRunHostType::Top, true);
    m_topEdgeRuns.push_back (topEdgeRun);

    AnnotationTableEdgeRun leftEdgeRun (*this);
    leftEdgeRun.Initialize (EdgeRunHostType::Left, false);
    m_leftEdgeRuns.push_back (leftEdgeRun);

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

        SymbologyEntry symbology(*this, 0);
        symbology.SetKey (0);   // needed to set the HasChanges flag
        m_symbologyDictionary.AddSymbology (symbology);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableCellCollection   AnnotationTableElement::GetCellCollection () const
    {
    return AnnotationTableCellCollection (*this);
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
        AnnotationTableCellIndex    newRootIndex (rowIndex + 1, rootIndex.col);
        AnnotationTableCellP        newRoot = GetCell (newRootIndex, true);

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
        DeleteAspect(edgeRun);

    // Not safe to use this anymore
    oldRow = NULL;

    // Remove the instance for the deleted row, it's cells and it's edgeRuns
    bvector<AnnotationTableRow>::iterator erasePos = m_rows.begin() + rowIndex;
    DeleteAspect (*erasePos);

    for (AnnotationTableCellR cell: erasePos->GetCellVectorR())
        DeleteAspect (cell);

    for (AnnotationTableEdgeRunR edgeRun: erasePos->GetEdgeRuns())
        DeleteAspect (edgeRun);

    // fixup the affected objects
    m_rows.erase (erasePos);

    uint32_t  adjustedIndex = rowIndex;
    for (bvector<AnnotationTableRow>::iterator rowIter = erasePos; rowIter < m_rows.end(); rowIter++)
        rowIter->SetIndex (adjustedIndex++);

    m_mergeDictionary.AdjustMergesAfterIndex (rowIndex, true, false);

    SetRowCount (static_cast <uint32_t> (m_rows.size()));

    // Layout any cells that span the new row.
    for (AnnotationTableCellP const& cell: cellsWithHeightChanges)
        cell->SetSizeFromContents(nullptr);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AnnotationTableElement::InsertRow (uint32_t indexOfSeedRow, TableInsertDirection direction)
    {
    uint32_t              indexOfNewRow = TableInsertDirection::Before == direction ? indexOfSeedRow : indexOfSeedRow + 1;
    AnnotationTableRowCP  seedRow = GetRow (indexOfSeedRow);

    if (NULL == seedRow)
        return ERROR;

    // Adjust the title/header/footer count if necessary.
    BumpRowHeaderFooterCount (*seedRow, true);

    bvector <AnnotationTableCellP> cellsWithHeightChanges;

    // Fix rowSpan for any cells that span the row.
    if (GetRowCount() != indexOfNewRow)
        {
        for (AnnotationTableCellP const& cell: GetRow(indexOfNewRow)->FindCells())
            {
            uint32_t  rowSpan = cell->GetRowSpan();

            if (1 == rowSpan)  // not a merged cell
                continue;

            // if this is the root of a merged cell, nothing to do it just shifts down.
            if (cell->GetIndex().row == indexOfNewRow)
                continue;

            cell->SetAsMergedCellRoot (rowSpan + 1, cell->GetColumnSpan());
            cellsWithHeightChanges.push_back (cell);
            }
        }

    // Expand the vertical edgeRuns that span the row.
    GetLeftEdgeRuns().InsertSpan (indexOfNewRow, 1, EdgeRunInitializer (*this, EdgeRunHostType::Left));

    for (AnnotationTableColumnR column: m_columns)
        column.GetEdgeRuns().InsertSpan (indexOfNewRow, 1, EdgeRunInitializer (*this, EdgeRunHostType::Column, column.GetIndex()));

    // Insert the Row and create its vector of cells
    AnnotationTableRow newRow (*this, indexOfNewRow);

    bvector<AnnotationTableRow>::iterator insertPos = m_rows.begin() + indexOfNewRow;
    m_rows.insert (insertPos, newRow);
    m_rows[indexOfNewRow].InitializeInternalCollections();

    // Adjust all the subsequent rows, this will also adjust all the cells in the row
    uint32_t  adjustedIndex = indexOfNewRow+1;
    insertPos = m_rows.begin() + indexOfNewRow;
    for (bvector<AnnotationTableRow>::iterator rowIter = insertPos + 1; rowIter < m_rows.end(); rowIter++)
        rowIter->SetIndex (adjustedIndex++);

    m_mergeDictionary.AdjustMergesAfterIndex (indexOfNewRow, true, true);

    SetRowCount (static_cast <uint32_t> (m_rows.size()));

    // Copy settings from the seed row
    if (TableInsertDirection::Before == direction)
        indexOfSeedRow++;

    CopyPropsForNewRow (indexOfNewRow, indexOfSeedRow);

    // Layout any cells that span the new row.
    for (AnnotationTableCellP const& cell: cellsWithHeightChanges)
        cell->SetSizeFromContents(nullptr);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void        AnnotationTableElement::CopyPropsForNewCell (AnnotationTableCellR newCell, AnnotationTableCellCR seedCell)
    {
    newCell.SetAlignment   (seedCell.GetAlignment());
    newCell.SetOrientation (seedCell.GetOrientation());
    newCell.SetMargins     (seedCell.GetMargins());

    AnnotationTableSymbologyValues symb;
    seedCell.GetFillSymbology (symb);
    newCell.SetFillSymbology (symb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void        AnnotationTableElement::CopyPropsForNewRow (uint32_t indexOfNewRow, uint32_t indexOfSeedRow)
    {
    AnnotationTableRowP   newRow  = GetRow (indexOfNewRow);
    AnnotationTableRowCP  seedRow = GetRow (indexOfSeedRow);

    // Copy row properties from the seed
    newRow->SetHeight (seedRow->GetHeight());
    newRow->SetHeightLock (seedRow->GetHeightLock());

    bvector<AnnotationTableCellP> seedCells = seedRow->FindCells();

    for (AnnotationTableCellCP seedCell : seedCells)
        {
        AnnotationTableCellIndexCR    seedIndex = seedCell->GetIndex();

        // Skip this one if the the new cell is an interior of the seed
        if (indexOfNewRow > seedIndex.row &&
            indexOfNewRow < seedIndex.row + seedCell->GetRowSpan())
            continue;

        AnnotationTableCellIndex  newCellIndex = AnnotationTableCellIndex (indexOfNewRow, seedIndex.col);
        AnnotationTableCellP      newCell = GetCell (newCellIndex);

        // Copy the colSpan from the seed
        uint32_t colSpan = seedCell->GetColumnSpan();

        if (1 < colSpan)
            {
            newCell->SetAsMergedCellRoot (1, colSpan);
            MarkAsMergedCellInteriors (newCellIndex, 1, colSpan, false);
            }

        // Copy cell properties from the seed
        CopyPropsForNewCell (*newCell, *seedCell);
        }

    // For the new horizontal edges, copy symbology from the seed
    EdgeRunsR   newEdges  = newRow->GetEdgeRuns (false);    // new edge is always the bottom
    EdgeRunsCR  seedEdges = newRow->GetEdgeRuns (true);     // seed edge is always the top
    newEdges.ApplySeed (seedEdges);

    // For the new vertical edges, copy symbology from the seed
    GetLeftEdgeRuns().ApplySeedAtIndex (indexOfNewRow, 1, indexOfSeedRow);
    GetLeftEdgeRuns().MergeRedundantRuns (this);

    for (AnnotationTableColumnR column : m_columns)
        {
        column.GetEdgeRuns().ApplySeedAtIndex (indexOfNewRow, 1, indexOfSeedRow);
        column.GetEdgeRuns().MergeRedundantRuns (this);
        }
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
        AnnotationTableCellIndex    newRootIndex (rootIndex.row, colIndex + 1);
        AnnotationTableCellP        newRoot = GetCell (newRootIndex, true);

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
        this->DeleteAspect (edgeRun);

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

    m_mergeDictionary.AdjustMergesAfterIndex (colIndex, false, false);

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
BentleyStatus   AnnotationTableElement::InsertColumn (uint32_t indexOfSeedColumn, TableInsertDirection direction)
    {
    uint32_t                  indexOfNewColumn = TableInsertDirection::Before == direction ? indexOfSeedColumn : indexOfSeedColumn + 1;
    AnnotationTableColumnCP   seedColumn = GetColumn (indexOfSeedColumn);

    if (NULL == seedColumn)
        return ERROR;

    // Since a table can only have at most one 'first' column and 'last' column, we don't bump
    // the count like we do for rows.  Instead the firstness or lastness transfers to the new column
    // so the count stays the same.  And then we apply the body style to the original seed.
    TableHeaderFooterType originalSeedType = seedColumn->GetHeaderFooterType();

    bvector <AnnotationTableCellP> cellsWithWidthChanges;

    // Fix colSpan for any cells that span the column.
    if (GetColumnCount() != indexOfNewColumn)
        {
        for (AnnotationTableCellP const& cell: GetColumn(indexOfNewColumn)->FindCells())
            {
            uint32_t  colSpan = cell->GetColumnSpan();

            if (1 == colSpan)  // not a merged cell
                continue;

            // if this is the root of a merged cell, nothing to do it just shifts over.
            if (cell->GetIndex().col == indexOfNewColumn)
                continue;

            cell->SetAsMergedCellRoot (cell->GetRowSpan(), colSpan + 1);
            cellsWithWidthChanges.push_back (cell);
            }
        }

    // Expand the horizontal edgeRuns and fillRuns that span the column.
    GetTopEdgeRuns().InsertSpan (indexOfNewColumn, 1, EdgeRunInitializer(*this, EdgeRunHostType::Top));

    for (AnnotationTableRowR row: m_rows)
        {
        row.GetEdgeRuns().InsertSpan (indexOfNewColumn, 1, EdgeRunInitializer (*this, EdgeRunHostType::Row, row.GetIndex()));
        row.GetFillRuns().InsertSpan (indexOfNewColumn, 1, FillRunInitializer (row.GetIndex()));
        }

    // Insert the new column
    AnnotationTableColumn newColumn (*this, indexOfNewColumn);

    bvector<AnnotationTableColumn>::iterator columnInsertPos = m_columns.begin() + indexOfNewColumn;
    m_columns.insert (columnInsertPos, newColumn);

    // Adjust all the subsequent columns
    uint32_t  adjustedIndex = indexOfNewColumn+1;
    columnInsertPos = m_columns.begin() + indexOfNewColumn;
    for (bvector<AnnotationTableColumn>::iterator colIter = columnInsertPos + 1; colIter < m_columns.end(); colIter++)
        colIter->SetIndex (adjustedIndex++);

    for (AnnotationTableRow& row: m_rows)
        {
        // Add the new cell
        AnnotationTableCell           newCell (*this, AnnotationTableCellIndex (row.GetIndex(), indexOfNewColumn));
        bvector<AnnotationTableCell>& cells = row.GetCellVectorR();

        bvector<AnnotationTableCell>::iterator cellInsertPos = cells.begin() + indexOfNewColumn;
        cells.insert (cellInsertPos, newCell);

        // Adjust all the subsequent cells
        adjustedIndex = indexOfNewColumn+1;
        cellInsertPos = cells.begin() + indexOfNewColumn;
        for (bvector<AnnotationTableCell>::iterator cellIter = cellInsertPos + 1; cellIter < cells.end(); cellIter++)
            cellIter->SetIndex (AnnotationTableCellIndex (row.GetIndex(), adjustedIndex++));
        }

    m_mergeDictionary.AdjustMergesAfterIndex (indexOfNewColumn, false, true);
    
    SetColumnCount (static_cast <uint32_t> (m_columns.size()));

    // Copy settings from the seed column
    if (TableInsertDirection::Before == direction)
        indexOfSeedColumn++;

    CopyPropsForNewColumn (indexOfNewColumn, indexOfSeedColumn);

    // Layout any cells that span the new column.
    for (AnnotationTableCellP const& cell : cellsWithWidthChanges)
        cell->WidthChanged();

    for (AnnotationTableCellP const& cell : GetColumn(indexOfNewColumn)->FindCells())
        cell->ApplyToFillRuns();

    AnnotationTableColumnP   postSeedColumn = GetColumn (indexOfSeedColumn);

    if (postSeedColumn->GetHeaderFooterType() != originalSeedType)
        postSeedColumn->ApplyHeaderFooterType ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void        AnnotationTableElement::CopyPropsForNewColumn (uint32_t indexOfNewColumn, uint32_t indexOfSeedColumn)
    {
    AnnotationTableColumnP   newColumn  = GetColumn (indexOfNewColumn);
    AnnotationTableColumnCP  seedColumn = GetColumn (indexOfSeedColumn);

    // Copy col properties from the seed
    newColumn->SetWidth (seedColumn->GetWidth());
    newColumn->SetWidthLock (seedColumn->GetWidthLock());

    bvector<AnnotationTableCellP> seedCells = seedColumn->FindCells();

    for (AnnotationTableCellCP seedCell : seedCells)
        {
        AnnotationTableCellIndexCR    seedIndex = seedCell->GetIndex();

        // Skip this one if the the new cell is an interior of the seed
        if (indexOfNewColumn > seedIndex.col &&
            indexOfNewColumn < seedIndex.col + seedCell->GetColumnSpan())
            continue;

        AnnotationTableCellIndex  newCellIndex = AnnotationTableCellIndex (seedIndex.row, indexOfNewColumn);
        AnnotationTableCellP      newCell = GetCell (newCellIndex);

        // Copy the rowSpan from the seed
        uint32_t rowSpan = seedCell->GetRowSpan();

        if (1 < rowSpan)
            {
            newCell->SetAsMergedCellRoot (rowSpan, 1);
            MarkAsMergedCellInteriors (newCellIndex, rowSpan, 1, false);
            }

        // Copy cell properties from the seed
        CopyPropsForNewCell (*newCell, *seedCell);
        }

    // For the new vertical edges, copy symbology from the seed
    EdgeRunsR   newEdges  = newColumn->GetEdgeRuns (false);    // new edge is always the right
    EdgeRunsCR  seedEdges = newColumn->GetEdgeRuns (true);     // seed edge is always the left
    newEdges.ApplySeed (seedEdges);

    // For the new horizontal edges, copy symbology from the seed
    GetTopEdgeRuns().ApplySeedAtIndex (indexOfNewColumn, 1, indexOfSeedColumn);
    GetTopEdgeRuns().MergeRedundantRuns (this);

    for (AnnotationTableRowR row : m_rows)
        {
        row.GetEdgeRuns().ApplySeedAtIndex (indexOfNewColumn, 1, indexOfSeedColumn);
        row.GetEdgeRuns().MergeRedundantRuns (this);
        }
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

    if ( ! allowMergedInteriors && cell.IsMergedCellInterior())
        return NULL;

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
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool            AnnotationTableElement::HasOverlappingMerges (bvector<AnnotationTableCellP>& consumedRoots, AnnotationTableCellIndexCR rootIndex, uint32_t numRows, uint32_t numCols)
    {
    uint32_t  maxRowIndex = rootIndex.row + numRows;
    uint32_t  maxColIndex = rootIndex.col + numCols;

    // Examine every cell in the proposed merge block
    for (uint32_t iRow = rootIndex.row; iRow < rootIndex.row + numRows; iRow++)
        {
        for (uint32_t iCol = rootIndex.col; iCol < rootIndex.col + numCols; iCol++)
            {
            AnnotationTableCellIndex    interiorIndex (iRow, iCol);
            AnnotationTableCellP        interiorCell = GetCell(interiorIndex);

            // If this cell was already part of a merge block...
            if (nullptr == interiorCell)
                {
                bool rootInConsumedList = false;

                // Check if we've already encountered the old root of this cell
                for (AnnotationTableCellP const& consumedRoot : consumedRoots)
                    {
                    if (consumedRoot->IndexIsInSpan(interiorIndex))
                        {
                        rootInConsumedList = true;
                        break;
                        }
                    }

                // If we haven't yet seen the old root, we won't, which means it
                // lies outside the proposed block.  Which means it's overlapping.
                if ( ! rootInConsumedList)
                    return true;

                continue;
                }

            // This cell is it's own root.
            uint32_t          interiorRowSpan = interiorCell->GetRowSpan();
            uint32_t          interiorColSpan = interiorCell->GetColumnSpan();

            if (1 == interiorRowSpan && 1 == interiorColSpan)
                continue;

            // Test if the cell's old block extends outside the proposed block.  If
            // it does, then it's an overlap.
            if (interiorIndex.row + interiorRowSpan > maxRowIndex)
                return true;

            if (interiorIndex.col + interiorColSpan > maxColIndex)
                return true;

            // Remember the roots that we've consumed so we know what to do when we
            // get to their old interiors.
            consumedRoots.push_back (interiorCell);
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            AnnotationTableElement::MarkAsMergedCellInteriors (AnnotationTableCellIndexCR rootIndex, uint32_t rowSpan, uint32_t colSpan, bool loading)
    {
    for (uint32_t iRow = rootIndex.row; iRow < rootIndex.row + rowSpan; iRow++)
        {
        for (uint32_t iCol = rootIndex.col; iCol < rootIndex.col + colSpan; iCol++)
            {
            AnnotationTableCellIndex  index (iRow, iCol);

            if (index == rootIndex)
                continue;

            AnnotationTableCellP  cell = GetCell(index);

            if (NULL == cell)  // it's already an interior
                continue;

            cell->SetAsMergedCellInterior (true);

            if ( ! loading)
                {
                DeleteAspect (*cell);
                continue;
                }

            // sanity check that we didn't already load an aspect for this cell
            BeAssert ( ! cell->HasValidAspectId());
            }
        }

    EdgeRuns    removedRuns;

    for (uint32_t iRow = rootIndex.row; iRow < rootIndex.row + rowSpan - 1; iRow++)
        {
        AnnotationTableRowP   row = GetRow (iRow);
        row->GetEdgeRuns().CreateGap (&removedRuns, rootIndex.col, colSpan);
        }

    for (uint32_t iCol = rootIndex.col; iCol < rootIndex.col + colSpan - 1; iCol++)
        {
        AnnotationTableColumnP   column = GetColumn (iCol);
        column->GetEdgeRuns().CreateGap (&removedRuns, rootIndex.row, rowSpan);
        }

    if ( ! loading)
        {
        for (AnnotationTableEdgeRunR edgeRun: removedRuns)
            DeleteAspect (edgeRun);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AnnotationTableElement::MergeCells (AnnotationTableCellIndexCR rootIndex, uint32_t numRows, uint32_t numCols)
    {
    if (GetRowCount()    <= rootIndex.row ||
        GetColumnCount() <= rootIndex.col)
        {
        return ERROR;
        }

    if (GetRowCount()    <= rootIndex.row + numRows - 1 ||
        GetColumnCount() <= rootIndex.col + numCols - 1)
        {
        return ERROR;
        }

    if (0 == numRows || 0 == numCols)
        return ERROR;

    if (2 > numRows && 2 > numCols)
        return ERROR;

    bvector<AnnotationTableCellP> consumedRoots;
    if (HasOverlappingMerges (consumedRoots, rootIndex, numRows, numCols))
        return ERROR;

    // Do this so the xattribute will be removed.
    for (AnnotationTableCellP const& consumedRoot : consumedRoots)
        consumedRoot->SetAsMergedCellRoot (1, 1);

    AnnotationTableCellP  rootCell = GetCell (rootIndex);
    rootCell->SetAsMergedCellRoot (numRows, numCols);

    MarkAsMergedCellInteriors (rootIndex, numRows, numCols, false);

    // Will set the verticalSpan in the main row, and open gaps on subsequent rows.
    rootCell->ApplyToFillRuns();

    if (1 < numCols)
        rootCell->WidthChanged();

    return SUCCESS;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CellEdgeAccessor
    {
protected:
    enum    TableCellEdgeId
        {
        EDGEID_Top      = 0,
        EDGEID_Right    = 1,
        EDGEID_Bottom   = 2,
        EDGEID_Left     = 3,
        };

    /*=================================================================================**//**
    +===============+===============+===============+===============+===============+======*/
    struct  Entry
        {
        AnnotationTableCellIndex  m_cellIndex;
        bool                      m_edgeExists[4];
        bool                      m_edgeProcessed[4];

        /* ctor */  Entry (AnnotationTableCellIndex index, bool hasTop, bool hasRight, bool hasBot, bool hasLeft);
        };

    /*=================================================================================**//**
    +===============+===============+===============+===============+===============+======*/
    struct  DoEdgeStatus
        {
        BentleyStatus   m_status;   // edge was successfully processed can skip mated edge if any
        bool            m_continue; // false to stop processing

        DoEdgeStatus (BentleyStatus s, bool c) : m_status(s), m_continue(c) {}
        };

    AnnotationTableElementCR    m_table;
    bvector<Entry>              m_entries;

    /* ctor */      CellEdgeAccessor (AnnotationTableElementCR table) : m_table (table) {}

private:
    TableCellEdgeId GetOpposingEdgeId (TableCellEdgeId);
    Entry*          FindEntry (AnnotationTableCellIndexCR index);
    Entry*          FindAdjacentEntry (TableCellEdgeId edgeId, AnnotationTableCellIndexCR index);
    bool            DoEdge (Entry* matedEntry, TableCellEdgeId primaryEdge, Entry& entry);
    bool            NeedToDoEdge (Entry*& matedEntry, TableCellEdgeId primaryEdge, TableCellListEdges edgesToProcess, Entry& entry);
    bool            ProcessOneEdge (TableCellEdgeId primaryEdge, TableCellListEdges edgesToProcess, Entry& entry);

protected:
    bool            IsHorizontal (TableCellEdgeId);
    EdgeRunsCP      FindEdgeRun (uint32_t& startIndex, TableCellEdgeId, AnnotationTableCellIndexCR);

    virtual DoEdgeStatus    _DoEdge (TableCellEdgeId primaryEdge, Entry& entry) = 0;

public:
    void            AddCell (AnnotationTableCellIndexCR cellIndex);
    void            ProcessCells (TableCellListEdges edgesToProcess);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
/* ctor */ CellEdgeAccessor::Entry::Entry (AnnotationTableCellIndex index, bool hasTop, bool hasRight, bool hasBot, bool hasLeft)
    {
    m_cellIndex                 = index;
    m_edgeExists[EDGEID_Top]    = hasTop;
    m_edgeExists[EDGEID_Right]  = hasRight;
    m_edgeExists[EDGEID_Bottom] = hasBot;
    m_edgeExists[EDGEID_Left]   = hasLeft;

    m_edgeProcessed[EDGEID_Top]    = false;
    m_edgeProcessed[EDGEID_Right]  = false;
    m_edgeProcessed[EDGEID_Bottom] = false;
    m_edgeProcessed[EDGEID_Left]   = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
EdgeRunsCP      CellEdgeAccessor::FindEdgeRun (uint32_t& startIndex, TableCellEdgeId edgeId, AnnotationTableCellIndexCR cellIndex)
    {
    EdgeRunHostType hostType;
    uint32_t          hostIndex;

    switch (edgeId)
        {
        case EDGEID_Top:
            {
            hostType   = 0 != cellIndex.row ? EdgeRunHostType::Row : EdgeRunHostType::Top;
            hostIndex  = cellIndex.row - 1;
            startIndex = cellIndex.col;
            break;
            }
        case EDGEID_Bottom:
            {
            hostType   = EdgeRunHostType::Row;
            hostIndex  = cellIndex.row;
            startIndex = cellIndex.col;
            break;
            }
        case EDGEID_Left:
            {
            hostType   = 0 != cellIndex.col ? EdgeRunHostType::Column : EdgeRunHostType::Left;
            hostIndex  = cellIndex.col - 1;
            startIndex = cellIndex.row;
            break;
            }
        case EDGEID_Right:
            {
            hostType   = EdgeRunHostType::Column;
            hostIndex  = cellIndex.col;
            startIndex = cellIndex.row;
            break;
            }
        default:
            {
            BeAssert(false);
            return NULL;
            }
        }

    return m_table.GetEdgeRuns (hostType, hostIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool    CellEdgeAccessor::IsHorizontal (TableCellEdgeId primaryEdgeId)
    {
    switch (primaryEdgeId)
        {
        default:                BeAssert(false);        //FALLTHRU
        case EDGEID_Top:        return true;
        case EDGEID_Right:      return false;
        case EDGEID_Bottom:     return true;
        case EDGEID_Left:       return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
CellEdgeAccessor::TableCellEdgeId  CellEdgeAccessor::GetOpposingEdgeId (TableCellEdgeId primaryEdgeId)
    {
    switch (primaryEdgeId)
        {
        default:                BeAssert(false);        //FALLTHRU
        case EDGEID_Top:        return EDGEID_Bottom;
        case EDGEID_Right:      return EDGEID_Left;
        case EDGEID_Bottom:     return EDGEID_Top;
        case EDGEID_Left:       return EDGEID_Right;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    CellEdgeAccessor::AddCell (AnnotationTableCellIndexCR cellIndex)
    {
    AnnotationTableCellCP  cell    = m_table.GetCell (cellIndex, true);

    if (UNEXPECTED_CONDITION (NULL == cell))
        return;

    uint32_t          rowSpan = cell->GetRowSpan();
    uint32_t          colSpan = cell->GetColumnSpan();

    for (uint32_t iRow = 0; iRow < rowSpan; iRow++)
        {
        for (uint32_t iCol = 0; iCol < colSpan; iCol++)
            {
            bool            hasTop   = 0           == iRow;
            bool            hasBot   = rowSpan - 1 == iRow;
            bool            hasLeft  = 0           == iCol;
            bool            hasRight = colSpan - 1 == iCol;

            if ( ! hasTop && ! hasRight && ! hasBot && ! hasLeft)
                continue;

            AnnotationTableCellIndex  index (cellIndex.row + iRow, cellIndex.col + iCol);
            Entry           entry (index, hasTop, hasRight, hasBot, hasLeft);

            m_entries.push_back (entry);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
CellEdgeAccessor::Entry* CellEdgeAccessor::FindEntry (AnnotationTableCellIndexCR index)
    {
    struct FindHostPredicate
        {
        AnnotationTableCellIndexCR  m_index;

        FindHostPredicate (AnnotationTableCellIndexCR m_index) : m_index (m_index) {}
        bool operator () (Entry const& entry) { return entry.m_cellIndex == m_index; }
        };

    FindHostPredicate   predicate (index);
    bvector<Entry>::iterator foundIter = std::find_if (m_entries.begin (), m_entries.end (), predicate);

    if (foundIter == m_entries.end())
        return NULL;

    return foundIter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
CellEdgeAccessor::Entry* CellEdgeAccessor::FindAdjacentEntry (TableCellEdgeId primaryEdge, AnnotationTableCellIndexCR index)
    {
    AnnotationTableCellIndex adjacentIndex = index;

    if (0 == index.row && EDGEID_Top == primaryEdge)
        return NULL;

    if (0 == index.col && EDGEID_Left == primaryEdge)
        return NULL;

    switch (primaryEdge)
        {
        case EDGEID_Top:        adjacentIndex.row -= 1;     break;
        case EDGEID_Bottom:     adjacentIndex.row += 1;     break;
        case EDGEID_Left:       adjacentIndex.col -= 1;     break;
        case EDGEID_Right:      adjacentIndex.col += 1;     break;
        default:                BeAssert (false);           return NULL;
        }

    return FindEntry (adjacentIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool    CellEdgeAccessor::NeedToDoEdge (Entry*& matedEntry, TableCellEdgeId primaryEdge, TableCellListEdges edgesToProcess, Entry& entry)
    {
    matedEntry = NULL;

    if ( ! entry.m_edgeExists[primaryEdge])
        return false;

    if (entry.m_edgeProcessed[primaryEdge])
        return false;

    switch (edgesToProcess)
        {
        case TableCellListEdges::Top:
            {
            return EDGEID_Top == primaryEdge;
            }
        case TableCellListEdges::Right:
            {
            return EDGEID_Right == primaryEdge;
            }
        case TableCellListEdges::Bottom:
            {
            return EDGEID_Bottom == primaryEdge;
            }
        case TableCellListEdges::Left:
            {
            return EDGEID_Left == primaryEdge;
            }
        case TableCellListEdges::All:
            {
            matedEntry = FindAdjacentEntry (primaryEdge, entry.m_cellIndex);
            return true;
            }
        case TableCellListEdges::Interior:
        case TableCellListEdges::InteriorHorizontal:
        case TableCellListEdges::InteriorVertical:
        case TableCellListEdges::Exterior:
            {
            matedEntry = FindAdjacentEntry (primaryEdge, entry.m_cellIndex);

            if (TableCellListEdges::Exterior == edgesToProcess)
                return NULL == matedEntry;

            // No adjacent entry, this is an exterior edge. Skip it.
            if (NULL == matedEntry)
                return false;

            // Its an interior, but we already processed it. Skip it.
            TableCellEdgeId  opposingEdge = GetOpposingEdgeId (primaryEdge);

            if (matedEntry->m_edgeProcessed[opposingEdge])
                return false;

            if (TableCellListEdges::InteriorHorizontal == edgesToProcess &&  ! IsHorizontal (primaryEdge))
                return false;

            if (TableCellListEdges::InteriorVertical == edgesToProcess &&  IsHorizontal (primaryEdge))
                return false;

            return true;
            }
        }

    BeAssert (false);
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool    CellEdgeAccessor::DoEdge (Entry* matedEntry, TableCellEdgeId primaryEdge, Entry& entry)
    {
    DoEdgeStatus status = _DoEdge (primaryEdge, entry);

    if (SUCCESS != status.m_status)
        return status.m_continue;

    if (matedEntry)
        {
        TableCellEdgeId  opposingEdge = GetOpposingEdgeId (primaryEdge);
        matedEntry->m_edgeProcessed[opposingEdge] = true;
        }

    return status.m_continue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool    CellEdgeAccessor::ProcessOneEdge (TableCellEdgeId primaryEdge, TableCellListEdges edgesToProcess, Entry& entry)
    {
    Entry*  matedEntry   = NULL;

    if ( ! NeedToDoEdge (matedEntry, primaryEdge, edgesToProcess, entry))
        return true;

    return DoEdge (matedEntry, primaryEdge, entry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    CellEdgeAccessor::ProcessCells (TableCellListEdges edgesToProcess)
    {
    for (Entry& entry : m_entries)
        {
        if ( ! ProcessOneEdge (EDGEID_Top, edgesToProcess, entry))
            break;

        if ( ! ProcessOneEdge (EDGEID_Right, edgesToProcess, entry))
            break;

        if ( ! ProcessOneEdge (EDGEID_Bottom, edgesToProcess, entry))
            break;

        if ( ! ProcessOneEdge (EDGEID_Left, edgesToProcess, entry))
            break;
        }
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InteriorEdgeFinder : CellEdgeAccessor
    {
private:
    bool    m_foundHorizontal;
    bool    m_foundVertical;

    virtual DoEdgeStatus    _DoEdge (TableCellEdgeId primaryEdge, Entry& entry) override;

public:
    bool            FoundHorizontal  () { return m_foundHorizontal; }
    bool            FoundVertical    () { return m_foundVertical;   }

    /* ctor */      InteriorEdgeFinder (AnnotationTableElementCR table) : CellEdgeAccessor (table), m_foundHorizontal(false), m_foundVertical(false) {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
CellEdgeAccessor::DoEdgeStatus  InteriorEdgeFinder::_DoEdge (TableCellEdgeId primaryEdge, Entry& entry)
    {
    if (IsHorizontal (primaryEdge))
        m_foundHorizontal = true;
    else
        m_foundVertical = true;

    bool canStop = m_foundHorizontal && m_foundVertical;

    return DoEdgeStatus (SUCCESS, ! canStop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void   AnnotationTableElement::HasInteriorEdges (bool* hasAny, bool* hasHorizontal, bool* hasVertical, bvector<AnnotationTableCellIndex> const& cells) const
    {
    InteriorEdgeFinder finder (*this);

    for (AnnotationTableCellIndexCR index : cells)
        finder.AddCell (index);

    finder.ProcessCells(TableCellListEdges::Interior);

    if (NULL != hasAny)
        *hasAny = finder.FoundHorizontal() || finder.FoundVertical();

    if (NULL != hasHorizontal)
        *hasHorizontal = finder.FoundHorizontal();

    if (NULL != hasVertical)
        *hasVertical = finder.FoundVertical();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CellEdgeSymbologyGetter : CellEdgeAccessor
    {
private:
    bset<uint32_t>    m_keys;

    virtual DoEdgeStatus    _DoEdge (TableCellEdgeId primaryEdge, Entry& entry) override;

public:
    /* ctor */      CellEdgeSymbologyGetter (AnnotationTableElementCR table) : CellEdgeAccessor (table) {}

    void            GetSymbologies (bvector<AnnotationTableSymbologyValues>&) const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
CellEdgeAccessor::DoEdgeStatus  CellEdgeSymbologyGetter::_DoEdge (TableCellEdgeId primaryEdge, Entry& entry)
    {
    uint32_t        startIndex;
    EdgeRunsCP      edgeRuns = FindEdgeRun (startIndex, primaryEdge, entry.m_cellIndex);

    if (UNEXPECTED_CONDITION (NULL == edgeRuns))
        return DoEdgeStatus (ERROR, true);

    AnnotationTableEdgeRunCP  edgeRun = edgeRuns->GetSpanningRun (startIndex);

    if (UNEXPECTED_CONDITION (NULL == edgeRun))
        return DoEdgeStatus (ERROR, true);

    uint32_t  key = edgeRun->GetSymbologyKey ();
    m_keys.insert (key);

    return DoEdgeStatus (SUCCESS, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    CellEdgeSymbologyGetter::GetSymbologies (bvector<AnnotationTableSymbologyValues>& symbs) const
    {
    SymbologyDictionary const&  dictionary = m_table.GetSymbologyDictionary();

    for (uint32_t key : m_keys)
        {
        SymbologyEntryCP entry = dictionary.GetSymbology (key);

        if (UNEXPECTED_CONDITION (NULL == entry))
            continue;

        AnnotationTableSymbologyValues values;

        if (entry->GetVisible())
            {
            values.SetLineColor  (ColorDef(entry->GetColor()));
            values.SetLineWeight (entry->GetWeight());
            values.SetLineStyle  (DgnStyleId(entry->GetLineStyleId()), entry->GetLineStyleScale());
            }
        else
            {
            values.SetLineVisible (false);
            }

        symbs.push_back (values);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void   AnnotationTableElement::GetEdgeSymbology (bvector<AnnotationTableSymbologyValues>& symb, TableCellListEdges edges, bvector<AnnotationTableCellIndex> const& cells) const
    {
    CellEdgeSymbologyGetter getter (*this);

    for (AnnotationTableCellIndexCR index : cells)
        getter.AddCell (index);

    getter.ProcessCells(edges);

    getter.GetSymbologies (symb);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CellEdgeSymbologySetter : CellEdgeAccessor
    {
private:
    AnnotationTableSymbologyValuesCR      m_symbology;

    virtual DoEdgeStatus    _DoEdge (TableCellEdgeId primaryEdge, Entry& entry) override;

public:
    /* ctor */      CellEdgeSymbologySetter (AnnotationTableElementR table, AnnotationTableSymbologyValuesCR symb) : CellEdgeAccessor (table), m_symbology(symb) {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/13
+---------------+---------------+---------------+---------------+---------------+------*/
CellEdgeAccessor::DoEdgeStatus  CellEdgeSymbologySetter::_DoEdge (TableCellEdgeId primaryEdge, Entry& entry)
    {
    uint32_t        startIndex;
    EdgeRunsCP      edgeRuns = FindEdgeRun (startIndex, primaryEdge, entry.m_cellIndex);

    if (UNEXPECTED_CONDITION (NULL == edgeRuns))
        return DoEdgeStatus (ERROR, true);

    EdgeRunsR               nonConstEdgeRuns = const_cast <EdgeRunsR> (*edgeRuns);
    AnnotationTableElementR nonConstTable    = const_cast <AnnotationTableElementR> (m_table);

    nonConstEdgeRuns.SetSymbology (m_symbology, startIndex, 1, nonConstTable.GetSymbologyDictionary());
    nonConstEdgeRuns.MergeRedundantRuns(&nonConstTable);

    return DoEdgeStatus (SUCCESS, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void   AnnotationTableElement::SetEdgeSymbology (AnnotationTableSymbologyValuesCR symb, TableCellListEdges edges, bvector<AnnotationTableCellIndex> const& cells)
    {
    CellEdgeSymbologySetter setter (*this, symb);

    for (AnnotationTableCellIndexCR index : cells)
        setter.AddCell (index);

    setter.ProcessCells(edges);
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
        {
        aspect._DiscloseSymbologyKeys (m_usedSymbologyKeys);
        return SUCCESS;
        }

    aspect._FlushChangesToProperties();

    bool    aspectIsMandatory = aspect._IsRequiredOnElement ();
    bool    shouldBePersisted = aspectIsMandatory || aspect._ShouldBePersisted (*this);

    // if all the properties are default we don't need the aspect anymore
    if ( ! shouldBePersisted)
        {
        if ( ! aspect.HasValidAspectId())
            return SUCCESS;

        return aspect.DeleteFromDb();
        }

    BentleyStatus status;

    if (aspect.HasValidAspectId ())
        status = aspect.UpdateInDb();
    else
        status = aspect.InsertInDb();

    aspect._DiscloseSymbologyKeys (m_usedSymbologyKeys);

    if (SUCCESS != status)
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool   AnnotationTableSerializer::SerializeEdgeRuns (EdgeRunsR edgeRuns)
    {
    bool bFailed = false;

    for (AnnotationTableEdgeRunR edgeRun: edgeRuns)
        bFailed |= (SUCCESS != SerializeAspectChanges (edgeRun));

    return bFailed;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
DgnDbStatus AnnotationTableSerializer::SerializeTableToDb()
    {
    bool    bFailed = false;

    /*-------------------------------------------------------------------------
        Table header data
    -------------------------------------------------------------------------*/
    bFailed |= (SUCCESS != SerializeAspectChanges (m_table.GetHeaderAspect()));

    // EdgeRuns associated with the m_table
    bFailed |= SerializeEdgeRuns (m_table.GetTopEdgeRuns());
    bFailed |= SerializeEdgeRuns (m_table.GetLeftEdgeRuns());

    /*-------------------------------------------------------------------------
        Column data
    -------------------------------------------------------------------------*/
    for (AnnotationTableColumnR column: m_table.GetColumnVectorR())
        {
        bFailed |= (SUCCESS != SerializeAspectChanges (column));

        // EdgeRuns associated with this column
        bFailed |= SerializeEdgeRuns (column.GetEdgeRuns());
        }

    /*-------------------------------------------------------------------------
        Row data
    -------------------------------------------------------------------------*/
    for (AnnotationTableRowR row: m_table.GetRowVectorR())
        {
        bFailed |= (SUCCESS != SerializeAspectChanges (row));

        /*-------------------------------------------------------------------------
            Cell data
        -------------------------------------------------------------------------*/
        for (AnnotationTableCellR cell: row.GetCellVectorR())
            bFailed |= (SUCCESS != SerializeAspectChanges (cell));

        // EdgeRuns associated with this row
        bFailed |=SerializeEdgeRuns (row.GetEdgeRuns());
        }

    /*-------------------------------------------------------------------------
        Merge dictionary
    -------------------------------------------------------------------------*/
    MergeDictionary&    mergeDict = m_table.GetMergeDictionary();
    MergeMap::iterator  mergeMapIter = mergeDict.begin();

    while (mergeMapIter != mergeDict.end())
        {
        MergeEntryR                 merge = mergeMapIter->second;

        if (1 == merge.GetRowSpan() && 1 == merge.GetColumnSpan())
            {
            m_table.DeleteAspect (merge);
            mergeMapIter = mergeDict.erase (mergeMapIter);
            continue;
            }

        bFailed |= (SUCCESS != SerializeAspectChanges (merge));
        ++mergeMapIter;
        }

    /*-------------------------------------------------------------------------
        Symbology dictionary
    -------------------------------------------------------------------------*/
    SymbologyDictionary&    symbologyDict = m_table.GetSymbologyDictionary();
    SymbologyMap::iterator  symbologyMapIter = symbologyDict.begin();

    while (symbologyMapIter != symbologyDict.end())
        {
        uint32_t                    key       = symbologyMapIter->first;
        SymbologyEntryR             symbology = symbologyMapIter->second;

        if (m_usedSymbologyKeys.end() == m_usedSymbologyKeys.find (key))
            {
            m_table.DeleteAspect (symbology);
            symbologyMapIter = symbologyDict.erase (symbologyMapIter);
            continue;
            }

        bFailed |= (SUCCESS != SerializeAspectChanges (symbology));
        ++symbologyMapIter;
        }

    /*-------------------------------------------------------------------------
        Remove deleted aspects
    -------------------------------------------------------------------------*/
    for (AnnotationTableAspectDescr& aspectDesc : m_table.GetAspectsPendingDelete())
        AnnotationTableAspect::DeleteAspectFromDb (aspectDesc.m_type, aspectDesc.m_aspectId, m_table);

    m_table.GetAspectsPendingDelete().clear();

    BeAssert (false == AnnotationTableAspect::DbContainsDuplicateRows (AnnotationTableAspectType::Row,       m_table));
    BeAssert (false == AnnotationTableAspect::DbContainsDuplicateRows (AnnotationTableAspectType::Column,    m_table));
    BeAssert (false == AnnotationTableAspect::DbContainsDuplicateRows (AnnotationTableAspectType::Cell,      m_table));
    BeAssert (false == AnnotationTableAspect::DbContainsDuplicateRows (AnnotationTableAspectType::Merge,     m_table));
    //BeAssert (false == AnnotationTableAspect::DbContainsDuplicateRows (AnnotationTableAspectType::Fill,      m_table));
    //BeAssert (false == AnnotationTableAspect::DbContainsDuplicateRows (AnnotationTableAspectType::Symbology, m_table));
    //BeAssert (false == AnnotationTableAspect::DbContainsDuplicateRows (AnnotationTableAspectType::EdgeRun,   m_table));

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
        Symbology data - need the SymbologyDictionary before deserializing edge runs.
    -------------------------------------------------------------------------*/
    statement = AnnotationTableAspect::GetPreparedSelectStatement (AnnotationTableAspectType::Symbology, *this);
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
        return DgnDbStatus::ReadError;

    while (DbResult::BE_SQLITE_ROW == statement->Step())
        {
        int key = statement->GetValue(1).GetInt();

        SymbologyEntry  newSymbology (*this, key);
        newSymbology.AssignProperties (*statement);

        EXPECTED_CONDITION (SUCCESS == GetSymbologyDictionary().AddSymbology (newSymbology));
        }

    /*-------------------------------------------------------------------------
        Row data
    -------------------------------------------------------------------------*/
    statement = AnnotationTableAspect::GetPreparedSelectStatement (AnnotationTableAspectType::Row, *this);
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
        return DgnDbStatus::ReadError;

    while (DbResult::BE_SQLITE_ROW == statement->Step())
        {
        uint32_t rowIndex = statement->GetValue(1).GetInt();

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
        uint32_t columnIndex = statement->GetValue(1).GetInt();

        AnnotationTableColumnP column = GetColumn (columnIndex);
        column->AssignProperties (*statement);
        }

    /*-------------------------------------------------------------------------
        EdgeRun instances
    -------------------------------------------------------------------------*/
    statement = AnnotationTableAspect::GetPreparedSelectStatement (AnnotationTableAspectType::EdgeRun, *this);
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
        return DgnDbStatus::ReadError;

    while (DbResult::BE_SQLITE_ROW == statement->Step())
        {
        EdgeRunHostType hostType  = (EdgeRunHostType) statement->GetValue(1).GetInt();
        uint32_t        hostIndex = statement->GetValue(2).GetInt();;

        AnnotationTableEdgeRun  newEdgeRun (*this);
        newEdgeRun.SetHostType (hostType);
        newEdgeRun.SetHostIndex (hostIndex);
        newEdgeRun.AssignProperties (*statement);

        EdgeRunsP   edgeRuns = newEdgeRun.GetHostEdgeRuns(*this);

        if (UNEXPECTED_CONDITION (NULL == edgeRuns))
            continue;

        edgeRuns->Insert (newEdgeRun);
        }

    /*-------------------------------------------------------------------------
        Merge data - do these before the cells
    -------------------------------------------------------------------------*/
    statement = AnnotationTableAspect::GetPreparedSelectStatement (AnnotationTableAspectType::Merge, *this);
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
        return DgnDbStatus::ReadError;

    while (DbResult::BE_SQLITE_ROW == statement->Step())
        {
        AnnotationTableCellIndex    rootIndex = AnnotationTableCellIndex::GetCellIndex (*statement, 1);

        MergeEntry  newMerge (*this, rootIndex);
        newMerge.AssignProperties (*statement);

        EXPECTED_CONDITION (SUCCESS == GetMergeDictionary().AddMerge (newMerge));

        uint32_t  rowSpan = newMerge.GetRowSpan();
        uint32_t  colSpan = newMerge.GetColumnSpan();
        MarkAsMergedCellInteriors (rootIndex, rowSpan, colSpan, true);
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
        AnnotationTableCellIndex    cellIndex = AnnotationTableCellIndex::GetCellIndex (*statement, 1);
        AnnotationTableCellP        cell      = GetCell (cellIndex);

        if (UNEXPECTED_CONDITION (nullptr == cell || cell->IsMergedCellInterior()))
            continue;

        cell->AssignProperties (*statement);

        cell->ApplyToFillRuns();
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

    m_tableHeader = rhs->m_tableHeader;

    Initialize (false);

    for (bpair<AnnotationTableRegion, AnnotationTextStyleCPtr> const& entry : rhs->m_textStyles)
        m_textStyles[entry.first] = entry.second->CreateCopy();

    for (AnnotationTableColumnCR rhsCol : rhs->m_columns)
        m_columns[rhsCol.GetIndex()] = rhsCol;

    for (AnnotationTableRowCR rhsRow : rhs->m_rows)
        m_rows[rhsRow.GetIndex()] = rhsRow;

    m_topEdgeRuns  = rhs->m_topEdgeRuns;
    m_leftEdgeRuns = rhs->m_leftEdgeRuns;

    m_symbologyDictionary = rhs->m_symbologyDictionary;
    m_mergeDictionary = rhs->m_mergeDictionary;
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

    ElementGeometryBuilderPtr builder = ElementGeometryBuilder::Create(*GetModel(), m_categoryId, m_placement.GetOrigin(), m_placement.GetAngle());

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
