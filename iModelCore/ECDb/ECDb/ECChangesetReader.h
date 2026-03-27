/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECSql/IECSqlFieldReader.h"
#include <BeSQLite/ChangeSet.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECDb;

//=======================================================================================
//! Implements IECSqlFieldReader backed by a BeSQLite::Changes::Change row, allowing
//! ECSqlField and its derived classes to read EC values from a changeset.
//!
//! @remarks Column indices used by this reader correspond to the raw SQLite column
//! positions within the changed table row, matching the indices in the Changes::Change.
//!
//! The reader maintains a "current stage" that determines which side of the change
//! (old or new) is returned by the IECSqlFieldReader interface methods. Direct
//! stage-aware overloads are also provided for use without ECSqlField.
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECChangesetReader final : IECSqlFieldReader
    {
    public:
        using Stage = BeSQLite::Changes::Change::Stage;

    private:
        ECDb const& m_ecdb;
        BeSQLite::Changes::Change const* m_currentChange = nullptr;
        Stage m_currentStage;

        BeSQLite::DbValue GetValue(int col, Stage stage) const;

    public:
        explicit ECChangesetReader(ECDb const& ecdb, Stage defaultStage = Stage::New)
            : m_ecdb(ecdb), m_currentStage(defaultStage)
            {}

        //! Set the change row to read from. Must be called before any value retrieval.
        void SetCurrentChange(BeSQLite::Changes::Change const& change) { m_currentChange = &change; }
        void ClearCurrentChange() { m_currentChange = nullptr; }

        //! Set the stage used by the IECSqlFieldReader interface methods.
        void SetCurrentStage(Stage stage) { m_currentStage = stage; }
        Stage GetCurrentStage() const { return m_currentStage; }

        // Stage-aware direct API
        bool IsColumnNull(int col, Stage stage) const;
        bool GetValueBoolean(int col, Stage stage) const;
        double GetValueDouble(int col, Stage stage) const;
        int GetValueInt(int col, Stage stage) const;
        int64_t GetValueInt64(int col, Stage stage) const;
        Utf8CP GetValueText(int col, Stage stage) const;
        int GetColumnBytes(int col, Stage stage) const;
        void const* GetValueBlob(int col, Stage stage) const;

        // IECSqlFieldReader (dispatch to m_currentStage)
        ECDb const& GetECDb() const override { return m_ecdb; }
        bool IsColumnNull(int col) const override { return IsColumnNull(col, m_currentStage); }
        bool GetValueBoolean(int col) const override { return GetValueBoolean(col, m_currentStage); }
        double GetValueDouble(int col) const override { return GetValueDouble(col, m_currentStage); }
        int GetValueInt(int col) const override { return GetValueInt(col, m_currentStage); }
        int64_t GetValueInt64(int col) const override { return GetValueInt64(col, m_currentStage); }
        Utf8CP GetValueText(int col) const override { return GetValueText(col, m_currentStage); }
        int GetColumnBytes(int col) const override { return GetColumnBytes(col, m_currentStage); }
        void const* GetValueBlob(int col) const override { return GetValueBlob(col, m_currentStage); }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
