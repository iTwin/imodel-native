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
    struct SelectionOptions : NonCopyableClass
        {
        private:            
            static bool IsSystemProperty (ECPropertyCR ecProperty)
                {
                static std::set<WCharCP, CompareIWChar> s_systemProperties
                    {
                    ECDbSystemSchemaHelper::ECARRAYINDEX_PROPNAME_W,
                    ECDbSystemSchemaHelper::ECINSTANCEID_PROPNAME_W,
                    ECDbSystemSchemaHelper::ECPROPERTYID_PROPNAME_W,
                    ECDbSystemSchemaHelper::OWNERECINSTANCEID_PROPNAME_W,
                    ECDbSystemSchemaHelper::SOURCEECCLASSID_PROPNAME_W,
                    ECDbSystemSchemaHelper::SOURCEECINSTANCEID_PROPNAME_W,
                    ECDbSystemSchemaHelper::TARGETECCLASSID_PROPNAME_W,
                    ECDbSystemSchemaHelper::TARGETECINSTANCEID_PROPNAME_W
                };
                return s_systemProperties.find (ecProperty.GetName().c_str()) != s_systemProperties.end ();
                }

        private:
            bool m_onlyInlcudeSystemProperties;
            std::set<ECPropertyCP> m_selection;
        public:
            explicit SelectionOptions (bool onlyIncludeSystemProperties)
                :m_onlyInlcudeSystemProperties (onlyIncludeSystemProperties)
                {
                }
            SelectionOptions ()
                :m_onlyInlcudeSystemProperties (false)
                {
                }
            ~SelectionOptions ()
                {}
            void AddProperty (ECPropertyCR ecProperty)
                {
                m_selection.insert (&ecProperty);
                }
            bool IsSelected (ECPropertyCR ecProperty) const
                {
                if (m_onlyInlcudeSystemProperties == false)
                    {
                    if (m_selection.find (&ecProperty) != m_selection.end ())
                        return true;
                    }

                return SelectionOptions::IsSystemProperty (ecProperty);
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
    SelectionOptions m_selectionOptions;
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
