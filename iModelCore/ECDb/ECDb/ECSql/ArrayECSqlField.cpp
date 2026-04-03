/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************************************
// ArrayECSqlField
//************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayECSqlField::_OnAfterStep()
    {
    DoReset();

    if (GetSqliteStatement().IsColumnNull(m_sqliteColumnIndex))
        m_json.SetArray();
    else
        {
        Utf8CP jsonStr = GetSqliteStatement().GetValueText(m_sqliteColumnIndex);
        if (m_json.Parse<0>(jsonStr).HasParseError())
            {
            LOG.errorv("Could not deserialize struct array JSON: '%s'", jsonStr);
            return ECSqlStatus::Error;
            }
        }

    BeAssert(m_json.IsArray());
    m_value = std::make_unique<JsonECSqlValue>(m_preparedECSqlStatement.GetECDb(), m_json, GetColumnInfo());
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayECSqlField::_OnAfterReset()
    {
    DoReset();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ArrayECSqlField::DoReset() const
    {
    m_json.Clear();
    m_value = nullptr;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
