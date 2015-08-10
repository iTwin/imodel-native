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
    // @bsiclass                                                 Affan.Khan    06/2013
    //+===============+===============+===============+===============+===============+======
    struct ExpScope
        {
    private:
        ExpCR m_exp;
        ECSqlType m_ecsqlType;
        ExpScope const* m_parent;
        int m_nativeSqlSelectClauseColumnCount;
        ECSqlType DetermineECSqlType (ExpCR exp) const;

    public:
        ExpScope (ExpCR exp, ExpScope const* parent);

        ExpScope const* GetParent() const;
        ExpCR GetExp () const;
        
        bool IsRootScope () const;
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

        void Push (ExpCR statementExp);
        void Pop ();

        size_t Depth () const;
        ExpScope const& Current () const;
        ExpScope& CurrentR ();


        };
    enum class SqlRenderStrategy
        {
        V0,
        V1
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
    //SELECT only
    SqlRenderStrategy m_sqlRenderStrategy;
    static bool FindLastParameterIndexBeforeWhereClause (int& index, Exp const& statementExp, WhereExp const* whereExp);
public:
    ECSqlPrepareContext(ECDbCR ecdb, ECSqlStatementBase& ecsqlStatement);
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

    ECSqlStatementBase& GetECSqlStatementR () const;

    NativeSqlBuilder const& GetSqlBuilder () const {return m_nativeSqlBuilder;}
    NativeSqlBuilder& GetSqlBuilderR () {return m_nativeSqlBuilder;}
    Utf8CP GetNativeSql () const;

    bool NativeStatementIsNoop () const { return m_nativeStatementIsNoop;  }
    bool NativeNothingToUpdate() const { return m_nativeNothingToUpdate;  }

    void SetNativeStatementIsNoop (bool flag) { m_nativeStatementIsNoop = flag; }
    void SetNativeNothingToUpdate(bool flag){ m_nativeNothingToUpdate = flag; }
    
    SqlRenderStrategy GetSqlRenderStrategy () const { return m_sqlRenderStrategy; }
    void SetSqlRenderStrategy (SqlRenderStrategy strategy){ m_sqlRenderStrategy = strategy; }

    ExpScope const& GetCurrentScope () const {return m_scopes.Current();}
    ExpScope& GetCurrentScopeR ()  {return m_scopes.CurrentR();}


    void PushScope (ExpCR exp) { m_scopes.Push (exp); }
    void PopScope () {m_scopes.Pop();}

    bool IsSuccess () const;
    ECSqlStatus GetStatus () const;
    ECSqlStatus SetError (ECSqlStatus status, Utf8CP fmt, ...);
    bool IsEmbeddedStatement () const { return m_parentCtx != nullptr; }
    bool IsPrimaryStatement () const { return !IsEmbeddedStatement (); }
    static Utf8String CreateECInstanceIdSelectionQuery (ECSqlPrepareContext& ctx, ClassNameExp const& classNameExpr, WhereExp const* whereExp);
    static int FindLastParameterIndexBeforeWhereClause (Exp const& statementExp, WhereExp const* whereExp);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
