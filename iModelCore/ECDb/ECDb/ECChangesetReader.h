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
    ECDB_EXPORT void OpenFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert = false);
    ECDB_EXPORT void OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert = false);
    ECDB_EXPORT void OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, Db const& db, bool invert = false);
    ECDB_EXPORT void Close();
    ECDB_EXPORT DbResult Step();

    // Primary value accessor
    ECDB_EXPORT IECSqlValue const& GetValue(Stage stage, int columnIndex) const;

};

END_BENTLEY_SQLITE_EC_NAMESPACE