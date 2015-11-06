/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSql.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbSql.h"
#include "ECDbImpl.h"

#define ECDBSQL_NULLTABLENAME "ECDbNotMapped"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************************************************************************************
//EditHandle
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
EditHandle::EditHandle () :m_canEdit (true), m_isModified (false)  {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
bool EditHandle::BeginEdit ()
    {
    if (CanEdit () || IsModified ())
        {
        BeAssert (false && "Already in edit mode");
        return false;
        }

    m_canEdit = true;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
bool EditHandle::IsModified () const { return m_isModified; }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
bool EditHandle::SetModified ()
    {
    if (AssertNotInEditMode ())
        return false;

    if (!m_isModified)
        m_isModified = true;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
bool EditHandle::EndEdit ()
    {
    if (!CanEdit ())
        {
        BeAssert (false && "Not in edit mode");
        return false;
        }

    m_canEdit = false;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
bool EditHandle::CanEdit () const{ return m_canEdit; }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
bool EditHandle::AssertNotInEditMode ()
    {
    if (!CanEdit ())
        {
        BeAssert (false && "Require object to be in edit mode. Call editHandle.BeginEdit() to enable it");
        return true;
        }

    return false;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
bool EditHandle::AssertInEditMode () const
    {
    if (CanEdit ())
        {
        BeAssert (false && "Object must not be in edit mode");
        return true;
        }

    return false;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
EditHandle::EditHandle (EditHandle const &&  eh)
    : m_canEdit (eh.m_canEdit), m_isModified (eh.m_isModified)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
EditHandle::~EditHandle (){ /*AssertInEditMode ();*/ }


//****************************************************************************************
//ECDbSqlTable
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::weak_ptr<ECDbSqlColumn> ECDbSqlTable::GetColumnWeakPtr (Utf8CP name) const
    {
    auto itor = m_columns.find (name);
    if (itor != m_columns.end ())
        {
        return itor->second;
        }

    return std::weak_ptr<ECDbSqlColumn> ();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTable::GetFilteredColumnList(std::vector<ECDbSqlColumn const*>& columns, PersistenceType persistenceType) const
    {
    for (auto column : m_orderedColumns)
        {
        if (column->GetPersistenceType() == persistenceType)
            columns.push_back(column);
        }

    return columns.empty() ? ERROR : SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTable::GetFilteredColumnList(std::vector<ECDbSqlColumn const*>& columns, ColumnKind kind) const
    {
    for (auto column : m_orderedColumns)
        {
        if (Enum::Intersects(column->GetKind(), kind))
            columns.push_back(column);
        }

    return columns.empty() ? ERROR : SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlColumn const* ECDbSqlTable::GetFilteredColumnFirst (ColumnKind kind) const
    {
    for (auto column : m_orderedColumns)
        {
        if (Enum::Intersects(column->GetKind(), kind))
            return column;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlPrimaryKeyConstraint* ECDbSqlTable::GetPrimaryKeyConstraint (bool createIfDonotExist)
    {
    for (auto& constraint : m_constraints)
    if (constraint->GetType () == ECDbSqlConstraint::Type::PrimaryKey)
        return static_cast<ECDbSqlPrimaryKeyConstraint*>(constraint.get ());

    if (createIfDonotExist)
        {
        if (GetEditHandleR ().AssertNotInEditMode ())
            return nullptr;

        ECDbSqlPrimaryKeyConstraint * constraint = new ECDbSqlPrimaryKeyConstraint (*this);
        m_constraints.push_back (std::unique_ptr<ECDbSqlConstraint> (constraint));
        return constraint;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlForeignKeyConstraint* ECDbSqlTable::CreateForeignKeyConstraint (ECDbSqlTable const& targetTable)
    {
    if (GetEditHandleR ().AssertNotInEditMode ())
        return nullptr;

    ECDbSqlForeignKeyConstraint * constraint = new ECDbSqlForeignKeyConstraint (*this, targetTable, GetDbDefR ().GetManagerR ().GetIdGenerator ().NextConstraintId ());
    m_constraints.push_back (std::unique_ptr<ECDbSqlConstraint> (constraint));
    return constraint;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::vector<ECDbSqlConstraint const*> ECDbSqlTable::GetConstraints () const
    {
    std::vector<ECDbSqlConstraint const*> constraints;
    for (auto const& constraint : m_constraints)
        constraints.push_back (constraint.get ());

    return constraints;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlColumn* ECDbSqlTable::CreateColumn (Utf8CP name, ECDbSqlColumn::Type type, ColumnKind kind, PersistenceType persistenceType)
    {
    return CreateColumn (name, type, m_orderedColumns.size (), kind, persistenceType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                          muhammad.zaighum                           01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTable::CreateTrigger(Utf8CP triggerName, ECDbSqlTable& table, Utf8CP condition, Utf8CP body, TriggerType ecsqlType,TriggerSubType triggerSubType)
    {
    if (m_triggers.find(triggerName) == m_triggers.end())
        {
        m_triggers[triggerName] = std::unique_ptr<ECDbSqlTrigger>(new ECDbSqlTrigger(triggerName, table, condition, body, ecsqlType, triggerSubType));
        return SUCCESS;
        }
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::vector<const ECDbSqlTrigger*> ECDbSqlTable::GetTriggers()const
    {
    std::vector<const ECDbSqlTrigger*> triggers;
    for (auto &trigger : m_triggers)
        {
        triggers.push_back(trigger.second.get());
        }
    return triggers;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
size_t ECDbSqlTable::IndexOf(ECDbSqlColumn const& column) const
    {
    BeAssert(&(column.GetTable()) == this);
    return std::distance(m_orderedColumns.begin(), std::find(m_orderedColumns.begin(), m_orderedColumns.end(), &column));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlColumn* ECDbSqlTable::CreateColumn (Utf8CP name, ECDbSqlColumn::Type type, size_t position, ColumnKind kind, PersistenceType persistenceType)
    {
    if (GetEditHandleR ().AssertNotInEditMode ())
        return nullptr;

    auto resolvePersistenceType = persistenceType;
    if (GetPersistenceType () == PersistenceType::Virtual)
        {
        //BeAssert (false && "Cannot create persisted columns in none persisted table");
        resolvePersistenceType = PersistenceType::Virtual;
        }

    std::shared_ptr<ECDbSqlColumn> newColumn;
    if (name)
        {
        if (FindColumnCP (name))
            {
            BeAssert (false && "Column name already exist");
            return nullptr;
            }

        newColumn = std::make_shared<ECDbSqlColumn> (name, type, *this, resolvePersistenceType, GetDbDefR ().GetManagerR ().GetIdGenerator ().NextColumnId ());
        }
    else
        {
        Utf8String generatedName;
        do
            {
            m_nameGeneratorForColumn.Generate (generatedName);
            } while (FindColumnCP (generatedName.c_str ()));

            newColumn = std::make_shared<ECDbSqlColumn> (generatedName.c_str (), type, *this, resolvePersistenceType, GetDbDefR ().GetManagerR ().GetIdGenerator ().NextColumnId ());
        }

    newColumn->SetKind (kind);
    m_orderedColumns.insert (m_orderedColumns.begin() + position, newColumn.get());
    m_columns[newColumn->GetName ().c_str ()] = newColumn;

    for (auto& eh: m_columnEvents)
        eh (ColumnEvent::Created, *newColumn);

    return newColumn.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ECDbSqlTable::DeleteColumn (Utf8CP name)
    {
    if (GetEditHandleR ().AssertNotInEditMode ())
        return false;

    auto itor = m_columns.find (name);
    if (itor != m_columns.end ())
        {
        for (auto& constraint : m_constraints)
            {
            if (constraint->GetType () == ECDbSqlConstraint::Type::ForeignKey)
                {
                auto fkc = static_cast<ECDbSqlForeignKeyConstraint*>(constraint.get ());
                fkc->Remove (itor->second->GetName ().c_str (), nullptr);
                }
            else if (constraint->GetType () == ECDbSqlConstraint::Type::PrimaryKey)
                {
                auto pkc = static_cast<ECDbSqlPrimaryKeyConstraint*>(constraint.get ());
                pkc->Remove (itor->second->GetName ().c_str ());
                }
            }

        for (auto& eh : m_columnEvents)
            eh (ColumnEvent::Deleted, *itor->second);

        m_orderedColumns.erase (std::find_if (m_orderedColumns.begin (), m_orderedColumns.end (), [itor] (ECDbSqlColumn const* column){return column == itor->second.get (); }));
        m_columns.erase (itor);
        return true;
        }

    return false;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlColumn const* ECDbSqlTable::FindColumnCP (Utf8CP name) const
    {
    auto itor = m_columns.find (name);
    if (itor != m_columns.end ())
        return itor->second.get ();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlColumn* ECDbSqlTable::FindColumnP (Utf8CP name) const
    {
    auto itor = m_columns.find (name);
    if (itor != m_columns.end ())
        return itor->second.get ();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::vector<ECDbSqlColumn const*> const& ECDbSqlTable::GetColumns () const
    {
    return m_orderedColumns;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ECDbSqlTable::TryGetECClassIdColumn (ECDbSqlColumn const*& classIdCol) const
    {
    if (!m_isClassIdColumnCached)
        {
        m_classIdColumn = GetFilteredColumnFirst(ColumnKind::ECClassId);
        m_isClassIdColumnCached = true;
        }

    classIdCol = m_classIdColumn;
    return m_classIdColumn != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTable::PersistenceManager::CreateOrUpdate(ECDbR ecdb) const
    {
    auto name = m_table.GetName().c_str();
    auto type = DbMetaDataHelper::GetObjectType(ecdb, name);
    if (type == DbMetaDataHelper::ObjectType::None)
        return Create(ecdb);

    if (m_table.GetPersistenceType() == PersistenceType::Virtual)
        return ERROR;

    //! Object type is view and exist in db. Action = DROP it and recreate it.
    if (type == DbMetaDataHelper::ObjectType::View)
        {
        Utf8String sql("DROP VIEW ");
        sql.append(name);
        auto r = ecdb.ExecuteSql(sql.c_str());
        if (r != BE_SQLITE_OK)
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to drop view '%s'", name);
            return ERROR;
            }

        return Create(ecdb);
        }

    BeAssert(type == DbMetaDataHelper::ObjectType::Table);

    bvector<Utf8String> existingDbColumns;
    if (!ecdb.GetColumns(existingDbColumns, name))
        {
        BeAssert(false && "Failed to get column list for table");
        return ERROR;
        }

    //Create a fast hash set of existing db column list
    std::set<Utf8String> existingColumnSet;
    for (auto& existingDbColumn : existingDbColumns)
        {
        existingColumnSet.insert(existingDbColumn);
        }

    //Create a fast hash set of in-memory column list;
    std::vector<ECDbSqlColumn const*>  persistedColumns;
    m_table.GetFilteredColumnList(persistedColumns, PersistenceType::Persisted);

    std::map<Utf8CP, ECDbSqlColumn const*, CompareIUtf8> modifiedColumnSet;

    for (auto column : persistedColumns)
        {
        modifiedColumnSet[column->GetName().c_str()] = column;
        }

    std::vector<Utf8CP> newColumns;
    //compute new columns;
    for (auto const& modifiedColumn : modifiedColumnSet)
        {
        if (existingColumnSet.find(modifiedColumn.first) == existingColumnSet.end())
            newColumns.push_back(modifiedColumn.first);
        }

    for (auto const& existingDbColumn : existingDbColumns)
        {
        if (modifiedColumnSet.find(existingDbColumn.c_str()) == modifiedColumnSet.end())
            {
            BeAssert(false && "This can only happen during schema upgrade which ECDb doesn't support yet in 06 at all. So code should never end up here.");
            return ERROR;
            }
        }

    if (SUCCESS != DDLGenerator::AddColumns(ecdb, m_table, newColumns))
        return ERROR;

    return CreateTriggers(ecdb, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTable::PersistenceManager::Create(ECDbR ecdb) const
    {
    if (!m_table.IsValid())
        {
        BeAssert(false && "Table definition is not valid");
        return ERROR;
        }

    auto sql = DDLGenerator::GetCreateTableDDL(m_table, DDLGenerator::CreateOption::Create);
    if (ecdb.ExecuteSql(sql.c_str()) != BE_SQLITE_OK)
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to create table %s: %s",
                                          m_table.GetName().c_str(), ecdb.GetLastError().c_str());
        return ERROR;
        }

    return CreateTriggers(ecdb, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle    06/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTable::PersistenceManager::CreateTriggers(ECDbR ecdb, bool failIfExists) const
    {
    for (ECDbSqlTrigger const* trigger : m_table.GetTriggers())
        {
        if (trigger->ExistsInDb(ecdb))
            {
            if (failIfExists)
                {
                ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Trigger %s already exists on table %s.", trigger->GetName().c_str(), m_table.GetName().c_str());
                return ERROR;
                }

            continue;
            }

        if (SUCCESS != trigger->CreateInDb(ecdb))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ECDbSqlTable::PersistenceManager::Exist (ECDbR ecdb) const
    {
    return ecdb.TableExists (m_table.GetName ().c_str ());
    }


//****************************************************************************************
//ECDbSqlDb
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ECDbMapDb::Reset ()
    {
    m_tables.clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ECDbMapDb::IsNameInUse(Utf8CP name) const
    {
    return m_tables.find(name) != m_tables.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapDb::DropTable (Utf8CP name)
    {
    auto itor = m_tables.find (name);
    if (itor == m_tables.end ())
        {
        BeAssert (false && "Fail to find table");
        return BentleyStatus::ERROR;
        }

    m_tables.erase (itor);
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlTable* ECDbMapDb::CreateTable (Utf8CP name, PersistenceType type)
    {
    ECDbSqlTable* newTableDef;
    if (!Utf8String::IsNullOrEmpty (name))
        {
        if (FindTable (name))
            {
            BeAssert (false && "Table with same name already exists");
            return nullptr;
            }

        newTableDef = new ECDbSqlTable (name, *this, GetManagerR ().GetIdGenerator ().NextTableId (), type, OwnerType::ECDb);
        }
    else
        {
        Utf8String generatedName;
        do
            {
            m_nameGenerator.Generate (generatedName);
            } while (FindTable (generatedName.c_str ()));

            newTableDef = new ECDbSqlTable (generatedName.c_str (), *this, GetManagerR ().GetIdGenerator ().NextTableId (), type, OwnerType::ECDb);
        }

    m_tables[newTableDef->GetName ().c_str ()] = std::unique_ptr<ECDbSqlTable> (newTableDef);
    return newTableDef;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlTable* ECDbMapDb::CreateTableForExistingTableMapStrategy (Utf8CP existingTableName)
    {
    if (Utf8String::IsNullOrEmpty (existingTableName))
        {
        BeAssert (false && "Existing table name cannot be null or empty");
        return nullptr;
        }

    if (FindTable (existingTableName))
        {
        BeAssert (false && "Table with same name already exists");
        return nullptr;
        }

    auto newTableDef = new ECDbSqlTable (existingTableName, *this, GetManagerR ().GetIdGenerator ().NextTableId (), PersistenceType::Persisted, OwnerType::ExistingTable);
    newTableDef->GetEditHandleR ().EndEdit (); //we do not want this table to be editable;
    m_tables[newTableDef->GetName ().c_str ()] = std::unique_ptr<ECDbSqlTable> (newTableDef);
    return newTableDef;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlTable* ECDbMapDb::CreateTableForExistingTableMapStrategy (ECDbCR ecdb, Utf8CP existingTableName)
    {
    if (Utf8String::IsNullOrEmpty (existingTableName))
        {
        BeAssert (false && "Existing table name cannot be null or empty");
        return nullptr;
        }

    if (!ecdb.TableExists(existingTableName))
        {
        LOG.errorv("Table '%s' specified in ClassMap custom attribute must exist if MapStrategy is ExistingTable.", existingTableName);
        return nullptr;
        }

    if (FindTable (existingTableName))
        {
        BeAssert (false && "Table with same name already exists");
        return nullptr;
        }


    ECDbSqlTable* newTableDef = new ECDbSqlTable(existingTableName, *this, GetManagerR().GetIdGenerator().NextTableId(), PersistenceType::Persisted, OwnerType::ExistingTable);
    
    Statement stmt;
    if (stmt.Prepare (ecdb, SqlPrintfString ("PRAGMA table_info('%s')", existingTableName)) != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to prepare statement");
        return nullptr;
        }

    while (stmt.Step () == BE_SQLITE_ROW)
        {
        Utf8CP name = stmt.GetValueText (1);
        Utf8String type = stmt.GetValueText (2);
        type.ToLower();
        type.Trim();
        const bool notnull = stmt.GetValueInt(3) == 1;
        Utf8CP dflt_value = stmt.GetValueText(4);
        const bool isPk = stmt.GetValueInt (5) == 1;

        ECDbSqlColumn::Type ecType = ECDbSqlColumn::Type::Any;
        if (type.rfind ("int64") != Utf8String::npos ||
            type.rfind ("int") != Utf8String::npos ||
            type.rfind("integer") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::Long;
        else if (type.rfind("char") != Utf8String::npos ||
                 type.rfind("clob") != Utf8String::npos ||
                 type.rfind("text") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::String;
        else if (type.rfind ("blob") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::Binary;
        else if (type.rfind("real") != Utf8String::npos ||
                 type.rfind("floa") != Utf8String::npos ||
                 type.rfind("doub") != Utf8String::npos)
                 ecType = ECDbSqlColumn::Type::Double;
        else if (type.rfind ("date") != Utf8String::npos ||
                 type.rfind("timestamp") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::DateTime;
        else if (type.rfind ("bool") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::Boolean;

        ECDbSqlColumn* column = newTableDef->CreateColumn (name, ecType);
        if (column == nullptr)
            {
            BeAssert (false && "Failed to create column");
            return nullptr;
            }

        if (!Utf8String::IsNullOrEmpty (dflt_value))
            column->GetConstraintR ().SetDefaultExpression (dflt_value);

        column->GetConstraintR ().SetIsNotNull (notnull);
        if (isPk)
            newTableDef->GetPrimaryKeyConstraint ()->Add (name);
        }
    
    newTableDef->GetEditHandleR ().EndEdit (); //we do not want this table to be editable;
    m_tables[newTableDef->GetName ().c_str ()] = std::unique_ptr<ECDbSqlTable> (newTableDef);
    return newTableDef;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlTable const* ECDbMapDb::FindTable (Utf8CP name) const
    {
    auto itor = m_tables.find (name);
    if (itor != m_tables.end ())
        return itor->second.get ();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlTable* ECDbMapDb::FindTableP (Utf8CP name) const
    {
    auto itor = m_tables.find (name);
    if (itor != m_tables.end ())
        return itor->second.get ();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
const std::vector<ECDbSqlTable const*> ECDbMapDb::GetTables () const
    {
    std::vector<ECDbSqlTable const*> tables;
    for (auto const& key : m_tables)
        tables.push_back (key.second.get ());

    return tables;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
const std::vector<ECDbSqlTable*> ECDbMapDb::GetTablesR ()
    {
    std::vector<ECDbSqlTable*> tables;
    for (auto const& key : m_tables)
        tables.push_back (key.second.get ());

    return tables;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ECDbMapDb::IsModified () const
    {
    for (auto& key : m_tables)
        {
        if (key.second->GetEditHandle ().IsModified ())
            return true;
        }

    return false;
    }

//****************************************************************************************
//ECDbSqlPrimaryKeyConstraint
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlPrimaryKeyConstraint::Add (Utf8CP columnName)
    {
    return InsertOrReplace (columnName, m_columns.size ());
    }

BentleyStatus ECDbSqlPrimaryKeyConstraint::InsertOrReplace (Utf8CP columnName, size_t position)
    {
    auto column = GetTable ().FindColumnCP (columnName);
    BeAssert (column != nullptr);
    if (column == nullptr)
        return BentleyStatus::ERROR;

    for (auto itor = m_columns.begin (); itor != m_columns.end (); ++itor)
        {
        if ((*itor)->GetName ().EqualsI (columnName))
            {
            m_columns.erase (itor);
            break;
            }
        }

    if (column->GetPersistenceType () == PersistenceType::Virtual)
        {
        BeAssert (false && "Virtual column cannot be use as primary key");
        return BentleyStatus::ERROR;
        }

    m_columns.insert ((m_columns.begin () + position), column);
    return BentleyStatus::SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlPrimaryKeyConstraint::Remove (Utf8CP columnName)
    {
    for (auto itor = m_columns.begin (); itor != m_columns.end (); ++itor)
    if ((*itor)->GetName ().EqualsI (columnName))
        {
        m_columns.erase (itor);
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//****************************************************************************************
//ECDbSqlForeignKeyConstraint
//****************************************************************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSqlForeignKeyConstraint::ContainsInSource (Utf8CP columnName) const
    {
    return std::find_if (m_sourceColumns.begin (), m_sourceColumns.end (), [columnName] (ECDbSqlColumn const* column){ return column->GetName ().EqualsI (columnName); }) != m_sourceColumns.end ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSqlForeignKeyConstraint::ContainsInTarget (Utf8CP columnName) const
    {
    return std::find_if (m_targetColumns.begin (), m_targetColumns.end (), [columnName] (ECDbSqlColumn const* column){ return column->GetName ().EqualsI (columnName); }) != m_targetColumns.end ();
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
//static
Utf8CP ECDbSqlForeignKeyConstraint::ToSQL (ForeignKeyActionType actionType)
    {
    switch (actionType)
        {
        case ForeignKeyActionType::Cascade:
            return "CASCADE";
        case ForeignKeyActionType::NoAction:
            return "NO ACTION";
        case ForeignKeyActionType::Restrict:
            return "RESTRICT";
        case ForeignKeyActionType::SetDefault:
            return "SET DEFAULT";
        case ForeignKeyActionType::SetNull:
            return "SET NULL";
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ForeignKeyActionType ECDbSqlForeignKeyConstraint::ToActionType(Utf8CP str)
    {
    if (BeStringUtilities::Stricmp(str, "Cascade") == 0)
        return ForeignKeyActionType::Cascade;

    if (BeStringUtilities::Stricmp(str, "SetNull") == 0)
        return ForeignKeyActionType::SetNull;

    if (BeStringUtilities::Stricmp(str, "SetDefault") == 0)
        return ForeignKeyActionType::SetDefault;

    if (BeStringUtilities::Stricmp(str, "Restrict") == 0)
        return ForeignKeyActionType::Restrict;

    return ForeignKeyActionType::NoAction;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlForeignKeyConstraint::Remove (Utf8CP sourceColumn, Utf8CP targetColumn)
    {
    bool cont;
    bool hasSouceColumn = !Utf8String::IsNullOrEmpty (sourceColumn);
    bool hasTargetColumn = !Utf8String::IsNullOrEmpty (targetColumn);
    auto size = m_sourceColumns.size ();
    do{
        cont = false;
        for (size_t i = 0; i < m_sourceColumns.size (); i++)
            {
            if (hasSouceColumn && !hasTargetColumn)
                {
                if (m_sourceColumns[i]->GetName () == sourceColumn)
                    {
                    Remove (i);
                    cont = true;
                    break;
                    }
                }
            else if (!hasSouceColumn && hasTargetColumn)
                {
                if (m_targetColumns[i]->GetName () == targetColumn)
                    {
                    Remove (i);
                    cont = true;
                    break;
                    }
                }
            else if (hasSouceColumn && hasTargetColumn)
                {
                if (m_sourceColumns[i]->GetName () == sourceColumn && m_targetColumns[i]->GetName () == targetColumn)
                    {
                    Remove (i);
                    cont = true;
                    break;
                    }
                }
            }
        } while (cont);

    return size != m_sourceColumns.size () ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlForeignKeyConstraint::Add (Utf8CP sourceColumn, Utf8CP targetColumn)
    {
    if (Utf8String::IsNullOrEmpty (sourceColumn) || Utf8String::IsNullOrEmpty (targetColumn))
        {
        BeAssert (false && "Source and target column cannot be empty of null");
        return BentleyStatus::ERROR;
        }

    auto source = GetSourceTable ().FindColumnCP (sourceColumn);
    auto target = GetTargetTable ().FindColumnCP (targetColumn);

    if (source == nullptr || target == nullptr)
        {
        BeAssert(source != nullptr && "Failed to resolve sourceColumn in sourceTable while creating foreignKey constraint");
        BeAssert(target != nullptr && "Failed to resolve targetColumn in targetTable while creating foreignKey constraint");
        return BentleyStatus::ERROR;
        }

    if (source->GetPersistenceType () == PersistenceType::Virtual)
        {
        BeAssert (false && "Virtual column cannot be use as foreign key");
        return BentleyStatus::ERROR;
        }

    if (target->GetPersistenceType () == PersistenceType::Virtual)
        {
        BeAssert (false && "Virtual column cannot be use as foreign key");
        return BentleyStatus::ERROR;
        }
   
    for (size_t i = 0; i < m_sourceColumns.size (); i++)
        {
        if (m_sourceColumns[i]->GetName ().EqualsI (sourceColumn) && m_targetColumns[i]->GetName ().EqualsI (targetColumn))
            {
            return BentleyStatus::SUCCESS;
            }
        }

    m_sourceColumns.push_back (source);
    m_targetColumns.push_back (target);
        
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlForeignKeyConstraint::Remove (size_t index)
    {
    if (index >= m_sourceColumns.size ())
        return BentleyStatus::ERROR;

    m_sourceColumns.erase (m_sourceColumns.begin () + index);
    m_targetColumns.erase (m_targetColumns.begin () + index);
    return BentleyStatus::SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTrigger::CreateInDb (ECDbR ecdb) const
    {
    Utf8String sql = DDLGenerator::GetCreateTriggerDDL(*this);
    if (ecdb.ExecuteSql(sql.c_str()) != BE_SQLITE_OK)
        {
        LOG.errorv("Failed to create trigger %s on table %s. Error: %s", m_triggerName.c_str(), m_table.GetName().c_str(),
                   ecdb.GetLastError().c_str());
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ECDbSqlTrigger::ExistsInDb(ECDbR ecdb) const
    {
    CachedStatementPtr stmt = nullptr;
    if (BE_SQLITE_OK != ecdb.GetCachedStatement(stmt, "select NULL from sqlite_master WHERE type='trigger' and name=?"))
        {
        BeAssert(false);
        return false;
        }

    stmt->BindText(1, GetName().c_str(), Statement::MakeCopy::No);
    return BE_SQLITE_ROW == stmt->Step();
    }

//****************************************************************************************
// DDLGenerator
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DDLGenerator::CopyRows(ECDbR ecdb, Utf8CP sourceTable, bvector<Utf8String>& sourceColumns, Utf8CP targetTable, bvector<Utf8String>& targetColumns)
    {
    Utf8String sourceColumnList;
    for (auto& column : sourceColumns)
        {
        sourceColumnList.append ("[");
        sourceColumnList.append (column);
        sourceColumnList.append ("]");
        if (column != *(sourceColumns.end () - 1))
            sourceColumnList.append (",");
        }

    Utf8String targetColumnList;
    for (auto& column : targetColumns)
        {
        targetColumnList.append ("[");
        targetColumnList.append (column);
        targetColumnList.append ("]");
        if (column != *(targetColumns.end () - 1))
            targetColumnList.append (",");
        }

    Utf8String sql;
    sql.Sprintf ("INSERT INTO [%s] (%s) SELECT %s FROM %s", targetTable, targetColumnList.c_str (), sourceColumnList.c_str (), sourceTable);
    if (BE_SQLITE_OK != ecdb.ExecuteSql(sql.c_str()))
        {
        ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to copy rows from %s to %s: Error message: %s",
                                          sourceTable, targetTable, ecdb.GetLastError().c_str());
        return ERROR;
        }


    if (LOG.isSeverityEnabled (NativeLogging::LOG_TRACE))
        LOG.tracev("Copied rows from %s to %s.", sourceTable, targetTable);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DDLGenerator::AddColumns(ECDbR ecdb, ECDbSqlTable const& table, std::vector<Utf8CP> const& newColumns)
    {
    if (newColumns.empty ())
        return SUCCESS;

    Utf8String alterDDLTemplate;
    alterDDLTemplate.Sprintf ("ALTER TABLE [%s] ADD COLUMN ", table.GetName ().c_str ());
    for (auto newColumn : newColumns)
        {
        //Limitation of ADD COLUMN http://www.sqlite.org/lang_altertable.html
        auto alterDDL = alterDDLTemplate;
        auto column = table.FindColumnCP (newColumn);
        if (column == nullptr)
            {
            BeAssert (false && "Expecting a column that exist in table definition");
            return ERROR;
            }

        alterDDL.append (GetColumnDDL (*column));
        DbResult r = ecdb.ExecuteSql (alterDDL.c_str ());
        if (r != BE_SQLITE_OK)
            {
            ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Failed to add new column (%s). Error message: %s", alterDDLTemplate.c_str(), ecdb.GetLastError ().c_str());
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                       muhammad.zaighum        1/2015
//---------------------------------------------------------------------------------------
//static 
Utf8String DDLGenerator::GetCreateTriggerDDL(ECDbSqlTrigger const& trigger)
    {
    // CREATE TRIGGER triggerName AFTER UPDATE ON tableName WHEN old.LastMod = new.LastMod BEGIN UPDATE tableName SET LastMod = julianday('now') WHERE Id = new.Id; END

    Utf8String sql = "";
    switch (trigger.GetType())
        {
        case TriggerType::Create:
            sql.append("CREATE ");
            break;
        default:
            BeAssert(false && "Only Create Trigger is supported Yet");
        }
    sql.append("TRIGGER ");
    sql.append(trigger.GetName());
    sql.append(" ");
    switch (trigger.GetSubType())
        {
        case TriggerSubType::After:
            sql.append("AFTER ");
            break;
        case TriggerSubType::Before:
            sql.append("Before ");
        default:
            break;
        }
    sql.append("UPDATE ON ");
    sql.append(trigger.GetTable().GetName());
    sql.append(" ");
    sql.append("WHEN ");
    sql.append(trigger.GetCondition());
    sql.append(" ");
    sql.append(trigger.GetBody());
    return sql;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
Utf8String DDLGenerator::GetCreateTableDDL (ECDbSqlTable const& o, DDLGenerator::CreateOption option)
    {
    if (o.GetOwnerType () == OwnerType::ExistingTable)
        {
        BeAssert (false && "Existing Table cannot be created");
        return "";
        }

    if (o.GetPersistenceType () == PersistenceType::Virtual)
        {
        BeAssert (false && "Virtual Table cannot be created");
        return "";
        }

    Utf8String sql = "CREATE ";
    if (option == CreateOption::CreateOrReplace)
        sql.append ("OR REPLACE ");

    sql.append ("TABLE ").append ("[").append (o.GetName ()).append ("]");
    // Append body of table
    sql.append (" (");
    
    // Append column definitions
    auto const& columns = o.GetColumns ();
    std::vector<ECDbSqlColumn const*>  persistedColumns;
    
    o.GetFilteredColumnList (persistedColumns, PersistenceType::Persisted);

    if (persistedColumns.empty ())
        {
        BeAssert (false && "Table have no persisted columns");
        return "";
        }

    sql.append (GetColumnsDDL (persistedColumns));
    // Append constraints;
    if (!columns.empty () && !o.GetConstraints ().empty ())
        sql.append (", ");

    sql.append (GetConstraintsDDL (o.GetConstraints ()));
    sql.append (")");
    return sql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
Utf8String DDLGenerator::GetColumnList (std::vector<ECDbSqlColumn const*> const& columns)
    {
    if (columns.empty ())
        {
        BeAssert (false && "There is no columns provided");
        return "";
        }

    Utf8String columnList;
    for (auto itor = columns.begin (); itor != columns.end (); ++itor)
        {
        columnList.append ("[").append ((*itor)->GetName ()).append ("]");
        if (*itor != columns.back ())
            {
            columnList.append (",");
            }
        }

    return columnList;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
Utf8String DDLGenerator::GetForeignKeyConstraintDDL (ECDbSqlForeignKeyConstraint const& constraint)
    {
    if (constraint.GetSourceColumns ().size () != constraint.GetTargetColumns ().size ())
        {
        BeAssert (false && "Key columns does not match");
        return "";
        }

    if (constraint.GetSourceColumns ().size () == 0 || constraint.GetTargetColumns ().size () == 0)
        {
        BeAssert (false && "Key columns are empty");
        return "";
        }

    Utf8String sql = "FOREIGN KEY (";
    sql.append (GetColumnList (constraint.GetSourceColumns ()))
        .append (") REFERENCES [")
        .append (constraint.GetTargetTable ().GetName ())
        .append ("] (")
        .append (GetColumnList (constraint.GetTargetColumns ()))
        .append (")");

   
    if (constraint.GetOnDeleteAction () != ForeignKeyActionType::NotSpecified)
            {
            sql.append (" ON DELETE ");
            sql.append (ECDbSqlForeignKeyConstraint::ToSQL (constraint.GetOnDeleteAction ()));
            }
    
    if (constraint.GetOnUpdateAction () != ForeignKeyActionType::NotSpecified)
            {
            sql.append (" ON UPDATE ");
            sql.append (ECDbSqlForeignKeyConstraint::ToSQL (constraint.GetOnUpdateAction ()));
            }

    return sql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
Utf8String DDLGenerator::GetPrimarykeyConstraintDDL (ECDbSqlPrimaryKeyConstraint const& constraint)
    {
    if (constraint.GetColumns ().size () == 0)
        {
        BeAssert (false && "Key columns are empty");
        return "";
        }

    Utf8String sql = "PRIMARY KEY (";
    sql.append (GetColumnList (constraint.GetColumns ())).append (")");
    return sql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
Utf8String DDLGenerator::GetConstraintsDDL (std::vector<ECDbSqlConstraint const*> const& constraints)
    {
    Utf8String constraintDDL;
    for (auto itor = constraints.begin (); itor != constraints.end (); ++itor)
        {
        auto constraint = (*itor);
        if (constraint->GetType () == ECDbSqlConstraint::Type::ForeignKey)
            {
            constraintDDL.append (GetForeignKeyConstraintDDL (*(static_cast<ECDbSqlForeignKeyConstraint const*>(constraint))));
            }
        else if (constraint->GetType () == ECDbSqlConstraint::Type::PrimaryKey)
            {
            constraintDDL.append (GetPrimarykeyConstraintDDL (*(static_cast<ECDbSqlPrimaryKeyConstraint const*>(constraint))));
            }

        if (*itor != constraints.back ())
            {
            constraintDDL.append (", ");
            }
        }

    return constraintDDL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
Utf8String DDLGenerator::GetColumnsDDL (std::vector<ECDbSqlColumn const*> columns)
    {
    Utf8String columnDDL;
    
    for (auto itor = columns.begin (); itor != columns.end (); ++itor)
        {
        columnDDL.append (GetColumnDDL (**itor));
        if (*itor != columns.back ())
            {
            columnDDL.append (", ");
            }
        }

    return columnDDL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
Utf8String DDLGenerator::GetColumnDDL (ECDbSqlColumn const& o)
    {
    Utf8String sql;
    sql.append ("[").append (o.GetName ()).append ("]").append (" ");
    switch (o.GetType ())
        {
        case ECDbSqlColumn::Type::Any:
            sql.append ("NONE"); break;
        case ECDbSqlColumn::Type::Binary:
            sql.append ("BINARY"); break;
        case ECDbSqlColumn::Type::Boolean:
            sql.append ("BOOLEAN"); break;
        case ECDbSqlColumn::Type::DateTime:
            sql.append ("TIMESTAMP"); break;
        case ECDbSqlColumn::Type::Double:
            sql.append ("DOUBLE"); break;
        case ECDbSqlColumn::Type::Integer:
        case ECDbSqlColumn::Type::Long:
            sql.append ("INTEGER"); break;
        case ECDbSqlColumn::Type::String:
            sql.append ("TEXT"); break;
        }

    if (o.GetConstraint ().IsNotNull())
        sql.append (" ").append ("NOT NULL");

    if (o.GetConstraint ().IsUnique())
        sql.append (" ").append ("UNIQUE");

    if (o.GetConstraint ().GetCollation () != ECDbSqlColumn::Constraint::Collation::Default)
        sql.append (" ").append("COLLATE ").append (ECDbSqlColumn::Constraint::CollationToString (o.GetConstraint ().GetCollation ()));

    if (!o.GetConstraint ().GetDefaultExpression ().empty ())
        sql.append (" ").append ("DEFAULT (").append (o.GetConstraint ().GetDefaultExpression ()).append (")");

    if (!o.GetConstraint ().GetCheckExpression ().empty ())
        sql.append (" ").append ("CHECK ('").append (o.GetConstraint ().GetCheckExpression ()).append ("')");

    return sql;
    }


//****************************************************************************************
//ECDbSqlColumn
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static
const std::map<ColumnKind, Utf8CP> ECDbSqlColumn::s_columnKindNames =
    {
            {ColumnKind::ECInstanceId, "ECInstanceId"},
            {ColumnKind::ECClassId, "ECClassId"},
            {ColumnKind::ECArrayIndex, "ECArrayIndex"},
            {ColumnKind::ECPropertyPathId, "ECPropertyPathId"},
            {ColumnKind::ParentECInstanceId, "ParentECInstanceId"},
            {ColumnKind::SourceECClassId, "SourceECClassId"},
            {ColumnKind::SourceECInstanceId, "SourceECInstanceId"},
            {ColumnKind::TargetECClassId, "TargetECClassId"},
            {ColumnKind::TargetECInstanceId, "TargetECInstanceId"}
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlColumn::SetKind(ColumnKind kind)
    {
    if (GetTableR().GetEditHandleR().AssertNotInEditMode())
        return BentleyStatus::ERROR;

    m_kind = kind;
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlColumn::AddKind(ColumnKind kind)
    {
    return SetKind(Enum::Or(m_kind, kind));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------

const Utf8String ECDbSqlColumn::GetFullName() const { return BuildFullName(GetTable().GetName().c_str(), GetName().c_str()); }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//--------------------------------------------------------------------------------------
//static 
const Utf8String ECDbSqlColumn::BuildFullName(Utf8CP table, Utf8CP column)
    {
    Utf8String str;
    str.append("[").append(table).append("].[").append(column).append("]");
    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
std::weak_ptr<ECDbSqlColumn> ECDbSqlColumn::GetWeakPtr() const
    {
    return GetTable().GetColumnWeakPtr(GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
ECDbSqlColumn::Type ECDbSqlColumn::StringToType (Utf8CP typeName)
    {
    static std::map<Utf8CP, ECDbSqlColumn::Type, CompareIUtf8> s_type
        {
            { "Integer", Type::Integer },
            { "Long", Type::Long },
            { "Double", Type::Double },
            { "DateTime", Type::DateTime },
            { "Binary", Type::Binary },
            { "Boolean", Type::Boolean },
            { "String", Type::String },
            { "Any", Type::Any }

    };
    auto itor = s_type.find (typeName);
    if (itor == s_type.end ())
        return Type::Any;

    return itor->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
Utf8CP ECDbSqlColumn::TypeToString (ECDbSqlColumn::Type type)
    {
    static std::map<Type, Utf8CP> s_type
        {
            { Type::Integer, "Integer" },
            { Type::Long, "Long" },
            { Type::Double, "Double" },
            { Type::DateTime, "DateTime" },
            { Type::Binary, "Binary" },
            { Type::Boolean, "Boolean" },
            { Type::String, "String" },
            { Type::Any, "Any" }

    };

    auto itor = s_type.find (type);
    if (itor == s_type.end ())
        return nullptr;

    return itor->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static
Utf8CP ECDbSqlColumn::KindToString(ColumnKind columnId)
    {
    auto itor = s_columnKindNames.find(columnId);
    if (itor != s_columnKindNames.end())
        {
        return itor->second;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
Utf8CP ECDbSqlColumn::Constraint::CollationToString(ECDbSqlColumn::Constraint::Collation collation)
    {
    static std::map<Collation, Utf8CP> s_type
        {
                {Collation::Default, "Default"},
                {Collation::Binary, "Binary",},
                {Collation::NoCase, "NoCase"},
                {Collation::RTrim, "RTrim"}
        };
    auto itor = s_type.find(collation);
    if (itor == s_type.end())
        return nullptr;

    return itor->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
bool ECDbSqlColumn::Constraint::TryParseCollationString(Collation& collation, Utf8CP str)
    {
    if (Utf8String::IsNullOrEmpty(str))
        {
        collation = Collation::Default;
        return true;
        }

    if (BeStringUtilities::Stricmp(str, "Binary") == 0)
        collation = Collation::Binary;
    else if (BeStringUtilities::Stricmp(str, "NoCase") == 0)
        collation = Collation::NoCase;
    else if (BeStringUtilities::Stricmp(str, "RTrim") == 0)
        collation = Collation::RTrim;
    else
        return false;

    return true;
    }
   
//****************************************************************************************
//ECDbSQLManager
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        1/2015
//---------------------------------------------------------------------------------------
ECDbSQLManager::ECDbSQLManager (ECDbR ecdb)
: m_ecdb (ecdb), m_idGenerator (ecdb), m_defaultDb (*this), m_persistence (ecdb), m_nullTable (nullptr), m_mapStorage (*this), m_loaded (false)
    {
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSQLManager::Load ()
    {
    Reset ();
    auto r = m_persistence.Read (m_defaultDb);
    if (r != BE_SQLITE_OK)
        return BentleyStatus::ERROR;

    m_nullTable = nullptr;
    m_loaded = true;
    return m_mapStorage.Load ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
void ECDbSQLManager::Reset ()
    {
    m_mapStorage.Reset ();
    m_defaultDb.Reset ();
    m_loaded = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
void ECDbSQLManager::SetupNullTable ()
    {
    m_nullTable = m_defaultDb.FindTableP (ECDBSQL_NULLTABLENAME);
    if (!m_nullTable)
        {
        m_nullTable = m_defaultDb.CreateTable (ECDBSQL_NULLTABLENAME, PersistenceType::Virtual);
        }

    if (m_nullTable->GetEditHandleR ().CanEdit ())
        m_nullTable->GetEditHandleR ().EndEdit ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//--------------------------------------------------------------------------------------
ECDbSqlTable const* ECDbSQLManager::GetNullTable () const
    {
    if (!m_nullTable)
        const_cast<ECDbSQLManager*>(this)->SetupNullTable ();

    return m_nullTable;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSQLManager::Save ()
    {
    if (GetECDb ().IsReadonly ())
        {
        BeAssert (false && "Db is readonly");
        return BentleyStatus::ERROR;
        }

    auto r = m_persistence.Insert (m_defaultDb);
    if (r != BE_SQLITE_OK)
        return BentleyStatus::ERROR;

    return m_mapStorage.Save();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
bool ECDbSQLManager::IsTableChanged (ECDbSqlTable const& table) const
    {
    bvector<Utf8String> existingDbColumns;
    if (!m_ecdb.GetColumns (existingDbColumns, table.GetName ().c_str ()))
        {
        LOG.errorv ("Failed to get column list for table %s", table.GetName ().c_str ());
        BeAssert (false && "Failed to get column list for table");
        return true;
        }

    //Create a fast hash set of existing db column list
    std::set<Utf8String> existingColumnSet;
    for (auto& existingDbColumn : existingDbColumns)
        {
        existingColumnSet.insert (existingDbColumn);
        }

    //Create a fast hash set of in-memory column list;
    std::map<Utf8CP, ECDbSqlColumn const*, CompareIUtf8> modifiedColumnSet;
    for (auto column : table.GetColumns ())
        {
        modifiedColumnSet[column->GetName ().c_str ()] = column;
        }

    //compute new columns;
    for (auto const& modifiedColumn : modifiedColumnSet)
        {
        if (existingColumnSet.find (modifiedColumn.first) == existingColumnSet.end ())
            return true;
        }

    //compute deleted columns;
    for (auto const& existingDbColumn : existingDbColumns)
        {
        if (modifiedColumnSet.find (existingDbColumn.c_str ()) == modifiedColumnSet.end ())
            return true;
        }
    return false;

    }

//****************************************************************************************
//StringPool
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
Utf8CP StringPool::Set (Utf8CP str)
    {
    auto v = Get (str);
    if (v == nullptr)
        {
        auto copy = (Utf8P)malloc (strlen (str) + 1);
        if (copy == nullptr)
            return nullptr;

        strcpy (copy, str);
        m_lookUp.insert (copy);
        return copy;
        }

    return v;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
bool StringPool::Exists (Utf8CP accessString) const
    {
    return Get (accessString) != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
Utf8CP StringPool::Get (Utf8CP str) const
    {
    auto itor = m_lookUp.find (str);
    if (itor == m_lookUp.end ())
        return nullptr;

    return *itor;
    }


//****************************************************************************************
//ECDbSqlHelper
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
ECDbSqlColumn::Type ECDbSqlHelper::PrimitiveTypeToColumnType (ECN::PrimitiveType type)
    {
    switch (type)
        {
        case ECN::PrimitiveType::PRIMITIVETYPE_Binary: return ECDbSqlColumn::Type::Binary;
        case ECN::PrimitiveType::PRIMITIVETYPE_Boolean: return ECDbSqlColumn::Type::Boolean;
        case ECN::PrimitiveType::PRIMITIVETYPE_DateTime: return ECDbSqlColumn::Type::DateTime;
        case ECN::PrimitiveType::PRIMITIVETYPE_Double: return ECDbSqlColumn::Type::Double;
        case ECN::PrimitiveType::PRIMITIVETYPE_IGeometry: return ECDbSqlColumn::Type::Binary;
        case ECN::PrimitiveType::PRIMITIVETYPE_Integer: return ECDbSqlColumn::Type::Integer;
        case ECN::PrimitiveType::PRIMITIVETYPE_Long: return ECDbSqlColumn::Type::Long;
        case ECN::PrimitiveType::PRIMITIVETYPE_String: return ECDbSqlColumn::Type::String;
            //case ECN::PrimitiveType::PRIMITIVETYPE_Point2D:
            //case ECN::PrimitiveType::PRIMITIVETYPE_Point3D:
        }

    BeAssert (false && "Type not supported");
    return ECDbSqlColumn::Type::Any;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//--------------------------------------------------------------------------------------
//static 
bool ECDbSqlHelper::IsCompatible (ECDbSqlColumn::Type target, ECDbSqlColumn::Type source)
    {
    switch (source)
        {
        case ECDbSqlColumn::Type::Any:
            switch (target)
                {
                case ECDbSqlColumn::Type::Any:
                    return true;
                }
            break;
        case ECDbSqlColumn::Type::Binary:
            switch (target)
                {
                case ECDbSqlColumn::Type::Any:
                case ECDbSqlColumn::Type::Binary:
                    return true;
                }
            break;
        case ECDbSqlColumn::Type::Boolean:
            switch (target)
                {
                case ECDbSqlColumn::Type::Any:
                case ECDbSqlColumn::Type::Boolean:
                    return true;
                }
            break;
        case ECDbSqlColumn::Type::DateTime:
            switch (target)
                {
                case ECDbSqlColumn::Type::Any:
                case ECDbSqlColumn::Type::DateTime:
                    return true;
                }
            break;
        case ECDbSqlColumn::Type::Double:
            switch (target)
                {
                case ECDbSqlColumn::Type::Any:
                case ECDbSqlColumn::Type::Double:
                    return true;
                }
            break;
        case ECDbSqlColumn::Type::Integer:
            switch (target)
                {
                case ECDbSqlColumn::Type::Any:
                case ECDbSqlColumn::Type::Long:
                case ECDbSqlColumn::Type::Integer:
                    return true;
                }
            break;
        case ECDbSqlColumn::Type::Long:
            switch (target)
                {
                case ECDbSqlColumn::Type::Any:
                case ECDbSqlColumn::Type::Long:
                case ECDbSqlColumn::Type::Integer:
                    return true;
                }
            break;
        case ECDbSqlColumn::Type::String:
            switch (target)
                {
                case ECDbSqlColumn::Type::Any:
                case ECDbSqlColumn::Type::String:
                    return true;
                }
            break;
        }

    return false;
    }

//****************************************************************************************
//ECDbSqlPersistence
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
CachedStatementPtr ECDbSqlPersistence::GetStatement (StatementType type) const
    {
    CachedStatementPtr stmtP;

    DbResult stat = BE_SQLITE_ERROR;

    switch (type)
        {
        case StatementType::SqlInsertTable:
            stat = m_ecdb.GetCachedStatement (stmtP, Sql_InsertTable);
            break;
        case StatementType::SqlInsertColumn:
            stat = m_ecdb.GetCachedStatement (stmtP, Sql_InsertColumn);
            break;
        case StatementType::SqlInsertForeignKey:
            stat = m_ecdb.GetCachedStatement (stmtP, Sql_InsertForeignKey);
            break;
        case StatementType::SqlInsertForeignKeyColumn:
            stat = m_ecdb.GetCachedStatement (stmtP, Sql_InsertForeignKeyColumn);
            break;
        case StatementType::SqlSelectTable:
            stat = m_ecdb.GetCachedStatement (stmtP, Sql_SelectTable);
            break;
        case StatementType::SqlSelectColumn:
            stat = m_ecdb.GetCachedStatement (stmtP, Sql_SelectColumn);
            break;
        case StatementType::SqlSelectForeignKey:
            stat = m_ecdb.GetCachedStatement (stmtP, Sql_SelectForeignKey);
            break;
        case StatementType::SqlSelectForeignKeyColumn:
            stat = m_ecdb.GetCachedStatement (stmtP, Sql_SelectForeignKeyColumn);
            break;
        }

    if (stat != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to prepare statement for inserting table definition");
        return nullptr;
        }
    
    return stmtP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::Read (ECDbMapDb& o)
    {
    IIdGenerator::DisableGeneratorScope disableIdGenerator (o.GetManagerR().GetIdGenerator());

    return ReadTables(o);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::ReadForeignKeys (ECDbMapDb& o)
    {
    for (ECDbSqlTable* table : o.GetTablesR())
        {
        const bool canEdit = table->GetEditHandle ().CanEdit ();
        if (!canEdit)
            table->GetEditHandleR ().BeginEdit ();

        //read foreign key
        const DbResult stat = ReadForeignKeys (*table);
        if (stat != BE_SQLITE_OK)
            return stat;

        if (!canEdit)
            table->GetEditHandleR ().EndEdit ();
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::ReadTables (ECDbMapDb& o)
    {
    auto stmt = GetStatement (StatementType::SqlSelectTable);
    if (stmt.IsNull())
        return BE_SQLITE_ERROR;

    while (stmt->Step () == BE_SQLITE_ROW)
        {
        const DbResult stat = ReadTable (*stmt, o);
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::ReadTable(Statement& stmt, ECDbMapDb& o)
    {
    auto id = stmt.GetValueInt64(0);
    auto name = stmt.GetValueText(1);
    auto ownerType = stmt.GetValueInt(2) == 1 ? OwnerType::ECDb : OwnerType::ExistingTable;
    auto persistenceType = stmt.GetValueInt(3) == 1 ? PersistenceType::Virtual : PersistenceType::Persisted;

    ECDbSqlTable* table = nullptr;
    if (ownerType == OwnerType::ECDb)
        table = o.CreateTable(name, persistenceType);
    else
        table = o.CreateTableForExistingTableMapStrategy(name);

    if (table == nullptr)
        {
        BeAssert(false && "Failed to create table definition");
        return BE_SQLITE_ERROR;
        }

    const bool canEdit = table->GetEditHandle().CanEdit();
    if (!canEdit)
        table->GetEditHandleR().BeginEdit();

    table->SetId(id);
    // Read columns
    const DbResult stat = ReadColumns(*table);
    if (stat != BE_SQLITE_OK)
        return stat;

    if (!canEdit)
        table->GetEditHandleR().EndEdit();

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::ReadColumns (ECDbSqlTable& o)
    {
    auto stmt = GetStatement (StatementType::SqlSelectColumn);
    if (stmt.IsNull())
        return BE_SQLITE_ERROR;

    DbResult stat = stmt->BindInt64(1, o.GetId ());
    if (stat != BE_SQLITE_OK)
        return stat;

    std::map<size_t, ECDbSqlColumn const*> primaryKeys;
    while (stmt->Step () == BE_SQLITE_ROW)
        {
        stat = ReadColumn (*stmt, o, primaryKeys);
        if (stat != BE_SQLITE_OK)
            return stat;
        }
 
    if (!primaryKeys.empty ())
        {
        auto n = o.GetPrimaryKeyConstraint ();
        if (!n)
            {
            BeAssert (false && "Failed to create primarykey definition");
            return BE_SQLITE_ERROR;
            }

        for (auto& kp : primaryKeys)
            {
            n->Add (kp.second->GetName ().c_str ());
            }
        }

    return BE_SQLITE_OK;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::ReadColumn (Statement& stmt, ECDbSqlTable& o , std::map<size_t, ECDbSqlColumn const*>& primaryKeys)
    {
    auto id = stmt.GetValueInt64 (0);
    auto name = stmt.GetValueText (1);
    auto type = static_cast<ECDbSqlColumn::Type>(stmt.GetValueInt (2));
    auto persistenceType = stmt.GetValueInt (3) == 1 ? PersistenceType::Virtual : PersistenceType::Persisted;
    auto constraintNotNull = stmt.GetValueInt (4) == 1;
    auto constraintUnique = stmt.GetValueInt (5) == 1;
    auto constraintCheck = !stmt.IsColumnNull (6) ? stmt.GetValueText (6) : nullptr;
    auto constraintDefault = !stmt.IsColumnNull (7) ? stmt.GetValueText (7) : nullptr;
    auto constraintCollate = static_cast<ECDbSqlColumn::Constraint::Collation>(stmt.GetValueInt (8));
    auto primaryKey_Ordianal = stmt.IsColumnNull (9)? -1 : stmt.GetValueInt (9);
    auto columnKind = Enum::FromInt<ColumnKind>(stmt.GetValueInt (10));

    ECDbSqlColumn* n = o.CreateColumn (name, type, columnKind, persistenceType);
    if (!n)
        {
        BeAssert (false && "Failed to create table definition");
        return BE_SQLITE_ERROR;
        }

    n->SetId (id);
    n->GetConstraintR ().SetIsNotNull (constraintNotNull);
    n->GetConstraintR ().SetIsUnique (constraintUnique);
    n->GetConstraintR ().SetCollation (constraintCollate);

    if (constraintCheck)
        n->GetConstraintR ().SetCheckExpression (constraintCheck);

    if (constraintDefault)
        n->GetConstraintR ().SetDefaultExpression (constraintDefault);

    if (primaryKey_Ordianal > -1)
        {
        primaryKeys[primaryKey_Ordianal] = n;
        }

    return BE_SQLITE_OK;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::ReadForeignKeys (ECDbSqlTable& o)
    {
    auto stmt = GetStatement (StatementType::SqlSelectForeignKey);
    if (stmt.IsNull())
        return BE_SQLITE_ERROR;

    stmt->BindInt64 (1, o.GetId ());
    while (stmt->Step () == BE_SQLITE_ROW)
        {
        const DbResult stat = ReadForeignKey (*stmt, o);
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::ReadForeignKey (Statement& stmt, ECDbSqlTable& o)
    {
    //F.Id, P.Name, R.Name, F.Name, F.OnDelete, F.OnUpdate
    auto id = stmt.GetValueInt64 (0);
    auto referenceTableName = stmt.GetValueText (1);
    auto name = stmt.IsColumnNull (2) ? nullptr : stmt.GetValueText (2);
    auto onDelete = stmt.IsColumnNull (3) ? ForeignKeyActionType::NotSpecified : static_cast<ForeignKeyActionType>(stmt.GetValueInt (3));
    auto onUpdate = stmt.IsColumnNull (4) ? ForeignKeyActionType::NotSpecified : static_cast<ForeignKeyActionType>(stmt.GetValueInt (4));

    auto referenceTable = o.GetDbDef ().FindTable (referenceTableName);
    if (!referenceTable)
        {
        BeAssert (false && "Referenced Table not found");
        return BE_SQLITE_ERROR;
        }

    auto n = o.CreateForeignKeyConstraint (*referenceTable);
    if (!n)
        {
        BeAssert (false && "Failed to create foreign key constraint");
        return BE_SQLITE_ERROR;
        }

    if (name != nullptr)
        n->SetName (name);

    n->SetId (id);
    n->SetOnDeleteAction (onDelete);
    n->SetOnUpdateAction (onUpdate);

    auto cstmt = GetStatement (StatementType::SqlSelectForeignKeyColumn);
    if (cstmt.IsNull ())
        return BE_SQLITE_ERROR;

    cstmt->BindInt64 (1, id);
    while (cstmt->Step () == BE_SQLITE_ROW)
        {
        auto columnL = cstmt->GetValueText (0);
        auto columnR = cstmt->GetValueText (1);

        if (n->Add (columnL, columnR) != BentleyStatus::SUCCESS)
            return BE_SQLITE_ERROR;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::Insert (ECDbMapDb const& db)
    {
    auto tables = db.GetTables ();

    for (auto table : tables)
        {
        DbResult stat = InsertTable (*table);
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    for (auto table : tables)
        {
        for (auto constraint : table->GetConstraints ())
            {
            DbResult stat = InsertConstraint (*constraint);
            if (stat != BE_SQLITE_OK)
                return stat;
            }
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::InsertTable (ECDbSqlTable const& o)
    {
    auto stmt = GetStatement (StatementType::SqlInsertTable);
    if (stmt.IsNull())
        return BE_SQLITE_ERROR;

    stmt->BindInt64 (1, o.GetId ());
    stmt->BindText (2, o.GetName ().c_str (), Statement::MakeCopy::No);
    stmt->BindInt (3, o.GetOwnerType () == OwnerType::ECDb ? 1 : 0);
    stmt->BindInt (4, o.GetPersistenceType () == PersistenceType::Virtual ? 1 : 0);

    auto stat = stmt->Step ();
    if (stat != BE_SQLITE_DONE)
        return stat;

    std::map<ECDbSqlColumn const*, int> primaryKeys; 
    int i = 0;
    if (auto const n = const_cast<ECDbSqlTable&>(o).GetPrimaryKeyConstraint (false))
        {
        for (auto pk : n->GetColumns())
            {
            primaryKeys[pk] = i++;
            }
        }
    for (auto column : o.GetColumns ())
        {
        auto itor = primaryKeys.find (column);
        stat = InsertColumn (*column, (itor == primaryKeys.end() ?  -1 : itor->second));
        if (stat != BE_SQLITE_OK)
            return stat;
        }

    return BE_SQLITE_OK;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::InsertColumn (ECDbSqlColumn const& o, int primaryKeyOrdinal)
    {
    auto stmt = GetStatement (StatementType::SqlInsertColumn);
    if (stmt.IsNull())
        return BE_SQLITE_ERROR;

    stmt->BindInt64 (1, o.GetId ());
    stmt->BindInt64 (2, o.GetTable ().GetId ());
    stmt->BindText (3, o.GetName ().c_str (), Statement::MakeCopy::No);
    stmt->BindInt (4, static_cast<int>(o.GetType ()));
    stmt->BindInt (5, o.GetPersistenceType () == PersistenceType::Virtual ? 1 : 0);
    stmt->BindInt64 (6, o.GetTable ().IndexOf (o));
    stmt->BindInt (7, o.GetConstraint ().IsNotNull () ? 1 : 0);
    stmt->BindInt (8, o.GetConstraint ().IsUnique () ? 1 : 0);

    if (!o.GetConstraint ().GetCheckExpression ().empty ())
        stmt->BindText (9, o.GetConstraint ().GetCheckExpression ().c_str (), Statement::MakeCopy::No);

    if (!o.GetConstraint ().GetDefaultExpression ().empty ())
        stmt->BindText (10, o.GetConstraint ().GetDefaultExpression ().c_str (), Statement::MakeCopy::No);

    stmt->BindInt (11, static_cast<int>(o.GetConstraint ().GetCollation ()));
    if (primaryKeyOrdinal > -1)
        stmt->BindInt (12, primaryKeyOrdinal);

    stmt->BindInt (13, Enum::ToInt(o.GetKind ()));
    const DbResult stat = stmt->Step ();
    return stat == BE_SQLITE_DONE ? BE_SQLITE_OK : stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::InsertConstraint (ECDbSqlConstraint const& o)
    {
    if (auto c = dynamic_cast<ECDbSqlForeignKeyConstraint const*>(&o))
        return InsertForeignKey (*c);

    if (dynamic_cast<ECDbSqlPrimaryKeyConstraint const*>(&o) != nullptr)
        return BE_SQLITE_OK;

    return BE_SQLITE_ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbSqlPersistence::InsertForeignKey (ECDbSqlForeignKeyConstraint const& o)
    {
    if (o.Count () == 0)
        {
        BeAssert (false && "ForeignKey constraint does not have any columns");
        return BE_SQLITE_ERROR;
        }

    auto stmt = GetStatement (StatementType::SqlInsertForeignKey);
    if (stmt.IsNull())
        return BE_SQLITE_ERROR;

    stmt->BindInt64 (1, o.GetId ());
    stmt->BindInt64 (2, o.GetTable ().GetId ());
    stmt->BindInt64 (3, o.GetTargetTable ().GetId ());
    if (!o.GetName ().empty ())
        stmt->BindText (4, o.GetName ().c_str (), Statement::MakeCopy::No);

    if (o.GetOnDeleteAction () != ForeignKeyActionType::NotSpecified)
        stmt->BindInt (5, static_cast<int>(o.GetOnDeleteAction ()));

    if (o.GetOnUpdateAction () != ForeignKeyActionType::NotSpecified)
        stmt->BindInt (6, static_cast<int>(o.GetOnUpdateAction ()));

    auto stat = stmt->Step ();
    if (stat != BE_SQLITE_DONE)
        return stat;

    stmt = GetStatement (StatementType::SqlInsertForeignKeyColumn);
    if (stmt.IsNull())
        return BE_SQLITE_ERROR;

    for (size_t i = 0; i < o.Count (); i++)
        {
        stmt->Reset ();
        stmt->ClearBindings ();
        stmt->BindInt64 (1, o.GetId ());
        stmt->BindInt64 (2, o.GetSourceColumns ().at (i)->GetId ());
        stmt->BindInt64 (3, o.GetTargetColumns ().at (i)->GetId ());
        stmt->BindInt64 (4, i);

        stat = stmt->Step ();
        if (stat != BE_SQLITE_DONE)
            return stat;
        }

    return BE_SQLITE_OK;
    }

//****************************************************************************************
//ECDbBriefcaseBaseId
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbTableId ECDbBriefcaseBasedId::_NextTableId ()
    {
    BeBriefcaseBasedId id;
    if (m_ecdb.GetECDbImplR().GetTableIdSequence ().GetNextValue (id) != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to generate new tableId");
        return 0;
        }

    return id.GetValue ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbColumnId ECDbBriefcaseBasedId::_NextColumnId ()
    {
    BeBriefcaseBasedId id;
    if (m_ecdb.GetECDbImplR().GetColumnIdSequence ().GetNextValue (id) != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to generate new columnid");
        return 0;
        }

    return id.GetValue ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbIndexId ECDbBriefcaseBasedId::_NextIndexId ()
    {
    BeBriefcaseBasedId id;
    if (m_ecdb.GetECDbImplR().GetIndexIdSequence ().GetNextValue (id) != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to generate new indexid");
        return 0;
        }

    return id.GetValue ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbConstraintId ECDbBriefcaseBasedId::_NextConstraintId ()
    {
    BeBriefcaseBasedId id;
    if (m_ecdb.GetECDbImplR().GetConstraintIdSequence ().GetNextValue (id) != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to generate new constraintid");
        return 0;
        }

    return id.GetValue ();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbClassMapId ECDbBriefcaseBasedId::_NextClassMapId ()
    {
    BeBriefcaseBasedId id;
    if (m_ecdb.GetECDbImplR().GetClassMapIdSequence ().GetNextValue (id) != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to generate new constraintid");
        return 0;
        }

    return id.GetValue ();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbPropertyPathId ECDbBriefcaseBasedId::_NextPropertyPathId ()
    {
    BeBriefcaseBasedId id;
    if (m_ecdb.GetECDbImplR().GetPropertyMapIdSequence ().GetNextValue (id) != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to generate new constraintid");
        return 0;
        }

    return id.GetValue ();
    }


//****************************************************************************************
//ECDbMapStorage
//****************************************************************************************
ECDbPropertyPath* ECDbMapStorage::Set (std::unique_ptr<ECDbPropertyPath> propertyPath)
    {
    if (m_propertyPaths.find (propertyPath->GetId ()) != m_propertyPaths.end ())
        {
        BeAssert (false && "PropertyPath with same id already exists");
        return nullptr;
        }

    auto p = propertyPath.get ();
    m_propertyPathByPropertyId[p->GetRootPropertyId ()].insert (std::make_pair (p->GetAccessString ().c_str (), p));
    m_propertyPaths[p->GetId ()] = std::move (propertyPath);
    return p;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbClassMapInfo* ECDbMapStorage::Set (std::unique_ptr<ECDbClassMapInfo> classMap)
    {
    if (m_classMaps.find (classMap->GetId ()) != m_classMaps.end ())
        {
        BeAssert (false && "ClassMap with same id already exists");
        return nullptr;
        }

    auto c = classMap.get ();
    auto& vect = m_classMapByClassId[c->GetClassId ()];
    if (c->GetBaseClassMap () == nullptr)
        vect.insert (vect.begin (), c);
    else
        vect.push_back (c);

    m_classMaps[c->GetId ()] = std::move (classMap);
    return c;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbPropertyPath const * ECDbMapStorage::FindPropertyPath (ECN::ECPropertyId rootPropertyId, Utf8CP accessString) const
    {
    auto itorA = m_propertyPathByPropertyId.find (rootPropertyId);
    if (itorA == m_propertyPathByPropertyId.end ())
        return nullptr;

    auto itorB = itorA->second.find (accessString);
    if (itorB == itorA->second.end ())
        return nullptr;

    return itorB->second;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbPropertyPath const* ECDbMapStorage::FindPropertyPath (ECDbPropertyPathId propertyPathId) const
    {
    auto itor = m_propertyPaths.find (propertyPathId);
    if (itor == m_propertyPaths.end ())
        return nullptr;

    return itor->second.get ();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbClassMapInfo const* ECDbMapStorage::FindClassMap (ECDbClassMapId id) const
    {
    auto itor = m_classMaps.find (id);
    if (itor == m_classMaps.end ())
        return nullptr;

    return itor->second.get ();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
std::vector<ECDbClassMapInfo const*> const* ECDbMapStorage::FindClassMapsByClassId (ECN::ECClassId id) const
    {
    auto itor = m_classMapByClassId.find (id);
    if (itor != m_classMapByClassId.end ())
        {
        return &itor->second;
        }

    //BeAssert (false && "Failed to find ECClassId");
    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbPropertyPath* ECDbMapStorage::CreatePropertyPath (ECN::ECPropertyId rootPropertyId, Utf8CP accessString)
    {
    if (FindPropertyPath (rootPropertyId, accessString))
        {
        BeAssert (false && "PropertyPath already exist");
        return nullptr;
        }

    return Set (std::unique_ptr<ECDbPropertyPath> (new ECDbPropertyPath (m_manager.GetIdGenerator().NextPropertyPathId(), rootPropertyId, accessString)));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbClassMapInfo* ECDbMapStorage::CreateClassMap (ECN::ECClassId classId, ECDbMapStrategy const& mapStrategy, ECDbClassMapId baseClassMapId)
    {
    if (baseClassMapId != 0LL)
        {
        if (FindClassMap (baseClassMapId) == nullptr)
            {
            BeAssert (false && "ClassMap already exist");
            return nullptr;
            }
        }
   
    return Set (std::unique_ptr<ECDbClassMapInfo> (new ECDbClassMapInfo (*this, m_manager.GetIdGenerator ().NextClassMapId (), classId, mapStrategy, baseClassMapId)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
CachedStatementPtr ECDbMapStorage::GetStatement (StatementType type)
    {
    CachedStatementPtr stmt;

    DbResult stat = BE_SQLITE_ERROR;
    switch (type)
        {
        case StatementType::SqlDeleteClassMap:
            stat = m_manager.GetECDbR().GetCachedStatement(stmt, Sql_DeleteClassMap);
            break;
        case StatementType::SqlInsertPropertyPath:
            stat = m_manager.GetECDbR ().GetCachedStatement (stmt, Sql_InsertPropertyPath);
            break;
        case StatementType::SqlInsertClassMap:
            stat = m_manager.GetECDbR ().GetCachedStatement (stmt, Sql_InsertClassMap);
            break;
        case StatementType::SqlInsertPropertyMap:
            stat = m_manager.GetECDbR ().GetCachedStatement (stmt, Sql_InsertPropertyMap);
            break;
        case StatementType::SqlSelectPropertyPath:
            stat = m_manager.GetECDbR ().GetCachedStatement (stmt, Sql_SelectPropertyPath);
            break;
        case StatementType::SqlSelectClassMap:
            stat = m_manager.GetECDbR ().GetCachedStatement (stmt, Sql_SelectClassMap);
            break;
        case StatementType::SqlSelectPropertyMap:
            stat = m_manager.GetECDbR ().GetCachedStatement (stmt, Sql_SelectPropertyMap);
            break;

        }
    if (stat != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to prepare statement");
        return nullptr;
        }

    return stmt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapStorage::InsertOrReplace ()
    {
    for (auto& propertyPath : m_propertyPaths)
        {
        const DbResult stat = InsertPropertyPath (*propertyPath.second);
        if (stat != BE_SQLITE_OK)
            {
            BeAssert (false && "Failed to insert propertypath");
            return stat;
            }
        }

    for (auto& classMap : m_classMaps)
        {
        const DbResult stat = InsertClassMap (*classMap.second);
        if (stat != BE_SQLITE_OK)
            {
            BeAssert (false && "Failed to insert classmap");
            return stat;
            }

        }

    for (auto& classMap : m_classMaps)
        {
        for (auto propertyMap : classMap.second->GetPropertyMaps (true))
            {
            const DbResult stat = InsertPropertyMap (*propertyMap);
            if (stat != BE_SQLITE_OK)
                {
                BeAssert (false && "Failed to insert property map");
                return stat;
                }
            }
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapStorage::InsertPropertyMap (ECDbPropertyMapInfo const& o)
    {
    auto stmt = GetStatement (StatementType::SqlInsertPropertyMap);
    if (stmt == nullptr)
        {
        BeAssert (false && "Failed to get statement");
        return BE_SQLITE_ERROR;
        }

    stmt->BindInt64 (1, o.GetClassMap ().GetId ());
    stmt->BindInt64 (2, o.GetPropertyPath ().GetId ());
    stmt->BindInt64 (3, o.GetColumn ().GetId ());
    const DbResult stat = stmt->Step();
    return stat == BE_SQLITE_DONE ? BE_SQLITE_OK : stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapStorage::InsertClassMap (ECDbClassMapInfo const& o)
    {
    //TEMP fix for conceptstation
    auto deleteSt = GetStatement(StatementType::SqlDeleteClassMap);
    deleteSt->BindInt64(1, o.GetId());
    deleteSt->Step();
    //===============================
    
    auto stmt = GetStatement (StatementType::SqlInsertClassMap);
    if (stmt == nullptr)
        {
        BeAssert (false && "Failed to get statement");
        return BE_SQLITE_ERROR;
        }

    stmt->BindInt64 (1, o.GetId ());
    if (o.GetBaseClassMap () == nullptr)
        stmt->BindNull (2);
    else
        stmt->BindInt64 (2, o.GetBaseClassMap ()->GetId ());

    stmt->BindInt64(3, o.GetClassId ());
    stmt->BindInt(4, (int) o.GetMapStrategy ().GetStrategy ());
    stmt->BindInt(5, (int) o.GetMapStrategy().GetOptions());
    stmt->BindInt(6, o.GetMapStrategy().AppliesToSubclasses() ? 1 : 0);

    const DbResult stat = stmt->Step();
    return stat == BE_SQLITE_DONE ? BE_SQLITE_OK : stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapStorage::InsertPropertyPath (ECDbPropertyPath const& o)
    {
    auto stmt = GetStatement (StatementType::SqlInsertPropertyPath);
    if (stmt == nullptr)
        {
        BeAssert (false && "Failed to get statement");
        return BE_SQLITE_ERROR;
        }

    stmt->BindInt64 (1, o.GetId ());
    stmt->BindInt64 (2, o.GetRootPropertyId ());
    stmt->BindText (3, o.GetAccessString ().c_str (), Statement::MakeCopy::No);
    const DbResult stat = stmt->Step ();
    return stat == BE_SQLITE_DONE ? BE_SQLITE_OK : stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapStorage::Read ()
    {
    auto r = ReadPropertyPaths ();
    if (r != BE_SQLITE_OK)
        return r;

    return ReadClassMaps ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapStorage::ReadPropertyMap (ECDbClassMapInfo& o)
    {
    auto stmt = GetStatement (StatementType::SqlSelectPropertyMap);
    if (stmt == nullptr)
        {
        BeAssert (false && "Failed to get statement");
        return BE_SQLITE_ERROR;
        }
    stmt->BindInt64 (1, o.GetId ());
    while (stmt->Step () == BE_SQLITE_ROW)
        {
        auto propertyPathId = stmt->GetValueInt64 (0);
        auto tableName = stmt->GetValueText (1);
        auto columnName = stmt->GetValueText (2);

        auto propertyPath = FindPropertyPath (propertyPathId);
        if (propertyPath == nullptr)
            {
            BeAssert (false && "Failed to resolve property path id");
            return BE_SQLITE_ERROR;
            }

        auto table = m_manager.GetDbSchema ().FindTable (tableName);
        if (table == nullptr)
            {
            BeAssert (false && "Failed to resolve table");
            return BE_SQLITE_ERROR;
            }

        auto column = table->FindColumnCP (columnName);
        if (column == nullptr)
            {
            BeAssert (false && "Failed to resolve column");
            return BE_SQLITE_ERROR;
            }

        auto propertyMap = o.CreatePropertyMap (*propertyPath, *column);
        if (propertyMap == nullptr)
            {
            BeAssert (false && "Failed to create propertyMap");
            return BE_SQLITE_ERROR;
            }
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapStorage::ReadClassMaps ()
    {
    auto stmt = GetStatement (StatementType::SqlSelectClassMap);
    if (stmt == nullptr)
        {
        BeAssert (false && "Failed to get statement");
        return BE_SQLITE_ERROR;
        }

    while (stmt->Step () == BE_SQLITE_ROW)
        {
        ECDbClassMapId id = stmt->GetValueInt64 (0);
        ECDbClassMapId parentId = stmt->IsColumnNull (1) ? 0LL : stmt->GetValueInt64 (1);
        ECN::ECClassId classId = stmt->GetValueInt64 (2);

        ECDbMapStrategy mapStrategy;
        if (SUCCESS != mapStrategy.Assign((ECDbMapStrategy::Strategy) stmt->GetValueInt(3),
                                           (ECDbMapStrategy::Options) stmt->GetValueInt(4),
                                            stmt->GetValueInt(5) == 1))
            {
            BeAssert(false && "Found invalid persistence values for ECDbMapStrategy");
            return BE_SQLITE_ERROR;
            }

        auto classMap = Set (std::unique_ptr<ECDbClassMapInfo> (new ECDbClassMapInfo (*this, id, classId, mapStrategy, parentId)));
        if (classMap == nullptr)
            {
            BeAssert (false && "Failed to create classMap");
            return BE_SQLITE_ERROR;
            }

        if (ReadPropertyMap (*classMap) != BE_SQLITE_OK)
            {
            BeAssert (false && "Failed to resolve property map");
            return BE_SQLITE_ERROR;
            }
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
DbResult ECDbMapStorage::ReadPropertyPaths ()
    {
    auto stmt = GetStatement (StatementType::SqlSelectPropertyPath);
    if (stmt == nullptr)
        {
        BeAssert (false && "Failed to get statement");
        return BE_SQLITE_ERROR;
        }

    while (stmt->Step () == BE_SQLITE_ROW)
        {
        ECDbPropertyPathId id = stmt->GetValueInt64 (0);
        ECPropertyId rootPropertyId = stmt->GetValueInt64 (1);
        Utf8CP accessString = stmt->GetValueText (2);
        auto propertyPath = Set (std::unique_ptr<ECDbPropertyPath> (new ECDbPropertyPath (id, rootPropertyId, accessString)));
        if (propertyPath == nullptr)
            {
            BeAssert (false && "Failed to resolve property path");
            return BE_SQLITE_ERROR;
            }
        }

    return BE_SQLITE_OK;
    }
//****************************************************************************************
//ECDbMapStorage
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbPropertyMapInfo const * ECDbClassMapInfo::FindPropertyMap (Utf8CP columnName) const
    {
    auto& mapSet = GetPropertyMapByColumnName (false);
    auto itor = mapSet.find (columnName);
    if (itor == mapSet.end ())
        return nullptr;

    return itor->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015::
//---------------------------------------------------------------------------------------
const std::vector<ECDbPropertyMapInfo const*>  ECDbClassMapInfo::GetPropertyMaps (bool onlyLocal) const
    {
    std::vector<ECDbPropertyMapInfo const*> r;
    GetPropertyMaps (r, onlyLocal);
    return r;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
const std::map<Utf8CP, ECDbPropertyMapInfo const*, CompareIUtf8> ECDbClassMapInfo::GetPropertyMapByColumnName (bool onlyLocal) const
    {
    std::map<Utf8CP, ECDbPropertyMapInfo const*, CompareIUtf8> map;
    for (auto propertyMap : GetPropertyMaps (onlyLocal))
        map[propertyMap->GetColumn ().GetName ().c_str ()] = propertyMap;

    return map;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
void ECDbClassMapInfo::GetPropertyMaps (std::vector<ECDbPropertyMapInfo const*>& propertyMaps, bool onlyLocal) const
    {
    if (!onlyLocal && GetBaseClassMap () != nullptr)
        {
        GetBaseClassMap ()->GetPropertyMaps (propertyMaps, onlyLocal);
        }

    for (auto& localPropertyMap : m_localPropertyMaps)
        propertyMaps.push_back (localPropertyMap.get ());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbClassMapInfo const*  ECDbClassMapInfo::GetBaseClassMap () const
    {
    if (m_ecBaseClassMap != nullptr)
        return m_ecBaseClassMap;

    if (m_ecBaseClassMapId != 0LL)
        m_ecBaseClassMap = m_map.FindClassMap (m_ecBaseClassMapId);

    return m_ecBaseClassMap;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbPropertyMapInfo const* ECDbClassMapInfo::FindPropertyMap (ECN::ECPropertyId rootPropertyId, Utf8CP accessString) const
    {
    for (auto kp : GetPropertyMaps(false))
        {
        if (kp->GetPropertyPath ().GetRootPropertyId () == rootPropertyId && kp->GetPropertyPath ().GetAccessString () == accessString)
            return kp;
        }

    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbPropertyMapInfo* ECDbClassMapInfo::CreatePropertyMap (ECDbPropertyPath const& propertyPath, ECDbSqlColumn const& column)
    {
    auto propertyMap = std::unique_ptr<ECDbPropertyMapInfo> (new ECDbPropertyMapInfo (*this, propertyPath, column));
    auto p = propertyMap.get ();
    m_localPropertyMaps.push_back (std::move (propertyMap));
    return p;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
ECDbPropertyMapInfo* ECDbClassMapInfo::CreatePropertyMap (ECN::ECPropertyId rootPropertyId, Utf8CP accessString, ECDbSqlColumn const& column)
    {
    auto propertyPath = m_map.FindPropertyPath (rootPropertyId, accessString);
    if (propertyPath == nullptr)
        {
        propertyPath = m_map.CreatePropertyPath (rootPropertyId, accessString);
        }

    return CreatePropertyMap (*propertyPath, column);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
