/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPrepareContext.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <vector>
#include <bitset>      
#include "Exp.h"
#include "OptionsExp.h"
#include "NativeSqlBuilder.h"
 
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#ifdef ECSQLPREPAREDSTATEMENT_REFACTOR

struct SingleECSqlPreparedStatement;

//=======================================================================================
// @bsiclass                                                 Affan.Khan    06/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlPrepareContext final : NonCopyableClass
    {
    public:
        //=======================================================================================
        // @bsiclass                                                 Affan.Khan    06/2013
        //+===============+===============+===============+===============+===============+======
        struct ExpScope final
            {
            enum class ExtendedOptions
                {
                None = 0,
                SkipTableAliasWhenPreparingDeleteWhereClause = 1
                };

            private:
                ExpCR m_exp;
                ECSqlType m_ecsqlType;
                ExpScope const* m_parent;
                OptionsExp const* m_options;
                int m_nativeSqlSelectClauseColumnCount;
                ExtendedOptions m_extendedOptions;

                ECSqlType DetermineECSqlType(ExpCR) const;

            public:
                ExpScope(ExpCR, ExpScope const* parent, OptionsExp const*);

                ECSqlType GetECSqlType() const { return m_ecsqlType; }
                ExpScope const* GetParent() const { return m_parent; }
                ExpCR GetExp() const { return m_exp; }
                OptionsExp const* GetOptions() const { return m_options; }
                bool IsRootScope() const { return m_parent == nullptr; }

                void SetExtendedOption(ExtendedOptions option) { m_extendedOptions = Enum::Or(m_extendedOptions, option); }
                bool HasExtendedOption(ExtendedOptions option) const { return Enum::Contains(m_extendedOptions, option); }

                //SELECT only
                void IncrementNativeSqlSelectClauseColumnCount(size_t value) { BeAssert(m_ecsqlType == ECSqlType::Select); m_nativeSqlSelectClauseColumnCount += (int) (value); }
                int GetNativeSqlSelectClauseColumnCount() const { BeAssert(m_ecsqlType == ECSqlType::Select); return m_nativeSqlSelectClauseColumnCount; }
            };

        //=======================================================================================
        // @bsiclass                                                 Krischan.Eberle    11/2013
        //+===============+===============+===============+===============+===============+======
        struct ExpScopeStack final
            {
            private:
                std::vector<ExpScope> m_scopes;

            public:
                ExpScopeStack() {}

                void Push(ExpCR statementExp, OptionsExp const*);
                void Pop() { m_scopes.pop_back(); }

                size_t Depth() const { return m_scopes.size(); }
                ExpScope const& Current() const { BeAssert(!m_scopes.empty()); return m_scopes.back(); }
                ExpScope& CurrentR() { BeAssert(!m_scopes.empty()); return m_scopes.back(); }

                void Clear() { m_scopes.clear(); }
            };

        //=======================================================================================
        // @bsiclass                                                 Affan.Khan    02/2015
        //+===============+===============+===============+===============+===============+======
        struct SelectClauseInfo final
            {
            private:
                bset<Utf8String, CompareIUtf8Ascii> m_selectClause;

                static bvector<Utf8String> Split(Utf8StringCR accessString, Utf8Char separator);

            public:
                SelectClauseInfo() {}

                void AddProperty(PropertyMap const&);
                bool IsSelected(Utf8StringCR accessString) const;
                bool IsConstantExpression() const { return m_selectClause.empty(); }

                void Clear() { m_selectClause.clear(); }
            };

    private:
        ECDbCR m_ecdb;
        SingleECSqlPreparedStatement* m_singlePreparedStatement = nullptr;
        NativeSqlBuilder m_nativeSqlBuilder;
        bool m_nativeStatementIsNoop = false;
        ExpScopeStack m_scopes;
        SelectClauseInfo m_selectionOptions;
        int m_nextSystemSqlParameterNameSuffix = 0;

    public:
        explicit ECSqlPrepareContext(ECDbCR ecdb) : m_ecdb(ecdb) {}
        void Reset(SingleECSqlPreparedStatement& preparedStmt)
            { 
            m_singlePreparedStatement = &preparedStmt;

            m_nativeSqlBuilder.Clear();
            m_scopes.Clear();
            m_selectionOptions.Clear();
            }

        ECDbCR GetECDb() const { return m_ecdb; }

        SelectClauseInfo const& GetSelectionOptions() const { return m_selectionOptions; }
        SelectClauseInfo& GetSelectionOptionsR() { return m_selectionOptions; }

        SingleECSqlPreparedStatement& GetPreparedStatement() const { BeAssert(m_singlePreparedStatement != nullptr); return *m_singlePreparedStatement; }
        template <class TECSqlPreparedStatement>
        TECSqlPreparedStatement& GetPreparedStatement() const
            {
            BeAssert(dynamic_cast<TECSqlPreparedStatement*> (&GetPreparedStatement()) != nullptr);
            return static_cast<TECSqlPreparedStatement&> (GetPreparedStatement());
            }

        NativeSqlBuilder const& GetSqlBuilder() const { return m_nativeSqlBuilder; }
        NativeSqlBuilder& GetSqlBuilderR() { return m_nativeSqlBuilder; }
        Utf8CP GetNativeSql() const { return m_nativeSqlBuilder.ToString(); }

        bool NativeStatementIsNoop() const { return m_nativeStatementIsNoop; }
        void SetNativeStatementIsNoop(bool flag) { m_nativeStatementIsNoop = flag; }

        ExpScope const& GetCurrentScope() const { return m_scopes.Current(); }
        ExpScope& GetCurrentScopeR() { return m_scopes.CurrentR(); }
        void PushScope(ExpCR exp, OptionsExp const* options = nullptr) { m_scopes.Push(exp, options); }
        void PopScope() { m_scopes.Pop(); }

        int IncrementSystemSqlParameterSuffix() { m_nextSystemSqlParameterNameSuffix++; return m_nextSystemSqlParameterNameSuffix; }
    };

