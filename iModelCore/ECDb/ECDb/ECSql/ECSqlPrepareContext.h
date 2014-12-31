/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPrepareContext.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
    // @bsiclass                                                 Affan.Khan    06/2013
    //+===============+===============+===============+===============+===============+======
    struct ExpScope
        {
    private:
        ExpCR m_exp;
        ECSqlType m_ecsqlType;
        ExpScope const* m_parent;

        ECSqlType DetermineECSqlType (ExpCR exp) const;

    public:
        ExpScope (ExpCR exp, ExpScope const* parent);

        ExpScope const* GetParent() const;
        ExpCR GetExp () const;
        
        bool IsRootScope () const;

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
        };
   
private:
    ECSqlStatementBase& m_ecsqlStatement;
    ECSqlPrepareContext const* m_parentCtx;
    ArrayECPropertyCP m_parentArrayProperty;
    ECSqlColumnInfo const* m_parentColumnInfo;
    NativeSqlBuilder m_nativeSqlBuilder;
    bool m_nativeStatementIsNoop;
    bool m_nativeNothingToUpdate;
    ExpScopeStack m_scopes;

    //SELECT only
    int m_nativeSqlSelectClauseColumnCount;
    static bool FindLastParameterIndexBeforeWhereClause (int& index, Exp const& statementExp, WhereExp const* whereExp);
public:
    explicit ECSqlPrepareContext (ECSqlStatementBase& ecsqlStatement);
    ECSqlPrepareContext (ECSqlStatementBase& ecsqlStatement, ECSqlPrepareContext const& parent, ArrayECPropertyCR parentArrayProperty, ECSqlColumnInfo const* parentColumnInfo);
    ECSqlPrepareContext (ECSqlStatementBase& preparedStatment, ECSqlPrepareContext const& parentCtx);
    //ECSqlPrepareContext is copyable. Using compiler-generated copy ctor and assignment op.

    //! Gets the view mode to be used for class maps for classes in the ECSQL to prepare
    //! ClassMap views differ for ECClasses that are domain classes and structs at the same time.
    //! @return View mode for class maps for this prepare context
    IClassMap::View GetClassMapViewMode () const;

    ECSqlPrepareContext const* GetParentContext() const { return m_parentCtx;}
    ArrayECPropertyCP GetParentArrayProperty () const { return m_parentArrayProperty;}
    ECSqlColumnInfo const* GetParentColumnInfo () const { return m_parentColumnInfo;}

    ECSqlStatementBase& GetECSqlStatementR () const;

    NativeSqlBuilder const& GetSqlBuilder () const {return m_nativeSqlBuilder;}
    NativeSqlBuilder& GetSqlBuilderR () {return m_nativeSqlBuilder;}
    Utf8CP GetNativeSql () const;

    bool NativeStatementIsNoop () const { return m_nativeStatementIsNoop;  }
    bool NativeNothingToUpdate() const { return m_nativeNothingToUpdate;  }

    void SetNativeStatementIsNoop (bool flag) { m_nativeStatementIsNoop = flag; }
    void SetNativeNothingToUpdate(bool flag){ m_nativeNothingToUpdate = flag; }
    
    //SELECT only
    void IncrementNativeSqlSelectClauseColumnCount (size_t value);
    int GetNativeSqlSelectClauseColumnCount () const;

    ExpScope const& GetCurrentScope () const {return m_scopes.Current();}
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
