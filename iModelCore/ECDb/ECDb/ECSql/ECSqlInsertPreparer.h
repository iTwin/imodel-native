/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlInsertPreparer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlPreparer.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    11/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlInsertPreparer
    {
private:
    enum class ECInstanceIdMode
        {
        Invalid = 0,
        NotUserProvided = 1,
        UserProvidedNotNull = 2,
        UserProvidedNull = 4
        };

    struct NativeSqlSnippets
        {
        ECInstanceIdMode m_ecinstanceIdMode;
        int m_ecinstanceIdExpIndex;
        NativeSqlBuilder m_classNameNativeSqlSnippet;
        std::vector<size_t> m_overflowPropertyIndexes;
        NativeSqlBuilder::ListOfLists m_propertyNamesNativeSqlSnippets;
        NativeSqlBuilder::List m_pkColumnNamesNativeSqlSnippets; 
        NativeSqlBuilder::ListOfLists m_valuesNativeSqlSnippets;
        NativeSqlBuilder::List m_pkValuesNativeSqlSnippets;
        };


    //static class
    ECSqlInsertPreparer ();
    ~ECSqlInsertPreparer ();
    
    static ECSqlStatus PrepareInsertIntoClass (ECSqlPrepareContext&, NativeSqlSnippets& nativeSqlSnippets, ClassMap const& classMap, InsertStatementExp const& exp);
    static ECSqlStatus PrepareInsertIntoRelationship (ECSqlPrepareContext&, NativeSqlSnippets& nativeSqlSnippets, InsertStatementExp const& exp, ClassMap const& classMap);
    static ECSqlStatus PrepareInsertIntoLinkTableRelationship (ECSqlPrepareContext&, NativeSqlSnippets&, InsertStatementExp const&, RelationshipClassLinkTableMap const&);
    static ECSqlStatus PrepareInsertIntoEndTableRelationship (ECSqlPrepareContext&, NativeSqlSnippets&, InsertStatementExp const&, RelationshipClassEndTableMap const&);

    static ECSqlStatus GenerateNativeSqlSnippets (NativeSqlSnippets& insertNativeSqlSnippets, ECSqlPrepareContext&, InsertStatementExp const&, ClassMap const&);
    static void PreparePrimaryKey (ECSqlPrepareContext&, NativeSqlSnippets& nativeSqlSnippets, ClassMap const&);

    static void BuildNativeSqlInsertStatement (NativeSqlBuilder& insertBuilder, NativeSqlSnippets const& insertNativeSqlSnippets, InsertStatementExp const& exp);
    static void BuildNativeSqlUpdateStatement (NativeSqlBuilder& updateBuilder, NativeSqlSnippets const& insertNativeSqlSnippets, std::vector<size_t> const& expIndexSkipList, RelationshipClassEndTableMap const& classMap);
    static ECInstanceIdMode ValidateUserProvidedECInstanceId (int& ecinstanceIdExpIndex, ECSqlPrepareContext&, InsertStatementExp const&, ClassMap const&);

public:
    static ECSqlStatus Prepare (ECSqlPrepareContext&, InsertStatementExp const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE