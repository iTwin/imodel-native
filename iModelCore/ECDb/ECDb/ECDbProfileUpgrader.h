/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbProfileUpgrader.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    virtual DbResult _Upgrade(ECDbCR) const = 0;

    static DbResult AlterColumnsInView(ECDbCR, Utf8CP viewName, Utf8CP allColumnNamesAfter);
    static DbResult AlterColumnsInTable(ECDbCR, Utf8CP tableName, Utf8CP newDdlBody, bool recreateIndices, Utf8CP allColumnNamesAfter, Utf8CP matchingColumnNamesWithOldNames);
    static bool IsView(ECDbCR, Utf8CP tableOrViewName);
    static DbResult RetrieveIndexDdlListForTable(std::vector<Utf8String>& indexDdlList, ECDbCR, Utf8CP tableName);

protected:
    //! Drops a table or view
    //! @remarks Does not check whether the table / view exists or not.
    //! @param[in] ecdb ECDb file handle
    //! @param[in] tableOrViewName Name of table / view to be dropped
    //! @return ::BE_SQLITE_OK in case of succes. Error codes otherwise.
    static DbResult DropTableOrView(ECDbCR ecdb, Utf8CP tableOrViewName);

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
    static DbResult AlterColumns(ECDbR, Utf8CP tableName, Utf8CP newDdlBody, bool recreateIndices, Utf8CP allColumnNamesAfter, Utf8CP matchingColumnNamesWithOldNames = nullptr);

//    static DbResult AlterTables(ECDbR, bvector<Utf8String> alterTableSqls);
//    static BentleyStatus CheckIntegrity(ECDbCR);
public:
    virtual ~ECDbProfileUpgrader() {}
    DbResult Upgrade(ECDbCR ecdb) const { return _Upgrade(ecdb); }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      06/2016
//+===============+===============+===============+===============+===============+======
struct ECDbProfileUpgrader_3712 : ECDbProfileUpgrader
    {
    //intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
    private:
        virtual DbResult _Upgrade(ECDbCR) const override;
    };

//=======================================================================================
// @bsiclass                                                 Affan Khan      06/2016
//+===============+===============+===============+===============+===============+======
struct ECDbProfileUpgrader_3711 : ECDbProfileUpgrader
    {
    //intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
    private:
        virtual DbResult _Upgrade(ECDbCR) const override;
    };


//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      05/2016
//+===============+===============+===============+===============+===============+======
struct ECDbProfileUpgrader_3710 : ECDbProfileUpgrader
    {
    //intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
private:
    virtual DbResult _Upgrade(ECDbCR) const override;
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ECDbProfileECSchemaUpgrader
    {
private:
    ECDbProfileECSchemaUpgrader();
    ~ECDbProfileECSchemaUpgrader();

    static Utf8CP GetECDbSystemECSchemaXml();

    static BentleyStatus ReadECDbSystemSchema(ECN::ECSchemaReadContextR readContext, Utf8CP ecdbFileName);
    static BentleyStatus ReadSchemaFromDisk(ECN::ECSchemaReadContextR readContext, ECN::SchemaKey&, Utf8CP ecdbFileName);

public:
    static DbResult ImportProfileSchemas(ECDbCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
