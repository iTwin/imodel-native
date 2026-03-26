/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/

#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using Stage = Changes::Change::Stage;

IECSqlValue const& ECChangesetReader::GetValue(Stage stage, int columnIndex) const {
    DbSchema const& dbSchema = m_ecdb.Schemas().Main().GetDbSchema();
    DbTable const* dbTable = dbSchema.FindTable(GetTableName());
    std::vector<ChangesetFieldFactory::FieldPtr> const& fields = ChangesetFieldFactory::Create(m_ecdb, *dbTable, *m_changeIter, stage);
    if(columnIndex < 0 || columnIndex >= (int)fields.size()) {
        LOG.errorv("Column index %d is out of range for table '%s'.", columnIndex, GetTableName().c_str());
        return NoopECSqlValue::GetSingleton();
    }
    return *(fields[columnIndex]);
}

END_BENTLEY_SQLITE_EC_NAMESPACE