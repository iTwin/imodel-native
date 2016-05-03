/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnChangeIterator.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*
These change iterators are only meant for internal use (extracting codes/locks from revisions) - 
they provide a way to iterate over specific changes to Element (Code, ModelId) and Model (Code) instances. 

The routines make simplifying assumptions about the mapping for these specific cases. 
See ChangeSummary-s for a more extensive treatment. 
*/
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct BaseChangeIterator;
struct ElementChangeIterator;
struct ModelChangeIterator;

//=======================================================================================
// @bsiclass                                                  Ramanujam.Raman   03/16
//=======================================================================================
struct BaseChangeEntry
{
private:
    BaseChangeIterator const& m_parent;

    void MoveToNextSqlChange();

    DbOpcode ExtractDbOpcode();
    ECInstanceId ExtractInstanceId() const;
    DgnCode ExtractCode(Changes::Change::Stage stage) const;

protected:
    BeSQLite::Changes::Change m_sqlChange;
    DbOpcode m_dbOpcode;
    ECInstanceId m_instanceId;
    DgnCode m_oldCode;
    DgnCode m_newCode;

    DbDupValue GetDbValue(Utf8StringCR columnName) const;
    BaseChangeEntry& operator++();

    virtual void _ExtractChanges();

public:
    BaseChangeEntry(BaseChangeIterator const& parent, BeSQLite::Changes::Change sqlChange);
    
    void ExtractChanges() { _ExtractChanges(); }

    DbOpcode GetDbOpcode() const { return m_dbOpcode; }
    ECInstanceId GetECInstanceId() const { return m_instanceId; }
    DgnCode GetOldCode() const { return m_oldCode; }
    DgnCode GetNewCode() const { return m_newCode; }
};

//=======================================================================================
// @bsiclass                                                  Ramanujam.Raman   03/16
//=======================================================================================
struct BaseChangeIterator
{
    friend struct BaseChangeEntry;
private:
    ChangeSummary::ColumnMap m_instanceIdColumnMap;
    ChangeSummary::ColumnMap m_codeAuthorityIdColumnMap;
    ChangeSummary::ColumnMap m_codeNamespaceColumnMap;
    ChangeSummary::ColumnMap m_codeValueColumnMap;
protected:
    DgnDbCR m_dgndb;
    Changes m_changes;
    ChangeSummary::TableMapPtr m_tableMap;

    BaseChangeIterator(DgnDbCR dgndb, IChangeSet& changeSet, ECN::ECClassCR ecClass);
    virtual void _Initialize();
public:
    void Initialize() { _Initialize(); }
};

//=======================================================================================
// @bsiclass                                                  Ramanujam.Raman   03/16
//=======================================================================================
struct ElementChangeEntry : BaseChangeEntry
{
    DEFINE_T_SUPER(BaseChangeEntry);
private:
    ElementChangeIterator const& m_parent;
    DgnModelId m_oldModelId;
    DgnModelId m_newModelId;

    DgnModelId ExtractModelId(Changes::Change::Stage stage) const;

protected:
    virtual void _ExtractChanges() override;

public:
    ElementChangeEntry(ElementChangeIterator const& parent, BeSQLite::Changes::Change sqlChange);

    DgnElementId GetElementId() const { return DgnElementId(m_instanceId.GetValueUnchecked()); }
    DgnModelId GetOldModelId() const { return m_oldModelId; }
    DgnModelId GetNewModelId() const { return m_newModelId; }

    ElementChangeEntry& operator++() {T_Super::operator++(); return *this;}
    ElementChangeEntry const& operator* () const { return *this; }
    bool operator!=(ElementChangeEntry const& rhs) const { return m_sqlChange != rhs.m_sqlChange; }
    bool operator==(ElementChangeEntry const& rhs) const { return m_sqlChange == rhs.m_sqlChange; }
};

//=======================================================================================
// @bsiclass                                                  Ramanujam.Raman   03/16
//=======================================================================================
struct ElementChangeIterator : BaseChangeIterator
{
    DEFINE_T_SUPER(BaseChangeIterator);
    friend struct ElementChangeEntry;
    typedef ElementChangeEntry const_iterator;
    typedef const_iterator iterator;

private:
    ChangeSummary::ColumnMap m_modelIdColumnMap;

protected:
    virtual void _Initialize() override;

public:
    ElementChangeIterator(DgnDbCR dgndb, IChangeSet& changeSet);
    const_iterator begin() const;
    const_iterator end() const;
};

//=======================================================================================
// @bsiclass                                                  Ramanujam.Raman   03/16
//=======================================================================================
struct ModelChangeEntry : BaseChangeEntry
{
    DEFINE_T_SUPER(BaseChangeEntry);

private:
    ModelChangeIterator const& m_parent;

public:
    ModelChangeEntry(ModelChangeIterator const& parent, BeSQLite::Changes::Change sqlChange);
    DgnModelId GetModelId() const { return DgnModelId(m_instanceId.GetValueUnchecked()); }

    ModelChangeEntry& operator++() {T_Super::operator++(); return *this;}
    ModelChangeEntry const& operator* () const { return *this; }
    bool operator!=(ModelChangeEntry const& rhs) const { return m_sqlChange != rhs.m_sqlChange; }
    bool operator==(ModelChangeEntry const& rhs) const { return m_sqlChange == rhs.m_sqlChange; }
};

//=======================================================================================
// @bsiclass                                                  Ramanujam.Raman   03/16
//=======================================================================================
struct ModelChangeIterator : BaseChangeIterator
{
    DEFINE_T_SUPER(BaseChangeIterator);
    typedef ModelChangeEntry const_iterator;
    typedef const_iterator iterator;

public:
    ModelChangeIterator(DgnDbCR dgndb, IChangeSet& changeSet);
    const_iterator begin() const;
    const_iterator end() const;
};

//=======================================================================================
// @bsiclass                                                  Ramanujam.Raman   03/16
//=======================================================================================
struct DgnChangeIterator
{
public:
    static ElementChangeIterator MakeElementChangeIterator(DgnDbCR dgndb, IChangeSet& changeSet);
    static ModelChangeIterator MakeModelChangeIterator(DgnDbCR dgndb, IChangeSet& changeSet);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
