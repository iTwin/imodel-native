/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileUpgrader.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ECDbProfileUpgrader
    {
private:
    virtual SchemaVersion _GetTargetVersion () const = 0;
    virtual DbResult _Upgrade (ECDbR ecdb) const = 0;

    static DbResult AlterColumnsInView (ECDbR ecdb, Utf8CP viewName, Utf8CP allColumnNamesAfter);
    static DbResult AlterColumnsInTable (ECDbR ecdb, Utf8CP tableName, Utf8CP newDdlBody, bool recreateIndices, Utf8CP allColumnNamesAfter, Utf8CP matchingColumnNamesWithOldNames);
    static bool IsView (ECDbCR ecdb, Utf8CP tableOrViewName);
    static DbResult RetrieveIndexDdlListForTable (std::vector<Utf8String>& indexDdlList, ECDbR ecdb, Utf8CP tableName);

protected:
    //! Drops a table or view
    //! @remarks Does not check whether the table / view exists or not.
    //! @param[in] ecdb ECDb file handle
    //! @param[in] tableOrViewName Name of table / view to be dropped
    //! @return ::BE_SQLITE_OK in case of succes. Error codes otherwise.
    static DbResult DropTableOrView (ECDbR ecdb, Utf8CP tableOrViewName);

    //! Modifies column(s) in a table.
    //! If tableName refers to an empty view, the empty view is modified.
    //! @param[in] ecdb ECDb file handle
    //! @param[in] tableName Name of table to be modified
    //! @param[in] newDdlBody DDL (without CREATE TABLE <tableName>) for how the table looks like after the modification
    //! @param[in] recreateIndices true, if any indices defined for the table should be recreated after the modification.
    //!                            false, if indices are not recreated, i.e. any indices present on the table before do no longer
    //!                            exist after the modification. 
    //!                            Pass true, if the modified columns do not affect the index definitions.
    //!                            Pass false, if the modified columns affect the index definition. In this case the index
    //!                            needs to be recreated separately.
    //! @param[in] allColumnNamesAfter Comma-separated list of all columns after the modification (used for INSERT statement)
    //! @param[in] matchingColumnNamesWithOldNames Comma-separated list of the same columns as in @p allColumnNamesAfter but with their names before the modification.
    //!               Pass nullptr if the modifications don't include a column rename.
    //! @return BE_SQLITE_OK in case of success. Error codes otherwise.
    static DbResult AlterColumns (ECDbR ecdb, Utf8CP tableName, Utf8CP newDdlBody, bool recreateIndices, Utf8CP allColumnNamesAfter, Utf8CP matchingColumnNamesWithOldNames = nullptr);

public:
    virtual ~ECDbProfileUpgrader () {}
    SchemaVersion GetTargetVersion () const;
    DbResult Upgrade (ECDbR ecdb) const;
    };


//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ECDbProfileECSchemaUpgrader
    {
private:
    static ECN::SchemaKey s_ecdbfileinfoSchemaKey; // cannot be const as schema location modifies the checksum in the key (which is not relevant for us)

    ECDbProfileECSchemaUpgrader ();
    ~ECDbProfileECSchemaUpgrader ();

    static Utf8CP GetECDbSystemECSchemaXml ();

    static BentleyStatus ReadECDbSystemSchema (ECN::ECSchemaReadContextR readContext, Utf8CP ecdbFileName);
    static BentleyStatus ReadECDbFileInfoSchema (ECN::ECSchemaReadContextR readContext, Utf8CP ecdbFileName);

public:
    static DbResult ImportProfileSchemas (ECDbR ecdb, bool updateSchema = true);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
