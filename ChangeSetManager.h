/*--------------------------------------------------------------------------------------+
|
|     $Source: IModelJsNative.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once
#include <Bentley/BeFileName.h>
#include <BeSQLite/BeSQLite.h>

BEGIN_BENTLEY_SQLITE_NAMESPACE

struct ChangeSetManager
{
private:
  Db &m_db;

public:
  ChangeSetManager(Db &db) : m_db(db) {}
  DbResult ApplyChangeSet(bvector<BeFileName> const &blockFileNames);
};

END_BENTLEY_SQLITE_NAMESPACE
