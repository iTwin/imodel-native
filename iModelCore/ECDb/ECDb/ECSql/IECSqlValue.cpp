/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/IECSqlValue.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/IECSqlValue.h>
#include "IECSqlPrimitiveValue.h"
#include "ECSqlStatementNoopImpls.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//********************** IECSqlValue **************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfoCR IECSqlValue::GetColumnInfo () const
    {
    return _GetColumnInfo ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool IECSqlValue::IsNull () const
    {
    return _IsNull ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
void const* IECSqlValue::GetBinary (int* binarySize) const
    {
    return _GetPrimitive ()._GetBinary (binarySize);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool IECSqlValue::GetBoolean () const
    {
    return _GetPrimitive ()._GetBoolean ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
DateTime IECSqlValue::GetDateTime () const
    {
    DateTime::Info metadata;
    const uint64_t jdHns = GetDateTimeJulianDaysHns (metadata);

    DateTime dt;
    if (SUCCESS != DateTime::FromJulianDay (dt, jdHns, metadata))
        {
        LOG.error("ECSqlStatement::GetDateTime> Could not convert JulianDays into DateTime.");
        return DateTime ();
        }

    return dt;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 07/2014
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t IECSqlValue::GetDateTimeJulianDaysHns(DateTime::Info& metadata) const
    {
    return _GetPrimitive()._GetDateTimeJulianDaysHns(metadata);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 07/2014
//+---------------+---------------+---------------+---------------+---------------+------
double IECSqlValue::GetDateTimeJulianDays (DateTime::Info& metadata) const
    {
    return _GetPrimitive ()._GetDateTimeJulianDays (metadata);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
double IECSqlValue::GetDouble () const
    {
    return _GetPrimitive ()._GetDouble ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
int IECSqlValue::GetInt () const
    {
    return _GetPrimitive ()._GetInt ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
int64_t IECSqlValue::GetInt64 () const
    {
    return _GetPrimitive ()._GetInt64 ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP IECSqlValue::GetText () const
    {
    return _GetPrimitive ()._GetText ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d IECSqlValue::GetPoint2d () const
    {
    return _GetPrimitive ()._GetPoint2d ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d IECSqlValue::GetPoint3d () const
    {
    return _GetPrimitive ()._GetPoint3d ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 11/2014
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr IECSqlValue::GetGeometry() const
    {
    return _GetPrimitive()._GetGeometry();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 11/2014
//+---------------+---------------+---------------+---------------+---------------+------
void const* IECSqlValue::GetGeometryBlob (int* blobSize) const
    {
    return _GetPrimitive ()._GetGeometryBlob (blobSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Shaun.Sewall        10/2016
//---------------------------------------------------------------------------------------
BeGuid IECSqlValue::GetGuid() const
    {
    if (IsNull())
        return BeGuid();

    int binarySize = 0;
    void const* binaryValue = GetBinary(&binarySize);
    if (binarySize != sizeof(BeGuid))
        return BeGuid();

    BeGuid guid;
    memcpy(&guid, binaryValue, sizeof(guid));
    return guid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle  11/2016
//---------------------------------------------------------------------------------------
ECInstanceId IECSqlValue::GetNavigation(ECN::ECClassId* relationshipECClassId) const
    {
    ECSqlColumnInfo const& colInfo = GetColumnInfo();
    if (!colInfo.GetDataType().IsNavigation())
        {
        LOG.error("ECSqlStatement::GetValueNavigation> This method can only be called for a NavigationECProperty column.");
        return ECInstanceId();
        }

    IECSqlStructValue const& navPropValue = GetStruct();
    if (relationshipECClassId != nullptr)
        {
        IECSqlValue const& relClassIdVal = navPropValue.GetValue(1);
        if (relClassIdVal.IsNull())
            *relationshipECClassId = ECN::ECClassId();
        else
            *relationshipECClassId = relClassIdVal.GetId<ECN::ECClassId>();
        }

    return navPropValue.GetValue(0).GetId<ECInstanceId>();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlStructValue const& IECSqlValue::GetStruct () const
    {
    return _GetStruct ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue const& IECSqlValue::GetArray () const
    {
    return _GetArray ();
    }

//********************** IECSqlStructValue **************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
int IECSqlStructValue::GetMemberCount () const
    {
    return _GetMemberCount ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& IECSqlStructValue::GetValue (int columnIndex) const
    {
    return _GetValue (columnIndex);
    }


//********************** IECSqlArrayValue **************************************

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
int IECSqlArrayValue::GetArrayLength () const
    {
    return _GetArrayLength ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue::const_iterator IECSqlArrayValue::begin () const
    {
    return const_iterator (*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue::const_iterator IECSqlArrayValue::end () const
    {
    return const_iterator ();
    }


//********************** IECSqlArrayValue::const_iterator **************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue::const_iterator::const_iterator ()
: m_arrayValue (nullptr)
    {
    BeAssert (IsEndIterator ());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue::const_iterator::const_iterator (IECSqlArrayValue const& arrayValue)
: m_arrayValue (&arrayValue)
    {
    BeAssert (!IsEndIterator ());
    m_arrayValue->_MoveNext (true); //iterator points to first element, therefore call MoveNext to initialize the iterator
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue::const_iterator::~const_iterator ()
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue::const_iterator::const_iterator (const_iterator const& rhs)
: m_arrayValue (rhs.m_arrayValue)
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue::const_iterator& IECSqlArrayValue::const_iterator::operator= (const_iterator const& rhs)
    {
    if (this != &rhs)
        {
        m_arrayValue = rhs.m_arrayValue;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue::const_iterator::const_iterator (const_iterator&& rhs)
: m_arrayValue (std::move (rhs.m_arrayValue))
    {
    rhs.m_arrayValue = nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue::const_iterator& IECSqlArrayValue::const_iterator::operator= (const_iterator&& rhs)
    {
    if (this != &rhs)
        {
        m_arrayValue = std::move (rhs.m_arrayValue);

        rhs.m_arrayValue = nullptr;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const* IECSqlArrayValue::const_iterator::operator*() const
    {
    if (IsEndIterator ())
        {
        BeAssert (false && "Don't call * on end iterator.");
        return nullptr;
        }

    return m_arrayValue->_GetCurrent ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue::const_iterator& IECSqlArrayValue::const_iterator::operator++()
    {
    BeAssert (!IsEndIterator () && "Don't call ++ on end iterator.");

    if (!IsEndIterator ())
        m_arrayValue->_MoveNext ();

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool IECSqlArrayValue::const_iterator::operator==(const_iterator const& rhs) const
    {
    const bool lhsEoc = IsAtEnd ();
    const bool rhsEoc = rhs.IsAtEnd ();
    if (lhsEoc || rhsEoc)
        return lhsEoc == rhsEoc;

    return operator*() == rhs.operator*();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool IECSqlArrayValue::const_iterator::operator!=(const_iterator const& rhs) const
    {
    return !(*this == rhs);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool IECSqlArrayValue::const_iterator::IsAtEnd () const
    {
    return IsEndIterator () || m_arrayValue->_IsAtEnd ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool IECSqlArrayValue::const_iterator::IsEndIterator () const
    {
    return m_arrayValue == nullptr;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
