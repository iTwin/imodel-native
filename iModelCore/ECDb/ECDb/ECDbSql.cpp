
/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSql.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbSql.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


#define ECDBSQL_NAME        "Name"
#define ECDBSQL_DB          "Db"
#define ECDBSQL_VERSION     "Version"
#define ECDBSQL_TABLES      "Tables"
#define ECDBSQL_TABLE       "Table"
#define ECDBSQL_INDEXES     "Indexes"
#define ECDBSQL_INDEX       "Index"
#define ECDBSQL_PRIMARY_KEY "PrimaryKey"
#define ECDBSQL_COLUMNS     "Columns"
#define ECDBSQL_COLUMN      "Column"
#define ECDBSQL_FORIEGNKEY  "ForiegnKey"
#define ECDBSQL_SOURCE      "Source"
#define ECDBSQL_TARGET      "Target"
#define ECDBSQL_ID          "Id"
#define ECDBSQL_TYPE        "Type"
#define ECDBSQL_WHERE       "Where"
#define ECDBSQL_PERSISTENCE_TYPE        "PersistenceType"
#define ECDBSQL_CONSTRAINTS "Constraints"
#define ECDBSQL_NOTNULL     "NotNull"
#define ECDBSQL_UNIQUE      "Unique"
#define ECDBSQL_CHECK       "Check"
#define ECDBSQL_DEFAULT     "Default"
#define ECDBSQL_COLLATE     "Collate"
#define ECDBSQL_OWNERTYPE   "OwnerType"
#define ECDBSQL_REFS        "Ref"
#define ECDBSQL_CLASSID     "CId"
#define ECDBSQL_Path        "Path"
#define ECDBSQL_USERFLAGS   "UserFlags"

