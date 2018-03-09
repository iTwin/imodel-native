/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ProfileUpgrader.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

// Unit tables are added as a whole by profile upgraders. So keep DDL centralized both for upgrader and when creating file from scratch
#define TABLEDDL_UnitSystem "CREATE TABLE " TABLE_UnitSystem \
                            "(Id INTEGER PRIMARY KEY," \
                            "SchemaId INTEGER NOT NULL REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE," \
                            "Name TEXT NOT NULL COLLATE NOCASE," \
                            "DisplayLabel TEXT," \
                            "Description TEXT);" \
                            "CREATE INDEX ix_ec_UnitSystem_SchemaId ON " TABLE_UnitSystem "(SchemaId);" \
                            "CREATE INDEX ix_ec_UnitSystem_Name ON " TABLE_UnitSystem "(Name);"

#define TABLEDDL_Phenomenon "CREATE TABLE " TABLE_Phenomenon \
                       "(Id INTEGER PRIMARY KEY," \
                       "SchemaId INTEGER NOT NULL REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE," \
                       "Name TEXT NOT NULL COLLATE NOCASE," \
                       "DisplayLabel TEXT," \
                       "Description TEXT," \
                       "Definition TEXT NOT NULL);" \
                       "CREATE INDEX ix_ec_Phenomenon_SchemaId ON " TABLE_Phenomenon "(SchemaId);" \
                       "CREATE INDEX ix_ec_Phenomenon_Name ON " TABLE_Phenomenon "(Name);"

#define TABLEDDL_Unit "CREATE TABLE " TABLE_Unit \
                       "(Id INTEGER PRIMARY KEY," \
                       "SchemaId INTEGER NOT NULL REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE," \
                       "Name TEXT NOT NULL COLLATE NOCASE," \
                       "DisplayLabel TEXT," \
                       "Description TEXT," \
                       "PhenomenonId INTEGER NOT NULL REFERENCES " TABLE_Phenomenon "(Id) ON DELETE CASCADE," \
                       "UnitSystemId INTEGER NOT NULL REFERENCES " TABLE_UnitSystem "(Id) ON DELETE NO ACTION," \
                       "Definition TEXT COLLATE NOCASE," \
                       "Numerator REAL," \
                       "Denominator REAL," \
                       "Offset REAL," \
                       "IsConstant BOOLEAN," \
                       "InvertingUnitId INTEGER REFERENCES " TABLE_Unit "(Id) ON DELETE SET NULL);" \
                       "CREATE INDEX ix_ec_Unit_SchemaId ON " TABLE_Unit "(SchemaId);" \
                       "CREATE INDEX ix_ec_Unit_Name ON " TABLE_Unit "(Name);" \
                       "CREATE INDEX ix_ec_Unit_PhenomenonId ON " TABLE_Unit "(PhenomenonId);" \
                       "CREATE INDEX ix_ec_Unit_UnitSystemId ON " TABLE_Unit "(UnitSystemId);" \
                       "CREATE INDEX ix_ec_Unit_InvertingUnitId ON " TABLE_Unit "(InvertingUnitId);"


//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ProfileUpgrader
    {
private:
    virtual DbResult _Upgrade(ECDbCR) const = 0;

protected:
    ProfileUpgrader() {}

public:
    virtual ~ProfileUpgrader() {}
    DbResult Upgrade(ECDbCR ecdb) const { return _Upgrade(ecdb); }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle     01/2018
//+===============+===============+===============+===============+===============+======
struct ProfileUpgrader_4002 final : ProfileUpgrader
    {
//intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
private:
    DbResult _Upgrade(ECDbCR) const override;

    static DbResult UpgradeECEnums(ECDbCR);
    static void UpgradeECDbEnum(bmap<int64_t, Utf8String>& enumMap, int64_t enumId, Utf8CP enumName);
    static DbResult UpgradeKoqs(ECDbCR);
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle     10/2017
//+===============+===============+===============+===============+===============+======
struct ProfileUpgrader_4001 final : ProfileUpgrader
    {
    //intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
    private:
        DbResult _Upgrade(ECDbCR) const override;
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2013
//+===============+===============+===============+===============+===============+======
struct ProfileSchemaUpgrader final
    {
private:
    ProfileSchemaUpgrader() = delete;
    ~ProfileSchemaUpgrader() = delete;

    static Utf8CP GetECDbSystemSchemaXml();

    static BentleyStatus ReadECDbSystemSchema(ECN::ECSchemaReadContextR readContext, Utf8CP ecdbFileName);
    static BentleyStatus ReadSchemaFromDisk(ECN::ECSchemaReadContextR readContext, ECN::SchemaKey&, Utf8CP ecdbFileName);

public:
    static DbResult ImportProfileSchemas(ECDbCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
