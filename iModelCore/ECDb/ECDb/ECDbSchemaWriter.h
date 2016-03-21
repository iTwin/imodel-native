/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaWriter.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct SqlUpdater
    {
    private:
        std::map<Utf8String, ECN::ECValue> m_updateMap;
        std::map<Utf8String, ECN::ECValue> m_whereMap;
        Utf8String m_table;
        BentleyStatus BindSet(Statement& stmt, Utf8StringCR column, int i) const
            {
            auto itor = m_updateMap.find(column);
            if (itor == m_updateMap.end())
                return ERROR;

            switch (itor->second.GetPrimitiveType())
                {
                    case PRIMITIVETYPE_Integer:
                        return stmt.BindInt(i, itor->second.GetInteger()) == BE_SQLITE_OK ? SUCCESS : ERROR;
                    case PRIMITIVETYPE_Long:
                        return stmt.BindInt64(i, itor->second.GetLong()) == BE_SQLITE_OK ? SUCCESS : ERROR;
                    case PRIMITIVETYPE_Double:
                        return stmt.BindDouble(i, itor->second.GetDouble()) == BE_SQLITE_OK ? SUCCESS : ERROR;
                    case PRIMITIVETYPE_String:
                        return stmt.BindText(i, itor->second.GetUtf8CP(), Statement::MakeCopy::No) == BE_SQLITE_OK ? SUCCESS : ERROR;
                    case PRIMITIVETYPE_Boolean:
                        return stmt.BindInt(i, itor->second.GetBoolean()) == BE_SQLITE_OK ? SUCCESS : ERROR;
                }

            BeAssert(false && "Unsupported case");
            return ERROR;
            }
        BentleyStatus BindWhere(Statement& stmt, Utf8StringCR column, int i) const
            {
            auto itor = m_whereMap.find(column);
            if (itor == m_whereMap.end())
                return ERROR;

            switch (itor->second.GetPrimitiveType())
                {
                    case PRIMITIVETYPE_Integer:
                        return stmt.BindInt(i, itor->second.GetInteger()) == BE_SQLITE_OK ? SUCCESS : ERROR;
                    case PRIMITIVETYPE_Long:
                        return stmt.BindInt64(i, itor->second.GetLong()) == BE_SQLITE_OK ? SUCCESS : ERROR;
                    case PRIMITIVETYPE_Double:
                        return stmt.BindDouble(i, itor->second.GetDouble()) == BE_SQLITE_OK ? SUCCESS : ERROR;
                    case PRIMITIVETYPE_String:
                        return stmt.BindText(i, itor->second.GetUtf8CP(), Statement::MakeCopy::No) == BE_SQLITE_OK ? SUCCESS : ERROR;
                    case PRIMITIVETYPE_Boolean:
                        return stmt.BindInt(i, itor->second.GetBoolean()) == BE_SQLITE_OK ? SUCCESS : ERROR;
                }

            BeAssert(false && "Unsupported case");
            return ERROR;
            }


    public:
        SqlUpdater(Utf8CP table) : m_table(table) {}
        void Set(Utf8CP column, Utf8CP value)
            {
            m_updateMap[column] = ECN::ECValue(value);
            }
        void Set(Utf8CP column, Utf8StringCR value)
            {
            m_updateMap[column] = ECN::ECValue(value.c_str());
            }
        void Set(Utf8CP column, double value)
            {
            m_updateMap[column] = ECN::ECValue(value);
            }

        void Set(Utf8CP column, bool value)
            {
            m_updateMap[column] = ECN::ECValue(value);
            }
        void Set(Utf8CP column, uint32_t value)
            {
            m_updateMap[column] = ECN::ECValue(static_cast<int64_t>(value));
            }
        void Set(Utf8CP column, uint64_t value)
            {
            m_updateMap[column] = ECN::ECValue(static_cast<int64_t>(value));
            }
        void Set(Utf8CP column, int32_t value)
            {
            m_updateMap[column] = ECN::ECValue(value);
            }
        void Set(Utf8CP column, int64_t value)
            {
            m_updateMap[column] = ECN::ECValue(value);
            }
        void Where(Utf8CP column, int64_t value)
            {
            m_whereMap[column] = ECN::ECValue(value);
            }
        BentleyStatus Apply(ECDb const& ecdb) const
            {
            if (m_updateMap.empty())
                return SUCCESS;

            NativeSqlBuilder sql;
            sql.Append("UPDATE ").AppendEscaped(m_table.c_str()).Append(" SET ");
            bool first = true;
            for (auto& key : m_updateMap)
                {
                if (first)
                    first = false;
                else
                    sql.Append(", ");

                sql.Append(key.first.c_str()).Append(" = ?");
                }

            if (m_whereMap.empty())
                {
                BeAssert(false && "WHERE must not be empty");
                return ERROR;
                }

            first = true;
            for (auto& key : m_whereMap)
                {
                if (first)
                    first = false;
                else
                    sql.Append(" AND ");

                sql.Append(key.first.c_str()).Append(" = ?");
                }

            Statement stmt;
            if (stmt.Prepare(ecdb, sql.ToString()) != BE_SQLITE_OK)
                return ERROR;

            int i = 1;
            for (auto& key : m_updateMap)
                if (BindSet(stmt, key.first, i++) != SUCCESS)
                    return ERROR;
            i = 1;
            for (auto& key : m_whereMap)
                if (BindWhere(stmt, key.first, i++) != SUCCESS)
                    return ERROR;

            return stmt.Step() == BE_SQLITE_DONE ? SUCCESS : ERROR;
            }
    };
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaWriter : NonCopyableClass
    {
private:
    ECDbCR m_ecdb;
    bmap<ECN::ECEnumerationCP, uint64_t> m_enumIdCache;
    BeMutex m_mutex;

    BentleyStatus CreateECSchemaEntry(ECSchemaCR);
    BentleyStatus CreateBaseClassEntry(ECClassId ecClassId, ECClassCR baseClass, int ordinal);
    BentleyStatus CreateECRelationshipConstraintEntry(ECClassId relationshipClassId, ECN::ECRelationshipConstraintR relationshipConstraint, ECRelationshipEnd endpoint);
    BentleyStatus InsertCAEntry(IECInstanceP customAttribute, ECClassId ecClassId, ECContainerId containerId, ECContainerType containerType, int ordinal);
    BentleyStatus CreateECSchemaReferenceEntry(ECSchemaId ecSchemaId, ECSchemaId ecReferencedSchemaId);

    BentleyStatus ImportCustomAttributes(IECCustomAttributeContainerCR sourceContainer, ECContainerId sourceContainerId, ECContainerType containerType, Utf8CP onlyImportCAWithClassName = nullptr);

    BentleyStatus ImportECClass(ECN::ECClassCR);
    BentleyStatus ImportECEnumeration(ECN::ECEnumerationCR);

    BentleyStatus ImportECProperty(ECN::ECPropertyCR, int ordinal);
    BentleyStatus ImportECRelationshipClass(ECN::ECRelationshipClassCP);
    BentleyStatus ImportECRelationshipConstraint(ECClassId relationshipClassId, ECN::ECRelationshipConstraintR, ECRelationshipEnd);
    BentleyStatus EnsureECSchemaExists(ECClassCR);
    BentleyStatus UpdateRelationshipConstraint(SqlUpdater& sqlUpdater, ECRelationshipConstraintChange& constraintChange, ECRelationshipConstraintCR oldConstraint, ECRelationshipConstraintCR newConstraint, Utf8CP constraintType, Utf8CP relationshipName);

    BentleyStatus UpdateClass(ECClassChange& classChange, ECClassCR oldClass, ECClassCR newClass);
    BentleyStatus UpdateProperty(ECPropertyChange& propertyChange, ECPropertyCR oldProperty, ECPropertyCR newProperty);
    BentleyStatus UpdateSchema(ECSchemaChange& schemaChange, ECSchemaCR oldSchema, ECSchemaCR newSchema);
    BentleyStatus Fail(Utf8CP fmt, ...) const
        {
        va_list args;
        va_start(args, fmt);

        Utf8String formattedMessage;
        formattedMessage.VSprintf(fmt, args);

        //Utf8String a;a.VSprintf
        m_ecdb.GetECDbImplR().GetIssueReporter()
            .Report(ECDbIssueSeverity::Error, formattedMessage.c_str());

        return ERROR;
        }
public:
    explicit ECDbSchemaWriter(ECDbCR ecdb) : m_ecdb (ecdb) {}
    BentleyStatus Import(ECSchemaCompareContext& ctx, ECSchemaCR ecSchema);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
