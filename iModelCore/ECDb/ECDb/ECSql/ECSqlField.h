/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/IECSqlValue.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlSelectPreparedStatement;

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlField : public IECSqlValue {
   protected:
    ECSqlSelectPreparedStatement& m_preparedECSqlStatement;
    ECSqlColumnInfo m_ecsqlColumnInfo;
    ECSqlColumnInfo m_ecsqlDynamicColumnInfo;
    bool m_requiresOnAfterStep = false;
    bool m_requiresOnAfterReset = false;

   private:
    ECSqlColumnInfoCR _GetColumnInfo() const override { return m_ecsqlDynamicColumnInfo.IsValid() ? m_ecsqlDynamicColumnInfo : m_ecsqlColumnInfo; }

    virtual ECSqlStatus _OnAfterReset() {
        SetDynamicColumnInfo(ECSqlColumnInfo());
        return ECSqlStatus::Success;
    }
    virtual ECSqlStatus _OnAfterStep() { return ECSqlStatus::Success; }
    virtual void _OnDynamicPropertyUpdated() {}

   protected:
    ECSqlField(ECSqlSelectPreparedStatement& ecsqlStatement, ECSqlColumnInfo const& ecsqlColumnInfo, bool needsOnAfterStep, bool needsOnAfterReset)
        : m_preparedECSqlStatement(ecsqlStatement), m_ecsqlColumnInfo(ecsqlColumnInfo), m_requiresOnAfterStep(needsOnAfterStep), m_requiresOnAfterReset(needsOnAfterReset) {}

    Statement& GetSqliteStatement() const;

   public:
    virtual ~ECSqlField() {}
    bool RequiresOnAfterStep() const { return m_requiresOnAfterStep; }
    ECSqlStatus OnAfterStep() { return _OnAfterStep(); }
    bool RequiresOnAfterReset() const { return m_requiresOnAfterReset || _GetColumnInfo().IsDynamic(); }
    ECSqlStatus OnAfterReset() { return _OnAfterReset(); }
    void SetDynamicColumnInfo(ECSqlColumnInfoCR info);
};
END_BENTLEY_SQLITE_EC_NAMESPACE