/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SqlUtilities.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Describe which command to generate in sql
// @bsiclass                                               Affan.Khan          09/2015
//+===============+===============+===============+===============+===============+======
enum class SqlOption
    {
    Create,
    CreateIfNotExist,
    Drop,
    DropIfExists
    };

//=======================================================================================
//! Allow to build a sql trigger
// @bsiclass                                               Affan.Khan          09/2015
//+===============+===============+===============+===============+===============+======
struct SqlTriggerBuilder
    {
    public:
        enum class Condition
            {
            After,
            Before,
            InsteadOf
            };

        enum class Type
            {
            Insert,
            Update,
            UpdateOf,
            Delete
            };
        //=======================================================================================
        //! Describe collection of trigger with create/delete
        // @bsiclass                                               Affan.Khan          09/2015
        //+===============+===============+===============+===============+===============+======
        struct TriggerList
            {
            typedef std::vector<SqlTriggerBuilder> List;
            private:
                List m_list;

            public:
                TriggerList();
                SqlTriggerBuilder& Create(SqlTriggerBuilder::Type type, SqlTriggerBuilder::Condition condition, bool temprary);
                List const& GetTriggers() const;
                void Delete(SqlTriggerBuilder const& trigger);
            };

    private:
        NativeSqlBuilder m_name;
        NativeSqlBuilder m_when;
        NativeSqlBuilder m_body;
        NativeSqlBuilder m_on;
        bool m_temprory;
        Type m_type;
        Condition m_condition;
        std::vector<Utf8String> m_ofColumns;

    public:
        SqlTriggerBuilder() {}
        SqlTriggerBuilder(Type type, Condition condition, bool temprary);
        SqlTriggerBuilder(SqlTriggerBuilder&& rhs);
        SqlTriggerBuilder& operator= (SqlTriggerBuilder&& rhs);
        SqlTriggerBuilder(SqlTriggerBuilder const& rhs);
        SqlTriggerBuilder& operator= (SqlTriggerBuilder const& rhs);
        NativeSqlBuilder& GetNameBuilder();
        NativeSqlBuilder& GetWhenBuilder();
        NativeSqlBuilder& GetBodyBuilder();
        NativeSqlBuilder& GetOnBuilder();
        Utf8CP GetName() const;
        Utf8CP GetWhen() const;
        Utf8CP GetBody() const;
        Utf8CP GetOn() const;
        bool IsTemporary() const;
        bool IsValid() const;
        Utf8String ToString(SqlOption option, bool escape) const;
        bool IsEmpty() const { return m_body.IsEmpty(); }
    };

//=======================================================================================
//! Describe a sql view
// @bsiclass                                               Affan.Khan          09/2015
//+===============+===============+===============+===============+===============+======
struct SqlViewBuilder
    {
private:
    Utf8String m_name;
    bool m_isNullView;
    NativeSqlBuilder::List m_selectStatementList;
    Utf8String m_sqlComment;

public:
    explicit SqlViewBuilder(Utf8CP viewName, bool isNullView, NativeSqlBuilder::List const& selectStatements) : m_name(viewName), m_isNullView(isNullView), m_selectStatementList(selectStatements) {}

    Utf8CP GetName() const { return m_name.c_str(); }
    bool IsNullView() const { return m_isNullView; }
    void SetComment(Utf8CP comment) { m_sqlComment.assign(comment); }
    bool IsEmpty() const { return m_name.empty() && m_selectStatementList.empty(); }
    bool IsValid() const;
    Utf8String ToString(SqlOption option, bool escape = false, bool useUnionAll = true) const;
    };



END_BENTLEY_SQLITE_EC_NAMESPACE