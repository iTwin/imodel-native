/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
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
                       "PhenomenonId INTEGER NOT NULL REFERENCES " TABLE_Phenomenon "(Id) ON DELETE NO ACTION," \
                       "UnitSystemId INTEGER REFERENCES " TABLE_UnitSystem "(Id) ON DELETE NO ACTION," \
                       "Definition TEXT COLLATE NOCASE," \
                       "Numerator REAL," \
                       "Denominator REAL," \
                       "Offset REAL," \
                       "IsConstant BOOLEAN," \
                       "InvertingUnitId INTEGER REFERENCES " TABLE_Unit "(Id) ON DELETE NO ACTION);" \
                       "CREATE INDEX ix_ec_Unit_SchemaId ON " TABLE_Unit "(SchemaId);" \
                       "CREATE INDEX ix_ec_Unit_Name ON " TABLE_Unit "(Name);" \
                       "CREATE INDEX ix_ec_Unit_PhenomenonId ON " TABLE_Unit "(PhenomenonId);" \
                       "CREATE INDEX ix_ec_Unit_UnitSystemId ON " TABLE_Unit "(UnitSystemId);" \
                       "CREATE INDEX ix_ec_Unit_InvertingUnitId ON " TABLE_Unit "(InvertingUnitId);"

#define TABLEDDL_Format "CREATE TABLE " TABLE_Format \
                       "(Id INTEGER PRIMARY KEY," \
                       "SchemaId INTEGER NOT NULL REFERENCES " TABLE_Schema "(Id) ON DELETE CASCADE," \
                       "Name TEXT NOT NULL COLLATE NOCASE," \
                       "DisplayLabel TEXT," \
                       "Description TEXT," \
                       "NumericSpec TEXT," \
                       "CompositeSpec TEXT);" \
                       "CREATE INDEX ix_ec_Format_SchemaId ON " TABLE_Format "(SchemaId);" \
                       "CREATE INDEX ix_ec_Format_Name ON " TABLE_Format "(Name);"

#define TABLEDDL_FormatCompositeUnit "CREATE TABLE " TABLE_FormatCompositeUnit \
                       "(Id INTEGER PRIMARY KEY," \
                       "FormatId INTEGER NOT NULL REFERENCES " TABLE_Format "(Id) ON DELETE CASCADE," \
                       "Label TEXT," \
                       "UnitId INTEGER REFERENCES " TABLE_Unit "(Id) ON DELETE NO ACTION," \
                       "Ordinal INTEGER NOT NULL);" \
                       "CREATE UNIQUE INDEX uix_ec_FormatCompositeUnit_FormatId_Ordinal ON " TABLE_FormatCompositeUnit "(FormatId,Ordinal);" \
                       "CREATE INDEX ix_ec_FormatCompositeUnit_UnitId ON " TABLE_FormatCompositeUnit "(UnitId);"

//=======================================================================================
// @bsiclass
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
// @bsiclass
//+===============+===============+===============+===============+===============+======
class ProfileUpgrader_4003 final : public ProfileUpgrader
    {
    DbResult _Upgrade(ECDbCR) const override;
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ProfileUpgrader_4002 final : ProfileUpgrader
    {
//intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
private:
    static constexpr Utf8CP UnitsSchemaName = "Units";
    static constexpr Utf8CP FormatsSchemaName = "Formats";

    struct KoqConversionContext final
        {
        ECDbCR m_ecdb;
        ECN::ECSchemaPtr m_unitsSchema = nullptr;
        ECN::ECSchemaPtr m_formatsSchema = nullptr;
        bset<ECN::ECSchemaId> m_schemasToReferenceUnits;
        bset<ECN::ECSchemaId> m_schemasToReferenceFormats;

        explicit KoqConversionContext(ECDbCR ecdb) : m_ecdb(ecdb) {}

        bool AreStandardSchemasDeserialized() const { return m_unitsSchema != nullptr; }
        bool NeedsToImportFormatSchema() const { return !m_schemasToReferenceFormats.empty(); }
        };

    DbResult _Upgrade(ECDbCR) const override;

    static DbResult UpgradeECEnums(ECDbCR);
    static DbResult UpgradeKoqs(ECDbCR);
    static BentleyStatus ConvertKoqFuses(KoqConversionContext&);
    static ECN::ECSchemaId InsertSchemaStub(ECDbCR, Utf8StringCR schemaName, Utf8StringCR alias);
    static BentleyStatus InsertReferencesToUnitsAndFormatsSchema(KoqConversionContext&, ECN::ECSchemaId unitsSchemaId, ECN::ECSchemaId formatsSchemaId);
    static BentleyStatus ImportFullUnitsAndFormatsSchemas(KoqConversionContext&);
    static BentleyStatus DeserializeUnitsAndFormatsSchemas(KoqConversionContext&);
    static DbResult FixMetaSchemaClassMapCAXml(ECDbCR);
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ProfileUpgrader_4001 final : ProfileUpgrader
    {
    //intentionally use compiler generated ctor, dtor, copy ctor and copy assignment op
    private:
        DbResult _Upgrade(ECDbCR) const override;
    };

//=======================================================================================
// @bsiclass
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
