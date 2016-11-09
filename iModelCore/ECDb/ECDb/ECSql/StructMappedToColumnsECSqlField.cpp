/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructMappedToColumnsECSqlField.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
bool StructMappedToColumnsECSqlField::_IsNull() const
    {
    for (std::unique_ptr<ECSqlField> const& field : m_structFields)
        {
        if (!field->IsNull())
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
IECSqlPrimitiveValue const& StructMappedToColumnsECSqlField::_GetPrimitive() const
    {
    ReportError(ECSqlStatus::Error, "GetPrimitive cannot be called for a struct column. Call GetStruct instead.");
    BeAssert(false && "GetPrimitive cannot be called for a struct column. Call GetStruct instead.");
    return NoopECSqlValue::GetSingleton().GetPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
IECSqlArrayValue const& StructMappedToColumnsECSqlField::_GetArray() const
    {
    ReportError(ECSqlStatus::Error, "GetArray cannot be called for a struct column. Call GetStruct instead.");
    BeAssert(false && "GetArray cannot be called for a struct column. Call GetStruct instead.");
    return NoopECSqlValue::GetSingleton().GetArray();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlValue const& StructMappedToColumnsECSqlField::_GetValue(int columnIndex) const
    {
    if (columnIndex < 0 || columnIndex >= _GetMemberCount())
        {
        Utf8String errorMessage;
        errorMessage.Sprintf("Column index '%d' passed to IECSqlStructValue::GetValue is out of bounds.", columnIndex);
        ReportError(ECSqlStatus::Error, errorMessage.c_str());
        return NoopECSqlValue::GetSingleton().GetValue(columnIndex);
        }

    return *m_structFields.at(columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
void StructMappedToColumnsECSqlField::AppendField(std::unique_ptr<ECSqlField> field)
    {
    if (field == nullptr)
        {
        BeAssert(false);
        return;
        }

    if (field->RequiresOnAfterStep())
        m_requiresOnAfterStep = true;

    if (field->RequiresOnAfterReset())
        m_requiresOnAfterReset = true;

    m_structFields.push_back(move(field));
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
