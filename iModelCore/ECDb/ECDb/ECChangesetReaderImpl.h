/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECChangesetReader {
    using Stage = Changes::Change::Stage;
   private:
    ChangeStream& m_changeStream;
    Changes::Change m_changeIter;
    Changes m_changes;
    ECDbCR m_ecdb;

   private:
    ECChangesetReader(ECChangesetReader const&) = delete;
    ECChangesetReader& operator=(ECChangesetReader const&) = delete;

   public:
    ECChangesetReader(ECDb& ecdb, ChangeStream& changeStream) : m_changeStream(changeStream), m_ecdb(ecdb), m_changeIter(nullptr, false), m_changes(changeStream.GetChanges()) {}
    virtual ~ECChangesetReader() {}
    Utf8StringCR GetTableName() const { return m_changeIter.GetTableName(); }
    DbOpcode GetOpcode() const { return m_changeIter.GetOpcode(); }
    bool IsDirect() const { return m_changeIter.IsDirect(); }
    bool IsUpdate() const { return m_changeIter.IsUpdate(); }
    bool IsInsert() const { return m_changeIter.IsInsert(); }
    bool IsDelete() const { return m_changeIter.IsDelete(); }
    bool IsIndirect() const { return m_changeIter.IsIndirect(); }
    ECSqlStatus Reset() {
        m_changeIter = Changes::Change(nullptr, false);
        return ECSqlStatus::Success;
    }
    int GetColumnCount() const {
        if (!m_changeIter.IsValid())
            return 0;
        return m_changeIter.GetColumnCount();
    }
    DbResult Step() {
        if (!m_changeIter.IsValid())
            m_changeIter = m_changes.begin();
        else
            ++m_changeIter;
        return m_changeIter.IsValid() ? BE_SQLITE_ROW : BE_SQLITE_DONE;
    }
    IECSqlValue const& GetValue(Stage stage, int columnIndex) const;
    ECSqlColumnInfoCR GetColumnInfo(int columnIndex, Stage stage) const { return GetValue(stage, columnIndex).GetColumnInfo(); }
    bool IsValueNull(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).IsNull(); }
    void const* GetValueBlob(Stage stage, int columnIndex, int* blobSize = nullptr) const { return GetValue(stage, columnIndex).GetBlob(blobSize); }
    bool GetValueBoolean(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetBoolean(); }
    DateTime GetValueDateTime(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetDateTime(); }
    double GetValueDouble(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetDouble(); }
    IGeometryPtr GetValueGeometry(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetGeometry(); }
    int GetValueInt(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetInt(); }
    ECN::ECEnumeratorCP GetValueEnum(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetEnum(); }
    int64_t GetValueInt64(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetInt64(); }
    DPoint2d GetValuePoint2d(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetPoint2d(); }
    DPoint3d GetValuePoint3d(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetPoint3d(); }
    Utf8CP GetValueText(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetText(); }
    template <class TBeInt64Id>
    TBeInt64Id GetValueId(Stage stage, int columnIndex) const { return TBeInt64Id(GetValueUInt64(stage, columnIndex)); }
    BeGuid GetValueGuid(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetGuid(); }
    template <class TBeInt64Id>
    TBeInt64Id GetValueNavigation(Stage stage, int columnIndex, ECN::ECClassId* relationshipECClassId = nullptr) const { return GetValue(stage, columnIndex).GetNavigation<TBeInt64Id>(relationshipECClassId); }
    uint64_t GetValueUInt64(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetUInt64(); }
};

END_BENTLEY_SQLITE_EC_NAMESPACE