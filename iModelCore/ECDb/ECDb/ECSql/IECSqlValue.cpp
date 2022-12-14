/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/IECSqlValue.h>
#include "ECSqlStatementNoopImpls.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//********************** IECSqlValue **************************************
//--------------------------------------------------------------------------------------
//not inlined to prevent it from being called outside of ECDb
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue::IECSqlValue() {}

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfoCR IECSqlValue::GetColumnInfo() const { return _GetColumnInfo(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool IECSqlValue::IsNull() const { return _IsNull(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void const* IECSqlValue::GetBlob(int* binarySize) const { return _GetBlob(binarySize); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool IECSqlValue::GetBoolean() const { return _GetBoolean(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DateTime IECSqlValue::GetDateTime() const
    {
    if (IsNull())
        return DateTime();

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
            switch (metadata.GetComponent())
                {
                    case DateTime::Component::Date:
                        metaDataStr = "DateTime::Component::Date";
                        break;
                    case DateTime::Component::TimeOfDay:
                        metaDataStr = "DateTime::Component::TimeOfDay";
                        break;
                    default:
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
                }
            LOG.errorv("ECSqlStatement::GetDateTime> Could not convert JulianDays value (%.1f, %s) into DateTime.", DateTime::MsecToRationalDay(jdMsec), metaDataStr);
            }

        return DateTime();
        }

    return dt;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t IECSqlValue::GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const { return _GetDateTimeJulianDaysMsec(metadata); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
double IECSqlValue::GetDateTimeJulianDays(DateTime::Info& metadata) const { return _GetDateTimeJulianDays(metadata); }


//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
double IECSqlValue::GetDouble() const { return _GetDouble(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int IECSqlValue::GetInt() const { return _GetInt(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int64_t IECSqlValue::GetInt64() const { return _GetInt64(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP IECSqlValue::GetText() const { return _GetText(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d IECSqlValue::GetPoint2d() const { return _GetPoint2d(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d IECSqlValue::GetPoint3d() const { return _GetPoint3d(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr IECSqlValue::GetGeometry() const { return _GetGeometry(); }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------------------------------------------------------------------------------
ECN::ECEnumeratorCP IECSqlValue::GetEnum() const
    {
    if (IsNull())
        return nullptr;

    ECN::ECEnumerationCP ecEnum = GetColumnInfo().GetEnumType();
    if (ecEnum == nullptr)
        {
        LOG.error("ECSqlStatement::GetEnum> This method can only be called for a column backed by a property of an enumeration type.");
        return nullptr;
        }

    if (ecEnum->GetType() == ECN::PRIMITIVETYPE_Integer)
        return ecEnum->FindEnumerator(GetInt());
    else if (ecEnum->GetType() == ECN::PRIMITIVETYPE_String)
        return ecEnum->FindEnumerator(GetText());

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus IECSqlValue::TryGetContainedEnumerators(bvector<ECN::ECEnumeratorCP>& enumerators) const
    {
    if (IsNull())
        return SUCCESS;

    ECN::ECEnumerationCP enumType = GetColumnInfo().GetEnumType();
    if (enumType == nullptr)
        {
        LOG.error("ECSqlStatement::TryGetContainedEnumerators> This method can only be called for a column backed by a property of an enumeration type.");
        return ERROR;
        }

    if (enumType->GetType() == ECN::PRIMITIVETYPE_Integer)
        {
        const int val = GetInt();
        int remainder = val;
        for (ECN::ECEnumeratorCP enumerator : enumType->GetEnumerators())
            {
            const int enumeratorVal = (int) enumerator->GetInteger();
            if (enumeratorVal == val)
                {
                enumerators.clear();
                enumerators.push_back(enumerator);
                return SUCCESS;
                }

            const int r = remainder & enumeratorVal;
            if (r == 0)
                continue;

            enumerators.push_back(enumerator);
            remainder -= r;
            }

        if (remainder > 0)
            {
            enumerators.clear();
            LOG.errorv("ECSqlStatement::TryGetContainedEnumerators> The value %d cannot be broken down into a combination of ECEnumerators of the ECEnumeration '%s'.", val, enumType->GetFullName().c_str());
            return ERROR;
            }

        return SUCCESS;
        }

    if (enumType->GetType() == ECN::PRIMITIVETYPE_String)
        {
        Utf8CP val = GetText();
        bvector<Utf8String> tokens;
        BeStringUtilities::Split(val, ",;|", tokens);

        bset<Utf8String> tokenSet(tokens.begin(), tokens.end());
        for (ECN::ECEnumeratorCP enumerator : enumType->GetEnumerators())
            {
            auto it = tokenSet.find(enumerator->GetString());
            if (it == tokenSet.end())
                continue;

            enumerators.push_back(enumerator);
            tokenSet.erase(it);
            }

        if (!tokenSet.empty())
            {
            enumerators.clear();
            LOG.errorv("ECSqlStatement::TryGetContainedEnumerators> The value '%s' cannot be broken down into a combination of ECEnumerators of the ECEnumeration '%s'.", val, enumType->GetFullName().c_str());
            return ERROR;
            }

        return SUCCESS;
        }

    BeAssert(false && "Unsupported ECEnumeration backing type");
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& IECSqlValue::operator[] (Utf8CP structMemberName) const { return _GetStructMemberValue(structMemberName); }

// --------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& IECSqlValue::GetStructIterable() const { return _GetStructIterable(); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int IECSqlValue::GetArrayLength() const { return _GetArrayLength(); }

// --------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& IECSqlValue::GetArrayIterable() const { return _GetArrayIterable(); }



//********************** IECSqlValueIterable **************************************
//--------------------------------------------------------------------------------------
//not inlined to prevent it from being called outside of ECDb
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::IECSqlValueIterable() {}

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::const_iterator IECSqlValueIterable::begin() const { return _CreateIterator(); }


//********************** IECSqlValueIterable::const_iterator **************************************
//--------------------------------------------------------------------------------------
//not inlined to prevent it from being called outside of ECDb
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::const_iterator::const_iterator(std::unique_ptr<IECSqlValueIterable::IIteratorState> state)
    : m_state(std::move(state))
    {
    BeAssert(m_state != nullptr && "Call default ctor to create an end iterator");

    if (m_state != nullptr)
        m_state->_MoveToNext(true);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::const_iterator::const_iterator(const_iterator const& rhs)
    {
    if (rhs.m_state != nullptr)
        m_state = rhs.m_state->_Copy();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::const_iterator& IECSqlValueIterable::const_iterator::operator=(const_iterator&& rhs)
    {
    if (this != &rhs)
        m_state = std::move(rhs.m_state);

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& IECSqlValueIterable::const_iterator::operator*() const
    {
    if (m_state == nullptr || m_state->_IsAtEnd())
        return NoopECSqlValue::GetSingleton();

    return m_state->_GetCurrent();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::const_iterator& IECSqlValueIterable::const_iterator::operator++()
    {
    if (m_state != nullptr && !m_state->_IsAtEnd())
        m_state->_MoveToNext(false);

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable::IIteratorState::IIteratorState() {}

END_BENTLEY_SQLITE_EC_NAMESPACE
