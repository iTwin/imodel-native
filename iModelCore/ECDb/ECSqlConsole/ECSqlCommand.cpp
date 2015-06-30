/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSqlConsole/ECSqlCommand.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECDb/ECDbApi.h>

#include "ConsoleCommand.h"
#include "ECSqlConsole.h"
#include "Console.h"
#include "ECSqlStatementIterator.h"
using namespace std;
USING_NAMESPACE_BENTLEY_SQLITE_EC

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ECSqlCommand::_GetName () const
    {
    return ".ecsql";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
Utf8String ECSqlCommand::_GetUsage () const
    {
    return " <ecsql>;                       Executes ECSQL and displays the results.\r\n"
           "                                The statement can span multiple lines.\r\n"
           "                                A semicolon indicates the end of the statement";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     10/2013
//---------------------------------------------------------------------------------------
void ECSqlCommand::_Run (ECSqlConsoleSession& session, vector<Utf8String> const& args) const
    {
    if (!session.HasECDb (true))
        return;

    //for ECSQL command the arg vector contains a single arg which contains the original command line.
    Utf8CP ecsql = args[0].c_str ();
    ECSqlStatement stmt;
    ECSqlStatus status = stmt.Prepare (session.GetECDbR (), ecsql);
    if (status != ECSqlStatus::Success)
        {
        Console::WriteErrorLine ("Failed to prepare ECSQL statement. %s", stmt.GetLastStatusMessage ().c_str ());
        return;
        }

    if (strnicmp(ecsql, "SELECT", 6) == 0)
        ExecuteSelect(session, stmt);
    else if (strnicmp(ecsql, "INSERT INTO", 11) == 0)
        ExecuteInsert (stmt);
    else 
        ExecuteUpdateOrDelete (stmt);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     11/2013
//---------------------------------------------------------------------------------------
void ECSqlCommand::ExecuteSelect (ECSqlConsoleSession& session, ECSqlStatement& statement) const
    {
    const int columnCount = statement.GetColumnCount ();
    for(int i=0; i < columnCount; i++)
        {
        auto const& columnInfo = statement.GetColumnInfo (i);
        auto prop = columnInfo.GetProperty ();
        Utf8String propName (prop->GetDisplayLabel ());
        Console::Write ("%s\t", propName.c_str ());
        }

    Console::WriteLine();
    Console::WriteLine("-------------------------------------------------------------");

    if (session.GetOutputFormat () == OutputFormat::List)
        {
        ListDataWriter writer;
        ECSqlStatementIterator::Iterate(statement, writer);
        return;
        }

    while (statement.Step () == ECSqlStepStatus::HasRow)
        {
        Utf8String out;            
        for(int i = 0; i < columnCount; i++)
            {
            IECSqlValue const& value = statement.GetValue (i);
            auto prop = value.GetColumnInfo ().GetProperty ();
            if (prop->GetIsPrimitive())
                out += PrimitiveToString(value) + "\t";
            else if (prop->GetIsStruct())
                out += StructToString (value) + "\t";
            else
                out += ArrayToString(value, prop) + "\t";
            }

        Console::WriteLine(out.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     11/2013
//---------------------------------------------------------------------------------------
void ECSqlCommand::ExecuteInsert (ECSqlStatement& statement) const
    {
    ECInstanceKey generatedECInstanceKey;
    auto stepStat = statement.Step (generatedECInstanceKey);

    if (stepStat != ECSqlStepStatus::Done)
        {
        Console::WriteErrorLine("Failed to execute ECSQL statement: %s", statement.GetLastStatusMessage ().c_str ());
        return;
        }

    Console::WriteLine ("New row inserted [ECInstanceId %lld].", generatedECInstanceKey.GetECInstanceId ().GetValue ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                  Krischan.Eberle     11/2013
//---------------------------------------------------------------------------------------
void ECSqlCommand::ExecuteUpdateOrDelete (ECSqlStatement& statement) const
    {
    statement.EnableDefaultEventHandler ();
    ECSqlStepStatus stepStat = statement.Step ();

    if (stepStat != ECSqlStepStatus::Done)
        {
        Console::WriteErrorLine("Failed to execute ECSQL statement: %s", statement.GetLastStatusMessage ().c_str ());
        return;
        }

    Console::WriteLine ("%d instances affected.", statement.GetDefaultEventHandler ()->GetInstancesAffectedCount ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8String ECSqlCommand::PrimitiveToString (IECSqlValue const& value, ECN::PrimitiveType type)
    {
    Utf8String out;
    if (value.IsNull ())
        {
        out = "NULL";
        return out;
        }
    switch (type)
        {
        case ECN::PRIMITIVETYPE_Binary:
            {
            int blobSize = -1;
            value.GetBinary (&blobSize);
            out.Sprintf("BINARY[%d bytes]", blobSize);
            break;
            }
        case ECN::PRIMITIVETYPE_Boolean:
            {
            out.Sprintf ("%s", value.GetBoolean () ? "True" : "False");
            break;
            }
        case ECN::PRIMITIVETYPE_DateTime:
            {
            out.Sprintf ("%s", value.GetDateTime ().ToUtf8String ().c_str ());
            break;
            }
        case ECN::PRIMITIVETYPE_Double:
            {
            out.Sprintf ("%.4f", value.GetDouble ());
            break;
            }
        case ECN::PRIMITIVETYPE_Integer:
            {
            out.Sprintf ("%d", value.GetInt ());
            break;
            }
        case ECN::PRIMITIVETYPE_Long:
            {
            out.Sprintf ("%lld", value.GetInt64 ());
            break;
            }
        case ECN::PRIMITIVETYPE_Point2D:
            {
            auto point2d = value.GetPoint2D ();
            out.Sprintf("(%2.1f, %2.1f)", point2d.x, point2d.y);
            break;
            }
        case ECN::PRIMITIVETYPE_Point3D:
            {
            auto point3d = value.GetPoint3D ();
            out.Sprintf("(%2.1f, %2.1f, %2.1f)", point3d.x, point3d.y, point3d.z);
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

            switch (geom->GetGeometryType ())
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
Utf8String ECSqlCommand::PrimitiveToString (IECSqlValue const& value)
    {
    Utf8String out;
    auto primitiveType = value.GetColumnInfo ().GetProperty ()->GetAsPrimitiveProperty()->GetType ();
    return PrimitiveToString (value, primitiveType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8String ECSqlCommand::ArrayToString (IECSqlValue const& value, ECN::ECPropertyCP property)
    {
    Utf8String out = "[";
    bool isFirstRow = true;
    IECSqlArrayValue const& arrayValue = value.GetArray ();
    for (IECSqlValue const* arrayElementValue : arrayValue)
        {
        if (!isFirstRow)
            out.append (", ");

        auto arrayProperty = property->GetAsArrayProperty ();
        if (arrayProperty->GetKind () == ECN::ArrayKind::ARRAYKIND_Primitive)
            out.append (PrimitiveToString (*arrayElementValue, arrayProperty->GetPrimitiveElementType ()));
        else
            out.append (StructToString (*arrayElementValue));

        isFirstRow = false;
        }

    out.append("]");
    return out;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
Utf8String ECSqlCommand::StructToString (IECSqlValue const& value)
    {
    IECSqlStructValue const& structValue = value.GetStruct ();
    Utf8String out;
    out.append ("{");
    bool isFirst = true;
    for (int i = 0; i < structValue.GetMemberCount (); i++)
        {
        if (!isFirst)
            out.append (", ");

        IECSqlValue const& structMemberValue = structValue.GetValue (i);
        auto property = structMemberValue.GetColumnInfo ().GetProperty ();
        BeAssert (property != nullptr && "ColumnInfo::GetProperty can be null.");
        if (property->GetIsPrimitive())
            out.append (PrimitiveToString (structMemberValue));
        else if (property->GetIsStruct())
            out.append (StructToString (structMemberValue));
        else
            out.append (ArrayToString (structMemberValue, property));

        isFirst = false;
        }

    out.append ("}");
    return out;
    }

