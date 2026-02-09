/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/ChangedIdsIterator.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ChangedIdsIterator::SqlChange final : public ChangeIterator::SqlChange {
public:
    using ChangeIterator::SqlChange::SqlChange;
};

//******************************************************************************
// ChangedIdsIterator
//******************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangedIdsIterator::ChangedIdsIterator(ECDbCR ecdb, Changes const& changes)
    : m_ecdb(ecdb), m_changes(changes), m_tableMaps()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
const ChangedIdsIterator::TableMap& ChangedIdsIterator::GetTableMap(Utf8StringCR tableName) const
    {
    const auto map = m_tableMaps.find(tableName);
    if (map != m_tableMaps.end())
        return map->second;

    const auto newMap = m_tableMaps.emplace(tableName, TableMap(m_ecdb, tableName));
    return newMap.first->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangedIdsIterator::const_iterator ChangedIdsIterator::begin() const { return const_iterator(*this, m_changes.begin()); }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangedIdsIterator::const_iterator ChangedIdsIterator::end() const { return const_iterator(*this, m_changes.end()); }


//******************************************************************************
// ChangedIdsIterator::const_iterator
//******************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangedIdsIterator::iterator::iterator(ChangedIdsIterator const& iterator, Changes::Change const& change) : m_iterator(iterator), m_change(change), m_sqlChange(nullptr), m_tableMap(nullptr)
    {
    Initialize();
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangedIdsIterator::iterator::iterator(ChangedIdsIterator::iterator const& other) : m_iterator(other.m_iterator), m_change(other.m_change)
    {
    *this = other;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangedIdsIterator::iterator& ChangedIdsIterator::iterator::operator=(ChangedIdsIterator::iterator const& other)
    {
    m_change = other.m_change;
    m_tableMap = other.m_tableMap;
    m_primaryInstanceId = other.m_primaryInstanceId;
    FreeSqlChange();
    m_sqlChange = new SqlChange(m_change);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangedIdsIterator::iterator::~iterator() { FreeSqlChange(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangedIdsIterator::iterator::Initialize()
    {
    if (!m_change.IsValid())
        return;

    m_sqlChange = new SqlChange(m_change);

    m_tableMap = &m_iterator.GetTableMap(m_sqlChange->GetTableName());
    BeAssert(m_tableMap != nullptr);

    if (m_tableMap->isMapped)
        InitPrimaryInstance();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangedIdsIterator::iterator::ReInitialize()
    {
    Reset();
    Initialize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangedIdsIterator::iterator::Reset()
    {
    FreeSqlChange();
    m_tableMap = nullptr;
    m_primaryInstanceId.Invalidate();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangedIdsIterator::iterator::FreeSqlChange()
    {
    if (m_sqlChange == nullptr)
        return;

    delete m_sqlChange;
    m_sqlChange = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangedIdsIterator::iterator::InitPrimaryInstance()
    {
    BeAssert(m_tableMap->isMapped);
    m_primaryInstanceId = m_sqlChange->GetValueId<ECInstanceId>(m_tableMap->instanceIdColumnMap.columnIndex);
    BeAssert(m_primaryInstanceId.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8StringCR ChangedIdsIterator::iterator::GetTableName() const
    {
    BeAssert(IsValid());
    return m_tableMap->tableName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ChangedIdsIterator::iterator::IsPrimaryTable() const
    {
    if (!IsValid())
        {
        BeAssert(false);
        return false;
        }

    return m_tableMap->m_dbTable->GetType() == DbTable::Type::Primary;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbOpcode ChangedIdsIterator::iterator::GetDbOpcode() const
    {
    BeAssert(m_sqlChange != nullptr);
    return m_sqlChange->GetDbOpcode();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int ChangedIdsIterator::iterator::GetIndirect() const
    {
    BeAssert(m_sqlChange != nullptr);
    return m_sqlChange->GetIndirect();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangedIdsIterator::iterator& ChangedIdsIterator::iterator::operator++()
    {
    ++m_change;
    ReInitialize();
    return *this;
    }


//******************************************************************************
// ChangedIdsIterator::TableClassMap
//******************************************************************************

//******************************************************************************
// ChangedIdsIterator::TableMap
//******************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangedIdsIterator::TableMap::Initialize(ECDbCR ecdb, Utf8StringCR tableName)
    {
    DbSchema const& dbSchema = ecdb.Schemas().Main().GetDbSchema();
    this->tableName = tableName;

    DbTable const* dbTable = dbSchema.FindTable(tableName);
    if (!dbTable || !dbTable->IsValid() || dbSchema.IsNullTable(*dbTable))
        {
        isMapped = false;
        return;
        }

    isMapped = true;
    m_dbTable = dbTable;

    InitSystemColumnMaps();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangedIdsIterator::TableMap::InitSystemColumnMaps()
    {
    DbColumn const* instanceIdColumn = m_dbTable->FindFirst(DbColumn::Kind::ECInstanceId);
    auto instanceIdColumnIndex = instanceIdColumn->DeterminePosition();
    const auto& instanceIdColumnName = instanceIdColumn->GetName();
    instanceIdColumnMap = ColumnMap{instanceIdColumnName, instanceIdColumnIndex};
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
