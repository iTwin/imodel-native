/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPrepareContext.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <vector>
#include <bitset>      
#include "Exp.h"
#include "OptionsExp.h"
#include "ECSqlPreparedStatement.h"
#include "NativeSqlBuilder.h"
 
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ECSqlStatementBase;

//=======================================================================================
// @bsiclass                                                 Affan.Khan    06/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlPrepareContext
    {
    public:
        //=======================================================================================
        // @bsiclass                                                 Affan.Khan    02/2015
        //+===============+===============+===============+===============+===============+======
        struct SelectionOptions
            {
            private:
                static bool IsSystemProperty(Utf8CP accessString)
                    {
                    return BeStringUtilities::StricmpAscii(accessString, ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME) == 0 ||
                        BeStringUtilities::StricmpAscii(accessString, ECDbSystemSchemaHelper::ECCLASSID_PROPNAME) == 0 ||
                        BeStringUtilities::StricmpAscii(accessString, ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME) == 0 ||
                        BeStringUtilities::StricmpAscii(accessString, ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME) == 0 ||
                        BeStringUtilities::StricmpAscii(accessString, ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME) == 0 ||
                        BeStringUtilities::StricmpAscii(accessString, ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME) == 0;
                    }

                static std::vector<Utf8String> Split(Utf8CP accessString, Utf8Char seperator)
                    {
                    Utf8String s = accessString;
                    std::vector<Utf8String> output;
                    std::string::size_type prev_pos = 0, pos = 0;
                    while ((pos = s.find(seperator, pos)) != Utf8String::npos)
                        {
                        output.push_back(s.substr(prev_pos, pos - prev_pos));
                        prev_pos = ++pos;
                        }

                    output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word
                    return output;
                    }
            private:
                std::set<Utf8String, CompareIUtf8Ascii> m_selection;

            public:
                SelectionOptions() {}
                ~SelectionOptions() {}

                void AddProperty(Utf8CP accessString)
                    {
                    Utf8String path;
                    for (auto const& subPath : Split(accessString, L'.'))
                        {
                        if (path.empty())
                            path.assign(subPath);
                        else
                            path.append(".").append(subPath);

                        m_selection.insert(path);
                        }
                    }

                bool IsSelected(Utf8CP accessString) const
                    {
                    if (m_selection.find(accessString) != m_selection.end())
                        return true;

                    return SelectionOptions::IsSystemProperty(accessString);
                    }

                bool IsConstantExpression() const { return m_selection.empty(); }
            };

        //=======================================================================================
        // @bsiclass                                                 Affan.Khan    10/2015
        //+===============+===============+===============+===============+===============+======
        struct JoinedTableInfo
            {
            public:
                struct Parameter
                    {
                    friend struct JoinedTableInfo;
                    typedef std::unique_ptr<Parameter> Ptr;
                    private:
                        size_t m_index;
                        Utf8String m_name;
                        bool m_shared;
                        Parameter const* m_orignalParameter;
                    public:
                        Parameter(size_t index, Utf8CP name)
                            :m_index(index), m_name(name), m_orignalParameter(nullptr), m_shared(false)
                            {}
                        Parameter(size_t index, Utf8CP name, Parameter const& orignalParamter)
                            :m_index(index), m_name(name), m_orignalParameter(&orignalParamter), m_shared(false)
                            {}
                        Parameter()
                            :m_index(-1), m_orignalParameter(nullptr)
                            {}
                        Parameter(Parameter const& rhs)
                            :m_index(rhs.m_index), m_orignalParameter(rhs.m_orignalParameter), m_name(rhs.m_name)
                            {}
                        Parameter& operator = (Parameter const& rhs)
                            {
                            if (&rhs != this)
                                {
                                m_index = rhs.m_index;
                                m_orignalParameter = rhs.m_orignalParameter;
                                m_name = rhs.m_name;
                                }

                            return *this;
                            }
                        ~Parameter()
                            {
                            m_orignalParameter = nullptr;
                            m_index = 0;
                            m_name.clear();
                            }
                        bool IsShared() const
                            {
                            return m_shared;
                            }
                        size_t GetIndex() const
                            {
                            return m_index;
                            }
                        Utf8CP GetName() const
                            {
                            return m_name.c_str();
                            }
                        bool IsNamed() const
                            {
                            return !m_name.empty();
                            }
                        Parameter const* GetOrignalParameter() const
                            {
                            return m_orignalParameter;
                            }
                    };

                struct ParameterSet
                    {
                    private:
                        std::vector<Parameter::Ptr> m_parameters;
                    public:
                        ParameterSet()
                            {}
                        ~ParameterSet()
                            {}
                        Parameter const* Find(size_t index) const
                            {
                            if (index > m_parameters.size() || index == 0)
                                return nullptr;

                            return m_parameters.at(index - 1).get();
                            }
                        Parameter const* Find(Utf8CP name) const
                            {
                            for (auto& param : m_parameters)
                                {
                                if (param->IsNamed())
                                    if (BeStringUtilities::StricmpAscii(param->GetName(), name) == 0)
                                        return param.get();
                                }

                            return nullptr;
                            }
                        Parameter const* Add(ParameterExp const& exp)
                            {
                            if (exp.IsNamedParameter())
                                {
                                if (auto r = Find(exp.GetParameterName()))
                                    {
                                    BeAssert(r->GetIndex() == exp.GetParameterIndex());
                                    return r;
                                    }
                                }

                            m_parameters.push_back(Parameter::Ptr(new Parameter(exp.GetParameterIndex(), exp.GetParameterName())));
                            return Find(Last());
                            }
                        Parameter const* Add(Parameter const& orignalParam)
                            {
                            m_parameters.push_back(Parameter::Ptr(new Parameter(m_parameters.size() + 1, orignalParam.GetName(), orignalParam)));
                            return Find(Last());
                            }
                        Parameter const* Add()
                            {
                            m_parameters.push_back(Parameter::Ptr(new Parameter(m_parameters.size() + 1, "")));
                            return Find(Last());
                            }
                        Parameter const* Add(Utf8CP name)
                            {
                            m_parameters.push_back(Parameter::Ptr(new Parameter(m_parameters.size() + 1, name)));
                            return Find(Last());
                            }

                        void  Add(std::vector<Parameter const*> const& params)
                            {
                            for (auto param : params)
                                {
                                BeAssert(param != nullptr);
                                Add(*param);
                                }
                            }
                        size_t First() const
                            {
                            if (m_parameters.size() > 0)
                                return 1;

                            return 0;
                            }
                        size_t Last() const
                            {
                            return m_parameters.size();
                            }
                        bool Empty() const
                            {
                            return First() == Last();
                            }
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

                JoinedTableInfo(ECN::ECClassCR ecClass) : m_ecinstanceIdIsUserProvided(false), m_primaryECInstanceIdParameterIndex(0), m_class(ecClass)
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
                size_t GetPrimaryECinstanceIdParameterIndex() const { return m_primaryECInstanceIdParameterIndex; }
            };
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
                void IncrementNativeSqlSelectClauseColumnCount(size_t value);
                int GetNativeSqlSelectClauseColumnCount() const;
            };

        //=======================================================================================
        // @bsiclass                                                 Krischan.Eberle    11/2013
        //+===============+===============+===============+===============+===============+======
        struct ExpScopeStack
            {
            private:
                std::vector<ExpScope> m_scopes;

            public:
                ExpScopeStack()
                    {}

                void Push(ExpCR statementExp, OptionsExp const*);
                void Pop();

                size_t Depth() const;
                ExpScope const& Current() const;
                ExpScope& CurrentR();


            };
    private:
        ECDbCR m_ecdb;

        ECSqlStatementBase& m_ecsqlStatement;
        ECSqlPrepareContext const* m_parentCtx;
        ECN::ArrayECPropertyCP m_parentArrayProperty;
        ECSqlColumnInfo const* m_parentColumnInfo;
        NativeSqlBuilder m_nativeSqlBuilder;
        bool m_nativeStatementIsNoop;
        ExpScopeStack m_scopes;
        SelectionOptions m_selectionOptions;
        std::unique_ptr<JoinedTableInfo> m_joinedTableInfo;
        ECN::ECClassId m_joinedTableClassId;

    public:
        ECSqlPrepareContext(ECDbCR, ECSqlStatementBase&);
        ECSqlPrepareContext(ECDbCR, ECSqlStatementBase&, ECN::ECClassId joinedTableClassId);
        ECSqlPrepareContext(ECDbCR, ECSqlStatementBase&, ECSqlPrepareContext const& parentCtx, ECN::ArrayECPropertyCR parentArrayProperty, ECSqlColumnInfo const* parentColumnInfo);
        ECSqlPrepareContext(ECDbCR, ECSqlStatementBase&, ECSqlPrepareContext const& parentCtx);
        //ECSqlPrepareContext is copyable. Using compiler-generated copy ctor and assignment op.

        ECDbCR GetECDb() const { return m_ecdb; }
        ECSqlPrepareContext const* GetParentContext() const { return m_parentCtx; }
        ECN::ArrayECPropertyCP GetParentArrayProperty() const { return m_parentArrayProperty; }
        ECSqlColumnInfo const* GetParentColumnInfo() const { return m_parentColumnInfo; }
        SelectionOptions const& GetSelectionOptions() const { return m_selectionOptions; }
        SelectionOptions& GetSelectionOptionsR() { return m_selectionOptions; }
        
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

        bool IsEmbeddedStatement() const { return m_parentCtx != nullptr; }
        bool IsPrimaryStatement() const { return !IsEmbeddedStatement(); }
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
