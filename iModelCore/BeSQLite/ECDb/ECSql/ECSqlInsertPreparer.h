/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlInsertPreparer.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ECSqlPreparer.h"
#include "StructArrayToSecondaryTableECSqlBinder.h"

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
    
    static ECSqlStatus PrepareInsertIntoClass (ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, IClassMap const& classMap);
    static ECSqlStatus PrepareInsertIntoRelationship (ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, InsertStatementExp const& exp, IClassMap const& classMap);
    static ECSqlStatus PrepareInsertIntoLinkTableRelationship (ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, RelationshipClassMapCR relationshipClassMap, ECN::ECClassId sourceECClassId, ECN::ECClassId targetECClassId);
    static ECSqlStatus PrepareInsertIntoEndTableRelationship (ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, InsertStatementExp const& exp, RelationshipClassMapCR relationshipClassMap, ECN::ECClassId sourceECClassId, ECN::ECClassId targetECClassId);

    static ECSqlStatus GenerateNativeSqlSnippets (NativeSqlSnippets& insertNativeSqlSnippets, ECSqlPrepareContext& ctx, InsertStatementExp const& exp, IClassMap const& classMap);
    static void PreparePrimaryKey (ECSqlPrepareContext& ctx, NativeSqlSnippets& nativeSqlSnippets, IClassMap const& classMap);
    static ECSqlStatus PrepareConstraintClassId (NativeSqlSnippets& insertNativeSqlSnippets, ECSqlPrepareContext& ctx, PropertyMapCR constraintClassIdPropMap, ECN::ECClassId constraintClassId);

    //! Checks whether for the given constraint a class id is necessary and if yes whether the one specified in the ECSQL is valid or not.
    //! If validation was successful, the class id is returned.
    static ECSqlStatus ValidateConstraintClassId (ECN::ECClassId& retrievedConstraintClassId, ECSqlPrepareContext& ctx, InsertStatementExp const& exp, RelationshipClassMapCR relationshipClassMap, ECN::ECRelationshipEnd constraintEnd);

    static ECSqlStatus GetConstraintClassIdExpValue (bool& isParameter, ECN::ECClassId& constraintClassId, ECSqlPrepareContext& ctx, RowValueConstructorListExp const& valueListExp, size_t valueExpIndex, Utf8CP constraintClassIdPropertyName);
    static int GetConstraintClassIdExpIndex (InsertStatementExp const& exp, ECN::ECRelationshipEnd constraintEnd);

    static void BuildNativeSqlInsertStatement (NativeSqlBuilder& insertBuilder, NativeSqlSnippets const& insertNativeSqlSnippets);
    static void BuildNativeSqlUpdateStatement (NativeSqlBuilder& updateBuilder, NativeSqlSnippets const& insertNativeSqlSnippets, std::vector<size_t> const& expIndexSkipList, RelationshipClassEndTableMapCR classMap);

    static ECInstanceIdMode ValidateUserProvidedECInstanceId (int& ecinstanceIdExpIndex, ECSqlPrepareContext& ctx, InsertStatementExp const& exp, IClassMap const& classMap);

    static ECSqlStatus SetupBindStructArrayParameter(StructArrayToSecondaryTableECSqlBinder* structArrayBinder, PropertyMapCR propertyMap, ECSqlNonSelectPreparedStatement* noneSelectPreparedStmt, ECSqlPrepareContext& ctx, InsertStatementExp const& exp);
    static ECSqlStatus SetupBindStructParameter(ECSqlBinder* binder, PropertyMapCR propertyMap, ECSqlNonSelectPreparedStatement* noneSelectPreparedStmt, ECSqlPrepareContext& ctx, InsertStatementExp const& exp);
    static int GetParamterCount(Exp const& exp, std::set<ParameterExp const*>& namedParameterList);
    static ECSqlStatus PrepareStepTask (ECSqlPrepareContext& ctx, InsertStatementExp const& exp);

public:
    static ECSqlStatus Prepare (ECSqlPrepareContext& ctx, InsertStatementExp const& exp);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE