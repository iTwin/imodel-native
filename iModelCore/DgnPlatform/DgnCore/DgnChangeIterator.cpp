/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnChangeIterator.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "DgnChangeIterator.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
BaseChangeEntry::BaseChangeEntry(BaseChangeIterator const& parent, BeSQLite::Changes::Change sqlChange) : m_parent(parent), m_sqlChange(sqlChange)
    {
    MoveToNextSqlChange();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
void BaseChangeEntry::MoveToNextSqlChange()
    {
    while (m_sqlChange.IsValid())
        {
        Utf8CP tableName;
        DbOpcode dbOpcode;
        int nCols;
        int indirect;

        DbResult rc = m_sqlChange.GetOperation(&tableName, &nCols, &dbOpcode, &indirect);
        BeAssert(rc == BE_SQLITE_OK);

        if (indirect == 0 && 0 == ::strcmp(tableName, m_parent.m_tableMap->GetTableName().c_str()))
            break;

        ++m_sqlChange;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
void BaseChangeEntry::_ExtractChanges()
    {
    if (!m_sqlChange.IsValid())
        {
        m_instanceId = ECInstanceId();
        m_oldCode = m_newCode = DgnCode();
        return;
        }

    m_dbOpcode = ExtractDbOpcode();
    m_instanceId = ExtractInstanceId();

    m_oldCode = (m_dbOpcode == DbOpcode::Insert) ? DgnCode() : ExtractCode(Changes::Change::Stage::Old);
    m_newCode = (m_dbOpcode == DbOpcode::Delete) ? DgnCode() : ExtractCode(Changes::Change::Stage::New);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
DbOpcode BaseChangeEntry::ExtractDbOpcode()
    {
    Utf8CP tableName;
    DbOpcode dbOpcode;
    int nCols;
    int indirect;

    DbResult rc = m_sqlChange.GetOperation(&tableName, &nCols, &dbOpcode, &indirect);
    BeAssert(rc == BE_SQLITE_OK);

    return dbOpcode;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
ECInstanceId BaseChangeEntry::ExtractInstanceId() const
    {
    DbValue idVal = (m_dbOpcode == DbOpcode::Insert) ? m_sqlChange.GetNewValue(m_parent.m_instanceIdColumnMap.GetIndex()) : m_sqlChange.GetOldValue(m_parent.m_instanceIdColumnMap.GetIndex());
    ECInstanceId instanceId = idVal.GetValueId<ECInstanceId>();
    BeAssert(instanceId.IsValid());
    return instanceId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
DgnCode BaseChangeEntry::ExtractCode(Changes::Change::Stage stage) const
    {
    DbValue codeAuthorityIdDbVal = m_sqlChange.GetValue(m_parent.m_codeAuthorityIdColumnMap.GetIndex(), stage);
    DbValue codeValueDbVal = m_sqlChange.GetValue(m_parent.m_codeValueColumnMap.GetIndex(), stage);
    DbValue codeNamespaceIndexDbVal = m_sqlChange.GetValue(m_parent.m_codeNamespaceColumnMap.GetIndex(), stage);

    if (codeAuthorityIdDbVal.IsValid() && codeValueDbVal.IsValid() && codeNamespaceIndexDbVal.IsValid())
        {
        DgnCode code;
        code.From(codeAuthorityIdDbVal.GetValueId<DgnAuthorityId>(), codeValueDbVal.GetValueText(), codeNamespaceIndexDbVal.GetValueText());
        return code;
        }
    
    if (m_dbOpcode != DbOpcode::Update)
        return DgnCode();

    DbDupValue codeAuthorityIdDbVal2(codeAuthorityIdDbVal.GetSqlValueP());
    if (!codeAuthorityIdDbVal2.IsValid())
        codeAuthorityIdDbVal2 = GetDbValue(m_parent.m_codeAuthorityIdColumnMap.GetName());

    DbDupValue codeValueDbVal2(codeValueDbVal.GetSqlValueP());
    if (!codeValueDbVal2.IsValid())
        codeValueDbVal2 = GetDbValue(m_parent.m_codeValueColumnMap.GetName());

    DbDupValue codeNamespaceIndexDbVal2(codeNamespaceIndexDbVal.GetSqlValueP());
    if (!codeNamespaceIndexDbVal2.IsValid())
        codeNamespaceIndexDbVal2 = GetDbValue(m_parent.m_codeNamespaceColumnMap.GetName());

    if (codeAuthorityIdDbVal2.IsValid() && codeValueDbVal2.IsValid() && codeNamespaceIndexDbVal2.IsValid())
        {
        DgnCode code;
        code.From(codeAuthorityIdDbVal2.GetValueId<DgnAuthorityId>(), codeValueDbVal2.GetValueText(), codeNamespaceIndexDbVal2.GetValueText());
        return code;
        }

    return DgnCode();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
DbDupValue BaseChangeEntry::GetDbValue(Utf8StringCR columnName) const
    {
    Utf8PrintfString sql("SELECT %s FROM %s WHERE %s=?", columnName.c_str(), m_parent.m_tableMap->GetTableName().c_str(), m_parent.m_instanceIdColumnMap.GetName().c_str());
    CachedStatementPtr statement = m_parent.m_dgndb.GetCachedStatement(sql.c_str());
    BeAssert(statement.IsValid());

    statement->BindId(1, m_instanceId);

    DbResult result = statement->Step();
    if (BE_SQLITE_ROW == result)
        return statement->GetDbValue(0);

    BeAssert(result == BE_SQLITE_DONE);
    return DbDupValue(nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
BaseChangeEntry& BaseChangeEntry::operator++()
    {
    BeAssert(m_sqlChange.IsValid());
    ++m_sqlChange;
    MoveToNextSqlChange();
    ExtractChanges();
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
BaseChangeIterator::BaseChangeIterator(DgnDbCR dgndb, IChangeSet& changeSet, ECClassCR ecClass) : m_dgndb(dgndb), m_changes(changeSet.GetChanges())
    {
    m_tableMap = ChangeSummary::GetPrimaryTableMap(dgndb, ecClass);
    BeAssert(m_tableMap.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
void BaseChangeIterator::_Initialize()
    {
    m_instanceIdColumnMap = m_tableMap->GetColumn("ECInstanceId");
    m_codeAuthorityIdColumnMap = m_tableMap->GetColumn("CodeAuthorityId");
    m_codeNamespaceColumnMap = m_tableMap->GetColumn("CodeNamespace");
    m_codeValueColumnMap = m_tableMap->GetColumn("CodeValue");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
ElementChangeEntry::ElementChangeEntry(ElementChangeIterator const& parent, BeSQLite::Changes::Change sqlChange) : m_parent(parent), T_Super(parent, sqlChange)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
void ElementChangeEntry::_ExtractChanges()
    {
    T_Super::_ExtractChanges();

    if (!m_sqlChange.IsValid())
        {
        m_oldModelId = m_newModelId = DgnModelId();
        return;
        }

    m_oldModelId = (m_dbOpcode == DbOpcode::Insert) ? DgnModelId() : ExtractModelId(Changes::Change::Stage::Old);
    m_newModelId = (m_dbOpcode == DbOpcode::Delete) ? DgnModelId() : ExtractModelId(Changes::Change::Stage::New);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
DgnModelId ElementChangeEntry::ExtractModelId(Changes::Change::Stage stage) const
    {
    DbValue idVal = m_sqlChange.GetValue(m_parent.m_modelIdColumnMap.GetIndex(), stage);

    if (idVal.IsValid())
        {
        DgnModelId modelId = idVal.GetValueId<DgnModelId>();
        return modelId;
        }

    if (m_dbOpcode != DbOpcode::Update)
        return DgnModelId();


    DbDupValue idVal2(idVal.GetSqlValueP());
    if (!idVal2.IsValid())
        idVal2 = GetDbValue(m_parent.m_modelIdColumnMap.GetName());

    if (idVal2.IsValid())
        {
        DgnModelId modelId = idVal2.GetValueId<DgnModelId>();
        return modelId;
        }

    return DgnModelId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
ElementChangeIterator::ElementChangeIterator(DgnDbCR dgndb, IChangeSet& changeSet) : T_Super(dgndb, changeSet, *(dgndb.Schemas().GetECClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Element)))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
void ElementChangeIterator::_Initialize()
    {
    T_Super::_Initialize();
    m_modelIdColumnMap = m_tableMap->GetColumn("ModelId");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
ElementChangeIterator::const_iterator ElementChangeIterator::begin() const
    {
    ElementChangeEntry entry(*this, m_changes.begin());
    entry.ExtractChanges();
    return entry;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
ElementChangeIterator::const_iterator ElementChangeIterator::end() const
    {
    ElementChangeEntry entry(*this, m_changes.end());
    entry.ExtractChanges();
    return entry;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
ModelChangeEntry::ModelChangeEntry(ModelChangeIterator const& parent, BeSQLite::Changes::Change sqlChange) : m_parent(parent), T_Super(parent, sqlChange)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
ModelChangeIterator::ModelChangeIterator(DgnDbCR dgndb, IChangeSet& changeSet) : T_Super(dgndb, changeSet, *(dgndb.Schemas().GetECClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Model)))
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
ModelChangeIterator::const_iterator ModelChangeIterator::begin() const
    {
    ModelChangeEntry entry(*this, m_changes.begin());
    entry.ExtractChanges();
    return entry;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
ModelChangeIterator::const_iterator ModelChangeIterator::end() const
    {
    ModelChangeEntry entry(*this, m_changes.end());
    entry.ExtractChanges();
    return entry;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
// static
ElementChangeIterator DgnChangeIterator::MakeElementChangeIterator(DgnDbCR dgndb, IChangeSet& changeSet)
    {
    ElementChangeIterator iter(dgndb, changeSet);
    iter.Initialize();
    return iter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Ramanujam.Raman     03/2016
//---------------------------------------------------------------------------------------
// static
ModelChangeIterator DgnChangeIterator::MakeModelChangeIterator(DgnDbCR dgndb, IChangeSet& changeSet)
    {
    ModelChangeIterator iter(dgndb, changeSet);
    iter.Initialize();
    return iter;
    }