//****************************************************************************************
//EditHandle
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        11/2014
//---------------------------------------------------------------------------------------
EditHandle::EditHandle ()
    :m_canEdit (true), m_isModified (false)
    {

    }

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
//ECDbSqlDbDebugWriter
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static 
void ECDbSqlDbDebugWriter::WriteDbDef (ECDbDebugStringWriter& writer, ECDbSqlDb const& obj)
    {
    writer.WriteLine ("[ObjectType = ECDbSqlDb] [Name = %s]", obj.GetName ().c_str ());
    auto scope = writer.CreateScope ();

    for (auto const childObj : obj.GetTables ())
        {
        WriteTableDef (writer, *childObj);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static 
void ECDbSqlDbDebugWriter::WriteTableDef (ECDbDebugStringWriter& writer, ECDbSqlTable const& obj)
    {
    writer.WriteLine ("[ObjectType = ECDbSqlTable] [Name = %s]", obj.GetName ().c_str ());
    auto scope = writer.CreateScope ();
    for (auto const childObj : obj.GetColumns ())
        {
        WriteColumnDef (writer, *childObj);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static 
void ECDbSqlDbDebugWriter::WriteColumnDef (ECDbDebugStringWriter& writer, ECDbSqlColumn const& obj)
    {
    Utf8String typeName = "<not-set>";
    switch (obj.GetType ())
        {
        case ECDbSqlColumn::Type::Any: typeName = "Any"; break;
        case ECDbSqlColumn::Type::Binary: typeName = "Binary"; break;
        case ECDbSqlColumn::Type::Boolean: typeName = "Boolean"; break;
        case ECDbSqlColumn::Type::DateTime: typeName = "DateTime"; break;
        case ECDbSqlColumn::Type::Double: typeName = "Double"; break;
        case ECDbSqlColumn::Type::Integer: typeName = "Integer"; break;
        case ECDbSqlColumn::Type::Long: typeName = "Long"; break;
        case ECDbSqlColumn::Type::String: typeName = "String"; break;
        }
    writer.WriteLine ("[ObjectType = ECDbSqlColumn] [Name = %s] [Type : %s]", obj.GetName ().c_str (), typeName.c_str ());
    }


//****************************************************************************************
//ECDbDebugStringWriter
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ECDbDebugStringWriter::Indent () { m_currentIndent++; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ECDbDebugStringWriter::Unindent () { if (m_currentIndent > 0) { m_currentIndent--; } }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbDebugStringWriter::ECDbDebugStringWriter (int indentSize, Utf8CP indentChar)
:m_indentSize (indentSize), m_indentChar (indentChar), m_currentIndent (0), m_eol ("\r\n"), m_needPrefix (true)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbDebugStringWriter::ScopePtr ECDbDebugStringWriter::CreateScope () { return ScopePtr (new Scope (*this)); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ECDbDebugStringWriter::Reset ()
    {
    m_currentIndent = 0;
    m_buffer.clear ();
    m_needPrefix = true;
    m_prefix.clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ECDbDebugStringWriter::Write (Utf8StringCR buffer)
    {
    if (m_needPrefix)
        {
        m_buffer.append (BuildPrefix ());
        m_needPrefix = false;
        }

    m_buffer.append (buffer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ECDbDebugStringWriter::Write (Utf8CP fmt, ...)
    {
    Utf8String tmp;
    va_list args;
    va_start (args, fmt);
    tmp.VSprintf (fmt, args);
    va_end (args);
    if (m_needPrefix)
        {
        m_buffer.append (BuildPrefix ());
        m_needPrefix = false;
        }

    m_buffer.append (tmp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ECDbDebugStringWriter::WriteLine (Utf8CP fmt, ...)
    {
    Utf8String tmp;
    va_list args;
    va_start (args, fmt);
    tmp.VSprintf (fmt, args);
    va_end (args);
    if (m_needPrefix)
        {
        m_buffer.append (BuildPrefix ());
        }

    m_buffer.append (tmp);
    WriteLine ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
Utf8CP ECDbDebugStringWriter::BuildPrefix ()
    {
    m_prefix.clear ();
    auto prefixLength = m_currentIndent * m_indentSize * m_indentChar.size ();
    if (prefixLength > 0)
        m_prefix.reserve (prefixLength);

    for (size_t i = 0; i < prefixLength; i++)
        m_prefix.append (m_indentChar);

    return m_prefix.c_str ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ECDbDebugStringWriter::WriteLine ()
    {
    m_buffer.append (m_eol);
    m_needPrefix = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
Utf8StringCR ECDbDebugStringWriter::GetString () const { return m_buffer; }

//****************************************************************************************
//ECDbSqlTable
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::set<ECN::ECClassId> ECDbSqlTable::GetReferences () const
    {
    std::set<ECN::ECClassId> tmp;
    for (auto column : m_orderedColumns)
        {
        for (auto id : column->GetDependentProperties ().GetClasses ())
            {
            tmp.insert (id);
            }
        }

    return tmp;
    }
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
BentleyStatus ECDbSqlTable::GetFilteredColumnList (std::vector<ECDbSqlColumn const*>& columns, PersistenceType persistenceType) const
    {
    for (auto column : m_orderedColumns)
        {
        if (column->GetPersistenceType () == persistenceType)
            columns.push_back (column);
        }

    return columns.empty () ? BentleyStatus::ERROR : BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTable::GetFilteredColumnList (std::vector<ECDbSqlColumn const*>& columns, uint32_t userFlag) const
    {
    for (auto column : m_orderedColumns)
        {
        if ((column->GetUserFlags () & userFlag) != 0)
            columns.push_back (column);
        }

    return columns.empty () ? BentleyStatus::ERROR : BentleyStatus::SUCCESS;
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
ECDbSqlForiegnKeyConstraint* ECDbSqlTable::CreateForiegnKeyConstraint (ECDbSqlTable const& targetTable)
    {
    if (GetEditHandleR ().AssertNotInEditMode ())
        return nullptr;

    ECDbSqlForiegnKeyConstraint * constraint = new ECDbSqlForiegnKeyConstraint (*this, targetTable);
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
ECDbSqlColumn* ECDbSqlTable::CreateColumn (Utf8CP name, ECDbSqlColumn::Type type, uint32_t userFlag, PersistenceType persistenceType)
    {
    return CreateColumn (name, type, m_orderedColumns.size (), userFlag, persistenceType);
    }
ECDbSqlTrigger::ECDbSqlTrigger(ECDbSqlTable& table) :m_table(table){}
//---------------------------------------------------------------------------------------
// @bsimethod                          muhammad.zaighum                           01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTable::CreateTrigger(Utf8CP triggerName, ECDbSqlTable& table, Utf8CP condition, Utf8CP body, TriggerType ecsqlType)
    {
    if (m_trigers.find(triggerName) == m_trigers.end())
        {
        m_trigers[triggerName] = std::unique_ptr<ECDbSqlTrigger>(new ECDbSqlTrigger(triggerName, table, condition, body, ecsqlType));
        return SUCCESS;
        }
    return ERROR;
    }
std::vector<const ECDbSqlTrigger*> ECDbSqlTable::GetTriggers()const
    {
    std::vector<const ECDbSqlTrigger*> triggers;
    for (auto &trigger : m_trigers)
        {
        triggers.push_back(trigger.second.get());
        }
    return triggers;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlColumn* ECDbSqlTable::CreateColumn (Utf8CP name, ECDbSqlColumn::Type type, size_t position, uint32_t userFlag, PersistenceType persistenceType)
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

        newColumn = std::make_shared<ECDbSqlColumn> (name, type, *this, resolvePersistenceType);
        }
    else
        {
        Utf8String generatedName;
        do
            {
            m_nameGeneratorForColumn.Generate (generatedName);
            } while (FindColumnCP (generatedName.c_str ()));

            newColumn = std::make_shared<ECDbSqlColumn> (generatedName.c_str (), type, *this, resolvePersistenceType);
        }

    newColumn->SetUserFlags (userFlag);
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
            if (constraint->GetType () == ECDbSqlConstraint::Type::ForiegnKey)
                {
                auto fkc = static_cast<ECDbSqlForiegnKeyConstraint*>(constraint.get ());
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
ECDbSqlIndex* ECDbSqlTable::CreateIndex (Utf8CP indexName)
    {
    return m_dbDef.CreateIndex (GetName ().c_str (), indexName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
const std::vector<ECDbSqlIndex const*> ECDbSqlTable::GetIndexes () const
    {
    std::vector<ECDbSqlIndex const*> indexes;
    for (auto index : m_dbDef.GetIndexes ())
        {
        if (&index->GetTable () == this)
            indexes.push_back (index);
        }

    return indexes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
const std::vector<ECDbSqlIndex*> ECDbSqlTable::GetIndexesR ()
    {
    std::vector<ECDbSqlIndex*> indexes;
    for (auto index : m_dbDef.GetIndexesR ())
        {
        if (&index->GetTable () == this)
            indexes.push_back (index);
        }

    return indexes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTable::WriteTo (BeXmlNodeR xmlNode) const
    {
    auto ecdbSqlTableXml = xmlNode.AddEmptyElement (ECDBSQL_TABLE);
    ecdbSqlTableXml->AddAttributeStringValue (ECDBSQL_NAME, WString (m_name.c_str (), BentleyCharEncoding::Utf8).c_str ());
    ecdbSqlTableXml->AddAttributeUInt32Value (ECDBSQL_ID, GetId ());
    if (m_type != PersistenceType::Persisted)
        ecdbSqlTableXml->AddAttributeUInt32Value (ECDBSQL_PERSISTENCE_TYPE, static_cast<int>(m_type));

    if (m_ownerType != OwnerType::ECDb)
        ecdbSqlTableXml->AddAttributeUInt32Value (ECDBSQL_OWNERTYPE, static_cast<int>(m_ownerType));

    auto ecdbSqlColumnXml = ecdbSqlTableXml->AddEmptyElement (ECDBSQL_COLUMNS);
    for (auto column : m_orderedColumns)
        {
        if (column->WriteTo (*ecdbSqlColumnXml) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }
    auto ecdbSqlConstraintXml = ecdbSqlTableXml->AddEmptyElement (ECDBSQL_CONSTRAINTS);
    for (auto& constraint : m_constraints)
        {
        if (constraint->WriteTo (*ecdbSqlConstraintXml) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static 
BentleyStatus ECDbSqlTable::ReadFrom (ECDbSqlDb& ecdbSqlDb, BeXmlNodeR xmlNode)
    {
    Utf8String name;
    Identity::Id id;
    uint32_t type = 0;

    if (!xmlNode.IsIName (ECDBSQL_TABLE))
        return BentleyStatus::ERROR;

    xmlNode.GetAttributeStringValue (name, ECDBSQL_NAME);
    xmlNode.GetAttributeUInt32Value (id, ECDBSQL_ID);

    PersistenceType persistenceType = PersistenceType::Persisted;
    if (xmlNode.GetAttributeUInt32Value (type, ECDBSQL_PERSISTENCE_TYPE) == BeXmlStatus::BEXML_Success)
        {
        persistenceType = static_cast<PersistenceType>(type);
        }

    OwnerType ownerType = OwnerType::ECDb;
    if (xmlNode.GetAttributeUInt32Value (type, ECDBSQL_OWNERTYPE) == BeXmlStatus::BEXML_Success)
        {
        ownerType = static_cast<OwnerType>(type);
        }

    ECDbSqlTable*  table = nullptr;
    if (ownerType == OwnerType::ECDb)
        table = ecdbSqlDb.CreateTable (name.c_str (), persistenceType);
    else if (ownerType == OwnerType::ExistingTable)
        table = ecdbSqlDb.CreateTableUsingExistingTableDefinition (name.c_str ());
    else
        {
        return BentleyStatus::ERROR;
        }

    table->SetId (id);

    for (auto child = xmlNode.GetFirstChild (); child != nullptr; child = child->GetNextSibling ())
        {
        if (child->IsName (ECDBSQL_COLUMNS))
            {
            for (auto column = child->GetFirstChild (); column != nullptr; column = column->GetNextSibling ())
                {
                if (ECDbSqlColumn::ReadFrom (*table, *column) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }
            }
        if (child->IsName (ECDBSQL_CONSTRAINTS))
            {
            for (auto constraint = child->GetFirstChild (); constraint != nullptr; constraint = constraint->GetNextSibling ())
                {
                if (ECDbSqlConstraint::ReadFrom (*table, *constraint) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }
            }
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTable::PersistenceManager::Syncronize (ECDbR ecdb)
    {
    auto name = GetTable ().GetName ().c_str ();
    auto type = DbMetaDataHelper::GetObjectType (ecdb, name);
    if (type == DbMetaDataHelper::ObjectType::None)
        {
        return Create (ecdb, true);
        }

    if (GetTable ().GetPersistenceType () == PersistenceType::Virtual)
        return BentleyStatus::ERROR;

    //! Object type is view and exist in db. Action = DROP it and recreate it.
    if (type == DbMetaDataHelper::ObjectType::View)
        {

        auto r = ecdb.ExecuteSql (SqlPrintfString ("DROP VIEW %s", name));
        if (r != BE_SQLITE_OK)
            {
            LOG.errorv ("Failed to drop view '%s'", name);
            return BentleyStatus::ERROR;
            }

        return Create (ecdb, true);
        }
    //! Object type is view and exist in db. Action = DROP it and recreate it.
    if (type == DbMetaDataHelper::ObjectType::Table)
        {
        //Add new columns to table.
        //Strategies
        //1. Create backup of table and drop the main table. Then recreat it and re-insert data in it.
        //2. Add new columns and indexes on then table.
        //Here we are going with 2. Though 1. is far better solution.
        //ALTER /rename
        //Process NEW Columns add to db.

        bvector<Utf8String> existingDbColumns;
        if (!ecdb.GetColumns (existingDbColumns, name))
            {
            LOG.errorv ("Failed to get column list for table %s", name);
            BeAssert (false && "Failed to get column list for table");
            return BentleyStatus::ERROR;
            }

        //Create a fast hash set of existing db column list
        std::set<Utf8String> existingColumnSet;
        for (auto& existingDbColumn : existingDbColumns)
            {
            existingColumnSet.insert (existingDbColumn);
            }

        //Create a fast hash set of in-memory column list;
        std::vector<ECDbSqlColumn const*>  persistedColumns;
        GetTable ().GetFilteredColumnList (persistedColumns, PersistenceType::Persisted);

        std::map<Utf8CP, ECDbSqlColumn const*, CompareIUtf8> modifiedColumnSet;

        for (auto column : persistedColumns)
            {
            modifiedColumnSet[column->GetName ().c_str ()] = column;
            }

        std::vector<Utf8CP> newColumns;
        //compute new columns;
        for (auto const& modifiedColumn : modifiedColumnSet)
            {
            if (existingColumnSet.find (modifiedColumn.first) == existingColumnSet.end ())
                newColumns.push_back (modifiedColumn.first);
            }

        std::vector<Utf8String> deleteColumns;
        //compute deleted columns;
        for (auto const& existingDbColumn : existingDbColumns)
            {
            if (modifiedColumnSet.find (existingDbColumn.c_str ()) == modifiedColumnSet.end ())
                deleteColumns.push_back (existingDbColumn);
            }

        //If we do not have any deleted column then we do not need the complicated operation of recreating table
        if (deleteColumns.empty ())
            {
            return DDLGenerator::AddColumns (GetTable (), newColumns, ecdb);
            }
        else
            {
            //Recreate the table and move rows to it.
            // get rid of indexes. NONE UNIQUE index are global and need to be remove so new table is create without a error
            for (auto index : GetTableR ().GetIndexesR ())
                {
                auto type = DbMetaDataHelper::GetObjectType (ecdb, index->GetName ().c_str ());
                if (type == DbMetaDataHelper::ObjectType::Index)
                    {
                    if (index->Drop () != BentleyStatus::SUCCESS)
                        {
                        LOG.errorv ("Failed to drop index %s on table %s", index->GetName ().c_str (), name);
                        BeAssert (false && "Failed to drop  index");
                        }
                    }
                }

            Utf8String workingTableName = "temp_ecdb_" + GetTable ().GetName ();
            if (!ecdb.RenameTable (name, workingTableName.c_str ()))
                {
                LOG.errorv ("Failed to rename table %s to %s", name, workingTableName.c_str ());
                BeAssert (false && "Failed to rename the table");
                return BentleyStatus::ERROR;
                }

            auto status = Create (ecdb, true);
            if (status != SUCCESS)
                {
                LOG.errorv ("Failed to create table %s", name);
                BeAssert (false && "Failed to create table");
                return BentleyStatus::ERROR;
                }

            bvector<Utf8String> columnList;
            if (!ecdb.GetColumns (columnList, name))
                {
                LOG.errorv ("Failed to get column list for table %s", name);
                BeAssert (false && "Failed to get column list for table");
                return BentleyStatus::ERROR;
                }

            columnList.erase (
                std::remove_if (columnList.begin (), columnList.end (),
                [&] (const Utf8String& v)
                {
                for (auto const& col : newColumns)
                    {
                    if (v.EqualsI (col))
                        return true;
                    };
                return false;
                }
            ), columnList.end ());

            //Remove new columns from the list
            if (DDLGenerator::CopyRows (ecdb, workingTableName.c_str (), columnList, name, columnList) != BentleyStatus::SUCCESS)
                {
                LOG.errorv ("Failed to copy rows from %s to %s", workingTableName.c_str (), name);
                BeAssert (false && "Failed to copy rows");
                return BentleyStatus::ERROR;
                }

            auto statusr = ecdb.DropTable (workingTableName.c_str ());
            if (statusr != BE_SQLITE_OK)
                {
                LOG.errorv ("Failed to drop temprory working table %s", workingTableName.c_str ());
                BeAssert (false && "Failed to drop temprory working table ");
                return BentleyStatus::ERROR;
                }
            }
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTable::PersistenceManager::Create (ECDbR ecdb, bool createIndexes)
    {
    if (!GetTable ().IsValid ())
        {
        BeAssert (false && "Table definition is not valid");
        return BentleyStatus::ERROR;
        }

    if (Exist (ecdb))
        {
        BeAssert (false && "Table already exists");
        return BentleyStatus::ERROR;
        }

    auto sql = DDLGenerator::GetCreateTableDDL (GetTable (), DDLGenerator::CreateOption::Create);
    if (ecdb.ExecuteSql (sql.c_str ()) != BE_SQLITE_OK)
        return BentleyStatus::ERROR;

    if (createIndexes)
        {
        for (auto index : GetTableR ().GetIndexesR ())
            {
            if (index->GetPersistenceManagerR ().Create (ecdb) != BentleyStatus::SUCCESS)
                {
                BeAssert (false && "Failed to create index");
                return BentleyStatus::ERROR;
                }
            }
        for (auto trigger : m_table.GetTriggers())
            {
            auto sql = DDLGenerator::GetCreateTriggerDDL(*trigger);
            if (ecdb.ExecuteSql(sql.c_str()) != BE_SQLITE_OK)
                return BentleyStatus::ERROR;
            }
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlTable::PersistenceManager::Drop (ECDbR ecdb)
    {
    if (!GetTable ().IsValid ())
        {
        BeAssert (false && "Table definition is not valid");
        return BentleyStatus::ERROR;
        }

    if (Exist (ecdb))
        {
        BeAssert (false && "Table already exists");
        return BentleyStatus::ERROR;
        }

    for (auto index : GetTableR ().GetIndexesR ())
        {
        if (index->GetPersistenceManagerR ().Drop (ecdb) != BentleyStatus::SUCCESS)
            {
            BeAssert (false && "Failed to drop index");
            return BentleyStatus::ERROR;
            }
        }

    auto sql = DDLGenerator::GetDropTableDDL (GetTable ());
    if (ecdb.ExecuteSql (sql.c_str ()) != BE_SQLITE_OK)
        return BentleyStatus::ERROR;



    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ECDbSqlTable::PersistenceManager::Exist (ECDbR ecdb) const
    {
    return ecdb.TableExists (GetTable ().GetName ().c_str ());
    }


//****************************************************************************************
//ECDbSqlDb
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ECDbSqlDb::Reset ()
    {
    m_index.clear ();
    m_tables.clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlDb::DropTable (Utf8CP name)
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
BentleyStatus ECDbSqlDb::DropIndex (Utf8CP name)
    {
    auto itor = m_index.find (name);
    if (itor == m_index.end ())
        {
        BeAssert (false && "Fail to find index");
        return BentleyStatus::ERROR;
        }

    m_index.erase (itor);
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlDb::WriteTo (Utf8StringR xml) const
    {
    auto xmlDom = BeXmlDom::CreateEmpty ();
    if (WriteTo (*xmlDom) == BentleyStatus::ERROR)
        return BentleyStatus::ERROR;

    xmlDom->ToString (xml, static_cast<BeXmlDom::ToStringOption>(BeXmlDom::TO_STRING_OPTION_Formatted | BeXmlDom::TO_STRING_OPTION_Indent));

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlTable* ECDbSqlDb::CreateTable (Utf8CP name, PersistenceType type)
    {
    ECDbSqlTable* newTableDef;
    if (!Utf8String::IsNullOrEmpty (name))
        {
        if (FindTable (name))
            {
            BeAssert (false && "Table with same name already exists");
            return nullptr;
            }

        newTableDef = new ECDbSqlTable (name, *this, GenerateId(), type, OwnerType::ECDb);
        }
    else
        {
        Utf8String generatedName;
        do
            {
            m_nameGenerator.Generate (generatedName);
            } while (FindTable (generatedName.c_str ()));

            newTableDef = new ECDbSqlTable (generatedName.c_str (), *this, GenerateId (), type, OwnerType::ECDb);
        }

    m_tables[newTableDef->GetName ().c_str ()] = std::unique_ptr<ECDbSqlTable> (newTableDef);
    return newTableDef;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlTable* ECDbSqlDb::CreateTableUsingExistingTableDefinition (Utf8CP existingTableName)
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

    auto newTableDef = new ECDbSqlTable (existingTableName, *this, GenerateId (), PersistenceType::Persisted, OwnerType::ExistingTable);
    newTableDef->GetEditHandleR ().EndEdit (); //we do not want this table to be editable;
    m_tables[newTableDef->GetName ().c_str ()] = std::unique_ptr<ECDbSqlTable> (newTableDef);
    return newTableDef;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlTable* ECDbSqlDb::CreateTableUsingExistingTableDefinition (ECDbCR ecdb, Utf8CP existingTableName)
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

    if (!ecdb.TableExists (existingTableName))
        {
        BeAssert (false && "Table does not exist in db");
        return nullptr;
        }

    auto newTableDef = new ECDbSqlTable (existingTableName, *this, GenerateId (), PersistenceType::Persisted, OwnerType::ExistingTable);
    
    Statement stmt;
    if (stmt.Prepare (ecdb, SqlPrintfString ("PRAGMA table_info('%s')", existingTableName)) != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to prepare statement");
        return nullptr;
        }

    while (stmt.Step () == BE_SQLITE_ROW)
        {
        auto name = stmt.GetValueText (1);
        Utf8String type = stmt.GetValueText (2);
        auto notnull = stmt.GetValueInt (3) == 1;
        auto dflt_value = stmt.GetValueText (4);
        auto pk = stmt.GetValueInt (5) == 1;
        type.ToLower ();
        type.Trim ();

        ECDbSqlColumn::Type ecType = ECDbSqlColumn::Type::Any;
        //following is somewhat base on sqlite funtion sqlite3AffinityType
        if (type.rfind ("int64") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::Long;
        else if (type.rfind ("int") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::Integer;
        else if (type.rfind ("char") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::String;
        else if (type.rfind ("clob") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::String;
        else if (type.rfind ("text") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::String;
        else if (type.rfind ("blob") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::Binary;
        else if (type.rfind ("real") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::Double;
        else if (type.rfind ("floa") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::Double;
        else if (type.rfind ("doub") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::Double;
        else if (type.rfind ("date") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::DateTime;
        else if (type.rfind ("time") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::DateTime;
        else if (type.rfind ("bool") != Utf8String::npos)
            ecType = ECDbSqlColumn::Type::Boolean;

        auto column = newTableDef->CreateColumn (name, ecType);
        if (column == nullptr)
            {
            BeAssert (false && "Failed to create column");
            return nullptr;
            }

        if (!Utf8String::IsNullOrEmpty (dflt_value))
            column->GetConstraintR ().SetDefaultExpression (dflt_value);

        column->GetConstraintR ().SetIsNotNull (notnull);
        if (pk)
            newTableDef->GetPrimaryKeyConstraint ()->Add (name);
        }
    
    newTableDef->GetEditHandleR ().EndEdit (); //we do not want this table to be editable;
    m_tables[newTableDef->GetName ().c_str ()] = std::unique_ptr<ECDbSqlTable> (newTableDef);
    return newTableDef;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ECDbSqlDb::HasObject (Utf8CP name)
    {
    return m_tables.find (name) != m_tables.end () || m_index.find (name) != m_index.end ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlIndex* ECDbSqlDb::CreateIndex (Utf8CP tableName, Utf8CP indexName)
    {
    auto table = FindTableP (tableName);
    if (table == nullptr)
        {
        BeAssert (false && "Table not found");
        return nullptr;
        }

    if (Utf8String::IsNullOrEmpty (indexName))
        {
        Utf8String generatedName;
        do
            {
            m_nameGenerator.Generate (generatedName);
            } while (HasObject (generatedName.c_str ()));

            auto index = new ECDbSqlIndex (*table, generatedName.c_str (), GenerateId ());
            m_index[index->GetName ().c_str ()] = std::unique_ptr<ECDbSqlIndex> (index);
            return index;
        }

    if (HasObject (indexName))
        {
        BeAssert (false && "Already have object with same name");
        return nullptr;
        }

    auto index = new ECDbSqlIndex (*table, indexName, GenerateId ());
    m_index[index->GetName ().c_str ()] = std::unique_ptr<ECDbSqlIndex> (index);
    return index;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
const std::vector<ECDbSqlIndex const*> ECDbSqlDb::GetIndexes () const
    {
    std::vector<ECDbSqlIndex const*> indexes;
    for (auto& key : m_index)
        {
        indexes.push_back (key.second.get ());
        }

    return indexes;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
std::vector<ECDbSqlIndex*> ECDbSqlDb::GetIndexesR ()
    {
    std::vector<ECDbSqlIndex*> indexes;
    for (auto& key : m_index)
        {
        indexes.push_back (key.second.get ());
        }

    return indexes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlTable const* ECDbSqlDb::FindTable (Utf8CP name) const
    {
    auto itor = m_tables.find (name);
    if (itor != m_tables.end ())
        return itor->second.get ();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlTable* ECDbSqlDb::FindTableP (Utf8CP name) const
    {
    auto itor = m_tables.find (name);
    if (itor != m_tables.end ())
        return itor->second.get ();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlIndex const* ECDbSqlDb::FindIndex (Utf8CP name) const
    {
    auto itor = m_index.find (name);
    if (itor != m_index.end ())
        return itor->second.get ();

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
ECDbSqlIndex* ECDbSqlDb::FindIndexP (Utf8CP name) 
    {
    auto itor = m_index.find (name);
    if (itor != m_index.end ())
        return itor->second.get ();

    return nullptr;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
const std::vector<ECDbSqlTable const*> ECDbSqlDb::GetTables () const
    {
    std::vector<ECDbSqlTable const*> tables;
    for (auto const& key : m_tables)
        tables.push_back (key.second.get ());

    return tables;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlDb::WriteTo (BeXmlDom& xmlDom) const
    {
    auto ecdbSqlDbXml = xmlDom.AddNewElement (ECDBSQL_DB, nullptr, nullptr);
    ecdbSqlDbXml->AddAttributeStringValue (ECDBSQL_NAME, WString (GetName ().c_str (), BentleyCharEncoding::Utf8).c_str ());
    ecdbSqlDbXml->AddAttributeStringValue (ECDBSQL_VERSION, WString (m_version.ToString ().c_str (), BentleyCharEncoding::Utf8).c_str ());
    auto ecdbSqlTables = ecdbSqlDbXml->AddEmptyElement (ECDBSQL_TABLES);
    for (auto& kp : m_tables)
        {
        //if (kp.second->GetEditHandle ().CanEdit ())
        //    {
        //    BeAssert (false && "One of table is in edit mode. Finish edition before calling this funtion");
        //    return BentleyStatus::ERROR;
        //    }

        if (kp.second->WriteTo (*ecdbSqlTables) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }

    auto ecdbSqlIndexes = ecdbSqlDbXml->AddEmptyElement (ECDBSQL_INDEXES);
    for (auto& kp : m_index)
        {
        if (kp.second->WriteTo (*ecdbSqlIndexes) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECDbSqlDb::ReadFrom (ECDbSqlDb& ecdbSqlDb, BeXmlDomR xmlDom)
    {
    ecdbSqlDb.Reset ();
    auto ecdbSqlDbXml = xmlDom.GetRootElement ();
    if (!ecdbSqlDbXml->IsIName (ECDBSQL_DB))
        return BentleyStatus::ERROR;

    ecdbSqlDbXml->GetAttributeStringValue (ecdbSqlDb.m_name, ECDBSQL_NAME);
    Utf8String versionString;
    ecdbSqlDbXml->GetAttributeStringValue (versionString, ECDBSQL_VERSION);
    ecdbSqlDb.m_version = ECDbVersion::Parse (versionString.c_str ());

    for (auto child = ecdbSqlDbXml->GetFirstChild (); child != nullptr; child = child->GetNextSibling ())
        {
        if (child->IsName (ECDBSQL_TABLES))
            {
            for (auto table = child->GetFirstChild (); table != nullptr; table = table->GetNextSibling ())
                {
                if (ECDbSqlTable::ReadFrom (ecdbSqlDb, *table) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }
            }
        if (child->IsName (ECDBSQL_INDEXES))
            {
            for (auto index = child->GetFirstChild (); index != nullptr; index = index->GetNextSibling ())
                {
                if (ECDbSqlIndex::ReadFrom (ecdbSqlDb, *child) != BentleyStatus::SUCCESS)
                    return BentleyStatus::ERROR;
                }
            }
        }

    //Finish editing for all tables;
    for (auto& kp : ecdbSqlDb.m_tables)
        {
        if (kp.second->GetEditHandle ().CanEdit ())
            {
            kp.second->GetEditHandleR ().EndEdit ();
            }
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static 
BentleyStatus ECDbSqlDb::ReadFrom (ECDbSqlDb& ecdbSqlDb, Utf8StringCR xml)
    {
    BeXmlStatus stat;
    auto xmlDom = BeXmlDom::CreateAndReadFromString (stat, xml.c_str ());
    if (stat != BeXmlStatus::BEXML_Success)
        return BentleyStatus::SUCCESS;

    return ReadFrom (ecdbSqlDb, *xmlDom);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ECDbSqlDb::IsModified () const
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
    auto column = GetTable ().FindColumnCP (columnName);
    BeAssert (column != nullptr);
    if (column == nullptr)
        return BentleyStatus::ERROR;

    for (auto itor = m_columns.begin (); itor != m_columns.end (); ++itor)
        {
        if ((*itor)->GetName ().EqualsI (columnName))
            {
            return BentleyStatus::SUCCESS;
            }
        }

    if (column->GetPersistenceType () == PersistenceType::Virtual)
        {
        BeAssert (false && "Virtual column cannot be use as primary key");
        return BentleyStatus::ERROR;
        }

    m_columns.push_back (column);
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


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlPrimaryKeyConstraint::WriteTo (BeXmlNodeR xmlNode) const
    {
    auto ecdbConstraintXml = xmlNode.AddEmptyElement (ECDBSQL_PRIMARY_KEY);
    for (auto column : m_columns)
        {
        ecdbConstraintXml->AddElementStringValue (ECDBSQL_COLUMN, WString (column->GetName ().c_str (), BentleyCharEncoding::Utf8).c_str ());
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static 
BentleyStatus ECDbSqlPrimaryKeyConstraint::ReadFrom (ECDbSqlTable& ecdbSqlTable, BeXmlNodeR xmlNode)
    {
    if (!xmlNode.IsIName (ECDBSQL_PRIMARY_KEY))
        return BentleyStatus::ERROR;

    Utf8String columName;
    for (auto child = xmlNode.GetFirstChild (); child != nullptr; child = child->GetNextSibling ())
        {
        if (child->IsName (ECDBSQL_COLUMN))
            {
            child->GetContent (columName);
            if (ecdbSqlTable.GetPrimaryKeyConstraint ()->Add (columName.c_str ()) == BentleyStatus::ERROR)
                return BentleyStatus::ERROR;
            }
        }

    return BentleyStatus::SUCCESS;
    }

//****************************************************************************************
//ECDbSqlForiegnKeyConstraint
//****************************************************************************************
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSqlForiegnKeyConstraint::ContainsInSource (Utf8CP columnName) const
    {
    return std::find_if (m_sourceColumns.begin (), m_sourceColumns.end (), [columnName] (ECDbSqlColumn const* column){ return column->GetName ().EqualsI (columnName); }) != m_sourceColumns.end ();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbSqlForiegnKeyConstraint::ContainsInTarget (Utf8CP columnName) const
    {
    return std::find_if (m_targetColumns.begin (), m_targetColumns.end (), [columnName] (ECDbSqlColumn const* column){ return column->GetName ().EqualsI (columnName); }) != m_targetColumns.end ();
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
//static 
Utf8CP ECDbSqlForiegnKeyConstraint::ToSQL (ECDbSqlForiegnKeyConstraint::MatchType matchType)
    {
    switch (matchType)
        {
        case MatchType::Full:
            return "FULL";
        case MatchType::Partial:
            return "PARTIAL";
        case MatchType::Simple:
            return "SIMPLE";
        }

    return nullptr;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
//static
Utf8CP ECDbSqlForiegnKeyConstraint::ToSQL (ECDbSqlForiegnKeyConstraint::ActionType actionType)
    {
    switch (actionType)
        {
        case ActionType::Cascade:
            return "CASCADE";
        case ActionType::NoAction:
            return "NO ACTION";
        case ActionType::Restrict:
            return "RESTRICT";
        case ActionType::SetDefault:
            return "SET DEFAULT";
        case ActionType::SetNull:
            return "SET NULL";
        }

    return nullptr;
    }
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    affan.khan      09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
//static
ECDbSqlForiegnKeyConstraint::MatchType ECDbSqlForiegnKeyConstraint::ParseMatchType (WCharCP matchType)
    {
    if (WString (L"Simple").EqualsI (matchType))
        return MatchType::Simple;

    if (WString (L"Full").EqualsI (matchType))
        return MatchType::Full;

    if (WString (L"Partial").EqualsI (matchType))
        return MatchType::Partial;

    return MatchType::Full;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static 
ECDbSqlForiegnKeyConstraint::ActionType ECDbSqlForiegnKeyConstraint::ParseActionType (WCharCP actionType)
    {
    if (WString (L"Cascade").EqualsI (actionType))
        return ActionType::Cascade;

    if (WString (L"NoAction").EqualsI (actionType))
        return ActionType::NoAction;

    if (WString (L"SetNull").EqualsI (actionType))
        return ActionType::SetNull;

    if (WString (L"SetDefault").EqualsI (actionType))
        return ActionType::SetDefault;

    if (WString (L"Restrict").EqualsI (actionType))
        return ActionType::Restrict;

    return ActionType::NoAction;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlForiegnKeyConstraint::Remove (Utf8CP sourceColumn, Utf8CP targetColumn)
    {
    bool cont;
    bool hasSouceColumn = Utf8String::IsNullOrEmpty (sourceColumn);
    bool hasTargetColumn = Utf8String::IsNullOrEmpty (targetColumn);
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
BentleyStatus ECDbSqlForiegnKeyConstraint::Add (Utf8CP sourceColumn, Utf8CP targetColumn)
    {
    if (Utf8String::IsNullOrEmpty (sourceColumn) || Utf8String::IsNullOrEmpty (targetColumn))
        {
        BeAssert (false && "Source and target column cannot be empty of null");
        return BentleyStatus::ERROR;
        }

    auto source = GetSourceTable ().FindColumnCP (sourceColumn);
    auto target = GetTargetTable ().FindColumnCP (targetColumn);
    BeAssert (source != nullptr && target != nullptr);
    if (source == nullptr || target == nullptr)
        return BentleyStatus::ERROR;

    if (source->GetPersistenceType () == PersistenceType::Virtual)
        {
        BeAssert (false && "Virtual column cannot be use as foriegn key");
        return BentleyStatus::ERROR;
        }

    if (target->GetPersistenceType () == PersistenceType::Virtual)
        {
        BeAssert (false && "Virtual column cannot be use as foriegn key");
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
BentleyStatus ECDbSqlForiegnKeyConstraint::Remove (size_t index)
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
BentleyStatus ECDbSqlForiegnKeyConstraint::WriteTo (BeXmlNodeR xmlNode) const
    {
    auto ecdbConstraintXml = xmlNode.AddEmptyElement (ECDBSQL_FORIEGNKEY);
    if (GetMatchType () != MatchType::NotSpecified)
        {
        ecdbConstraintXml->AddAttributeUInt32Value ("MatchType", static_cast<uint32_t>(GetMatchType ()));
        }
    if (GetOnDeleteAction () != ActionType::NotSpecified)
        {
        ecdbConstraintXml->AddAttributeUInt32Value ("OnDelete", static_cast<uint32_t>(GetOnDeleteAction ()));
        }
    if (GetOnDeleteAction () != ActionType::NotSpecified)
        {
        ecdbConstraintXml->AddAttributeUInt32Value ("OnUpdate", static_cast<uint32_t>(GetOnDeleteAction ()));
        }

    auto sourceColumnXml = ecdbConstraintXml->AddEmptyElement (ECDBSQL_SOURCE);
    for (auto column : m_sourceColumns)
        {
        sourceColumnXml->AddElementStringValue (ECDBSQL_COLUMN, WString (column->GetName ().c_str (), BentleyCharEncoding::Utf8).c_str ());
        }

    auto targetXml = xmlNode.AddEmptyElement (ECDBSQL_TARGET);
    targetXml->AddAttributeStringValue (ECDBSQL_TABLE, WString (m_targetTable.GetName ().c_str (), BentleyCharEncoding::Utf8).c_str ());
    for (auto column : m_targetColumns)
        {
        targetXml->AddElementStringValue (ECDBSQL_COLUMN, WString (column->GetName ().c_str (), BentleyCharEncoding::Utf8).c_str ());
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static 
BentleyStatus ECDbSqlForiegnKeyConstraint::ReadFrom (ECDbSqlTable& ecdbSqlTable, BeXmlNodeR xmlNode)
    {
    if (!xmlNode.IsIName (ECDBSQL_FORIEGNKEY))
        return BentleyStatus::ERROR;
    uint32_t flag;
    MatchType matchType = MatchType::NotSpecified;
    ActionType onDelete = ActionType::NotSpecified;
    ActionType onUpdate = ActionType::NotSpecified;

    if (xmlNode.GetAttributeUInt32Value (flag, "MatchType") == BeXmlStatus::BEXML_Success)
        {
        matchType = static_cast<MatchType>(flag);
        }
    if (xmlNode.GetAttributeUInt32Value (flag, "OnDelete") == BeXmlStatus::BEXML_Success)
        {
        onDelete = static_cast<ActionType>(flag);
        }
    if (xmlNode.GetAttributeUInt32Value (flag, "OnUpdate") == BeXmlStatus::BEXML_Success)
        {
        onUpdate = static_cast<ActionType>(flag);
        }

    Utf8String columName, targetTable;
    std::vector<Utf8String> sourceColumns, targetColumns;


    for (auto child = xmlNode.GetFirstChild (); child != nullptr; child = child->GetNextSibling ())
        {
        if (child->IsName (ECDBSQL_SOURCE))
            {
            for (auto column = child->GetFirstChild (); column != nullptr; column = column->GetNextSibling ())
                {
                column->GetContent (columName);
                sourceColumns.push_back (columName);
                }
            }
        if (child->IsName (ECDBSQL_TARGET))
            {
            child->GetAttributeStringValue (targetTable, ECDBSQL_TABLE);
            for (auto column = child->GetFirstChild (); column != nullptr; column = column->GetNextSibling ())
                {
                column->GetContent (columName);
                targetColumns.push_back (columName);
                }
            }
        auto table = ecdbSqlTable.GetDbDef ().FindTable (targetTable.c_str ());
        auto constraint = ecdbSqlTable.CreateForiegnKeyConstraint (*table);
        constraint->SetMatchType (matchType);
        constraint->SetOnDeleteAction (onDelete);
        constraint->SetOnUpdateAction (onUpdate);
        for (auto sItor = sourceColumns.begin (), tItor = targetColumns.begin (); sItor != sourceColumns.end () && tItor != targetColumns.end (); ++sItor, ++tItor)
            {
            if (constraint->Add (sItor->c_str (), tItor->c_str ()) != BentleyStatus::SUCCESS)
                return BentleyStatus::ERROR;
            }
        }

    return BentleyStatus::SUCCESS;
    }

//****************************************************************************************
//ECDbSqlIndex
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ECDbSqlIndex::IsPartialIndex () const
    {
    return !GetWhereExpression ().empty ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
bool ECDbSqlIndex::Contains (Utf8CP column) const
    {
    for (auto col : m_columns)
        {
        if (col->GetName ().EqualsI (column))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlIndex::Add (Utf8CP column)
    {
    if (GetTableR ().GetEditHandleR ().AssertNotInEditMode ())
        return BentleyStatus::ERROR;

    if (Contains (column))
        return BentleyStatus::ERROR;

    auto col = GetTable ().FindColumnCP (column);
    if (col == nullptr)
        {
        BeAssert (false && "Failed to find column");
        return BentleyStatus::ERROR;
        }

    m_columns.push_back (col);
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlIndex::Remove (Utf8CP column)
    {
    if (GetTableR ().GetEditHandleR ().AssertNotInEditMode ())
        return BentleyStatus::ERROR;

    if (!Contains (column))
        return BentleyStatus::ERROR;

    for (auto itor = m_columns.begin (); itor != m_columns.end (); ++itor)
        {
        if ((*itor)->GetName ().EqualsI (column))
            {
            m_columns.erase (itor);
            return BentleyStatus::SUCCESS;
            }
        }

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ECDbSqlIndex::SetIsUnique (bool isUnique)
    {
    if (GetTableR ().GetEditHandleR ().AssertNotInEditMode ())
        return;

    m_isUnique = isUnique;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlIndex::Drop ()
    {
    if (GetTableR ().GetEditHandleR ().AssertNotInEditMode ())
        return BentleyStatus::ERROR;

    return GetTableR ().GetDbDefR ().DropIndex (m_name.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlIndex::WriteTo (BeXmlNodeR xmlNode) const
    {
    auto ecdbSqlIndexXml = xmlNode.AddEmptyElement (ECDBSQL_INDEX);
    ecdbSqlIndexXml->AddAttributeStringValue (ECDBSQL_NAME, WString (m_name.c_str (), BentleyCharEncoding::Utf8).c_str ());
    ecdbSqlIndexXml->AddAttributeUInt32Value (ECDBSQL_ID, GetId ());
    ecdbSqlIndexXml->AddAttributeStringValue (ECDBSQL_TABLE, WString (m_table.GetName ().c_str (), BentleyCharEncoding::Utf8).c_str ());
    if (m_isUnique)
        ecdbSqlIndexXml->AddAttributeBooleanValue (ECDBSQL_UNIQUE, m_isUnique);

    if (!m_whereExpression.empty ())
        ecdbSqlIndexXml->AddAttributeStringValue (ECDBSQL_WHERE, WString (m_whereExpression.c_str (), BentleyCharEncoding::Utf8).c_str ());

    for (auto column : m_columns)
        {
        ecdbSqlIndexXml->AddElementStringValue (ECDBSQL_COLUMN, WString (column->GetName ().c_str (), BentleyCharEncoding::Utf8).c_str ());
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
//static 
BentleyStatus ECDbSqlIndex::ReadFrom (ECDbSqlDb& ecdbSqlDb, BeXmlNodeR xmlNode)
    {
    if (!xmlNode.IsIName (ECDBSQL_INDEX))
        return BentleyStatus::ERROR;

    Utf8String name, whereExpr, table;
    Identity::Id id;
    bool isUnique = false;

    xmlNode.GetAttributeStringValue (name, ECDBSQL_NAME);
    xmlNode.GetAttributeUInt32Value (id, ECDBSQL_ID);
    xmlNode.GetAttributeStringValue (table, ECDBSQL_TABLE);
    xmlNode.GetAttributeBooleanValue (isUnique, ECDBSQL_UNIQUE);
    xmlNode.GetAttributeStringValue (whereExpr, ECDBSQL_WHERE);
    
    auto index = ecdbSqlDb.CreateIndex (table.c_str (), name.c_str ());
    index->SetId (id);
    index->SetIsUnique (isUnique);
    index->SetWhereExpression (whereExpr.c_str ());

    for (auto column = xmlNode.GetFirstChild (); column != nullptr; column = column->GetNextSibling ())
        {
        if (!column->IsIName (ECDBSQL_COLUMN))
            return BentleyStatus::ERROR;

        column->GetContent (name);
        if (index->Add (name.c_str ()) != BentleyStatus::SUCCESS)
            return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlIndex::PersistenceManager::Create (ECDbR ecdb)
    {
    if (!GetIndex().IsValid())
        {
        BeAssert (false && "Index definition is not valid");
        return BentleyStatus::ERROR;
        }

    if (Exist (ecdb))
        {
        BeAssert (false && "Index already exist");
        return BentleyStatus::ERROR;
        }

    auto sql = DDLGenerator::GetCreateIndexDDL (GetIndex (), DDLGenerator::CreateOption::Create);
    if (ecdb.ExecuteSql (sql.c_str ()) != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to create index");
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlIndex::PersistenceManager::Drop (ECDbR ecdb)
    {
    if (!Exist (ecdb))
        {
        BeAssert (false && "Failed to find the index");
        return BentleyStatus::ERROR;
        }

    auto sql = DDLGenerator::GetDropIndexDDL (GetIndex ());
    if (ecdb.ExecuteSql (sql.c_str ()) != BE_SQLITE_OK)
        {
        BeAssert (false && "Failed to create index");
        return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
bool ECDbSqlIndex::PersistenceManager::Exist (ECDbR ecdb) const
    {
    Statement stmt;
    stmt.Prepare (ecdb, SqlPrintfString ("select COUNT(*) from sqlite_master WHERE type = 'index' and name = '%s'", GetIndex ().GetName ().c_str ()));
    return stmt.GetValueInt (0) == 1;
    }

//****************************************************************************************
// DDLGenerator
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
Utf8String DDLGenerator::GetCreateIndexDDL (ECDbSqlIndex const& o, DDLGenerator::CreateOption option)
    {
    //CREATE UNIQUE INDEX name ON table () WHERE (expr)
    Utf8String sql = "CREATE ";
    if (option == CreateOption::CreateOrReplace)
        sql.append ("OR REPLACE ");

    if (o.GetIsUnique ())
        sql.append ("UNIQUE ");

    sql.append ("INDEX [").append (o.GetName ()).append ("] ON [").append(o.GetTable().GetName()).append("] (").append (GetColumnList (o.GetColumns ())).append (")");
    if (o.IsPartialIndex ())
        {
        sql.append (" WHERE (").append (o.GetWhereExpression ()).append (")");
        }

    return sql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DDLGenerator::CopyRows (BeSQLiteDbR db, Utf8CP sourceTable, bvector<Utf8String>& sourceColumns, Utf8CP targetTable, bvector<Utf8String>& targetColumns)
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
    LOG.infov ("Copying rows from table %s to %s", sourceTable, targetTable);
    return db.ExecuteSql (sql.c_str ()) == BE_SQLITE_OK ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DDLGenerator::AddColumns (ECDbSqlTable const& table, std::vector<Utf8CP> const& newColumns, BeSQLiteDbR &db)
    {
    if (newColumns.empty ())
        return BentleyStatus::SUCCESS;

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
            return BentleyStatus::ERROR;
            }
        alterDDL.append (GetColumnDDL (*column));
        auto r = db.ExecuteSql (alterDDL.c_str ());
        if (r != BE_SQLITE_OK)
            {
            LOG.errorv ("Failed to add new column. SQL: %s", alterDDLTemplate.c_str ());
            return BentleyStatus::ERROR;
            }
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                       muhammad.zaighum        1/2015
//---------------------------------------------------------------------------------------
//static 

Utf8String DDLGenerator::GetCreateTriggerDDL(ECDbSqlTrigger const& trigger)
    {
    //CREATE TRIGGER lastModElm AFTER UPDATE ON  dgn_Element WHEN old.LastMod = new.LastMod BEGIN UPDATE dgn_Element SET LastMod = julianday('now') WHERE Id = new.Id; END

    Utf8String sql = "";
    switch (trigger.GetType())
        {
        case TriggerType::Create:
            sql.append("CREATE ");
        }
    sql.append("TRIGGER ");
    sql.append(trigger.GetName());
    sql.append(" ");
    sql.append("AFTER UPDATE ON ");
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
            columnList.append (", ");
            }
        }

    return columnList;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
Utf8String DDLGenerator::GetForiegnKeyConstraintDDL (ECDbSqlForiegnKeyConstraint const& constraint)
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


    if (constraint.GetMatchType () != ECDbSqlForiegnKeyConstraint::MatchType::NotSpecified)
            {
            sql.append (" MATCH ");
            sql.append (ECDbSqlForiegnKeyConstraint::ToSQL (constraint.GetMatchType ()));
            }
    
    if (constraint.GetOnDeleteAction () != ECDbSqlForiegnKeyConstraint::ActionType::NotSpecified)
            {
            sql.append (" ON DELETE ");
            sql.append (ECDbSqlForiegnKeyConstraint::ToSQL (constraint.GetOnDeleteAction ()));
            }
    
    if (constraint.GetOnUpdateAction () != ECDbSqlForiegnKeyConstraint::ActionType::NotSpecified)
            {
            sql.append (" ON UPDATE ");
            sql.append (ECDbSqlForiegnKeyConstraint::ToSQL (constraint.GetOnUpdateAction ()));
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
        if (constraint->GetType () == ECDbSqlConstraint::Type::ForiegnKey)
            {
            constraintDDL.append (GetForiegnKeyConstraintDDL (*(static_cast<ECDbSqlForiegnKeyConstraint const*>(constraint))));
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
            sql.append ("DATETIME"); break;
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

    if (o.GetConstraint ().GetCollate () != ECDbSqlColumn::Constraint::Collate::Default)
        sql.append (" ").append("COLLATE ").append (ECDbSqlColumn::Constraint::CollateToString (o.GetConstraint ().GetCollate ()));

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
ECDbSqlColumn::Constraint::Collate ECDbSqlColumn::Constraint::StringToCollate (Utf8CP typeName)
    {
    static std::map<Utf8CP, Collate, CompareIUtf8> s_type
        {
            { "Default", Collate::Default },
            { "Binary", Collate::Binary },
            { "NoCase", Collate::NoCase },
            { "RTrim", Collate::RTrim }
    };
    auto itor = s_type.find (typeName);
    if (itor == s_type.end ())
        return Collate::Default;

    return itor->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
Utf8CP ECDbSqlColumn::Constraint::CollateToString (ECDbSqlColumn::Constraint::Collate collate)
    {
    static std::map<Collate ,Utf8CP> s_type
        {
            { Collate::Default, "Default" },
            { Collate::Binary, "Binary", },
            { Collate::NoCase, "NoCase" },
            {Collate::RTrim, "RTrim" }
    };
    auto itor = s_type.find (collate);
    if (itor == s_type.end ())
        return nullptr;

    return itor->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlColumn::SetUserFlags (uint32_t userFlags)
    {
    if (GetTableR ().GetEditHandleR ().AssertNotInEditMode ())
        return BentleyStatus::ERROR;

    m_userFlags = userFlags;
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
uint32_t ECDbSqlColumn::GetUserFlags () const
    {
    return m_userFlags;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------

const Utf8String ECDbSqlColumn::GetFullName () const { return BuildFullName (GetTable ().GetName ().c_str (), GetName ().c_str ()); }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//--------------------------------------------------------------------------------------
//static 
const Utf8String ECDbSqlColumn::BuildFullName (Utf8CP table, Utf8CP column)
    {
    Utf8String str;
    str.append ("[").append (table).append ("].[").append (column).append ("]");
    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
std::weak_ptr<ECDbSqlColumn> ECDbSqlColumn::GetWeakPtr () const
    {
    return GetTable ().GetColumnWeakPtr (GetName ().c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlColumn::WriteTo (BeXmlNodeR xmlNode) const
    {
    auto ecdbSqlColumn = xmlNode.AddEmptyElement (ECDBSQL_COLUMN);
    ecdbSqlColumn->AddAttributeStringValue (ECDBSQL_NAME, WString (m_name.c_str (), BentleyCharEncoding::Utf8).c_str ());
    ecdbSqlColumn->AddAttributeStringValue (ECDBSQL_TYPE, WString (TypeToString (m_type), BentleyCharEncoding::Utf8).c_str ());
    if (m_constraints.IsNotNull ())
        ecdbSqlColumn->AddAttributeBooleanValue (ECDBSQL_NOTNULL, m_constraints.IsNotNull ());

    if (m_constraints.IsUnique ())
        ecdbSqlColumn->AddAttributeBooleanValue (ECDBSQL_UNIQUE, m_constraints.IsUnique ());

    if (!m_constraints.GetCheckExpression ().empty ())
        ecdbSqlColumn->AddAttributeStringValue (ECDBSQL_CHECK, WString (m_constraints.GetCheckExpression ().c_str (), BentleyCharEncoding::Utf8).c_str ());

    if (!m_constraints.GetDefaultExpression ().empty ())
        ecdbSqlColumn->AddAttributeStringValue (ECDBSQL_DEFAULT, WString (m_constraints.GetDefaultExpression ().c_str (), BentleyCharEncoding::Utf8).c_str ());

    if (m_constraints.GetCollate () != Constraint::Collate::Default)
        ecdbSqlColumn->AddAttributeStringValue (ECDBSQL_COLLATE, WString (Constraint::CollateToString(m_constraints.GetCollate ()), BentleyCharEncoding::Utf8).c_str ());

    if (GetPersistenceType () != PersistenceType::Persisted)
        {
        ecdbSqlColumn->AddAttributeUInt32Value (ECDBSQL_PERSISTENCE_TYPE, static_cast <uint32_t > (GetPersistenceType ()));
        }

    if (GetUserFlags () != NullUserFlags)
        {        
        ecdbSqlColumn->AddAttributeUInt32Value (ECDBSQL_USERFLAGS, GetUserFlags ());
        }
    for (auto id : GetDependentProperties().GetClasses())
        {
        auto ref = ecdbSqlColumn->AddEmptyElement (ECDBSQL_REFS);
        ref->AddAttributeUInt64Value (ECDBSQL_CLASSID, id);
        ref->AddAttributeStringValue (ECDBSQL_Path, WString (GetDependentProperties ().Find (id), BentleyCharEncoding::Utf8).c_str ());
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static 
BentleyStatus ECDbSqlColumn::ReadFrom (ECDbSqlTable& ecdbSqlTable, BeXmlNodeR xmlNode)
    {
    Utf8String name, type, check, defaultStr, collate;
    uint64_t id;
    uint32_t userFlag = 0;
    bool isNotNull = false, isUnique = false;
    uint32_t ptype;
    xmlNode.GetAttributeStringValue (name, ECDBSQL_NAME);
    xmlNode.GetAttributeStringValue (type, ECDBSQL_TYPE);
    xmlNode.GetAttributeBooleanValue (isNotNull, ECDBSQL_NOTNULL);
    xmlNode.GetAttributeBooleanValue (isUnique, ECDBSQL_UNIQUE);
    xmlNode.GetAttributeStringValue (check, ECDBSQL_CHECK);
    xmlNode.GetAttributeStringValue (defaultStr, ECDBSQL_DEFAULT);
    xmlNode.GetAttributeStringValue (collate, ECDBSQL_COLLATE);
   

    PersistenceType persistenceType = PersistenceType::Persisted;
    if (xmlNode.GetAttributeUInt32Value (ptype, ECDBSQL_PERSISTENCE_TYPE) == BeXmlStatus::BEXML_Success)
        {
        persistenceType = static_cast<PersistenceType>(ptype);
        }
    xmlNode.GetAttributeUInt32Value (userFlag, ECDBSQL_USERFLAGS);

    auto column = ecdbSqlTable.CreateColumn (name.c_str (), StringToType (type.c_str ()), userFlag,  persistenceType);
    if (!check.empty ()) column->GetConstraintR ().SetCheckExpression (check.c_str ());
    if (!defaultStr.empty ()) column->GetConstraintR ().SetDefaultExpression (defaultStr.c_str ());
    if (!collate.empty ()) column->GetConstraintR ().SetCollate (Constraint::StringToCollate (collate.c_str ()));

    column->GetConstraintR ().SetIsNotNull (isNotNull);
    column->GetConstraintR ().SetIsUnique (isUnique);


    for (auto ref = xmlNode.GetFirstChild (); ref != nullptr; ref = ref->GetNextSibling ())
        {
        if (!ref->IsIName (ECDBSQL_REFS))
            return BentleyStatus::ERROR;
 
        xmlNode.GetAttributeUInt64Value (id, ECDBSQL_CLASSID);
        xmlNode.GetAttributeStringValue (name, ECDBSQL_Path);
        column->GetDependentPropertiesR ().Add (id, name.c_str());
        }


    return BentleyStatus::SUCCESS;
    }

//****************************************************************************************
//ECDbSqlConstraint
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSqlConstraint::WriteTo (BeXmlNodeR xmlNode) const
    {
    if (GetType () == Type::PrimaryKey)
        return static_cast<ECDbSqlPrimaryKeyConstraint const*>(this)->WriteTo (xmlNode);
    else if (GetType () == Type::ForiegnKey)
        return static_cast<ECDbSqlForiegnKeyConstraint const*>(this)->WriteTo (xmlNode);

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
//static
BentleyStatus ECDbSqlConstraint::ReadFrom (ECDbSqlTable& ecdbSqlTable, BeXmlNodeR xmlNode)
    {
    if (xmlNode.IsIName(ECDBSQL_PRIMARY_KEY))
        return ECDbSqlPrimaryKeyConstraint::ReadFrom (ecdbSqlTable, xmlNode);
    else if (xmlNode.IsIName (ECDBSQL_FORIEGNKEY))
        return ECDbSqlForiegnKeyConstraint::ReadFrom (ecdbSqlTable, xmlNode);

    return BentleyStatus::ERROR;
    }

//****************************************************************************************
//ECDbSQLPath
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
ECDbSqlPath::ECDbSqlPath (ECDbSqlDb& dbDef, Utf8CP tableName)
    :m_dbDef (dbDef), m_dataType (ECDbSqlColumn::Type::Any), m_column (nullptr), m_table (nullptr)
    {
    ResolveTable (tableName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ECDbSqlPath::ResolveTable (Utf8CP tableName)
    {
    if (m_table == nullptr)
        {
        m_table = m_dbDef.FindTableP (tableName);
        if (m_table == nullptr)
            {
            m_table = m_dbDef.CreateTable (tableName);
            if (!m_table)
                {
                BeAssert (false && "Failed to create table");
                return;
                }
            }
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
void ECDbSqlPath::ResolveColumn (Utf8CP columnName)
    {
    BeAssert (!Utf8String::IsNullOrEmpty (columnName));
    m_column = m_table->FindColumnP (columnName);
    if (m_column == nullptr)
        {

        m_column = m_table->CreateColumn (columnName, m_dataType);
        if (!m_column)
            {
            BeAssert (false && "Failed to create column");
            return;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
Utf8StringCR ECDbSqlPath::GetTableName () const
    {
    BeAssert (m_table != nullptr);
    return m_table->GetName ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        09/2014
//---------------------------------------------------------------------------------------
Utf8StringCR ECDbSqlPath::GetColumnName () const
    {
    BeAssert (m_column != nullptr);
    return m_column->GetName ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
void ECDbSqlPath::SetColumnName (Utf8CP name, ECDbSqlColumn::Type type)
    {
    m_dataType = type;
    ResolveColumn (name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
ECDbSqlTable const* ECDbSqlPath::GetTable () const
    {
    return m_table;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
ECDbSqlColumn const* ECDbSqlPath::GetColumn () const
    {
    return m_column;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus DependentPropertyCollection::Add (ECClassId ecClassId, WCharCP accessString)
    {
    return Add (ecClassId, Utf8String (accessString).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus DependentPropertyCollection::Add (ECClassId ecClassId, Utf8CP accessString)
    {
    //if (GetColumnR ().GetTableR ().GetEditHandleR ().AssertNotInEditMode ())
    //    return BentleyStatus::ERROR;

    if (Utf8String::IsNullOrEmpty (accessString))
        {
        BeAssert (false && "accessString should be not null");
        return BentleyStatus::ERROR;
        }

    if (Find (ecClassId) != nullptr)
        {
        //BeAssert (false && "accessString already exist");
        return BentleyStatus::SUCCESS;
        }

    m_map[ecClassId] = GetColumnR ().GetTableR ().GetDbDefR ().GetStringPoolR ().Set (accessString);
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus DependentPropertyCollection::Remove (ECClassId ecClassId)
    {
    if (GetColumnR ().GetTableR ().GetEditHandleR ().AssertNotInEditMode ())
        return BentleyStatus::ERROR;

    auto itor = m_map.find (ecClassId);
    if (itor != m_map.end ())
        {
        m_map.erase (itor);
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
Utf8CP DependentPropertyCollection::Find (ECClassId ecClassId) const
    {
    auto itor = m_map.find (ecClassId);
    if (itor != m_map.end ())
        {
        return itor->second;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
bool DependentPropertyCollection::Contains (ECClassId ecClassId) const
    {
    return Find (ecClassId) != nullptr;
    }
   
//****************************************************************************************
//ECDbSQLManager
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan        10/2014
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSQLManager::Load ()
    {
    Utf8String xml;
    if (GetECDbR ().QueryProperty (xml, m_propertySpec, ECDBSQL_VERSION_MAJOR, ECDBSQL_VERSION_MINOR) != BE_SQLITE_OK)
        return BentleyStatus::ERROR;

    if (ECDbSqlDb::ReadFrom (m_defaultDb, xml) != BentleyStatus::SUCCESS)
        return BentleyStatus::ERROR;

    SetupNullTable ();
    return BentleyStatus::SUCCESS;
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
//---------------------------------------------------------------------------------------
BentleyStatus ECDbSQLManager::Save ()
    {
    if (GetECDb ().IsReadonly ())
        {
        BeAssert (false && "Db is readonly");
        return BentleyStatus::ERROR;
        }

    Utf8String xml;
    if (GetDbSchema ().WriteTo (xml) == BentleyStatus::SUCCESS)
        {
        if (GetECDbR ().SavePropertyString (m_propertySpec, xml, ECDBSQL_VERSION_MAJOR, ECDBSQL_VERSION_MINOR) != BE_SQLITE_OK)
            return BentleyStatus::ERROR;

        return BentleyStatus::SUCCESS;
        }

    BeAssert (false && "Failed to serialize to xml");
    return BentleyStatus::ERROR;
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
bool ECDbSqlHelper::IsCompatiable (ECDbSqlColumn::Type target, ECDbSqlColumn::Type source)
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

END_BENTLEY_SQLITE_EC_NAMESPACE
