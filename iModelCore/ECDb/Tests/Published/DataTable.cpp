/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/DataTable.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "DataTable.h"
BEGIN_ECDBUNITTESTS_NAMESPACE
/////////////////////////////////////////DataTable::Value////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DataTable::Value::ToString() const
    {
    Utf8String str;
    switch (GetType())
        {
            case Type::Blob:
                str.append("..."); break;
            case Type::Float:
                str.Sprintf("%f", GetFloat()); break;
            case Type::Integer:
                str.Sprintf("%d", GetInteger()); break;
            case Type::Null:
                str = "(null)"; break;
            case Type::Text:
                str = GetText(); break;
        }
    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void DataTable::Value::SetNull()
    {
    if (IsNull())
        return;
    if (IsBlob() || IsText())
        {
        if (m_buff != nullptr)
            free(m_buff);
        }
    m_type = Type::Null;
    m_size = 0;
    m_buff = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void DataTable::Value::SetValue(int64_t i)
    {
    if (!IsInteger())
        SetNull();

    m_i64 = i;
    m_type = Type::Integer;
    m_size = sizeof(int64_t);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void DataTable::Value::SetValue(double i)
    {
    if (!IsFloat())
        SetNull();

    m_flt = i;
    m_type = Type::Float;
    m_size = sizeof(double);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void DataTable::Value::SetValue(Utf8CP str)
    {
    Copy((void*) str, strlen(str) + 1, Type::Text);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void DataTable::Value::SetValue(const void* blob, size_t n)
    {
    Copy(blob, n, Type::Blob);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
DataTable::Value::Value(Value const& rhs)
    {
    CopyObject(rhs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
DataTable::Value::Value(Value && rhs)
    {
    MoveObject(rhs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
DataTable::Value& DataTable::Value::operator =(DataTable::Value const& rhs)
    {
    if (this != &rhs)
        CopyObject(rhs);

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
DataTable::Value& DataTable::Value::operator =(DataTable::Value && rhs)
    {
    if (this != &rhs)
        MoveObject(rhs);

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
bool DataTable::Value::Equals(DataTable::Value const& rhs) const
    {
    if (rhs.GetType() != GetType())
        return false;

    if (IsInteger())
        return GetInteger() == rhs.GetInteger();

    if (IsFloat())
        return IsFloat() == rhs.IsFloat();

    if (IsText())
        return strcmp(rhs.GetText(), GetText()) == 0;

    if (IsBlob())
        {
        if (rhs.GetSize() != GetSize())
            return false;

        if (rhs.GetSize() == 0)
            return true;

        return memcpy(rhs.GetBlob(), GetBlob(), GetSize()) == 0;
        }

    BeAssert(false);
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
bool DataTable::Value::EqualsI(DataTable::Value const& rhs) const
    {
    if (rhs.GetType() != GetType())
        return false;

    if (IsInteger())
        return GetInteger() == rhs.GetInteger();

    if (IsFloat())
        return IsFloat() == rhs.IsFloat();

    if (IsText())
        return BeStringUtilities::StricmpAscii(rhs.GetText(), GetText()) == 0;

    if (IsBlob())
        {
        if (rhs.GetSize() != GetSize())
            return false;

        if (rhs.GetSize() == 0)
            return true;

        return memcpy(rhs.GetBlob(), GetBlob(), GetSize()) == 0;
        }

    BeAssert(false);
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
bool DataTable::Value::operator == (DataTable::Value const& rhs) const
    {
    return Equals(rhs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
bool DataTable::Value::operator != (DataTable::Value const& rhs) const
    {
    return !Equals(rhs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void DataTable::Value::Copy(const void* blob, size_t n, DataTable::Value::Type type)
    {
    SetNull();
    if (blob == nullptr || n == 0)
        return;

    m_size = n;
    if (type == Type::Text)
        n = n + 1;

    m_buff = malloc(n);
    BeAssert(m_buff != nullptr);
    memcpy(m_buff, blob, (int) n);

    m_type = type;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void DataTable::Value::CopyObject(Value const& rhs)
    {
    if (rhs.IsText())
        SetValue(rhs.GetText());

    if (rhs.IsBlob())
        SetValue(rhs.GetBlob(), rhs.GetSize());

    if (rhs.IsInteger())
        SetValue(rhs.GetInteger());

    if (rhs.IsFloat())
        SetValue(rhs.GetFloat());

    SetNull();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void DataTable::Value::Value::MoveObject(Value & rhs)
    {
    if (rhs.IsText() || rhs.IsBlob())
        m_buff = rhs.m_buff;

    if (rhs.IsInteger())
        m_i64 = rhs.m_i64;

    if (rhs.IsFloat())
        m_flt = rhs.m_flt;

    m_size = rhs.m_size;
    m_type = rhs.m_type;
    rhs.m_type = Type::Null;
    rhs.m_buff = nullptr;
    }

////////////////////////////////////DataTable::Row///////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
DataTable::Value& DataTable::Row::GetValueR(size_t columnIndex)
    {
    return m_values[columnIndex];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
DataTable::Value const& DataTable::Row::GetValue(size_t columnIndex) const
    {
    return m_values[columnIndex];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
DataTable::Value& DataTable::Row::GetValueR(Column const& column)
    {
    return GetValueR(column.GetIndex());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
DataTable::Value const& DataTable::Row::GetValue(Column const& column) const
    {
    return GetValue(column.GetIndex());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void DataTable::Row::SetValue(Column const& column, DataTable::Value&& value)
    {
    m_values[column.GetIndex()] = std::move(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void DataTable::Row::SetValue(Column const& column, DataTable::Value const& value)
    {
    m_values[column.GetIndex()] = value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String DataTable::Row::ToString() const
    {
    Utf8String str;
    for (size_t i = 0; i < m_table.GetColumnCount(); i++)
        {
        Column const& column = m_table.GetColumn(i);
        Value const& v = GetValue(i);
        if (i != 0)
            str.append(", ");

        str.append(column.GetName()).append("=").append(v.ToString());
        }

    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
DataTable::Row::Row(DataTable const& table, size_t index)
    :m_table(table), m_index(index)
    {
    m_values.reserve(table.GetColumnCount());
    for (int i = 0; i < table.GetColumnCount(); i++)
        m_values.push_back(Value());
    m_values.capacity();
    }
////////////////////////////////////////DataTable////////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
DataTable::Column const& DataTable::AddColumn(Utf8CP name, Value::Type type)
    {
    Column* col = nullptr;
    if (m_lock)
        {
        BeAssert(false && "Table is locked down and cannot accept any changes");
        return *col;
        }

    col = new Column(*this, m_columns.size(), name, type);
    m_columns.push_back(std::unique_ptr<Column>(col));
    return *col;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
DataTable::Column const& DataTable::AddColumn(Utf8CP name)
    {
    return AddColumn(name, Value::Type::Blob);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
DataTable::Row& DataTable::NewRow()
    {
    Row* row = nullptr;
    if (!m_lock)
        {
        BeAssert(false && "Call LocalDefinition() first before calling this api");
        return *row;
        }

    row = new Row(*this, m_rows.size());
    m_rows.push_back(std::unique_ptr<Row>(row));
    return *row;
    }
////////////////////////////////////////CSVAdaptor///////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
//static 
Utf8Char CSVAdaptor::PreviousChar(Utf8CP current, Utf8CP bof)
    {
    if ((current - 1) >= bof)
        return *(current - 1);

    return *bof;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
//static 
Utf8Char CSVAdaptor::NextChar(Utf8CP current)
    {
    if (*current == '\0')
        return *current;

    return *(current + 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CSVAdaptor::ParseField(DataTable& table, Utf8CP begin, Utf8CP end, int line, int field)
    {
    Utf8String v(begin, end);
    v.Trim();
    if (line == 1)
        {
        table.AddColumn(v.c_str());
        return SUCCESS;
        }

    if (!table.IsDefinitionLocked())
        table.LockDefinition();

    if (table.GetColumnCount() < field)
        {
        BeAssert(false);
        return ERROR;
        }
    const int rowId = line - 1;
    DataTable::Row* row = nullptr;
    if (table.GetRowCount() < rowId)//0 < 1  1< 1
        row = &table.NewRow();
    else
        row = &table.GetRowR(rowId - 1);//2-2=0

    if (v.EndsWithI("(null)") || v.empty())
        return SUCCESS;

    Utf8CP c = v.c_str();
    while (*c != '\0')
        if (!isdigit(*c++))break;

    double d;
    if (*c == '\0')
        {
        int64_t i64;
        sscanf(v.c_str(), "%" SCNd64, &i64);
        row->GetValueR(field - 1).SetValue(i64);
        }
    else  if (sscanf(v.c_str(), "%lf", &d) == 1)
        {
        row->GetValueR(field - 1).SetValue(d);
        }
    else
        {
        row->GetValueR(field - 1).SetValue(v.c_str());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CSVAdaptor::ParseLine(DataTable& table, Utf8CP begin, Utf8CP end, int line)
    {
    Utf8CP cur = begin;
    Utf8CP fstart = cur;
    Utf8CP fend = cur;
    int field = 0;
    enum class State
        {
        Start,
        QuotedBegin,
        QuotedEnd,
        End
        };

    State state = State::Start;
    while (cur != end)
        {
        if (*cur == '"')
            {
            if (state == State::Start)
                {
                fstart = cur + 1;
                fend = cur + 1;
                state = State::QuotedBegin;
                }
            else if (state == State::QuotedBegin)
                {
                if (NextChar(cur) == '"')
                    cur++;
                else
                    {
                    fend = cur;
                    state = State::QuotedEnd;
                    }
                }
            else if (state == State::QuotedEnd)
                {
                BeAssert(false);
                return ERROR;
                }
            }

        if (*cur == ',' && state != State::QuotedBegin)
            {
            if (state == State::QuotedEnd)
                {
                if (ParseField(table, fstart, fend, line, ++field) != SUCCESS) return ERROR;
                }
            else if (state == State::Start)
                {
                if (ParseField(table, fstart, cur, line, ++field) != SUCCESS) return ERROR;
                }
            fstart = cur + 1;
            fend = cur + 1;
            state = State::Start;
            }

        cur++;
        }

    if (state == State::QuotedBegin)
        return ERROR;

    else if (state == State::Start && fstart != cur)
        {
        if (ParseField(table, fstart, cur, line, ++field) != SUCCESS) return ERROR;
        }
    else if (state == State::QuotedEnd)
        {
        if (ParseField(table, fstart, fend, line, ++field) != SUCCESS) return ERROR;
        }

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
void CSVAdaptor::Fill(Utf8StringR toCSVBuffer, DataTable const& fromDataTable)
    {
    //https://www.ietf.org/rfc/rfc4180.txt
    const Utf8CP CSV_EOL = "\r\n";
    const Utf8CP CSV_DELIMITER = ", ";
    const Utf8CP CSV_NULL = "(null)";
    const Utf8CP CSV_STRING_DELIMITER = "\"";

    for (size_t i = 0; i < fromDataTable.GetColumnCount(); i++)
        {
        if (i != 0)
            toCSVBuffer.append(CSV_DELIMITER);

        toCSVBuffer.append(fromDataTable.GetColumn(i).GetName());
        }

    toCSVBuffer.append(CSV_EOL);
    Utf8String cell;
    for (size_t r = 0; r < fromDataTable.GetRowCount(); r++)
        {
        DataTable::Row const& row = fromDataTable.GetRow(r);
        for (size_t c = 0; c < fromDataTable.GetColumnCount(); c++)
            {
            if (c != 0)
                toCSVBuffer.append(CSV_DELIMITER);

            DataTable::Value const& value = row.GetValue(c);
            switch (value.GetType())
                {
                    case DataTable::Value::Type::Blob:
                        BeAssert(false);
                        break;
                    case DataTable::Value::Type::Float:
                        cell.Sprintf("%lf", value.GetFloat());
                        toCSVBuffer.append(cell);
                        break;
                    case DataTable::Value::Type::Integer:
                        cell.Sprintf("%" PRId64 "", value.GetInteger());
                        toCSVBuffer.append(cell);
                        break;
                    case DataTable::Value::Type::Null:
                        toCSVBuffer.append(CSV_NULL);
                        break;
                    case DataTable::Value::Type::Text:
                    {
                    Utf8CP c = value.GetText();
                    bool hasEscapeChar = false;
                    while (*c != '\0' && !hasEscapeChar)
                        {
                        if (isspace((int) (*c)) || *c == ',' || *c == '"')
                            hasEscapeChar = true;

                        c++;
                        }

                    if (hasEscapeChar)
                        toCSVBuffer.append(CSV_STRING_DELIMITER).append(value.GetText()).append(CSV_STRING_DELIMITER);
                    else
                        toCSVBuffer.append(value.GetText());

                    break;
                    }
                }
            }

        toCSVBuffer.append(CSV_EOL);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CSVAdaptor::Fill(DataTable& toDataTable, Utf8CP fromCSVBuffer)
    {
    const Utf8Char CSV_EOF = '\0';
    Utf8CP cur = fromCSVBuffer;
    Utf8CP start = cur;
    int line = 0;

    while (*cur != CSV_EOF)
        {
        if (*cur == '\n')
            {
            if (PreviousChar(cur, fromCSVBuffer) == '\r')
                {
                if (ParseLine(toDataTable, start, cur - 1, ++line) != SUCCESS)return ERROR;
                }
            else
                {
                if (ParseLine(toDataTable, start, cur, ++line) != SUCCESS)return ERROR;
                }

            start = ++cur;
            }
        else
            ++cur;
        }

    if (start != cur)
        {
        return ParseLine(toDataTable, start, cur - 1, ++line);
        }

    return SUCCESS;
    }

////////////////////////////////////////SqlAdaptor///////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
//static 
void SqlAdaptor::Fill(DataTable& table, Statement& stmt, Utf8CP tableName)
    {
    for (int i = 0; i < stmt.GetColumnCount(); i++)
        table.AddColumn(stmt.GetColumnName(i));

    table.LockDefinition();
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        DataTable::Row& row = table.NewRow();
        for (int i = 0; i < stmt.GetColumnCount(); i++)
            {
            switch (stmt.GetColumnType(i))
                {
                    case DbValueType::BlobVal:
                        row.GetValueR(table.GetColumn(i)).SetValue(stmt.GetValueBlob(i), (size_t) stmt.GetColumnBytes(i)); break;
                    case DbValueType::FloatVal:
                        row.GetValueR(table.GetColumn(i)).SetValue(stmt.GetValueDouble(i)); break;
                    case DbValueType::IntegerVal:
                        row.GetValueR(table.GetColumn(i)).SetValue(stmt.GetValueDouble(i)); break;
                    case DbValueType::NullVal:
                        row.GetValueR(table.GetColumn(i)).SetNull(); break;
                    case DbValueType::TextVal:
                        row.GetValueR(table.GetColumn(i)).SetValue(stmt.GetValueText(i)); break;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Affan.Khan                       05/17
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus SqlAdaptor::Fill(DbCR db, DataTable const& table)
    {
    if (!table.IsDefinitionLocked())
        return ERROR;

    if (Utf8String::IsNullOrEmpty(table.GetName()))
        return ERROR;

    if (table.GetColumnCount() > 0)
        return ERROR;

    Utf8String createTableSQL = "CREATE TABLE [";
    createTableSQL.append(table.GetName()).append("] (Id INTEGER PRIMARY KEY");
    for (int i = 0; i < table.GetColumnCount(); i++)
        {
        createTableSQL.append(",");
        createTableSQL.append(table.GetColumn(i).GetName());
        }

    createTableSQL.append(")");
    if (db.ExecuteSql(createTableSQL.c_str()) != BE_SQLITE_OK)
        {
        return ERROR;
        }

    Utf8String insertSQL = "INSERT INTO [";
    Utf8String valuesSQL = "VALUES (null";
    insertSQL.append(table.GetName()).append("] (Id");
    for (int i = 0; i < table.GetColumnCount(); i++)
        {
        insertSQL.append(",");
        insertSQL.append(table.GetColumn(i).GetName());
        valuesSQL.append(",?");
        }

    insertSQL.append(")").append(valuesSQL).append(")");
    Statement stmt;
    stmt.Prepare(db, insertSQL.c_str());
    for (size_t r=0; r < table.GetRowCount(); r++)
        {
        stmt.Reset();
        stmt.ClearBindings();
        DataTable::Row const& row = table.GetRow(r);
        for (int c=0; c < table.GetColumnCount(); c++)
            {
            DataTable::Value const& v = row.GetValue(c);
            switch (v.GetType())
                {
                    case DataTable::Value::Type::Blob:
                        stmt.BindBlob(c, v.GetBlob(), (int) v.GetSize(), Statement::MakeCopy::No); break;
                    case DataTable::Value::Type::Float:
                        stmt.BindDouble(c, v.GetFloat()); break;
                    case DataTable::Value::Type::Integer:
                        stmt.BindInt64(c, v.GetInteger()); break;
                    case DataTable::Value::Type::Text:
                        stmt.BindText(c, v.GetText(), Statement::MakeCopy::No); break;
                }
            }
        if (stmt.Step() != BE_SQLITE_DONE)
            return ERROR;
        }

    return SUCCESS;
    }
   
END_ECDBUNITTESTS_NAMESPACE