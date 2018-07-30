/*--------------------------------------------------------------------------------------+
|
|     $Source: JsInterop.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ChangeSetManager.h"
#include <Bentley/ScopedArray.h>
#include <BeSQLite/RevisionChangesFile.h>
#include <BeSQLite/BeLzma.h>

USING_NAMESPACE_BENTLEY_SQLITE

#define REVISION_FORMAT_VERSION 0x10
#define CHANGESET_LZMA_MARKER "ChangeSetLzma"
#define JSON_PROP_ContainsSchemaChanges "ContainsSchemaChanges"
#define JSON_PROP_DDL "DDL"

namespace
{
struct RevisionChangesFileReader : RevisionChangesFileReaderBase
{
  RevisionChangesFileReader(bvector<BeFileName> const &files, Db const &db) : RevisionChangesFileReaderBase(files, db) {}
};
} // namespace

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                       07/2018
//---------------------------------------------------------------------------------------
DbResult ChangeSetManager::ApplyChangeSet(bvector<BeFileName> const &blockFileNames)
{
  RevisionChangesFileReader changesetReader(blockFileNames, m_db);

  // Apply DDL, if any
  DbSchemaChangeSet dbSchemaChanges;
  bool containsSchemaChanges;
  DbResult result = changesetReader.GetSchemaChanges(containsSchemaChanges, dbSchemaChanges);
  if (result != BE_SQLITE_OK)
  {
    BeAssert(false);
    return result;
  }

  if (containsSchemaChanges && !dbSchemaChanges.IsEmpty())
  {
    DbResult status = m_db.ExecuteSql(dbSchemaChanges.ToString().c_str());
    if (BE_SQLITE_OK != status)
      return status;
  }

  // Apply normal/data changes (including ec_ table data changes)
  return changesetReader.ApplyChanges(m_db);
}