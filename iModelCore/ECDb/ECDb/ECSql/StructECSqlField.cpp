/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructECSqlField.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
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
// @bsimethod                                               Krischan.Eberle      03/2014
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
// @bsimethod                                                Affan.Khan      09/2013
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
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void const* StructECSqlField::_GetBlob(int* blobSize) const
    {
    LOG.error("GetBlob cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool StructECSqlField::_GetBoolean() const
    {
    LOG.error("GetBoolean cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetBoolean();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
uint64_t StructECSqlField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const
    {
    LOG.error("GetDateTime cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
double StructECSqlField::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    LOG.error("GetDateTime cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
double StructECSqlField::_GetDouble() const
    {
    LOG.error("GetDouble cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetDouble();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
int StructECSqlField::_GetInt() const
    {
    LOG.error("GetInt cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetInt();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
int64_t StructECSqlField::_GetInt64() const
    {
    LOG.error("GetInt64 cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetInt64();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP StructECSqlField::_GetText() const
    {
    LOG.error("GetText cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetText();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    11/2014
//+---------------+---------------+---------------+---------------+---------------+--------
IGeometryPtr StructECSqlField::_GetGeometry() const
    {
    LOG.error("GetGeometry cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetGeometry();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d StructECSqlField::_GetPoint2d() const
    {
    LOG.error("GetPoint2d cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetPoint2d();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       06/2013
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d StructECSqlField::_GetPoint3d() const
    {
    LOG.error("GetPoint3d cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetPoint3d();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
int StructECSqlField::_GetArrayLength() const
    {
    LOG.error("GetArrayLength cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    02/2017
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& StructECSqlField::_GetArrayIterable() const
    {
    LOG.error("GetArrayIterable cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      09/2013
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
// @bsimethod                                                Krischan.Eberle      10/2013
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
