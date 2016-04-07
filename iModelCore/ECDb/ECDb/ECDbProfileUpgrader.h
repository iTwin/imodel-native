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
    virtual SchemaVersion _GetTargetVersion() const = 0;
    virtual DbResult _Upgrade(ECDbR) const = 0;

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
    static DbResult AlterColumns(ECDbR ecdb, Utf8CP tableName, Utf8CP newDdlBody, bool recreateIndices, Utf8CP allColumnNamesAfter, Utf8CP matchingColumnNamesWithOldNames = nullptr);

public:
    virtual ~ECDbProfileUpgrader() {}
    SchemaVersion GetTargetVersion() const { return _GetTargetVersion(); }
    DbResult Upgrade(ECDbR ecdb) const { return _Upgrade(ecdb); }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbProfileUpgrader_3001 : ECDbProfileUpgrader
    {
//intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
private:
    virtual SchemaVersion _GetTargetVersion() const override { return SchemaVersion(3, 0, 0, 1); }
    virtual DbResult _Upgrade(ECDbR) const override;
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      02/2016
//+===============+===============+===============+===============+===============+======
struct ECDbProfileUpgrader_3100 : ECDbProfileUpgrader
    {
//intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
private:
    virtual SchemaVersion _GetTargetVersion() const override { return SchemaVersion(3, 1, 0, 0); }
    virtual DbResult _Upgrade(ECDbR) const override;
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct ECDbProfileUpgrader_3200 : ECDbProfileUpgrader
    {
//intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
private:
    virtual SchemaVersion _GetTargetVersion() const override { return SchemaVersion(3, 2, 0, 0); }
    virtual DbResult _Upgrade(ECDbR) const override;
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct ECDbProfileUpgrader_3201 : ECDbProfileUpgrader
    {
    //intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
    private:
        virtual SchemaVersion _GetTargetVersion() const override { return SchemaVersion(3, 2, 0, 1); }
        virtual DbResult _Upgrade(ECDbR) const override;
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      03/2016
//+===============+===============+===============+===============+===============+======
struct ECDbProfileUpgrader_3202 : ECDbProfileUpgrader
    {
    //intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
    private:
        virtual SchemaVersion _GetTargetVersion() const override { return SchemaVersion(3, 2, 0, 2); }
        virtual DbResult _Upgrade(ECDbR) const override;
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      0/2016
//+===============+===============+===============+===============+===============+======
struct ECDbProfileUpgrader_3300 : ECDbProfileUpgrader
    {
//intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
private:
    virtual SchemaVersion _GetTargetVersion() const override { return SchemaVersion(3, 3, 0, 0); }
    virtual DbResult _Upgrade(ECDbR) const override;

    //! ec_CustomAttribute stored a proprietary container type which has now been changed
    //! to store the values from the ECN::CustomAttributeContainerType enum
    static DbResult UpdateGeneralizedCustomContainerTypeInCAInstanceTable(ECDbCR);
    static DbResult SetCustomContainerType(ECDbCR, Statement&, Utf8CP caClassName, CustomAttributeContainerType);
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ECDbProfileECSchemaUpgrader
    {
private:
    static ECN::SchemaKey s_ecdbfileinfoSchemaKey; // cannot be const as schema location modifies the checksum in the key (which is not relevant for us)

    ECDbProfileECSchemaUpgrader();
    ~ECDbProfileECSchemaUpgrader();

    static Utf8CP GetECDbSystemECSchemaXml();

    static BentleyStatus ReadECDbSystemSchema(ECN::ECSchemaReadContextR readContext, Utf8CP ecdbFileName);
    static BentleyStatus ReadECDbFileInfoSchema(ECN::ECSchemaReadContextR readContext, Utf8CP ecdbFileName);

public:
    static DbResult ImportProfileSchemas(ECDbCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
