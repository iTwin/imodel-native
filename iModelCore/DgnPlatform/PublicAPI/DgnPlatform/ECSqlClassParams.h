/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    typedef std::function<DgnDbStatus(ECN::ECValueR, DgnElement const&)> T_ElementPropGet;
    typedef std::function<DgnDbStatus(DgnElement&, ECN::ECValueCR)> T_ElementPropSet;
    typedef std::function<DgnDbStatus(DgnElement&, ECN::ECValueCR)> T_ElementPropValidator;

private:
    friend struct ECSqlClassParams;
    friend struct DgnElements;

    Utf8String  m_select;
    Utf8String  m_selectEcProps; // lazy-initialized by DgnElements::GetSelectEcPropsECSql using its mutex
    Utf8String  m_insert;
    Utf8String  m_update;
    uint16_t    m_updateParameterIndex;
    bmap<uint32_t, uint32_t> m_autoPropertyStatementType; // non-default statement types found for auto-handled properties
    bmap<uint32_t, bpair<T_ElementPropGet, T_ElementPropSet>> m_propertyAccessors; // custom-handled property get and set functions
    bmap<uint32_t, T_ElementPropValidator> m_autoPropertyValidators; // auto-handled property custom validation functions

public:
    //! @private
    ECSqlClassInfo() : m_updateParameterIndex(0xffff) { }

    //! @private
    Utf8StringCR GetSelectECSql() const { return m_select; }
    //! @private
    Utf8StringCR GetInsertECSql() const { return m_insert; }
    //! @private
    Utf8StringCR GetUpdateECSql() const { return m_update; }

    //! @private
    BeSQLite::EC::CachedECSqlStatementPtr GetSelectStmt(DgnDbCR dgndb, BeSQLite::EC::ECInstanceId instanceId) const;
    //! @private
    BeSQLite::EC::CachedECSqlStatementPtr GetInsertStmt(DgnDbCR dgndb) const;
    //! @private
    BeSQLite::EC::CachedECSqlStatementPtr GetUpdateStmt(DgnDbCR dgndb, BeSQLite::EC::ECInstanceId instanceId) const;

    //! Register the C++ functions that should be used to access custom-handled properties by name. @see DgnElement::_GetPropertyValue
    //! @param layout The layout of the element's class
    //! @param propName The name of the property
    //! @param getFunc The function to get the property value
    //! @param setFunc The function to set the property value
    DGNPLATFORM_EXPORT void RegisterPropertyAccessors(ECN::ClassLayout const& layout, Utf8CP propName, T_ElementPropGet getFunc, T_ElementPropSet setFunc);

    //! @private
    bpair<T_ElementPropGet,T_ElementPropSet> const* GetPropertyAccessors(uint32_t propIdx) const;

    //! Register a C++ function that should be used to validate auto-handled property values when they are set. @see DgnElement::_SetPropertyValue
    //! @param layout The layout of the element's class
    //! @param propName The name of the property
    //! @param validator The function to validate new property values before they are set
    void RegisterPropertyValidator(ECN::ClassLayout const& layout, Utf8CP propName, T_ElementPropValidator validator);

    //! @private
    T_ElementPropValidator const* GetPropertyValidator(uint32_t propIdx) const;

    //! @private
    uint32_t GetAutoPropertyStatementType(uint32_t propIdx);
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
        uint16_t        m_sortPriority;

        Entry(Utf8StringCR name, StatementType type, uint16_t sortPriority) : m_name(name), m_type(type), m_sortPriority(sortPriority) {}
        };

    typedef bvector<Entry> Entries;

private:
    Entries     m_entries;
    bool        m_initialized;

    Entries const& GetEntries() const { return m_entries; }
    uint16_t BuildInsertECSql(Utf8StringR ecsql, DgnDbCR dgndb, ECN::ECClassCR ecclass) const; // Build INSERT ECSql returning the number of INSERT params
    uint16_t BuildUpdateECSql(Utf8StringR ecsql, DgnDbCR dgndb, ECN::ECClassCR ecclass) const; // Build UPDATE ECSql returning the number of UPDATE params
    uint16_t BuildSelectECSql(Utf8StringR ecsql, DgnDbCR dgndb, ECN::ECClassCR ecclass) const; // BUILD SELECT ECSql returning the number of SELECT params

    static void AppendClassName(Utf8StringR className, DgnDbCR db, ECN::ECClassCR ecclass);
public:
    ECSqlClassParams() : m_initialized(false) { }

    bool IsInitialized() const { return m_initialized; }
    void Initialize(IECSqlClassParamsProvider& paramsProvider);
    bool BuildClassInfo(ECSqlClassInfo& info, DgnDbCR dgndb, DgnClassId classId) const;

    //! Adds a parameter to the list
    //! @param[in] parameterName The name of the parameter.
    //! @param[in] type The type(s) of statements in which this parameter is used.
    //! @param[in] sortPriority Use to sort SELECT properties according to when they were added to schemas. Newer properties must be higher than older properties.
    DGNPLATFORM_EXPORT void Add(Utf8StringCR parameterName, StatementType type = StatementType::All, uint16_t sortPriority = 0);

    //! Returns an index usable for accessing the columns with the specified name in the results of an ECSql SELECT query.
    //! @param[in]      parameterName The name of the parameter
    //! @return The index of the corresponding column in the query results, or -1 if no such column exists
    DGNPLATFORM_EXPORT int GetSelectIndex(Utf8CP parameterName) const;
};

ENUM_IS_FLAGS(ECSqlClassParams::StatementType);

END_BENTLEY_DGN_NAMESPACE
