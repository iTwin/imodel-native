/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECChangesetReader provides EC-typed value access for iterating over changesets.
//! It follows the same PIMPL pattern as ECSqlStatement.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECChangesetReader {
public:
    struct Impl;
    using Stage = Changes::Change::Stage;

private:
    Impl* m_pimpl = nullptr;

    ECChangesetReader(ECChangesetReader const&) = delete;
    ECChangesetReader& operator=(ECChangesetReader const&) = delete;

public:
    ECChangesetReader();
    ~ECChangesetReader();
    ECChangesetReader(ECChangesetReader&& rhs);
    ECChangesetReader& operator=(ECChangesetReader&& rhs);

    // Lifecycle
    void OpenFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert = false);
    void OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert = false);
    void OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, Db const& db, bool invert = false);
    void Close();
    DbResult Step();

    // Row metadata
    bool         IsOpen()        const;
    Utf8StringCR GetTableName()  const;
    DbOpcode     GetOpcode()     const;
    bool         IsDirect()      const;
    bool         IsUpdate()      const;
    bool         IsInsert()      const;
    bool         IsDelete()      const;
    bool         IsIndirect()    const;
    int          GetColumnCount() const;
    Utf8StringCR GetDdl()        const;

    // Primary value accessor
    IECSqlValue const& GetValue(Stage stage, int columnIndex) const;

    // Convenience accessors
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