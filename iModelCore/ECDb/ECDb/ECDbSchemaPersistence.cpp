/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaPersistence.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECSchemaInfo (BeSQLite::Db& db, DbECSchemaInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_Schema (ECSchemaId, Name, DisplayLabel, Description, NamespacePrefix, VersionMajor, VersionMinor, SchemaType) VALUES(?, ?, ?, ?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECSchemaInfo::COL_ECSchemaId     ) stmt->BindInt64 (1,  info.m_ecSchemaId);
    if (info.ColsInsert & DbECSchemaInfo::COL_Name           ) stmt->BindText (2,  info.m_name, Statement::MakeCopy::No);        
    if (info.ColsInsert & DbECSchemaInfo::COL_DisplayLabel   ) stmt->BindText (3,  info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECSchemaInfo::COL_Description    ) stmt->BindText (4,  info.m_description, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECSchemaInfo::COL_NamespacePrefix) stmt->BindText (5,  info.m_namespacePrefix, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECSchemaInfo::COL_VersionMajor   ) stmt->BindInt (6,  info.m_versionMajor);
    if (info.ColsInsert & DbECSchemaInfo::COL_VersionMinor   ) stmt->BindInt (7,  info.m_versionMinor);
    if (info.ColsInsert & DbECSchemaInfo::COL_SchemaType     ) stmt->BindInt (10, info.m_schemaType);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECSchemaInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECSchemaInfoCR spec)
    {
    SqlSelect  sql;
    Utf8String select;
    sql.AddFrom("ec_Schema");
    //prepare select list
    if (spec.ColsSelect & DbECSchemaInfo::COL_ECSchemaId     ) sql.AddSelect  ("ECSchemaId");
    if (spec.ColsSelect & DbECSchemaInfo::COL_Name           ) sql.AddSelect  ("Name");
    if (spec.ColsSelect & DbECSchemaInfo::COL_DisplayLabel   ) sql.AddSelect  ("DisplayLabel");
    if (spec.ColsSelect & DbECSchemaInfo::COL_Description    ) sql.AddSelect  ("Description");
    if (spec.ColsSelect & DbECSchemaInfo::COL_NamespacePrefix) sql.AddSelect  ("NamespacePrefix");
    if (spec.ColsSelect & DbECSchemaInfo::COL_VersionMajor   ) sql.AddSelect  ("VersionMajor");
    if (spec.ColsSelect & DbECSchemaInfo::COL_VersionMinor   ) sql.AddSelect  ("VersionMinor");
    if (spec.ColsSelect & DbECSchemaInfo::COL_SchemaType     ) sql.AddSelect  ("SchemaType");

    //prepare where
    if (spec.ColsWhere & DbECSchemaInfo::COL_ECSchemaId     ) sql.AddWhere ("ECSchemaId"     , "=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_Name           ) sql.AddWhere ("Name"           , "=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_NamespacePrefix) sql.AddWhere ("NamespacePrefix", "=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_VersionMajor   ) sql.AddWhere ("VersionMajor"   , "=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_VersionMinor   ) sql.AddWhere ("VersionMinor"   , "=?");
    if (spec.ColsWhere & DbECSchemaInfo::COL_SchemaType     ) sql.AddWhere ("SchemaType  "   , "=?");


    sql.GetSql(select);
    auto stat = db.GetCachedStatement (stmt, select.c_str());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (spec.ColsWhere & DbECSchemaInfo::COL_ECSchemaId     ) stmt->BindInt64 (nCol++, spec.m_ecSchemaId);  
    if (spec.ColsWhere & DbECSchemaInfo::COL_Name           ) stmt->BindText (nCol++, spec.m_name, Statement::MakeCopy::No);        
    if (spec.ColsWhere & DbECSchemaInfo::COL_NamespacePrefix) stmt->BindText (nCol++, spec.m_namespacePrefix, Statement::MakeCopy::No);    
    if (spec.ColsWhere & DbECSchemaInfo::COL_VersionMajor   ) stmt->BindInt (nCol++, spec.m_versionMajor);
    if (spec.ColsWhere & DbECSchemaInfo::COL_VersionMinor   ) stmt->BindInt (nCol++, spec.m_versionMinor);
    if (spec.ColsWhere & DbECSchemaInfo::COL_SchemaType     ) stmt->BindInt (nCol++, spec.m_schemaType);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECSchemaInfoR info,  BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECSchemaInfo::COL_ECSchemaId  )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_ECSchemaId;      nCol++; } else info.m_ecSchemaId   = stmt.GetValueInt64(nCol++);
        if (info.ColsSelect & DbECSchemaInfo::COL_Name        )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_Name;            nCol++; } else info.m_name         = stmt.GetValueText (nCol++);        
        if (info.ColsSelect & DbECSchemaInfo::COL_DisplayLabel)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_DisplayLabel;    nCol++; } else info.m_displayLabel = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECSchemaInfo::COL_Description )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_Description;     nCol++; } else info.m_description  = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECSchemaInfo::COL_NamespacePrefix    )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_NamespacePrefix; nCol++; } else info.m_namespacePrefix = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECSchemaInfo::COL_VersionMajor)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_VersionMajor;    nCol++; } else info.m_versionMajor = stmt.GetValueInt  (nCol++);
        if (info.ColsSelect & DbECSchemaInfo::COL_VersionMinor)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_VersionMinor;    nCol++; } else info.m_versionMinor = stmt.GetValueInt  (nCol++);
        if (info.ColsSelect & DbECSchemaInfo::COL_SchemaType  )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECSchemaInfo::COL_SchemaType;       nCol++; } else info.m_schemaType =  (PersistedSchemaType)stmt.GetValueInt  (nCol++) ;
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECClassInfo (BeSQLite::Db& db, DbECClassInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_Class (ECClassId, ECSchemaId, Name, DisplayLabel, Description, IsDomainClass, IsStruct, IsCustomAttribute, RelationStrength, RelationStrengthDirection, IsRelationship) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECClassInfo::COL_ECClassId                ) stmt->BindInt64 (1, info.m_ecClassId);
    if (info.ColsInsert & DbECClassInfo::COL_ECSchemaId               ) stmt->BindInt64 (2, info.m_ecSchemaId);
    if (info.ColsInsert & DbECClassInfo::COL_Name                     ) stmt->BindText (3, info.m_name, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECClassInfo::COL_DisplayLabel             ) stmt->BindText (4, info.m_displayLabel, Statement::MakeCopy::No); 
    if (info.ColsInsert & DbECClassInfo::COL_Description              ) stmt->BindText (5, info.m_description, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECClassInfo::COL_IsDomainClass            ) stmt->BindInt (6, info.m_isDomainClass? 1 : 0);
    if (info.ColsInsert & DbECClassInfo::COL_IsStruct                 ) stmt->BindInt (7, info.m_isStruct? 1 : 0);
    if (info.ColsInsert & DbECClassInfo::COL_IsCustomAttribute        ) stmt->BindInt (8, info.m_isCustomAttribute? 1 : 0);
    if (info.ColsInsert & DbECClassInfo::COL_RelationStrength         ) stmt->BindInt (9, info.m_relationStrength);
    if (info.ColsInsert & DbECClassInfo::COL_RelationStrengthDirection) stmt->BindInt (10, ToInt (info.m_relationStrengthDirection));
    if (info.ColsInsert & DbECClassInfo::COL_IsRelationship           ) stmt->BindInt (11, info.m_isRelationship? 1 : 0 );

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECClassInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECClassInfoCR info)
    {
    SqlSelect  sql;
    Utf8String select;
    sql.AddFrom("ec_Class");
    //prepare select list
    if (info.ColsSelect & DbECClassInfo::COL_ECClassId                ) sql.AddSelect  ("ECClassId");
    if (info.ColsSelect & DbECClassInfo::COL_ECSchemaId               ) sql.AddSelect  ("ECSchemaId");
    if (info.ColsSelect & DbECClassInfo::COL_Name                     ) sql.AddSelect  ("Name");
    if (info.ColsSelect & DbECClassInfo::COL_DisplayLabel             ) sql.AddSelect  ("DisplayLabel");
    if (info.ColsSelect & DbECClassInfo::COL_Description              ) sql.AddSelect  ("Description");
    if (info.ColsSelect & DbECClassInfo::COL_IsDomainClass            ) sql.AddSelect  ("IsDomainClass");
    if (info.ColsSelect & DbECClassInfo::COL_IsStruct                 ) sql.AddSelect  ("IsStruct");
    if (info.ColsSelect & DbECClassInfo::COL_IsCustomAttribute        ) sql.AddSelect  ("IsCustomAttribute");
    if (info.ColsSelect & DbECClassInfo::COL_RelationStrength         ) sql.AddSelect  ("RelationStrength");
    if (info.ColsSelect & DbECClassInfo::COL_RelationStrengthDirection) sql.AddSelect  ("RelationStrengthDirection");
    if (info.ColsSelect & DbECClassInfo::COL_IsRelationship           ) sql.AddSelect  ("IsRelationship");

    //prepare where
    if (info.ColsWhere & DbECClassInfo::COL_ECClassId                ) sql.AddWhere ("ECClassId"        , "=?");
    if (info.ColsWhere & DbECClassInfo::COL_ECSchemaId               ) sql.AddWhere ("ECSchemaId"       , "=?");
    if (info.ColsWhere & DbECClassInfo::COL_Name                     ) sql.AddWhere ("Name"             , "=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsDomainClass            ) sql.AddWhere ("IsDomainClass"    , "=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsStruct                 ) sql.AddWhere ("IsStruct"         , "=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsCustomAttribute        ) sql.AddWhere ("IsCustomAttribute", "=?");

    sql.GetSql(select);
    auto stat = db.GetCachedStatement (stmt, select.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECClassInfo::COL_ECClassId                ) stmt->BindInt64      (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECClassInfo::COL_ECSchemaId               ) stmt->BindInt64      (nCol++, info.m_ecSchemaId);
    if (info.ColsWhere & DbECClassInfo::COL_Name                     ) stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECClassInfo::COL_Description              ) stmt->BindText (nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECClassInfo::COL_IsDomainClass            ) stmt->BindInt        (nCol++, info.m_isDomainClass? 1 : 0);
    if (info.ColsWhere & DbECClassInfo::COL_IsStruct                 ) stmt->BindInt        (nCol++, info.m_isStruct? 1 : 0);
    if (info.ColsWhere & DbECClassInfo::COL_IsCustomAttribute        ) stmt->BindInt        (nCol++, info.m_isCustomAttribute? 1 : 0);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECClassInfoR info,  BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECClassInfo::COL_ECClassId                )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_ECClassId;                 nCol++; } else info.m_ecClassId                 = stmt.GetValueInt64(nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_ECSchemaId               )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_ECSchemaId;                nCol++; } else info.m_ecSchemaId                = stmt.GetValueInt64(nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_Name                     )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_Name;                      nCol++; } else info.m_name                      = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_DisplayLabel             )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_DisplayLabel;              nCol++; } else info.m_displayLabel              = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_Description              )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_Description;               nCol++; } else info.m_description               = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_IsDomainClass            )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_IsDomainClass;             nCol++; } else info.m_isDomainClass             = stmt.GetValueInt  (nCol++) == 1;
        if (info.ColsSelect & DbECClassInfo::COL_IsStruct                 )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_IsStruct;                  nCol++; } else info.m_isStruct                  = stmt.GetValueInt  (nCol++) == 1;
        if (info.ColsSelect & DbECClassInfo::COL_IsCustomAttribute        )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_IsCustomAttribute;         nCol++; } else info.m_isCustomAttribute         = stmt.GetValueInt  (nCol++) == 1;
        if (info.ColsSelect & DbECClassInfo::COL_RelationStrength         )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_RelationStrength;          nCol++; } else info.m_relationStrength          = (StrengthType)stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECClassInfo::COL_RelationStrengthDirection)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_RelationStrengthDirection; nCol++; }  else info.m_relationStrengthDirection= ToECRelatedInstanceDirection (stmt.GetValueInt (nCol++));
        if (info.ColsSelect & DbECClassInfo::COL_IsRelationship           )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassInfo::COL_IsRelationship;            nCol++; } else info.m_isRelationship            = stmt.GetValueInt  (nCol++) == 1;
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateECSchemaInfo (BeSQLite::Db& db, DbECSchemaInfoCR info)
    {
    SqlUpdate update;
    update.SetTable("ec_Schema");

    if (info.ColsWhere & DbECSchemaInfo::COL_ECSchemaId                ) update.AddWhere("ECSchemaId","=?");
    if (info.ColsWhere & DbECSchemaInfo::COL_Name                      ) update.AddWhere("Name","=?");
    if (info.ColsWhere & DbECSchemaInfo::COL_NamespacePrefix           ) update.AddWhere("NamespacePrefix","=?");
    if (info.ColsWhere & DbECSchemaInfo::COL_VersionMajor              ) update.AddWhere("VersionMajor","=?");
    if (info.ColsWhere & DbECSchemaInfo::COL_VersionMinor              ) update.AddWhere("VersionMinor","=?");
    if (info.ColsWhere & DbECSchemaInfo::COL_SchemaType                ) update.AddWhere("SchemaType","=?");

    if (info.ColsUpdate & DbECSchemaInfo::COL_ECSchemaId               ) update.AddUpdateColumn("ECSchemaId","?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_Name                     ) update.AddUpdateColumn("Name","?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_DisplayLabel             ) update.AddUpdateColumn("DisplayLabel","?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_Description              ) update.AddUpdateColumn("Description","?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_NamespacePrefix          ) update.AddUpdateColumn("NamespacePrefix","?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_VersionMajor             ) update.AddUpdateColumn("VersionMajor","?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_VersionMinor             ) update.AddUpdateColumn("VersionMinor","?");
    if (info.ColsUpdate & DbECSchemaInfo::COL_SchemaType               ) update.AddUpdateColumn("SchemaType","?");

    Utf8String sql;
    update.GetSql(sql);
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;

    if (info.ColsUpdate & DbECSchemaInfo::COL_ECSchemaId)
        if (info.ColsNull & DbECSchemaInfo::COL_ECSchemaId) nCol++; else stmt->BindInt64 (nCol++, info.m_ecSchemaId);
    if (info.ColsUpdate & DbECSchemaInfo::COL_Name)
        if (info.ColsNull & DbECSchemaInfo::COL_Name) nCol++; else stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);  
    if (info.ColsUpdate & DbECSchemaInfo::COL_DisplayLabel)
        if (info.ColsNull & DbECSchemaInfo::COL_DisplayLabel) nCol++; else stmt->BindText (nCol++, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECSchemaInfo::COL_Description)
        if (info.ColsNull & DbECSchemaInfo::COL_Description) nCol++; else stmt->BindText (nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECSchemaInfo::COL_NamespacePrefix)
        if (info.ColsNull & DbECSchemaInfo::COL_NamespacePrefix) nCol++; else stmt->BindText (nCol++, info.m_namespacePrefix, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECSchemaInfo::COL_VersionMajor)
        if (info.ColsNull & DbECSchemaInfo::COL_VersionMajor) nCol++; else stmt->BindInt (nCol++, info.m_versionMajor);
    if (info.ColsUpdate & DbECSchemaInfo::COL_VersionMinor)
        if (info.ColsNull & DbECSchemaInfo::COL_VersionMinor) nCol++; else stmt->BindInt (nCol++, info.m_versionMinor);
    if (info.ColsUpdate & DbECSchemaInfo::COL_SchemaType)
        if (info.ColsNull & DbECSchemaInfo::COL_SchemaType) nCol++; else stmt->BindInt (nCol++, info.m_schemaType);
 
    if (info.ColsWhere & DbECSchemaInfo::COL_ECSchemaId) stmt->BindInt64 (nCol++, info.m_ecSchemaId);
    if (info.ColsWhere & DbECSchemaInfo::COL_Name) stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);        
    if (info.ColsWhere & DbECSchemaInfo::COL_NamespacePrefix) stmt->BindText (nCol++, info.m_namespacePrefix, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECSchemaInfo::COL_VersionMajor) stmt->BindInt (nCol++, info.m_versionMajor);
    if (info.ColsWhere & DbECSchemaInfo::COL_VersionMinor) stmt->BindInt (nCol++, info.m_versionMinor);
    if (info.ColsWhere & DbECSchemaInfo::COL_SchemaType) stmt->BindInt (nCol++, info.m_schemaType);
    
    stat = stmt->Step();
    BeAssert (stat != BE_SQLITE_DONE || db.GetModifiedRowCount () > 0);
    return stat;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateECClassInfo (BeSQLite::Db& db, DbECClassInfoCR info)
    {
    SqlUpdate update;
    update.SetTable("ec_Class");

    if (info.ColsWhere & DbECClassInfo::COL_ECClassId                ) update.AddWhere("ECClassId","=?");
    if (info.ColsWhere & DbECClassInfo::COL_ECSchemaId               ) update.AddWhere("ECSchemaId","=?");
    if (info.ColsWhere & DbECClassInfo::COL_Name                     ) update.AddWhere("Name","=?");
    if (info.ColsWhere & DbECClassInfo::COL_DisplayLabel             ) update.AddWhere("DisplayLabel","=?");
    if (info.ColsWhere & DbECClassInfo::COL_Description              ) update.AddWhere("Description","=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsDomainClass            ) update.AddWhere("IsDomainClass","=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsStruct                 ) update.AddWhere("IsStruct","=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsCustomAttribute        ) update.AddWhere("IsCustomAttribute","=?");
    if (info.ColsWhere & DbECClassInfo::COL_RelationStrength         ) update.AddWhere("RelationStrength","=?");
    if (info.ColsWhere & DbECClassInfo::COL_RelationStrengthDirection) update.AddWhere("RelationStrengthDirection","=?");
    if (info.ColsWhere & DbECClassInfo::COL_IsRelationship           ) update.AddWhere("IsRelationship","=?");

    if (info.ColsUpdate & DbECClassInfo::COL_ECClassId                ) update.AddUpdateColumn("ECClassId","?");
    if (info.ColsUpdate & DbECClassInfo::COL_ECSchemaId               ) update.AddUpdateColumn("ECSchemaId","?");
    if (info.ColsUpdate & DbECClassInfo::COL_Name                     ) update.AddUpdateColumn("Name","?");
    if (info.ColsUpdate & DbECClassInfo::COL_DisplayLabel             ) update.AddUpdateColumn("DisplayLabel","?");
    if (info.ColsUpdate & DbECClassInfo::COL_Description              ) update.AddUpdateColumn("Description","?");
    if (info.ColsUpdate & DbECClassInfo::COL_IsDomainClass            ) update.AddUpdateColumn("IsDomainClass","?");
    if (info.ColsUpdate & DbECClassInfo::COL_IsStruct                 ) update.AddUpdateColumn("IsStruct","?");
    if (info.ColsUpdate & DbECClassInfo::COL_IsCustomAttribute        ) update.AddUpdateColumn("IsCustomAttribute","?");
    if (info.ColsUpdate & DbECClassInfo::COL_RelationStrength         ) update.AddUpdateColumn("RelationStrength","?");
    if (info.ColsUpdate & DbECClassInfo::COL_RelationStrengthDirection) update.AddUpdateColumn("RelationStrengthDirection","?");
    if (info.ColsUpdate & DbECClassInfo::COL_IsRelationship           ) update.AddUpdateColumn("IsRelationship","?");

    Utf8String sql;
    update.GetSql(sql);

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;

    if (info.ColsUpdate & DbECClassInfo::COL_ECClassId                )
        if (info.ColsNull & DbECClassInfo::COL_ECClassId) nCol++; else stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsUpdate & DbECClassInfo::COL_ECSchemaId               )
        if (info.ColsNull & DbECClassInfo::COL_ECSchemaId) nCol++; else stmt->BindInt64(nCol++, info.m_ecSchemaId);
    if (info.ColsUpdate & DbECClassInfo::COL_Name                     )
        if (info.ColsNull & DbECClassInfo::COL_Name) nCol++; else stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECClassInfo::COL_DisplayLabel             )
        if (info.ColsNull & DbECClassInfo::COL_DisplayLabel) nCol++; else stmt->BindText (nCol++, info.m_displayLabel, Statement::MakeCopy::No); 
    if (info.ColsUpdate & DbECClassInfo::COL_Description              )
        if (info.ColsNull & DbECClassInfo::COL_Description) nCol++; else stmt->BindText (nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECClassInfo::COL_IsDomainClass            )
        if (info.ColsNull & DbECClassInfo::COL_IsDomainClass) nCol++; else stmt->BindInt (nCol++, info.m_isDomainClass? 1 : 0);
    if (info.ColsUpdate & DbECClassInfo::COL_IsStruct                 )
        if (info.ColsNull & DbECClassInfo::COL_IsStruct) nCol++; else stmt->BindInt (nCol++, info.m_isStruct? 1 : 0);
    if (info.ColsUpdate & DbECClassInfo::COL_IsCustomAttribute        )
        if (info.ColsNull & DbECClassInfo::COL_IsCustomAttribute) nCol++; else stmt->BindInt (nCol++, info.m_isCustomAttribute? 1 : 0);
    if (info.ColsUpdate & DbECClassInfo::COL_RelationStrength         )
        if (info.ColsNull & DbECClassInfo::COL_RelationStrength) nCol++; else stmt->BindInt (nCol++, info.m_relationStrength);       
    if (info.ColsUpdate & DbECClassInfo::COL_RelationStrengthDirection)
        if (info.ColsNull & DbECClassInfo::COL_RelationStrengthDirection) nCol++; else stmt->BindInt (nCol++, ToInt (info.m_relationStrengthDirection));     
    if (info.ColsUpdate & DbECClassInfo::COL_IsRelationship           )
        if (info.ColsNull & DbECClassInfo::COL_IsRelationship) nCol++; else stmt->BindInt (nCol++, info.m_isRelationship? 1 : 0 );

    if (info.ColsWhere & DbECClassInfo::COL_ECClassId                ) stmt->BindInt64      (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECClassInfo::COL_ECSchemaId               ) stmt->BindInt64      (nCol++, info.m_ecSchemaId);
    if (info.ColsWhere & DbECClassInfo::COL_Name                     ) stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECClassInfo::COL_DisplayLabel             ) stmt->BindText (nCol++, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECClassInfo::COL_Description              ) stmt->BindText (nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECClassInfo::COL_IsDomainClass            ) stmt->BindInt        (nCol++, info.m_isDomainClass? 1 : 0);
    if (info.ColsWhere & DbECClassInfo::COL_IsStruct                 ) stmt->BindInt        (nCol++, info.m_isStruct? 1 : 0);
    if (info.ColsWhere & DbECClassInfo::COL_IsCustomAttribute        ) stmt->BindInt        (nCol++, info.m_isCustomAttribute? 1 : 0);
    if (info.ColsWhere & DbECClassInfo::COL_RelationStrength         ) stmt->BindInt        (nCol++, info.m_relationStrength);       
    if (info.ColsWhere & DbECClassInfo::COL_RelationStrengthDirection) stmt->BindInt        (nCol++, ToInt (info.m_relationStrengthDirection));     
    if (info.ColsWhere & DbECClassInfo::COL_IsRelationship           ) stmt->BindInt        (nCol++, info.m_isRelationship? 1 : 0 );

    stat = stmt->Step();
    BeAssert (stat != BE_SQLITE_DONE || db.GetModifiedRowCount () > 0);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECClassMapInfo (BeSQLite::Db& db, DbECClassMapInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_ClassMap (ECClassId, MapParentECClassId, MapStrategy, MapToDbTable, PrimaryKeyColumnName) VALUES(?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECClassMapInfo::COL_ECClassId) stmt->BindInt64 (1, info.m_ecClassId);
    if (info.ColsInsert & DbECClassMapInfo::COL_MapParentECClassId) stmt->BindInt64 (2, info.m_mapParentECClassId);
    if (info.ColsInsert & DbECClassMapInfo::COL_MapStrategy) stmt->BindInt (3, ToInt (info.m_mapStrategy));
    if (info.ColsInsert & DbECClassMapInfo::COL_MapToDbTable) stmt->BindText (4, info.m_mapToDbTable, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECClassMapInfo::COL_ECInstanceIdColumn) stmt->BindText (5, info.m_primaryKeyColumnName, Statement::MakeCopy::No);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECClassMapInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECClassMapInfoCR info)
    {
    SqlSelect  sql;
    Utf8String select;
    sql.AddFrom("ec_ClassMap");
    //prepare select list
    if (info.ColsSelect & DbECClassMapInfo::COL_ECClassId) sql.AddSelect ("ECClassId");
    if (info.ColsSelect & DbECClassMapInfo::COL_MapParentECClassId) sql.AddSelect ("MapParentECClassId");
    if (info.ColsSelect & DbECClassMapInfo::COL_MapStrategy) sql.AddSelect ("MapStrategy");
    if (info.ColsSelect & DbECClassMapInfo::COL_MapToDbTable) sql.AddSelect ("MapToDbTable");
    if (info.ColsSelect & DbECClassMapInfo::COL_ECInstanceIdColumn) sql.AddSelect ("PrimaryKeyColumnName");

    //prepare where
    if (info.ColsWhere & DbECClassMapInfo::COL_ECClassId) sql.AddWhere ("ECClassId", "=?");

    sql.GetSql(select);

    auto stat = db.GetCachedStatement (stmt, select.c_str());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECClassMapInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::SetECPropertyMapColumnName (ECPropertyId propertyId, Utf8CP mapColumnName, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_PropertyMap (ECPropertyId, MapColumnName) VALUES(?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, propertyId);
    stmt->BindText (2, mapColumnName, Statement::MakeCopy::No);
    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetECPropertyMapColumnName (Utf8StringR mapColumnName, ECPropertyId propertyId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT MapColumnName FROM ec_PropertyMap WHERE ECPropertyId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, propertyId);
    stat = stmt->Step();
    if (stat != BE_SQLITE_ROW)
        return stat;

    mapColumnName = stmt->GetValueText(0);
    return stat;
    }    
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECClassMapInfoR info,  BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECClassMapInfo::COL_ECClassId                )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassMapInfo::COL_ECClassId;            nCol++; } else info.m_ecClassId           = stmt.GetValueInt64(nCol++);
        if (info.ColsSelect & DbECClassMapInfo::COL_MapParentECClassId          )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassMapInfo::COL_MapParentECClassId;   nCol++; } else info.m_mapParentECClassId = (ECClassId)stmt.GetValueInt64(nCol++);
        if (info.ColsSelect & DbECClassMapInfo::COL_MapStrategy              )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassMapInfo::COL_MapStrategy;          nCol++; } else info.m_mapStrategy        = ToMapStrategy (stmt.GetValueInt (nCol++));
        if (info.ColsSelect & DbECClassMapInfo::COL_MapToDbTable             )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassMapInfo::COL_MapToDbTable;         nCol++; } else info.m_mapToDbTable       = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECClassMapInfo::COL_ECInstanceIdColumn      )
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECClassMapInfo::COL_ECInstanceIdColumn; nCol++; } else info.m_primaryKeyColumnName= stmt.GetValueText  (nCol++);
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateECClassMapInfo (BeSQLite::Db& db, DbECClassMapInfoCR info)
    {
    SqlUpdate update;
    update.SetTable("ec_ClassMap");

    if (info.ColsWhere & DbECClassMapInfo::COL_ECClassId                ) update.AddWhere("ECClassId","=?");

    if (info.ColsUpdate & DbECClassMapInfo::COL_ECClassId                ) update.AddUpdateColumn("ECClassId","?");
    if (info.ColsUpdate & DbECClassMapInfo::COL_MapParentECClassId       ) update.AddUpdateColumn("MapParentECClassId","?");
    if (info.ColsUpdate & DbECClassMapInfo::COL_MapStrategy              ) update.AddUpdateColumn("MapStrategy","?");
    if (info.ColsUpdate & DbECClassMapInfo::COL_MapToDbTable             ) update.AddUpdateColumn("MapToDbTable","?");
    if (info.ColsUpdate & DbECClassMapInfo::COL_ECInstanceIdColumn               ) update.AddUpdateColumn("PrimaryKeyColumnName","?");

    Utf8String sql;
    update.GetSql(sql);

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, sql.c_str());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;

    if (info.ColsUpdate & DbECClassMapInfo::COL_ECClassId)
        if (info.ColsNull & DbECClassMapInfo::COL_ECClassId) nCol++; else stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsUpdate & DbECClassMapInfo::COL_MapParentECClassId)
        if (info.ColsNull & DbECClassMapInfo::COL_MapParentECClassId) nCol++; else stmt->BindInt64 (nCol++, info.m_mapParentECClassId);
    if (info.ColsUpdate & DbECClassMapInfo::COL_MapStrategy)
        if (info.ColsNull & DbECClassMapInfo::COL_MapStrategy) nCol++; else stmt->BindInt (nCol++, ToInt (info.m_mapStrategy));
    if (info.ColsUpdate & DbECClassMapInfo::COL_MapToDbTable) 
        if (info.ColsNull & DbECClassMapInfo::COL_MapToDbTable) nCol++; else stmt->BindText (nCol++, info.m_mapToDbTable, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECClassMapInfo::COL_ECInstanceIdColumn)
        if (info.ColsNull & DbECClassMapInfo::COL_ECInstanceIdColumn) nCol++; else stmt->BindText (nCol++, info.m_primaryKeyColumnName, Statement::MakeCopy::No);

    if (info.ColsWhere & DbECClassMapInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertBaseClass (BeSQLite::Db& db, DbBaseClassInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_BaseClass (ECClassId, BaseECClassId, ECIndex) VALUES (?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbBaseClassInfo::COL_ECClassId) stmt->BindInt64 (1, info.m_ecClassId);
    if (info.ColsInsert & DbBaseClassInfo::COL_BaseECClassId) stmt->BindInt64 (2, info.m_baseECClassId);
    if (info.ColsInsert & DbBaseClassInfo::COL_ECIndex) stmt->BindInt (3, info.m_ecIndex);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindBaseClassInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbBaseClassInfoCR info)
    {
    SqlSelect sql;
    sql.AddFrom    ("ec_BaseClass");
    sql.AddOrderBy ("ECIndex");
    //prepare select list
    if (info.ColsSelect & DbBaseClassInfo::COL_ECClassId ) sql.AddSelect ("ECClassId");
    if (info.ColsSelect & DbBaseClassInfo::COL_BaseECClassId) sql.AddSelect ("BaseECClassId");
    if (info.ColsSelect & DbBaseClassInfo::COL_ECIndex) sql.AddSelect ("ECIndex");

    //prepare where
    if (info.ColsWhere & DbBaseClassInfo::COL_ECClassId) sql.AddWhere("ECClassId", "=?");
    if (info.ColsWhere & DbBaseClassInfo::COL_BaseECClassId) sql.AddWhere("BaseECClassId", "=?");
    if (info.ColsWhere & DbBaseClassInfo::COL_ECIndex) sql.AddWhere("ECIndex", "=?");

    Utf8String select;
    sql.GetSql(select);
    auto stat = db.GetCachedStatement (stmt, select.c_str());
    if (BE_SQLITE_OK != stat)
        return stat;
    
    int nCol = 1;
    if (info.ColsWhere & DbBaseClassInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbBaseClassInfo::COL_BaseECClassId) stmt->BindInt64 (nCol++, info.m_baseECClassId);
    if (info.ColsWhere & DbBaseClassInfo::COL_ECIndex) stmt->BindInt (nCol++, info.m_ecIndex);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbBaseClassInfoR info , BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbBaseClassInfo::COL_ECClassId) info.m_ecClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbBaseClassInfo::COL_BaseECClassId) info.m_baseECClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbBaseClassInfo::COL_ECIndex) info.m_ecIndex = stmt.GetValueInt (nCol++);
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECSchemaReferenceInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECSchemaReferenceInfoCR info)
    {
    SqlSelect           sql;
    sql.AddFrom    ("ec_SchemaReference");
    //prepare select list
    if (info.ColsSelect & DbECSchemaReferenceInfo::COL_ECSchemaId) sql.AddSelect ("ECSchemaId");
    if (info.ColsSelect & DbECSchemaReferenceInfo::COL_ReferenceECSchemaId) sql.AddSelect ("ReferenceECSchemaId");

    //prepare where
    if (info.ColsWhere & DbECSchemaReferenceInfo::COL_ECSchemaId) sql.AddWhere("ECSchemaId", "=?");
    if (info.ColsWhere & DbECSchemaReferenceInfo::COL_ReferenceECSchemaId) sql.AddWhere("ReferenceECSchemaId", "=?");

    Utf8String select;
    sql.GetSql (select);

    auto stat = db.GetCachedStatement (stmt, select.c_str());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECSchemaReferenceInfo::COL_ECSchemaId) stmt->BindInt64 (nCol++, info.m_ecSchemaId);
    if (info.ColsWhere & DbECSchemaReferenceInfo::COL_ReferenceECSchemaId) stmt->BindInt64 (nCol++, info.m_referenceECSchemaId);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECSchemaReferenceInfoR info, BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step ()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECSchemaReferenceInfo::COL_ECSchemaId) info.m_ecSchemaId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbECSchemaReferenceInfo::COL_ReferenceECSchemaId) info.m_referenceECSchemaId = stmt.GetValueInt64 (nCol++);
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECPropertyInfo (BeSQLite::Db& db, DbECPropertyInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_Property (ECClassId, ECPropertyId, Name, DisplayLabel, Description, IsArray, TypeCustom, TypeECPrimitive, TypeGeometry, TypeECStruct, ECIndex, IsReadOnly, MinOccurs, MaxOccurs) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECPropertyInfo::COL_ECClassId) stmt->BindInt64 (1, info.m_ecClassId);
    if (info.ColsInsert & DbECPropertyInfo::COL_ECPropertyId) stmt->BindInt64 (2, info.m_ecPropertyId);
    if (info.ColsInsert & DbECPropertyInfo::COL_Name) stmt->BindText (3, info.m_name, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_DisplayLabel) stmt->BindText (4, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_Description) stmt->BindText (5, info.m_description, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_IsArray) stmt->BindInt (6, info.m_isArray ? 1 : 0);
    if (info.ColsInsert & DbECPropertyInfo::COL_TypeCustom) stmt->BindText (7, info.m_typeCustom, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_TypeECPrimitive) stmt->BindInt (8, info.m_typeECPrimitive);
    if (info.ColsInsert & DbECPropertyInfo::COL_TypeGeometry) stmt->BindText (9, info.m_typeGeometry, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECPropertyInfo::COL_TypeECStruct) stmt->BindInt64 (10, info.m_typeECStruct);
    if (info.ColsInsert & DbECPropertyInfo::COL_ECIndex) stmt->BindInt (11, info.m_ecIndex);
    if (info.ColsInsert & DbECPropertyInfo::COL_IsReadOnly) stmt->BindInt (12, info.m_isReadOnly ? 1 : 0);

    if (info.m_isArray)
        {
        if (info.ColsInsert & DbECPropertyInfo::COL_MinOccurs) stmt->BindInt (13, (int32_t) info.m_minOccurs);
        if (info.ColsInsert & DbECPropertyInfo::COL_MaxOccurs) stmt->BindInt (14, (int32_t) info.m_maxOccurs);
        }

    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECPropertyInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECPropertyInfoCR info)
    {
    SqlSelect           sql;
    sql.AddFrom ("ec_Property");
    sql.AddOrderBy ("ECIndex");
    //prepare select list
    if (info.ColsSelect & DbECPropertyInfo::COL_ECClassId) sql.AddSelect ("ECClassId");
    if (info.ColsSelect & DbECPropertyInfo::COL_ECPropertyId) sql.AddSelect ("ECPropertyId");
    if (info.ColsSelect & DbECPropertyInfo::COL_Name) sql.AddSelect ("Name");
    if (info.ColsSelect & DbECPropertyInfo::COL_DisplayLabel) sql.AddSelect ("DisplayLabel");
    if (info.ColsSelect & DbECPropertyInfo::COL_Description) sql.AddSelect ("Description");
    if (info.ColsSelect & DbECPropertyInfo::COL_IsArray) sql.AddSelect ("IsArray");
    if (info.ColsSelect & DbECPropertyInfo::COL_TypeCustom) sql.AddSelect ("TypeCustom");
    if (info.ColsSelect & DbECPropertyInfo::COL_TypeECPrimitive) sql.AddSelect ("TypeECPrimitive");
    if (info.ColsSelect & DbECPropertyInfo::COL_TypeGeometry) sql.AddSelect ("TypeGeometry");
    if (info.ColsSelect & DbECPropertyInfo::COL_TypeECStruct) sql.AddSelect ("TypeECStruct");
    if (info.ColsSelect & DbECPropertyInfo::COL_ECIndex) sql.AddSelect ("ECIndex");
    if (info.ColsSelect & DbECPropertyInfo::COL_IsReadOnly) sql.AddSelect ("IsReadOnly");
    if (info.ColsSelect & DbECPropertyInfo::COL_MinOccurs) sql.AddSelect ("MinOccurs");
    if (info.ColsSelect & DbECPropertyInfo::COL_MaxOccurs) sql.AddSelect ("MaxOccurs");

    //prepare where
    if (info.ColsWhere & DbECPropertyInfo::COL_ECClassId) sql.AddWhere ("ECClassId", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_ECPropertyId) sql.AddWhere ("ECPropertyId", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_Name) sql.AddWhere ("Name", "=?");

    Utf8String select;
    sql.GetSql (select);

    auto stat = db.GetCachedStatement (stmt, select.c_str ());
    if (stat != BE_SQLITE_OK)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECPropertyInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECPropertyInfo::COL_ECPropertyId) stmt->BindInt64 (nCol++, info.m_ecPropertyId);
    if (info.ColsWhere & DbECPropertyInfo::COL_Name) stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step(DbECPropertyInfoR info , BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECPropertyInfo::COL_ECClassId)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_ECClassId;       nCol++; } else info.m_ecClassId = stmt.GetValueInt64 (nCol++);            
        if (info.ColsSelect & DbECPropertyInfo::COL_ECPropertyId)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_ECPropertyId;    nCol++; } else info.m_ecPropertyId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_Name)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_Name;            nCol++; } else info.m_name = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_DisplayLabel)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_DisplayLabel;    nCol++; } else info.m_displayLabel = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_Description)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_Description;     nCol++; } else info.m_description = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_IsArray)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_IsArray;         nCol++; } else info.m_isArray = stmt.GetValueInt  (nCol++) == 1;
        if (info.ColsSelect & DbECPropertyInfo::COL_TypeCustom)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_TypeCustom;      nCol++; } else info.m_typeCustom = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_TypeECPrimitive)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_TypeECPrimitive; nCol++; } else info.m_typeECPrimitive = (ECN::PrimitiveType)stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_TypeGeometry)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_TypeGeometry;    nCol++; } else info.m_typeGeometry = stmt.GetValueText (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_TypeECStruct)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_TypeECStruct;    nCol++; } else info.m_typeECStruct = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_ECIndex)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_ECIndex;         nCol++; } else info.m_ecIndex = stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_IsReadOnly)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbECPropertyInfo::COL_IsReadOnly;      nCol++; } else info.m_isReadOnly = stmt.GetValueInt (nCol++) == 1;
        if (info.ColsSelect & DbECPropertyInfo::COL_MinOccurs)
            if (stmt.IsColumnNull (nCol)) { info.ColsNull |= DbECPropertyInfo::COL_MinOccurs;      nCol++; } else info.m_minOccurs = (uint32_t)stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECPropertyInfo::COL_MaxOccurs)
            if (stmt.IsColumnNull (nCol)) { info.ColsNull |= DbECPropertyInfo::COL_MaxOccurs;      nCol++; } else info.m_maxOccurs = (uint32_t)stmt.GetValueInt (nCol++);

        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateECPropertyInfo (BeSQLite::Db& db, DbECPropertyInfoCR info)
    {
    SqlUpdate update;
    update.SetTable ("ec_Property");

    if (info.ColsWhere & DbECPropertyInfo::COL_ECClassId) update.AddWhere ("ECClassId", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_ECPropertyId) update.AddWhere ("ECPropertyId", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_Name) update.AddWhere ("Name", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_DisplayLabel) update.AddWhere ("DisplayLabel", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_Description) update.AddWhere ("Description", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_IsArray) update.AddWhere ("IsArray", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeCustom) update.AddWhere ("TypeCustom", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeECPrimitive) update.AddWhere ("TypeECPrimitive", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeGeometry) update.AddWhere ("TypeGeometry", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeECStruct) update.AddWhere ("TypeECStruct", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_ECIndex) update.AddWhere ("ECIndex", "=?");
    if (info.ColsWhere & DbECPropertyInfo::COL_IsReadOnly) update.AddWhere ("IsReadOnly", "=?");


    if (info.ColsUpdate & DbECPropertyInfo::COL_ECClassId) update.AddUpdateColumn ("ECClassId", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_ECPropertyId) update.AddUpdateColumn ("ECPropertyId", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_Name) update.AddUpdateColumn ("Name", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_DisplayLabel) update.AddUpdateColumn ("DisplayLabel", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_Description) update.AddUpdateColumn ("Description", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_IsArray) update.AddUpdateColumn ("IsArray", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeCustom) update.AddUpdateColumn ("TypeCustom", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeECPrimitive) update.AddUpdateColumn ("TypeECPrimitive", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeGeometry) update.AddUpdateColumn ("TypeGeometry", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeECStruct) update.AddUpdateColumn ("TypeECStruct", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_ECIndex) update.AddUpdateColumn ("ECIndex", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_IsReadOnly) update.AddUpdateColumn ("IsReadOnly", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_MinOccurs) update.AddUpdateColumn ("MinOccurs", "?");
    if (info.ColsUpdate & DbECPropertyInfo::COL_MaxOccurs) update.AddUpdateColumn ("MaxOccurs", "?");


    Utf8String sql;
    update.GetSql (sql);
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, sql.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsUpdate & DbECPropertyInfo::COL_ECClassId)
        if (info.ColsNull & DbECPropertyInfo::COL_ECClassId)  nCol++; else stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsUpdate & DbECPropertyInfo::COL_ECPropertyId)
        if (info.ColsNull & DbECPropertyInfo::COL_ECPropertyId)  nCol++; else stmt->BindInt64 (nCol++, info.m_ecPropertyId);
    if (info.ColsUpdate & DbECPropertyInfo::COL_Name)
        if (info.ColsNull & DbECPropertyInfo::COL_Name) nCol++; else stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECPropertyInfo::COL_DisplayLabel)
        if (info.ColsNull & DbECPropertyInfo::COL_DisplayLabel) nCol++; else stmt->BindText (nCol++, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECPropertyInfo::COL_Description)
        if (info.ColsNull & DbECPropertyInfo::COL_Description)  nCol++; else stmt->BindText (nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECPropertyInfo::COL_IsArray)
        if (info.ColsNull & DbECPropertyInfo::COL_IsArray) nCol++; else stmt->BindInt (nCol++, info.m_isArray ? 1 : 0);
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeCustom)
        if (info.ColsNull & DbECPropertyInfo::COL_TypeCustom) nCol++; else stmt->BindText (nCol++, info.m_typeCustom, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeECPrimitive)
        if (info.ColsNull & DbECPropertyInfo::COL_TypeECPrimitive) nCol++; else stmt->BindInt (nCol++, info.m_typeECPrimitive);
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeGeometry)
        if (info.ColsNull & DbECPropertyInfo::COL_TypeGeometry) nCol++; else stmt->BindText (nCol++, info.m_typeGeometry, Statement::MakeCopy::No);
    if (info.ColsUpdate & DbECPropertyInfo::COL_TypeECStruct)
        if (info.ColsNull & DbECPropertyInfo::COL_TypeECStruct) nCol++; else stmt->BindInt64 (nCol++, info.m_typeECStruct);
    if (info.ColsUpdate & DbECPropertyInfo::COL_ECIndex)
        if (info.ColsNull & DbECPropertyInfo::COL_ECIndex) nCol++; else stmt->BindInt (nCol++, info.m_ecIndex);
    if (info.ColsUpdate & DbECPropertyInfo::COL_IsReadOnly)
        if (info.ColsNull & DbECPropertyInfo::COL_IsReadOnly) nCol++; else stmt->BindInt (nCol++, info.m_isReadOnly ? 1 : 0);
    if (info.ColsUpdate & DbECPropertyInfo::COL_MinOccurs)
        if (info.ColsNull & DbECPropertyInfo::COL_MinOccurs) nCol++; else stmt->BindInt (nCol++, (int32_t) info.m_minOccurs);
    if (info.ColsUpdate & DbECPropertyInfo::COL_MaxOccurs)
        if (info.ColsNull & DbECPropertyInfo::COL_MaxOccurs) nCol++; else stmt->BindInt (nCol++, (int32_t) info.m_maxOccurs);

    if (info.ColsWhere & DbECPropertyInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECPropertyInfo::COL_ECPropertyId) stmt->BindInt64 (nCol++, info.m_ecPropertyId);
    if (info.ColsWhere & DbECPropertyInfo::COL_Name) stmt->BindText (nCol++, info.m_name, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECPropertyInfo::COL_DisplayLabel) stmt->BindText (nCol++, info.m_displayLabel, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECPropertyInfo::COL_Description) stmt->BindText (nCol++, info.m_description, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECPropertyInfo::COL_IsArray) stmt->BindInt (nCol++, info.m_isArray ? 1 : 0);
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeCustom) stmt->BindText (nCol++, info.m_typeCustom, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeECPrimitive) stmt->BindInt (nCol++, info.m_typeECPrimitive);
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeGeometry) stmt->BindText (nCol++, info.m_typeGeometry, Statement::MakeCopy::No);
    if (info.ColsWhere & DbECPropertyInfo::COL_TypeECStruct) stmt->BindInt64 (nCol++, info.m_typeECStruct);
    if (info.ColsWhere & DbECPropertyInfo::COL_ECIndex) stmt->BindInt (nCol++, info.m_ecIndex);
    if (info.ColsWhere & DbECPropertyInfo::COL_IsReadOnly) stmt->BindInt (nCol++, info.m_isReadOnly ? 1 : 0);

    stat = stmt->Step ();
    BeAssert (stat != BE_SQLITE_DONE || db.GetModifiedRowCount () > 0);
    return stat;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECRelationConstraintInfo (BeSQLite::Db& db, DbECRelationshipConstraintInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_RelationshipConstraint (ECClassId, ECRelationshipEnd, CardinalityLowerLimit, CardinalityUpperLimit, RoleLable, IsPolymorphic) VALUES (?, ?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_ECClassId) stmt->BindInt64 (1, info.m_ecClassId);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) stmt->BindInt (2, info.m_ecRelationshipEnd);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) stmt->BindInt (3, info.m_cardinalityLowerLimit);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) stmt->BindInt (4, info.m_cardinalityUpperLimit);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_RoleLabel) stmt->BindText (5, info.m_roleLabel, Statement::MakeCopy::No);
    if (info.ColsInsert & DbECRelationshipConstraintInfo::COL_IsPolymorphic) stmt->BindInt (6, info.m_isPolymorphic ? 1 : 0);

    return stmt->Step();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateECRelationConstraintInfo (BeSQLite::Db& db, DbECRelationshipConstraintInfoCR info)
    {
    SqlUpdate sql;
    sql.SetTable("ec_RelationshipConstraint");

    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECClassId) 
        sql.AddWhere ("ECClassId", "=?");

    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) 
        sql.AddWhere ("ECRelationshipEnd", "=?");

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_ECClassId) 
        sql.AddUpdateColumn ("ECClassId", "?");
    
    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) 
        sql.AddUpdateColumn ("ECRelationshipEnd", "?");
    
    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_RoleLabel) 
        sql.AddUpdateColumn ("RoleLable", "?");
    
    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_IsPolymorphic) 
        sql.AddUpdateColumn ("IsPolymorphic", "?");
    
    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) 
        sql.AddUpdateColumn ("CardinalityLowerLimit", "?");

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) 
        sql.AddUpdateColumn ("CardinalityUpperLimit", "?");

    Utf8String update;
    sql.GetSql (update);

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, update.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol= 1;
    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_ECClassId)
        if (info.ColsNull & DbECRelationshipConstraintInfo::COL_ECClassId) 
            nCol++; 
        else 
            stmt->BindInt64 (nCol++, info.m_ecClassId);

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd)
        if (info.ColsNull & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) 
            nCol++; 
        else 
            stmt->BindInt (nCol++, info.m_ecRelationshipEnd);

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_RoleLabel)
        if (info.ColsNull & DbECRelationshipConstraintInfo::COL_RoleLabel) 
            nCol++; 
        else 
            stmt->BindText (nCol++, info.m_roleLabel.c_str(), Statement::MakeCopy::No);

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_IsPolymorphic)
        if (info.ColsNull & DbECRelationshipConstraintInfo::COL_IsPolymorphic) 
            nCol++; 
        else 
            stmt->BindInt (nCol++, info.m_isPolymorphic);

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit)
        if (info.ColsNull & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) 
            nCol++; 
        else 
            stmt->BindInt (nCol++, info.m_cardinalityLowerLimit);

    if (info.ColsUpdate & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit)
        if (info.ColsNull & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) 
            nCol++; 
        else 
            stmt->BindInt (nCol++, info.m_cardinalityUpperLimit);

    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECClassId)
        stmt->BindInt64 (nCol++, info.m_ecClassId);

    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) 
        stmt->BindInt   (nCol++, info.m_ecRelationshipEnd);

    stat = stmt->Step();
    BeAssert (stat != BE_SQLITE_DONE || db.GetModifiedRowCount () > 0);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECRelationshipConstraintInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECRelationshipConstraintInfoCR info)
    {
    SqlSelect sql;

    sql.AddFrom ("ec_RelationshipConstraint");
    //prepare select list
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_ECClassId) sql.AddSelect ("ECClassId");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) sql.AddSelect ("ECRelationshipEnd");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) sql.AddSelect ("CardinalityLowerLimit");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) sql.AddSelect ("CardinalityUpperLimit");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_RoleLabel) sql.AddSelect ("RoleLable");
    if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_IsPolymorphic) sql.AddSelect ("IsPolymorphic");


    //prepare where
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECClassId) sql.AddWhere ("ECClassId", "=?");
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) sql.AddWhere ("ECRelationshipEnd", "=?");

    Utf8String select;
    sql.GetSql (select);
    auto stat = db.GetCachedStatement (stmt, select.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) stmt->BindInt (nCol++, info.m_ecRelationshipEnd);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECRelationshipConstraintInfoR info, BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step ()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_ECClassId) info.m_ecClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_ECRelationshipEnd) info.m_ecRelationshipEnd = (ECN::ECRelationshipEnd)stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_CardinalityLowerLimit) info.m_cardinalityLowerLimit = stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_CardinalityUpperLimit) info.m_cardinalityUpperLimit = stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_RoleLabel)
            if (stmt.IsColumnNull (nCol)) { info.ColsNull |= DbECRelationshipConstraintInfo::COL_RoleLabel; nCol++; }
            else info.m_roleLabel = stmt.GetValueText (nCol++);
            if (info.ColsSelect & DbECRelationshipConstraintInfo::COL_IsPolymorphic) info.m_isPolymorphic = stmt.GetValueInt (nCol++) == 1;
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECRelationConstraintClassInfo (BeSQLite::Db& db, DbECRelationshipConstraintClassInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_RelationshipConstraintClass (ECClassId, ECRelationshipEnd, RelationECClassId) VALUES (?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbECRelationshipConstraintClassInfo::COL_ECClassId) stmt->BindInt64 (1, info.m_ecClassId);
    if (info.ColsInsert & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) stmt->BindInt (2, info.m_ecRelationshipEnd);
    if (info.ColsInsert & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) stmt->BindInt64 (3, info.m_relationECClassId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindECRelationConstraintClassInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbECRelationshipConstraintClassInfoCR info)
    {
    SqlSelect sql;
    sql.AddFrom ("ec_RelationshipConstraintClass");
    //prepare select list
    if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_ECClassId) sql.AddSelect ("ECClassId");
    if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) sql.AddSelect ("ECRelationshipEnd");
    if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) sql.AddSelect ("RelationECClassId");


    //prepare where
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECClassId) sql.AddWhere ("ECClassId", "=?");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) sql.AddWhere ("ECRelationshipEnd", "=?");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) sql.AddWhere ("RelationECClassId", "=?");

    Utf8String select;
    sql.GetSql (select);
    auto stat = db.GetCachedStatement (stmt, select.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) stmt->BindInt (nCol++, info.m_ecRelationshipEnd);
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) stmt->BindInt64 (nCol++, info.m_relationECClassId);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbECRelationshipConstraintClassInfoR info, BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step ()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_ECClassId) info.m_ecClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) info.m_ecRelationshipEnd = (ECRelationshipEnd) stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) info.m_relationECClassId = stmt.GetValueInt64 (nCol++);
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateECRelationshipConstraintClass (BeSQLite::Db& db, DbECRelationshipConstraintClassInfoCR info)
    {
    SqlUpdate sql;
    sql.SetTable ("ec_RelationshipConstraintClass");

    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECClassId) sql.AddWhere ("ECClassId", "=?");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) sql.AddWhere ("ECRelationshipEnd", "=?");
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) sql.AddWhere ("RelationECClassId", "=?");

    Utf8String update;
    sql.GetSql (update);
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, update.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_ECRelationshipEnd) stmt->BindInt (nCol++, info.m_ecRelationshipEnd);
    if (info.ColsWhere & DbECRelationshipConstraintClassInfo::COL_RelationECClassId) stmt->BindInt64 (nCol++, info.m_relationECClassId);

    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertCustomAttributeInfo (BeSQLite::Db& db, DbCustomAttributeInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_CustomAttribute (ContainerId, ContainerType, OverridenByContainerId, ECClassId, [Index], ECId, Instance) VALUES(?, ?, ?, ?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    if (info.ColsInsert & DbCustomAttributeInfo::COL_ContainerId) stmt->BindInt64 (1, info.m_containerId);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_ContainerType) stmt->BindInt (2, info.m_containerType);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_OverridenByContainerId) stmt->BindInt64 (3, info.m_overridenByContainerId);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_ECClassId) stmt->BindInt64 (4, info.m_ecClassId);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_Index) stmt->BindInt (5, info.m_index);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_ECInstanceId) stmt->BindId (6, info.m_ecInstanceId);
    if (info.ColsInsert & DbCustomAttributeInfo::COL_Instance)
        {
        //old format (ECD): stmt->BindBlob  (8, info.m_caAsBlob.GetData(), (int)info.m_caAsBlob.GetLength(), Statement::MakeCopy::No);
        DbResult stat = stmt->BindText (7, info.GetCaInstanceXml (), Statement::MakeCopy::No);
        POSTCONDITION (stat == BE_SQLITE_OK, stat);
        }

    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::FindCustomAttributeInfo (BeSQLite::CachedStatementPtr& stmt, BeSQLite::Db& db, DbCustomAttributeInfoCR info, bool getOnlyPrimaryCustomAttributes)
    {
    SqlSelect sql;
    sql.AddFrom ("ec_CustomAttribute");
    //AddOrderBy does not automatically add brackets. So we have to do it here
    sql.AddOrderBy ("[Index]");

    //prepare select list
    if (info.ColsSelect & DbCustomAttributeInfo::COL_ContainerId)            sql.AddSelect ("ContainerId");
    if (info.ColsSelect & DbCustomAttributeInfo::COL_ContainerType)          sql.AddSelect ("ContainerType");
    if (info.ColsSelect & DbCustomAttributeInfo::COL_OverridenByContainerId) sql.AddSelect ("OverridenByContainerId");
    if (info.ColsSelect & DbCustomAttributeInfo::COL_ECClassId)              sql.AddSelect ("ECClassId");
    //this overload of AddSelect doesn't add brackets automatically. So we have to do it here
    if (info.ColsSelect & DbCustomAttributeInfo::COL_Index)                  sql.AddSelect ("[Index]");
    if (info.ColsSelect & DbCustomAttributeInfo::COL_ECInstanceId)
        {
        if (getOnlyPrimaryCustomAttributes)
            sql.AddSelect ("ECId");
        else
            sql.AddSelect ("(CASE WHEN (OverridenByContainerId IS NULL) "
            "THEN ECId "
            "ELSE (SELECT S.ECId FROM ec_CustomAttribute S WHERE S.ContainerType = ec_CustomAttribute.ContainerType AND S.ECClassId = ec_CustomAttribute.ECClassId AND S.ContainerId = ec_CustomAttribute.OverridenByContainerId) "
            "END) AS ECId");
        }

    if (info.ColsSelect & DbCustomAttributeInfo::COL_Instance)
        {
        if (getOnlyPrimaryCustomAttributes)
            sql.AddSelect ("Instance");
        else
            sql.AddSelect ("(CASE WHEN OverridenByContainerId IS NULL "
            "THEN Instance "
            "ELSE (SELECT S.Instance FROM ec_CustomAttribute S WHERE S.ContainerType = ec_CustomAttribute.ContainerType AND S.ECClassId = ec_CustomAttribute.ECClassId AND S.ContainerId = ec_CustomAttribute.OverridenByContainerId) "
            "END) AS Instance");
        }

    //prepare where
    if (info.ColsWhere & DbCustomAttributeInfo::COL_ContainerId)
        sql.AddWhere ("ContainerId", info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerId ? "is null" : "=?");
    if (info.ColsWhere & DbCustomAttributeInfo::COL_ContainerType)
        sql.AddWhere ("ContainerType", info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerType ? "is null" : "=?");
    if (info.ColsWhere & DbCustomAttributeInfo::COL_OverridenByContainerId)
        sql.AddWhere ("OverridenByContainerId", info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_OverridenByContainerId ? "is null" : "=?");
    if (info.ColsWhere & DbCustomAttributeInfo::COL_ECClassId)
        sql.AddWhere ("ECClassId", info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ECClassId ? "is null" : "=?");
    if (info.ColsWhere & DbCustomAttributeInfo::COL_Index)
        {
        //This overload of AddWhere does add brackets around column name automatically
        sql.AddWhere ("Index", info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_Index ? "is null" : "=?");
        }
    if (info.ColsWhere & DbCustomAttributeInfo::COL_ECInstanceId)
        sql.AddWhere ("ECId", info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ECInstanceId ? "is null" : "=?");

    Utf8String select;
    sql.GetSql (select);

    auto stat = db.GetCachedStatement (stmt, select.c_str ());
    if (stat != BE_SQLITE_OK)
        return stat;

    int nCol = 1;
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerId)            stmt->BindInt64 (nCol++, info.m_containerId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerType)          stmt->BindInt (nCol++, info.m_containerType);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_OverridenByContainerId) stmt->BindInt64 (nCol++, info.m_overridenByContainerId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ECClassId)              stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_Index)                  stmt->BindInt (nCol++, info.m_index);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ECInstanceId)           stmt->BindId (nCol++, info.m_ecInstanceId);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdateCustomAttributeInfo (BeSQLite::Db& db, DbCustomAttributeInfoCR info)
    {
    SqlUpdate sql;
    sql.SetTable ("ec_CustomAttribute");

    if (info.ColsUpdate & DbCustomAttributeInfo::COL_Instance) sql.AddUpdateColumn ("Instance", "?");
    if (info.ColsUpdate & DbCustomAttributeInfo::COL_OverridenByContainerId) sql.AddUpdateColumn ("OverridenByContainerId", "?");
    if (info.ColsUpdate & DbCustomAttributeInfo::COL_ECInstanceId) sql.AddUpdateColumn ("ECId", "?");

    if (info.ColsWhere & DbCustomAttributeInfo::COL_ContainerId)
        sql.AddWhere ("ContainerId", info.ColsNull & DbCustomAttributeInfo::COL_ContainerId ? "is null" : " = ?");

    if (info.ColsWhere & DbCustomAttributeInfo::COL_ContainerType)
        sql.AddWhere ("ContainerType", info.ColsNull & DbCustomAttributeInfo::COL_ContainerType ? "is null" : " = ?");

    if (info.ColsWhere & DbCustomAttributeInfo::COL_ECClassId)
        sql.AddWhere ("ECClassId", info.ColsNull & DbCustomAttributeInfo::COL_ECClassId ? "is null" : " = ?");

    if (info.ColsWhere & DbCustomAttributeInfo::COL_Index)
        {
        // This overload of AddWhere adds brackets around the column automatically (which is required for the Index column
        // as it is a reserved word in SQLite)
        sql.AddWhere ("Index", info.ColsNull & DbCustomAttributeInfo::COL_Index ? "is null" : " = ?");
        }

    Utf8String update;
    sql.GetSql (update);
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, update.c_str ());
    if (BE_SQLITE_OK != stat)
        return stat;

    int nCol = 1;

    if (~info.ColsNull & info.ColsUpdate & DbCustomAttributeInfo::COL_Instance)
        {
        stat = stmt->BindText (nCol++, info.GetCaInstanceXml (), Statement::MakeCopy::No);
        POSTCONDITION (stat == BE_SQLITE_OK, stat);
        }
    if (~info.ColsNull & info.ColsUpdate & DbCustomAttributeInfo::COL_OverridenByContainerId) stmt->BindInt64 (nCol++, info.m_overridenByContainerId);
    if (~info.ColsNull & info.ColsUpdate & DbCustomAttributeInfo::COL_ECInstanceId) stmt->BindId (nCol++, info.m_ecInstanceId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerId) stmt->BindInt64 (nCol++, info.m_containerId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ContainerType) stmt->BindInt (nCol++, info.m_containerType);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_ECClassId) stmt->BindInt64 (nCol++, info.m_ecClassId);
    if (~info.ColsNull & info.ColsWhere & DbCustomAttributeInfo::COL_Index) stmt->BindInt (nCol++, info.m_index);

    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::Step (DbCustomAttributeInfoR info, BeSQLite::Statement& stmt)
    {
    DbResult r;
    if ((r = stmt.Step()) == BE_SQLITE_ROW)
        {
        int nCol = 0;
        info.ColsNull = 0;
        if (info.ColsSelect & DbCustomAttributeInfo::COL_ContainerId)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbCustomAttributeInfo::COL_ContainerId; nCol++; } else info.m_containerId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbCustomAttributeInfo::COL_ContainerType) 
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbCustomAttributeInfo::COL_ContainerType; nCol++; } else info.m_containerType = (ECContainerType)stmt.GetValueInt (nCol++);
        if (info.ColsSelect & DbCustomAttributeInfo::COL_OverridenByContainerId) 
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbCustomAttributeInfo::COL_OverridenByContainerId; nCol++; } else info.m_overridenByContainerId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbCustomAttributeInfo::COL_ECClassId)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbCustomAttributeInfo::COL_ECClassId; nCol++; } else info.m_ecClassId = stmt.GetValueInt64 (nCol++);
        if (info.ColsSelect & DbCustomAttributeInfo::COL_Index) 
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbCustomAttributeInfo::COL_Index; nCol++; } else info.m_index = stmt.GetValueInt(nCol++);
        if (info.ColsSelect & DbCustomAttributeInfo::COL_ECInstanceId)
            if (stmt.IsColumnNull(nCol)) { info.ColsNull |= DbCustomAttributeInfo::COL_ECInstanceId; nCol++; } else info.m_ecInstanceId = stmt.GetValueId<ECInstanceId> (nCol++);
        if (info.ColsSelect & DbCustomAttributeInfo::COL_Instance)
            if (stmt.IsColumnNull(nCol)) 
                { 
                info.ColsNull |= DbCustomAttributeInfo::COL_Instance; 
                nCol++; 
                } 
            else
                {
                //TODO: is it ensured that the info object is not used after the next call to ECDbSchemaPersistence::Step?
                Utf8CP caInstanceXml = stmt.GetValueText (nCol);
                info.SetCaInstanceXml (caInstanceXml);
                //old way with ECD format
                //info.m_caAsBlob.SetData (const_cast<byte*>((byte*)stmt.GetValueBlob(nCol)), stmt.GetColumnBytes(nCol));
                }
        }
    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECRelationConstraintClassPropertyInfo (BeSQLite::Db& db, DbECRelationshipConstraintClassPropertyInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_RelationshipConstraintClassProperty (ECClassId, ECRelationshipEnd, RelationECClassId, RelationECPropertyId) VALUES (?, ?, ?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, info.m_ecClassId);
    stmt->BindInt (2, info.m_ecRelationshipEnd);
    stmt->BindInt64 (3, info.m_relationECClassId);
    stmt->BindInt64 (4, info.m_relationECPropertyId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::InsertECSchemaReferenceInfo (BeSQLite::Db& db, DbECSchemaReferenceInfoCR info)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "INSERT INTO ec_SchemaReference (ECSchemaId, ReferenceECSchemaId) VALUES(?, ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, info.m_ecSchemaId);
    stmt->BindInt64 (2, info.m_referenceECSchemaId);
    return stmt->Step ();
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::ContainsECClass (BeSQLite::Db& db, ECClassCR ecClass)
    {
    if (ecClass.HasId()) //This is unsafe but since we donot delete ecclass any class that hasId does exist in db
        return true;

    auto classId = GetECClassIdBySchemaName(db , 
                        Utf8String(ecClass.GetSchema().GetName().c_str()).c_str(),
                        Utf8String(ecClass.GetName().c_str()).c_str());

    return classId != 0;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::ContainsECSchemaReference (BeSQLite::Db& db, ECSchemaId ecPrimarySchemaId, ECSchemaId ecReferenceSchemaId)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECSchemaId FROM ec_SchemaReference WHERE ECSchemaId = ? AND ReferenceECSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return false;

    stmt->BindInt64 (1, ecPrimarySchemaId);
    stmt->BindInt64 (2, ecReferenceSchemaId);
    return stmt->Step () == BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::ResolveECClassId (Utf8StringR schemaName, uint32_t& versionMajor, uint32_t& versionMinor, Utf8StringR className, ECClassId ecClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT s.Name, s.VersionMajor, s.VersionMinor, c.Name  FROM ec_Class c INNER JOIN ec_Schema s ON s.ECSchemaId = c.ECSchemaId WHERE ECClassId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecClassId);

    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return stat;

    schemaName = stmt->GetValueText (0);
    versionMajor = (uint32_t) stmt->GetValueInt (1);
    versionMinor = (uint32_t) stmt->GetValueInt (2);
    className = stmt->GetValueText (3);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::ResolveECClassId (DbECClassEntryR key, ECClassId ecClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT Name, ECSchemaId  FROM ec_Class WHERE ECClassId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecClassId);

    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return stat;

    key.m_className = stmt->GetValueText (0);
    key.m_ecSchemaId = stmt->GetValueInt64 (1);
    key.m_ecClassId = ecClassId;
    key.m_resolvedECClass = nullptr;
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::ResolveECSchemaId (DbECSchemaEntryR key, ECSchemaId ecSchemaId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT S.ECSchemaId, S.Name, S.VersionMajor, S.VersionMinor, (SELECT COUNT(*) FROM ec_Class C WHERE S.ECSchemaId = C.ECSchemaID) FROM ec_Schema S WHERE S.ECSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecSchemaId);

    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return stat;

    key.m_ecSchemaId = stmt->GetValueInt64 (0);
    key.m_schemaName = stmt->GetValueText (1);
    key.m_versionMajor = (uint32_t) stmt->GetValueInt (2);
    key.m_versionMinor = (uint32_t) stmt->GetValueInt (3);
    key.m_nClassesInSchema = (uint32_t) stmt->GetValueInt (4);
    key.m_nClassesLoaded = 0;
    key.m_resolvedECSchema = nullptr;
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetPrimaryECSchemas (ECSchemaKeyListR schemaKeys, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT S.ECSchemaId, S.Name, S.VersionMajor, S.VersionMinor, COUNT(*) FROM ec_Schema S INNER JOIN ec_Class C ON S.ECSchemaId = C.ECSchemaID WHERE S.SchemaType=? GROUP BY S.ECSchemaId, S.Name, S.VersionMajor, S.VersionMinor");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt (1, PERSISTEDSCHEMATYPE_Primary);

    while ((stat = stmt->Step ()) == BE_SQLITE_ROW)
        {
        DbECSchemaEntry key;
        key.m_ecSchemaId = stmt->GetValueInt64 (0);
        key.m_schemaName = stmt->GetValueText (1);
        key.m_versionMajor = (uint32_t) stmt->GetValueInt (2);
        key.m_versionMinor = (uint32_t) stmt->GetValueInt (3);
        key.m_nClassesInSchema = (uint32_t) stmt->GetValueInt (4);
        key.m_nClassesLoaded = 0;
        key.m_resolvedECSchema = nullptr;
        schemaKeys.push_back (key);
        }

    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::ResolveECSchemaId (Utf8StringR schemaName, uint32_t& versionMajor, uint32_t& versionMinor, ECSchemaId ecSchemaId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT Name, VersionMajor, VersionMinor FROM ec_Schema WHERE ECSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;
    stmt->BindInt64 (1, ecSchemaId);

    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return stat;

    schemaName = stmt->GetValueText (0);
    versionMajor = (uint32_t) stmt->GetValueInt (1);
    versionMinor = (uint32_t) stmt->GetValueInt (2);
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetECSchemaKeys (DbECSchemaKeysR keys, BeSQLite::Db& db)
    {
    keys.clear ();
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECSchemaId, Name, VersionMajor, VersionMinor, DisplayLabel FROM ec_Schema ORDER BY Name");
    if (BE_SQLITE_OK != stat)
        return stat;

    while ((stat = stmt->Step ()) == BE_SQLITE_ROW)
        {
        keys.push_back (
            DbECSchemaKey (
            stmt->GetValueInt64 (0),
            stmt->GetValueText (1),
            (uint32_t) stmt->GetValueInt (2),
            (uint32_t) stmt->GetValueInt (3),
            stmt->IsColumnNull (4) ? (Utf8CP)nullptr : stmt->GetValueText (4)));
        }
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::DeleteECClass (ECClassId ecClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "DELETE FROM ec_Class WHERE ECClassId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecClassId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::DeleteCustomAttribute (ECContainerId containerId, ECContainerType containerType, ECClassId customAttributeClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "DELETE FROM ec_CustomAttribute WHERE (ContainerId = ? AND ContainerType = ? AND ECClassId = ?)");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, containerId);
    stmt->BindInt (2, static_cast<int>(containerType));
    stmt->BindInt64 (3, customAttributeClassId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::DeleteECClassMap (ECClassId ecClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "DELETE FROM ec_ClassMap WHERE ECClassId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecClassId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::DeleteECSchema (ECSchemaId ecSchemaId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "DELETE FROM ec_Schema WHERE ECSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecSchemaId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::ContainsECSchemaWithId (BeSQLite::Db& db, ECSchemaId ecSchemaId)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECSchemaId FROM ec_Schema WHERE ECSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return false;

    stmt->BindInt64 (1, ecSchemaId);
    return stmt->Step () == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaPersistence::GetECSchemaId (BeSQLite::Db& db, Utf8CP schemaName)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT ECSchemaId FROM ec_Schema WHERE Name = ?");
    if (BE_SQLITE_OK != stat)
        return 0LL;

    stmt->BindText (1, schemaName, Statement::MakeCopy::No);
    stat = stmt->Step();
    if (BE_SQLITE_ROW != stat)
        return 0LL;

    return stmt->GetValueInt64(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
ECSchemaId ECDbSchemaPersistence::GetECSchemaId (BeSQLite::Db& db, ECSchemaCR ecSchema)
    {
    Utf8String schemaName(ecSchema.GetName());
    return GetECSchemaId (db, schemaName.c_str());
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetDerivedECClasses (ECClassIdListR classIds, ECClassId baseClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECClassId FROM ec_BaseClass WHERE BaseECClassId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, baseClassId);
    while ((stat = stmt->Step ()) == BE_SQLITE_ROW)
        classIds.push_back (stmt->GetValueInt64 (0));
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::IsCustomAttributeDefined (BeSQLite::Db& db, ECClassId classId, ECContainerId containerId, ECContainerType containerType)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT COUNT(*) FROM ec_CustomAttribute WHERE ContainerId = ? AND ContainerType = ? AND ECClassId = ?");
    if (BE_SQLITE_OK != stat)
        return false;

    stmt->BindInt64 (1, containerId);
    stmt->BindInt (2, containerType);
    stmt->BindInt64 (3, classId);

    if (stmt->Step () == BE_SQLITE_ROW)
        return stmt->GetValueInt64 (0) > 0;

    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::DeleteECProperty (ECPropertyId propertyId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "DELETE FROM ec_Property WHERE ECPropertyId = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, propertyId);
    return stmt->Step ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetBaseECClasses (ECClassIdListR baseClassIds, ECClassId ecClassId, BeSQLite::Db& db)  // WIP_FNV: take a name, instead... and modify the where clause
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT BaseECClassId FROM ec_BaseClass WHERE ECClassId = ? ORDER BY ECIndex");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, ecClassId);
    while ((stat = stmt->Step ()) == BE_SQLITE_ROW)
        baseClassIds.push_back (stmt->GetValueInt64 (0));
    
    return stat;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetECClassKeys (DbECClassKeysR keys, ECSchemaId schemaId, BeSQLite::Db& db) // WIP_FNV: take a name, instead... and modify the where clause
    {
    keys.clear ();
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECClassId, Name, DisplayLabel FROM ec_Class WHERE ECSchemaId = ? ORDER BY Name");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, schemaId);
    while ((stat = stmt->Step ()) == BE_SQLITE_ROW)
        {
        keys.push_back (
            DbECClassKey (
            stmt->GetValueInt64 (0),
            stmt->GetValueText (1),
            (stmt->IsColumnNull (2) ? (Utf8CP)nullptr : stmt->GetValueText (2))));
        }
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle        04/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
int ECDbSchemaPersistence::ToInt (ECN::ECRelatedInstanceDirection direction)
    {
    switch (direction)
        {
        case ECRelatedInstanceDirection::Forward:
            return 1;
        case ECRelatedInstanceDirection::Backward:
            return 2;
        default:
            BeAssert (false && "ECRelatedInstanceDirection has a new value. ECDbSchemaPersistence::ToInt needs to adopt to it.");
            return -1;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle        04/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
int ECDbSchemaPersistence::ToInt (MapStrategy mapStrategy)
    {
    return static_cast<int> (mapStrategy) + MapStrategyPersistedIntOffset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle        04/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECRelatedInstanceDirection ECDbSchemaPersistence::ToECRelatedInstanceDirection (int relatedInstanceDirection)
    {
    BeAssert ((relatedInstanceDirection == 1 || relatedInstanceDirection == 2) &&"Integer cannot be converted to ECRelatedInstanceDirection.");

    if (relatedInstanceDirection == 2)
        return ECRelatedInstanceDirection::Backward;

    return ECRelatedInstanceDirection::Forward;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Krischan.Eberle        04/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
MapStrategy ECDbSchemaPersistence::ToMapStrategy (int mapStrategy)
    {
    BeAssert (mapStrategy - MapStrategyPersistedIntOffset >= 0);
    return static_cast<MapStrategy> (mapStrategy - MapStrategyPersistedIntOffset);
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
MapStrategy ECDbSchemaPersistence::GetClassMapStrategy (bool* hasMapEntry, ECN::ECClassCR ecClass, BeSQLite::Db& db)
    {
    if (hasMapEntry)
        *hasMapEntry = false;

    if (!ecClass.HasId ())
        {
        auto ecClassId = GetECClassIdBySchemaName (db,
            Utf8String (ecClass.GetSchema ().GetName ().c_str ()).c_str (),
            Utf8String (ecClass.GetName ().c_str ()).c_str ());
        if (ecClassId == 0)
            return MapStrategy::DoNotMap;
        const_cast<ECClassR>(ecClass).SetId (ecClassId);
        }

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT MapStrategy FROM ec_ClassMap WHERE ECClassId = ?");
    if (BE_SQLITE_OK != stat)
        return MapStrategy::DoNotMap;

    stmt->BindInt64 (1, ecClass.GetId ());
    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return MapStrategy::DoNotMap;

    if (hasMapEntry)
        *hasMapEntry = true;

    return ToMapStrategy (stmt->GetValueInt (0));
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetClassesMappedToTable (std::vector<ECClassId>& classIds, DbTableCR table, BeSQLite::Db& db, bool includeRelationshipEndTables)
    {
    BeSQLite::Statement stmt;
    auto stat = stmt.Prepare(db, "SELECT ec_ClassMap.ECClassId, MapStrategy FROM ec_ClassMap  JOIN ec_Class ON ec_ClassMap.ECClassId = ec_Class.ECClassId WHERE MapToDbTable = ? AND ec_Class.IsRelationship = 0");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt.BindText (1, table.GetName (), Statement::MakeCopy::No);

    while ((stat = stmt.Step ()) == BE_SQLITE_ROW)
        {
        auto ecClassId = stmt.GetValueInt64 (0);
        auto mapStrategy = ToMapStrategy (stmt.GetValueInt (1));
        classIds.push_back (ecClassId);
        if (mapStrategy == MapStrategy::TablePerHierarchy)
            {
            stat = GetClassesMappedToParent (classIds, ecClassId, db);
            if (stat != BE_SQLITE_DONE)
                return stat;
            }
        }
    
    if (includeRelationshipEndTables)
        return GetRelationshipEndsMappedToTable (classIds, table, db);

    return stat;
    }
    
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2014
* This work for end table mapping. If a relationship has a class on any end that is mapped
* to the provided table only then the ECClassId of the relationship is retuned.
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetRelationshipEndsMappedToTable (std::vector<ECClassId>& classIds, DbTableCR table, BeSQLite::Db& db)
    {
    BeSQLite::Statement stmt;
    //The following SQL determine the relationship that is mapped using EndTable Mapping, if one of the end table matchs the 
    //table provided to this funtion. 
    auto sql =
        "SELECT DISTINCT RCC.ECClassId"
        "   FROM ec_RelationshipConstraintClass RCC"
        "       JOIN ec_ClassMap EndClassMap          ON EndClassMap.ECClassId          = RCC.RelationECClassId"
        "       JOIN ec_ClassMap RelationshipClassMap ON RelationshipClassMap.ECClassId = RCC.ECClassId"
        "   WHERE EndClassMap.MapToDbTable = ? AND RelationshipClassMap.MapStrategy = 10";

    stmt.Prepare (db, sql);
    stmt.BindText (1, table.GetName (), Statement::MakeCopy::No);
    DbResult r;
    while ((r = stmt.Step ()) == BE_SQLITE_ROW)
        {
        classIds.push_back (stmt.GetValueInt64 (0));
        }

    return r;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::GetClassesMappedToParent (std::vector<ECClassId>& classIds, ECClassId baseClassId, BeSQLite::Db& db)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECClassId FROM ec_ClassMap WHERE MapParentECClassId = ? AND MapStrategy = ?");
    if (BE_SQLITE_OK != stat)
        return stat;

    stmt->BindInt64 (1, baseClassId);
    stmt->BindInt (2, ToInt (MapStrategy::InParentTable));

    while ((stat = stmt->Step ()) == BE_SQLITE_ROW)
        {
        auto ecClassId = stmt->GetValueInt64 (0);
        classIds.push_back (ecClassId);
        stat = GetClassesMappedToParent (classIds, ecClassId, db);
        if (stat != BE_SQLITE_DONE)
            return stat;
        }

    return stat;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSchemaPersistence::IsECSchemaMapped (bool* schemaNotFound, ECN::ECSchemaCR ecSchema, BeSQLite::Db& db)
    {
    if (schemaNotFound)
        *schemaNotFound = true;

    if (!ecSchema.HasId ())
        {
        auto ecSchemaId = GetECSchemaId (db,
            Utf8String (ecSchema.GetName ().c_str ()).c_str ());
        if (ecSchemaId == 0)
            return false;

        const_cast<ECSchemaR>(ecSchema).SetId (ecSchemaId);
        }

    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT COUNT(*) FROM ec_ClassMap INNER JOIN  ec_Class ON ecClass.ECClassId = ec_ClassMap.ECClassId WHERE ec_Class.ECSchemaId = ?");
    if (BE_SQLITE_OK != stat)
        return false;

    stmt->BindInt64 (1, ecSchema.GetId ());
    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return false;

    if (schemaNotFound)
        *schemaNotFound = false;

    return stmt->GetValueInt (0) > 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Casey.Mullen      01/2013
//---------------------------------------------------------------------------------------
//static
ECClassId ECDbSchemaPersistence::GetECClassIdBySchemaName (BeSQLite::Db& db, Utf8CP schemaName, Utf8CP className)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement (stmt, "SELECT ECClassId FROM ec_Class JOIN ec_Schema WHERE ec_Class.ECSchemaId = ec_Schema.ECSchemaId AND ec_Schema.Name = ? AND ec_Class.Name = ?");
    if (BE_SQLITE_OK != stat)
        return 0LL;

    stmt->BindText (1, schemaName, Statement::MakeCopy::No);
    stmt->BindText (2, className, Statement::MakeCopy::No);
    stat = stmt->Step ();
    if (BE_SQLITE_ROW != stat)
        return 0LL;

    return stmt->GetValueInt64 (0);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                          muhammad.zaighum          10/2014
//---------------------------------------------------------------------------------------
//static
ECClassId ECDbSchemaPersistence::GetECClassIdBySchemaNameSpacePrefix(BeSQLite::Db& db, Utf8CP schemaName, Utf8CP className)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT ECClassId FROM ec_Class JOIN ec_Schema WHERE ec_Class.ECSchemaId = ec_Schema.ECSchemaId AND ec_Schema.NamespacePrefix = ? AND ec_Class.Name = ?");
    if (BE_SQLITE_OK != stat)
        return 0LL;

    stmt->BindText(1, schemaName, Statement::MakeCopy::No);
    stmt->BindText(2, className, Statement::MakeCopy::No);
    stat = stmt->Step();
    if (BE_SQLITE_ROW != stat)
        return 0LL;

    return stmt->GetValueInt64(0);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      05/2013
//---------------------------------------------------------------------------------------
ECPropertyId ECDbSchemaPersistence::GetECPropertyId (BeSQLite::Db& db, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName)
    {
    BeSQLite::CachedStatementPtr stmt = nullptr;
    auto stat = db.GetCachedStatement(stmt, "SELECT ECPropertyId FROM ec_Property INNER JOIN ec_Class ON ec_Property.ECClassId = ec_Class.ECClassId INNER JOIN ec_Schema WHERE ec_Class.ECSchemaId = ec_Schema.ECSchemaId AND ec_Schema.Name = ? AND ec_Class.Name = ? AND ec_Property.Name = ?");
    if (BE_SQLITE_OK != stat)
        return 0LL;

    stmt->BindText (1, schemaName, Statement::MakeCopy::No);
    stmt->BindText (2, className, Statement::MakeCopy::No);
    stmt->BindText (3, propertyName, Statement::MakeCopy::No);

    stat = stmt->Step();
    if (BE_SQLITE_ROW != stat)
        return 0LL;

    return stmt->GetValueInt64(0);
    }


//***************************** DbBuffer ************************************************

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbBuffer::~DbBuffer ()
    {
    Reset ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Dettach ()
    {
    m_ownsBuffer = false;
    Reset ();
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Reset ()
    {
    if (m_ownsBuffer && m_data != nullptr)
        free (m_data);

    m_data = nullptr;
    m_length = 0;
    m_ownsBuffer = false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbBuffer::DbBuffer ()
:m_data (nullptr), m_length (0), m_ownsBuffer (false)
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DbBuffer::DbBuffer (size_t length)
    {
    Allocate (length);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbBuffer::Allocate (size_t length)
    {
    Reset ();
    m_data = (Byte*) malloc (length);
    if (m_data != nullptr)
        {
        m_length = length;
        m_ownsBuffer = true;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool  DbBuffer::SetData (Byte* data, size_t length, bool createCopy)
    {
    Reset ();
    if (createCopy == true)
        {
        if (Allocate (length) == true)
            {
            memcpy (m_data, data, length);
            return true;
            }
        else
            return false;
        }
    m_data = data;
    m_length = length;
    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DbBuffer::Resize (size_t length)
    {
    if (m_data != nullptr)
        {
        size_t minLength = length > m_length ? m_length : length;
        Byte* data = (Byte*) malloc (length);
        memcpy (data, m_data, minLength);
        Reset ();
        m_data = data;
        m_ownsBuffer = true;
        m_length = length;
        }
    else
        Allocate (length);
    }

//******************** DbCustomAttributeInfo *******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbCustomAttributeInfo::SerializeCaInstance 
(
IECInstanceR caInstance
)
    {
    Utf8String caXml;
    InstanceWriteStatus serializerStat = caInstance.WriteToXmlString (caXml, 
                    //don't write XML description header as we only store an XML fragment
                    false, 
                    //store instance id for the rare cases where the client specified one.
                    true);

    if (serializerStat != INSTANCE_WRITE_STATUS_Success)
        {
        LOG.errorv (L"Serializing custom attribute instance to XML failed with error code: %d", serializerStat);
        BeAssert (false && L"Serializing custom attribute instance to XML failed.");
        return ERROR;
        }

    m_caInstanceXml = caXml;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbCustomAttributeInfo::DeserializeCaInstance 
(
IECInstancePtr& caInstance,
ECSchemaCR schema
) const
    {
    ECInstanceReadContextPtr readContext = ECInstanceReadContext::CreateContext (schema);
    IECInstancePtr deserializedCa = nullptr;
    InstanceReadStatus deserializeStat = IECInstance::ReadFromXmlString (deserializedCa, GetCaInstanceXml (), *readContext);

    if (deserializeStat != INSTANCE_READ_STATUS_Success)
        {
        LOG.errorv (L"Deserializing custom attribute instance from XML failed with error code: %d", deserializeStat);
        BeAssert (false && L"Deserializing custom attribute instance from XML failed.");
        return ERROR;
        }

    caInstance = deserializedCa;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
void DbCustomAttributeInfo::SetCaInstanceXml 
(
Utf8CP caInstanceXml
)
    {
    m_caInstanceXml = Utf8String (caInstanceXml);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle  11/2012
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP DbCustomAttributeInfo::GetCaInstanceXml 
(
) const
    {
    return m_caInstanceXml.c_str ();
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CustomAttributeTrackerStatus CustomAttributeTracker::Init (bvector<ECSchemaP> const& schemas)
    {
    bvector<ECSchemaP>::const_iterator itor = schemas.begin();
    for (; itor != schemas.end(); ++itor)
        {
        CustomAttributeTrackerStatus status = AddSchema(**itor);
        if (status != CUSTOMATTRIBUTETRACKERSTATUS_Success)
            return status;
        }
    return CUSTOMATTRIBUTETRACKERSTATUS_Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomAttributeTracker::SchemaExist(ECSchemaCR schema)
    {
    bvector<ECSchemaPtr>::const_iterator itor = m_schemas.begin();
    for ( ; itor != m_schemas.end(); itor++)
        if ( (*itor).get() == &schema)
            return true;
    return false;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomAttributeTracker::AddCustomAttributes (IECCustomAttributeContainerCR container)
    {
    ECCustomAttributeInstanceIterable customAttributes = container.GetPrimaryCustomAttributes(false);
    for (ECCustomAttributeInstanceIterable::const_iterator itor = customAttributes.begin(); itor != customAttributes.end(); ++itor)
        {
        IECInstancePtr const & customAttribute = *itor;
        m_caContainerByCA[customAttribute.get()] = &container;
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomAttributeTracker::AddCustomAttributes (ECClassCR relationshipClass, ECRelationshipConstraintCR container)
    {
    ECCustomAttributeInstanceIterable customAttributes = container.GetPrimaryCustomAttributes(false);
    for(ECCustomAttributeInstanceIterable::const_iterator itor = customAttributes.begin(); itor != customAttributes.end(); ++itor)
        {
        IECInstancePtr const & customAttribute = *itor;
        m_caContainerByCA[customAttribute.get()] = &relationshipClass; // We get the CAs from the container, but we store the ECRelationshipClass as the "container", because we will use it to get the id
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomAttributeTracker::TryGetContainer(IECCustomAttributeContainerCP& container, IECInstanceCR customAttribute) // WIP4: could return the actual container object pointer
    {
    container = nullptr;
    CAContainerByCA::const_iterator itor = m_caContainerByCA.find (&customAttribute);
    if (itor != m_caContainerByCA.end())
        {
        container = itor->second;
        return true;
        }
    return false;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CustomAttributeTrackerStatus CustomAttributeTracker::AddSchema (ECSchemaR schema)
    {
    if (SchemaExist(schema))
        return CUSTOMATTRIBUTETRACKERSTATUS_SchemaAlreadyExist;

    AddCustomAttributes (schema);
    for (ECClassCP ecClass : schema.GetClasses())
        {
        AddCustomAttributes(*ecClass);
        for (ECPropertyCP ecProperty : ecClass->GetProperties(false))
            AddCustomAttributes(*ecProperty);
        ECRelationshipClassCP relationshipClass = ecClass->GetRelationshipClassCP();
        if (relationshipClass != nullptr)
            {
            AddCustomAttributes (*ecClass, relationshipClass->GetSource());
            AddCustomAttributes (*ecClass, relationshipClass->GetTarget());
            }
        }
    m_schemas.push_back(&schema);
    return CUSTOMATTRIBUTETRACKERSTATUS_Success;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CustomAttributeTrackerPtr CustomAttributeTracker::Create(CustomAttributeTrackerStatus* status, bvector<ECSchemaP> const& schemas)
    {
    CustomAttributeTrackerP tracker = new CustomAttributeTracker();
    CustomAttributeTrackerStatus r = tracker->Init (schemas); 
    if (status != nullptr)
        *status = r;
    return tracker;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ECDbSchemaPersistence::UpdatePropertyPath (ECDb& db)
    {
    auto sql =
        "WITH RECURSIVE "
        "   TravseECClassStructProperties (parentECClassId, AccessString, LeafECPropertyId, PropertyType, RootECClassId, ProperytPathDepth, ArrayNestingLevel, RootECPropertyId) "
        "   AS ( "
        "       SELECT ECClassId, NULL, NULL, '', ECClassId, 0, 0, NULL  FROM ec_Class "
        "       UNION"
        "       SELECT TypeECStruct,"
        "           (CASE when AccessString IS NOT NULL > 0 THEN AccessString || '.' ELSE '' END) "
        "           || P.Name, "
        "           P.ECPropertyId, "
        "           S.Name, "
        "           RootECClassId, "
        "           ProperytPathDepth + 1, "
        "           ArrayNestingLevel + IsArray, "
        "           (CASE when ProperytPathDepth = 0 THEN P.ECPropertyId ELSE RootECPropertyId END) "
        "       FROM ec_Property P, TravseECClassStructProperties "
        "           INNER JOIN ec_Class C ON C.ECClassId = P.ECClassId "
        "           INNER JOIN ec_Class S ON S.ECClassId = P.TypeECStruct "
        "       WHERE TypeECStruct IS NOT NULL  AND TravseECClassStructProperties.[parentECClassId] = C.ECClassId "
        "       ORDER BY 2 "
        "   ) "
        "SELECT RootECClassId, RootECPropertyId, LeafECPropertyId, AccessString "
        "FROM TravseECClassStructProperties "
        "   INNER JOIN ec_Property ON ec_Property.ECPropertyId = LeafECPropertyId "
        "WHERE ProperytPathDepth > 1 AND ArrayNestingLevel = 1 AND ec_Property.IsArray = 1 "
        "EXCEPT SELECT RootECClassId, RootECPropertyId, LeafECPropertyId, AccessString FROM ec_PropertyAlias";

    Statement newPropertyPathStmt, propertyPathInsertStmt;
    auto stat = newPropertyPathStmt.Prepare (db, sql);
    if (stat != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to prepare CTE query to get new property path that need to be added");
        return stat;
        }

    stat = propertyPathInsertStmt.Prepare (db, "INSERT INTO ec_PropertyAlias (AliasECPropertyId, RootECClassId, RootECPropertyId, LeafECPropertyId, AccessString) VALUES (?, ?, ?, ?, ?)");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to prepare insert query for ec_PropertyAlias");
        return stat;
        }

    while (newPropertyPathStmt.Step () == BE_SQLITE_ROW)
        {
        BeRepositoryBasedId propertyPathId;
        stat = db.GetImplR ().GetECPropertyIdSequence ().GetNextValue (propertyPathId);
        if (stat != BE_SQLITE_OK)
            {
            BeAssert (false && "Failed to generate new aliasECPropertyId");
            return stat;
            }
        stat = propertyPathInsertStmt.Reset ();
        stat = propertyPathInsertStmt.ClearBindings ();
        stat = propertyPathInsertStmt.BindId (1, propertyPathId); //AliasECPropertyId
        stat = propertyPathInsertStmt.BindInt64 (2, newPropertyPathStmt.GetValueInt64 (0)); // RootECClassId
        stat = propertyPathInsertStmt.BindInt64 (3, newPropertyPathStmt.GetValueInt64 (1)); // RootECPropertyId
        stat = propertyPathInsertStmt.BindInt64 (4, newPropertyPathStmt.GetValueInt64 (2)); // LeafECPropertyId
        stat = propertyPathInsertStmt.BindText (5, newPropertyPathStmt.GetValueText (3), Statement::MakeCopy::No); //AccessString
        stat = propertyPathInsertStmt.Step ();
        if (stat != BE_SQLITE_DONE)
            {
            BeAssert (false && "Failed to insert record into ec_PropertyAlias");
            return stat;
            }
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyId ECDbSchemaPersistence::GetECPropertyAlias (ECClassId ecClassId, Utf8CP accessString, BeSQLite::Db& db)
    {
    Statement stmt;
    auto stat = stmt.Prepare (db, "SELECT AliasECPropertyId FROM ec_PropertyAlias WHERE RootECClassId = ? AND AccessString = ?");
    if (stat != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to prepare statement");
        return 0;
        }

    stmt.BindInt64 (1, ecClassId);
    stmt.BindText (2, accessString, Statement::MakeCopy::No);
    if (stmt.Step () == BE_SQLITE_ROW)
        return stmt.GetValueInt64 (0);

    return 0;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
