/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool StructECSqlField::_IsNull() const
    {
    for (auto const& field : m_structMemberFields)
        {
        if (!field.second->IsNull())
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlValue const& StructECSqlField::_GetStructMemberValue(Utf8CP memberName) const
    {
    auto it = m_structMemberFields.find(memberName);
    if (it == m_structMemberFields.end())
        {
        LOG.errorv("Struct member '%s' passed to struct IECSqlValue[Utf8CP] does not exist.", memberName);
        return NoopECSqlValue::GetSingleton()[memberName];
        }

    return *it->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void StructECSqlField::AppendField(std::unique_ptr<ECSqlField> field)
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

    Utf8CP memberName = field->GetColumnInfo().GetProperty()->GetName().c_str();
    BeAssert(m_structMemberFields.find(memberName) == m_structMemberFields.end());
    m_structMemberFields[memberName] = std::move(field);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void const* StructECSqlField::_GetBlob(int* blobSize) const
    {
    LOG.error("GetBlob cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool StructECSqlField::_GetBoolean() const
    {
    LOG.error("GetBoolean cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetBoolean();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
uint64_t StructECSqlField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const
    {
    LOG.error("GetDateTime cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
double StructECSqlField::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    LOG.error("GetDateTime cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
double StructECSqlField::_GetDouble() const
    {
    LOG.error("GetDouble cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetDouble();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
int StructECSqlField::_GetInt() const
    {
    LOG.error("GetInt cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetInt();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
int64_t StructECSqlField::_GetInt64() const
    {
    LOG.error("GetInt64 cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetInt64();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP StructECSqlField::_GetText() const
    {
    LOG.error("GetText cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetText();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
IGeometryPtr StructECSqlField::_GetGeometry() const
    {
    LOG.error("GetGeometry cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetGeometry();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d StructECSqlField::_GetPoint2d() const
    {
    LOG.error("GetPoint2d cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetPoint2d();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d StructECSqlField::_GetPoint3d() const
    {
    LOG.error("GetPoint3d cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetPoint3d();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int StructECSqlField::_GetArrayLength() const
    {
    LOG.error("GetArrayLength cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& StructECSqlField::_GetArrayIterable() const
    {
    LOG.error("GetArrayIterable cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlField::_OnAfterStep()
    {
    for (auto const& memberField : m_structMemberFields)
        {
        ECSqlStatus stat = memberField.second->OnAfterStep();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlField::_OnAfterReset()
    {
    for (auto const& memberField : m_structMemberFields)
        {
        ECSqlStatus stat = memberField.second->OnAfterReset();
        if (!stat.IsSuccess())
            return stat;
        }

    return ECSqlStatus::Success;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
