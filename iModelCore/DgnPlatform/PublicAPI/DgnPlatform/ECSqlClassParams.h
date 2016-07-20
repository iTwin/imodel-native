/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ECSqlClassParams.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_DGN_NAMESPACE

struct DgnElement;

//=======================================================================================
//! Interface adopted by a DgnDomain::Handler which can supply ECSqlClassParams
// @bsiclass                                                     Paul.Connelly   09/15
//=======================================================================================
struct IECSqlClassParamsProvider
{
    virtual void _GetClassParams(ECSqlClassParamsR ecSqlParams) = 0;
};

//=======================================================================================
//! Encapsulates ECSql statement strings specific to an ECClass.
// @bsiclass                                                     Paul.Connelly   09/15
//=======================================================================================
struct ECSqlClassInfo
{
private:
    friend struct ECSqlClassParams;

    Utf8String  m_select;
    Utf8String  m_insert;
    Utf8String  m_update;
    uint16_t    m_updateParameterIndex;
public:
    ECSqlClassInfo() : m_updateParameterIndex(0xffff) { }

    Utf8StringCR GetSelectECSql() const { return m_select; }
    Utf8StringCR GetInsertECSql() const { return m_insert; }
    Utf8StringCR GetUpdateECSql() const { return m_update; }

    BeSQLite::EC::CachedECSqlStatementPtr GetSelectStmt(DgnDbCR dgndb, BeSQLite::EC::ECInstanceId instanceId) const;
    BeSQLite::EC::CachedECSqlStatementPtr GetInsertStmt(DgnDbCR dgndb) const;
    BeSQLite::EC::CachedECSqlStatementPtr GetUpdateStmt(DgnDbCR dgndb, BeSQLite::EC::ECInstanceId instanceId) const;
};

//=======================================================================================
//! A list of parameters used in ECSql SELECT, INSERT, and UPDATE statements for a
//! specific ECClass. Maps names to indices in the results of a SELECT statement or in
//! the bindings of an INSERT or UPDATE statement.
//! @ingroup GROUP_DgnElement
// @bsiclass                                                     Paul.Connelly   09/15
//=======================================================================================
struct ECSqlClassParams
{
    friend struct DgnElement;
public:
    enum class StatementType
        {
        None = 0, //!< Property should not be included in any statement -- it has completely custom I/O
        Select = 1 << 0, //!< Property should be included in SELECT statements from DgnElement::_LoadFromDb()
        Insert = 1 << 1, //!< Property should be included in INSERT statements from DgnElement::_InsertInDb()
        Update = 1 << 2, //!< Property should be included in UPDATE statements from DgnElement::_UpdateInDb()
        ReadOnly = Select | Insert, //!< Property cannot be modified via UPDATE statement
        All = Select | Insert | Update, //!< Property should be included in all ECSql statements
        InsertUpdate = Insert | Update, //!< Property should not be included in SELECT statements
        };

    struct Entry
        {
        Utf8String      m_name;
        StatementType   m_type;

        Entry() : m_type(StatementType::All) {}
        Entry(Utf8StringCR name, StatementType type) : m_name(name), m_type(type) {}
        };

    typedef bvector<Entry> Entries;

private:
    Entries     m_entries;
    Utf8String  m_selectTemplate;
    Utf8String  m_insertTemplate;
    Utf8String  m_updateTemplate;
    uint16_t    m_numUpdateParams;
    bool        m_initialized;

    Entries const& GetEntries() const { return m_entries; }

    void RemoveAllButSelect();
    static bool AppendClassName(Utf8StringR className, DgnDbCR db, DgnClassId classId);
public:
    ECSqlClassParams() : m_numUpdateParams(0), m_initialized(false) { }

    bool IsInitialized() const { return m_initialized; }
    void Initialize(IECSqlClassParamsProvider& paramsProvider);
    uint16_t GetNumUpdateParams() const { return m_numUpdateParams; }

    bool BuildInsertECSql(Utf8StringR ecsql, DgnDbCR dgndb, DgnClassId classId) const;
    bool BuildUpdateECSql(Utf8StringR ecsql, DgnDbCR dgndb, DgnClassId classId) const;
    bool BuildSelectECSql(Utf8StringR ecsql, DgnDbCR dgndb, DgnClassId classId) const;
    bool BuildClassInfo(ECSqlClassInfo& info, DgnDbCR dgndb, DgnClassId classId) const;

    //! Adds a parameter to the list
    //! @param[in]      parameterName The name of the parameter.
    //! @param[in]      type          The type(s) of statements in which this parameter is used.
    DGNPLATFORM_EXPORT void Add(Utf8StringCR parameterName, StatementType type = StatementType::All);

    //! Returns an index usable for accessing the columns with the specified name in the results of an ECSql SELECT query.
    //! @param[in]      parameterName The name of the parameter
    //! @return The index of the corresponding column in the query results, or -1 if no such column exists
    DGNPLATFORM_EXPORT int GetSelectIndex(Utf8StringCR parameterName) const;

};

ENUM_IS_FLAGS(ECSqlClassParams::StatementType);

END_BENTLEY_DGN_NAMESPACE
