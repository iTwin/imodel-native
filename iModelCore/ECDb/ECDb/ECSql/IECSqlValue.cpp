/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/IECSqlValue.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/IECSqlValue.h>
#include "ECSqlStatementNoopImpls.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//********************** IECSqlValue **************************************
//--------------------------------------------------------------------------------------
//not inlined to prevent it from being called outside of ECDb
// @bsimethod                                    Krischan.Eberle                 02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue::IECSqlValue() {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfoCR IECSqlValue::GetColumnInfo() const { return _GetColumnInfo(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool IECSqlValue::IsNull() const { return _IsNull(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
void const* IECSqlValue::GetBlob(int* binarySize) const { return _GetBlob(binarySize); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool IECSqlValue::GetBoolean() const { return _GetBoolean(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
DateTime IECSqlValue::GetDateTime() const
    {
    DateTime::Info metadata;
    const uint64_t jdMsec = GetDateTimeJulianDaysMsec(metadata);
    
    //If GetDateTime is called on a select clause item which is an expression or not a DateTime ECProperty
    //metadata is not available. This is not an error, we just take DateTime::Kind::Unspecified as default
    //in this case
    if (!metadata.IsValid())
        metadata = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified);

    DateTime dt;
    if (SUCCESS != DateTime::FromJulianDay(dt, jdMsec, metadata))
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            {
            Utf8CP metaDataStr = nullptr;
            if (metadata.GetComponent() == DateTime::Component::Date)
                metaDataStr = "DateTime::Component::Date";
            else
                {
                BeAssert(metadata.GetComponent() == DateTime::Component::DateAndTime && "DateTime::Component has changed. This code must be adjusted.");
                switch (metadata.GetKind())
                    {
                        case DateTime::Kind::Utc:
                            metaDataStr = "DateTime::Kind::Utc";
                            break;
                        case DateTime::Kind::Unspecified:
                            metaDataStr = "DateTime::Kind::Unspecified";
                            break;
                        case DateTime::Kind::Local:
                            metaDataStr = "DateTime::Kind::Local";
                            break;
                        default:
                            metaDataStr = "DateTime::Kind invalid";
                            BeAssert(false && "DateTime::Kind has changed. This code must be adjusted.");
                            break;
                    }
                }

            LOG.errorv("ECSqlStatement::GetDateTime> Could not convert JulianDays value (%.1f, %s) into DateTime.", DateTime::MsecToRationalDay(jdMsec), metaDataStr);
            }

        return DateTime();
        }

    return dt;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 07/2014
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t IECSqlValue::GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const { return _GetDateTimeJulianDaysMsec(metadata); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 07/2014
//+---------------+---------------+---------------+---------------+---------------+------
double IECSqlValue::GetDateTimeJulianDays(DateTime::Info& metadata) const { return _GetDateTimeJulianDays(metadata); }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
double IECSqlValue::GetDouble() const { return _GetDouble(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
int IECSqlValue::GetInt() const { return _GetInt(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
int64_t IECSqlValue::GetInt64() const { return _GetInt64(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP IECSqlValue::GetText() const { return _GetText(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d IECSqlValue::GetPoint2d() const { return _GetPoint2d(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d IECSqlValue::GetPoint3d() const { return _GetPoint3d(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 11/2014
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr IECSqlValue::GetGeometry() const { return _GetGeometry(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Shaun.Sewall        10/2016
//---------------------------------------------------------------------------------------
BeGuid IECSqlValue::GetGuid() const
    {
    if (IsNull())
        return BeGuid();

    int blobSize = 0;
    void const* blobValue = GetBlob(&blobSize);
    if (blobSize != sizeof(BeGuid))
        return BeGuid();

    BeGuid guid;
    memcpy(&guid, blobValue, sizeof(guid));
    return guid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle  11/2016
//---------------------------------------------------------------------------------------
BeInt64Id IECSqlValue::GetNavigation(ECN::ECClassId* relationshipECClassId) const
    {
    ECSqlColumnInfo const& colInfo = GetColumnInfo();
    if (!colInfo.GetDataType().IsNavigation())
        {
        LOG.error("ECSqlStatement::GetValueNavigation> This method can only be called for a NavigationECProperty column.");
        return ECInstanceId();
        }

    if (relationshipECClassId != nullptr)
        {
        IECSqlValue const& relClassIdVal = _GetStructMemberValue(ECDBSYS_PROP_NavPropRelECClassId);
        if (relClassIdVal.IsNull())
            *relationshipECClassId = ECN::ECClassId();
        else
            *relationshipECClassId = relClassIdVal.GetId<ECN::ECClassId>();
        }

    return _GetStructMemberValue(ECDBSYS_PROP_NavPropId).GetId<BeInt64Id>();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& IECSqlValue::operator[] (Utf8CP structMemberName) const { return _GetStructMemberValue(structMemberName); }

// --------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& IECSqlValue::GetStructIterable() const { return _GetStructIterable(); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
int IECSqlValue::GetArrayLength() const { return _GetArrayLength(); }

// --------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& IECSqlValue::GetArrayIterable() const { return _GetArrayIterable(); }



//********************** IECSqlValueIterable **************************************
//--------------------------------------------------------------------------------------
//not inlined to prevent it from being called outside of ECDb
// @bsimethod                                    Krischan.Eberle                 02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::IECSqlValueIterable() {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::const_iterator IECSqlValueIterable::begin() const { return _CreateIterator(); }


//********************** IECSqlValueIterable::const_iterator **************************************
//--------------------------------------------------------------------------------------
//not inlined to prevent it from being called outside of ECDb
// @bsimethod                                    Krischan.Eberle                 02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::const_iterator::const_iterator(std::unique_ptr<IECSqlValueIterable::IIteratorState> state)
    : m_state(std::move(state)) 
    {
    BeAssert(m_state != nullptr && "Call default ctor to create an end iterator");

    if (m_state != nullptr)
        m_state->_MoveToNext(true);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::const_iterator::const_iterator(const_iterator const& rhs)
    {
    if (rhs.m_state != nullptr)
        m_state = rhs.m_state->_Copy();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::const_iterator& IECSqlValueIterable::const_iterator::operator=(const_iterator const& rhs)
    {
    if (this != &rhs)
        {
        if (rhs.m_state != nullptr)
            m_state = rhs.m_state->_Copy();
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::const_iterator& IECSqlValueIterable::const_iterator::operator=(const_iterator&& rhs)
    {
    if (this != &rhs)
        m_state = std::move(rhs.m_state);

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& IECSqlValueIterable::const_iterator::operator*() const
    {
    if (m_state == nullptr || m_state->_IsAtEnd())
        return NoopECSqlValue::GetSingleton();

    return m_state->_GetCurrent();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::const_iterator& IECSqlValueIterable::const_iterator::operator++()
    {
    if (m_state != nullptr && !m_state->_IsAtEnd())
        m_state->_MoveToNext(false);

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool IECSqlValueIterable::const_iterator::operator==(const_iterator const& rhs) const
    {
    const bool lhsEoc = m_state == nullptr || m_state->_IsAtEnd();
    const bool rhsEoc = rhs.m_state == nullptr || rhs.m_state->_IsAtEnd();
    if (lhsEoc || rhsEoc)
        return lhsEoc == rhsEoc;

    return &operator*() == &rhs.operator*();
    }

//--------------------------------------------------------------------------------------
//not inlined to prevent it from being called outside of ECDb
// @bsimethod                                    Krischan.Eberle                 02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::IIteratorState::IIteratorState() {}

END_BENTLEY_SQLITE_EC_NAMESPACE
