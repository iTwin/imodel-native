/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <BeSQLite/BeSqlite.h>
#include <BeSQLite/ChangeSet.h>
#include <ECDb/ECDb.h>

#include "DbSchema.h"
#include "ECDbInternalTypes.h"
#include "PropertyMap.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ChangesetReader {
    enum class Status {
        ROW = DbResult::BE_SQLITE_ROW,
        DONE = DbResult::BE_SQLITE_DONE,
        ERROR = DbResult::BE_SQLITE_ERROR,
        ERROR_FAILED_TO_DECODE_CLASS_ID = ERROR + 1,
        ERROR_FAILED_TO_DECODE_INSTANCE_ID = ERROR + 2,
        ERROR_INSTANCE_ID_REUSED = ERROR + 3,
        ERROR_ClASS_NOT_FOUND = ERROR + 4,
        ERROR_CLASSMAP_NOT_FOUND = ERROR + 5
    };

  private:
    ECDbCR m_db;
    BeSQLite::ChangeStreamR m_changeset;
    std::unique_ptr<BeSQLite::Changes> m_changes;
    BeSQLite::Changes::Change m_change;
    struct
    {
        ECClassId fallbackClassId;
        ECInstanceId instanceId;
        ECClassId classId;
        ECClassId previousClassId;
    } m_meta;

    mutable std::map<Utf8String, std::vector<Utf8String>> m_sqliteTableSchemaCache;
    mutable std::map<Utf8String, int> m_classIdColumnIndexCache;
    const std::vector<Utf8String> *GetColumns(Utf8StringCR tableName) const;

    int GetClassIdColumnIndex(Utf8StringCR tableName) const;
    ECClassId FindClassIdFromDb(Utf8StringCR tableName, ECInstanceId instanceId);
    void ClearMetaData();
    Status SetClassMap();

  public:
    ChangesetReader(ECDbCR db, BeSQLite::ChangeStreamR changeset)
        : m_db(db), m_changeset(changeset), m_changes(nullptr), m_change(nullptr, false) {
    }

    void Reset();
    ECClassId GetFallbackClassId() const { return m_meta.fallbackClassId; }
    ECInstanceId GetInstanceId() const { return m_meta.instanceId; }
    ECClassId GetClassId() const { return m_meta.classId; }
    bool IsInstanceIdReused() const { return m_meta.previousClassId.IsValid(); }
    Status Step();
    BeSQLite::Changes::Change const &GetChange() const { return m_change; }
    IECSqlValue const &GetOldValue(int colIndex) const { return m_change.GetOldValue(colIndex); }
};

struct ChangesetField : public IECSqlValue {
protected:
    ChangesetReader &m_reader;
    ECSqlColumnInfo m_columnInfo;
    bool m_requiresOnAfterStep = false;
    bool m_requiresOnAfterReset = false;

private:
    ECSqlColumnInfoCR _GetColumnInfo() const override { return m_columnInfo; }
    virtual ECSqlStatus _OnAfterReset() { return ECSqlStatus::Success; }
    virtual ECSqlStatus _OnAfterStep() { return ECSqlStatus::Success; }

protected:
    ChangesetField(ChangesetReader &reader, ECSqlColumnInfo const &columnInfo, bool needsOnAfterStep, bool needsOnAfterReset)
        : m_reader(reader), m_columnInfo(columnInfo), m_requiresOnAfterStep(needsOnAfterStep), m_requiresOnAfterReset(needsOnAfterReset) {
    }

    const ChangesetReader &GetReader() const { return m_reader;}

public:
    virtual ~ChangesetField() {}
    bool RequiresOnAfterStep() const { return m_requiresOnAfterStep; }
    ECSqlStatus OnAfterStep() { return _OnAfterStep(); }
    bool RequiresOnAfterReset() const { return m_requiresOnAfterReset || _GetColumnInfo().IsDynamic(); }
    ECSqlStatus OnAfterReset() { return _OnAfterReset(); }
};
END_BENTLEY_SQLITE_EC_NAMESPACE