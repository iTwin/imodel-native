/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/IECSqlBinder.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECSQLSYS_PARAM_FORMAT "_ecdb_ecsqlparam_ix%d"
#define ECSQLSYS_PARAM_Id "_ecdb_ecsqlparam_id"

#define ECSQLSYS_SQLPARAM_FORMAT "_ecdb_sqlparam_ix%d"
#define ECSQLSYS_SQLPARAM_Id "_ecdb_sqlparam_id"

struct SingleECSqlPreparedStatement;

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlBinder : IECSqlBinder
    {
    public:
        struct SqlParamNameGenerator final
            {
        private:
            Utf8String m_nameRoot;
            int m_currentSuffix = 0;

            //not copyable
            SqlParamNameGenerator(SqlParamNameGenerator const&) = delete;
            SqlParamNameGenerator& operator=(SqlParamNameGenerator const&) = delete;

        public:
            SqlParamNameGenerator(ECSqlPrepareContext& ctx, Utf8CP nameRoot) : m_nameRoot(nameRoot) 
                {
                if (m_nameRoot.empty())
                    m_nameRoot.Sprintf(ECSQLSYS_SQLPARAM_FORMAT, ctx.IncrementSystemSqlParameterSuffix());
                }

            Utf8String GetNextName()
                {
                BeAssert(!m_nameRoot.empty());
                //suffix is 1-based
                m_currentSuffix++;
                Utf8String nextName;
                nextName.Sprintf(":%s_col%d", m_nameRoot.c_str(), m_currentSuffix);
                return nextName;
                }
            };
    
    protected:
        SingleECSqlPreparedStatement& m_preparedStatement;

    private:
        ECSqlTypeInfo m_typeInfo;
        std::vector<Utf8String> m_mappedSqlParameterNames;
        bool m_hasToCallOnBeforeStep = false;
        bool m_hasToCallOnClearBindings = false;

        virtual ECSqlStatus _OnBeforeStep() { return ECSqlStatus::Success; }
        virtual void _OnClearBindings() {}

    protected:
        ECSqlBinder(ECSqlPrepareContext&, ECSqlTypeInfo const&, SqlParamNameGenerator&, int mappedSqlParameterCount, bool hasToCallOnBeforeStep, bool hasToCallOnClearBindings);
        //! Use this ctor for compound binders where the mapped sql parameter count depends on its member binders
        ECSqlBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, SqlParamNameGenerator& paramNameGen, bool hasToCallOnBeforeStep, bool hasToCallOnClearBindings) : ECSqlBinder(ctx, typeInfo, paramNameGen, -1, hasToCallOnBeforeStep, hasToCallOnClearBindings) {}

        void AddChildMemberMappedSqlParameterIndices(ECSqlBinder const& memberBinder)
            {
            std::vector<Utf8String> const& memberBinderMappedParameterNames = memberBinder.GetMappedSqlParameterNames();
            m_mappedSqlParameterNames.insert(m_mappedSqlParameterNames.end(), memberBinderMappedParameterNames.begin(), memberBinderMappedParameterNames.end());
            }

        ECSqlStatus LogSqliteError(DbResult sqliteStat, Utf8CP errorMessageHeader = nullptr) const;

        Statement& GetSqliteStatement() const;
        ECDbCR GetECDb() const;
        static Statement::MakeCopy ToBeSQliteBindMakeCopy(IECSqlBinder::MakeCopy makeCopy);

    public:
        virtual ~ECSqlBinder() {}

        bool HasToCallOnBeforeStep() const { return m_hasToCallOnBeforeStep; }
        bool HasToCallOnClearBindings() const { return m_hasToCallOnClearBindings; }

        std::vector<Utf8String> const& GetMappedSqlParameterNames() const { return m_mappedSqlParameterNames; }

        ECSqlTypeInfo const& GetTypeInfo() const { return m_typeInfo; }

        ECSqlStatus OnBeforeStep() { return _OnBeforeStep(); }
        void OnClearBindings() { return _OnClearBindings(); }
    };

struct IdECSqlBinder;
struct IdSetBinder;

//=======================================================================================
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlBinderFactory final
    {
    private:
        ECSqlBinderFactory();
        ~ECSqlBinderFactory();

        static bool RequiresNoopBinder(ECSqlPrepareContext&, PropertyMap const&, ECSqlSystemPropertyInfo const& sysPropertyInfo);
    public:
        static std::unique_ptr<ECSqlBinder> CreateBinder(ECSqlPrepareContext&, ECSqlTypeInfo const&, ECSqlBinder::SqlParamNameGenerator&);
        static std::unique_ptr<ECSqlBinder> CreateBinder(ECSqlPrepareContext&, ParameterExp const& parameterExp);
        static std::unique_ptr<ECSqlBinder> CreateBinder(ECSqlPrepareContext& ctx, PropertyMap const& propMap, ECSqlBinder::SqlParamNameGenerator& gen) { return CreateBinder(ctx, ECSqlTypeInfo(propMap), gen); }

        static std::unique_ptr<IdECSqlBinder> CreateIdBinder(ECSqlPrepareContext&, PropertyMap const&, ECSqlSystemPropertyInfo const&, ECSqlBinder::SqlParamNameGenerator&);
        static std::unique_ptr<IdSetBinder> CreateIdSetBinder(ECSqlPrepareContext&, ECSqlTypeInfo const&, ECSqlBinder::SqlParamNameGenerator&);

    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlParameterMap final
    {
    private:
        std::vector<std::unique_ptr<ECSqlBinder>> m_ownedBinders;
        std::vector<ECSqlBinder*> m_binders;
        bmap<Utf8String, int, CompareIUtf8Ascii> m_nameToIndexMapping;

        std::vector<ECSqlBinder*> m_bindersToCallOnClearBindings;
        std::vector<ECSqlBinder*> m_bindersToCallOnStep;

        //not copyable
        ECSqlParameterMap(ECSqlParameterMap const&) = delete;
        ECSqlParameterMap& operator=(ECSqlParameterMap const&) = delete;

        bool Contains(int& ecsqlParameterIndex, Utf8StringCR ecsqlParameterName) const;

    public:
        ECSqlParameterMap() {}
        ~ECSqlParameterMap() {}

        //! @remarks only named parameters have an identity. Therefore each unnamed parameters has its own binder
        bool TryGetBinder(ECSqlBinder*&, Utf8StringCR ecsqlParameterName) const;
        //!@param[in] ecsqlParameterIndex ECSQL parameter index (1-based)
        ECSqlStatus TryGetBinder(ECSqlBinder*&, int ecsqlParameterIndex) const;

        //!@return ECSQL Parameter index (1-based) or -1 if index could not be found for @p ecsqlParameterName
        int GetIndexForName(Utf8StringCR ecsqlParameterName) const;

        ECSqlBinder* AddBinder(ECSqlPrepareContext&, ParameterExp const&);
        ECSqlStatus OnBeforeStep();

        //Bindings in SQLite have already been cleared at this point. The method
        //allows subclasses to clean-up additional resources tied to binding parameters
        void OnClearBindings();
    };



//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ArrayConstraintValidator
    {
    private:
        ArrayConstraintValidator();
        ~ArrayConstraintValidator();

    public:
        static ECSqlStatus Validate(ECDbCR, ECSqlTypeInfo const& expected, uint32_t actualArrayLength);
        static ECSqlStatus ValidateMaximum(ECDbCR, ECSqlTypeInfo const& expected, uint32_t actualArrayLength);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE