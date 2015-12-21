/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ECSqlClassParams.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! A list of parameters used in ECSql SELECT, INSERT, and UPDATE statements for a
//! specific ECClass. Maps names to indices in the results of a SELECT statement or in
//! the bindings of an INSERT or UPDATE statement.
//! @ingroup DgnElementGroup
// @bsiclass                                                     Paul.Connelly   09/15
//=======================================================================================
struct ECSqlClassParams
{
friend struct ECSqlClassInfo;
public:
    enum class StatementType
        {
        Select = 1 << 0, //!< Property should be included in SELECT statements from DgnElement::_LoadFromDb()
        Insert = 1 << 1, //!< Property should be included in INSERT statements from DgnElement::_InsertInDb()
        Update = 1 << 2, //!< Property should be included in UPDATE statements from DgnElement::_UpdateInDb()
        ReadOnly = Select | Insert, //!< Property cannot be modified via UPDATE statement
        All = Select | Insert | Update, //!< Property should be included in all ECSql statements
        InsertUpdate = Insert | Update, //!< Property should not be included in SELECT statements
        };

    struct Entry
        {
        Utf8CP          m_name;
        StatementType   m_type;

        Entry() : m_name(nullptr), m_type(StatementType::All) {}
        Entry(Utf8CP name, StatementType type) : m_name(name), m_type(type) {}
        };

    typedef bvector<Entry> Entries;

private:
    Entries m_entries;

    Entries const& GetEntries() const { return m_entries; }

    void RemoveAllButSelect();
public:
    //! Adds a parameter to the list
    //! @param[in]      parameterName The name of the parameter. @em Must be a pointer to a string with static storage duration.
    //! @param[in]      type          The type(s) of statements in which this parameter is used.
    DGNPLATFORM_EXPORT void Add(Utf8CP parameterName, StatementType type = StatementType::All);

    //! Returns an index usable for accessing the columns with the specified name in the results of an ECSql SELECT query.
    //! @param[in]      parameterName The name of the parameter
    //! @return The index of the corresponding column in the query results, or -1 if no such column exists
    DGNPLATFORM_EXPORT int GetSelectIndex(Utf8CP parameterName) const;
};

ENUM_IS_FLAGS(ECSqlClassParams::StatementType);

//=======================================================================================
//! Stores information about an ECClass used in ECSql SELECT, INSERT, and UPDATE statements.
//! An ECSqlClassInfo is constructed once and cached for each ElementHandler/ModelHandler, 
//! based onthe results of _GetClassParams(). The cached information is
//! subsequently used for efficient CRUD operations when loading, inserting, and updating 
//! elements or models in the DgnDb.
//! @ingroup DgnElementGroup
// @bsiclass                                                     Paul.Connelly   09/15
//=======================================================================================
struct ECSqlClassInfo
{
private:
    Utf8String m_select;
    Utf8String m_insert;
    Utf8String m_update;
    ECSqlClassParams m_params;
    uint16_t m_numUpdateParams;
    bool m_initialized;

public:
    ECSqlClassInfo() : m_numUpdateParams(0), m_initialized(false) {}

    void Initialize(Utf8StringCR fullClassName, ECSqlClassParamsCR ecSqlParams);
    bool IsInitialized() const { return m_initialized; }

    Utf8StringCR GetInsertECSql() const { return m_insert; }
    Utf8StringCR GetSelectECSql() const { return m_select; }
    Utf8StringCR GetUpdateECSql() const { return m_update; }

    BeSQLite::EC::CachedECSqlStatementPtr GetInsertStmt(DgnDbCR dgndb) const;
    BeSQLite::EC::CachedECSqlStatementPtr GetSelectStmt(DgnDbCR dgndb, BeSQLite::EC::ECInstanceId id) const;
    BeSQLite::EC::CachedECSqlStatementPtr GetUpdateStmt(DgnDbCR dgndb, BeSQLite::EC::ECInstanceId id) const;

    ECSqlClassParamsCR GetParams() const { return m_params; }
    uint16_t GetNumUpdateParams() const { return m_numUpdateParams; }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
