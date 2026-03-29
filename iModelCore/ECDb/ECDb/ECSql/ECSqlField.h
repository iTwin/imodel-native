/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/IECSqlValue.h>
#include "IDbRow.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlSelectPreparedStatement;

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlField : public IECSqlValue
    {
private:
    ECDbCR m_ecdb;
    std::unique_ptr<IDbRow> m_row;
    ECSqlColumnInfo m_ecsqlColumnInfo;
    ECSqlColumnInfo m_ecsqlDynamicColumnInfo;
    bool m_requiresOnAfterStep = false;
    bool m_requiresOnAfterReset = false;
private:

    ECSqlColumnInfoCR _GetColumnInfo() const override { return m_ecsqlDynamicColumnInfo.IsValid() ? m_ecsqlDynamicColumnInfo : m_ecsqlColumnInfo; }

    virtual ECSqlStatus _OnAfterReset() { SetDynamicColumnInfo(ECSqlColumnInfo()); return ECSqlStatus::Success; }
    virtual ECSqlStatus _OnAfterStep() { return ECSqlStatus::Success; }
    virtual void _OnDynamicPropertyUpdated() {}
protected:
    //! Constructor — callers supply the appropriate IDbRow implementation.
    ECSqlField(ECDbCR ecdb, std::unique_ptr<IDbRow> row, ECSqlColumnInfo const& ecsqlColumnInfo, bool needsOnAfterStep, bool needsOnAfterReset)
        : m_ecdb(ecdb), m_row(std::move(row)),
          m_ecsqlColumnInfo(ecsqlColumnInfo), m_requiresOnAfterStep(needsOnAfterStep), m_requiresOnAfterReset(needsOnAfterReset)
        {}

    IDbRow& GetRow() const { return *m_row; }
    ECDbCR GetECDb() const { return m_ecdb; }
    void SetRequiresOnAfterStep(bool req) { m_requiresOnAfterStep = req; }
    void SetRequiresOnAfterReset(bool req) { m_requiresOnAfterReset = req; }

public:
    virtual ~ECSqlField() {}
    bool RequiresOnAfterStep() const { return m_requiresOnAfterStep; }
    ECSqlStatus OnAfterStep() { return _OnAfterStep(); }
    bool RequiresOnAfterReset() const { return m_requiresOnAfterReset || _GetColumnInfo().IsDynamic(); }
    ECSqlStatus OnAfterReset() { return _OnAfterReset(); }
    void SetDynamicColumnInfo(ECSqlColumnInfoCR info);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE