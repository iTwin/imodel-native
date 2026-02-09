/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Statement& ECSqlField::GetSqliteStatement() const {
    return m_preparedECSqlStatement.GetSqliteStatement();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSqlField::SetDynamicColumnInfo(ECSqlColumnInfoCR info) {
    if (!m_ecsqlColumnInfo.IsDynamic())
        return;

    if (info.IsValid()) {
        if (info.GetDataType() != m_ecsqlColumnInfo.GetDataType() || (info.GetProperty() != m_ecsqlColumnInfo.GetProperty())) {
            m_ecsqlDynamicColumnInfo = ECSqlColumnInfo(info, true);
            _OnDynamicPropertyUpdated();
        }
    } else {
        m_ecsqlDynamicColumnInfo = ECSqlColumnInfo();
        _OnDynamicPropertyUpdated();
    }
}

END_BENTLEY_SQLITE_EC_NAMESPACE
