//-------------------------------------------------------------------------------------- 
//     $Source: DgnCore/AnnotationTable.cpp $
//  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//-------------------------------------------------------------------------------------- 

#include <DgnPlatformInternal.h> 
#include <DgnPlatform/DgnCore/AnnotationTable.h>

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

static const double s_doubleTol = 1.e-8;

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(AnnotationTableHandler)
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
static Utf8String buildECSqlInsertString (Utf8CP schemaName, Utf8CP className, bvector<Utf8String> const& propertyNames, bool isUniqueAspect)
    {
    Utf8CP  idPropertyName = isUniqueAspect ? PARAM_ECInstanceId : PARAM_ElementId;

    Utf8PrintfString ecSql("INSERT INTO %s.%s (%s, ", schemaName, className, idPropertyName);
    Utf8PrintfString values(":%s, ", idPropertyName);

    int propCount = 0;
    for (Utf8StringCR propertyName : propertyNames)
        {
        if (propCount != 0)
            {
            ecSql.append(", ");
            values.append(", ");
            }
        ecSql.append("[").append(propertyName).append("]");
        values.append(":").append(propertyName);
        propCount++;
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

    // For multi-aspects, property[0] identifies the aspect (ex. rowIndex) which we never want to update.
    bool    skipFirst = ! isUniqueAspect;
    bool    addedOne  = false;
    for (Utf8StringCR propertyName : propertyNames)
        {
        if (skipFirst)
            {
            skipFirst = false;
            continue;
            }

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
        ecSql.append("[").append(propertyNames[0]).append("]");
        ecSql.append("=:").append(propertyNames[0]);
        }

    return ecSql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
static Utf8String buildECSqlDeleteString (Utf8CP schemaName, Utf8CP className, Utf8CP aspectIdProp)
    {
    Utf8CP  idPropertyName = nullptr == aspectIdProp ? PARAM_ECInstanceId : PARAM_ElementId;

    Utf8PrintfString ecSql("DELETE FROM %s.%s WHERE ", schemaName, className);
    ecSql.append("[").append(idPropertyName).append("]");
    ecSql.append("=:").append(idPropertyName);

    if (aspectIdProp)
        {
        ecSql.append(" AND ");
        ecSql.append("[").append(aspectIdProp).append("]");
        ecSql.append("=:").append(aspectIdProp);
        }

    return ecSql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
static Utf8String buildECSqlSelectString (Utf8CP schemaName, Utf8CP className, bvector<Utf8String> const& propertyNames, bool isUniqueAspect)
    {
    Utf8String ecSql("SELECT ");
    bool first = true;
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
CachedECSqlStatementPtr AnnotationTableAspect::GetPreparedSelectStatement
(
Utf8StringR                 sqlString,
bvector<Utf8String> const&  propertyNames,
Utf8CP                      className,
bool                        isUniqueAspect,
AnnotationTableElementCR    table
)
    {
    if (sqlString.empty())
        sqlString = buildECSqlSelectString (DGN_ECSCHEMA_NAME, className, propertyNames, isUniqueAspect);

    CachedECSqlStatementPtr statement = table.GetDgnDb().GetPreparedECSqlStatement(sqlString.c_str());
    if (!statement.IsValid())
        return nullptr;

    statement->BindId(1, table.GetElementId());

    return statement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTableAspect::InsertInDb()
    {
    Utf8StringR sqlString = _GetECSqlInsertStringBuffer();

    if (sqlString.empty())
        sqlString = buildECSqlInsertString (DGN_ECSCHEMA_NAME, _GetECClassName(), _GetPropertyNames(), _IsUniqueAspect());

    CachedECSqlStatementPtr statement = m_table.GetDgnDb().GetPreparedECSqlStatement(sqlString.c_str());
    if (!statement.IsValid())
        return ERROR;

    _BindAllProperties (*statement);

    if (DbResult::BE_SQLITE_DONE != statement->Step())
        return ERROR;

    ClearHasChanges();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
BentleyStatus AnnotationTableAspect::UpdateInDb()
    {
    Utf8StringR sqlString = _GetECSqlUpdateStringBuffer();

    if (sqlString.empty())
        sqlString = buildECSqlUpdateString (DGN_ECSCHEMA_NAME, _GetECClassName(), _GetPropertyNames(), _IsUniqueAspect());

    CachedECSqlStatementPtr statement = m_table.GetDgnDb().GetPreparedECSqlStatement(sqlString.c_str());
    if (!statement.IsValid())
        return ERROR;

    _BindAllProperties (*statement);

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
BentleyStatus AnnotationTableAspect::DeleteFromDb()
    {
    Utf8StringR sqlString = _GetECSqlDeleteStringBuffer();

    if (sqlString.empty())
        {
        Utf8CP  aspectIdProp = nullptr;

        if ( ! _IsUniqueAspect())
            aspectIdProp = _GetPropertyNames()[0].c_str();

        sqlString = buildECSqlDeleteString (DGN_ECSCHEMA_NAME, _GetECClassName(), aspectIdProp);
        }

    CachedECSqlStatementPtr statement = m_table.GetDgnDb().GetPreparedECSqlStatement(sqlString.c_str());
    if (!statement.IsValid())
        return ERROR;

    _BindIdProperties (*statement);

    if (UNEXPECTED_CONDITION (BE_SQLITE_DONE != statement->Step()))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableAspect::AssignProperties (ECSqlStatement const& statement)
    {
    // The first property for multi-aspects is always the elementId.  We don't need to assign that.
    int firstIndex = _IsUniqueAspect () ? 0 : 1;

    for (int propIndex = firstIndex; propIndex < statement.GetColumnCount (); propIndex++)
        {
        IECSqlValue const&  value = statement.GetValue (propIndex);

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
void    AnnotationTableColumn::CopyDataFrom (AnnotationTableColumnCR rhs)
    {
    m_index             = rhs.m_index;
    m_widthLock         = rhs.m_widthLock;
    m_width             = rhs.m_width;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
Utf8StringR     AnnotationTableColumn::_GetECSqlInsertStringBuffer() { static Utf8String s_str; return s_str; }
Utf8StringR     AnnotationTableColumn::_GetECSqlUpdateStringBuffer() { static Utf8String s_str; return s_str; }
Utf8StringR     AnnotationTableColumn::_GetECSqlDeleteStringBuffer() { static Utf8String s_str; return s_str; }
Utf8CP          AnnotationTableColumn::_GetECClassName() { return DGN_CLASSNAME_AnnotationTableColumn; }
bvector<Utf8String> const& AnnotationTableColumn::_GetPropertyNames() { return GetPropertyNames(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
bvector<Utf8String> const&      AnnotationTableColumn::GetPropertyNames()
    {
    static PropertyNames s_propNames;

    if ( ! s_propNames.empty())
        return s_propNames;

    s_propNames = 
        {
        { (int) AnnotationTableColumn::PropIndex::ColumnIndex,  COLUMN_PARAM_Index      },
        { (int) AnnotationTableColumn::PropIndex::WidthLock,    COLUMN_PARAM_WidthLock  },
        { (int) AnnotationTableColumn::PropIndex::Width,        COLUMN_PARAM_Width      },
        };

    return s_propNames;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
CachedECSqlStatementPtr AnnotationTableColumn::GetPreparedSelectStatement (AnnotationTableElementCR table)
    {
    static Utf8String  s_sqlString;

    return AnnotationTableAspect::GetPreparedSelectStatement (s_sqlString, GetPropertyNames(), DGN_CLASSNAME_AnnotationTableColumn, false, table);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void  AnnotationTableColumn::SetWidth                     (double val)    { m_width.SetValue (val); SetHasChanges(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
double  AnnotationTableColumn::GetWidth () const      { return m_width.IsValid() ? m_width.GetValue() : GetTable().GetDefaultColumnWidth(); }
bool    AnnotationTableColumn::GetWidthLock () const  { return m_widthLock.GetValue(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableColumn::_BindIdProperties(ECSqlStatement& statement)
    {
    statement.BindInt (statement.GetParameterIndex(COLUMN_PARAM_Index), m_index);
    statement.BindId  (statement.GetParameterIndex(PARAM_ElementId), GetTable().GetElementId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableColumn::_BindAllProperties(ECSqlStatement& statement)
    {
    _BindIdProperties (statement);

    BindBool    (statement, COLUMN_PARAM_WidthLock,       m_widthLock);
    BindDouble  (statement, COLUMN_PARAM_Width,           m_width);
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
void    AnnotationTableRow::CopyDataFrom (AnnotationTableRowCR rhs)
    {
    m_index             = rhs.m_index;
    m_heightLock        = rhs.m_heightLock;
    m_height            = rhs.m_height;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
Utf8StringR     AnnotationTableRow::_GetECSqlInsertStringBuffer() { static Utf8String s_str; return s_str; }
Utf8StringR     AnnotationTableRow::_GetECSqlUpdateStringBuffer() { static Utf8String s_str; return s_str; }
Utf8StringR     AnnotationTableRow::_GetECSqlDeleteStringBuffer() { static Utf8String s_str; return s_str; }
Utf8CP          AnnotationTableRow::_GetECClassName() { return DGN_CLASSNAME_AnnotationTableRow; }
bvector<Utf8String> const& AnnotationTableRow::_GetPropertyNames() { return GetPropertyNames(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
bvector<Utf8String> const&      AnnotationTableRow::GetPropertyNames()
    {
    static PropertyNames s_propNames;

    if ( ! s_propNames.empty())
        return s_propNames;

    s_propNames = 
        {
        { (int) AnnotationTableRow::PropIndex::RowIndex,   ROW_PARAM_Index       },
        { (int) AnnotationTableRow::PropIndex::HeightLock, ROW_PARAM_HeightLock  },
        { (int) AnnotationTableRow::PropIndex::Height,     ROW_PARAM_Height      },
        };

    return s_propNames;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
CachedECSqlStatementPtr AnnotationTableRow::GetPreparedSelectStatement (AnnotationTableElementCR table)
    {
    static Utf8String  s_sqlString;

    return AnnotationTableAspect::GetPreparedSelectStatement (s_sqlString, GetPropertyNames(), DGN_CLASSNAME_AnnotationTableRow, false, table);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void  AnnotationTableRow::SetHeight                     (double val)    { m_height.SetValue (val); SetHasChanges(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
double  AnnotationTableRow::GetHeight () const      { return m_height.IsValid() ? m_height.GetValue() : GetTable().GetDefaultRowHeight(); }
bool    AnnotationTableRow::GetHeightLock () const  { return m_heightLock.GetValue(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableRow::_BindIdProperties(ECSqlStatement& statement)
    {
    statement.BindInt (statement.GetParameterIndex(ROW_PARAM_Index), m_index);
    statement.BindId  (statement.GetParameterIndex(PARAM_ElementId), GetTable().GetElementId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
void    AnnotationTableRow::_BindAllProperties(ECSqlStatement& statement)
    {
    _BindIdProperties (statement);

    BindBool    (statement, ROW_PARAM_HeightLock,       m_heightLock);
    BindDouble  (statement, ROW_PARAM_Height,           m_height);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
bvector<Utf8String> const&      TableHeaderAspect::GetPropertyNames()
    {
    static PropertyNames s_propNames;

    if ( ! s_propNames.empty())
        return s_propNames;

    s_propNames = 
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

    return s_propNames;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   10/2015
//---------------------------------------------------------------------------------------
Utf8StringR     TableHeaderAspect::_GetECSqlInsertStringBuffer() { static Utf8String s_str; return s_str; }
Utf8StringR     TableHeaderAspect::_GetECSqlUpdateStringBuffer() { static Utf8String s_str; return s_str; }
Utf8StringR     TableHeaderAspect::_GetECSqlDeleteStringBuffer() { static Utf8String s_str; return s_str; }
Utf8CP          TableHeaderAspect::_GetECClassName() { return DGN_CLASSNAME_AnnotationTableHeader; }
bvector<Utf8String> const& TableHeaderAspect::_GetPropertyNames() { return GetPropertyNames(); }

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
void    TableHeaderAspect::CopyDataFrom(TableHeaderAspect const& rhs)
    {
    m_rowCount                  = rhs.m_rowCount;
    m_columnCount               = rhs.m_columnCount;
    m_textStyleId               = rhs.m_textStyleId;
    m_titleRowCount             = 0;
    m_headerRowCount            = 0;
    m_footerRowCount            = 0;
    m_headerColumnCount         = 0;
    m_footerColumnCount         = 0;
    m_breakType                 = 0;
    m_breakPosition             = 0;
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
void    TableHeaderAspect::_BindIdProperties(ECSqlStatement& statement)
    {
    statement.BindId (statement.GetParameterIndex(PARAM_ECInstanceId), GetTable().GetElementId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   08/2015
//---------------------------------------------------------------------------------------
void    TableHeaderAspect::_BindAllProperties(ECSqlStatement& statement)
    {
    _BindIdProperties (statement);

    BindInt     (statement, HEADER_PARAM_RowCount, m_rowCount);
    BindInt     (statement, HEADER_PARAM_ColumnCount, m_columnCount);
    BindInt64   (statement, HEADER_PARAM_TextStyleId, m_textStyleId);
    statement.BindInt       (statement.GetParameterIndex(HEADER_PARAM_TitleRowCount),              m_titleRowCount);
    statement.BindInt       (statement.GetParameterIndex(HEADER_PARAM_HeaderRowCount),             m_headerRowCount);
    statement.BindInt       (statement.GetParameterIndex(HEADER_PARAM_FooterRowCount),             m_footerRowCount);
    statement.BindInt       (statement.GetParameterIndex(HEADER_PARAM_HeaderColumnCount),          m_headerColumnCount);
    statement.BindInt       (statement.GetParameterIndex(HEADER_PARAM_FooterColumnCount),          m_footerColumnCount);
    statement.BindInt       (statement.GetParameterIndex(HEADER_PARAM_BreakType),                  m_breakType);
    statement.BindInt       (statement.GetParameterIndex(HEADER_PARAM_BreakPosition),              m_breakPosition);
    BindDouble  (statement, HEADER_PARAM_BreakLength,               m_breakLength);
    BindDouble  (statement, HEADER_PARAM_BreakGap,                  m_breakGap);
    BindBool    (statement, HEADER_PARAM_RepeatHeaders,             m_repeatHeaders);
    BindBool    (statement, HEADER_PARAM_RepeatFooters,             m_repeatFooters);
    BindDouble  (statement, HEADER_PARAM_DefaultColumnWidth, m_defaultColumnWidth);
    BindDouble  (statement, HEADER_PARAM_DefaultRowHeight, m_defaultRowHeight);
    BindDouble  (statement, HEADER_PARAM_DefaultMarginTop,           m_defaultMarginTop);
    BindDouble  (statement, HEADER_PARAM_DefaultMarginBottom,        m_defaultMarginBottom);
    BindDouble  (statement, HEADER_PARAM_DefaultMarginLeft,          m_defaultMarginLeft);
    BindDouble  (statement, HEADER_PARAM_DefaultMarginRight,         m_defaultMarginRight);
    statement.BindInt       (statement.GetParameterIndex(HEADER_PARAM_DefaultCellAlignment),       m_defaultCellAlignment);
    statement.BindInt       (statement.GetParameterIndex(HEADER_PARAM_FillSymbologyKeyOddRow),     m_fillSymbologyKeyOddRow);
    statement.BindInt       (statement.GetParameterIndex(HEADER_PARAM_FillSymbologyKeyEvenRow),    m_fillSymbologyKeyEvenRow);
    BindInt64   (statement, HEADER_PARAM_TitleRowTextStyle,     m_titleRowTextStyle);
    BindInt64   (statement, HEADER_PARAM_HeaderRowTextStyle,    m_headerRowTextStyle);
    BindInt64   (statement, HEADER_PARAM_FooterRowTextStyle,    m_footerRowTextStyle);
    BindInt64   (statement, HEADER_PARAM_HeaderColumnTextStyle, m_headerColumnTextStyle);
    BindInt64   (statement, HEADER_PARAM_FooterColumnTextStyle, m_footerColumnTextStyle);
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
CachedECSqlStatementPtr TableHeaderAspect::GetPreparedSelectStatement (AnnotationTableElementCR table)
    {
    static Utf8String  s_sqlString;

    return AnnotationTableAspect::GetPreparedSelectStatement (s_sqlString, GetPropertyNames(), DGN_CLASSNAME_AnnotationTableHeader, true, table);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
int         TableHeaderAspect::GetInteger (PropIndex propIndex) const
    {
    TableIntValue const* intValue = nullptr;

    switch (propIndex)
        {
        case PropIndex::RowCount:                  { intValue = &m_rowCount;     break; }
        case PropIndex::ColumnCount:               { intValue = &m_columnCount;  break; }
        case PropIndex::TitleRowCount:             { break; }
        case PropIndex::HeaderRowCount:            { break; }
        case PropIndex::FooterRowCount:            { break; }
        case PropIndex::HeaderColumnCount:         { break; }
        case PropIndex::FooterColumnCount:         { break; }
        case PropIndex::BreakType:                 { break; }
        case PropIndex::BreakPosition:             { break; }
        case PropIndex::DefaultCellAlignment:      { break; }
        case PropIndex::FillSymbologyKeyOddRow:    { break; }
        case PropIndex::FillSymbologyKeyEvenRow:   { break; }
        case PropIndex::TitleRowTextStyle:         { break; }
        case PropIndex::HeaderRowTextStyle:        { break; }
        case PropIndex::FooterRowTextStyle:        { break; }
        case PropIndex::HeaderColumnTextStyle:     { break; }
        case PropIndex::FooterColumnTextStyle:     { break; }
        case PropIndex::DataSourceProviderId:      { break; }
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
        case PropIndex::DefaultColumnWidth:             { doubleValue = &m_defaultColumnWidth;  break; }
        case PropIndex::DefaultRowHeight:               { doubleValue = &m_defaultRowHeight;    break; }
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
        case PropIndex::RowCount:                  { value = &m_rowCount;      break; }
        case PropIndex::ColumnCount:               { value = &m_columnCount;   break; }
        case PropIndex::TitleRowCount:             { break; }
        case PropIndex::HeaderRowCount:            { break; }
        case PropIndex::FooterRowCount:            { break; }
        case PropIndex::HeaderColumnCount:         { break; }
        case PropIndex::FooterColumnCount:         { break; }
        case PropIndex::BreakType:                 { break; }
        case PropIndex::BreakPosition:             { break; }
        case PropIndex::DefaultCellAlignment:      { break; }
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

    // Try to update an existing row, if that fails the row must not exist so create it.
    if (SUCCESS != (status = aspect.UpdateInDb()))
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

    bFailed |= (SUCCESS != SerializeAspectChanges (m_table.GetHeaderAspect()));

    for (AnnotationTableColumnR column: m_table.GetColumnVectorR())
        {
        bFailed |= (SUCCESS != SerializeAspectChanges (column));
        }

    for (AnnotationTableRowR row: m_table.GetRowVectorR())
        {
        bFailed |= (SUCCESS != SerializeAspectChanges (row));
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Josh.Schifter   09/2015
//---------------------------------------------------------------------------------------
int         AnnotationTableElement::GetRowCount         () const     { return m_tableHeader.GetInteger (TableHeaderAspect::PropIndex::RowCount); }
int         AnnotationTableElement::GetColumnCount      () const     { return m_tableHeader.GetInteger (TableHeaderAspect::PropIndex::ColumnCount); }
double      AnnotationTableElement::GetDefaultRowHeight () const     { return m_tableHeader.GetDouble (TableHeaderAspect::PropIndex::DefaultRowHeight); }
double      AnnotationTableElement::GetDefaultColumnWidth () const   { return m_tableHeader.GetDouble (TableHeaderAspect::PropIndex::DefaultColumnWidth); }

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
void AnnotationTableElement::SetRowCount            (int                v)  { m_tableHeader.SetInteger (v, TableHeaderAspect::PropIndex::RowCount); }
void AnnotationTableElement::SetColumnCount         (int                v)  { m_tableHeader.SetInteger (v, TableHeaderAspect::PropIndex::ColumnCount); }
void AnnotationTableElement::SetDefaultColumnWidth  (double             v)  { m_tableHeader.SetDouble  (v, TableHeaderAspect::PropIndex::DefaultColumnWidth); }
void AnnotationTableElement::SetDefaultRowHeight    (double             v)  { m_tableHeader.SetDouble  (v, TableHeaderAspect::PropIndex::DefaultRowHeight); }

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
void            AnnotationTableElement::ClearTextStyleFromCache (TextTableRegion region)
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
AnnotationTableElementPtr AnnotationTableElement::Create (int rowCount, int columnCount, AnnotationTextStyleId textStyleId, double backupTextHeight, CreateParams const& params)
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
    m_columns.reserve (GetColumnCount());

    for (int columnIndex = 0; columnIndex < GetColumnCount(); columnIndex++)
        {
        AnnotationTableColumn column (*this, columnIndex);
        m_columns.push_back (column);
        }

    m_rows.reserve (GetRowCount());

    for (int rowIndex = 0; rowIndex < GetRowCount(); rowIndex++)
        {
        AnnotationTableRow row (*this, rowIndex);
        m_rows.push_back (row);

        //m_rows[rowIndex].InitializeInternalCollections();
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

#if defined (NEEDSWORK)
        textHeight += 2 * TextBlockHolder::ComputeDescenderAdjustment (*textStyle);
#endif

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
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableRowP   AnnotationTableElement::GetRow (int rowIndex)
    {
    if (GetRowCount() <= rowIndex)
        return NULL;

    return &m_rows[rowIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableRowCP  AnnotationTableElement::GetRow (int rowIndex) const
    {
    return (const_cast <AnnotationTableElement*> (this))->GetRow (rowIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableColumnP   AnnotationTableElement::GetColumn (int colIndex)
    {
    if (GetColumnCount() <= colIndex)
        return NULL;

    return &m_columns[colIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/13
+---------------+---------------+---------------+---------------+---------------+------*/
AnnotationTableColumnCP  AnnotationTableElement::GetColumn (int colIndex) const
    {
    return (const_cast <AnnotationTableElement*> (this))->GetColumn (colIndex);
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

    /*-------------------------------------------------------------------------
        Table header data
    -------------------------------------------------------------------------*/
    CachedECSqlStatementPtr statement = TableHeaderAspect::GetPreparedSelectStatement (*this);
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
        return DgnDbStatus::ReadError;

    if (DbResult::BE_SQLITE_ROW != statement->Step())
        return DgnDbStatus::ReadError;

    m_tableHeader.AssignProperties (*statement);

    Initialize (false);

    /*-------------------------------------------------------------------------
        Row data
    -------------------------------------------------------------------------*/
    statement = AnnotationTableRow::GetPreparedSelectStatement (*this);
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
        return DgnDbStatus::ReadError;

    while (DbResult::BE_SQLITE_ROW == statement->Step())
        {
        int rowIndex = statement->GetValue(0).GetInt();

        AnnotationTableRowP row = GetRow (rowIndex);
        row->AssignProperties (*statement);
        }

    /*-------------------------------------------------------------------------
        Column data
    -------------------------------------------------------------------------*/
    statement = AnnotationTableColumn::GetPreparedSelectStatement (*this);
    if (UNEXPECTED_CONDITION ( ! statement.IsValid()))
        return DgnDbStatus::ReadError;

    while (DbResult::BE_SQLITE_ROW == statement->Step())
        {
        int columnIndex = statement->GetValue(0).GetInt();

        AnnotationTableColumnP column = GetColumn (columnIndex);
        column->AssignProperties (*statement);
        }

    return DgnDbStatus::Success;
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
