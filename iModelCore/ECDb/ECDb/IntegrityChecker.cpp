/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::GetNavigationProperties (std::map<ECClassId, std::vector<std::string>>& navProps) {
	auto sql = R"sql(
		SELECT
			[P].[ClassId],
			[P].[Name]
		FROM   [main].[ec_Property] [P]
			JOIN [main].[ec_ClassMap] [M] ON [M].[ClassId] = [P].[ClassId]
		WHERE  [P].[NavigationRelationshipClassId] IS NOT NULL
				AND [M].[MapStrategy] <> 0
		ORDER  BY
				[p].[ClassId],
				[P].[Id]
	)sql";
	Statement stmt;
	auto rc = stmt.Prepare(m_conn, sql);
	if (rc != BE_SQLITE_OK) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	while((rc = stmt.Step()) == BE_SQLITE_ROW) {
		navProps[stmt.GetValueId<ECClassId>(0)].push_back(stmt.GetValueText(1));
	}
	if (rc != BE_SQLITE_DONE) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	return BE_SQLITE_OK;
}
//---------------------------------------------------------------------------------------
// @bsimethod
// does not return existing table
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::GetRootLinkTableRelationships (std::vector<ECClassId>& rootRels) {
	// On OwnTable, TablePerHierarchy and Relationship classes
	auto sql = R"sql(
		SELECT [c].[Id]
		FROM   [ec_Class] [c]
			JOIN [ec_ClassMap] [m] ON [m].[ClassId] = [c].[Id] AND [m].[MapStrategy] IN (1, 2)
		WHERE  [c].[Type] = 1
				AND NOT EXISTS (SELECT NULL
			FROM   [ec_ClassHasBaseClasses] [h]
			WHERE  [h].[ClassId] = [c].[Id]);
	)sql";
	Statement stmt;
	auto rc = stmt.Prepare(m_conn, sql);
	if (rc != BE_SQLITE_OK) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	while((rc = stmt.Step()) == BE_SQLITE_ROW) {
        rootRels.push_back(stmt.GetValueId<ECClassId>(0));
    }
	if (rc != BE_SQLITE_DONE) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	return BE_SQLITE_OK;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::GetTablePerHierarchyClasses (std::vector<ECClassId>& entityClasses) {
	//! On  TablePerHierarchy and Skip IsMixin.
	auto sql = R"sql(
		SELECT [c].[Id]
		FROM   [ec_Class] [c]
			JOIN [ec_ClassMap] [m] ON [m].[ClassId] = [c].[Id] AND [m].[MapStrategy] IN (2)
		WHERE  [c].[Type] IN (0, 1)
				AND NOT EXISTS (SELECT NULL
			FROM   [ec_ClassHasBaseClasses] [h]
			WHERE  [h].[ClassId] = [c].[Id])
				AND NOT EXISTS (SELECT NULL
			FROM   [ec_CustomAttribute] [k]
					JOIN [ec_Class] [p] ON [p].[Id] = [k].[ClassId] AND [p].[Name] = 'IsMixin'
					JOIN [ec_Schema] [s] ON [s].[Id] = [p].[SchemaId]
						AND [s].[Name] = 'CoreCustomAttributes'
			WHERE  [k].[ContainerId] = [c].[Id]);
	)sql";
	Statement stmt;
	auto rc = stmt.Prepare(m_conn, sql);
	if (rc != BE_SQLITE_OK) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	while((rc = stmt.Step()) == BE_SQLITE_ROW) {
        entityClasses.push_back(stmt.GetValueId<ECClassId>(0));
    }
	if (rc != BE_SQLITE_DONE) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	return BE_SQLITE_OK;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::GetMappedClasses (std::set<ECClassId>& mappedClassIds) {
    auto sql = "SELECT [ClassId] FROM [ec_ClassMap] WHERE [MapStrategy] <> 0 ORDER BY [ClassId]";
    Statement stmt;
	auto rc = stmt.Prepare(m_conn, sql);
	if (rc != BE_SQLITE_OK) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	while((rc = stmt.Step()) == BE_SQLITE_ROW) {
		mappedClassIds.insert(stmt.GetValueId<ECN::ECClassId>(0));
	}
	if (rc != BE_SQLITE_DONE) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckDataColumns(std::function<bool(std::string,std::string)> callback) {
	auto sql = R"sql(
		SELECT
			[t].[Name] [tbl],
			[c].[Name] [col]
		FROM   [main].[ec_Table] [t]
			JOIN [main].[ec_column] [c] ON [c].[TableId] = [t].[Id]
			LEFT JOIN PRAGMA_TABLE_INFO ([t].[name], 'main') p ON [p].[name] = [c].[name]
		WHERE  [t].[Type] <> 4
				AND [c].[IsVirtual] = 0
				AND [p].[name] IS NULL
		ORDER  BY
				[t].[Id],
				[c].[Id]
	)sql";
	Statement stmt;
	auto rc = stmt.Prepare(m_conn, sql);
	if (rc != BE_SQLITE_OK) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	while((rc = stmt.Step()) == BE_SQLITE_ROW) {
		if(!callback(stmt.GetValueText(0), stmt.GetValueText(1))) {
            return BE_SQLITE_OK;
		}
    }
	if (rc != BE_SQLITE_DONE) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckDataTableExists(std::function<bool(std::string)> callback) {
	auto sql = R"sql(
		SELECT [t].[Name] [tbl]
		FROM   [main].[ec_Table] [t]
			LEFT JOIN [main].[sqlite_master] [p] ON [p].[name] = [t].[name] AND [p].[type] = 'table'
		WHERE  [t].[Type] <> 4 AND [p].[name] IS NULL
		ORDER  BY [t].[Id]
	)sql";
	Statement stmt;
	auto rc = stmt.Prepare(m_conn, sql);
	if (rc != BE_SQLITE_OK) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	while((rc = stmt.Step()) == BE_SQLITE_ROW) {
		if (!callback(stmt.GetValueText(0))) {
            return BE_SQLITE_OK;
		}
    }
	if (rc != BE_SQLITE_DONE) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckDataIndexExists(std::function<bool(std::string)> callback) {
	auto sql = R"sql(
		SELECT [i].[Name]
		FROM   [main].[ec_Index] [i]
			LEFT JOIN [main].[sqlite_master] [m] ON [m].[name] = [i].[name] AND [m].[type] = 'index'
		WHERE  [m].[name] IS NULL
		ORDER  BY [i].[Id]
	)sql";
	Statement stmt;
	auto rc = stmt.Prepare(m_conn, sql);
	if (rc != BE_SQLITE_OK) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	while((rc = stmt.Step()) == BE_SQLITE_ROW) {
		if (!callback(stmt.GetValueText(0))) {
            return BE_SQLITE_OK;
		}
    }
	if (rc != BE_SQLITE_DONE) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckEcProfile(std::function<bool(std::string,std::string,std::string)> callback) {
	if (m_conn.GetECDbProfileVersion() <  ProfileVersion(4,0,0,2)) {
		return CheckProfileTablesAndIndexes4001AndOlder(callback);
	}
	return CheckProfileTablesAndIndexes4002AndLater(callback);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckProfileTablesAndIndexes4002AndLater(std::function<bool(std::string,std::string,std::string)> callback) {
	if (m_conn.GetECDbProfileVersion() <  ProfileVersion(4,0,0,2)) {
		m_lastError = "File profile version is < 4.0.0.2";
		return BE_SQLITE_ERROR;
	}
	/*
	SELECT PRINTF ('{"%s", "%s"},', [name], replace([sql], ', ', ','))
	FROM   [sqlite_master]
	WHERE  [tbl_name] LIKE 'ec\_%' ESCAPE '\'
			AND [sql] IS NOT NULL
			AND [type] = 'table'
	*/
	const auto metaTables = std::map<std::string,std::string> {
		{"ec_Schema", "CREATE TABLE ec_Schema(Id INTEGER PRIMARY KEY,Name TEXT UNIQUE NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,Alias TEXT UNIQUE NOT NULL COLLATE NOCASE,VersionDigit1 INTEGER NOT NULL,VersionDigit2 INTEGER NOT NULL,VersionDigit3 INTEGER NOT NULL,OriginalECXmlVersionMajor INTEGER,OriginalECXmlVersionMinor INTEGER)"},
		{"ec_SchemaReference", "CREATE TABLE ec_SchemaReference(Id INTEGER PRIMARY KEY,SchemaId INTEGER REFERENCES ec_Schema(Id) ON DELETE CASCADE,ReferencedSchemaId INTEGER REFERENCES ec_Schema(Id) ON DELETE CASCADE)"},
		{"ec_Class", "CREATE TABLE ec_Class(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,Type INTEGER NOT NULL,Modifier INTEGER NOT NULL,RelationshipStrength INTEGER,RelationshipStrengthDirection INTEGER,CustomAttributeContainerType INTEGER)"},
		{"ec_ClassHasBaseClasses", "CREATE TABLE ec_ClassHasBaseClasses(Id INTEGER PRIMARY KEY,ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,BaseClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,Ordinal INTEGER NOT NULL)"},
		{"ec_Enumeration", "CREATE TABLE ec_Enumeration(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,UnderlyingPrimitiveType INTEGER NOT NULL,IsStrict BOOLEAN NOT NULL CHECK(IsStrict IN (0,1)),EnumValues TEXT NOT NULL)"},
		{"ec_KindOfQuantity", "CREATE TABLE ec_KindOfQuantity(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,PersistenceUnit TEXT NOT NULL,RelativeError REAL NOT NULL,PresentationUnits TEXT)"},
		{"ec_UnitSystem", "CREATE TABLE ec_UnitSystem(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT)"},
		{"ec_Phenomenon", "CREATE TABLE ec_Phenomenon(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,Definition TEXT NOT NULL)"},
		{"ec_Unit", "CREATE TABLE ec_Unit(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,PhenomenonId INTEGER NOT NULL REFERENCES ec_Phenomenon(Id) ON DELETE NO ACTION,UnitSystemId INTEGER REFERENCES ec_UnitSystem(Id) ON DELETE NO ACTION,Definition TEXT COLLATE NOCASE,Numerator REAL,Denominator REAL,Offset REAL,IsConstant BOOLEAN,InvertingUnitId INTEGER REFERENCES ec_Unit(Id) ON DELETE NO ACTION)"},
		{"ec_Format", "CREATE TABLE ec_Format(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,NumericSpec TEXT,CompositeSpec TEXT)"},
		{"ec_FormatCompositeUnit", "CREATE TABLE ec_FormatCompositeUnit(Id INTEGER PRIMARY KEY,FormatId INTEGER NOT NULL REFERENCES ec_Format(Id) ON DELETE CASCADE,Label TEXT,UnitId INTEGER REFERENCES ec_Unit(Id) ON DELETE NO ACTION,Ordinal INTEGER NOT NULL)"},
		{"ec_PropertyCategory", "CREATE TABLE ec_PropertyCategory(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,Priority INTEGER)"},
		{"ec_Property", "CREATE TABLE ec_Property(Id INTEGER PRIMARY KEY,ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,IsReadonly BOOLEAN NOT NULL CHECK (IsReadonly IN (0,1)),Priority INTEGER,Ordinal INTEGER NOT NULL,Kind INTEGER NOT NULL,PrimitiveType INTEGER,PrimitiveTypeMinLength INTEGER,PrimitiveTypeMaxLength INTEGER,PrimitiveTypeMinValue NUMERIC,PrimitiveTypeMaxValue NUMERIC,EnumerationId INTEGER REFERENCES ec_Enumeration(Id) ON DELETE CASCADE,StructClassId INTEGER REFERENCES ec_Class(Id) ON DELETE CASCADE,ExtendedTypeName TEXT,KindOfQuantityId INTEGER REFERENCES ec_KindOfQuantity(Id) ON DELETE CASCADE,CategoryId INTEGER REFERENCES ec_PropertyCategory(Id) ON DELETE CASCADE,ArrayMinOccurs INTEGER,ArrayMaxOccurs INTEGER,NavigationRelationshipClassId INTEGER REFERENCES ec_Class(Id) ON DELETE CASCADE,NavigationDirection INTEGER)"},
		{"ec_PropertyPath", "CREATE TABLE ec_PropertyPath(Id INTEGER PRIMARY KEY,RootPropertyId INTEGER NOT NULL REFERENCES ec_Property(Id) ON DELETE CASCADE,AccessString TEXT NOT NULL COLLATE NOCASE)"},
		{"ec_RelationshipConstraint", "CREATE TABLE ec_RelationshipConstraint(Id INTEGER PRIMARY KEY,RelationshipClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,RelationshipEnd INTEGER NOT NULL,MultiplicityLowerLimit INTEGER NOT NULL,MultiplicityUpperLimit INTEGER,IsPolymorphic BOOLEAN NOT NULL CHECK (IsPolymorphic IN (0,1)),RoleLabel TEXT,AbstractConstraintClassId INTEGER REFERENCES ec_Class(Id) ON DELETE SET NULL)"},
		{"ec_RelationshipConstraintClass", "CREATE TABLE ec_RelationshipConstraintClass(Id INTEGER PRIMARY KEY,ConstraintId INTEGER NOT NULL REFERENCES ec_RelationshipConstraint(Id) ON DELETE CASCADE,ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE)"},
		{"ec_CustomAttribute", "CREATE TABLE ec_CustomAttribute(Id INTEGER PRIMARY KEY,ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,ContainerId INTEGER NOT NULL,ContainerType INTEGER NOT NULL,Ordinal INTEGER NOT NULL,Instance TEXT NOT NULL)"},
		{"ec_ClassMap", "CREATE TABLE ec_ClassMap(ClassId INTEGER PRIMARY KEY REFERENCES ec_Class(Id) ON DELETE CASCADE,MapStrategy INTEGER NOT NULL,ShareColumnsMode INTEGER,MaxSharedColumnsBeforeOverflow INTEGER,JoinedTableInfo INTEGER)"},
		{"ec_PropertyMap", "CREATE TABLE ec_PropertyMap(Id INTEGER PRIMARY KEY,ClassId INTEGER NOT NULL REFERENCES ec_ClassMap(ClassId) ON DELETE CASCADE,PropertyPathId INTEGER NOT NULL REFERENCES ec_PropertyPath(Id) ON DELETE CASCADE,ColumnId INTEGER NOT NULL REFERENCES ec_Column(Id) ON DELETE CASCADE)"},
		{"ec_Table", "CREATE TABLE ec_Table(Id INTEGER PRIMARY KEY,ParentTableId INTEGER REFERENCES ec_Table(Id) ON DELETE CASCADE,Name TEXT UNIQUE NOT NULL COLLATE NOCASE,Type INTEGER NOT NULL,ExclusiveRootClassId INTEGER REFERENCES ec_Class(Id) ON DELETE SET NULL,UpdatableViewName TEXT)"},
		{"ec_Column", "CREATE TABLE ec_Column(Id INTEGER PRIMARY KEY,TableId INTEGER NOT NULL REFERENCES ec_Table(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,Type INTEGER NOT NULL,IsVirtual BOOLEAN NOT NULL CHECK (IsVirtual IN (0,1)),Ordinal INTEGER NOT NULL,NotNullConstraint BOOLEAN NOT NULL CHECK (NotNullConstraint IN (0,1)),UniqueConstraint BOOLEAN NOT NULL CHECK (UniqueConstraint IN (0,1)),CheckConstraint TEXT COLLATE NOCASE,DefaultConstraint TEXT COLLATE NOCASE,CollationConstraint INTEGER NOT NULL,OrdinalInPrimaryKey INTEGER,ColumnKind INTEGER NOT NULL)"},
		{"ec_Index", "CREATE TABLE ec_Index(Id INTEGER PRIMARY KEY,Name TEXT UNIQUE NOT NULL COLLATE NOCASE,TableId INTEGER NOT NULL REFERENCES ec_Table(Id) ON DELETE CASCADE,IsUnique BOOLEAN NOT NULL CHECK (IsUnique IN (0,1)),AddNotNullWhereExp BOOLEAN NOT NULL CHECK (AddNotNullWhereExp IN (0,1)),IsAutoGenerated BOOLEAN NOT NULL CHECK (IsAutoGenerated IN (0,1)),ClassId INTEGER REFERENCES ec_Class(Id) ON DELETE CASCADE,AppliesToSubclassesIfPartial BOOLEAN NOT NULL CHECK (AppliesToSubclassesIfPartial IN (0,1)))"},
		{"ec_IndexColumn", "CREATE TABLE ec_IndexColumn(Id INTEGER PRIMARY KEY,IndexId INTEGER NOT NULL REFERENCES ec_Index (Id) ON DELETE CASCADE,ColumnId INTEGER NOT NULL REFERENCES ec_Column (Id) ON DELETE CASCADE,Ordinal INTEGER NOT NULL)"},
		{"ec_cache_ClassHasTables", "CREATE TABLE ec_cache_ClassHasTables(Id INTEGER PRIMARY KEY,ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,TableId INTEGER NOT NULL REFERENCES ec_Table(Id) ON DELETE CASCADE)"},
		{"ec_cache_ClassHierarchy", "CREATE TABLE ec_cache_ClassHierarchy(Id INTEGER PRIMARY KEY,ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,BaseClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE)"},
	};
	/*
	SELECT PRINTF ('{"%s", "%s"},', [name], replace([sql], ', ', ','))
	FROM   [sqlite_master]
	WHERE  [tbl_name] LIKE 'ec\_%' ESCAPE '\'
			AND [sql] IS NOT NULL
			AND [type] = 'index'
	*/
	const auto metaIndexes = std::map<std::string,std::string> {
		{"uix_ec_SchemaReference_SchemaId_ReferencedSchemaId", "CREATE UNIQUE INDEX uix_ec_SchemaReference_SchemaId_ReferencedSchemaId ON ec_SchemaReference(SchemaId,ReferencedSchemaId)"},
		{"ix_ec_SchemaReference_ReferencedSchemaId", "CREATE INDEX ix_ec_SchemaReference_ReferencedSchemaId ON ec_SchemaReference(ReferencedSchemaId)"},
		{"ix_ec_Class_SchemaId_Name", "CREATE INDEX ix_ec_Class_SchemaId_Name ON ec_Class(SchemaId,Name)"},
		{"ix_ec_Class_Name", "CREATE INDEX ix_ec_Class_Name ON ec_Class(Name)"},
		{"uix_ec_ClassHasBaseClasses_ClassId_BaseClassId", "CREATE UNIQUE INDEX uix_ec_ClassHasBaseClasses_ClassId_BaseClassId ON ec_ClassHasBaseClasses(ClassId,BaseClassId)"},
		{"ix_ec_ClassHasBaseClasses_BaseClassId", "CREATE INDEX ix_ec_ClassHasBaseClasses_BaseClassId ON ec_ClassHasBaseClasses(BaseClassId)"},
		{"uix_ec_ClassHasBaseClasses_ClassId_Ordinal", "CREATE UNIQUE INDEX uix_ec_ClassHasBaseClasses_ClassId_Ordinal ON ec_ClassHasBaseClasses(ClassId,Ordinal)"},
		{"ix_ec_Enumeration_SchemaId", "CREATE INDEX ix_ec_Enumeration_SchemaId ON ec_Enumeration(SchemaId)"},
		{"ix_ec_Enumeration_Name", "CREATE INDEX ix_ec_Enumeration_Name ON ec_Enumeration(Name)"},
		{"ix_ec_KindOfQuantity_SchemaId", "CREATE INDEX ix_ec_KindOfQuantity_SchemaId ON ec_KindOfQuantity(SchemaId)"},
		{"ix_ec_KindOfQuantity_Name", "CREATE INDEX ix_ec_KindOfQuantity_Name ON ec_KindOfQuantity(Name)"},
		{"ix_ec_UnitSystem_SchemaId", "CREATE INDEX ix_ec_UnitSystem_SchemaId ON ec_UnitSystem(SchemaId)"},
		{"ix_ec_UnitSystem_Name", "CREATE INDEX ix_ec_UnitSystem_Name ON ec_UnitSystem(Name)"},
		{"ix_ec_Phenomenon_SchemaId", "CREATE INDEX ix_ec_Phenomenon_SchemaId ON ec_Phenomenon(SchemaId)"},
		{"ix_ec_Phenomenon_Name", "CREATE INDEX ix_ec_Phenomenon_Name ON ec_Phenomenon(Name)"},
		{"ix_ec_Unit_SchemaId", "CREATE INDEX ix_ec_Unit_SchemaId ON ec_Unit(SchemaId)"},
		{"ix_ec_Unit_Name", "CREATE INDEX ix_ec_Unit_Name ON ec_Unit(Name)"},
		{"ix_ec_Unit_PhenomenonId", "CREATE INDEX ix_ec_Unit_PhenomenonId ON ec_Unit(PhenomenonId)"},
		{"ix_ec_Unit_UnitSystemId", "CREATE INDEX ix_ec_Unit_UnitSystemId ON ec_Unit(UnitSystemId)"},
		{"ix_ec_Unit_InvertingUnitId", "CREATE INDEX ix_ec_Unit_InvertingUnitId ON ec_Unit(InvertingUnitId)"},
		{"ix_ec_Format_SchemaId", "CREATE INDEX ix_ec_Format_SchemaId ON ec_Format(SchemaId)"},
		{"ix_ec_Format_Name", "CREATE INDEX ix_ec_Format_Name ON ec_Format(Name)"},
		{"uix_ec_FormatCompositeUnit_FormatId_Ordinal", "CREATE UNIQUE INDEX uix_ec_FormatCompositeUnit_FormatId_Ordinal ON ec_FormatCompositeUnit(FormatId,Ordinal)"},
		{"ix_ec_FormatCompositeUnit_UnitId", "CREATE INDEX ix_ec_FormatCompositeUnit_UnitId ON ec_FormatCompositeUnit(UnitId)"},
		{"ix_ec_PropertyCategory_SchemaId", "CREATE INDEX ix_ec_PropertyCategory_SchemaId ON ec_PropertyCategory(SchemaId)"},
		{"ix_ec_PropertyCategory_Name", "CREATE INDEX ix_ec_PropertyCategory_Name ON ec_PropertyCategory(Name)"},
		{"uix_ec_Property_ClassId_Name", "CREATE UNIQUE INDEX uix_ec_Property_ClassId_Name ON ec_Property(ClassId,Name)"},
		{"uix_ec_Property_ClassId_Ordinal", "CREATE UNIQUE INDEX uix_ec_Property_ClassId_Ordinal ON ec_Property(ClassId,Ordinal)"},
		{"ix_ec_Property_Name", "CREATE INDEX ix_ec_Property_Name ON ec_Property(Name)"},
		{"ix_ec_Property_EnumerationId", "CREATE INDEX ix_ec_Property_EnumerationId ON ec_Property(EnumerationId)"},
		{"ix_ec_Property_StructClassId", "CREATE INDEX ix_ec_Property_StructClassId ON ec_Property(StructClassId)"},
		{"ix_ec_Property_KindOfQuantityId", "CREATE INDEX ix_ec_Property_KindOfQuantityId ON ec_Property(KindOfQuantityId)"},
		{"ix_ec_Property_CategoryId", "CREATE INDEX ix_ec_Property_CategoryId ON ec_Property(CategoryId)"},
		{"ix_ec_Property_NavigationRelationshipClassId", "CREATE INDEX ix_ec_Property_NavigationRelationshipClassId ON ec_Property(NavigationRelationshipClassId)"},
		{"uix_ec_PropertyPath_RootPropertyId_AccessString", "CREATE UNIQUE INDEX uix_ec_PropertyPath_RootPropertyId_AccessString ON ec_PropertyPath(RootPropertyId,AccessString)"},
		{"uix_ec_RelationshipConstraint_RelationshipClassId_RelationshipEnd", "CREATE UNIQUE INDEX uix_ec_RelationshipConstraint_RelationshipClassId_RelationshipEnd ON ec_RelationshipConstraint(RelationshipClassId,RelationshipEnd)"},
		{"ix_ec_RelationshipConstraint_AbstractConstraintClassId", "CREATE INDEX ix_ec_RelationshipConstraint_AbstractConstraintClassId ON ec_RelationshipConstraint(AbstractConstraintClassId)"},
		{"uix_ec_RelationshipConstraintClass_ConstraintId_ClassId", "CREATE UNIQUE INDEX uix_ec_RelationshipConstraintClass_ConstraintId_ClassId ON ec_RelationshipConstraintClass(ConstraintId,ClassId)"},
		{"ix_ec_RelationshipConstraintClass_ClassId", "CREATE INDEX ix_ec_RelationshipConstraintClass_ClassId ON ec_RelationshipConstraintClass(ClassId)"},
		{"uix_ec_CustomAttribute_ContainerId_ContainerType_Ordinal", "CREATE UNIQUE INDEX uix_ec_CustomAttribute_ContainerId_ContainerType_Ordinal ON ec_CustomAttribute(ContainerId,ContainerType,Ordinal)"},
		{"uix_ec_CustomAttribute_ContainerId_ContainerType_ClassId", "CREATE UNIQUE INDEX uix_ec_CustomAttribute_ContainerId_ContainerType_ClassId ON ec_CustomAttribute(ContainerId,ContainerType,ClassId)"},
		{"ix_ec_CustomAttribute_ClassId", "CREATE INDEX ix_ec_CustomAttribute_ClassId ON ec_CustomAttribute(ClassId)"},
		{"uix_ec_PropertyMap_ClassId_PropertyPathId_ColumnId", "CREATE UNIQUE INDEX uix_ec_PropertyMap_ClassId_PropertyPathId_ColumnId ON ec_PropertyMap(ClassId,PropertyPathId,ColumnId)"},
		{"ix_ec_PropertyMap_PropertyPathId", "CREATE INDEX ix_ec_PropertyMap_PropertyPathId ON ec_PropertyMap(PropertyPathId)"},
		{"ix_ec_PropertyMap_ColumnId", "CREATE INDEX ix_ec_PropertyMap_ColumnId ON ec_PropertyMap(ColumnId)"},
		{"ix_ec_Table_ParentTableId", "CREATE INDEX ix_ec_Table_ParentTableId ON ec_Table(ParentTableId)"},
		{"ix_ec_Table_ExclusiveRootClassId", "CREATE INDEX ix_ec_Table_ExclusiveRootClassId ON ec_Table(ExclusiveRootClassId)"},
		{"uix_ec_Column_TableId_Name", "CREATE UNIQUE INDEX uix_ec_Column_TableId_Name ON ec_Column(TableId,Name)"},
		{"uix_ec_Column_TableId_Ordinal", "CREATE UNIQUE INDEX uix_ec_Column_TableId_Ordinal ON ec_Column(TableId,Ordinal)"},
		{"ix_ec_Index_TableId", "CREATE INDEX ix_ec_Index_TableId ON ec_Index(TableId)"},
		{"ix_ec_Index_ClassId", "CREATE INDEX ix_ec_Index_ClassId ON ec_Index(ClassId)"},
		{"uix_ec_IndexColumn_IndexId_ColumnId_Ordinal", "CREATE UNIQUE INDEX uix_ec_IndexColumn_IndexId_ColumnId_Ordinal ON ec_IndexColumn(IndexId,ColumnId,Ordinal)"},
		{"ix_ec_IndexColumn_IndexId_Ordinal", "CREATE INDEX ix_ec_IndexColumn_IndexId_Ordinal ON ec_IndexColumn(IndexId,Ordinal)"},
		{"ix_ec_IndexColumn_ColumnId", "CREATE INDEX ix_ec_IndexColumn_ColumnId ON ec_IndexColumn(ColumnId)"},
		{"ix_ec_cache_ClassHasTables_ClassId_TableId", "CREATE INDEX ix_ec_cache_ClassHasTables_ClassId_TableId ON ec_cache_ClassHasTables(ClassId)"},
		{"ix_ec_cache_ClassHasTables_TableId", "CREATE INDEX ix_ec_cache_ClassHasTables_TableId ON ec_cache_ClassHasTables(TableId)"},
		{"ix_ec_cache_ClassHierarchy_ClassId", "CREATE INDEX ix_ec_cache_ClassHierarchy_ClassId ON ec_cache_ClassHierarchy(ClassId)"},
		{"ix_ec_cache_ClassHierarchy_BaseClassId", "CREATE INDEX ix_ec_cache_ClassHierarchy_BaseClassId ON ec_cache_ClassHierarchy(BaseClassId)"},
	};
	/*
	SELECT PRINTF ('{"%s", "%s"},', [name], REPLACE(REPLACE(REPLACE(REPLACE ([sql], ', ', ','),'  ',' '),'[',''),']', ''))
	FROM   [sqlite_master]
	WHERE  ([tbl_name] LIKE 'dgn\_%' ESCAPE '\'
			OR [tbl_name] LIKE 'bis\_%' ESCAPE '\')
			AND [sql] IS NOT NULL
			AND [type] = 'trigger';
	*/
	const auto metaTriggers = std::map<std::string,std::string> {
		{"bis_Element_CurrentTimeStamp", "CREATE TRIGGER bis_Element_CurrentTimeStamp AFTER UPDATE ON bis_Element WHEN old.LastMod=new.LastMod AND old.LastMod!=julianday('now') BEGIN UPDATE bis_Element SET LastMod=julianday('now') WHERE Id=new.Id; END"},
		{"dgn_prjrange_del", "CREATE TRIGGER dgn_prjrange_del AFTER DELETE ON bis_GeometricElement3d BEGIN DELETE FROM dgn_SpatialIndex WHERE ElementId=old.ElementId;END"},
		{"dgn_rtree_upd", "CREATE TRIGGER dgn_rtree_upd AFTER UPDATE OF Origin_X,Origin_Y,Origin_Z,Yaw,Pitch,Roll,BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z ON bis_GeometricElement3d WHEN new.Origin_X IS NOT NULL AND 1 = new.InSpatialIndex BEGIN INSERT OR REPLACE INTO dgn_SpatialIndex(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5) FROM (SELECT DGN_placement_aabb(DGN_placement(DGN_point(NEW.Origin_X,NEW.Origin_Y,NEW.Origin_Z),DGN_angles(NEW.Yaw,NEW.Pitch,NEW.Roll),DGN_bbox(NEW.BBoxLow_X,NEW.BBoxLow_Y,NEW.BBoxLow_Z,NEW.BBoxHigh_X,NEW.BBoxHigh_Y,NEW.BBoxHigh_Z))) as bb);END"},
		{"dgn_rtree_upd1", "CREATE TRIGGER dgn_rtree_upd1 AFTER UPDATE OF Origin_X,Origin_Y,Origin_Z,Yaw,Pitch,Roll,BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z ON bis_GeometricElement3d WHEN OLD.Origin_X IS NOT NULL AND NEW.Origin_X IS NULL BEGIN DELETE FROM dgn_SpatialIndex WHERE ElementId=OLD.ElementId;END"},
		{"dgn_rtree_ins", "CREATE TRIGGER dgn_rtree_ins AFTER INSERT ON bis_GeometricElement3d WHEN new.Origin_X IS NOT NULL AND 1 = new.InSpatialIndex BEGIN INSERT INTO dgn_SpatialIndex(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5) FROM (SELECT DGN_placement_aabb(DGN_placement(DGN_point(NEW.Origin_X,NEW.Origin_Y,NEW.Origin_Z),DGN_angles(NEW.Yaw,NEW.Pitch,NEW.Roll),DGN_bbox(NEW.BBoxLow_X,NEW.BBoxLow_Y,NEW.BBoxLow_Z,NEW.BBoxHigh_X,NEW.BBoxHigh_Y,NEW.BBoxHigh_Z))) as bb);END"},
		{"dgn_fts_ai", "CREATE TRIGGER dgn_fts_ai AFTER INSERT ON dgn_fts_content BEGIN INSERT INTO dgn_fts_idx(rowid,Type,Id,Text) VALUES(new.rowid,new.Type,new.Id,new.Text); END"},
		{"dgn_fts_ad", "CREATE TRIGGER dgn_fts_ad AFTER DELETE ON dgn_fts_content BEGIN INSERT INTO dgn_fts_idx(dgn_fts_idx,rowid,Type,Id,Text) VALUES('delete',old.rowid,old.Type,old.Id,old.Text); END"},
		{"dgn_fts_au", "CREATE TRIGGER dgn_fts_au AFTER UPDATE ON dgn_fts_content BEGIN INSERT INTO dgn_fts_idx(dgn_fts_idx,rowid,Type,Id,Text) VALUES('delete',old.rowid,old.Type,old.Id,old.Text); INSERT INTO dgn_fts_idx(rowid,Type,Id,Text) VALUES(new.rowid,new.Type,new.Id,new.Text); END"},
	};
	return CheckEcProfile(metaTables, metaIndexes, metaTriggers, callback);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckProfileTablesAndIndexes4001AndOlder(std::function<bool(std::string,std::string,std::string)> callback) {
	if (m_conn.GetECDbProfileVersion() >  ProfileVersion(4,0,0,1)) {
		m_lastError = "File profile version is < 4.0.0.2";
		return BE_SQLITE_ERROR;
	}
	/*
	SELECT PRINTF ('{"%s", "%s"},', [name], replace([sql], ', ', ','))
	FROM   [sqlite_master]
	WHERE  [tbl_name] LIKE 'ec\_%' ESCAPE '\'
			AND [sql] IS NOT NULL
			AND [type] = 'table'
	*/
	const auto metaTables = std::map<std::string,std::string> {
		{"ec_Schema", "CREATE TABLE ec_Schema(Id INTEGER PRIMARY KEY,Name TEXT UNIQUE NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,Alias TEXT UNIQUE NOT NULL COLLATE NOCASE,VersionDigit1 INTEGER NOT NULL,VersionDigit2 INTEGER NOT NULL,VersionDigit3 INTEGER NOT NULL)"},
		{"ec_SchemaReference", "CREATE TABLE ec_SchemaReference(Id INTEGER PRIMARY KEY,SchemaId INTEGER REFERENCES ec_Schema(Id) ON DELETE CASCADE,ReferencedSchemaId INTEGER REFERENCES ec_Schema(Id) ON DELETE CASCADE)"},
		{"ec_Class", "CREATE TABLE ec_Class(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,Type INTEGER NOT NULL,Modifier INTEGER NOT NULL,RelationshipStrength INTEGER,RelationshipStrengthDirection INTEGER,CustomAttributeContainerType INTEGER)"},
		{"ec_ClassHasBaseClasses", "CREATE TABLE ec_ClassHasBaseClasses(Id INTEGER PRIMARY KEY,ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,BaseClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,Ordinal INTEGER NOT NULL)"},
		{"ec_Enumeration", "CREATE TABLE ec_Enumeration(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,UnderlyingPrimitiveType INTEGER NOT NULL,IsStrict BOOLEAN NOT NULL CHECK(IsStrict IN (0,1)),EnumValues TEXT NOT NULL)"},
		{"ec_KindOfQuantity", "CREATE TABLE ec_KindOfQuantity(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,PersistenceUnit TEXT NOT NULL,RelativeError REAL NOT NULL,PresentationUnits TEXT)"},
		{"ec_PropertyCategory", "CREATE TABLE ec_PropertyCategory(Id INTEGER PRIMARY KEY,SchemaId INTEGER NOT NULL REFERENCES ec_Schema(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,Priority INTEGER)"},
		{"ec_Property", "CREATE TABLE ec_Property(Id INTEGER PRIMARY KEY,ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,IsReadonly BOOLEAN NOT NULL CHECK (IsReadonly IN (0,1)),Priority INTEGER,Ordinal INTEGER NOT NULL,Kind INTEGER NOT NULL,PrimitiveType INTEGER,PrimitiveTypeMinLength INTEGER,PrimitiveTypeMaxLength INTEGER,PrimitiveTypeMinValue NUMERIC,PrimitiveTypeMaxValue NUMERIC,EnumerationId INTEGER REFERENCES ec_Enumeration(Id) ON DELETE CASCADE,StructClassId INTEGER REFERENCES ec_Class(Id) ON DELETE CASCADE,ExtendedTypeName TEXT,KindOfQuantityId INTEGER REFERENCES ec_KindOfQuantity(Id) ON DELETE CASCADE,CategoryId INTEGER REFERENCES ec_PropertyCategory(Id) ON DELETE CASCADE,ArrayMinOccurs INTEGER,ArrayMaxOccurs INTEGER,NavigationRelationshipClassId INTEGER REFERENCES ec_Class(Id) ON DELETE CASCADE,NavigationDirection INTEGER)"},
		{"ec_PropertyPath", "CREATE TABLE ec_PropertyPath(Id INTEGER PRIMARY KEY,RootPropertyId INTEGER NOT NULL REFERENCES ec_Property(Id) ON DELETE CASCADE,AccessString TEXT NOT NULL COLLATE NOCASE)"},
		{"ec_RelationshipConstraint", "CREATE TABLE ec_RelationshipConstraint(Id INTEGER PRIMARY KEY,RelationshipClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,RelationshipEnd INTEGER NOT NULL,MultiplicityLowerLimit INTEGER NOT NULL,MultiplicityUpperLimit INTEGER,IsPolymorphic BOOLEAN NOT NULL CHECK (IsPolymorphic IN (0,1)),RoleLabel TEXT,AbstractConstraintClassId INTEGER REFERENCES ec_Class(Id) ON DELETE SET NULL)"},
		{"ec_RelationshipConstraintClass", "CREATE TABLE ec_RelationshipConstraintClass(Id INTEGER PRIMARY KEY,ConstraintId INTEGER NOT NULL REFERENCES ec_RelationshipConstraint(Id) ON DELETE CASCADE,ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE)"},
		{"ec_CustomAttribute", "CREATE TABLE ec_CustomAttribute(Id INTEGER PRIMARY KEY,ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,ContainerId INTEGER NOT NULL,ContainerType INTEGER NOT NULL,Ordinal INTEGER NOT NULL,Instance TEXT NOT NULL)"},
		{"ec_ClassMap", "CREATE TABLE ec_ClassMap(ClassId INTEGER PRIMARY KEY REFERENCES ec_Class(Id) ON DELETE CASCADE,MapStrategy INTEGER NOT NULL,ShareColumnsMode INTEGER,MaxSharedColumnsBeforeOverflow INTEGER,JoinedTableInfo INTEGER)"},
		{"ec_PropertyMap", "CREATE TABLE ec_PropertyMap(Id INTEGER PRIMARY KEY,ClassId INTEGER NOT NULL REFERENCES ec_ClassMap(ClassId) ON DELETE CASCADE,PropertyPathId INTEGER NOT NULL REFERENCES ec_PropertyPath(Id) ON DELETE CASCADE,ColumnId INTEGER NOT NULL REFERENCES ec_Column(Id) ON DELETE CASCADE)"},
		{"ec_Table", "CREATE TABLE ec_Table(Id INTEGER PRIMARY KEY,ParentTableId INTEGER REFERENCES ec_Table(Id) ON DELETE CASCADE,Name TEXT UNIQUE NOT NULL COLLATE NOCASE,Type INTEGER NOT NULL,ExclusiveRootClassId INTEGER REFERENCES ec_Class(Id) ON DELETE SET NULL,UpdatableViewName TEXT)"},
		{"ec_Column", "CREATE TABLE ec_Column(Id INTEGER PRIMARY KEY,TableId INTEGER NOT NULL REFERENCES ec_Table(Id) ON DELETE CASCADE,Name TEXT NOT NULL COLLATE NOCASE,Type INTEGER NOT NULL,IsVirtual BOOLEAN NOT NULL CHECK (IsVirtual IN (0,1)),Ordinal INTEGER NOT NULL,NotNullConstraint BOOLEAN NOT NULL CHECK (NotNullConstraint IN (0,1)),UniqueConstraint BOOLEAN NOT NULL CHECK (UniqueConstraint IN (0,1)),CheckConstraint TEXT COLLATE NOCASE,DefaultConstraint TEXT COLLATE NOCASE,CollationConstraint INTEGER NOT NULL,OrdinalInPrimaryKey INTEGER,ColumnKind INTEGER NOT NULL)"},
		{"ec_Index", "CREATE TABLE ec_Index(Id INTEGER PRIMARY KEY,Name TEXT UNIQUE NOT NULL COLLATE NOCASE,TableId INTEGER NOT NULL REFERENCES ec_Table(Id) ON DELETE CASCADE,IsUnique BOOLEAN NOT NULL CHECK (IsUnique IN (0,1)),AddNotNullWhereExp BOOLEAN NOT NULL CHECK (AddNotNullWhereExp IN (0,1)),IsAutoGenerated BOOLEAN NOT NULL CHECK (IsAutoGenerated IN (0,1)),ClassId INTEGER REFERENCES ec_Class(Id) ON DELETE CASCADE,AppliesToSubclassesIfPartial BOOLEAN NOT NULL CHECK (AppliesToSubclassesIfPartial IN (0,1)))"},
		{"ec_IndexColumn", "CREATE TABLE ec_IndexColumn(Id INTEGER PRIMARY KEY,IndexId INTEGER NOT NULL REFERENCES ec_Index (Id) ON DELETE CASCADE,ColumnId INTEGER NOT NULL REFERENCES ec_Column (Id) ON DELETE CASCADE,Ordinal INTEGER NOT NULL)"},
		{"ec_cache_ClassHasTables", "CREATE TABLE ec_cache_ClassHasTables(Id INTEGER PRIMARY KEY,ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,TableId INTEGER NOT NULL REFERENCES ec_Table(Id) ON DELETE CASCADE)"},
		{"ec_cache_ClassHierarchy", "CREATE TABLE ec_cache_ClassHierarchy(Id INTEGER PRIMARY KEY,ClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE,BaseClassId INTEGER NOT NULL REFERENCES ec_Class(Id) ON DELETE CASCADE)"},
	};
	/*
	SELECT PRINTF ('{"%s", "%s"},', [name], replace([sql], ', ', ','))
	FROM   [sqlite_master]
	WHERE  [tbl_name] LIKE 'ec\_%' ESCAPE '\'
			AND [sql] IS NOT NULL
			AND [type] = 'index'
	*/
	const auto metaIndexes = std::map<std::string,std::string> {
		{"uix_ec_SchemaReference_SchemaId_ReferencedSchemaId", "CREATE UNIQUE INDEX uix_ec_SchemaReference_SchemaId_ReferencedSchemaId ON ec_SchemaReference(SchemaId,ReferencedSchemaId)"},
		{"ix_ec_SchemaReference_ReferencedSchemaId", "CREATE INDEX ix_ec_SchemaReference_ReferencedSchemaId ON ec_SchemaReference(ReferencedSchemaId)"},
		{"ix_ec_Class_SchemaId_Name", "CREATE INDEX ix_ec_Class_SchemaId_Name ON ec_Class(SchemaId,Name)"},
		{"ix_ec_Class_Name", "CREATE INDEX ix_ec_Class_Name ON ec_Class(Name)"},
		{"uix_ec_ClassHasBaseClasses_ClassId_BaseClassId", "CREATE UNIQUE INDEX uix_ec_ClassHasBaseClasses_ClassId_BaseClassId ON ec_ClassHasBaseClasses(ClassId,BaseClassId)"},
		{"ix_ec_ClassHasBaseClasses_BaseClassId", "CREATE INDEX ix_ec_ClassHasBaseClasses_BaseClassId ON ec_ClassHasBaseClasses(BaseClassId)"},
		{"uix_ec_ClassHasBaseClasses_ClassId_Ordinal", "CREATE UNIQUE INDEX uix_ec_ClassHasBaseClasses_ClassId_Ordinal ON ec_ClassHasBaseClasses(ClassId,Ordinal)"},
		{"ix_ec_Enumeration_SchemaId", "CREATE INDEX ix_ec_Enumeration_SchemaId ON ec_Enumeration(SchemaId)"},
		{"ix_ec_Enumeration_Name", "CREATE INDEX ix_ec_Enumeration_Name ON ec_Enumeration(Name)"},
		{"ix_ec_KindOfQuantity_SchemaId", "CREATE INDEX ix_ec_KindOfQuantity_SchemaId ON ec_KindOfQuantity(SchemaId)"},
		{"ix_ec_KindOfQuantity_Name", "CREATE INDEX ix_ec_KindOfQuantity_Name ON ec_KindOfQuantity(Name)"},
		{"ix_ec_PropertyCategory_SchemaId", "CREATE INDEX ix_ec_PropertyCategory_SchemaId ON ec_PropertyCategory(SchemaId)"},
		{"ix_ec_PropertyCategory_Name", "CREATE INDEX ix_ec_PropertyCategory_Name ON ec_PropertyCategory(Name)"},
		{"uix_ec_Property_ClassId_Name", "CREATE UNIQUE INDEX uix_ec_Property_ClassId_Name ON ec_Property(ClassId,Name)"},
		{"uix_ec_Property_ClassId_Ordinal", "CREATE UNIQUE INDEX uix_ec_Property_ClassId_Ordinal ON ec_Property(ClassId,Ordinal)"},
		{"ix_ec_Property_Name", "CREATE INDEX ix_ec_Property_Name ON ec_Property(Name)"},
		{"ix_ec_Property_EnumerationId", "CREATE INDEX ix_ec_Property_EnumerationId ON ec_Property(EnumerationId)"},
		{"ix_ec_Property_StructClassId", "CREATE INDEX ix_ec_Property_StructClassId ON ec_Property(StructClassId)"},
		{"ix_ec_Property_KindOfQuantityId", "CREATE INDEX ix_ec_Property_KindOfQuantityId ON ec_Property(KindOfQuantityId)"},
		{"ix_ec_Property_CategoryId", "CREATE INDEX ix_ec_Property_CategoryId ON ec_Property(CategoryId)"},
		{"ix_ec_Property_NavigationRelationshipClassId", "CREATE INDEX ix_ec_Property_NavigationRelationshipClassId ON ec_Property(NavigationRelationshipClassId)"},
		{"uix_ec_PropertyPath_RootPropertyId_AccessString", "CREATE UNIQUE INDEX uix_ec_PropertyPath_RootPropertyId_AccessString ON ec_PropertyPath(RootPropertyId,AccessString)"},
		{"uix_ec_RelationshipConstraint_RelationshipClassId_RelationshipEnd", "CREATE UNIQUE INDEX uix_ec_RelationshipConstraint_RelationshipClassId_RelationshipEnd ON ec_RelationshipConstraint(RelationshipClassId,RelationshipEnd)"},
		{"ix_ec_RelationshipConstraint_AbstractConstraintClassId", "CREATE INDEX ix_ec_RelationshipConstraint_AbstractConstraintClassId ON ec_RelationshipConstraint(AbstractConstraintClassId)"},
		{"uix_ec_RelationshipConstraintClass_ConstraintId_ClassId", "CREATE UNIQUE INDEX uix_ec_RelationshipConstraintClass_ConstraintId_ClassId ON ec_RelationshipConstraintClass(ConstraintId,ClassId)"},
		{"ix_ec_RelationshipConstraintClass_ClassId", "CREATE INDEX ix_ec_RelationshipConstraintClass_ClassId ON ec_RelationshipConstraintClass(ClassId)"},
		{"uix_ec_CustomAttribute_ContainerId_ContainerType_Ordinal", "CREATE UNIQUE INDEX uix_ec_CustomAttribute_ContainerId_ContainerType_Ordinal ON ec_CustomAttribute(ContainerId,ContainerType,Ordinal)"},
		{"uix_ec_CustomAttribute_ContainerId_ContainerType_ClassId", "CREATE UNIQUE INDEX uix_ec_CustomAttribute_ContainerId_ContainerType_ClassId ON ec_CustomAttribute(ContainerId,ContainerType,ClassId)"},
		{"ix_ec_CustomAttribute_ClassId", "CREATE INDEX ix_ec_CustomAttribute_ClassId ON ec_CustomAttribute(ClassId)"},
		{"uix_ec_PropertyMap_ClassId_PropertyPathId_ColumnId", "CREATE UNIQUE INDEX uix_ec_PropertyMap_ClassId_PropertyPathId_ColumnId ON ec_PropertyMap(ClassId,PropertyPathId,ColumnId)"},
		{"ix_ec_PropertyMap_PropertyPathId", "CREATE INDEX ix_ec_PropertyMap_PropertyPathId ON ec_PropertyMap(PropertyPathId)"},
		{"ix_ec_PropertyMap_ColumnId", "CREATE INDEX ix_ec_PropertyMap_ColumnId ON ec_PropertyMap(ColumnId)"},
		{"ix_ec_Table_ParentTableId", "CREATE INDEX ix_ec_Table_ParentTableId ON ec_Table(ParentTableId)"},
		{"ix_ec_Table_ExclusiveRootClassId", "CREATE INDEX ix_ec_Table_ExclusiveRootClassId ON ec_Table(ExclusiveRootClassId)"},
		{"uix_ec_Column_TableId_Name", "CREATE UNIQUE INDEX uix_ec_Column_TableId_Name ON ec_Column(TableId,Name)"},
		{"uix_ec_Column_TableId_Ordinal", "CREATE UNIQUE INDEX uix_ec_Column_TableId_Ordinal ON ec_Column(TableId,Ordinal)"},
		{"ix_ec_Index_TableId", "CREATE INDEX ix_ec_Index_TableId ON ec_Index(TableId)"},
		{"ix_ec_Index_ClassId", "CREATE INDEX ix_ec_Index_ClassId ON ec_Index(ClassId)"},
		{"uix_ec_IndexColumn_IndexId_ColumnId_Ordinal", "CREATE UNIQUE INDEX uix_ec_IndexColumn_IndexId_ColumnId_Ordinal ON ec_IndexColumn(IndexId,ColumnId,Ordinal)"},
		{"ix_ec_IndexColumn_IndexId_Ordinal", "CREATE INDEX ix_ec_IndexColumn_IndexId_Ordinal ON ec_IndexColumn(IndexId,Ordinal)"},
		{"ix_ec_IndexColumn_ColumnId", "CREATE INDEX ix_ec_IndexColumn_ColumnId ON ec_IndexColumn(ColumnId)"},
		{"ix_ec_cache_ClassHasTables_ClassId_TableId", "CREATE INDEX ix_ec_cache_ClassHasTables_ClassId_TableId ON ec_cache_ClassHasTables(ClassId)"},
		{"ix_ec_cache_ClassHasTables_TableId", "CREATE INDEX ix_ec_cache_ClassHasTables_TableId ON ec_cache_ClassHasTables(TableId)"},
		{"ix_ec_cache_ClassHierarchy_ClassId", "CREATE INDEX ix_ec_cache_ClassHierarchy_ClassId ON ec_cache_ClassHierarchy(ClassId)"},
		{"ix_ec_cache_ClassHierarchy_BaseClassId", "CREATE INDEX ix_ec_cache_ClassHierarchy_BaseClassId ON ec_cache_ClassHierarchy(BaseClassId)"},
	};
	/*
	SELECT PRINTF ('{"%s", "%s"},', [name], REPLACE(REPLACE(REPLACE(REPLACE ([sql], ', ', ','),'  ',' '),'[',''),']', ''))
	FROM   [sqlite_master]
	WHERE  ([tbl_name] LIKE 'dgn\_%' ESCAPE '\'
			OR [tbl_name] LIKE 'bis\_%' ESCAPE '\')
			AND [sql] IS NOT NULL
			AND [type] = 'trigger';
	*/
	const auto metaTriggers = std::map<std::string,std::string> {
		{"bis_Element_CurrentTimeStamp", "CREATE TRIGGER bis_Element_CurrentTimeStamp AFTER UPDATE ON bis_Element WHEN old.LastMod=new.LastMod AND old.LastMod!=julianday('now') BEGIN UPDATE bis_Element SET LastMod=julianday('now') WHERE Id=new.Id; END"},
		{"dgn_prjrange_del", "CREATE TRIGGER dgn_prjrange_del AFTER DELETE ON bis_GeometricElement3d BEGIN DELETE FROM dgn_SpatialIndex WHERE ElementId=old.ElementId;END"},
		{"dgn_rtree_upd", "CREATE TRIGGER dgn_rtree_upd AFTER UPDATE OF Origin_X,Origin_Y,Origin_Z,Yaw,Pitch,Roll,BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z ON bis_GeometricElement3d WHEN new.Origin_X IS NOT NULL AND 1 = new.InSpatialIndex BEGIN INSERT OR REPLACE INTO dgn_SpatialIndex(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5) FROM (SELECT DGN_placement_aabb(DGN_placement(DGN_point(NEW.Origin_X,NEW.Origin_Y,NEW.Origin_Z),DGN_angles(NEW.Yaw,NEW.Pitch,NEW.Roll),DGN_bbox(NEW.BBoxLow_X,NEW.BBoxLow_Y,NEW.BBoxLow_Z,NEW.BBoxHigh_X,NEW.BBoxHigh_Y,NEW.BBoxHigh_Z))) as bb);END"},
		{"dgn_rtree_upd1", "CREATE TRIGGER dgn_rtree_upd1 AFTER UPDATE OF Origin_X,Origin_Y,Origin_Z,Yaw,Pitch,Roll,BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z ON bis_GeometricElement3d WHEN OLD.Origin_X IS NOT NULL AND NEW.Origin_X IS NULL BEGIN DELETE FROM dgn_SpatialIndex WHERE ElementId=OLD.ElementId;END"},
		{"dgn_rtree_ins", "CREATE TRIGGER dgn_rtree_ins AFTER INSERT ON bis_GeometricElement3d WHEN new.Origin_X IS NOT NULL AND 1 = new.InSpatialIndex BEGIN INSERT INTO dgn_SpatialIndex(ElementId,minx,maxx,miny,maxy,minz,maxz) SELECT new.ElementId,DGN_bbox_value(bb,0),DGN_bbox_value(bb,3),DGN_bbox_value(bb,1),DGN_bbox_value(bb,4),DGN_bbox_value(bb,2),DGN_bbox_value(bb,5) FROM (SELECT DGN_placement_aabb(DGN_placement(DGN_point(NEW.Origin_X,NEW.Origin_Y,NEW.Origin_Z),DGN_angles(NEW.Yaw,NEW.Pitch,NEW.Roll),DGN_bbox(NEW.BBoxLow_X,NEW.BBoxLow_Y,NEW.BBoxLow_Z,NEW.BBoxHigh_X,NEW.BBoxHigh_Y,NEW.BBoxHigh_Z))) as bb);END"},
		{"dgn_fts_ai", "CREATE TRIGGER dgn_fts_ai AFTER INSERT ON dgn_fts_content BEGIN INSERT INTO dgn_fts_idx(rowid,Type,Id,Text) VALUES(new.rowid,new.Type,new.Id,new.Text); END"},
		{"dgn_fts_ad", "CREATE TRIGGER dgn_fts_ad AFTER DELETE ON dgn_fts_content BEGIN INSERT INTO dgn_fts_idx(dgn_fts_idx,rowid,Type,Id,Text) VALUES('delete',old.rowid,old.Type,old.Id,old.Text); END"},
		{"dgn_fts_au", "CREATE TRIGGER dgn_fts_au AFTER UPDATE ON dgn_fts_content BEGIN INSERT INTO dgn_fts_idx(dgn_fts_idx,rowid,Type,Id,Text) VALUES('delete',old.rowid,old.Type,old.Id,old.Text); INSERT INTO dgn_fts_idx(rowid,Type,Id,Text) VALUES(new.rowid,new.Type,new.Id,new.Text); END"},
	};
	return CheckEcProfile(metaTables, metaIndexes, metaTriggers, callback);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckEcProfile(std::map<std::string,std::string> const& metaTables, std::map<std::string,std::string> const& metaIndexes, std::map<std::string,std::string> const& metaTriggers, std::function<bool(std::string,std::string,std::string)> callback) {
	const auto kIssueNotFound = "missing";
	const auto kIssueDDLMismatch = "ddl mismatch";
	const auto kTypeTable = "table";
	const auto kTypeIndex = "index";
	const auto kTypeTrigger = "trigger";

	const auto tableSql = R"sql(
		SELECT replace([sql], ', ', ',')
		FROM   [main].[sqlite_master]
		WHERE  [tbl_name] LIKE 'ec\_%' ESCAPE '\'
				AND [sql] IS NOT NULL
				AND [type] = 'table'
				AND [name] = ?;

	)sql";

	Statement tableStmt;
	auto rc = tableStmt.Prepare(m_conn, tableSql);
	if (rc != BE_SQLITE_OK) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}

	for(auto & kv : metaTables) {
		LOG.infov("integrity_check(check_profile_and_indexes) analyzing [table: %s]", kv.first.c_str());
		tableStmt.Reset();
		tableStmt.ClearBindings();
		tableStmt.BindText(1, kv.first.c_str(), Statement::MakeCopy::No);
		rc = tableStmt.Step();
		if (rc != BE_SQLITE_ROW && rc != BE_SQLITE_DONE) {
			m_lastError = m_conn.GetLastError();
			return rc;
		}
		if (rc == BE_SQLITE_DONE) {
			//! missing table
			if (!callback(kTypeTable, kv.first, kIssueNotFound)) {
                return BE_SQLITE_OK;
			}
        } else {
			const std::string sql = tableStmt.GetValueText(0);
			if (sql != kv.second) {
				//! miss match declaration
                if(!callback(kTypeTable, kv.first, kIssueDDLMismatch)) {
                    return BE_SQLITE_OK;
				}
            }
		}
	}

	const auto indexSql = R"sql(
		SELECT replace([sql], ', ', ',')
		FROM   [main].[sqlite_master]
		WHERE  [tbl_name] LIKE 'ec\_%' ESCAPE '\'
				AND [sql] IS NOT NULL
				AND [type] = 'index'
				AND [name] = ?;

	)sql";

	Statement indexStmt;
	rc = indexStmt.Prepare(m_conn, indexSql);
	if (rc != BE_SQLITE_OK) {
		m_lastError = m_conn.GetLastError();
		return rc;
	}

	for(auto & kv : metaIndexes) {
		LOG.infov("integrity_check(check_profile_and_indexes) analyzing [index: %s]", kv.first.c_str());
		indexStmt.Reset();
		indexStmt.ClearBindings();
		indexStmt.BindText(1, kv.first.c_str(), Statement::MakeCopy::No);
		rc = indexStmt.Step();
		if (rc != BE_SQLITE_ROW && rc != BE_SQLITE_DONE) {
			m_lastError = m_conn.GetLastError();
			return rc;
		}
		if (rc == BE_SQLITE_DONE) {
			//! missing index
			if (!callback(kTypeIndex, kv.first, kIssueNotFound)) {
                return BE_SQLITE_OK;
			}
        } else {
			const std::string sql = indexStmt.GetValueText(0);
			if (sql != kv.second) {
				//! miss match declaration
				if(!callback(kTypeIndex, kv.first, kIssueDDLMismatch)) {
                    return BE_SQLITE_OK;
				}
            }
		}
	}
	if(m_conn.Schemas().GetSchema("BisCore") != nullptr) {
		const auto triggerSql = R"sql(
			SELECT REPLACE(REPLACE(REPLACE(REPLACE ([sql], ', ', ','),'  ',' '),'[',''),']', '')
			FROM   [main].[sqlite_master]
			WHERE  ([tbl_name] LIKE 'dgn\_%' ESCAPE '\'
					OR [tbl_name] LIKE 'bis\_%' ESCAPE '\')
					AND [sql] IS NOT NULL
					AND [type] = 'trigger'
					AND [name] = ?;
		)sql";

		Statement triggerStmt;
		rc = triggerStmt.Prepare(m_conn, triggerSql);
		if (rc != BE_SQLITE_OK) {
			m_lastError = m_conn.GetLastError();
			return rc;
		}

		for(auto & kv : metaTriggers) {
			LOG.infov("integrity_check(check_profile_and_indexes) analyzing [trigger: %s]", kv.first.c_str());
			triggerStmt.Reset();
			triggerStmt.ClearBindings();
			triggerStmt.BindText(1, kv.first.c_str(), Statement::MakeCopy::No);
			rc = triggerStmt.Step();
			if (rc != BE_SQLITE_ROW && rc != BE_SQLITE_DONE) {
				m_lastError = m_conn.GetLastError();
				return rc;
			}
			if (rc == BE_SQLITE_DONE) {
				//! missing index
				if (!callback(kTypeTrigger, kv.first, kIssueNotFound)) {
					return BE_SQLITE_OK;
				}
			} else {
				const std::string sql = triggerStmt.GetValueText(0);
				if (sql != kv.second) {
					//! miss match declaration
					if(!callback(kTypeTrigger, kv.first, kIssueDDLMismatch)) {
						return BE_SQLITE_OK;
					}
				}
			}
		}
	}
	return BE_SQLITE_OK;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckNavIds(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, Utf8CP)> callback) {
	std::map<ECN::ECClassId, std::vector<std::string>> navProps;
	auto rc = GetNavigationProperties(navProps);
	if (BE_SQLITE_OK != rc) {
		return rc;
	}

	for (auto & navProp : navProps) {
		const auto classId = navProp.first;
		const auto& props = navProp.second;
		const auto classCP = m_conn.Schemas().GetClass(classId);
		if (classCP == nullptr) {
			m_lastError = SqlPrintfString("failed to find class with id '%s'.", classId.ToHexStr().c_str());
			return BE_SQLITE_ERROR;
		}

		auto classMap = m_conn.Schemas().Main().GetClassMap(*classCP);
		if (classCP == nullptr) {
			m_lastError = SqlPrintfString("failed to find classmap for class '%s'.", classCP->GetFullName());
			return BE_SQLITE_ERROR;
		}

        for(auto& prop : props) {
			auto propertyCP = classCP->GetPropertyP(prop.c_str(), false);
			if (propertyCP == nullptr) {
				m_lastError = SqlPrintfString("failed to find property '%s' in class '%s'.", prop.c_str(), classCP->GetFullName());
				return BE_SQLITE_ERROR;
			}

			auto navPropCP = propertyCP->GetAsNavigationProperty();
            auto propMap = classMap->GetPropertyMaps().Find(prop.c_str());
			if (propMap == nullptr) {
				m_lastError = SqlPrintfString("failed to find propertymap for property '%s' in classmap '%s'.", prop.c_str(), classCP->GetFullName());
				return BE_SQLITE_ERROR;
			}

            ECClassCP otherClass = nullptr;
            if (navPropCP->GetDirection() == ECN::ECRelatedInstanceDirection::Backward) {
                otherClass = navPropCP->GetRelationshipClass()->GetSource().GetConstraintClasses().front();
            } else {
				otherClass = navPropCP->GetRelationshipClass()->GetTarget().GetConstraintClasses().front();
			}
            LOG.infov("integrity_check(check_nav_ids) analyzing [class: %s] [nav_prop: %s]", classCP->GetFullName(), prop.c_str());
            std::string query = SqlPrintfString("SELECT s.ECInstanceId, s.%s.Id FROM %s s LEFT JOIN %s t ON s.%s.Id=t.ECInstanceId WHERE t.ECInstanceId IS NULL AND s.%s.Id IS NOT NULL",
												navPropCP->GetName().c_str(),
												classCP->GetECSqlName().c_str(),
												otherClass->GetECSqlName().c_str(),
												navPropCP->GetName().c_str(),
												navPropCP->GetName().c_str()).GetUtf8CP();

			// this is special case for root imodel which has subject as ModeledElement
			if (classCP->GetECSqlName() == "[BisCore].[Model]" && otherClass->GetECSqlName() == "[BisCore].[ISubModeledElement]" && navPropCP->GetName() == "ModeledElement") {
                query.append(" AND s.ECInstanceId <> 1");
            }

			ECSqlStatement navStmt;
            if (navStmt.Prepare(m_conn, query.c_str()) != ECSqlStatus::Success) {
				m_lastError = "failed to prepared ecsql for nav prop integrity check";
				return BE_SQLITE_ERROR;
			}
			while((rc = navStmt.Step()) == BE_SQLITE_ROW) {
				if (!callback(
					navStmt.GetValueId<ECInstanceId>(0),
					classCP->GetFullName(),
					navPropCP->GetName().c_str(),
					navStmt.GetValueId<ECInstanceId>(1),
					otherClass->GetFullName())) {
					return BE_SQLITE_OK;
				}
			}
		}
	}
	return BE_SQLITE_OK;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckNavClassIds(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, ECN::ECClassId)> callback) {
	std::map<ECN::ECClassId, std::vector<std::string>> navProps;
	auto rc = GetNavigationProperties(navProps);
	if (BE_SQLITE_OK != rc) {
		return rc;
	}
	for (auto & navProp : navProps) {
		const auto classId = navProp.first;
		const auto& props = navProp.second;
		const auto classCP = m_conn.Schemas().GetClass(classId);
		if (classCP == nullptr) {
			m_lastError = SqlPrintfString("failed to find class with id '%s'.", classId.ToHexStr().c_str());
			return BE_SQLITE_ERROR;
		}

		auto classMap = m_conn.Schemas().Main().GetClassMap(*classCP);
		if (classCP == nullptr) {
			m_lastError = SqlPrintfString("failed to find classmap for class '%s'.", classCP->GetFullName());
			return BE_SQLITE_ERROR;
		}

        for(auto& prop : props) {
			auto propertyCP = classCP->GetPropertyP(prop.c_str(), false);
			if (propertyCP == nullptr) {
				m_lastError = SqlPrintfString("failed to find property '%s' in class '%s'.", prop.c_str(), classCP->GetFullName());
				return BE_SQLITE_ERROR;
			}

			auto navPropCP = propertyCP->GetAsNavigationProperty();
            auto propMap = classMap->GetPropertyMaps().Find(prop.c_str());
			if (propMap == nullptr) {
				m_lastError = SqlPrintfString("failed to find propertymap for property '%s' in classmap '%s'.", prop.c_str(), classCP->GetFullName());
				return BE_SQLITE_ERROR;
			}

			// skip any nav property for which we do not store RelECClassId in db
            if (propMap->GetAs<NavigationPropertyMap>().GetRelECClassIdPropertyMap().GetColumn().IsVirtual()) {
                continue;
            }
			LOG.infov("integrity_check(check_nav_class_ids) analyzing [class: %s] [nav_prop: %s]", classCP->GetFullName(), prop.c_str());
			std::string incorrectClassIdsForTableQuery = SqlPrintfString("SELECT DISTINCT %s.RelECClassId FROM %s WHERE %s.RelECClassId IS NOT NULL AND %s.RelECClassId NOT IN (SELECT SourceECInstanceId FROM ECDbMeta.ClassHasAllBaseClasses WHERE TargetECInstanceId = %s)",
												navPropCP->GetName().c_str(),
												classCP->GetECSqlName().c_str(),
												navPropCP->GetName().c_str(),
												navPropCP->GetName().c_str(),
												std::to_string(propMap->GetAs<NavigationPropertyMap>().GetRelECClassIdPropertyMap().GetDefaultClassId().GetValue()).c_str()).GetUtf8CP();

			ECSqlStatement incorrectClassIdsForTableStmt;
			if (incorrectClassIdsForTableStmt.Prepare(m_conn, incorrectClassIdsForTableQuery.c_str()) != ECSqlStatus::Success) {
				m_lastError = "failed to prepare ecsql for nav prop integrity check";
				return BE_SQLITE_ERROR;
			}
			std::string incorrectClassIds = "";
			bool isFirst = true;
			while (incorrectClassIdsForTableStmt.Step() == BE_SQLITE_ROW) {
				if (!isFirst) {
					incorrectClassIds += ", ";
				}
				isFirst = false;
				incorrectClassIds += std::to_string(incorrectClassIdsForTableStmt.GetValueId<ECClassId>(0).GetValue());
			}
			if (incorrectClassIds.empty()) {
				continue;
			}
			std::string rowsWithIncorrectClassIdQuery = SqlPrintfString("SELECT ECInstanceId, %s.Id, %s.RelECClassId FROM %s WHERE %s.RelECClassId IN (%s)",
												navPropCP->GetName().c_str(),
												navPropCP->GetName().c_str(),
												classCP->GetECSqlName().c_str(),
												navPropCP->GetName().c_str(),
												incorrectClassIds.c_str()).GetUtf8CP();

			ECSqlStatement rowsWithIncorrectClassIdStmt;
			if (rowsWithIncorrectClassIdStmt.Prepare(m_conn, rowsWithIncorrectClassIdQuery.c_str()) != ECSqlStatus::Success) {
				m_lastError = "failed to prepare ecsql for nav prop integrity check";
				return BE_SQLITE_ERROR;
			}
			while (rowsWithIncorrectClassIdStmt.Step() == BE_SQLITE_ROW) {
				if (!callback(
					rowsWithIncorrectClassIdStmt.GetValueId<ECInstanceId>(0),
					classCP->GetFullName(),
					prop.c_str(),
					rowsWithIncorrectClassIdStmt.GetValueId<ECInstanceId>(1),
					rowsWithIncorrectClassIdStmt.GetValueId<ECClassId>(2))) {
					return BE_SQLITE_OK;
				}
			}
		}
	}
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckLinkTableFkIds(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, Utf8CP)> callback) {
	std::vector<ECClassId> rootRels;
	auto rc = GetRootLinkTableRelationships(rootRels);
	if (BE_SQLITE_OK != rc) {
		return rc;
	}

	for (auto & relId : rootRels) {
		const auto classCP = m_conn.Schemas().GetClass(relId);
		if (classCP == nullptr) {
			m_lastError = SqlPrintfString("failed to find class with id '%s'.", relId.ToHexStr().c_str());
			return BE_SQLITE_ERROR;
		}

        auto relCP = classCP->GetRelationshipClassCP();
		if ("source ids") {
			auto sourceClassCP = relCP->GetSource().GetConstraintClasses().front();
			LOG.infov("integrity_check(check_link_table_source_and_target_ids) analyzing [relationship: %s] [prop: SourceECInstanceId]", classCP->GetFullName());
			std::string query = SqlPrintfString("SELECT R.ECInstanceId, R.SourceECInstanceId FROM %s R LEFT JOIN %s O ON O.ECInstanceId = R.SourceECInstanceId WHERE O.ECInstanceId IS NULL",
				relCP->GetECSqlName().c_str(),
				sourceClassCP->GetECSqlName().c_str()
			).GetUtf8CP();

			ECSqlStatement stmt;
			if (ECSqlStatus::Success != stmt.Prepare(m_conn, query.c_str())){
				m_lastError = "failed to prepared ecsql for nav prop integrity check";
				return BE_SQLITE_ERROR;
			}
			auto prop = stmt.GetColumnInfo(1).GetProperty();
			while((rc=stmt.Step()) == BE_SQLITE_ROW) {
				if (!callback(
					stmt.GetValueId<ECInstanceId>(0),
					relCP->GetFullName(),
					prop->GetName().c_str(),
					stmt.GetValueId<ECInstanceId>(1),
					sourceClassCP->GetFullName())) {
					return BE_SQLITE_OK;
				}
			}
		}

		if ("target ids") {
			auto targetClassCP = relCP->GetTarget().GetConstraintClasses().front();
			LOG.infov("integrity_check(check_link_table_source_and_target_ids) analyzing [relationship: %s] [prop: TargetECInstanceId]", classCP->GetFullName());
			std::string query = SqlPrintfString("SELECT R.ECInstanceId, R.TargetECInstanceId FROM %s R LEFT JOIN %s O ON O.ECInstanceId = R.TargetECInstanceId WHERE O.ECInstanceId IS NULL",
				relCP->GetECSqlName().c_str(),
				targetClassCP->GetECSqlName().c_str()
			).GetUtf8CP();

			ECSqlStatement stmt;
			if (ECSqlStatus::Success != stmt.Prepare(m_conn, query.c_str())){
				m_lastError = "failed to prepared ecsql for nav prop integrity check";
				return BE_SQLITE_ERROR;
			}
			auto prop = stmt.GetColumnInfo(1).GetProperty();
			while((rc=stmt.Step()) == BE_SQLITE_ROW) {
				if (!callback(
					stmt.GetValueId<ECInstanceId>(0),
					relCP->GetFullName(),
					prop->GetName().c_str(),
					stmt.GetValueId<ECInstanceId>(1),
					targetClassCP->GetFullName())) {
					return BE_SQLITE_OK;
				}
			}
		}
    }
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckLinkTableFkClassIds(std::function<bool(ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, ECN::ECClassId)> callback) {
	std::vector<ECClassId> rootRels;
	auto rc = GetRootLinkTableRelationships(rootRels);
	if (BE_SQLITE_OK != rc) {
		return rc;
	}

	for (auto & relId : rootRels) {
		const auto classCP = m_conn.Schemas().GetClass(relId);
		if (classCP == nullptr) {
			m_lastError = SqlPrintfString("failed to find class with id '%s'.", relId.ToHexStr().c_str());
			return BE_SQLITE_ERROR;
		}

		auto classMap = m_conn.Schemas().Main().GetClassMap(*classCP);
		if (classCP == nullptr) {
			m_lastError = SqlPrintfString("failed to find classmap for class '%s'.", classCP->GetFullName());
			return BE_SQLITE_ERROR;
		}

        auto& primaryTable = classMap->GetPrimaryTable();
        auto& relMap = classMap->GetAs<RelationshipClassLinkTableMap>();
        if ("source ids") {
            auto sourceECClassIdPropMap = relMap.GetSourceECClassIdPropMap();
			if(sourceECClassIdPropMap->IsMappedToSingleTable()) {
                auto s = sourceECClassIdPropMap->GetTables();

                (void)(s);
            }
			if (sourceECClassIdPropMap->IsMappedToSingleTable() && sourceECClassIdPropMap->FindDataPropertyMap(primaryTable) && sourceECClassIdPropMap->FindDataPropertyMap(primaryTable)->GetColumn().IsVirtual()) {
				LOG.infov("integrity_check(check_link_table_source_and_target_class_ids) analyzing [relationship: %s] [prop: SourceECClassId]", classCP->GetFullName());
				std::string query = SqlPrintfString("SELECT R.ECInstanceId, R.SourceECInstanceId, R.SourceECClassId FROM %s R LEFT JOIN meta.ECClassDef O ON O.ECInstanceId = R.SourceECClassId WHERE O.ECInstanceId IS NULL",
													relMap.GetClass().GetECSqlName().c_str()).GetUtf8CP();

				ECSqlStatement stmt;
				if (ECSqlStatus::Success != stmt.Prepare(m_conn, query.c_str())){
					m_lastError = "failed to prepared ecsql for nav prop integrity check";
					return BE_SQLITE_ERROR;
				}
				while((rc = stmt.Step()) == BE_SQLITE_ROW) {
					if (!callback(
						stmt.GetValueId<ECInstanceId>(0),
						relMap.GetRelationshipClass().GetECSqlName().c_str(),
						sourceECClassIdPropMap->GetProperty().GetName().c_str(),
						stmt.GetValueId<ECInstanceId>(1),
						stmt.GetValueId<ECClassId>(2))) {
						return BE_SQLITE_OK;
					}
				}
			}
		}

		if ("target ids") {
            auto targetECClassIdPropMap = relMap.GetTargetECClassIdPropMap();
			if (targetECClassIdPropMap->IsMappedToSingleTable() && targetECClassIdPropMap->FindDataPropertyMap(primaryTable) && targetECClassIdPropMap->FindDataPropertyMap(primaryTable)->GetColumn().IsVirtual()) {
				LOG.infov("integrity_check(check_link_table_source_and_target_class_ids) analyzing [relationship: %s] [prop: TargetECClassId]", classCP->GetFullName());
				std::string query = SqlPrintfString("SELECT R.ECInstanceId, R.TargetECInstanceId, R.TargetECClassId FROM %s R LEFT JOIN meta.ECClassDef O ON O.ECInstanceId = R.TargetECClassId WHERE O.ECInstanceId IS NULL",
													relMap.GetClass().GetECSqlName().c_str()).GetUtf8CP();

				ECSqlStatement stmt;
				if (ECSqlStatus::Success != stmt.Prepare(m_conn, query.c_str())){
					m_lastError = "failed to prepared ecsql for nav prop integrity check";
					return BE_SQLITE_ERROR;
				}
				while((rc = stmt.Step()) == BE_SQLITE_ROW) {
					if (!callback(
						stmt.GetValueId<ECInstanceId>(0),
						relMap.GetRelationshipClass().GetECSqlName().c_str(),
						targetECClassIdPropMap->GetProperty().GetName().c_str(),
						stmt.GetValueId<ECInstanceId>(1),
						stmt.GetValueId<ECClassId>(2))) {
						return BE_SQLITE_OK;
					}
				}
			}
		}
	}
	return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckClassIds(std::function<bool(Utf8CP, ECInstanceId, ECN::ECClassId, Utf8CP)> callback) {
	std::vector<ECClassId> classIds;
	auto rc = GetTablePerHierarchyClasses(classIds);
	if (BE_SQLITE_OK != rc) {
		return rc;
	}
	for (auto classId : classIds) {
		const auto classCP = m_conn.Schemas().GetClass(classId);
		if (classCP == nullptr) {
			m_lastError = SqlPrintfString("failed to find class with id '%s'.", classId.ToHexStr().c_str());
			return BE_SQLITE_ERROR;
		}
		LOG.infov("integrity_check(check_entity_and_rel_class_Ids) analyzing root table for [class: %s]", classCP->GetFullName());
		std::string query = SqlPrintfString("SELECT R.ECInstanceId, R.ECClassId FROM %s R LEFT JOIN meta.ECClassDef O ON O.ECInstanceId = R.ECClassId WHERE O.ECInstanceId IS NULL",
										classCP->GetECSqlName().c_str()).GetUtf8CP();
		ECSqlStatement stmt;
		if (ECSqlStatus::Success != stmt.Prepare(m_conn, query.c_str())){
			m_lastError = "failed to prepared ecsql for nav prop integrity check";
			return BE_SQLITE_ERROR;
		}
		while((rc = stmt.Step()) == BE_SQLITE_ROW) {
			if (!callback(
				classCP->GetFullName(),
				stmt.GetValueId<ECInstanceId>(0),
				stmt.GetValueId<ECClassId>(1), "primary")) {
				return BE_SQLITE_OK;
			}
		}
	}

	// JOINED table
	if ("check JOINED table ECClassId") {
		Statement stmt;
		rc = stmt.Prepare(m_conn, "SELECT [ExclusiveRootClassId] FROM [ec_table] WHERE [type] = 1;");
		if (BE_SQLITE_OK != rc) {
			m_lastError = m_conn.GetLastError();
			return rc;
		}
		while((rc = stmt.Step()) == BE_SQLITE_ROW) {
			auto classId = stmt.GetValueId<ECClassId>(0);
			const auto classCP = m_conn.Schemas().GetClass(classId);
			if (classCP == nullptr) {
				m_lastError = SqlPrintfString("failed to find class with id '%s'.", classId.ToHexStr().c_str());
				return BE_SQLITE_ERROR;
			}
			LOG.infov("integrity_check(check_entity_and_rel_class_Ids) analyzing joined table for [class: %s]", classCP->GetFullName());
			std::string query = SqlPrintfString("SELECT R.ECInstanceId, R.ECClassId FROM %s R LEFT JOIN meta.ECClassDef O ON O.ECInstanceId = R.ECClassId WHERE O.ECInstanceId IS NULL",
											classCP->GetECSqlName().c_str()).GetUtf8CP();
			ECSqlStatement ecSqlStmt;
			if (ECSqlStatus::Success != ecSqlStmt.Prepare(m_conn, query.c_str())){
				m_lastError = "failed to prepared ecsql for nav prop integrity check";
				return BE_SQLITE_ERROR;
			}
			while((rc = ecSqlStmt.Step()) == BE_SQLITE_ROW) {
				if (!callback(
					classCP->GetFullName(),
					ecSqlStmt.GetValueId<ECInstanceId>(0),
					ecSqlStmt.GetValueId<ECClassId>(1), "joined")) {
					return BE_SQLITE_OK;
				}
			}
		}
	}

	//OVERFLOW table 3
	if ("check OVERFLOW table ECClassId") {
		Statement overflowTableStmt;
		rc = overflowTableStmt.Prepare(m_conn, R"sql(
			SELECT
				[p].[ExclusiveRootClassId],
				[o].[Name]
			FROM   [ec_table] [o]
				JOIN [ec_table] [p] ON [p].[Id] = [o].[ParentTableId]
			WHERE  [o].[type] = 3;
		)sql");
		if (BE_SQLITE_OK != rc) {
			m_lastError = m_conn.GetLastError();
			return rc;
		}
		while((rc = overflowTableStmt.Step()) == BE_SQLITE_ROW) {
			auto classId = overflowTableStmt.GetValueId<ECClassId>(0);
			auto overflowTableName = overflowTableStmt.GetValueText(1);
			const auto classCP = m_conn.Schemas().GetClass(classId);

			if (classCP == nullptr) {
				m_lastError = SqlPrintfString("failed to find class with id '%s'.", classId.ToHexStr().c_str());
				return BE_SQLITE_ERROR;
			}
			LOG.infov("integrity_check(check_entity_and_rel_class_Ids) analyzing overflow table for [class: %s]", classCP->GetFullName());
			std::string query = SqlPrintfString("SELECT [T].[RowId], [T].[ECClassId] FROM [main].[%s] [T] LEFT JOIN [main].[ec_Class] [C] ON [C].[Id] = [T].[ECClassId] WHERE [C].[Id] IS NULL",
											overflowTableName).GetUtf8CP();
			Statement stmt;
			if (BE_SQLITE_OK != stmt.Prepare(m_conn, query.c_str())){
				m_lastError = "failed to prepared ecsql for nav prop integrity check";
				return BE_SQLITE_ERROR;
			}
			while((rc = stmt.Step()) == BE_SQLITE_ROW) {
				if (!callback(
					classCP->GetFullName(),
					stmt.GetValueId<ECInstanceId>(0),
					stmt.GetValueId<ECClassId>(1), "overflow")) {
					return BE_SQLITE_OK;
				}
			}
		}
	}
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckMissingChildRows(std::function<bool(Utf8CP, ECInstanceId, ECN::ECClassId, Utf8CP)> callback)
	{
	if ("Check missing child rows from BisCore:Element")
		{
		Statement getChildClassesStmt;
		auto rc = getChildClassesStmt.Prepare(m_conn, R"sql(
			SELECT 
			       [Tables], 
			       GROUP_CONCAT ([Id])
			FROM   (SELECT 
			               [CL].[Id] [Id], 
			               GROUP_CONCAT ([Tb].[Name]) [Tables]
			        FROM   (SELECT DISTINCT [ECClassId] [Id]
			                FROM   [bis_Element]) CL
			               JOIN [ec_cache_ClassHasTables] [CT] ON [CT].[ClassId] = [CL].[Id]
			               JOIN [ec_Table] [TB] ON [TB].[Id] = [CT].[TableId]
			        GROUP  BY [CL].[Id])
			GROUP  BY [Tables];
		)sql");
		if (BE_SQLITE_OK != rc)
			{
			m_lastError = m_conn.GetLastError();
			return rc;
			}

		Utf8String getMissingRowsQueryTemplate = R"sql(select a.Id, a.ECClassId, '%s' as MissingRowInTables from bis_Element a %s where a.ECClassId in (%s) and (%s))sql";
		Utf8String finalQuery;

		while(getChildClassesStmt.Step() == BE_SQLITE_ROW)
			{
			bvector<Utf8String> childClasses;
			BeStringUtilities::Split(getChildClassesStmt.GetValueText(0), ",", childClasses);
			Utf8String joins;
			Utf8String whereClause;
			int alias = 1;
			for (auto i = 1; i < childClasses.size(); ++i, ++alias)
				{
				const auto childClass = childClasses[i];
				joins += Utf8PrintfString("left join %s b%d on b%d.ElementId = a.Id ", childClass.c_str(), alias, alias);
				if (!Utf8String::IsNullOrEmpty(whereClause.c_str()))
					whereClause += " or ";
				whereClause += Utf8PrintfString("b%d.ElementId is null", alias);
				}
			if (!Utf8String::IsNullOrEmpty(finalQuery.c_str()))
				finalQuery += " union ";

			finalQuery += Utf8PrintfString(getMissingRowsQueryTemplate.c_str(), BeStringUtilities::Join(bvector<Utf8String>(childClasses.begin()+1, childClasses.end()), ",").c_str(), joins.c_str(), getChildClassesStmt.GetValueText(1), whereClause.c_str());
			}

		Statement getMissingRowsStmt;
		rc = getMissingRowsStmt.Prepare(m_conn, finalQuery.c_str());
		if (BE_SQLITE_OK != rc)
			{
			m_lastError = m_conn.GetLastError();
			return rc;
			}
		while(getMissingRowsStmt.Step() == BE_SQLITE_ROW)
			{
			if (!callback("BisCore:Element", getMissingRowsStmt.GetValueId<ECInstanceId>(0), getMissingRowsStmt.GetValueId<ECClassId>(1), getMissingRowsStmt.GetValueText(2)))
				return BE_SQLITE_OK;
			}
		}
		return BE_SQLITE_OK;
	}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckDataSchema(std::function<bool(std::string, std::string)> callback) {
    auto rc = CheckDataTableExists([&](std::string table) {
        return callback(table, "table");
    });
	if (rc != BE_SQLITE_OK) {
        return rc;
    }
	rc = CheckDataIndexExists([&](std::string index) {
        return callback(index, "index");
    });
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP IntegrityChecker::GetCheckName(Checks check) {
    static auto s_map = std::map<Checks, Utf8CP>{
		{Checks::CheckEcProfile, check_ec_profile},
		{Checks::CheckDataSchema, check_data_schema},
		{Checks::CheckDataColumns, check_data_columns},
		{Checks::CheckNavClassIds, check_nav_class_ids},
		{Checks::CheckNavIds, check_nav_ids},
		{Checks::CheckLinkTableFkClassIds, check_linktable_fk_class_ids},
		{Checks::CheckLinkTableFkIds, check_linktable_fk_ids},
		{Checks::CheckClassIds, check_class_ids},
		{Checks::CheckSchemaLoad, check_schema_load},
		{Checks::CheckMissingChildRows, check_missing_child_rows},
    };
    const auto it = s_map.find(check);
	if (it != s_map.end())  {
        return it->second;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IntegrityChecker::Checks IntegrityChecker::GetCheckId(Utf8CP checkName) {

    static auto s_map = std::map<Utf8CP, Checks, CompareIUtf8Ascii> {
		{check_ec_profile, Checks::CheckEcProfile},
		{check_data_schema, Checks::CheckDataSchema},
		{check_data_columns, Checks::CheckDataColumns},
		{check_nav_class_ids, Checks::CheckNavClassIds},
		{check_nav_ids, Checks::CheckNavIds},
		{check_linktable_fk_class_ids, Checks::CheckLinkTableFkClassIds},
		{check_linktable_fk_ids, Checks::CheckLinkTableFkIds},
		{check_class_ids, Checks::CheckClassIds},
		{check_schema_load, Checks::CheckSchemaLoad},
		{check_missing_child_rows, Checks::CheckMissingChildRows},
    };
    const auto it = s_map.find(checkName);
	if (it != s_map.end())  {
        return it->second;
    }
    return Checks::None;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::CheckSchemaLoad(std::function<bool(Utf8CP)> callback) {
    Statement stmt;
    auto rc = stmt.Prepare(m_conn, "SELECT [Name] FROM [ec_schema] ORDER BY [id]");
	if (rc != BE_SQLITE_OK) {
        m_lastError = m_conn.GetLastError();
        return rc;
    }
	while((rc = stmt.Step()) == BE_SQLITE_ROW) {
        Utf8String schemaName = stmt.GetValueText(0);
        auto schema = m_conn.Schemas().GetSchema(schemaName, true);
		if (schema == nullptr) {
			if (!callback(schemaName.c_str())){
                return BE_SQLITE_OK;
            }
		}
    }
	if (rc != BE_SQLITE_DONE) {
        return rc;
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult IntegrityChecker::QuickCheck(Checks checks, std::function<void(Utf8CP, bool, BeDuration)> callback) {
    DbResult rc;
    if (Enum::Contains<Checks>(checks, Checks::CheckDataColumns)) {
        StopWatch stopWatch(true);
        bool errorFound = false;
        rc = CheckDataColumns([&errorFound](std::string, std::string) {
			errorFound = true;
			return false;
        });
		if (rc != BE_SQLITE_OK) {
            return rc;
        }
        callback(GetCheckName(Checks::CheckDataColumns), !errorFound, stopWatch.GetCurrent());
    }
    if (Enum::Contains<Checks>(checks, Checks::CheckEcProfile)) {
		StopWatch stopWatch(true);
        bool errorFound = false;
        rc = CheckEcProfile([&errorFound](std::string, std::string, std::string) {
			errorFound = true;
			return false;
        });
		if (rc != BE_SQLITE_OK) {
            return rc;
        }
		callback(GetCheckName(Checks::CheckEcProfile), !errorFound, stopWatch.GetCurrent());
    }
    if (Enum::Contains<Checks>(checks, Checks::CheckNavClassIds)) {
		StopWatch stopWatch(true);
        bool errorFound = false;
        rc = CheckNavClassIds([&errorFound](ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, ECN::ECClassId) {
			errorFound = true;
			return false;
        });
		if (rc != BE_SQLITE_OK) {
            return rc;
        }
		callback(GetCheckName(Checks::CheckNavClassIds), !errorFound, stopWatch.GetCurrent());
    }
    if (Enum::Contains<Checks>(checks, Checks::CheckNavIds)) {
		StopWatch stopWatch(true);
        bool errorFound = false;
        rc = CheckNavIds([&errorFound](ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, Utf8CP) {
			errorFound = true;
			return false;
        });
		if (rc != BE_SQLITE_OK) {
            return rc;
        }
		callback(GetCheckName(Checks::CheckNavIds), !errorFound, stopWatch.GetCurrent());
    }
    if (Enum::Contains<Checks>(checks, Checks::CheckLinkTableFkClassIds)) {
		StopWatch stopWatch(true);
        bool errorFound = false;
        rc = CheckLinkTableFkClassIds([&errorFound](ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, ECN::ECClassId) {
			errorFound = true;
			return false;
        });
		if (rc != BE_SQLITE_OK) {
            return rc;
        }
		callback(GetCheckName(Checks::CheckLinkTableFkClassIds), !errorFound, stopWatch.GetCurrent());
    }
	if (Enum::Contains<Checks>(checks, Checks::CheckLinkTableFkIds)) {
		StopWatch stopWatch(true);
		bool errorFound = false;
		rc = CheckLinkTableFkIds([&errorFound](ECInstanceId, Utf8CP, Utf8CP, ECInstanceId, Utf8CP) {
			errorFound = true;
			return false;
		});
		if (rc != BE_SQLITE_OK) {
			return rc;
		}
		callback(GetCheckName(Checks::CheckLinkTableFkIds), !errorFound, stopWatch.GetCurrent());
	}
    if (Enum::Contains<Checks>(checks, Checks::CheckClassIds)) {
		StopWatch stopWatch(true);
        bool errorFound = false;
        rc = CheckClassIds([&errorFound](Utf8CP, ECInstanceId, ECN::ECClassId, Utf8CP) {
			errorFound = true;
			return false;
        });
		if (rc != BE_SQLITE_OK) {
            return rc;
        }
		callback(GetCheckName(Checks::CheckClassIds), !errorFound, stopWatch.GetCurrent());
    }
    if (Enum::Contains<Checks>(checks, Checks::CheckDataSchema)) {
		StopWatch stopWatch(true);
        bool errorFound = false;
        rc = CheckDataSchema([&errorFound](std::string, std::string) {
			errorFound = true;
			return false;
        });
		if (rc != BE_SQLITE_OK) {
            return rc;
        }
		callback(GetCheckName(Checks::CheckDataSchema), !errorFound, stopWatch.GetCurrent());
    }
    if (Enum::Contains<Checks>(checks, Checks::CheckSchemaLoad)) {
		StopWatch stopWatch(true);
        bool errorFound = false;
        rc = CheckSchemaLoad([&errorFound](Utf8CP) {
			errorFound = true;
			return false;
        });
		if (rc != BE_SQLITE_OK) {
            return rc;
        }
		callback(GetCheckName(Checks::CheckSchemaLoad), !errorFound, stopWatch.GetCurrent());
    }
	if (Enum::Contains<Checks>(checks, Checks::CheckMissingChildRows))
		{
		StopWatch stopWatch(true);
        bool errorFound = false;
        rc = CheckMissingChildRows([&errorFound](Utf8CP, ECInstanceId, ECN::ECClassId, Utf8CP)
			{
			errorFound = true;
			return false;
        	});
		if (rc != BE_SQLITE_OK)
			{
            return rc;
        	}
		callback(GetCheckName(Checks::CheckMissingChildRows), !errorFound, stopWatch.GetCurrent());
    	}
    return BE_SQLITE_OK;
}

END_BENTLEY_SQLITE_EC_NAMESPACE