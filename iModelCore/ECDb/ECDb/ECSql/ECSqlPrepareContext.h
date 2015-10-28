/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPrepareContext.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <vector>
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
            static bool IsSystemProperty (Utf8CP accessString)
                {
                static std::set<Utf8CP, CompareUtf8> s_systemProperties
                    {
                    ECDbSystemSchemaHelper::ECARRAYINDEX_PROPNAME,
                    ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME,
                    ECDbSystemSchemaHelper::ECPROPERTYPATHID_PROPNAME,
                    ECDbSystemSchemaHelper::OWNERECINSTANCEID_PROPNAME,
                    ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME,
                    ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME,
                    ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME,
                    ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME
                    };
                
                return s_systemProperties.find (accessString) != s_systemProperties.end ();
                }
            static const std::vector<Utf8String> Split (Utf8CP accessString, Utf8Char seperator)
                {
                Utf8String s = accessString;
                std::vector<Utf8String> output;
                std::string::size_type prev_pos = 0, pos = 0;
                while ((pos = s.find (seperator, pos)) != Utf8String::npos)
                    {
                    output.push_back (s.substr (prev_pos, pos - prev_pos));
                    prev_pos = ++pos;
                    }

                output.push_back (s.substr (prev_pos, pos - prev_pos)); // Last word
                return output;
                }
        private:
            std::set<Utf8String> m_selection;
        public:
             SelectionOptions ()
                {
                }

            ~SelectionOptions ()
                {}


            void AddProperty (Utf8CP accessString)
                {
                Utf8String path;
                for (auto const& subPath : Split (accessString, L'.'))
                    {
                    if (path.empty ())
                        path.assign (subPath);
                    else
                        path.append (".").append (subPath);

                    m_selection.insert (path);
                    }
                }
            bool IsSelected (Utf8CP accessString) const
                {

                if (m_selection.find (accessString) != m_selection.end ())
                        return true;

                return SelectionOptions::IsSystemProperty (accessString);
                }
            bool IsConstantExpression () const
                {
                return m_selection.empty ();
                }
        };

    //=======================================================================================
    // @bsiclass                                                 Affan.Khan    10/2015
    //+===============+===============+===============+===============+===============+======
    struct JoinTableInfo
        {
        public:
            typedef std::unique_ptr<JoinTableInfo> Ptr;
            struct Parameter
                {
                friend struct JoinTableInfo;
                typedef std::unique_ptr<Parameter> Ptr;
                private:
                    size_t m_index;
                    Utf8String m_name;
                    bool m_shared;
                    Parameter const* m_orignalParameter;
                public:
                    Parameter(size_t index, Utf8CP name)
                        :m_index(index), m_name(name), m_orignalParameter(nullptr), m_shared(false)
                        {
                        }
                    Parameter(size_t index, Utf8CP name, Parameter const& orignalParamter)
                        :m_index(index), m_name(name), m_orignalParameter(&orignalParamter), m_shared(false)
                        {
                        }
                    Parameter()
                        :m_index(-1), m_orignalParameter(nullptr)
                        {
                        }    
                    Parameter(Parameter const& rhs)
                        :m_index(rhs.m_index), m_orignalParameter(rhs.m_orignalParameter), m_name(rhs.m_name)
                        {
                        }
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
                        {
                        }
                    ~ParameterSet(){}
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
                                if (BeStringUtilities::Stricmp(param->GetName(), name) == 0)
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
                    ParameterSet& GetOrignalR() {return m_orignal;}
                    ParameterSet& GetPrimaryR() {return m_primary;}
                    ParameterSet& GetSecondaryR() {return m_secondary;}

                    ParameterSet const& GetOrignal() const {return m_orignal;}
                    ParameterSet const& GetPrimary() const {return m_primary;}
                    ParameterSet const& GetSecondary() const {return m_secondary;}
                };
        private:
            Utf8String m_parentStatement;
            Utf8String m_statement;
            Utf8String m_orginalStatement;
            ParameterMap m_parameterMap;
            bool m_userProvidedECInstanceId;
            size_t m_primaryECInstanceIdParameterIndex;
            ECClassCP m_class;
            ECClassCP m_parentClass;

        private:
            static Ptr TrySetupJoinTableContextForInsert(ECSqlPrepareContext& ctx, InsertStatementExp const& exp);
            static Ptr TrySetupJoinTableContextForUpdate(ECSqlPrepareContext& ctx, UpdateStatementExp const& exp);
            static NativeSqlBuilder BuildAssignmentExpression(NativeSqlBuilder::List const& prop, NativeSqlBuilder::List const& values);
            JoinTableInfo(){}
        public:
            ~JoinTableInfo() {}
            static Ptr TrySetupJoinTableContextIfAny(ECSqlPrepareContext& ctx, ECSqlParseTreeCR const& exp, Utf8CP orignalECSQL);
            Utf8CP GetECSQlStatement() const {return m_statement.c_str();}
            Utf8CP GetParentECSQlStatement() const {return m_parentStatement.c_str();}
            Utf8CP GetOrignalECSQlStatement() const {return m_orginalStatement.c_str();}
            bool HasParentECSQlStatement() const {return !m_parentStatement.empty();}
            bool HasECSQlStatement() const {return !m_statement.empty();}
            ECClassCR GetClass() const {return *m_class;}
            ECClassCR GetParentClass() const {return *m_parentClass;}

            ParameterMap const& GetParameterMap() const {return m_parameterMap;}
            bool IsUserProvidedECInstanceId ()const {return m_userProvidedECInstanceId;}
            size_t GetPrimaryECinstanceIdParameterIndex() const {return m_primaryECInstanceIdParameterIndex;}

        };
    //=======================================================================================
    // @bsiclass                                                 Affan.Khan    06/2013
    //+===============+===============+===============+===============+===============+======
    struct ExpScope
        {
    private:
        ExpCR m_exp;
        ECSqlType m_ecsqlType;
        ExpScope const* m_parent;
        OptionsExp const* m_options;
        int m_nativeSqlSelectClauseColumnCount;
        ECSqlType DetermineECSqlType (ExpCR exp) const;
    public:
        ExpScope (ExpCR exp, ExpScope const* parent, OptionsExp const* options);

        ExpScope const* GetParent() const { return m_parent; }
        ExpCR GetExp() const { return m_exp; }
        OptionsExp const* GetOptions() const { return m_options; }

        bool IsRootScope() const { return m_parent == nullptr; }
        //SELECT only
        void IncrementNativeSqlSelectClauseColumnCount (size_t value);
        int GetNativeSqlSelectClauseColumnCount () const;
        ECSqlType GetECSqlType () const { return m_ecsqlType; }
        };

    //=======================================================================================
    // @bsiclass                                                 Krischan.Eberle    11/2013
    //+===============+===============+===============+===============+===============+======
    struct ExpScopeStack
        {
    private:
        std::vector<ExpScope> m_scopes;

    public:
        ExpScopeStack () {}

        void Push (ExpCR statementExp, OptionsExp const* options);
        void Pop ();

        size_t Depth () const;
        ExpScope const& Current () const;
        ExpScope& CurrentR ();


        };
