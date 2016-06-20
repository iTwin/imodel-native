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
        NativeSqlBuilder::ListOfLists m_propertyNamesNativeSqlSnippets;
        NativeSqlBuilder::List m_pkColumnNamesNativeSqlSnippets; 
        NativeSqlBuilder::ListOfLists m_valuesNativeSqlSnippets;
        NativeSqlBuilder::List m_pkValuesNativeSqlSnippets;
        };


    //static class
    ECSqlInsertPreparer ();
    ~ECSqlInsertPreparer ();
    
    static ECSqlStatus PrepareInsertIntoClass (ECSqlPrepareContext&, NativeSqlSnippets& nativeSqlSnippets, ClassMap const& classMap);
    static ECSqlStatus PrepareInsertIntoRelationship (ECSqlPrepareContext&, NativeSqlSnippets& nativeSqlSnippets, InsertStatementExp const& exp, ClassMap const& classMap);
    static ECSqlStatus PrepareInsertIntoLinkTableRelationship (ECSqlPrepareContext&, NativeSqlSnippets& nativeSqlSnippets, RelationshipClassMapCR relationshipClassMap, ECN::ECClassId sourceECClassId, ECN::ECClassId targetECClassId);
    static ECSqlStatus PrepareInsertIntoEndTableRelationship (ECSqlPrepareContext&, NativeSqlSnippets& nativeSqlSnippets, InsertStatementExp const& exp, RelationshipClassMapCR relationshipClassMap, ECN::ECClassId sourceECClassId, ECN::ECClassId targetECClassId);

    static ECSqlStatus GenerateNativeSqlSnippets (NativeSqlSnippets& insertNativeSqlSnippets, ECSqlPrepareContext&, InsertStatementExp const&, ClassMap const&);
    static void PreparePrimaryKey (ECSqlPrepareContext&, NativeSqlSnippets& nativeSqlSnippets, ClassMap const&);
    static ECSqlStatus PrepareConstraintClassId (NativeSqlSnippets& insertNativeSqlSnippets, ECSqlPrepareContext&, RelConstraintECClassIdPropertyMap const&, ECN::ECClassId constraintClassId);

    //! Checks whether for the given constraint a class id is necessary and if yes whether the one specified in the ECSQL is valid or not.
    //! If validation was successful, the class id is returned.
    static ECSqlStatus ValidateConstraintClassId (ECN::ECClassId& retrievedConstraintClassId, ECSqlPrepareContext&, InsertStatementExp const& exp, RelationshipClassMapCR relationshipClassMap, ECN::ECRelationshipEnd constraintEnd);

    static ECSqlStatus GetConstraintClassIdExpValue (bool& isParameter, ECN::ECClassId& constraintClassId, ECSqlPrepareContext&, ValueExpListExp const& valueListExp, size_t valueExpIndex, Utf8CP constraintClassIdPropertyName);
    static int GetConstraintClassIdExpIndex (InsertStatementExp const&, ECN::ECRelationshipEnd constraintEnd);

    static void BuildNativeSqlInsertStatement (NativeSqlBuilder& insertBuilder, NativeSqlSnippets const& insertNativeSqlSnippets);
    static void BuildNativeSqlUpdateStatement (NativeSqlBuilder& updateBuilder, NativeSqlSnippets const& insertNativeSqlSnippets, std::vector<size_t> const& expIndexSkipList, RelationshipClassEndTableMap const& classMap);
    static ECInstanceIdMode ValidateUserProvidedECInstanceId (int& ecinstanceIdExpIndex, ECSqlPrepareContext&, InsertStatementExp const&, ClassMap const&);

public:
    static ECSqlStatus Prepare (ECSqlPrepareContext&, InsertStatementExp const&);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE