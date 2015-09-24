/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructMappedToColumnsECSqlField.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************* StructMappedToColumnsECSqlField *****************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
StructMappedToColumnsECSqlField::StructMappedToColumnsECSqlField (ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo&& ecsqlColumnInfo)
    : ECSqlField (ecsqlStatement, std::move (ecsqlColumnInfo), false, false)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
bool StructMappedToColumnsECSqlField::_IsNull () const 
    {
    for (auto& field : m_structFields)
        {
        if (!field->IsNull ())
            return false;
        }
    return true;
    }

       
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
IECSqlStructValue const& StructMappedToColumnsECSqlField::_GetStruct () const
    {
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
IECSqlPrimitiveValue const& StructMappedToColumnsECSqlField::_GetPrimitive () const
    {
    ReportError (ECSqlStatus::Error, "GetPrimitive cannot be called for a struct column. Call GetStruct instead.");
    BeAssert (false && "GetPrimitive cannot be called for a struct column. Call GetStruct instead.");
    return NoopECSqlValue::GetSingleton ().GetPrimitive ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
IECSqlArrayValue const& StructMappedToColumnsECSqlField::_GetArray () const
    { 
    ReportError (ECSqlStatus::Error, "GetArray cannot be called for a struct column. Call GetStruct instead.");
    BeAssert (false && "GetArray cannot be called for a struct column. Call GetStruct instead.");
    return NoopECSqlValue::GetSingleton ().GetArray ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlValue const& StructMappedToColumnsECSqlField::_GetValue (int columnIndex) const
    {
    if (!CanRead (columnIndex))
        return NoopECSqlValue::GetSingleton ().GetValue (columnIndex);

    return *m_structFields.at (columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
int StructMappedToColumnsECSqlField::_GetMemberCount () const
    {
    return static_cast<int>(m_structFields.size());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
ECSqlField::Collection const& StructMappedToColumnsECSqlField::GetChildren () const
    {
    return m_structFields;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
void StructMappedToColumnsECSqlField::AppendField (std::unique_ptr<ECSqlField> field)
    {
    PRECONDITION (field != nullptr, );
    
    if (field->RequiresInit())
        m_requiresInit = true;

    if (field->RequiresReset())
        m_requiresReset = true;

    m_structFields.push_back (move (field));

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
bool StructMappedToColumnsECSqlField::CanRead (int columnindex) const
    {
    const bool canRead = columnindex >= 0 && columnindex < _GetMemberCount ();
    if (!canRead)
        {
        Utf8String errorMessage;
        errorMessage.Sprintf ("Column index '%d' passed to IECSqlStructValue::GetValue is out of bounds.", columnindex);
        ReportError (ECSqlStatus::Error, errorMessage.c_str ());
        }

    return canRead;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