private:
    ECDbCR m_ecdb;
   
    ECSqlStatementBase& m_ecsqlStatement;
    ECSqlPrepareContext const* m_parentCtx;
    ArrayECPropertyCP m_parentArrayProperty;
    ECSqlColumnInfo const* m_parentColumnInfo;
    NativeSqlBuilder m_nativeSqlBuilder;
    bool m_nativeStatementIsNoop;
    bool m_nativeNothingToUpdate;
    ExpScopeStack m_scopes;
    SelectionOptions m_selectionOptions;    
    JoinTableInfo::Ptr m_joinTableInfo;
    ECClassId m_joinTableClassId;
    //SELECT only
    static bool FindLastParameterIndexBeforeWhereClause (int& index, Exp const& statementExp, WhereExp const* whereExp);
public:
    ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& ecsqlStatement);
    ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& ecsqlStatement, ECN::ECClassId joinTableClassI);
    ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& ecsqlStatement, ECSqlPrepareContext const& parent, ArrayECPropertyCR parentArrayProperty, ECSqlColumnInfo const* parentColumnInfo);
    ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& preparedStatment, ECSqlPrepareContext const& parentCtx);
    //ECSqlPrepareContext is copyable. Using compiler-generated copy ctor and assignment op.

    //! Gets the view mode to be used for class maps for classes in the ECSQL to prepare
    //! ClassMap views differ for ECClasses that are domain classes and structs at the same time.
    //! @return View mode for class maps for this prepare context
    IClassMap::View GetClassMapViewMode () const;

    ECDbCR GetECDb() const { return m_ecdb; }
    ECSqlPrepareContext const* GetParentContext() const { return m_parentCtx;}
    ArrayECPropertyCP GetParentArrayProperty () const { return m_parentArrayProperty;}
    ECSqlColumnInfo const* GetParentColumnInfo () const { return m_parentColumnInfo;}
    SelectionOptions const& GetSelectionOptions () const { return m_selectionOptions; }
    SelectionOptions& GetSelectionOptionsR () { return m_selectionOptions; }
    ECClassId GetJoinTableClassId() const { return m_joinTableClassId; }
    bool IsParentOfJoinTable() const {return m_joinTableClassId != 0;}
    JoinTableInfo const* GetJoinTableInfo() const { return m_joinTableInfo.get(); }
    JoinTableInfo const* TrySetupJoinTableContextIfAny(ECSqlParseTreeCR exp, Utf8CP orignalECSQL)
        {
        m_joinTableInfo = JoinTableInfo::TrySetupJoinTableContextIfAny(*this, exp, orignalECSQL);
        return GetJoinTableInfo();
        }
    ECSqlStatementBase& GetECSqlStatementR () const;
    NativeSqlBuilder const& GetSqlBuilder () const {return m_nativeSqlBuilder;}
    NativeSqlBuilder& GetSqlBuilderR () {return m_nativeSqlBuilder;}
    Utf8CP GetNativeSql () const;

    bool NativeStatementIsNoop () const { return m_nativeStatementIsNoop;  }
    bool NativeNothingToUpdate() const { return m_nativeNothingToUpdate;  }

    void SetNativeStatementIsNoop (bool flag) { m_nativeStatementIsNoop = flag; }
    void SetNativeNothingToUpdate(bool flag){ m_nativeNothingToUpdate = flag; }
    

    ExpScope const& GetCurrentScope () const {return m_scopes.Current();}
    ExpScope& GetCurrentScopeR ()  {return m_scopes.CurrentR();}


    void PushScope (ExpCR exp, OptionsExp const* options = nullptr) { m_scopes.Push (exp, options); }
    void PopScope () {m_scopes.Pop();}

    bool IsEmbeddedStatement () const { return m_parentCtx != nullptr; }
    bool IsPrimaryStatement () const { return !IsEmbeddedStatement (); }
    static Utf8String CreateECInstanceIdSelectionQuery (ECSqlPrepareContext& ctx, ClassNameExp const& classNameExpr, WhereExp const* whereExp);
    static int FindLastParameterIndexBeforeWhereClause (Exp const& statementExp, WhereExp const* whereExp);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