#else

struct ECSqlStatementBase;

//=======================================================================================
// @bsiclass                                                 Affan.Khan    06/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlPrepareContext
    {
    public:
        //=======================================================================================
        // @bsiclass                                                 Affan.Khan    06/2013
        //+===============+===============+===============+===============+===============+======
        struct ExpScope
            {
            enum class ExtendedOptions
                {
                None = 0,
                SkipTableAliasWhenPreparingDeleteWhereClause = 1
                };

            private:
                ExpCR m_exp;
                ECSqlType m_ecsqlType;
                ExpScope const* m_parent;
                OptionsExp const* m_options;
                int m_nativeSqlSelectClauseColumnCount;
                ExtendedOptions m_extendedOptions;

                ECSqlType DetermineECSqlType(ExpCR) const;

            public:
                ExpScope(ExpCR, ExpScope const* parent, OptionsExp const*);

                ECSqlType GetECSqlType() const { return m_ecsqlType; }
                ExpScope const* GetParent() const { return m_parent; }
                ExpCR GetExp() const { return m_exp; }
                OptionsExp const* GetOptions() const { return m_options; }
                bool IsRootScope() const { return m_parent == nullptr; }

                void SetExtendedOption(ExtendedOptions option) { m_extendedOptions = Enum::Or(m_extendedOptions, option); }
                bool HasExtendedOption(ExtendedOptions option) const { return Enum::Contains(m_extendedOptions, option); }

                //SELECT only
                void IncrementNativeSqlSelectClauseColumnCount(size_t value) { BeAssert(m_ecsqlType == ECSqlType::Select); m_nativeSqlSelectClauseColumnCount += (int) (value); }
                int GetNativeSqlSelectClauseColumnCount() const { BeAssert(m_ecsqlType == ECSqlType::Select); return m_nativeSqlSelectClauseColumnCount; }
            };

        //=======================================================================================
        // @bsiclass                                                 Krischan.Eberle    11/2013
        //+===============+===============+===============+===============+===============+======
        struct ExpScopeStack
            {
            private:
                std::vector<ExpScope> m_scopes;

            public:
                ExpScopeStack() {}

                void Push(ExpCR statementExp, OptionsExp const*);
                void Pop() { m_scopes.pop_back(); }

                size_t Depth() const { return m_scopes.size(); }
                ExpScope const& Current() const { BeAssert(!m_scopes.empty()); return m_scopes.back(); }
                ExpScope& CurrentR() { BeAssert(!m_scopes.empty()); return m_scopes.back(); }
            };

        //=======================================================================================
        // @bsiclass                                                 Affan.Khan    02/2015
        //+===============+===============+===============+===============+===============+======
        struct SelectClauseInfo
            {
            private:
                bset<Utf8String, CompareIUtf8Ascii> m_selectClause;

                static bvector<Utf8String> Split(Utf8StringCR accessString, Utf8Char separator);

            public:
                SelectClauseInfo() {}

                void AddProperty(PropertyMap const&);
                bool IsSelected(Utf8StringCR accessString) const;
                bool IsConstantExpression() const { return m_selectClause.empty(); }
            };

        //=======================================================================================
        // @bsiclass                                                 Affan.Khan    10/2015
        //+===============+===============+===============+===============+===============+======
        struct JoinedTableInfo
            {
            public:
                struct Parameter : NonCopyableClass
                    {
                    friend struct JoinedTableInfo;

                    private:
                        size_t m_index;
                        Utf8String m_name;
                        Parameter const* m_originalParameter;
                        bool m_isShared;

                    public:
                        Parameter(size_t index, Utf8StringCR name, Parameter const* originalParameter) :m_index(index), m_name(name), m_originalParameter(originalParameter), m_isShared(false) {}
                        size_t GetIndex() const { return m_index; }
                        bool IsNamed() const { return !m_name.empty(); }
                        Utf8StringCR GetName() const { return m_name; }
                        Parameter const* GetOrignalParameter() const { return m_originalParameter; }
                        bool IsShared() const { return m_isShared; }
                    };

                struct ParameterSet
                    {
                    private:
                        std::vector<std::unique_ptr<Parameter>> m_parameters;
                    public:
                        ParameterSet() {}

                        Parameter const* Find(size_t index) const;
                        Parameter const* Find(Utf8StringCR name) const;
                        Parameter const* Add(ParameterExp const&);
                        Parameter const* Add(Parameter const& orignalParam);
                        Parameter const* Add();
                        void Add(bvector<Parameter const*> const&);

                        size_t First() const;
                        size_t Last() const { return m_parameters.size(); }
                        bool Empty() const  { return First() == Last(); }
                    };
                struct ParameterMap
                    {
                    private:
                        ParameterSet m_orignal;
                        ParameterSet m_primary;
                        ParameterSet m_secondary;

                    public:
                        ParameterSet& GetOrignalR() { return m_orignal; }
                        ParameterSet& GetPrimaryR() { return m_primary; }
                        ParameterSet& GetSecondaryR() { return m_secondary; }
                        ParameterSet const& GetOrignal() const { return m_orignal; }
                        ParameterSet const& GetPrimary() const { return m_primary; }
                        ParameterSet const& GetSecondary() const { return m_secondary; }
                    };

            private:
                Utf8String m_originalECSql;
                Utf8String m_parentOfJoinedTableECSql;
                Utf8String m_joinedTableECSql;
                ParameterMap m_parameterMap;
                bool m_ecinstanceIdIsUserProvided;
                size_t m_primaryECInstanceIdParameterIndex;
                ECN::ECClassCR m_class;

                explicit JoinedTableInfo(ECN::ECClassCR ecClass) : m_ecinstanceIdIsUserProvided(false), m_primaryECInstanceIdParameterIndex(0), m_class(ecClass)
                    {}

                static std::unique_ptr<JoinedTableInfo> CreateForInsert(ECSqlPrepareContext& ctx, InsertStatementExp const& exp);
                static std::unique_ptr<JoinedTableInfo> CreateForUpdate(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp);
                static NativeSqlBuilder BuildAssignmentExpression(NativeSqlBuilder::List const& prop, NativeSqlBuilder::List const& values);

            public:
                ~JoinedTableInfo() {}
                static std::unique_ptr<JoinedTableInfo> Create(ECSqlPrepareContext&, Exp const&, Utf8CP orignalECSQL);
                Utf8CP GetJoinedTableECSql() const { return m_joinedTableECSql.c_str(); }
                Utf8CP GetParentOfJoinedTableECSql() const { return m_parentOfJoinedTableECSql.c_str(); }
                Utf8CP GetOrignalECSql() const { return m_originalECSql.c_str(); }
                bool HasParentOfJoinedTableECSql() const { return !m_parentOfJoinedTableECSql.empty(); }
                bool HasJoinedTableECSql() const { return !m_joinedTableECSql.empty(); }
                ECN::ECClassCR GetClass() const { return m_class; }

                ParameterMap const& GetParameterMap() const { return m_parameterMap; }
                bool IsUserProvidedECInstanceId()const { return m_ecinstanceIdIsUserProvided; }
                size_t GetPrimaryECInstanceIdParameterIndex() const { return m_primaryECInstanceIdParameterIndex; }
            };

    private:
        ECDbCR m_ecdb;
        ECCrudWriteToken const* m_writeToken = nullptr;

        ECSqlStatementBase& m_ecsqlStatement;
        NativeSqlBuilder m_nativeSqlBuilder;
        bool m_nativeStatementIsNoop = false;
        ExpScopeStack m_scopes;
        SelectClauseInfo m_selectionOptions;
        std::unique_ptr<JoinedTableInfo> m_joinedTableInfo;
        ECN::ECClassId m_joinedTableClassId;
        int m_nextSystemSqlParameterNameSuffix = 0;

    public:
        ECSqlPrepareContext(ECDbCR, ECSqlStatementBase&, ECCrudWriteToken const*);
        ECSqlPrepareContext(ECDbCR, ECSqlStatementBase&, ECN::ECClassId joinedTableClassId, ECCrudWriteToken const*);
        //ECSqlPrepareContext is copyable. Using compiler-generated copy ctor and assignment op.

        ECDbCR GetECDb() const { return m_ecdb; }
        ECCrudWriteToken const* GetWriteToken() const { return m_writeToken; }

        SelectClauseInfo const& GetSelectionOptions() const { return m_selectionOptions; }
        SelectClauseInfo& GetSelectionOptionsR() { return m_selectionOptions; }
        
        ECN::ECClassId GetJoinedTableClassId() const { return m_joinedTableClassId; }
        bool IsParentOfJoinedTable() const { return m_joinedTableClassId.IsValid(); }
        void MarkAsParentOfJoinedTable(ECN::ECClassId classId) { BeAssert(!IsParentOfJoinedTable()); m_joinedTableClassId = classId; }
        JoinedTableInfo const* GetJoinedTableInfo() const { return m_joinedTableInfo.get(); }
        JoinedTableInfo const* TrySetupJoinedTableInfo(Exp const&, Utf8CP originalECSQL);
        ECSqlStatementBase& GetECSqlStatementR() const;

        NativeSqlBuilder const& GetSqlBuilder() const { return m_nativeSqlBuilder; }
        NativeSqlBuilder& GetSqlBuilderR() { return m_nativeSqlBuilder; }
        Utf8CP GetNativeSql() const;

        bool NativeStatementIsNoop() const { return m_nativeStatementIsNoop; }
        void SetNativeStatementIsNoop(bool flag) { m_nativeStatementIsNoop = flag; }

        ExpScope const& GetCurrentScope() const { return m_scopes.Current(); }
        ExpScope& GetCurrentScopeR() { return m_scopes.CurrentR(); }
        void PushScope(ExpCR exp, OptionsExp const* options = nullptr) { m_scopes.Push(exp, options); }
        void PopScope() { m_scopes.Pop(); }

        int IncrementSystemSqlParameterSuffix() { m_nextSystemSqlParameterNameSuffix++; return m_nextSystemSqlParameterNameSuffix; }
    };
#endif


END_BENTLEY_SQLITE_EC_NAMESPACE
