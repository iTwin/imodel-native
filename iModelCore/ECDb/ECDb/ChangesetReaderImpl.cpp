#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
const std::vector<Utf8String> *ChangesetReader::GetColumns(Utf8StringCR tableName) const {
    auto it = m_sqliteTableSchemaCache.find(tableName);
    if (it != m_sqliteTableSchemaCache.end())
        return &it->second;

    MetaData::CompleteTableInfo tableInfo;
    auto rc = MetaData::QueryTable(m_db, "main", tableName, tableInfo);
    if (rc != BE_SQLITE_OK)
        return nullptr;

    if (tableInfo.columns.empty())
        return nullptr;

    std::vector<Utf8String> columnNames;
    columnNames.reserve(tableInfo.columns.size());
    for (auto const &col : tableInfo.columns)
        columnNames.push_back(col.name);

    m_sqliteTableSchemaCache[tableName] = std::move(columnNames);
    return &m_sqliteTableSchemaCache[tableName];
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
int ChangesetReader::GetClassIdColumnIndex(Utf8StringCR tableName) const {
    auto it = m_classIdColumnIndexCache.find(tableName);
    if (it != m_classIdColumnIndexCache.end())
        return it->second;

    const auto columns = GetColumns(tableName);
    if (columns == nullptr)
        return -1;

    for (size_t i = 0; i < columns->size(); ++i) {
        if (columns->at(i) == ECDBSYS_PROP_ECClassId) {
            m_classIdColumnIndexCache[tableName] = static_cast<int>(i);
            return static_cast<int>(i);
        }
    }

    m_classIdColumnIndexCache[tableName] = -1;
    return -1;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECClassId ChangesetReader::FindClassIdFromDb(Utf8StringCR tableName, ECInstanceId instanceId) {
    auto stmt = m_db.GetCachedStatement(BeSQLite::SqlPrintfString("SELECT [" COL_ECClassId "] FROM [%s] WHERE [ROWID] = ?", tableName.c_str()));
    if (stmt == nullptr)
        return ECClassId();

    stmt->BindId(1, instanceId);
    if (stmt->Step() != BE_SQLITE_ROW)
        return ECClassId();

    return stmt->GetValueId<ECClassId>(0);
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ChangesetReader::ClearMetaData() {
    m_meta.fallbackClassId = ECClassId();
    m_meta.instanceId = ECInstanceId();
    m_meta.classId = ECClassId();
    m_meta.previousClassId = ECClassId();
}


//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ChangesetReader::Reset() {
    m_changes = nullptr;
    m_change = BeSQLite::Changes::Change(nullptr, false);
    ClearMetaData();
}


//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ChangesetReader::Status ChangesetReader::Step() {
    ClearMetaData();
    if (m_changes == nullptr) {
        m_changes = std::make_unique<BeSQLite::Changes>(m_changeset, false);
        m_change = m_changes->begin();
    } else {
        ++m_change;
    }

    if (!m_change.IsValid())
        return Status::DONE;

    // EC change will have table mapped in the DbSchema
    auto table = m_db.Schemas().Main().GetDbSchema().FindTable(m_change.GetTableName());
    while (table == nullptr) {
        ++m_change;
        if (!m_change.IsValid())
            return Status::DONE;
        table = m_db.Schemas().Main().GetDbSchema().FindTable(m_change.GetTableName());
    }

    const auto opCode = m_change.GetOpcode();
    const auto &ecClassIdColumn = table->GetECClassIdColumn();
    const auto instanceId = (opCode == DbOpcode::Insert) ? m_change.GetNewValue(0).GetValueId<ECInstanceId>() : m_change.GetOldValue(0).GetValueId<ECInstanceId>();

    if (!instanceId.IsValid()) {
        return Status::ERROR_FAILED_TO_DECODE_INSTANCE_ID;
    }

    const auto fallbackClassId = table->GetExclusiveRootECClassId();
    ECClassId classId;
    if (ecClassIdColumn.IsVirtual()) {
        classId = table->GetExclusiveRootECClassId();
    } else {
        const int classIdColIndex = GetClassIdColumnIndex(m_change.GetTableName());
        if (classIdColIndex == -1) {
            return Status::ERROR_FAILED_TO_DECODE_CLASS_ID;
        }

        if (opCode == BeSQLite::DbOpcode::Delete) {
            auto classIdVal = m_change.GetOldValue(classIdColIndex);
            if (classIdVal.IsNull()) {
                return Status::ERROR_FAILED_TO_DECODE_CLASS_ID;
            }
            classId = classIdVal.GetValueId<ECClassId>();
        } else if (opCode == BeSQLite::DbOpcode::Insert) {
            auto classIdVal = m_change.GetNewValue(classIdColIndex);
            if (classIdVal.IsNull()) {
                return Status::ERROR_FAILED_TO_DECODE_CLASS_ID;
            }
            classId = classIdVal.GetValueId<ECClassId>();
        } else {
            auto newVal = m_change.GetNewValue(classIdColIndex);
            auto oldVal = m_change.GetOldValue(classIdColIndex);

            if (newVal.IsNull() && oldVal.IsNull()) {
                classId = FindClassIdFromDb(m_change.GetTableName(), instanceId);
            }

            if (!newVal.IsNull() && !oldVal.IsNull()) {
                classId = newVal.GetValueId<ECClassId>();
                m_meta.previousClassId = oldVal.GetValueId<ECClassId>();
                return Status::ROW;
            }
        }
    }

    m_meta.fallbackClassId = fallbackClassId;
    m_meta.instanceId = instanceId;
    m_meta.classId = classId;
    return Status::ROW;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ChangesetReader::Status ChangesetReader::SetClassMap() {
    const auto classId = GetClassId().IsValid() ? GetClassId() : GetFallbackClassId();

    auto ecClass = m_db.Schemas().GetClass(classId);
    if (ecClass == nullptr)
        return Status::ERROR_ClASS_NOT_FOUND;

    auto classMap = m_db.Schemas().Main().GetClassMap(*ecClass);
    if (classMap == nullptr || classMap->GetType() == ClassMap::Type::NotMapped)
        return Status::ERROR_CLASSMAP_NOT_FOUND;

    for (auto propertyMap : classMap->GetPropertyMaps()) {
        if (propertyMap->GetType() == PropertyMap::Type::Point2d) {
            const auto &pt2dMap = propertyMap->GetAs<Point2dPropertyMap>();
        } else if (propertyMap->GetType() == PropertyMap::Type::Point3d) {
            const auto &pt3dMap = propertyMap->GetAs<Point3dPropertyMap>();        
        } else {
            const auto &primMap = propertyMap->GetAs<SingleColumnDataPropertyMap>();
        }
    }
    return Status::ROW;
}

END_BENTLEY_SQLITE_EC_NAMESPACE