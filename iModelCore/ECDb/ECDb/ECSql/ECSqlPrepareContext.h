/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <vector>
#include <bitset>      
#include "Exp.h"
#include "OptionsExp.h"
#include "NativeSqlBuilder.h"
 
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct IECSqlPreparedStatement;
struct SingleECSqlPreparedStatement;
struct SingleContextTableECSqlPreparedStatement;


//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECSqlPrepareContext final
    {
    public:
        //=======================================================================================
        // @bsiclass
        //+===============+===============+===============+===============+===============+======
        struct ExpScope final
            {
            enum class ExtendedOptions
                {
                None = 0,
                UsePrimaryTableForSystemPropertyResolution = 1
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
        // @bsiclass
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
        // @bsiclass
        //+===============+===============+===============+===============+===============+======
        struct SelectClauseInfo final
            {
            private:
                bset<Utf8String, CompareIUtf8Ascii> m_selectClause;

                static bvector<Utf8String> Split(Utf8StringCR accessString, Utf8Char separator);

            public:
                SelectClauseInfo() {}

                void AddProperty(PropertyMap const&);
                bool IsSelected(Utf8StringCR accessString, bool alwaysSelectSystemProperties = true) const;
                bool IsSystemProperty(Utf8StringCR accessString) const;
                bool IsConstantExpression() const { return m_selectClause.empty(); }

                void Clear() { m_selectClause.clear(); }
            };

    private:
        ECDbCR m_ecdb;
        Db const& m_dataSourceECDb;
        IssueDataSource const& m_issues;
        SingleECSqlPreparedStatement* m_singlePreparedStatement = nullptr;
        NativeSqlBuilder m_nativeSqlBuilder;
        bool m_nativeStatementIsNoop = false;
        ExpScopeStack m_scopes;
        SelectClauseInfo m_selectionOptions;
        int m_nextSystemSqlParameterNameSuffix = 0;
        //not copyable
        ECSqlPrepareContext(ECSqlPrepareContext const&) = delete;
        ECSqlPrepareContext& operator=(ECSqlPrepareContext const&) = delete;
        uint32_t m_colAliasPrefix = 0;
        mutable ViewColumnInfosByViewId m_viewColInfos;

    public:
        ECSqlPrepareContext(IECSqlPreparedStatement&, Db const& dataSourceECDb, IssueDataSource const&);
        void Reset(SingleECSqlPreparedStatement&);

        ECDbCR GetECDb() const { return m_ecdb; }
        //! Used to prepare the generated SQLite statement against. Usually this is the same as GetECDb, but
        //! it can be a different connection (e.g. in a different thread) to the very same ECDb file.
        Db const& GetDataSourceConnection() const { return m_dataSourceECDb; }
        IssueDataSource const& Issues() const { return m_issues; }
        SelectClauseInfo const& GetSelectionOptions() const { return m_selectionOptions; }
        SelectClauseInfo& GetSelectionOptionsR() { return m_selectionOptions; }
        SingleECSqlPreparedStatement& GetPreparedStatement() const { BeAssert(m_singlePreparedStatement != nullptr); return *m_singlePreparedStatement; }
        template <class TECSqlPreparedStatement>
        TECSqlPreparedStatement& GetPreparedStatement() const
            {
            BeAssert(dynamic_cast<TECSqlPreparedStatement*> (&GetPreparedStatement()) != nullptr);
            return static_cast<TECSqlPreparedStatement&> (GetPreparedStatement());
            }

        NativeSqlBuilder& GetSqlBuilder() { return m_nativeSqlBuilder; }

        bool NativeStatementIsNoop() const { return m_nativeStatementIsNoop; }
        void SetNativeStatementIsNoop(bool flag) { m_nativeStatementIsNoop = flag; }

        ExpScope const& GetCurrentScope() const { return m_scopes.Current(); }
        ExpScope& GetCurrentScopeR() { return m_scopes.CurrentR(); }
        void PushScope(ExpCR exp, OptionsExp const* options = nullptr) { m_scopes.Push(exp, options); }
        void PopScope() { m_scopes.Pop(); }
        uint32_t GetColAliasPrefix() { return m_colAliasPrefix; };
        void SetColAliasPrefix(uint32_t value) { m_colAliasPrefix = value; }

        int IncrementSystemSqlParameterSuffix() { m_nextSystemSqlParameterNameSuffix++; return m_nextSystemSqlParameterNameSuffix; }
        ViewColumnInfosByViewId& GetViewColumnInfos() const { return m_viewColInfos; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
