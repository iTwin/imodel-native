/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Nullable.h>
#include "Command.h"
#include "iModelConsole.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ECSqlCommand::_GetUsage() const
    {
    return " <ecsql>;                       Executes ECSQL and displays the results.\r\n"
        COMMAND_USAGE_IDENT "The statement can span multiple lines. A semicolon indicates the end of the statement.\r\n";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ECSqlCommand::_Run(Session& session, Utf8StringCR args) const
    {
    if (!session.IsECDbFileLoaded(true))
        return;

    //for ECSQL command the arg vector contains a single arg which contains the original command line.
    Utf8StringCR ecsql = args;
    ECSqlStatement stmt;
    ECSqlStatus status = stmt.Prepare(*session.GetFile().GetECDbHandle(), ecsql.c_str());
    if (!status.IsSuccess())
        {
        if (session.GetIssues().HasIssue())
            IModelConsole::WriteErrorLine("Failed to prepare ECSQL statement. %s", session.GetIssues().GetIssue());
        else
            IModelConsole::WriteErrorLine("Failed to prepare ECSQL statement.");

        return;
        }

    IModelConsole::WriteLine();

    if (ecsql.StartsWithIAscii("select") || ecsql.StartsWithIAscii("values"))
        ExecuteSelect(session, stmt);
    else if (ecsql.StartsWithIAscii("insert into"))
        ExecuteInsert(session, stmt);
    else
        ExecuteUpdateOrDelete(session, stmt);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     11/2013
//---------------------------------------------------------------------------------------
void ECSqlCommand::ExecuteSelect(Session& session, ECSqlStatement& statement) const
    {
    const int columnCount = statement.GetColumnCount();
    bvector<int> columnSizes;
    Utf8String header;
    for (int i = 0; i < columnCount; i++)
        {
        ECSqlColumnInfo const& columnInfo = statement.GetColumnInfo(i);
        const int columnSize = ComputeColumnSize(columnInfo);
        columnSizes.push_back(columnSize);
        
        Utf8String formatString;
        formatString.Sprintf("%%-%ds", columnSize);
        Utf8String columnHeader;
        columnHeader.Sprintf(formatString.c_str(), columnInfo.GetProperty()->GetDisplayLabel().c_str());
        header.append(columnHeader);

        if (i < columnCount - 1)
            header.append("|");
        }

    IModelConsole::WriteLine(header.c_str());
    Utf8String line(header.size(), '-');
    IModelConsole::WriteLine(line.c_str());

    while (BE_SQLITE_ROW == statement.Step())
        {
        Utf8String rowString;
        for (int i = 0; i < columnCount; i++)
            {
            Utf8String cellValue = ValueToString(statement.GetValue(i));
            const int columnSize = columnSizes[(size_t) i];

            Utf8String formatString;
            formatString.Sprintf("%%-%ds", columnSize);

            Utf8String formattedCellValue;
            formattedCellValue.Sprintf(formatString.c_str(), cellValue.c_str());
            rowString.append(formattedCellValue);

            if (i < columnCount - 1)
                rowString.append("|");
            }

        IModelConsole::WriteLine(rowString.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     11/2013
//---------------------------------------------------------------------------------------
void ECSqlCommand::ExecuteInsert(Session& session, ECSqlStatement& statement) const
    {
    ECInstanceKey generatedECInstanceKey;
    if (BE_SQLITE_DONE != statement.Step(generatedECInstanceKey))
        {
        if (session.GetIssues().HasIssue())
            IModelConsole::WriteErrorLine("Failed to execute ECSQL statement. %s", session.GetIssues().GetIssue());
        else
            IModelConsole::WriteErrorLine("Failed to execute ECSQL statement.");

        return;
        }

    IModelConsole::WriteLine("New row inserted [Id %s].", IModelConsole::FormatId(generatedECInstanceKey.GetInstanceId()).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     11/2013
//---------------------------------------------------------------------------------------
void ECSqlCommand::ExecuteUpdateOrDelete(Session& session, ECSqlStatement& statement) const
    {
    if (BE_SQLITE_DONE != statement.Step())
        {
        if (session.GetIssues().HasIssue())
            IModelConsole::WriteErrorLine("Failed to execute ECSQL statement. %s", session.GetIssues().GetIssue());
        else
            IModelConsole::WriteErrorLine("Failed to execute ECSQL statement.");

        return;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    11/2016
//---------------------------------------------------------------------------------------
//static
int ECSqlCommand::ComputeColumnSize(ECSqlColumnInfo const& colInfo)
    {
    const int defaultSize = 20;
    if (colInfo.GetDataType().IsPrimitive())
        {
        switch (colInfo.GetDataType().GetPrimitiveType())
            {
                case PRIMITIVETYPE_Binary:
                    //Output: "BINARY[%d bytes]"
                    return 14 + std::numeric_limits<int>::digits10;

                case PRIMITIVETYPE_Boolean:
                    //Output: "True" or "False"
                    return 5;

                case PRIMITIVETYPE_DateTime:
                {
                //Output: ISO string representation of a DateTime
                DateTime::Info dtInfoCA;
                if (ECObjectsStatus::Success != StandardCustomAttributeHelper::GetDateTimeInfo(dtInfoCA, *colInfo.GetProperty()))
                    return defaultSize;

                if (dtInfoCA.IsValid())
                    {
                    if (dtInfoCA.GetComponent() == DateTime::Component::Date)
                        //Just the date string, e.g "2016-11-02"
                        return 10;

                    if (dtInfoCA.GetKind() == DateTime::Kind::Utc)
                        //ISO date time string with trailing Z for UTC: e.g. "2016-11-02T17:39:01.438Z"
                        return 24;
                    }

                //ISO date time string without trailing Z: e.g. "2016-11-02T17:39:01.438"
                return 23;
                }

                case PRIMITIVETYPE_Double:
                    return std::numeric_limits<double>::max_digits10;

                case PRIMITIVETYPE_IGeometry:
                    //Output: Type name of top-level geometry types
                    return 14;

                case PRIMITIVETYPE_Integer:
                    return std::numeric_limits<int32_t>::digits10;
                case PRIMITIVETYPE_Long:
                    return std::numeric_limits<int64_t>::digits10;
                case PRIMITIVETYPE_Point2d:
                    //Output: "(x,y)"
                    return 2 * std::numeric_limits<double>::max_digits10 + 3;
                case PRIMITIVETYPE_Point3d:
                    //Output: "(x,y,z)"
                    return 3 * std::numeric_limits<double>::max_digits10 + 4;
                default:
                    return defaultSize;
            }
        }

    //for struct and arrays we don't anticipate a column size
    return defaultSize;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle    12/2017
//---------------------------------------------------------------------------------------
//static
Utf8String ECSqlCommand::ValueToString(IECSqlValue const& value)
    {
    if (value.IsNull())
        return "NULL";

    ECTypeDescriptor const& dataType = value.GetColumnInfo().GetDataType();

    if (dataType.IsPrimitive())
        {
        ECEnumerationCP ecEnum = value.GetColumnInfo().GetProperty()->GetAsPrimitiveProperty()->GetEnumeration();
        if (ecEnum != nullptr)
            return PrimitiveToString(value, *ecEnum);

        return PrimitiveToString(value, dataType.GetPrimitiveType());
        }

    if (dataType.IsStruct())
        return StructToString(value);

    if (dataType.IsArray())
        return ArrayToString(value);

    if (dataType.IsNavigation())
        {
        ECN::NavigationECPropertyCP navProp = value.GetColumnInfo().GetProperty()->GetAsNavigationProperty();
        if (navProp->IsMultiple())
            return "{...}";

        ECClassId relClassId;
        BeInt64Id navId = value.GetNavigation(&relClassId);
        Utf8String str;
        str.Sprintf("{Id:%s,RelECClassId:%s}", IModelConsole::FormatId(navId).c_str(), relClassId.IsValid() ? IModelConsole::FormatId(relClassId).c_str() : "-");
        return str;
        }

    BeAssert(false);
    return "<unknown>";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8String ECSqlCommand::PrimitiveToString(IECSqlValue const& value, ECEnumerationCR ecEnum)
    {
    const bool isIntEnum = ecEnum.GetType() == PRIMITIVETYPE_Integer;
    Nullable<int> actualIntValue;
    Utf8CP actualStrValue = nullptr;
    if (isIntEnum)
        actualIntValue = value.GetInt();
    else
        actualStrValue = value.GetText();

    ECEnumeratorCP enumValue = nullptr;
    if (isIntEnum)
        enumValue = ecEnum.FindEnumerator(actualIntValue.Value());
    else
        enumValue = ecEnum.FindEnumerator(actualStrValue);

    if (enumValue != nullptr)
        return Utf8PrintfString("%s.%s", ecEnum.GetName().c_str(), enumValue->GetName().c_str());

    //no enum value matches -> print out the value as is
    return PrimitiveToString(value, isIntEnum ? PRIMITIVETYPE_Integer : PRIMITIVETYPE_String);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8String ECSqlCommand::PrimitiveToString(IECSqlValue const& value, ECN::PrimitiveType type)
    {
    Utf8String out;
    if (value.IsNull())
        {
        out = "NULL";
        return out;
        }
    switch (type)
        {
            case ECN::PRIMITIVETYPE_Binary:
            {
            int blobSize = -1;
            value.GetBlob(&blobSize);
            out.Sprintf("BLOB[%d bytes]", blobSize);
            break;
            }
            case ECN::PRIMITIVETYPE_Boolean:
            {
            out.Sprintf("%s", value.GetBoolean() ? "True" : "False");
            break;
            }
            case ECN::PRIMITIVETYPE_DateTime:
            {
            out.Sprintf("%s", value.GetDateTime().ToString().c_str());
            break;
            }
            case ECN::PRIMITIVETYPE_Double:
            {
            out.Sprintf("%.4f", value.GetDouble());
            break;
            }
            case ECN::PRIMITIVETYPE_Integer:
            {
            out.Sprintf("%d", value.GetInt());
            break;
            }
            case ECN::PRIMITIVETYPE_Long:
            {
            if (value.GetColumnInfo().IsSystemProperty())
                out.append(IModelConsole::FormatId(value.GetId<BeInt64Id>()));
            else
                out.Sprintf("%" PRId64, value.GetInt64());

            break;
            }
            case ECN::PRIMITIVETYPE_Point2d:
            {
            DPoint2d point2d = value.GetPoint2d();
            out.Sprintf("(%.4f,%.4f)", point2d.x, point2d.y);
            break;
            }
            case ECN::PRIMITIVETYPE_Point3d:
            {
            DPoint3d point3d = value.GetPoint3d();
            out.Sprintf("(%.4f,%.4f,%.4f)", point3d.x, point3d.y, point3d.z);
            break;
            }
            case ECN::PRIMITIVETYPE_String:
            {
            out = value.GetText();
            break;
            }
            case ECN::PRIMITIVETYPE_IGeometry:
            {
            IGeometryPtr geom = value.GetGeometry();
            if (geom == nullptr) // can be null if conversion from blob to IGeometry failed
                {
                out = "<invalid geom format>";
                break;
                }

            switch (geom->GetGeometryType())
                {
                    case IGeometry::GeometryType::BsplineSurface:
                        out = "BsplineSurface";
                        break;
                    case IGeometry::GeometryType::CurvePrimitive:
                        out = "CurvePrimitive";
                        break;
                    case IGeometry::GeometryType::CurveVector:
                        out = "CurveVector";
                        break;
                    case IGeometry::GeometryType::Polyface:
                        out = "Polyface";
                        break;
                    case IGeometry::GeometryType::SolidPrimitive:
                        out = "SolidPrimitive";
                        break;
                    default:
                    {
                    BeAssert(false && "Adjust code to new value in enum IGeometry::GeometryType");
                    out = "IGeometry";
                    break;
                    }
                }
            break;
            }
        }
    return out;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8String ECSqlCommand::ArrayToString(IECSqlValue const& value)
    {
    Nullable<PrimitiveType> primType;
    ECEnumerationCP ecEnum = nullptr;
    if (value.GetColumnInfo().GetDataType().IsPrimitiveArray())
        {
        PrimitiveArrayECPropertyCP arrayProp = value.GetColumnInfo().GetProperty()->GetAsPrimitiveArrayProperty();
        ecEnum = arrayProp->GetEnumeration();
        if (ecEnum == nullptr)
            primType = arrayProp->GetPrimitiveElementType();
        }

    Utf8String out = "[";
    bool isFirstRow = true;
    for (IECSqlValue const& arrayElementValue : value.GetArrayIterable())
        {
        if (!isFirstRow)
            out.append(",");

        if (!primType.IsNull())
            out.append(PrimitiveToString(arrayElementValue, primType.Value()));
        else if (ecEnum != nullptr)
            out.append(PrimitiveToString(arrayElementValue, *ecEnum));
        else
            out.append(StructToString(arrayElementValue));
        isFirstRow = false;
        }

    out.append("]");
    return out;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8String ECSqlCommand::StructToString(IECSqlValue const& value)
    {
    Utf8String out;
    out.append("{");
    bool isFirst = true;
    for (IECSqlValue const& structMemberValue : value.GetStructIterable())
        {
        if (structMemberValue.IsNull())
            continue;

        if (!isFirst)
            out.append(",");

        out.append(structMemberValue.GetColumnInfo().GetProperty()->GetName()).append(":");
        out.append(ValueToString(structMemberValue));
        isFirst = false;
        }

    out.append("}");
    return out;
    }

