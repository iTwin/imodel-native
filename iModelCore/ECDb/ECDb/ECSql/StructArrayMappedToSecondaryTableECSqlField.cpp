/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayMappedToSecondaryTableECSqlField.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//**************** StructArrayMappedToSecondaryTableECSqlField **************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      10/2013
//---------------------------------------------------------------------------------------
StructArrayMappedToSecondaryTableECSqlField::StructArrayMappedToSecondaryTableECSqlField (ECSqlPrepareContext& parentPrepareContext, ArrayECPropertyCR arrayProperty, ECSqlColumnInfo&& parentColumnInfo)
: ECSqlField (parentPrepareContext.GetECSqlStatementR (), std::move (parentColumnInfo)), m_reader (*this, parentPrepareContext, arrayProperty)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayMappedToSecondaryTableECSqlField::_Reset ()
    {
    return m_reader.Reset(true);        
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayMappedToSecondaryTableECSqlField::_Init ()
    {
    _Reset ();
    return m_binder.Execute (GetECSqlStatementR(), GetSecondaryECSqlStatement(), IECSqlBinder::MakeCopy::Yes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
bool StructArrayMappedToSecondaryTableECSqlField::_IsNull () const 
    {
    return false; // struct arrays are always considered to be not null
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
IECSqlStructValue const& StructArrayMappedToSecondaryTableECSqlField::_GetStruct () const
    {
    ReportError (ECSqlStatus::Error, "GetStruct cannot be called for array column. Call GetArray instead.");
    BeAssert (false && "GetStruct cannot be called for array column. Call GetArray instead.");
    return NoopECSqlValue::GetSingleton ().GetStruct ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
IECSqlArrayValue const& StructArrayMappedToSecondaryTableECSqlField::_GetArray () const
    { 
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
IECSqlPrimitiveValue const& StructArrayMappedToSecondaryTableECSqlField::_GetPrimitive () const
    {
    ReportError (ECSqlStatus::Error, "GetPrimitive cannot be called for array column. Call GetArray instead.");
    BeAssert (false && "GetPrimitive cannot be called for array column. Call GetArray instead.");
    return NoopECSqlValue::GetSingleton ().GetPrimitive ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
int StructArrayMappedToSecondaryTableECSqlField::_GetArrayLength () const
    {
    return m_reader.GetArrayLength ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void StructArrayMappedToSecondaryTableECSqlField::_MoveNext (bool onInitializingIterator) const
    {
    m_reader.MoveNext (onInitializingIterator);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
bool StructArrayMappedToSecondaryTableECSqlField::_IsAtEnd () const
    {
    return m_reader.IsAtEnd ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlValue const* StructArrayMappedToSecondaryTableECSqlField::_GetCurrent () const
    {
    return &m_reader;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      10/2013
//---------------------------------------------------------------------------------------
ECSqlField::Collection const& StructArrayMappedToSecondaryTableECSqlField::GetChildren () const 
    {
    BeAssert (m_reader.GetSecondaryECSqlStatement ().GetPreparedStatementP ()->GetType () == ECSqlType::Select);
    return m_reader.GetSecondaryECSqlStatement().GetPreparedStatementP<ECSqlSelectPreparedStatement> ()->GetFields();
    }

//**************** StructArrayMappedToSecondaryTableECSqlField::Reader **************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
StructArrayMappedToSecondaryTableECSqlField::Reader::Reader (StructArrayMappedToSecondaryTableECSqlField& parentField, ECSqlPrepareContext& parentPrepareContext, ArrayECPropertyCR arrayProperty)
: m_parentField (&parentField), m_currentArrayIndex (-1), m_arrayLength (-1), m_isAtEnd (false), m_hiddenMemberStartIndex (-1)
    {
    m_arrayColumnInfo = ECSqlColumnInfo::CreateForArrayElement (m_parentField->GetColumnInfo (), m_currentArrayIndex);
    //ECSqlPrepareContext& parentPrepareContext, ArrayECPropertyCP arrayProperty, ECSqlColumnInfo const* parentColumnInfo
    m_secondaryECSqlStatement.Initialize (parentPrepareContext, &arrayProperty, &m_arrayColumnInfo);

    Reset ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                               Affan.Khan      07/2014
//---------------------------------------------------------------------------------------
void StructArrayMappedToSecondaryTableECSqlField::Reader::SetHiddenMemberStartIndex (int index)
    {
    auto nColumns = m_secondaryECSqlStatement.GetColumnCount ();
    BeAssert (index < nColumns && "SetHiddenMemeberStartIndex() parameter out of range");
    if (index < nColumns)
        m_hiddenMemberStartIndex = index;
    else
        m_hiddenMemberStartIndex = -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlColumnInfoCR StructArrayMappedToSecondaryTableECSqlField::Reader::_GetColumnInfo () const
    {
    return m_arrayColumnInfo;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
bool StructArrayMappedToSecondaryTableECSqlField::Reader::_IsNull () const
    {
    int memberCount = _GetMemberCount ();
    for (int i = 0; i < memberCount; i++)
        {
        if (!m_secondaryECSqlStatement.GetValue (i).IsNull ())
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveValue const& StructArrayMappedToSecondaryTableECSqlField::Reader::_GetPrimitive () const
    {
    m_parentField->ReportError (ECSqlStatus::Error, "GetPrimitive cannot be called for struct array element. Call GetStruct instead.");
    return NoopECSqlValue::GetSingleton ().GetPrimitive ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlStructValue const& StructArrayMappedToSecondaryTableECSqlField::Reader::_GetStruct () const
    {
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlArrayValue const& StructArrayMappedToSecondaryTableECSqlField::Reader::_GetArray () const
    {
    m_parentField->ReportError (ECSqlStatus::Error, "GetArray cannot be called for struct array element. Call GetStruct instead.");
    return NoopECSqlValue::GetSingleton ().GetArray ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
int StructArrayMappedToSecondaryTableECSqlField::Reader::GetArrayLength () const
    {
    if (m_arrayLength >= 0)
        return m_arrayLength;

    auto arrayIndexBeforeCounting = m_currentArrayIndex;
    //for ( : ) does not work with empty loops (unused local variable)
    for (auto arrayIt = m_parentField->begin (); arrayIt != m_parentField->end (); ++arrayIt)
        {}

    //now iterate to index it was before counting (a new iterator resets the secondary statement)
    for (auto arrayIt = m_parentField->begin (); m_currentArrayIndex != arrayIndexBeforeCounting && arrayIt != m_parentField->end (); ++arrayIt)
        {}

    return m_arrayLength;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
void StructArrayMappedToSecondaryTableECSqlField::Reader::MoveNext (bool onInitializingIterator)
    {
    if (onInitializingIterator)
        Reset ();

    m_currentArrayIndex++;

    if (BE_SQLITE_ROW != m_secondaryECSqlStatement.Step ())
        {
        m_arrayLength = m_currentArrayIndex;
        m_isAtEnd = true;
        }
    else
        m_arrayColumnInfo.GetPropertyPathR ().SetLeafArrayIndex (m_currentArrayIndex);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
bool StructArrayMappedToSecondaryTableECSqlField::Reader::IsAtEnd () const
    {
    return m_isAtEnd;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
int StructArrayMappedToSecondaryTableECSqlField::Reader::_GetMemberCount () const
    {
    auto nColumns = m_secondaryECSqlStatement.GetColumnCount ();
    if (m_hiddenMemberStartIndex != -1)
        return m_hiddenMemberStartIndex;

    return nColumns;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlValue const& StructArrayMappedToSecondaryTableECSqlField::Reader::_GetValue (int columnIndex) const
    {
    return m_secondaryECSqlStatement.GetValue (columnIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
//---------------------------------------------------------------------------------------
void StructArrayMappedToSecondaryTableECSqlField::Reader::Reset ()
    {
    Reset (true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      10/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayMappedToSecondaryTableECSqlField::Reader::Reset (bool resetLength)
    {
    if (resetLength) 
        m_arrayLength = -1;

    m_currentArrayIndex = -1;
    m_isAtEnd = false;

    m_arrayColumnInfo.GetPropertyPathR ().SetLeafArrayIndex (m_currentArrayIndex);

    //if secondary statement has not been prepared yet (i.e. when the reader is initialized)
    //no need to call reset (would cause an error otherwise)
    if (m_secondaryECSqlStatement.IsPrepared ())
        return m_secondaryECSqlStatement.Reset();

    return ECSqlStatus::Success;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
