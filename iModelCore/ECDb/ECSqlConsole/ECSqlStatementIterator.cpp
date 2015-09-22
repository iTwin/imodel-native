/*--------------------------------------------------------------------------------------+
|
|     $Source: ECSqlConsole/ECSqlStatementIterator.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "Console.h"
#include "ECSqlStatementIterator.h"

using namespace std;
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC


//! WIP==============================================================================<<<<<

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void ListDataWriter::OnRest()
    {
    m_indent = 0;
    m_linePrefix.clear();
    m_indexTabSize = 2;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void ListDataWriter::OnPrimitive(ECSqlPropertyPath const& propertyPath, int rowId, ECN::ECValueCR value)
    {
    auto property = Utf8String(propertyPath.GetLeafEntry().GetProperty()->GetName().c_str());
    Console::Write(m_linePrefix.c_str());
    if (rowId > 0 )
        Console::Write("%2d. %s -> ", rowId, property.c_str());
    else
        Console::Write("%s : ", property.c_str());

    if (value.IsNull())
        {
        Console::WriteLine("<null>");
        return;
        }

    switch (value.GetPrimitiveType())
        {
        case ECN::PRIMITIVETYPE_Binary:
            {
            size_t blobSize = -1;
            value.GetBinary(blobSize);
            Console::WriteLine("BINARY[%d bytes]", blobSize);
            return;
            }
        case ECN::PRIMITIVETYPE_Boolean:
            Console::WriteLine("%s", value.GetBoolean() ? "True" : "False"); return;
        case ECN::PRIMITIVETYPE_DateTime:
            Console::WriteLine("%s", value.GetDateTime().ToUtf8String().c_str()); return;
        case ECN::PRIMITIVETYPE_Double:
            Console::WriteLine("%f\t", value.GetDouble()); return;
        case ECN::PRIMITIVETYPE_Integer:
            Console::WriteLine("%d", value.GetInteger()); return;
        case ECN::PRIMITIVETYPE_Long:
            Console::WriteLine("%lld", value.GetLong()); return;
        case ECN::PRIMITIVETYPE_Point2D:
            Console::WriteLine("(%2.1f, %2.1f)", value.GetPoint2D().x, value.GetPoint2D().y); return;
        case ECN::PRIMITIVETYPE_Point3D:
            Console::WriteLine("(%2.1f, %2.1f, %2.1f)", value.GetPoint3D().x, value.GetPoint3D().y, value.GetPoint3D().z); return;
        case ECN::PRIMITIVETYPE_String:
            Console::WriteLine("%s", value.GetUtf8CP()); return;
        }
    Console::WriteLine("<unknow>");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void ListDataWriter::OnRow (ECSqlPropertyPath const* propertyPath, int rowId, Event event, bool isNull)
    {
    if (propertyPath == nullptr)
        return;
    auto property = Utf8String(propertyPath->GetLeafEntry().GetProperty()->GetName().c_str());

    if (event == Event::Begin)
        {            
        Console::Write(m_linePrefix.c_str());
        if (rowId > 0 )
            Console::Write("%s [%d] : ",  property.c_str(), rowId);
        else
            Console::Write("%s : ", property.c_str());

        if (isNull)
            Console::WriteLine("<null>");
        else
            Console::WriteLine();

        Indent();            
        }
    else
        {
        Unindent();   
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void ListDataWriter::OnRows (ECSqlPropertyPath const* propertyPath, Event event)
    {
    if (propertyPath == nullptr)
        return;

    if (event == Event::Begin)
        {            
        Console::Write(m_linePrefix.c_str());
        Console::WriteLine((propertyPath->ToString() + "[]").c_str());
        Indent();
        }
    else
        {
        Unindent();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void ListDataWriter::Indent()
    {
    m_linePrefix = Utf8String(++m_indent * m_indexTabSize, ' ');
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
void ListDataWriter::Unindent()
    {
    m_linePrefix = Utf8String(--m_indent * m_indexTabSize, ' ');
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
ECSqlStatus ECSqlStatementIterator::IteratePrimitve (IECSqlIteratorCallback& callback, IECSqlValue const& ecsqlValue, int rowId, ECSqlPropertyPath const* propertyPath)
    {
    ECN::PrimitiveType primitiveType;
    BeAssert(propertyPath != nullptr);
    auto& property = *(propertyPath->GetLeafEntry().GetProperty());
    if (property.GetIsPrimitive())
        {
        auto primitiveProperty = property.GetAsPrimitiveProperty();
        primitiveType = primitiveProperty->GetType();
        }
    else
        {
        auto arrayProperty = property.GetAsArrayProperty();
        primitiveType = arrayProperty->GetPrimitiveElementType();
        }

    ECN::ECValue value;
    if (ecsqlValue.IsNull ())
        {
        value.SetToNull();
        }
    else
        {
        switch (primitiveType)
            {
            case ECN::PRIMITIVETYPE_Binary: 
                {
                int size;
                auto bytes = (const Byte*)ecsqlValue.GetBinary (&size);
                value.SetBinary(bytes, static_cast<size_t>(size), false); 
                break;
                }
            case ECN::PRIMITIVETYPE_Boolean: value.SetBoolean (ecsqlValue.GetBoolean ()); break;
            case ECN::PRIMITIVETYPE_DateTime: value.SetDateTime (ecsqlValue.GetDateTime ()); break;
            case ECN::PRIMITIVETYPE_Double: value.SetDouble (ecsqlValue.GetDouble ()); break;
            case ECN::PRIMITIVETYPE_Integer: value.SetInteger (ecsqlValue.GetInt ()); break;
            case ECN::PRIMITIVETYPE_Long: value.SetLong (ecsqlValue.GetInt64 ()); break;
            case ECN::PRIMITIVETYPE_Point2D: value.SetPoint2D (ecsqlValue.GetPoint2D ()); break;
            case ECN::PRIMITIVETYPE_Point3D: value.SetPoint3D (ecsqlValue.GetPoint3D ()); break;
            case ECN::PRIMITIVETYPE_String: value.SetUtf8CP (ecsqlValue.GetText ()); break;
            case ECN::PRIMITIVETYPE_IGeometry: break;
            }
        }
    callback.OnPrimitive(*propertyPath, rowId, value);
    return ECSqlStatus::Success;
    }
   
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
ECSqlStatus ECSqlStatementIterator::IterateCell (IECSqlIteratorCallback& callback, IECSqlValue const& ecsqlValue, int rowId)
    {
    auto status = ECSqlStatus::Success;
    const bool isNull = ecsqlValue.IsNull ();
    auto const& columnInfo = ecsqlValue.GetColumnInfo ();
    auto const& propertyPath = columnInfo.GetPropertyPath ();
    callback.OnRow (&propertyPath, rowId, IECSqlIteratorCallback::Event::Begin, isNull);
    if (!isNull)
        {
        if (columnInfo.GetProperty () == nullptr)
            {
            status = IteratePrimitve (callback, ecsqlValue, -1, &propertyPath);
            }
        else
            {
            auto property = columnInfo.GetProperty ();
            BeAssert (property != nullptr && "ColumnInfo::GetProperty can be null");

            if (property->GetIsPrimitive ())
                status = IteratePrimitve (callback, ecsqlValue, -1, &columnInfo.GetPropertyPath ());
            else if (property->GetIsStruct ())
                {
                IECSqlStructValue const& structValue = ecsqlValue.GetStruct ();
                int memberCount = structValue.GetMemberCount ();
                for (int i = 0; i < memberCount; i++)
                    {
                    status = IterateCell (callback, structValue.GetValue (i), -1);
                    if (status != ECSqlStatus::Success)
                        return status;
                    }
                }
            else
                {
                IECSqlArrayValue const& arrayValue = ecsqlValue.GetArray ();
                callback.OnRows (&propertyPath, IECSqlIteratorCallback::Event::Begin);
                for (IECSqlValue const* arrayElementValue : arrayValue)
                    {
                    status = IterateCell (callback, *arrayElementValue, -1);
                    if (status != ECSqlStatus::Success)
                        return status;
                    }
                callback.OnRows (&propertyPath, IECSqlIteratorCallback::Event::End);
                }
            }
        if (status != ECSqlStatus::Success)
            return status;

        }
    callback.OnRow (&propertyPath, rowId, IECSqlIteratorCallback::Event::End, isNull);
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     10/2013
//---------------------------------------------------------------------------------------
//static
ECSqlStatus ECSqlStatementIterator::Iterate (ECSqlStatement& statement, IECSqlIteratorCallback& callback)
    {                
    callback.OnRest ();

    callback.OnRows (nullptr, IECSqlIteratorCallback::Event::Begin);
    const int columnCount = statement.GetColumnCount ();
    int rowId = 1;
    while (BE_SQLITE_ROW == statement.Step ())
        {
        for (int columnIndex = 0; columnIndex < columnCount; columnIndex++)
            {
            IECSqlValue const& ecsqlValue = statement.GetValue (columnIndex);
            auto stat = IterateCell (callback, ecsqlValue, rowId);
            if (!stat.IsSuccess())
                return stat;
            }

        rowId++;
        }
    callback.OnRows (nullptr, IECSqlIteratorCallback::Event::End);
    return ECSqlStatus::Success;

    }
//! WIP==============================================================================>>>>>
